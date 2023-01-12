#include "uriscv/arch.h"
#include "uriscv/bios.h"
#include "uriscv/cp0.h"

#define LEAF_FUNC(func)				\
	.globl	func;              \
	.type	func, @function;   \
func:                      

#define END_LEAF_FUNC(func)			\
	.end	func;                 \
	.size	func, . - func


#define DEFINE_CP0_GETTER(suffix, reg)          \
LEAF_FUNC(get ## suffix);                       \
/*
	.set	noreorder;                         \
	.set	nomacro;                           \
  */
	nop;                                    \
  /*
	mfc0	v0, reg;                       \
  */
	nop;                                    \
  /*
	.set	reorder;                           \
	.set	macro;                             \
  */
	jr	ra

#define DEFINE_CP0_SETTER(suffix, reg)          \
LEAF_FUNC(set ## suffix);                       \
  /*
	.set	noreorder;                      \
	.set	nomacro;                        \
  */
	nop;                                    \
  /*
	mtc0	a0, reg;                       \
  */
	nop;                                    \
  /*
	mfc0	v0, reg;                       \
  */
	nop;                                    \
  /*
	.set	reorder;                        \
	.set	macro;                          \
  */
	jr	ra


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
 * Define CP0 register accessors.
 *
 * Each accessor is of the form: u32 get<REG_NAME>(void)
 */
 /*
DEFINE_CP0_GETTER(INDEX, CP0_Index)
*/
LEAF_FUNC(getINDEX)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(RANDOM, CP0_Random)
*/
LEAF_FUNC(getRANDOM)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(ENTRYLO, CP0_EntryLo)
*/
LEAF_FUNC(getENTRYLO)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(BADVADDR, CP0_BadVAddr)
*/
LEAF_FUNC(getBADVADDR)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(TIMER, CP0_Timer)
*/
LEAF_FUNC(getTIMER)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(ENTRYHI, CP0_EntryHi)
*/
LEAF_FUNC(getENTRYHI)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(STATUS, CP0_Status)
*/
LEAF_FUNC(getSTATUS)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(CAUSE, CP0_Cause)
*/
LEAF_FUNC(getCAUSE)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(EPC, CP0_EPC)
*/
LEAF_FUNC(getEPC)
  addi a0,a0,3
	jr	ra
/*
DEFINE_CP0_GETTER(PRID, CP0_PRID)
*/
LEAF_FUNC(getPRID)
  addi a0,a0,3
	jr	ra

/*
 * CP0 setter functions
 * Each setter is of the form: u32 set<REG_NAME>(u32 value)
 * Each function returns the updated register value.
 */
/*
DEFINE_CP0_SETTER(INDEX, CP0_Index)
*/
LEAF_FUNC(setINDEX)
  addi a1,a1,3
	jr	ra
/*
DEFINE_CP0_SETTER(ENTRYLO, CP0_EntryLo)
*/
LEAF_FUNC(setENTRYLO)
  addi a1,a1,3
	jr	ra
/*
DEFINE_CP0_SETTER(TIMER, CP0_Timer)
*/
LEAF_FUNC(setTIMER)
  addi a1,a1,3
	jr	ra
/*
DEFINE_CP0_SETTER(ENTRYHI, CP0_EntryHi)
*/
LEAF_FUNC(setENTRYHI)
  addi a1,a1,3
	jr	ra
/*
DEFINE_CP0_SETTER(STATUS, CP0_Status)
*/
LEAF_FUNC(setSTATUS)
  addi a1,a1,3
	jr	ra
/*
DEFINE_CP0_SETTER(CAUSE, CP0_Cause)
*/
LEAF_FUNC(setCAUSE)
  addi a1,a1,3
	jr	ra



/*
 * TLBWR instruction wrapper
 */
LEAF_FUNC(TLBWR)
  /*
	.set	noreorder
	.set	nomacro
  */
	nop
  /*
	tlbwr
  */
  addi a7,a7,20
  ecall
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

/*
 * TLBWI instruction wrapper
 */
LEAF_FUNC(TLBWI)
  /*
	.set	noreorder
	.set	nomacro
  */
	nop
  /*
	tlbwi
  */
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

/*
 * TLBP instruction wrapper
 */
LEAF_FUNC(TLBP)
  /*
	.set	noreorder
	.set	nomacro
  */
	nop
  /*
	tlbp
  */
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

/*
 * TLBR instruction wrapper
 * Reads TLB entry at INDEX and returns result in ENTRYHI and ENTRYLO
 */
LEAF_FUNC(TLBR)
  /*
	.set	noreorder
	.set	nomacro
  */
	nop
  /*
	tlbr
  */
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

/*
 * TLBCLR instruction wrapper
 */
LEAF_FUNC(TLBCLR)
  /*
 	.set	noreorder
 	.set	nomacro
  */
 	nop
  /*
 	mtc0	a0, 4
  */
 	nop
  /*
 	.set	reorder
 	.set	macro
  */
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
  /*
	.set	noat
	sw	1, 16(a0)
	sw	v0, 20(a0)
  */

	/* gets CAUSE register and stores it */
  /*
	mfc0	v0, CP0_Cause
	sw 	v0, 4(a0)
  */

  /*
	sw	v1, 24(a0)
	sw	a0, 28(a0)
	sw	a1, 32(a0)
	sw	a2, 36(a0)
	sw	a3, 40(a0)
	sw	t0, 44(a0)
	sw	t1, 48(a0)
	sw	t2, 52(a0)
	sw	t3, 56(a0)
	sw	t4, 60(a0)
	sw	t5, 64(a0)
	sw	t6, 68(a0)
	sw	t7, 72(a0)
	sw	s0, 76(a0)
	sw	s1, 80(a0)
	sw	s2, 84(a0)
	sw	s3, 88(a0)
	sw	s4, 92(a0)
	sw	s5, 96(a0)
	sw	s6, 100(a0)
	sw	s7, 104(a0)
	sw	t8, 108(a0)
	sw	t9, 112(a0)
  */

	/* k0 and k1 are not saved */

  /*
	sw	gp, 116(a0)
	sw	sp, 120(a0)
	sw	fp, 124(a0)
	sw	ra, 128(a0)
	mfhi	a1
	sw	a1, 132(a0)
	mflo	a1
	sw	a1, 136(a0)
  */
	/* all processor registers saved */

	/* gets EntryHI and stores it */
  /*
	mfc0	v0, CP0_EntryHi
	sw	v0, 0(a0)
  */

	/* and now saves STATUS register and zeroes PC */
  /*
	mfc0	v0, CP0_Status
	sw	v0, 8(a0)
	sw	0, 12(a0)
  */

	/* reloads v1 and a1 registers */
  /*
	lw	v1, 24(a0)
	lw 	a1, 32(a0)
  */

	/* sets v0 to return value */
  /*
	move	v0, ra
  */
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
  /*
	move	a1, a0
  */
  addi a1,a0,0

  /*
	.set	noreorder
	.set	nomacro
  */
	li	a0, BIOS_SRV_LDST
	ebreak
	nop
  /*
	.set	reorder
	.set	macro
  */

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
  /*
	move	a3, a2
	move	a2, a1
	move	a1, a0
  */
  addi a3,a2,0
  addi a2,a1,0
  addi a1,a0,0

  /*
	.set	noreorder
	.set	nomacro
  */
	li	a0, BIOS_SRV_LDCXT
	ebreak
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr ra


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

/*
 * HALT
 */
LEAF_FUNC(HALT)
  /*
	.set	noreorder
	.set	nomacro
  */
	li	a0, BIOS_SRV_HALT
	ebreak
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra


/*
 * PANIC
 */
LEAF_FUNC(PANIC)
  /* 
	.set	noreorder
	.set	nomacro
  */
	li	a0, BIOS_SRV_PANIC
	ebreak
	nop
  /*
	.set	reorder
	.set	macro
  */
	jr	ra

