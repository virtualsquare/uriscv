#include "uriscv/arch.h"
#include "uriscv/bios.h"

#define LEAF_FUNC(func)				\
	.globl	func;              \
	.type	func, @function;   \
func:                      

#define END_LEAF_FUNC(func)			\
	.end	func;                 \
	.size	func, . - func

	/* We don't want abicalls unconditionally. */
#ifdef ABICALLS
	.abicalls
#endif

	/*
	 * Code start
	 */

	.text
	.align 2

/*
 * This function cause a system call trap.
 * System call code is in a0, return value in v0.
 * It is programmer's task to load the return value into
 * register, and to set PC correctly for returning _after_
 * syscall
 */
/* SYSCALL(a0,a1,a2,a3) */
LEAF_FUNC(SYSCALL)
	nop
  ecall
	nop
	jr	ra

/*
 * WAIT instruction wrapper
 */
LEAF_FUNC(WAIT)
  /*
	.set	noreorder
	.set	nomacro
  */
	nop
  /*
	.set	mips32
  */
	wfi
  /*
	.set	mips0
  */
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

