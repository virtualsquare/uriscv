#ifndef URISCV_PROCESSOR_H
#define URISCV_PROCESSOR_H

#include "bus.h"
#include "config.h"
#include "const.h"
#include "types.h"

#define NUM_REGS 32
#define NUM_CSRS 4096

class Processor {
public:
  Processor(Config *config);
  virtual ~Processor(){};

  void Init(Word pc, Word sp);

  Word Fetch(Word pc);
  Word Fetch();
  bool Excecute(Word instr);

  Word GRegRead(Word reg);
  void GRegWrite(Word reg, Word value);

  Word CSRRead(Word reg);
  void CSRWrite(Word reg, Word value);

  void initCSR();

private:
  Config *config;
  Bus *bus;

  Word pc, sp;
  /*
   * Registers
   * x0 - zero
   * x1 - ra
   * x2 - sp
   * x3 - gp
   * x4 - tp
   * x5-x7,x28-x31 t0-t6
   * x8,x9,x18-x27 s0-s11
   * x8 fp
   * x10-x17 a0-a7
   */
  Word gregs[NUM_REGS];
  Word fregs[NUM_REGS];
  csr_t csrs[NUM_CSRS];
};

#endif
