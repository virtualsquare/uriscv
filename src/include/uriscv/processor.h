#ifndef URISCV_PROCESSOR_H
#define URISCV_PROCESSOR_H

#include "bus.h"
#include "config.h"
#include "uriscv/hart.h"

class Processor {
public:
  Processor(Config *config, Bus *bus);
  virtual ~Processor(){};

  void Init(Word nharts, Word pc, Word sp);

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
  Config *config;
  Bus *bus;
  Hart **harts;

  // currently selected hart
  Word selHart;
  uint8_t old_mode;
  uint8_t mode;

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
