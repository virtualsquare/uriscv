#ifndef UMPS_PROCESSOR_RISCV_H
#define UMPS_PROCESSOR_RISCV_H

#include "base/lang.h"
#include "uriscv/const.h"
#include "uriscv/types.h"

class MachineConfig;
class Machine;
class SystemBus;

class Processor {
public:
  static const unsigned int kNumRegisters = 32;

  Processor(const MachineConfig *config, Word id, Machine *machine,
            SystemBus *bus);
  virtual ~Processor();

  Word getId() const { return id; }
  Word Id() const { return id; }

  ProcessorStatus getStatus() const { return status; }

  bool isHalted() const { return status == PS_HALTED; }
  bool isRunning() const { return status == PS_RUNNING; }
  bool isIdle() const { return status == PS_IDLE; }

  void Reset(Word pc, Word sp);
  void Halt();

  // This method makes Processor execute a single instruction.  For
  // simulation purposes, it differs from traditional processor cycle:
  // the first instruction after a reset is pre-loaded, and cycle is
  // execute - fetch instead of fetch - execute.  This way, it is
  // possible to know what instruction will be executed before its
  // execution happens
  void Cycle();

  uint32_t IdleCycles() const;

  void Skip(uint32_t cycles);

  // This method allows SystemBus and Processor itself to signal
  // Processor when an exception happens. SystemBus signal IBE/DBE
  // exceptions; Processor itself signal all other kinds of exception.
  void SignalExc(unsigned int exc, Word cpuNum = 0UL);

  void AssertIRQ(unsigned int il);
  void DeassertIRQ(unsigned int il);

  // The following methods allow inspection of Processor internal
  // status. Name & parameters are self-explanatory: remember that
  // all addresses are _virtual_ when not marked Phys/P/phys (for
  // physical ones), and all index checking must be performed from
  // caller.

  void getCurrStatus(Word *asid, Word *pc, Word *instr, bool *isLD, bool *isBD);

  Word getASID() const;
  Word getPC() const { return currPC; }
  Word getInstruction() const { return currInstr; }

  bool InUserMode() const;
  bool InKernelMode() const;

  void getPrevStatus(Word *pc, Word *instr);
  const char *getExcCauseStr();
  Word getNextPC(void);
  Word getSuccPC(void);
  Word getPrevPPC(void);
  Word getCurrPPC(void);
  SWord getRegister(unsigned int num);

  // The following methods allow to change Processor internal status
  // Name & parameters are almost self-explanatory: remember that
  // all addresses are _virtual_ when not marked Phys/P/phys (for
  // physical ones), and all index checking must be performed from
  // caller.  Processor status modification is allowed during
  // debugging inside the simulation.

  void setRegister(unsigned int num, SWord val);
  void setNextPC(Word npc);
  void setSuccPC(Word spc);

  // Signals
  sigc::signal<void> StatusChanged;
  sigc::signal<void, unsigned int> SignalException;
  sigc::signal<void, unsigned int> SignalTLBChanged;

private:
};

#endif
