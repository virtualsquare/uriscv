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
*/

/*
 *
 * Docs : https://wiki.osdev.org/RISC-V#Hardware_Threads
 *
 */

#include "bus.h"
#include "config.h"
#include "const.h"
#include "types.h"

// Hardware Thread
// Can have a coprocessor
class Hart {
public:
  Hart();
  virtual ~Hart();

private:
  Word id;
};

#endif
