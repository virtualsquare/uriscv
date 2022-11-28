#ifndef URISCV_HART_H
#define URISCV_HART_H

/*
The base RISC-V ISA supports multiple concurrent threads of execution within a
single user address space. Each RISC-V hardware thread, or hart, has its own
user register state and program counter, and executes an independent sequential
instruction stream. The execution environment will define how RISC-V harts are
created and managed. RISC-V harts can communicate and synchronize with other
harts either via calls to the execution environment, which are documented
separately in the specification for each execution environment, or directly via
the shared memory system. RISC-V harts can also interact with I/O devices, and
indirectly with each other, via loads and stores to portions of the address
space assigned to I/O.

A	hardware	thread	(HART)	can	either
read	or	write	values	to/from	the	memory space
*/

/*
 *
 * Docs : https://wiki.osdev.org/RISC-V#Hardware_Threads
 *
 */

#include "bus.h"
#include "config.h"
#include <iterator>

#define NUM_REGS 32
#define NUM_CSRS 4096

// Hardware Thread
// Can have a coprocessor
class Hart {
public:
  Hart(Bus *bus);
  virtual ~Hart(){};

  void Init(Word id, Word pc, Word sp);
  void initCSR();

  exception_t Fetch(Word pc, Word *instr);
  exception_t Fetch(Word *instr);

  Word GRegRead(Word reg);
  void GRegWrite(Word reg, Word value);

  Word CSRRead(Word reg);
  void CSRWrite(Word reg, Word value);

  void SetPC(Word value);
  void SetSP(Word value);
  Word IncrementPC(Word value);
  Word IncrementSP(Word value);
  Word GetPC();
  Word GetSP();

private:
  Bus *bus;

  Word id;
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
