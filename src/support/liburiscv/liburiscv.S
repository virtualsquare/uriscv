#include "uriscv/arch.h"
#include "uriscv/bios.h"
#include "uriscv/cpu.h"
#include "uriscv/const.h"

#define LEAF_FUNC(func)				\
	.globl	func;              \
	.type	func, @function;   \
func:                      


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
 * Define register accessors.
 *
 * Each accessor is of the form: u32 get<REG_NAME>(void)
 */

LEAF_FUNC(getINDEX)
  csrrw a0,CSR_INDEX,zero
	jr	ra

LEAF_FUNC(getRANDOM)
  csrrw a0,CSR_RANDOM,zero
	jr	ra

LEAF_FUNC(getENTRYLO)
  csrrw a0,CSR_ENTRYLO,zero
	jr	ra

LEAF_FUNC(getBADVADDR)
  csrrw a0,CSR_BADVADDR,zero
	jr	ra

LEAF_FUNC(getTIMER)
  csrrw a0,time,zero
	jr	ra

LEAF_FUNC(getENTRYHI)
  csrrw a0,CSR_ENTRYHI,zero
	jr	ra

LEAF_FUNC(getSTATUS)
  csrrw a0,mstatus,zero
	jr	ra

LEAF_FUNC(getCAUSE)
  csrrw a0,mcause,zero
	jr	ra

LEAF_FUNC(getEPC)
  csrrw a0,mepc,zero
	jr	ra

LEAF_FUNC(getPRID)
  addi a0,a0,3
	jr	ra

LEAF_FUNC(getMIE)
  csrrw a0,mie,zero
	jr	ra

LEAF_FUNC(getMIP)
  csrrw a0,mip,zero
	jr	ra

/*
 * Setter functions
 * Each setter is of the form: u32 set<REG_NAME>(u32 value)
 * Each function returns the updated register value.
 */

LEAF_FUNC(setINDEX)
  csrrw zero,CSR_INDEX,a0
	jr	ra

LEAF_FUNC(setENTRYLO)
  csrrw zero,CSR_ENTRYLO,a0
	jr	ra

LEAF_FUNC(setTIMER)
  csrrw zero,time,a0
	jr	ra

LEAF_FUNC(setENTRYHI)
  csrrw zero,CSR_ENTRYHI,a0
	jr	ra

LEAF_FUNC(setSTATUS)
  csrrw zero,mstatus,a0
	jr	ra

LEAF_FUNC(setCAUSE)
  csrrw zero,mcause,a0
	jr	ra

LEAF_FUNC(setMIE)
  csrrw zero,mie,a0
	jr	ra


/* TLBWR instruction wrapper */
LEAF_FUNC(TLBWR)
	nop
	li	a0, BIOS_SRV_TLBWR
  ebreak
	nop
	jr	ra

/*
 * TLBWI instruction wrapper
 */
LEAF_FUNC(TLBWI)
	nop
	li	a0, BIOS_SRV_TLBWI
  ebreak
	nop
	jr	ra

/*
 * TLBP instruction wrapper
 */
LEAF_FUNC(TLBP)
	nop
	li	a0, BIOS_SRV_TLBP
  ebreak
	nop
	jr	ra

/*
 * TLBR instruction wrapper
 * Reads TLB entry at INDEX and returns result in ENTRYHI and ENTRYLO
 */
LEAF_FUNC(TLBR)
	nop
	li	a0, BIOS_SRV_TLBR
  ebreak
	nop
	jr	ra

/*
 * TLBCLR instruction wrapper
 */
LEAF_FUNC(TLBCLR)
 	nop
	li	a0, BIOS_SRV_TLBCLR
  ebreak
 	nop
 	jr	ra

/*
 * STST
 *
 * SYNOPSIS:
 *	void STST(state t *statep)
 *
 * This function will save processor status to memory block pointed by
 * register a0 (a0), and return PC value of instruction immediately
 * following the call as return value in v0.
 * PC field itself is intentionally left at 0 value
 */
LEAF_FUNC(STST)

	/* gets CAUSE register and stores it */
	sw	t1, 44(a0)

  csrrw t1,mcause,zero
	sw	t1, 4(a0)

	/* all processor registers saved */

	sw	ra, 24(a0)
	sw	sp, 28(a0)
	sw 	gp, 32(a0)
	sw	tp, 36(a0)
  sw  t0, 40(a0)
	sw	t1, 44(a0)
	sw	t2, 48(a0)
	sw	t3, 52(a0)
	sw	t4, 56(a0)
	sw	t5, 60(a0)
	sw	t6, 64(a0)
	sw	s0, 68(a0)
	sw	s1, 72(a0)
	sw	s2, 76(a0)
	sw	s3, 80(a0)
	sw	s4, 84(a0)
	sw	s5, 88(a0)
	sw	s6, 92(a0)
	sw	s7, 96(a0)
	sw	s8, 100(a0)
	sw	s9, 104(a0)
	sw	s10, 108(a0)
	sw	s11, 112(a0)
	sw	a0, 116(a0)
	sw	a1, 120(a0)
	sw	a2, 124(a0)
	sw	a3, 128(a0)
	sw	a4, 132(a0)
	sw	a5, 136(a0)
	sw	a6, 140(a0)
	sw	a7, 144(a0)

	/* gets EntryHI and stores it */
  csrrw t1,CSR_ENTRYHI,zero
	sw	t1, 0(a0)

  csrrw t1,mstatus,zero
	sw	t1, 8(a0)

  /* zeroes PC */
	sw	zero, 12(a0)

  csrrw t1,mie,zero
	sw	t1, 16(a0)

  lw a1, 120(a0)

  addi a0,ra,0


	jr	ra


/*
 * LDST
 *
 * SYNOPSIS:
 *	void LDST(state t *statep)
 *
 * This function forces the complete reload of processor state from
 * vector state area pointed by argument in a0 (a0): it works only in
 * kernel mode. There is no real return: a1 is used as BIOS
 * argument, but it is reloaded too.
 */
LEAF_FUNC(LDST)
  addi a1,a0,0

	li	a0, BIOS_SRV_LDST
	ebreak
	nop

	jr	ra

/*
 * LDCXT
 *
 * SYNOPSIS:
 *	void LDCXT(unsigned int stackPtr, unsigned int status, unsigned int pc)
 *
 * This function forces a processor to change its operating mode reloading
 * SP (a0), status (a1) and PC (a2) registers: it works only in
 * kernel mode. There is no real return: a1 is used as BIOS
 * argument, but it is reloaded too.
 */
LEAF_FUNC(LDCXT)
  addi a3,a2,0
  addi a2,a1,0
  addi a1,a0,0

	li	a0, BIOS_SRV_LDCXT
	ebreak
	nop
	jr ra


/*
 * WAIT instruction wrapper
 */
LEAF_FUNC(WAIT)
	nop
	wfi
	nop
	jr	ra

/*
 * HALT
 */
LEAF_FUNC(HALT)
	li	a0, BIOS_SRV_HALT
	ebreak
	nop
	jr	ra


/*
 * PANIC
 */
LEAF_FUNC(PANIC)
	li	a0, BIOS_SRV_PANIC
	ebreak
	nop
	jr	ra

