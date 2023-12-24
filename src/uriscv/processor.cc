/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2020 Mattia Biondi
 * Copyright (C) 2023 Gianmaria Rovelli
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "uriscv/arch.h"
#include "uriscv/bios.h"
#include "uriscv/const.h"
#include "uriscv/cpu.h"
#include "uriscv/csr.h"
#include "uriscv/disassemble.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/processor_defs.h"
#include "uriscv/systembus.h"
#include "uriscv/types.h"
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

  // first instruction should not be skipped
  skipCycle = false;

  // clear general purpose registers
  for (i = 0; i < CPUREGNUM; i++)
    gpr[i] = 0;
  gpr[REG_SP] = sp;

  // no previous instruction is available
  prevPC = MAXWORDVAL;
  prevPhysPC = MAXWORDVAL;
  prevInstr = NOP;

  csrWrite(CSR_ENTRYHI, 0);
  csrWrite(CSR_ENTRYLO, 0);
  csrWrite(CSR_INDEX, 0);
  csrWrite(CSR_BADVADDR, 0);
  csrWrite(CSR_RANDOM, ((tlbSize - 1UL) << RNDIDXOFFS) - RANDOMSTEP);
  csrWrite(MSTATUS, MSTATUS_MPP_M);
  csrWrite(MIE, 0);
  csrWrite(MCAUSE, 0);
  csrWrite(TIME, 0);
  mode = 0x3;

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
  // read MIE bit
  if (csrRead(MIE) & MIE_MTIE_MASK) {
    if (csrRead(TIME) == 0) {
      AssertIRQ(IL_CPUTIMER);
    }
    csrWrite(TIME, csrRead(TIME) - 1);
  } else {
    DeassertIRQ(IL_CPUTIMER);
  }

  // In low-power state, only the per-cpu timer keeps running
  if (isIdle()) {
    return;
  }

  // Instruction decode & exec
  if (!skipCycle && execInstr(currInstr))
    handleExc();

  // Check if we entered sleep mode as a result of the last
  // instruction; if so, we effectively stall the pipeline.
  if (isIdle()) {
    return;
  }

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

  if (skipCycle)
    skipCycle = false;

  // processor cycle fetch part
  if (mapVirtual(currPC, &currPhysPC, EXEC)) {
    // TLB or Address exception caused: current instruction is nullified
    // currInstr = NOP;
    handleExc();
    skipCycle = true;
  } else if (bus->InstrRead(currPhysPC, &currInstr, this)) {
    // IBE exception caused: current instruction is nullified
    // currInstr = NOP;
    handleExc();
    skipCycle = true;
  }
}
uint32_t Processor::IdleCycles() {
  if (isHalted())
    return (uint32_t)-1;
  else if (isIdle())
    return (csrRead(MIE) & MIE_MTIE_MASK) ? csrRead(TIME) : (uint32_t)-1;
  else
    return 0;
}

void Processor::Skip(uint32_t cycles) {
  assert(isIdle() && cycles <= IdleCycles());

  if (csrRead(MIE) & MIE_MTIE_MASK)
    csrWrite(TIME, csrRead(TIME) - cycles);
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
  // used only for EXC_II handling
  copENum = cpuNum;
}

void Processor::AssertIRQ(unsigned int il) {

  csrWrite(MIP, csrRead(MIP) | CAUSE_IP(il));

  // If in standby mode, go back to being a power hog.
  if (isIdle())
    setStatus(PS_RUNNING);
}

void Processor::DeassertIRQ(unsigned int il) {
  csrWrite(MIP, csrRead(MIP) & ~CAUSE_IP(il));
}

// This method allows to get critical information on Processor current
// internal status. Parameters are self-explanatory: they are extracted from
// proper places inside Processor itself
void Processor::getCurrStatus(Word *asid, Word *pc, Word *instr, bool *isLD,
                              bool *isBD) {
  *asid = (ASID(csrRead(CSR_ENTRYHI))) >> ASIDOFFS;
  *pc = currPC;
  *instr = currInstr;
  *isLD = (loadPending != LOAD_TARGET_NONE);
  *isBD = isBranchD;
}

Word Processor::getASID() { return ASID(csrRead(CSR_ENTRYHI)) >> ASIDOFFS; }

bool Processor::InUserMode() {
  // return (csrRead(MSTATUS) & MSTATUS_MPP_MASK) == MSTATUS_MPP_U;
  return mode == 0x0;
}

bool Processor::InKernelMode() {
  // return (csrRead(MSTATUS) & MSTATUS_MPP_MASK) == MSTATUS_MPP_M;
  return mode == 0x3;
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
Word Processor::getCP0Reg(unsigned int num) { return (0); }

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
  assert(reg >= 0 && reg < CPUREGNUM);
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
  assert(reg >= 0 && reg < kNumCSRRegisters);
  return csr[reg].value;
}
void Processor::csrWrite(Word reg, Word value) {
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

// This method allows to modify the current value of currPC to force a
// immediate jump: gdb purposes
void Processor::setPC(Word npc) { currPC = npc; }

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
  csrWrite(CSR_RANDOM, (csrRead(CSR_RANDOM) - RANDOMSTEP) &
                           (((tlbSize - 1UL) << RNDIDXOFFS)));
  if (csrRead(CSR_RANDOM) < RANDOMBASE)
    csrWrite(csrRead(CSR_RANDOM), ((tlbSize - 1UL) << RNDIDXOFFS));
}

// This method pushes the KU/IE bit stacks in CP0 STATUS register to start
// exception handling
void Processor::pushKUIEStack() {

  if (BitVal(csrRead(MSTATUS), MSTATUS_MIE_BIT)) {
    csrWrite(MSTATUS, SetBit(csrRead(MSTATUS), MSTATUS_MPIE_BIT));
  } else
    csrWrite(MSTATUS, ResetBit(csrRead(MSTATUS), MSTATUS_MPIE_BIT));

  csrWrite(MSTATUS, ResetBit(csrRead(MSTATUS), MSTATUS_MIE_BIT));

  // set machine mode
  csrWrite(MSTATUS,
           (csrRead(MSTATUS) & ~MSTATUS_MPP_MASK) | (mode << MSTATUS_MPP_BIT));
  mode = 0x3;
}

// This method pops the KU/IE bit stacks in CP0 STATUS register to end
// exception handling. It is invoked on RFE instruction execution
void Processor::popKUIEStack() {
  if (BitVal(csrRead(MSTATUS), MSTATUS_MPIE_BIT))
    csrWrite(MSTATUS, SetBit(csrRead(MSTATUS), MSTATUS_MIE_BIT));
  else
    csrWrite(MSTATUS, ResetBit(csrRead(MSTATUS), MSTATUS_MIE_BIT));

  csrWrite(MSTATUS, ResetBit(csrRead(MSTATUS), MSTATUS_MPIE_BIT));

  if ((csrRead(MSTATUS) & MSTATUS_MPP_MASK) >> MSTATUS_MPP_BIT)
    csrWrite(MSTATUS, ResetBit(csrRead(MSTATUS), MSTATUS_MPRV_BIT));

  // xPP is set to the least-privileged supported mode
  mode = (csrRead(MSTATUS) & MSTATUS_MPP_MASK) >> MSTATUS_MPP_BIT;
  csrWrite(MSTATUS, (csrRead(MSTATUS) & ~MSTATUS_MPP_MASK) | MSTATUS_MPP_U);
}

// This method test for pending interrupts, checking global abilitation (IEc
// bit in STATUS register) and comparing interrupt mask with interrupts
// pending on bus; returns TRUE if an INT exception must be raised, FALSE
// otherwise, and sets CP0 registers if needed
bool Processor::checkForInt() {
  // check if interrupts are enabled and pending
  if (csrRead(MSTATUS) & MSTATUS_MIE_MASK && (csrRead(MIE) & csrRead(MIP))) {
    uint l = 0;
    uint mip = csrRead(MIP);
    while (mip > 1) {
      mip >>= 1;
      l++;
    }
    SignalExc((1 << 31) | l);
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
  // if (!(cpreg[CAUSE] & CAUSE_IP_MASK))
  if (!(csrRead(MIP)))
    setStatus(PS_IDLE);
}

// This method sets the appropriate CP0 registers at exception
// raising, following the MIPS conventions; it also set the PC value
// to point the appropriate exception handler vector.
void Processor::handleExc() {

  // Set PC to the right handler
  Word excVector = csrRead(MTVEC) >> 2;

  pushKUIEStack();

  unsigned int mcause = excCause;

  csrWrite(MCAUSE, mcause);
  csrWrite(MEPC, currPC);

  if (CAUSE_IS_INT(mcause)) {
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
void Processor::completeLoad() {}

// This method maps the virtual addresses to physical ones following the
// complex mapping algorithm and TLB used by MIPS (see external doc).
// It returns TRUE if conversion was not possible (this implies an exception
// have been raised) and FALSE if conversion has taken place: physical value
// for address conversion is returned thru paddr pointer.
// AccType details memory access type (READ/WRITE/EXECUTE)
bool Processor::mapVirtual(Word vaddr, Word *paddr, Word accType) {
  // SignalProcVAccess() is always done so it is possible
  // to track accesses which produce exceptions
  machine->HandleVMAccess(ENTRYHI_GET_ASID(csrRead(CSR_ENTRYHI)), vaddr,
                          accType, this);

  // address validity and bounds check
  if (BADADDR(vaddr) ||
      (InUserMode() && (INBOUNDS(vaddr, KSEG0BASE, KUSEGBASE)))) {
    // bad offset or kernel segment access from user mode
    *paddr = MAXWORDVAL;

    // the bad virtual address is put into BADVADDR reg
    csrWrite(CSR_BADVADDR, vaddr);

    if (accType == WRITE)
      SignalExc(EXC_SAF);
    else {
      SignalExc(EXC_LAF);
    }

    return true;
  } else if (INBOUNDS(vaddr, KSEG0BASE, tlbFloorAddress)) {
    if (vaddr >= KUSEGBASE) {
      ERRORMSG("ADDRESS IN KSEG0 - TLBFLOOR %x\n", vaddr);
    }
    // no bad offset; if vaddr < KUSEGBASE the processor is surely
    // in kernelMode
    // valid access to KSEG0 area
    *paddr = vaddr;
    return false;
  }

  // The access is in user mode to user space, or in kernel mode
  // to KSEG0 or KUSEG spaces.

  unsigned int index;
  if (probeTLB(&index, csrRead(CSR_ENTRYHI), vaddr)) {
    if (tlb[index].IsV()) {
      if (accType != WRITE || tlb[index].IsD()) {
        // All OK
        *paddr = PHADDR(vaddr, tlb[index].getLO());
        return false;
      } else {
        // write operation on frame with D bit set to 0
        *paddr = MAXWORDVAL;
        setTLBRegs(vaddr);
        SignalExc(EXC_MOD);
        return true;
      }
    } else {
      // invalid access to frame with V bit set to 0
      *paddr = MAXWORDVAL;
      setTLBRegs(vaddr);
      if (accType == WRITE)
        SignalExc(EXC_TLBS);
      else
        SignalExc(EXC_TLBL);

      return true;
    }
  } else {
    // bad or missing VPN match: Refill event required
    *paddr = MAXWORDVAL;
    setTLBRegs(vaddr);

    if (accType == WRITE)
      SignalExc(EXC_UTLBS);
    else
      SignalExc(EXC_UTLBL);

    return true;
  }
}

// This method sets the CP0 special registers on exceptions forced by TLB
// handling (see mapVirtual() for invocation/specific cases).
void Processor::setTLBRegs(Word vaddr) {
  // Note that ENTRYLO is left undefined!
  csrWrite(CSR_BADVADDR, vaddr);
  csrWrite(CSR_ENTRYHI, VPN(vaddr) | ASID(csrRead(CSR_ENTRYHI)));
}

bool Processor::execInstrL(Word instr) {
  DISASSMSG("\tI-type | ");
  bool e = false;
  int8_t rd = RD(instr);
  int8_t rs1 = RS1(instr);
  int16_t imm = I_IMM(instr);
  imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
  uint8_t FUNC3 = FUNC3(instr);
  Word read = 0;
  Word vaddr = regRead(rs1) + imm, paddr = 0;

  switch (FUNC3) {
  case OP_LB: {
    DISASSMSG("LB\n");
    // just 8 bits
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !this->bus->DataRead(paddr, &read, this)) {
      regWrite(rd, signExtByte(read, BYTEPOS(vaddr)));
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  case OP_LH: {
    DISASSMSG("LBH\n");
    // just 16 bits
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !this->bus->DataRead(paddr, &read, this)) {
      regWrite(rd, signExtHWord(read, HWORDPOS(vaddr)));
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  case OP_LW: {
    DISASSMSG("LW %s,%s(%x),%d\n", regName[rd], regName[rs1], regRead(rs1),
              (Word)imm);
    if (!mapVirtual(vaddr, &paddr, READ) &&
        !this->bus->DataRead(paddr, &read, this)) {
      regWrite(rd, read);
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  case OP_LBU: {
    // just 8 bits
    if (!mapVirtual(ALIGN(vaddr), &paddr, READ) &&
        !this->bus->DataRead(paddr, &read, this)) {
      DISASSMSG("LBU %s,%s(%x),%d -> %x\n", regName[rd], regName[rs1],
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
        !this->bus->DataRead(paddr, &read, this)) {
      DISASSMSG("LHU\n");
      regWrite(rd, zExtHWord(read, HWORDPOS(vaddr)));
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    break;
  }
  }
  return e;
}

bool Processor::execInstrR(Word instr) {
  DISASSMSG("\tR-type | ");
  bool e = false;
  int8_t rs1 = RS1(instr);
  int8_t rs2 = RS2(instr);
  uint8_t rd = RD(instr);
  uint8_t FUNC7 = FUNC7(instr);
  uint8_t FUNC3 = FUNC3(instr);
  switch (FUNC3) {
  /* 0x0 */
  case OP_ADD_FUNC3: {
    switch (FUNC7) {
    case OP_ADD_FUNC7: {
      DISASSMSG("ADD %s,%s(%x),%s(%x) -> %x\n", regName[rd], regName[rs1],
                regRead(rs1), regName[rs2], regRead(rs2),
                regRead(rs1) + regRead(rs2));
      regWrite(rd, regRead(rs1) + regRead(rs2));
      break;
    }
    case OP_SUB_FUNC7: {
      DISASSMSG("SUB %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) - regRead(rs2));
      break;
    }
    case OP_MUL_FUNC7: {
      DISASSMSG("MUL %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) * regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("ADD not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x1 */
  case OP_MULH_FUNC3 || OP_SLL_FUNC3: {
    switch (FUNC7) {
    case OP_MULH_FUNC7: {
      SWord high = 0, low = 0;
      SignMult(regRead(rs1), regRead(rs2), &high, &low);
      DISASSMSG("MULH %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                regName[rs2], high);
      regWrite(rd, high);
      break;
    }
    case OP_SLL_FUNC7: {
      DISASSMSG("SLL %s(%x),%s(%x),%d\n", regName[rd], regRead(rd),
                regName[rs1], regRead(rs1), regRead(rs2));
      regWrite(rd, regRead(rs1) << regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x2 */
  case OP_MULHSU_FUNC3 | OP_SLT_FUNC3: {
    switch (FUNC7) {
    case OP_MULHSU_FUNC7: {
      SWord high = 0, low = 0;
      UnsSignMult((SWord)regRead(rs1), (SWord)regRead(rs2), &high, &low);
      DISASSMSG("MULHSU %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                regName[rs2], high);
      regWrite(rd, high);
      break;
    }
    case OP_SLT_FUNC7: {
      DISASSMSG("SLT %s(%x),%s(%x),%d\n", regName[rd], regRead(rd),
                regName[rs1], regRead(rs1), regRead(rs2));
      regWrite(rd, SWord(regRead(rs1)) < SWord(regRead(rs2)) ? 1 : 0);
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x3 */
  case OP_MULHU_FUNC3 | OP_SLTU_FUNC3: {
    switch (FUNC7) {
    case OP_MULHU_FUNC7: {
      Word high = 0, low = 0;
      UnsMult(regRead(rs1), regRead(rs2), &high, &low);
      DISASSMSG("MULHU %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                regName[rs2], high);
      regWrite(rd, high);
      break;
    }
    case OP_SLTU_FUNC7: {
      DISASSMSG("SLTU %s(%x),%s(%x),%d\n", regName[rd], regRead(rd),
                regName[rs1], regRead(rs1), regRead(rs2));
      regWrite(rd, Word(regRead(rs1)) < Word(regRead(rs2)) ? 1 : 0);
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x4 */
  case OP_DIV_FUNC3 | OP_XOR_FUNC3: {
    switch (FUNC7) {
    case OP_DIV_FUNC7: {
      if (regRead(rs2) != 0) {
        Word r = (Word)regRead(rs1) / (Word)regRead(rs2);
        DISASSMSG("DIV %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                  regName[rs2], r);
        regWrite(rd, r);
      } else {
        ERRORMSG("Division by zero detected");
      }
      break;
    }
    case OP_XOR_FUNC7: {
      DISASSMSG("XOR %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) ^ regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x5 */
  case OP_DIVU_FUNC3 | OP_SRL_FUNC3 | OP_SRA_FUNC3: {
    switch (FUNC7) {
    case OP_DIVU_FUNC7: {
      if (regRead(rs2) != 0) {
        SWord r = (SWord)regRead(rs1) / (SWord)regRead(rs2);
        DISASSMSG("DIVU %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                  regName[rs2], r);
        regWrite(rd, r);
      }
      break;
    }
    case OP_SRA_FUNC7: {
      DISASSMSG("SRA %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      uint8_t msb = rs1 & 0x80000000;
      regWrite(rd, regRead(rs1) >> regRead(rs2) | msb);
      regWrite(rd, regRead(rs1) ^ regRead(rs2));
      break;
    }
    case OP_SRL_FUNC7: {
      DISASSMSG("SRL %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) >> regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x6 */
  case OP_REM_FUNC3 | OP_OR_FUNC3: {
    switch (FUNC7) {
    case OP_REM_FUNC7: {
      if (regRead(rs2) != 0) {
        SWord r = (SWord)regRead(rs1) % (SWord)regRead(rs2);
        DISASSMSG("REM %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                  regName[rs2], r);
        regWrite(rd, r);
      }
      break;
    }
    case OP_OR_FUNC7: {
      DISASSMSG("OR %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) | regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
    /* 0x7 */
  case OP_REMU_FUNC3 | OP_AND_FUNC3: {
    switch (FUNC7) {
    case OP_REMU_FUNC7: {
      if (regRead(rs2) != 0) {
        Word r = (Word)regRead(rs1) % (Word)regRead(rs2);
        DISASSMSG("REMU %s,%s,%s -> %x\n", regName[rd], regName[rs1],
                  regName[rs2], r);
        regWrite(rd, r);
      }
      break;
    }
    case OP_AND_FUNC7: {
      DISASSMSG("AND %s,%s,%s\n", regName[rd], regName[rs1], regName[rs2]);
      regWrite(rd, regRead(rs1) & regRead(rs2));
      break;
    }
    default: {
      ERRORMSG("R-type not recognized (%x)\n", FUNC7);
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    }
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    break;
  }
  }
  setNextPC(getPC() + WORDLEN);
  return e;
}

bool Processor::execInstrI(Word instr) {
  DISASSMSG("\tI-type | ");
  bool e = false;
  int8_t rs1 = RS1(instr);
  SWord imm = I_IMM(instr);
  uint8_t rd = RD(instr);
  uint8_t FUNC3 = FUNC3(instr);

  switch (FUNC3) {
  case OP_ADDI: {
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    DISASSMSG("ADDI %s,%s(%x),%d\n", regName[rd], regName[rs1], regRead(rs1),
              imm);
    regWrite(rd, (Word)(regRead(rs1) + imm));
    break;
  }
  case OP_SLLI: {
    DISASSMSG("SLLI %s(%x),%s(%x),%d\n", regName[rd], regRead(rd), regName[rs1],
              regRead(rs1), imm);
    regWrite(rd, regRead(rs1) << imm);
    break;
  }
  case OP_SLTI: {
    DISASSMSG("SLTI\n");
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    regWrite(rd, SWord(regRead(rs1)) < SWord(imm) ? 1 : 0);
    break;
  }
  case OP_SLTIU: {
    DISASSMSG("SLTIU\n");
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    regWrite(rd, regRead(rs1) < Word(imm) ? 1 : 0);
    break;
  }
  case OP_XORI: {
    DISASSMSG("XORI\n");
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    regWrite(rd, SWord(regRead(rs1)) ^ SWord(imm));
    break;
  }
  case OP_SR: {
    uint8_t FUNC7 = FUNC7(instr);
    switch (FUNC7) {
    case OP_SRLI_FUNC7: {
      DISASSMSG("SRLI %s,%s(%x),%x\n", regName[rd], regName[rs1], regRead(rs1),
                imm);
      regWrite(rd, regRead(rs1) >> imm);
      break;
    }
    case OP_SRAI_FUNC7: {
      DISASSMSG("SRAI %s,%s(%x),%x\n", regName[rd], regName[rs1], regRead(rs1),
                imm);
      uint8_t msb = rs1 & 0x80000000;
      regWrite(rd, regRead(rs1) >> imm | msb);
      break;
    }
    default:
      SignalExc(EXC_II, 0);
      e = true;
      break;
    }
    break;
  }
  case OP_ORI: {
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    DISASSMSG("ORI %s,%s(%x),%x\n", regName[rd], regName[rs1], regRead(rs1),
              imm);
    regWrite(rd, SWord(regRead(rs1)) | SWord(imm));
    break;
  }
  case OP_ANDI: {
    DISASSMSG("ANDI %s,%s(%x),%x\n", regName[rd], regName[rs1], regRead(rs1),
              imm);
    imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
    regWrite(rd, SWord(regRead(rs1)) & SWord(imm));
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    break;
  }
  }
  setNextPC(getPC() + WORDLEN);
  return e;
}

bool Processor::execInstrI2(Word instr) {
  DISASSMSG("\tI2-type | ");
  Word e = NOEXCEPTION;
  uint16_t imm = I_IMM(instr);
  int8_t rs1 = RS1(instr);
  uint8_t rd = RD(instr);
  uint8_t FUNC3 = FUNC3(instr);

  switch (FUNC3) {
  case OP_ECALL_EBREAK: {
    switch (imm) {
    case ECALL_IMM: {
      // a7 - syscall number
      // a0 - result of syscall
      //
      DISASSMSG("ECALL %d\n", regRead(REG_A0));
      setNextPC(getPC() + WORDLEN);
      // only M or U modes are possible
      if (mode == 0x3)
        SignalExc(EXC_ECM);
      else
        SignalExc(EXC_ECU);
      e = true;
      break;
    }
    case EBREAK_IMM: {
      int mode = regRead(REG_A0);
      DISASSMSG("EBREAK\n");
      if (mode == BIOS_SRV_PANIC)
        exit(0);
      if (mode <= BIOS_SRV_HALT) {
        SignalExc(EXC_BP);
        e = true;
      } else {
        unsigned int i;
        switch (mode) {
        case BIOS_SRV_TLBP:
          // solution "by the book"
          DISASSMSG(" TLBP\n");
          csrWrite(CSR_INDEX, SIGNMASK);
          if (probeTLB(&i, csrRead(CSR_ENTRYHI), csrRead(CSR_ENTRYHI)))
            csrWrite(CSR_INDEX, (i << RNDIDXOFFS));
          break;

        case BIOS_SRV_TLBR:
          DISASSMSG(" TLBR\n");
          csrWrite(CSR_ENTRYHI, tlb[RNDIDX(csrRead(CSR_INDEX))].getHI());
          csrWrite(CSR_ENTRYLO, tlb[RNDIDX(csrRead(CSR_INDEX))].getLO());
          break;

        case BIOS_SRV_TLBWI:
          DISASSMSG(" TLBWI\n");
          tlb[RNDIDX(csrRead(CSR_INDEX))].setHI(csrRead(CSR_ENTRYHI));
          tlb[RNDIDX(csrRead(CSR_INDEX))].setLO(csrRead(CSR_ENTRYLO));
          SignalTLBChanged(RNDIDX(csrRead(CSR_INDEX)));
          break;

        case BIOS_SRV_TLBWR:
          DISASSMSG(" TLBWR\n");
          tlb[RNDIDX(csrRead(CSR_RANDOM))].setHI(csrRead(CSR_ENTRYHI));
          tlb[RNDIDX(csrRead(CSR_RANDOM))].setLO(csrRead(CSR_ENTRYLO));
          DISASSMSG("\n\nENTRYHI %x\n", csrRead(CSR_ENTRYHI));
          DISASSMSG("ENTRYLO %x\n\n", csrRead(CSR_ENTRYLO));
          SignalTLBChanged(RNDIDX(csrRead(CSR_INDEX)));
          break;

        case BIOS_SRV_TLBCLR:
          zapTLB();
          break;

        default:
          ERRORMSG(" NOT FOUND\n");
          // invalid instruction
          SignalExc(EXC_II, 0);
          e = true;
        }
        setNextPC(getPC() + WORDLEN);
      }
      break;
    }
    case MRET_IMM: {
      /*
       An xRET instruction can be executed in privilege mode x or higher,
       where executing a lower-privilege xRET instruction will pop the
       relevant lower-privilege interrupt enable and privilege mode stack.
       In addition to manipulating the privilege stack as described in
       Section 3.1.6.1, xRET sets the pc to the value stored in the xepc
       register.
      */
      DISASSMSG("MRET %x\n", csrRead(MEPC));
      popKUIEStack();
      setNextPC(csrRead(MEPC));
      break;
    }
    case EWFI_IMM: {
      DISASSMSG("EWFI\n");
      setNextPC(getPC() + WORDLEN);
      suspend();
      break;
    }
    default: {
      SignalExc(EXC_II, 0);
      e = true;
      ERROR("not found");
      break;
    }
    }
    break;
  }
  case OP_CSRRW: {
    DISASSMSG("CSRRW %s(%x),%s(%x),%x\n", regName[rd], regRead(rd),
              regName[rs1], regRead(rs1), imm);
    if (rd != REG_ZERO)
      regWrite(rd, csrRead(imm));
    if (rs1 != REG_ZERO) {
      csrWrite(imm, regRead(rs1));
      if (imm == TIME)
        DeassertIRQ(IL_CPUTIMER);
    }
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_CSRRS: {
    std::cout << "CSRRS\n";
    regWrite(rd, csrRead(imm));
    csrWrite(imm, csrRead(imm) | regRead(rs1));
    if (imm == TIME)
      DeassertIRQ(IL_CPUTIMER);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_CSRRC: {
    std::cout << "CSRRC\n";
    regWrite(rd, csrRead(imm));
    if (rs1 != 0x0) {
      csrWrite(imm, csrRead(imm) & ~regRead(rs1));
      if (imm == TIME)
        DeassertIRQ(IL_CPUTIMER);
    }
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_CSRRWI: {
    std::cout << "CSRRWI\n";
    regWrite(rd, csrRead(imm));
    csrWrite(imm, csrRead(imm) & ~rs1);
    if (imm == TIME)
      DeassertIRQ(IL_CPUTIMER);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_CSRRSI: {
    std::cout << "CSRRSI\n";
    regWrite(rd, csrRead(imm));
    csrWrite(imm, csrRead(imm) | rs1);
    if (imm == TIME)
      DeassertIRQ(IL_CPUTIMER);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_CSRRCI: {
    std::cout << "CSRRCI\n";
    regWrite(rd, csrRead(imm));
    csrWrite(imm, csrRead(imm) & ~rs1);
    if (imm == TIME)
      DeassertIRQ(IL_CPUTIMER);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    ERROR("not found");
    break;
  }
  }
  return e;
}

bool Processor::execInstrB(Word instr) {
  DISASSMSG("\tB-type | ");
  Word e = NOEXCEPTION;
  int8_t rs1 = RS1(instr);
  int8_t rs2 = RS2(instr);
  int16_t imm = B_IMM(instr);
  imm = SIGN_EXTENSION(imm, I_IMM_SIZE);
  uint8_t FUNC3 = FUNC3(instr);

  switch (FUNC3) {
  case OP_BEQ: {
    DISASSMSG("BEQ %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm);
    if ((SWord)regRead(rs1) == (SWord)regRead(rs2))
      setNextPC((SWord)getPC() + ((SWord)imm));
    else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_BNE: {
    DISASSMSG("BNE %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm);
    if ((SWord)regRead(rs1) != (SWord)regRead(rs2))
      setNextPC((SWord)getPC() + ((SWord)imm));
    else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_BLT: {
    DISASSMSG("BLT %s(%x),%s(%x),%d (%d)\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm, regRead(rs1) < regRead(rs2));
    if ((SWord)regRead(rs1) < (SWord)regRead(rs2))
      setNextPC((SWord)getPC() + ((SWord)imm));
    else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_BGE: {
    DISASSMSG("BGE %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm);
    if ((SWord)regRead(rs1) >= (SWord)regRead(rs2)) {
      setNextPC((SWord)getPC() + ((SWord)imm));
    } else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_BLTU: {
    DISASSMSG("BLTU %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm);
    if (regRead(rs1) < regRead(rs2))
      setNextPC(getPC() + (imm));
    else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_BGEU: {
    DISASSMSG("BGEU %s(%x),%s(%x),%d\n", regName[rs1], regRead(rs1),
              regName[rs2], regRead(rs2), imm);
    if (regRead(rs1) >= regRead(rs2))
      setNextPC(getPC() + (imm));
    else
      setNextPC(getPC() + WORDLEN);
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    break;
  }
  }
  return e;
}

bool Processor::execInstrS(Word instr) {
  DISASSMSG("\tS-type | ");
  Word e = NOEXCEPTION;
  int8_t rs1 = RS1(instr);
  int8_t rs2 = RS2(instr);
  int16_t imm = S_IMM(instr);
  imm = SIGN_EXTENSION(imm, S_IMM_SIZE);
  uint8_t FUNC3 = FUNC3(instr);
  Word vaddr = (SWord)regRead(rs1) + (SWord)imm;
  Word paddr = 0;
  Word old = 0;

  switch (FUNC3) {
  case OP_SB: {
    DISASSMSG("SB %s(%x),%s(%x),%d\n", regName[rs2], regRead(rs2), regName[rs1],
              regRead(rs1), imm);
    if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
        !bus->DataRead(paddr, &old, this)) {
      old = mergeByte(old, regRead(rs2), BYTEPOS(vaddr));
      e = this->bus->DataWrite(paddr, old, this);
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;

    break;
  }
  case OP_SH: {
    DISASSMSG("SH %s(%x),%s(%x),%d -> %x\n", regName[rs2], regRead(rs2),
              regName[rs1], regRead(rs1), imm, old);
    if (!mapVirtual(ALIGN(vaddr), &paddr, WRITE) &&
        !bus->DataRead(paddr, &old, this)) {
      old = mergeHWord(old, regRead(rs2), HWORDPOS(vaddr));
      e = this->bus->DataWrite(paddr, old, this);
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  case OP_SW: {
    DISASSMSG("SW $(%s(%x)+%d)<-%s(%x)\n", regName[rs1], regRead(rs1), imm,
              regName[rs2], regRead(rs2));
    if (!mapVirtual(vaddr, &paddr, WRITE) &&
        !this->bus->DataWrite(paddr, regRead(rs2), this)) {
      setNextPC(getPC() + WORDLEN);
    } else
      e = true;
    break;
  }
  default: {
    SignalExc(EXC_II, 0);
    e = true;
    break;
  }
  }
  return e;
}

// This method make Processor execute a single MIPS instruction, emulating
// pipeline constraints and load delay slots (see external doc).
bool Processor::execInstr(Word instr) {
  Word e = NOEXCEPTION;
  uint8_t opcode = OPCODE(instr);
  // const Symbol *sym =
  //     machine->getStab()->Probe(config->getSymbolTableASID(), getPC(), true);
  // if (sym != NULL && sym->getName() != prevFunc) {
  //   DISASSMSG("<FUN %s\n", prevFunc.c_str());
  //   prevFunc = sym->getName();
  //   DISASSMSG("\n>FUN %s\n", sym->getName());
  // }
  // DISASSMSG("[%08x] (%08x) ", getPC(), instr);

  switch (opcode) {
  case OP_L: {
    e = execInstrL(instr);
    break;
  }
  case R_TYPE: {
    e = execInstrR(instr);
    break;
  }
  case I_TYPE: {
    e = execInstrI(instr);
    break;
  }
  case I2_TYPE: {
    e = execInstrI2(instr);
    break;
  }
  case B_TYPE: {
    e = execInstrB(instr);
    break;
  }
  case S_TYPE: {
    e = execInstrS(instr);
    break;
  }
  case OP_AUIPC: {
    DISASSMSG("\tU-type | AUIPC\n");
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
    DISASSMSG("\tU-type | LUI %s,%x\n", regName[rd], imm);
    this->regWrite(rd, imm);
    setNextPC(getPC() + WORDLEN);
    break;
  }
  case OP_JAL: {
    uint8_t rd = RD(instr);
    Word imm = SIGN_EXTENSION(J_IMM(instr), J_IMM_SIZE) & 0xfffffffe;
    DISASSMSG("\tJ-type | JAL %s,%x\n", regName[rd], getPC() + imm);
    regWrite(rd, getPC() + WORDLEN);
    setNextPC(getPC() + imm);
    break;
  }
  case OP_JALR: {
    uint8_t rd = RD(instr);
    uint8_t rs1 = RS1(instr);
    Word imm = SIGN_EXTENSION(I_IMM(instr), I_IMM_SIZE);
    DISASSMSG("\tJ-type | JALR %s,%s(%x),%x\n", regName[rd], regName[rs1],
              regRead(rs1), (regRead(rs1) + imm) & 0xfffffffe);
    regWrite(rd, getPC() + WORDLEN);
    setNextPC(((regRead(rs1) + imm) & 0xfffffffe));
    break;
  }
  default: {
    ERRORMSG("OpCode not handled %x\n", instr);
    SignalExc(EXC_II, 0);
    e = true;
    ERROR("opcode not handled");
    break;
  }
  }

  return e;
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
