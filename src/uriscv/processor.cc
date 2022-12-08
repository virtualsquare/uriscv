/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2020 Mattia Biondi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA. */
/*
 * This module implements the Processor class. A Processor object emulates
 * MIPS R2/3000 processor features almost perfectly. It is linked to a
 * SystemBus object for physical memory accesses.
 *
 * MIPS processor features are too complex to be fully descripted
 * here: refer to external documentation.
 *
 * This module also contains TLBEntry class definition: it is used to
 * streamline TLB build and handling in Processor.
 */

#include "uriscv/processor.h"

#include <cassert>
#include <complex>

#include "uriscv/const.h"
#include "uriscv/cp0.h"
#include "uriscv/disassemble.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/processor_defs.h"
#include "uriscv/systembus.h"
#include "uriscv/utility.h"

const char *const regName[] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0/fp", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3",    "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5",    "t6"};

// Names of exceptions
HIDDEN const char *const excName[] = {
    "NO EXCEPTION", "INT",  "MOD",  "TLBL Refill", "TLBL", "TLBS Refill",
    "TLBS",         "ADEL", "ADES", "DBE",         "IBE",  "SYS",
    "BP",           "RI",   "CPU",  "OV"};

// exception code table (each corresponding to an exception cause);
// each exception cause is mapped to one exception type expressed in
// CAUSE register field format
HIDDEN const Word excCode[] = {0UL, 0UL, 1UL, 2UL, 2UL, 3UL,  3UL,  4UL,
                               5UL, 6UL, 7UL, 8UL, 9UL, 10UL, 11UL, 12UL};

// Each TLBEntry object represents a single entry in the TLB contained in
// the CP0 coprocessor part of a real MIPS processor.
// Each one is a 64-bit field split in two parts (HI and LO), with special
// fields and control bits (see external documentation for more details).

class TLBEntry {
public:
  // This method builds an entry and sets its initial contents
  TLBEntry(Word entHI = 0, Word entLO = 0);

  // This method returns the HI 32-bit part of the entry
  Word getHI() const { return tlbHI; }

  // This method returns the LO 32-bit part of the entry
  Word getLO() const { return tlbLO; }

  // This method sets the entry HI part (leaving the zero-filled
  // field untouched)
  void setHI(Word entHI);

  // This method sets the entry LO part (leaving the zero-filled field
  // untouched)
  void setLO(Word entLO);

  // This method compares the entry contents with the VPN part of a
  // virtual address and returns TRUE if a match is found, FALSE
  // otherwise
  bool VPNMatch(Word vaddr);

  // This method compares the entry contents with the ASID field in a
  // CP0 special register and returns TRUE if a match is found, FALSE
  // otherwise
  bool ASIDMatch(Word cpreg);

  // the following methods return the bit value for the corresponding
  // access control bit
  bool IsG();
  bool IsV();
  bool IsD();

private:
  // contains the VPN + ASID fields, and a zero-filled field
  Word tlbHI;

  // contains the PFN field, some access control bits and a
  // zero-filled field
  Word tlbLO;
};

// This method builds an entry and sets its initial contents
TLBEntry::TLBEntry(Word entHI, Word entLO) {
  tlbHI = entHI;
  tlbLO = entLO;
}

// This method sets the entry HI part (leaving the zero-filled field untouched)
void TLBEntry::setHI(Word entHI) { tlbHI = (entHI & (VPNMASK | ASIDMASK)); }

// This method sets the entry LO part (leaving the zero-filled field untouched)
void TLBEntry::setLO(Word entLO) { tlbLO = (entLO & ENTRYLOMASK); }

// This method compares the entry contents with the VPN part of a virtual
// address and returns TRUE if a match is found, FALSE otherwise
bool TLBEntry::VPNMatch(Word vaddr) { return VPN(tlbHI) == VPN(vaddr); }

// This method compares the entry contents with the ASID field in a CP0
// special register and returns TRUE if a match is found, FALSE otherwise
bool TLBEntry::ASIDMatch(Word cpreg) { return ASID(tlbHI) == ASID(cpreg); }

// This method returns the value of G (Global) access control bit in an
// entry LO part
bool TLBEntry::IsG() { return BitVal(tlbLO, GBITPOS); }

// This method returns the value of V (Valid) access control bit in an
// entry LO part
bool TLBEntry::IsV() { return BitVal(tlbLO, VBITPOS); }

// This method returns the value of D (Dirty) access control bit in an
// entry LO part
bool TLBEntry::IsD() { return BitVal(tlbLO, DBITPOS); }

Processor::Processor(const MachineConfig *config, Word cpuId, Machine *machine,
                     SystemBus *bus)
    : id(cpuId), config(config), machine(machine), bus(bus), status(PS_HALTED),
      tlbSize(config->getTLBSize()), tlb(new TLBEntry[tlbSize]),
      tlbFloorAddress(config->getTLBFloorAddress()) {
  initCSR();
}

Processor::~Processor() {}

void Processor::initCSR() {
  csr[CYCLE].perm = RRR;
  csr[CYCLEH].perm = NNW;
  csr[MCYCLE].perm = RRR;
  csr[MCYCLEH].perm = RRR;

  csr[TIME].perm = RRR;
  csr[TIMEH].perm = RRR;

  csr[INSTRET].perm = RRR;
  csr[MINSTRET].perm = NNW;
  csr[INSTRETH].perm = RRR;
  csr[MINSTRETH].perm = NNW;

  csr[0x000].perm = WWW;
  csr[0x100].perm = NWW;
  csr[0x300].perm = NNW;

  csr[0x004].perm = WWW;
  csr[0x104].perm = NWW;
  csr[0x304].perm = NNW;

  csr[UTVEC].perm = WWW;
  csr[STVEC].perm = NWW;
  csr[MTVEC].perm = NNW;

  csr[0x102].perm = NWW;
  csr[0x302].perm = NNW;

  csr[0x103].perm = NWW;
  csr[0x303].perm = NNW;

  csr[0x106].perm = NWW;
  csr[0x306].perm = NNW;

  for (Word i = 0; i < 5; i++) {
    csr[0x040 + i].perm = WWW;
    csr[0x140 + i].perm = NWW;
    csr[0x340 + i].perm = NNW;
  }

  csr[0x180].perm = NWW;

  csr[0x301].perm = NNW;
  csr[0xF11].perm = NNR;
  csr[0xF12].perm = NNR;
  csr[0xF13].perm = NNR;
  csr[0xF14].perm = NNR;

  csr[0x001].perm = WWW;
  csr[0x002].perm = WWW;
  csr[0x003].perm = WWW;

  for (Word i = 0; i < 0x33F - 0x323 + 1; i++) {
    csr[0x323 + i].perm = NNW;
    csr[0xC03 + i].perm = RRR;
    csr[0xB03 + i].perm = NNW;
    csr[0xC83 + i].perm = RRR;
    csr[0xB83 + i].perm = NNW;
  }

  csr[0x3A0].perm = NNW;
  csr[0x3A1].perm = NNW;
  csr[0x3A2].perm = NNW;
  csr[0x3A3].perm = NNW;

  for (Word i = 0; i < 0xF; i++) {
    csr[0x3B0 + i].perm = NNW;
  }

  for (Word i = 0; i < 0x7B2 - 0x7A0 + 1; i++) {
    csr[0x7A0 + i].perm = NNW;
  }
}

void Processor::setStatus(ProcessorStatus newStatus) {
  if (status != newStatus) {
    status = newStatus;
    StatusChanged.emit();
  }
}

// This method puts Processor in startup state. This is done following the
// MIPS conventions on register fields to be set; it also pre-loads the
// first instruction since Cycle() goes on with execute-load loop
void Processor::Reset(Word pc, Word sp) {
  unsigned int i;

  // first instruction is not in a branch delay slot
  isBranchD = false;

  // no exception pending at start
  excCause = NOEXCEPTION;
  copENum = 0;

  // no loads pending at start
  loadPending = LOAD_TARGET_NONE;
  loadReg = 0;
  loadVal = MAXSWORDVAL;

  // clear general purpose registers
  for (i = 0; i < CPUREGNUM; i++)
    gpr[i] = 0;
  gpr[29] = sp;

  // no previous instruction is available
  prevPC = MAXWORDVAL;
  prevPhysPC = MAXWORDVAL;
  prevInstr = NOP;

  // clears CP0 registers and then sets them
  for (i = 0; i < CP0REGNUM; i++)
    cpreg[i] = 0UL;

  // first instruction is already loaded
  cpreg[RANDOM] = ((tlbSize - 1UL) << RNDIDXOFFS) - RANDOMSTEP;
  cpreg[STATUS] = STATUSRESET;
  cpreg[PRID] = id;

  currPC = pc;

  // maps PC to physical address space and fetches first instruction
  // mapVirtual and SystemBus cannot signal TRUE on this call
  if (mapVirtual(currPC, &currPhysPC, EXEC) ||
      bus->InstrRead(currPhysPC, &currInstr, this))
    Panic("Illegal memory access in Processor::Reset");

  // sets values for following PCs
  nextPC = currPC + WORDLEN;
  succPC = nextPC + WORDLEN;

  setStatus(PS_RUNNING);
}

void Processor::Halt() { setStatus(PS_HALTED); }

// This method makes Processor execute a single instruction.
// For simulation purposes, it differs from traditional processor cycle:
// the first instruction after a reset is pre-loaded, and cycle is
// execute - fetch instead of fetch - execute.
// This way, it is possible to know what instruction will be executed
// before its execution happens
//
// Method works as follows:
// the instruction currently loaded is executed; if no exception occurs,
// interrupt status is checked to see if an INT exception has to be
// processed; if no interrupts are pending, next instruction is located and
// fetched, so Processor is ready for another Cycle(). If an exception
// occurs, first instruction at vector address is loaded, so a new Cycle()
// may start.
// Other MIPS-specific minor tasks are performed at proper points.
void Processor::Cycle() {
  // Nothing to do if the cpu is halted
  if (isHalted())
    return;

  // Update internal timer
  if (cpreg[STATUS] & STATUS_TE) {
    if (cpreg[CP0REG_TIMER] == 0)
      AssertIRQ(IL_CPUTIMER);
    cpreg[CP0REG_TIMER]--;
  } else {
    DeassertIRQ(IL_CPUTIMER);
  }

  // In low-power state, only the per-cpu timer keeps running
  if (isIdle())
    return;

  // Instruction decode & exec
  if (execInstr(currInstr))
    handleExc();

  // Check if we entered sleep mode as a result of the last
  // instruction; if so, we effectively stall the pipeline.
  if (isIdle())
    return;

  // PC saving for book-keeping purposes
  prevPC = currPC;
  prevPhysPC = currPhysPC;
  prevInstr = currInstr;

  // RANDOM register is decremented
  randomRegTick();

  // currPC is loaded so a new cycle fetch may start: this "PC stack" is
  // used to emulate delayed branch slots
  currPC = nextPC;
  nextPC = succPC;
  succPC += WORDLEN;

  // Check for interrupt exception; note that this will _not_
  // trigger another exception if we're already in "exception mode".
  if (checkForInt())
    handleExc();

  // processor cycle fetch part
  if (mapVirtual(currPC, &currPhysPC, EXEC)) {
    // TLB or Address exception caused: current instruction is nullified
    currInstr = NOP;
    handleExc();
  } else if (bus->InstrRead(currPhysPC, &currInstr, this)) {
    // IBE exception caused: current instruction is nullified
    currInstr = NOP;
    handleExc();
  }
}

uint32_t Processor::IdleCycles() const {
  if (isHalted())
    return (uint32_t)-1;
  else if (isIdle())
    return (cpreg[STATUS] & STATUS_TE) ? cpreg[CP0REG_TIMER] : (uint32_t)-1;
  else
    return 0;
}

void Processor::Skip(uint32_t cycles) {
  assert(isIdle() && cycles <= IdleCycles());
  if (cpreg[STATUS] & STATUS_TE)
    cpreg[CP0REG_TIMER] -= cycles;
}

// This method allows SystemBus and Processor itself to signal Processor
// when an exception happens. SystemBus signal IBE/DBE exceptions; Processor
// itself signal all other kinds of exception.
//
// Method works as follows:
// the exception internal code is put into proper Processor private
// variable(s), and is signaled to Watch (so Watch may stop simulation if
// needed).
// Exception processing is done by Processor private method handleExc()
void Processor::SignalExc(unsigned int exc, Word cpuNum) {
  excCause = exc;
  SignalException.emit(excCause);
  // used only for CPUEXCEPTION handling
  copENum = cpuNum;
}

void Processor::AssertIRQ(unsigned int il) {
  cpreg[CAUSE] |= CAUSE_IP(il);

  // If in standby mode, go back to being a power hog.
  if (isIdle())
    setStatus(PS_RUNNING);
}

void Processor::DeassertIRQ(unsigned int il) { cpreg[CAUSE] &= ~CAUSE_IP(il); }

// This method allows to get critical information on Processor current
// internal status. Parameters are self-explanatory: they are extracted from
// proper places inside Processor itself
void Processor::getCurrStatus(Word *asid, Word *pc, Word *instr, bool *isLD,
                              bool *isBD) {
  *asid = (ASID(cpreg[ENTRYHI])) >> ASIDOFFS;
  *pc = currPC;
  *instr = currInstr;
  *isLD = (loadPending != LOAD_TARGET_NONE);
  *isBD = isBranchD;
}

Word Processor::getASID() const { return ASID(cpreg[ENTRYHI]) >> ASIDOFFS; }

bool Processor::InUserMode() const { return BitVal(cpreg[STATUS], KUCBITPOS); }

bool Processor::InKernelMode() const {
  return (!BitVal(cpreg[STATUS], KUCBITPOS));
}

// This method allows to get Processor previously executed instruction and
// location
void Processor::getPrevStatus(Word *pc, Word *instr) {
  *pc = prevPC;
  *instr = prevInstr;
}

// This method allows to get a human-readable mnemonic expression for the last
// exception happened (thanks to excName[] array)
const char *Processor::getExcCauseStr() {
  // 0 means no exception
  if (excCause)
    return excName[excCause];
  else
    return (EMPTYSTR);
}

// This method allows to add value to the current PC
Word Processor::incrementPC(Word value) {
  currPC += value;
  return currPC;
}

// This method allows to get the physical location of instruction executed
// in the previous Processor Cycle()
Word Processor::getPrevPPC() { return (prevPhysPC); }

// This method allows to get the physical location of instruction executed
// in the current Processor Cycle()
Word Processor::getCurrPPC() { return (currPhysPC); }

// This method allows to get the virtual location of instruction that will
// be executed in the next Processor Cycle()
Word Processor::getNextPC() { return (nextPC); }

// This method allows to get the virtual location of instruction that will
// be executed in the Processor Cycle() _after_ the next
Word Processor::getSuccPC() { return (succPC); }

// This method allows to get the value of the general purpose register
// indexed by num (HI and LO are the last ones in the array)
SWord Processor::getGPR(unsigned int num) { return (gpr[num]); }

// This method allows to get the value of the CP0 special register indexed
// by num. num coding itself is internal (see h/processor.h for mapping)
Word Processor::getCP0Reg(unsigned int num) { return (cpreg[num]); }

void Processor::getTLB(unsigned int index, Word *hi, Word *lo) const {
  *hi = tlb[index].getHI();
  *lo = tlb[index].getLO();
}

Word Processor::getTLBHi(unsigned int index) const {
  return tlb[index].getHI();
}

Word Processor::getTLBLo(unsigned int index) const {
  return tlb[index].getLO();
}

Word Processor::regRead(Word reg) {
  assert(reg >= 0 && reg < CPUGPRNUM);
  if (reg == REG_ZERO)
    return 0;
  return gpr[reg];
}
void Processor::regWrite(Word reg, Word value) {
  assert(reg >= 0 && reg < CPUGPRNUM);
  if (reg != REG_ZERO)
    gpr[reg] = value;
}
Word Processor::csrRead(Word reg) {
  // TODO: check perm
  assert(reg >= 0 && reg < kNumCSRRegisters);
  return csr[reg].value;
}
void Processor::csrWrite(Word reg, Word value) {
  // TODO: check perm
  assert(reg >= 0 && reg < kNumCSRRegisters);
  csr[reg].value = value;
}

// This method allows to modify the current value of a general purpose
// register (HI and LO are the last ones in the array)
void Processor::setGPR(unsigned int num, SWord val) {
  if (num > 0)
    // register $0 is read-only
    gpr[num] = val;
}

// This method allows to modify the current value of a CP0 special
// register. num coding itself is internal (see h/processor.h for mapping)
void Processor::setCP0Reg(unsigned int num, Word val) {
  if (num < CP0REGNUM)
    cpreg[num] = val;
}

// This method allows to modify the current value of nextPC to force sudden
// branch: it violates delayed branch slot conventions
void Processor::setNextPC(Word npc) { nextPC = npc; }

// This method allows to modify the current value of succPC to force sudden
// branch: it does not violate branch slot conventions, if used to change
// the target of a branch already taken
void Processor::setSuccPC(Word spc) { succPC = spc; }

// This method allows to modify the current value of a specified TLB entry
void Processor::setTLB(unsigned int index, Word hi, Word lo) {
  if (index < tlbSize) {
    tlb[index].setHI(hi);
    tlb[index].setLO(lo);
    SignalTLBChanged(index);
  } else {
    Panic("Unknown TLB entry in Processor::setTLB()");
  }
}

void Processor::setTLBHi(unsigned int index, Word value) {
  assert(index < tlbSize);
  tlb[index].setHI(value);
  SignalTLBChanged(index);
}

void Processor::setTLBLo(unsigned int index, Word value) {
  assert(index < tlbSize);
  tlb[index].setLO(value);
  SignalTLBChanged(index);
}

//
// Processor private methods start here
//

// This method advances CP0 RANDOM register, following MIPS conventions; it
// cycles from RANDOMTOP to RANDOMBASE, one STEP less for each clock tick
void Processor::randomRegTick() {
  cpreg[RANDOM] =
      (cpreg[RANDOM] - RANDOMSTEP) & (((tlbSize - 1UL) << RNDIDXOFFS));
  if (cpreg[RANDOM] < RANDOMBASE)
    cpreg[RANDOM] = ((tlbSize - 1UL) << RNDIDXOFFS);
}

// This method pushes the KU/IE bit stacks in CP0 STATUS register to start
// exception handling
void Processor::pushKUIEStack() {
  unsigned int bitp;

  // push the KUIE stack
  for (bitp = KUOBITPOS; bitp > KUCBITPOS; bitp--)
    if (BitVal(cpreg[STATUS], bitp - 2))
      cpreg[STATUS] = SetBit(cpreg[STATUS], bitp);
    else
      cpreg[STATUS] = ResetBit(cpreg[STATUS], bitp);

  // sets to 0 current KU IE bits
  cpreg[STATUS] = ResetBit(cpreg[STATUS], KUCBITPOS);
  cpreg[STATUS] = ResetBit(cpreg[STATUS], IECBITPOS);
}

// This method pops the KU/IE bit stacks in CP0 STATUS register to end
// exception handling. It is invoked on RFE instruction execution
void Processor::popKUIEStack() {
  unsigned int bitp;

  for (bitp = IECBITPOS; bitp < IEOBITPOS; bitp++) {
    if (BitVal(cpreg[STATUS], bitp + 2))
      cpreg[STATUS] = SetBit(cpreg[STATUS], bitp);
    else
      cpreg[STATUS] = ResetBit(cpreg[STATUS], bitp);
  }
}

// This method test for pending interrupts, checking global abilitation (IEc
// bit in STATUS register) and comparing interrupt mask with interrupts
// pending on bus; returns TRUE if an INT exception must be raised, FALSE
// otherwise, and sets CP0 registers if needed
bool Processor::checkForInt() {
  if ((cpreg[STATUS] & STATUS_IEc) &&
      (cpreg[CAUSE] & cpreg[STATUS] & CAUSE_IP_MASK)) {
    SignalExc(INTEXCEPTION);
    // Clear all Cause fields except IP
    cpreg[CAUSE] &= CAUSE_IP_MASK;
    return true;
  } else {
    // No interrupt on this cycle
    return false;
  }
}

/**
 * Try to enter standby mode
 */
void Processor::suspend() {
  if (!(cpreg[CAUSE] & CAUSE_IP_MASK))
    setStatus(PS_IDLE);
}

// This method sets the appropriate CP0 registers at exception
// raising, following the MIPS conventions; it also set the PC value
// to point the appropriate exception handler vector.
void Processor::handleExc() {
  // If there is a load pending, it is completed while the processor
  // prepares for exception handling (a small bubble...).
  completeLoad();

  // set the excCode into CAUSE reg
  cpreg[CAUSE] = IM(cpreg[CAUSE]) | (excCode[excCause] << CAUSE_EXCCODE_BIT);

  if (isBranchD) {
    // previous instr. is branch/jump: must restart from it
    cpreg[CAUSE] = SetBit(cpreg[CAUSE], CAUSE_BD_BIT);
    cpreg[EPC] = prevPC;
    // first instruction in exception handler itself is not in a BD slot
    isBranchD = false;
  } else {
    // BD is already set to 0 by CAUSE masking with IM; sets only EPC
    cpreg[EPC] = currPC;
  }

  // Set coprocessor unusable number in CAUSE register for
  // `Coprocessor Unusable' exceptions
  if (excCause == CPUEXCEPTION)
    cpreg[CAUSE] = cpreg[CAUSE] | (copENum << COPEOFFSET);

  // Set PC to the right handler
  Word excVector;
  if (BitVal(cpreg[STATUS], STATUS_BEV_BIT)) {
    // Machine bootstrap exception handler
    excVector = BOOTEXCBASE;
  } else {
    excVector = KSEG0BASE;
  }

  pushKUIEStack();

  if (excCause == UTLBLEXCEPTION || excCause == UTLBSEXCEPTION)
    excVector += TLBREFOFFS;
  else
    excVector += OTHEREXCOFFS;

  if (excCause == INTEXCEPTION) {
    // interrupt: test is before istruction fetch, so handling
    // could start immediately
    currPC = excVector;
    nextPC = currPC + WORDLEN;
    succPC = nextPC + WORDLEN;
  } else {
    // other exception, at instruction fetch or later: handling
    // could be done only in next processor cycle (current
    // instruction has been nullified)
    nextPC = excVector;
    succPC = nextPC + WORDLEN;
  }
}

// This method zeroes out the TLB
void Processor::zapTLB() {
  // Leave out the first entry ([0])
  for (size_t i = 1; i < tlbSize; ++i) {
    tlb[i].setHI(0);
    tlb[i].setLO(0);
    SignalTLBChanged(i);
  }
}

// This method allows to handle the delayed load slot: it provides to load
// the target register with the needed value during the execution of other
// instructions, when invoked at the appropriate point in the "pipeline"
void Processor::completeLoad() {
  // loadPending tells whether a general purpose or a CP0 special register
  // is the target
  switch (loadPending) {
  case LOAD_TARGET_GPREG:
    if (loadReg != 0)
      gpr[loadReg] = loadVal;
    loadPending = LOAD_TARGET_NONE;
    break;

  case LOAD_TARGET_CPREG:
    // CP0 registers have some zero-filled fields or are read-only
    // so individual handling is needed
    switch (loadReg) {
    case INDEX:
      // loadable part is index field only, and P bit
      // remain unchanged
      cpreg[INDEX] = (cpreg[INDEX] & SIGNMASK) |
                     (((Word)loadVal) & ((tlbSize - 1UL) << RNDIDXOFFS));
      break;

    case ENTRYLO:
      // loadable part is PFN and status bits only
      cpreg[ENTRYLO] = ((Word)loadVal) & ENTRYLOMASK;
      break;

    case CP0REG_TIMER:
      cpreg[CP0REG_TIMER] = (Word)loadVal;
      DeassertIRQ(IL_CPUTIMER);
      break;

    case ENTRYHI:
      // loadable parts are VPN and ASID fields
      cpreg[ENTRYHI] = ((Word)loadVal) & (VPNMASK | ASIDMASK);
      break;

    case STATUS:
      // loadable parts are CU0 bit, TE bit, BEV bit in DS, IM mask and
      // KUIE bit stack
      cpreg[STATUS] = ((Word)loadVal) & STATUSMASK;
      break;

    case EPC:
    case PRID:
    case RANDOM:
    case BADVADDR:
    default:
      // read-only regs: writes have no effects
      break;
    }
    loadPending = LOAD_TARGET_NONE;
    break;

  case LOAD_TARGET_NONE:
  default:
    break;
  }
}

// This method maps the virtual addresses to physical ones following the
// complex mapping algorithm and TLB used by MIPS (see external doc).
// It returns TRUE if conversion was not possible (this implies an exception
// have been raised) and FALSE if conversion has taken place: physical value
// for address conversion is returned thru paddr pointer.
// AccType details memory access type (READ/WRITE/EXECUTE)
bool Processor::mapVirtual(Word vaddr, Word *paddr, Word accType) {
  // SignalProcVAccess() is always done so it is possible
  // to track accesses which produce exceptions
  machine->HandleVMAccess(ENTRYHI_GET_ASID(cpreg[ENTRYHI]), vaddr, accType,
                          this);

  // address validity and bounds check
  if (BADADDR(vaddr) ||
      (InUserMode() && (INBOUNDS(vaddr, KSEG0BASE, KUSEGBASE)))) {
    // bad offset or kernel segment access from user mode
    *paddr = MAXWORDVAL;

    // the bad virtual address is put into BADVADDR reg
    cpreg[BADVADDR] = vaddr;

    if (accType == WRITE)
      SignalExc(ADESEXCEPTION);
    else
      SignalExc(ADELEXCEPTION);

    return true;
  } else if (INBOUNDS(vaddr, KSEG0BASE, tlbFloorAddress)) {
    // no bad offset; if vaddr < KUSEGBASE the processor is surely
    // in kernelMode
    // valid access to KSEG0 area
    *paddr = vaddr;
    return false;
  }

  // The access is in user mode to user space, or in kernel mode
  // to KSEG0 or KUSEG spaces.

  unsigned int index;
  if (probeTLB(&index, cpreg[ENTRYHI], vaddr)) {
    if (tlb[index].IsV()) {
      if (accType != WRITE || tlb[index].IsD()) {
        // All OK
        *paddr = PHADDR(vaddr, tlb[index].getLO());
        return false;
      } else {
        // write operation on frame with D bit set to 0
        *paddr = MAXWORDVAL;
        setTLBRegs(vaddr);
        SignalExc(MODEXCEPTION);
        return true;
      }
    } else {
      // invalid access to frame with V bit set to 0
      *paddr = MAXWORDVAL;
      setTLBRegs(vaddr);
      if (accType == WRITE)
        SignalExc(TLBSEXCEPTION);
      else
        SignalExc(TLBLEXCEPTION);

      return true;
    }
  } else {
    // bad or missing VPN match: Refill event required
    *paddr = MAXWORDVAL;
    setTLBRegs(vaddr);
    if (accType == WRITE)
      SignalExc(UTLBSEXCEPTION);
    else
      SignalExc(UTLBLEXCEPTION);

    return true;
  }
}

// This method sets the CP0 special registers on exceptions forced by TLB
// handling (see mapVirtual() for invocation/specific cases).
void Processor::setTLBRegs(Word vaddr) {
  // Note that ENTRYLO is left undefined!
  cpreg[BADVADDR] = vaddr;
  cpreg[ENTRYHI] = VPN(vaddr) | ASID(cpreg[ENTRYHI]);
}

// This method make Processor execute a single MIPS instruction, emulating
// pipeline constraints and load delay slots (see external doc).
bool Processor::execInstr(Word instr) {
  Word e = NOEXCEPTION;
  uint8_t opcode = OPCODE(instr);
  uint8_t func3 = FUNC3(instr);
  const Symbol *sym =
      machine->getStab()->Probe(config->getSymbolTableASID(), getPC(), true);
  if (sym != NULL && sym->getName() != prevFunc) {
    prevFunc = sym->getName();
    DEBUGMSG("FUN %s\n", sym->getName());
  }
  DEBUGMSG("[%x] (%08x) ", getPC(), instr);

  switch (opcode) {
  case OP_L: {
    DEBUGMSG("\tI-type | ");
    int8_t rd = RD(instr);
    int8_t rs1 = RS1(instr);
    int16_t imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    Word read = 0;
    Word vaddr = regRead(rs1) + imm, paddr = 0;
    switch (func3) {
    case OP_LB: {
      DEBUGMSG("LB\n");
      // just 8 bits
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !this->bus->DataRead((SWord)regRead(rs1) + (SWord)imm, &read, this)) {
        regWrite(rd, signExtByte(read, BYTEPOS(vaddr)));
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      exit(-1);
      break;
    }
    case OP_LH: {
      DEBUGMSG("LBH\n");
      // just 16 bits
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !this->bus->DataRead((SWord)regRead(rs1) + (SWord)imm, &read, this)) {
        regWrite(rd, signExtHWord(read, HWORDPOS(vaddr)));
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    case OP_LW: {
      if (!mapVirtual(vaddr, &paddr, READ) &&
          !this->bus->DataRead((SWord)regRead(rs1) + (SWord)imm, &read, this)) {
        regWrite(rd, read);
        DEBUGMSG("LW %s,%s(%x),%d -> %x\n", regName[rd], regName[rs1],
                 regRead(rs1), (Word)imm, read);
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    case OP_LBU: {
      // just 8 bits
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !this->bus->DataRead((SWord)regRead(rs1) + (SWord)imm, &read, this)) {
        DEBUGMSG("LBU %s,%s(%x),%d -> %x\n", regName[rd], regName[rs1],
                 regRead(rs1), imm, read);
        regWrite(rd, zExtByte(read, BYTEPOS(vaddr)));
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    case OP_LHU: {
      // just 16 bits
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !this->bus->DataRead((SWord)regRead(rs1) + (SWord)imm, &read, this)) {
        DEBUGMSG("LHU\n");
        regWrite(rd, zExtHWord(read, HWORDPOS(vaddr)));
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      break;
    }
    }
    break;
  }
  case R_TYPE: {
    DEBUGMSG("\tR-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    uint8_t rd = RD(instr);
    uint8_t func7 = FUNC7(instr);
    switch (func3) {
    case OP_ADD: {
      switch (func7) {
      case OP_ADD_func7: {
        DEBUGMSG("ADD %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
        regWrite(rd, regRead(rs1) + regRead(rs2));
        break;
      }

      case OP_SUB_func7: {
        DEBUGMSG("SUB %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
        regWrite(rd, regRead(rs1) - regRead(rs2));
        break;
      }
      default: {
        SignalExc(CPUEXCEPTION, 0);
        e = true;
        break;
      }
      }
      break;
    }
    case OP_OR: {
      DEBUGMSG("OR %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) | regRead(rs2));
      break;
    }
    case OP_XOR: {
      DEBUGMSG("XOR %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) ^ regRead(rs2));
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      break;
    }
    }
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case I_TYPE: {
    DEBUGMSG("\tI-type | ");
    int8_t rs1 = RS1(instr);
    SWord imm = I_IMM(instr);
    uint8_t rd = RD(instr);
    switch (func3) {
    case OP_ADDI: {
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      DEBUGMSG("ADDI %s,%s(%x),%d\n", regName[rd], regName[rs1], regRead(rs1),
               imm);
      regWrite(rd, regRead(rs1) + imm);
      break;
    }
    case OP_SLLI: {
      DEBUGMSG("SLLI %s(%x),%s(%x),%d\n", regName[rd], regRead(rd),
               regName[rs1], regRead(rs1), imm);
      regWrite(rd, regRead(rs1) << imm);
      break;
    }
    case OP_SLTI: {
      DEBUGMSG("SLTI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      regWrite(rd, SWord(regRead(rs1)) < SWord(imm) ? 1 : 0);
      break;
    }
    case OP_SLTIU: {
      DEBUGMSG("SLTIU\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      regWrite(rd, regRead(rs1) < Word(imm) ? 1 : 0);
      break;
    }
    case OP_XORI: {
      DEBUGMSG("XORI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      regWrite(rd, SWord(regRead(rs1)) ^ SWord(imm));
      break;
    }
    case OP_SR: {
      uint8_t func7 = FUNC7(instr);
      switch (func7) {
      case OP_SRLI_func7: {
        DEBUGMSG("SRLI\n");
        regWrite(rd, regRead(rs1) >> imm);
        break;
      }
      case OP_SRAI_func7: {
        DEBUGMSG("SRAI\n");
        uint8_t msb = rs1 & 0x80000000;
        regWrite(rd, regRead(rs1) >> imm | msb);
        break;
      }
      default:
        SignalExc(CPUEXCEPTION, 0);
        e = true;
        break;
      }
      regWrite(rd, SWord(regRead(rs1)) ^ SWord(imm));
      break;
    }
    case OP_ORI: {
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      DEBUGMSG("ORI %s,%s(%x),%x\n", regName[rd], regName[rs1], regRead(rs1),
               imm);
      regWrite(rd, SWord(regRead(rs1)) | SWord(imm));
      break;
    }
    case OP_ANDI: {
      DEBUGMSG("ANDI\n");
      imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
      regWrite(rd, SWord(regRead(rs1)) & SWord(imm));
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      break;
    }
    }
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case I2_TYPE: {
    DEBUGMSG("\tI2-type | ");
    uint16_t imm = I_IMM(instr);
    int8_t rs1 = RS1(instr);
    uint8_t rd = RD(instr);
    switch (func3) {
    case OP_ECALL_EBREAK: {
      switch (imm) {
      case ECALL_IMM: {
        // a7 - syscall number
        // a0 - result of syscall
        //
        // https://www.cs.cornell.edu/courses/cs3410/2019sp/schedule/slides/14-ecf-pre.pdf
        DEBUGMSG("ECALL\n");
        ERROR("ECALL");
        // TODO
        // switch (this->mode) {
        // case MODE_USER: {
        //   return EXC_ENV_CALL_U;
        // }
        // case MODE_SUPERVISOR: {
        //   return EXC_ENV_CALL_S;
        // }
        // case MODE_MACHINE: {
        //   return EXC_ENV_CALL_M;
        // }
        // }
        break;
      }
      case EBREAK_IMM: {
        DEBUGMSG("EBREAK\n");
        ERROR("BREAK");
        break;
      }
      case EWFI_IMM: {
        DEBUGMSG("EWFI\n");
        suspend();
        break;
      }
      default: {
        SignalExc(CPUEXCEPTION, 0);
        e = true;
        ERROR("not found");
        break;
      }
      }
      break;
    }
    case OP_CSRRW: {
      std::cout << "CSRRW\n";
      if (rd != 0x0)
        regWrite(rd, csrRead(imm));
      if (rs1 != 0x0)
        csrWrite(imm, regRead(rs1));
      setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_CSRRS: {
      std::cout << "CSRRS\n";
      regWrite(rd, csrRead(imm));
      csrWrite(imm, csrRead(imm) | regRead(rs1));
      setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_CSRRC: {
      std::cout << "CSRRC\n";
      regWrite(rd, csrRead(imm));
      if (rs1 != 0x0)
        csrWrite(imm, csrRead(imm) & ~regRead(rs1));
      setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_CSRRWI: {
      std::cout << "CSRRWI\n";
      regWrite(rd, csrRead(imm));
      csrWrite(imm, csrRead(imm) & ~rs1);
      setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_CSRRSI: {
      std::cout << "CSRRSI\n";
      regWrite(rd, csrRead(imm));
      csrWrite(imm, csrRead(imm) | rs1);
      setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_CSRRCI: {
      std::cout << "CSRRCI\n";
      regWrite(rd, csrRead(imm));
      csrWrite(imm, csrRead(imm) & ~rs1);
      setNextPC(getPC() + WORDLEN);
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      ERROR("not found");
      break;
    }
    }
    break;
  }
  case B_TYPE: {
    DEBUGMSG("\tB-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    int16_t imm = B_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    switch (func3) {
    case OP_BEQ: {
      DEBUGMSG("BEQ %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
               regName[rs2], regRead(rs2), imm);
      if (regRead(rs1) == regRead(rs2))
        setNextPC((SWord)getPC() + ((SWord)imm));
      else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_BNE: {
      DEBUGMSG("BNE %s(%d),%s(%d),%d\n", regName[rs1], regRead(rs1),
               regName[rs2], regRead(rs2), imm);
      if (regRead(rs1) != regRead(rs2))
        setNextPC((SWord)getPC() + ((SWord)imm));
      else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_BLT: {
      std::cout << "BLT\n";
      if (regRead(rs1) < regRead(rs2))
        setNextPC((SWord)getPC() + ((SWord)imm));
      else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_BGE: {
      std::cout << "BGE\n";
      if (regRead(rs1) >= regRead(rs2)) {
        setNextPC((SWord)getPC() + ((SWord)imm));
      } else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_BLTU: {
      std::cout << "BLTU\n";
      if (regRead(rs1) < regRead(rs2))
        setNextPC(getPC() + (imm));
      else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    case OP_BGEU: {
      std::cout << "BGEU\n";
      if (regRead(rs1) >= regRead(rs2))
        setNextPC(getPC() + (imm));
      else
        setNextPC(getPC() + WORDLEN);
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      break;
    }
    }
    break;
  }
  case S_TYPE: {
    DEBUGMSG("\tS-type | ");
    int8_t rs1 = RS1(instr);
    int8_t rs2 = RS2(instr);
    int16_t imm = S_IMM(instr);
    imm = SIGN_EXTENSION(imm, S_IMM_SIZE);
    Word vaddr = (SWord)regRead(rs1) + (SWord)imm;
    Word paddr = 0;
    Word old = 0;
    e = this->bus->DataRead(vaddr, &old, this);
    switch (func3) {
    case OP_SB: {
      if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
          !bus->DataRead(paddr, &old, this)) {
        old = mergeByte(old, regRead(rs2), BYTEPOS(vaddr));
        e = this->bus->DataWrite(paddr, old, this);
        DEBUGMSG("SB %s(%x),%s(%x),%d -> %x\n", regName[rs2], regRead(rs2),
                 regName[rs1], regRead(rs1), imm, old);
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;

      break;
    }
    case OP_SH: {
      if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
          !bus->DataRead(paddr, &old, this)) {
        DEBUGMSG("SH %s(%x),%s(%x),%d -> %x\n", regName[rs2], regRead(rs2),
                 regName[rs1], regRead(rs1), imm, old);
        old = mergeHWord(old, regRead(rs2), HWORDPOS(vaddr));
        e = this->bus->DataWrite(paddr, old, this);
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    case OP_SW: {
      if (!mapVirtual(vaddr, &paddr, WRITE) &&
          !this->bus->DataWrite(paddr, regRead(rs2), this)) {
        DEBUGMSG("SW %s(%x),%s(%x),%d\n", regName[rs2], regRead(rs2),
                 regName[rs1], regRead(rs1), imm);
        setNextPC(getPC() + WORDLEN);
      } else
        e = true;
      break;
    }
    default: {
      SignalExc(CPUEXCEPTION, 0);
      e = true;
      break;
    }
    }
    break;
  }
  case OP_AUIPC: {
    DEBUGMSG("\tU-type | AUIPC\n");
    uint8_t rd = RD(instr);
    SWord imm = U_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    regWrite(rd, (SWord)getPC() + imm);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_LUI: {
    uint8_t rd = RD(instr);
    SWord imm = SIGN_EXTENSION(U_IMM(instr), U_IMM_SIZE) << 12;
    DEBUGMSG("\tU-type | LUI %s,%d\n", regName[rd], imm);
    this->regWrite(rd, imm);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_JAL: {
    uint8_t rd = RD(instr);
    Word imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE) & 0xfffffffe;
    DEBUGMSG("\tJ-type | JAL %s,%x\n", regName[rd], getPC() + imm);
    regWrite(rd, getPC() + WORDLEN);
    setNextPC(getPC() + imm);
    break;
  }
  case OP_JALR: {
    // TODO: understand diffs with JAL
    uint8_t rd = RD(instr);
    uint8_t rs1 = RS1(instr);
    Word imm = I_IMM(instr);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    DEBUGMSG("\tJ-type | JALR %s,%s(%x),%x\n", regName[rd], regName[rs1],
             regRead(rs1), (regRead(rs1) + imm) & 0xfffffffe);
    regWrite(rd, getPC() + WORDLEN);
    setNextPC(((regRead(rs1) + imm) & 0xfffffffe));
    break;
  }
  // case OP_FENCE: {
  //   DEBUGMSG("FENCE\n!!! Da implementare\n");
  //   // DEBUGMSG("PR %d ", FENCE_PR(instr));
  //   // DEBUGMSG("PW %d ", FENCE_PW(instr));
  //   // DEBUGMSG("PI %d ", FENCE_PI(instr));
  //   // DEBUGMSG("PO %d ", FENCE_PO(instr));
  //   // DEBUGMSG("SR %d ", FENCE_SR(instr));
  //   // DEBUGMSG("SW %d ", FENCE_SW(instr));
  //   // DEBUGMSG("SI %d ", FENCE_SI(instr));
  //   // DEBUGMSG("SO %d\n", FENCE_SO(instr));
  //   setNextPC(getPC() + WORDLEN);
  //   break;
  // }
  default: {
    ERRORMSG("OpCode not handled %x\n", instr);
    SignalExc(CPUEXCEPTION, 0);
    e = true;
    ERROR("opcode not handled");
    break;
  }
  }

  return e;
}

// This method tests for CP0 availability (as set in STATUS register and in
// MIPS conventions)
bool Processor::cp0Usable() {
  // CP0 is usable only when marked in user mode, and always in kernel mode
  return BitVal(cpreg[STATUS], STATUS_CU0_BIT) ||
         !BitVal(cpreg[STATUS], STATUS_KUc_BIT);
}

// This method scans the TLB looking for a entry that matches ASID/VPN pair;
// scan algorithm follows MIPS specifications, and returns the _highest_
// entry that matches
bool Processor::probeTLB(unsigned int *index, Word asid, Word vpn) {
  bool found = false;

  for (unsigned int i = 0; i < tlbSize; i++) {
    if (tlb[i].VPNMatch(vpn) && (tlb[i].IsG() || tlb[i].ASIDMatch(asid))) {
      found = true;
      *index = i;
    }
  }

  return found;
}

// This method sets delayed load handling variables when needed by
// instruction execution
void Processor::setLoad(LoadTargetType loadCode, unsigned int regNum,
                        SWord regVal) {
  loadPending = loadCode;
  loadReg = regNum;
  loadVal = regVal;
}

// This method returns a sign-extended byte from inside a word following
// MIPS conventions about big and little endianness; it is requested by LB
// instruction execution
SWord Processor::signExtByte(Word val, unsigned int bytep) {
  if (BIGENDIANCPU)
    // byte to be extended is on the other "side" of the word
    bytep = (WORDLEN - 1) - bytep;

  // shifts byte into first position
  val = val >> (BYTELEN * bytep);

  if (BitVal(val, BYTELEN - 1))
    // must sign-extend with 1
    return (val | ~(BYTESIGNMASK));
  else
    return (val & BYTESIGNMASK);
}

// This method returns a zero-extended byte from inside a word following
// MIPS conventions about big and little endianness; requested by LBU
// instruction
Word Processor::zExtByte(Word val, unsigned int bytep) {
  if (BIGENDIANCPU)
    // byte is on the other "side" of the word
    bytep = (WORDLEN - 1) - bytep;

  // shifts byte into first position and zero-extends it
  return ((val >> (BYTELEN * bytep)) & BYTEMASK);
}

// This method returns a word with one byte overwritten, taken from another
// word, and following MIPS conventions about big and little endianness:
// requested by SB instruction
Word Processor::mergeByte(Word dest, Word src, unsigned int bytep) {
  if (BIGENDIANCPU)
    bytep = (WORDLEN - 1) - bytep;

  // shifts least significant byte of src into position and clears around it
  src = (src & BYTEMASK) << (bytep * BYTELEN);

  // clears specified byte and overwrites it with src
  return ((dest & ~(BYTEMASK << (bytep * BYTELEN))) | src);
}

// This method returns the sign-extended halfword taken from a word, following
// MIPS conventions about big and little endianness (LH instruction)
SWord Processor::signExtHWord(Word val, unsigned int hwp) {
  /*
  if (BIGENDIANCPU)
    hwp = 1 - hwp;

  // shifts halfword into first position
  val = val >> (HWORDLEN * hwp);

  return (SignExtImm(val));
  */
  return val;
}

// This method returns the zero-extended halfword taken from a word, following
// MIPS conventions about big and little endianness (LHU instruction)
Word Processor::zExtHWord(Word val, unsigned int hwp) {
  if (BIGENDIANCPU)
    hwp = 1 - hwp;

  // shifts halfword into first position
  val = val >> (HWORDLEN * hwp);

  return (ZEXTIMM(val));
}

// this method returns a word partially overwritten with another, following
// MIPS conventions about big and little endianness (SH instruction)
Word Processor::mergeHWord(Word dest, Word src, unsigned int hwp) {
  if (BIGENDIANCPU)
    hwp = 1 - hwp;

  // shifts least significant halfword of src into position and clears around
  // it
  src = (src & IMMMASK) << (hwp * HWORDLEN);

  // clears specified halfword and overwrites it with src
  return ((dest & ~(IMMMASK << (hwp * HWORDLEN))) | src);
}

// This method copies into dest some bytes from src (starting from bytep
// position in src), beginning the copy from the left or right side of dest.
// It computes the partial overlaps needed the LWL/LWR and SWL/SWR.
// instructions following the MIPS conventions on big and little endianness
Word Processor::merge(Word dest, Word src, unsigned int bytep, bool loadBig,
                      bool startLeft) {
  if (loadBig)
    // LWL/LWR with BIGENDIANCPU == 1 or SWR/SWL with BIGENDIANCPU == 0
    bytep = (WORDLEN - 1) - bytep;
  // else bytep is already correct:
  // LWL/LWR with BIGENDIANCPU == 0 or SWR/SWL with BIGENDIANCPU = 1

  if (startLeft) {
    // starts from left end of dest:
    // shifts src part into position and clear right of it
    src = src << (((WORDLEN - 1) - bytep) * BYTELEN);

    // clear the left part of dest and merges it with src
    dest = (dest & ~(MAXWORDVAL << (((WORDLEN - 1) - bytep) * BYTELEN))) | src;
  } else {
    // starts from right end of dest: shifts src part into position
    // and clears left of it (because src is unsigned)
    src = src >> (bytep * BYTELEN);

    // clears the right part of dest and merges it with src
    dest = (dest & ~(MAXWORDVAL >> (bytep * BYTELEN))) | src;
  }
  return (dest);
}

// This method executes a MIPS register-type instruction, following MIPS
// guidelines; returns instruction result thru res pointer, and branch delay
// slot indication if needed (due to JR/JALR presence). It also returns TRUE
// if an exception occurred, FALSE otherwise
bool Processor::execRegInstr(Word *res, Word instr, bool *isBD) {
  /*
  bool error = false;
  Word paddr;
  bool atomic;

  *isBD = false;

  error = InvalidRegInstr(instr);
  if (!error) {
    // instruction format is correct
    switch (FUNCT(instr)) {
    case SFN_ADD:
      if (SignAdd(res, gpr[RS(instr)], gpr[RT(instr)])) {
        SignalExc(OVEXCEPTION);
        error = true;
      }
      break;

    case SFN_ADDU:
      *res = gpr[RS(instr)] + gpr[RT(instr)];
      break;

    case SFN_AND:
      *res = gpr[RS(instr)] & gpr[RT(instr)];
      break;

    case SFN_BREAK:
      SignalExc(BPEXCEPTION);
      error = true;
      break;

    case SFN_DIV:
      if (gpr[RT(instr)] != 0) {
        gpr[LO] = gpr[RS(instr)] / gpr[RT(instr)];
        gpr[HI] = gpr[RS(instr)] % gpr[RT(instr)];
      } else {
        // divisor is zero
        gpr[LO] = MAXSWORDVAL;
        gpr[HI] = 0;
      }
      break;

    case SFN_DIVU:
      if (gpr[RT(instr)] != 0) {
        gpr[LO] = ((Word)gpr[RS(instr)]) / ((Word)gpr[RT(instr)]);
        gpr[HI] = ((Word)gpr[RS(instr)]) % ((Word)gpr[RT(instr)]);
      } else {
        // divisor is zero
        gpr[LO] = MAXSWORDVAL;
        gpr[HI] = 0;
      }
      break;
    case SFN_JALR:
      // solution "by the book"
      succPC = gpr[RS(instr)];
      *res = currPC + (2 * WORDLEN);
      *isBD = true;
      // alternative: *res = succPC; succPC = gpr[RS(instr)]
      break;

    case SFN_JR:
      succPC = gpr[RS(instr)];
      *isBD = true;
      break;

    case SFN_MFHI:
      *res = gpr[HI];
      break;

    case SFN_MFLO:
      *res = gpr[LO];
      break;

    case SFN_MTHI:
      gpr[HI] = gpr[RS(instr)];
      break;

    case SFN_MTLO:
      gpr[LO] = gpr[RS(instr)];
      break;

    case SFN_MULT:
      SignMult(gpr[RS(instr)], gpr[RT(instr)], &(gpr[HI]), &(gpr[LO]));
      break;

    case SFN_MULTU:
      UnsMult((Word)gpr[RS(instr)], (Word)gpr[RT(instr)], (Word *)&(gpr[HI]),
              (Word *)&(gpr[LO]));
      break;

    case SFN_NOR:
      *res = ~(gpr[RS(instr)] | gpr[RT(instr)]);
      break;

    case SFN_OR:
      *res = gpr[RS(instr)] | gpr[RT(instr)];
      break;

    case SFN_SLL:
      *res = gpr[RT(instr)] << SHAMT(instr);
      break;

    case SFN_SLLV:
      *res = gpr[RT(instr)] << REGSHAMT(gpr[RS(instr)]);
      break;

    case SFN_SLT:
      if (gpr[RS(instr)] < gpr[RT(instr)])
        *res = 1UL;
      else
        *res = 0UL;
      break;

    case SFN_SLTU:
      if (((Word)gpr[RS(instr)]) < ((Word)gpr[RT(instr)]))
        *res = 1UL;
      else
        *res = 0UL;
      break;

    case SFN_SRA:
      *res = (gpr[RT(instr)] >> SHAMT(instr));
      break;

    case SFN_SRAV:
      *res = (gpr[RT(instr)] >> REGSHAMT(gpr[RS(instr)]));
      break;

    case SFN_SRL:
      *res = (((Word)gpr[RT(instr)]) >> SHAMT(instr));
      break;

    case SFN_SRLV:
      *res = (((Word)gpr[RT(instr)]) >> REGSHAMT(gpr[RS(instr)]));
      break;

    case SFN_SUB:
      if (SignSub(res, gpr[RS(instr)], gpr[RT(instr)])) {
        SignalExc(OVEXCEPTION);
        error = true;
      }
      break;

    case SFN_SUBU:
      *res = gpr[RS(instr)] - gpr[RT(instr)];
      break;

    case SFN_SYSCALL:
      SignalExc(SYSEXCEPTION);
      error = true;
      break;

    case SFN_XOR:
      *res = gpr[RS(instr)] ^ gpr[RT(instr)];
      break;

    case SFN_CAS:
      if (mapVirtual(gpr[RS(instr)], &paddr, WRITE) ||
          bus->CompareAndSet(paddr, gpr[RT(instr)], gpr[RD(instr)], &atomic,
                             this))
        error = true;
      else
        *res = atomic;
      break;

    default:
      // unknown instruction
      SignalExc(RIEXCEPTION);
      error = true;
    }
  } else {
    // istruction is ill-formed
    SignalExc(RIEXCEPTION);
  }

  return error;
  */
  return true;
}

// This method executes a MIPS immediate-type instruction, following MIPS
// guidelines; returns instruction result thru res pointer. It also returns
// TRUE if an exception occurred, FALSE otherwise
bool Processor::execImmInstr(Word *res, Word instr) {
  /*
  bool error = false;

  switch (OPCODE(instr)) {
  case ADDI:
    if (SignAdd(res, gpr[RS(instr)], SignExtImm(instr))) {
      SignalExc(OVEXCEPTION);
      error = true;
    }
    break;

  case ADDIU:
    *res = gpr[RS(instr)] + SignExtImm(instr);
    break;

  case ANDI:
    *res = gpr[RS(instr)] & ZEXTIMM(instr);
    break;

  case LUI:
    if (!RS(instr))
      *res = (ZEXTIMM(instr) << HWORDLEN);
    else {
      // instruction is ill-formed
      SignalExc(RIEXCEPTION);
      error = true;
    }
    break;

  case ORI:
    *res = gpr[RS(instr)] | ZEXTIMM(instr);
    break;

  case SLTI:
    if (gpr[RS(instr)] < SignExtImm(instr))
      *res = 1UL;
    else
      *res = 0UL;
    break;

  case SLTIU:
    if (((Word)gpr[RS(instr)]) < ((Word)SignExtImm(instr)))
      *res = 1UL;
    else
      *res = 0UL;
    break;

  case XORI:
    *res = gpr[RS(instr)] ^ ZEXTIMM(instr);
    break;

  default:
    SignalExc(RIEXCEPTION);
    error = true;
    break;
  }
  return (error);
  */
  return true;
}

// This method executes a MIPS branch-type instruction, following MIPS
// guidelines; returns instruction result thru res pointer, and branch delay
// slot indication if needed. It also returns TRUE if an exception occurred,
// FALSE otherwise
bool Processor::execBranchInstr(Word instr, bool *isBD) {
  /*
  bool error = false;

  switch (OPCODE(instr)) {
  case BEQ:
    if (gpr[RS(instr)] == gpr[RT(instr)])
      succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
    break;

  case BGL:
    // uses RT field to choose which branch type is requested
    switch (RT(instr)) {
    case BGEZ:
      if (!SIGNBIT(gpr[RS(instr)]))
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
      break;

    case BGEZAL:
      // solution "by the book"; alternative: gpr[..] = succPC
      gpr[LINKREG] = currPC + (2 * WORDLEN);
      if (!SIGNBIT(gpr[RS(instr)]))
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
      break;

    case BLTZ:
      if (SIGNBIT(gpr[RS(instr)]))
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
      break;

    case BLTZAL:
      gpr[LINKREG] = currPC + (2 * WORDLEN);
      if (SIGNBIT(gpr[RS(instr)]))
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
      break;

    default:
      // unknown instruction
      SignalExc(RIEXCEPTION);
      error = true;
      break;
    }
    break;

  case BGTZ:
    if (!RT(instr)) {
      // instruction is well formed
      if (gpr[RS(instr)] > 0)
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
    } else {
      // istruction is ill-formed
      SignalExc(RIEXCEPTION);
      error = true;
    }
    break;

  case BLEZ:
    if (!RT(instr)) {
      // instruction is well formed
      if (gpr[RS(instr)] <= 0)
        succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
    } else {
      // istruction is ill-formed
      SignalExc(RIEXCEPTION);
      error = true;
    }
    break;

  case BNE:
    if (gpr[RS(instr)] != gpr[RT(instr)])
      succPC = nextPC + (SignExtImm(instr) << WORDSHIFT);
    break;

  case J:
    succPC = JUMPTO(nextPC, instr);
    break;

  case JAL:
    // solution "by the book": alt. gpr[..] = succPC
    gpr[LINKREG] = currPC + (2 * WORDLEN);
    succPC = JUMPTO(nextPC, instr);
    break;

  default:
    SignalExc(RIEXCEPTION);
    error = true;
    break;
  }

  // Next instruction is BD slot, unless an exception occurred
  *isBD = !error;

  return error;
  */
  return true;
}

// This method executes a MIPS load-type instruction, following MIPS
// guidelines. It returns TRUE if an exception occurred, FALSE
// otherwise.
bool Processor::execLoadInstr(Word instr) {
  /*
  Word paddr, vaddr, temp;
  bool error = false;

  switch (OPCODE(instr)) {
  case LB:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);

    // reads the full word from bus and then extracts the byte
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !bus->DataRead(paddr, &temp, this))
      setLoad(LOAD_TARGET_GPREG, RT(instr), signExtByte(temp,
  BYTEPOS(vaddr))); else
      // exception signaled: rt not loadable
      error = true;
    break;

  case LBU:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);

    // reads the full word from bus and then extracts the byte
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !bus->DataRead(paddr, &temp, this))
      setLoad(LOAD_TARGET_GPREG, RT(instr),
              (SWord)zExtByte(temp, BYTEPOS(vaddr)));
    else
      // exception signaled: rt not loadable
      error = true;
    break;

  case LH:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (BitVal(vaddr, 0)) {
      // unaligned halfword
      SignalExc(ADELEXCEPTION);
      error = true;
    } else
      // reads the full word from bus and then extracts the halfword
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !bus->DataRead(paddr, &temp, this))
        setLoad(LOAD_TARGET_GPREG, RT(instr),
                signExtHWord(temp, HWORDPOS(vaddr)));
      else
        // exception signaled: rt not loadable
        error = true;
    break;

  case LHU:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (BitVal(vaddr, 0)) {
      // unaligned halfword
      SignalExc(ADELEXCEPTION);
      error = true;
    } else
      // reads the full word from bus and then extracts the halfword
      if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
          !bus->DataRead(paddr, &temp, this))
        setLoad(LOAD_TARGET_GPREG, RT(instr),
                (SWord)zExtHWord(temp, HWORDPOS(vaddr)));
      else
        // exception signaled: rt not loadable
        error = true;
    break;

  case LW:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (!mapVirtual(vaddr, &paddr, READ) && !bus->DataRead(paddr, &temp,
  this)) setLoad(LOAD_TARGET_GPREG, RT(instr), (SWord)temp); else
      // exception signaled: rt not loadable
      error = true;
    break;

  case LWL:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);

    // reads the full word from bus and then extracts the desired part
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !bus->DataRead(paddr, &temp, this)) {
      temp =
          merge((Word)gpr[RT(instr)], temp, BYTEPOS(vaddr), BIGENDIANCPU,
  true); setLoad(LOAD_TARGET_GPREG, RT(instr), temp); } else
      // exception signaled: rt not loadable
      error = true;
    break;

  case LWR:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);

    // reads the full word from bus and then extracts the desired part
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !bus->DataRead(paddr, &temp, this)) {
      temp = merge((Word)gpr[RT(instr)], temp, BYTEPOS(vaddr), BIGENDIANCPU,
                   false);
      setLoad(LOAD_TARGET_GPREG, RT(instr), temp);
    } else
      // exception signaled: rt not loadable
      error = true;
    break;

  default:
    SignalExc(RIEXCEPTION);
    error = true;
    break;
  }
  return (error);
  */
  return true;
}

// This method executes a MIPS store-type instruction, following MIPS
// guidelines; It returns TRUE if an exception occurred, FALSE
// otherwise.
bool Processor::execStoreInstr(Word instr) {
  /*
  Word paddr, vaddr, temp;
  bool error = false;

  switch (OPCODE(instr)) {
  case SB:
    // here things are a little dirty: instead of writing
    // the byte directly into memory, it reads the full word,
    // modifies the byte as needed, and writes the word back.
    // This works because there could be read-only memory but
    // not write-only...
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
        !bus->DataRead(paddr, &temp, this)) {
      temp = mergeByte(temp, (Word)gpr[RT(instr)], BYTEPOS(vaddr));
      if (bus->DataWrite(paddr, temp, this))
        // bus exception signaled
        error = true;
    } else
      // address or bus exception signaled
      error = true;
    break;

  case SH:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (BitVal(vaddr, 0)) {
      // unaligned halfword
      SignalExc(ADESEXCEPTION);
      error = true;
    } else
      // the same "dirty" thing here...
      if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
          !bus->DataRead(paddr, &temp, this)) {
        temp = mergeHWord(temp, (Word)gpr[RT(instr)], HWORDPOS(vaddr));
        if (bus->DataWrite(paddr, temp, this))
          // bus exception signaled
          error = true;
      } else
        // address or bus exception signaled
        error = true;
    break;

  case SW:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (mapVirtual(vaddr, &paddr, WRITE) ||
        bus->DataWrite(paddr, (Word)gpr[RT(instr)], this))
      // address or bus exception signaled
      error = true;
    break;

  case SWL:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
        !bus->DataRead(paddr, &temp, this)) {
      temp = merge(temp, (Word)gpr[RT(instr)], BYTEPOS(vaddr),
  !(BIGENDIANCPU), false);

      if (bus->DataWrite(paddr, temp, this))
        // bus exception
        error = true;
    } else
      // address or bus exception signaled
      error = true;
    break;

  case SWR:
    vaddr = gpr[RS(instr)] + SignExtImm(instr);
    if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
        !bus->DataRead(paddr, &temp, this)) {
      temp = merge(temp, (Word)gpr[RT(instr)], BYTEPOS(vaddr),
  !(BIGENDIANCPU), true); if (bus->DataWrite(paddr, temp, this))
        // bus exception signaled
        error = true;
    } else
      // addresss or bus exception signaled
      error = true;
    break;

  default:
    SignalExc(RIEXCEPTION);
    error = true;
    break;
  }
  return (error);
  */
  return true;
}
