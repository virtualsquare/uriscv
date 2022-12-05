#ifndef URISCV_PROCESSOR_H
#define URISCV_PROCESSOR_H

#include "config.h"
#include "uriscv/hart.h"
#include "uriscv/machine.h"
#include "uriscv/mybus.h"

enum MyProcessorStatus { PS_HALTED, PS_RUNNING, PS_IDLE };

class MyProcessor {
public:
  MyProcessor(Config *config, MyBus *bus);
  MyProcessor(const MachineConfig *config, Word id, Machine *machine,
              SystemBus *bus);

  virtual ~MyProcessor(){};

  void Init(Word nharts, Word pc, Word sp);

  /*
   *
   * Legacy
   *
   */
  size_t tlbSize;

  Word tlbFloorAddress;

  Word getId() const { return id; }
  Word Id() const { return id; }
  MyProcessorStatus getStatus() const { return status; }
  bool isHalted() const { return status == PS_HALTED; }
  bool isRunning() const { return status == PS_RUNNING; }
  bool isIdle() const { return status == PS_IDLE; }
  void Reset(Word pc, Word sp);
  void Halt();

  uint32_t IdleCycles() const;

  void zapTLB(void);

  bool mapVirtual(Word vaddr, Word *paddr, Word accType);
  bool probeTLB(unsigned int *index, Word asid, Word vpn);

  void setTLBRegs(Word vaddr);

  void Skip(uint32_t cycles);

  // This method allows MyBus and MyProcessor itself to signal
  // MyProcessor when an exception happens. MyBus signal IBE/DBE
  // exceptions; MyProcessor itself signal all other kinds of exception.
  void SignalExc(unsigned int exc, Word cpuNum = 0UL);

  void AssertIRQ(unsigned int il);
  void DeassertIRQ(unsigned int il);

  // The following methods allow inspection of MyProcessor internal
  // status. Name & parameters are self-explanatory: remember that
  // all addresses are _virtual_ when not marked Phys/P/phys (for
  // physical ones), and all index checking must be performed from
  // caller.

  void getCurrStatus(Word *asid, Word *pc, Word *instr, bool *isLD, bool *isBD);

  Word getASID() const;
  Word getInstruction() const { return currInstr; }

  bool InUserMode() const;
  bool InKernelMode() const;

  // Signals
  sigc::signal<void> StatusChanged;
  sigc::signal<void, unsigned int> SignalException;
  sigc::signal<void, unsigned int> SignalTLBChanged;

  /*
   *
   * End legacy
   *
   */

  exception_t Fetch(Word pc, Word *instr);
  exception_t Fetch(Word *instr);
  exception_t Excecute(Word instr);

  exception_t Cycle();
  void TrapHandler(exception_t e);

  Word GRegRead(Word reg);
  void GRegWrite(Word reg, Word value);

  Word CSRRead(Word reg);
  void CSRWrite(Word reg, Word value);

  Hart *GetCurrentHart();

  void SetPC(Word value);
  void SetSP(Word value);
  Word IncrementPC(Word value);
  Word IncrementSP(Word value);
  Word GetPC();
  Word GetSP();

private:
  const Word id;

  Config *config;
  MyBus *bus;
  Hart **harts;
  MyProcessorStatus status;

  // currently selected hart
  Word selHart;
  uint8_t old_mode;
  uint8_t mode;
  Word currInstr;

  // Bit Description
  // 0	 USIE	–	user	software	interrupt	enable
  // 1	 SSIE	–	supervisor
  // 3	 MSIE	–	machine
  // 4	 UTIE	–	user	timer	interrupt	enable
  // 5	 STIE	–	supervisor
  // 7	 MTIE	–	machine
  // 8	 UEIE	–	user	external	interrupt	enable
  // 9	 SEIE	–	supervisor
  // 11	 MEIE	–	machine
  uint16_t mie;

  // Bit Description
  // 0	 USIP	–	user	software	interrupt	pending
  // 1	 SSIP	–	supervisor
  // 3	 MSIP	–	machine
  // 4	 UTIP	–	user	timer	interrupt	pending
  // 5	 STIP	–	supervisor
  // 7	 MTIP	–	machine
  // 8	 UEIP	–	user	external	interrupt	pending
  // 9	 SEIP	–	supervisor
  // 11	 MEIP	–	machine
  uint16_t mip;

  // Same but for Supervisor and User mode
  uint16_t sie;
  uint16_t sip;
  uint16_t uie;
  uint16_t uip;

  Word mprv;
};

#endif
