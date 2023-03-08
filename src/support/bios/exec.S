/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2011 Tomislav Jonjic
 * Copyright (C) 2020 Mattia Biondi, Mikey Goldweber
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "uriscv/arch.h"
#include "uriscv/cp0.h"
#include "uriscv/bios_defs.h"

/* Constants */

#define ASIDSTEP    0x00000040
#define VPNMASK     0xFFFFF000
#define VECTSIZE    140

/* NOP padding: 30 nops - string length (32 bytes) */
#define GENEXC_PADDING    (30 * 4) - 32
#define ENDSTRADDR        0x00000008
#define PANICSTRADDR      (ENDSTRADDR + 16)

/* Terminal device related stuff */
#define TERM0COMMAND      DEV_REG_ADDR(IL_TERMINAL, 0) + 0xC
#define BUSYCODE          3
#define BYTELEN           8
#define PRINTCHR          2

#define CAUSENEGMASK      0xFFFFFF83
#define KUPSTATUSMASK     0x00000008
#define SAFESTATUSMASK    0xFFFFFFFC

#define READK0(reg) \
  csrrw reg,mscratch,zero
#define WRITEK0(reg) \
  csrrw zero,mscratch,reg
#define READK1(reg) \
  csrrw reg,dscratch,zero
#define WRITEK1(reg) \
  csrrw zero,dscratch,reg

	/*
	 * Code start
	 */

	.text
	.align	2
	.globl	jios
	.type	jios,@function
	//.ent	jios
bios:
/*
	.frame	fp,0,k1
	.mask	0x00000000,0
	.fmask	0x00000000,0
  */

	/*
	 * Tell gas not to use at in pseudoop expansions
	 */
	//.set	noat

	/*
	 * 0x00000000 address
	 * This is the entry point for UTLB type exceptions.
	 */
	//.set	noreorder
	//.set	nomacro
	j	LUTLBHandler
	nop

EndStr:
	.string	"System halted \n"
PanicStr:
	.string "kernel panic()\n"

	.space	GENEXC_PADDING

	/*
	 * 0x00000080 address
	 * This is the entry point for general ("other", non-TLB) exceptions.
	 */
	j	LEXCHandler
	nop

	; .space	0x80 - 0x8
	/*
	 * 0x00000100 address
	 * Secondary processor startup routine
	 */
	; j	LInitSecondaryProcessor
	; nop

	/*
	 * 0x00000108: panic()
	 * Print a message on terminal 0 and loop forever
	 */

	//.set	reorder
	//.set	macro

LPanic:
	li	a0, PANICSTRADDR - 1

LTermOp:
	li 	a3, TERM0COMMAND
	li 	t0, BUSYCODE

LWaitForReady:
	lw 	a2, -4(a3)
	beq	a2, t0, LWaitForReady

LPrintLoop:
	addi	a0,a0, 1
	lbu	a1, 0(a0)

	# char 0 ends the string
	beq	zero, a1, LInfinite

	#prepares PRINTCHAR command
	sll 	a1,a1, BYTELEN
	addi	a1,a1, PRINTCHR
	sw	a1, 0(a3)

	j	LWaitForReady

LInfinite:
	j	LInfinite

LHalt:
	# this call prints a message on terminal 0 and loops forever
	li	a0, ENDSTRADDR - 1
	j 	LTermOp

/*
 * Get a cpu up and runing: initialize BIOS related structures and
 * load the supplied processor state.
 */
LInitSecondaryProcessor:
	/* Initialize ptr to exception state vector */
	li	t0, VECTSIZE
	/* mfc0	t1, CP0_PRID // CP0 not enabled */
/*	 mult	t0, t1 // TODO: add mult */
  mul t0,t0,t1
	/* mflo	t0 // CPO not enabled */
	 li	t2, BIOS_DATA_PAGE_BASE
	add	t0, t0, t2
	li	t2, BIOS_EXCPT_VECT_BASE
	sw	t0, 0(t2)

	/* Initialize ptr to PC/SP area */
	li	t0, 16
  mul t0,t0,t1 
	/* mflo	t0 // CPO not enabled */
	li	t1, BIOS_EXEC_HANDLERS_ADDRS
	add	t0, t0, t1
	li	t2, BIOS_PC_AREA_BASE
	sw	t0, 0(t2)

	/* Load the new state - address of start_state cached at start of exception vector */
	li	t0, BIOS_EXCPT_VECT_BASE
	/* sw	k0, 0(t0) */
  READK0(t1)
	sw 	t1, 0(t0) // mem[t0] = k0
	j	LLoadStart

/*
 * Handle TLBL/TLBS refill misses
 * k1 is address of kernel branch address (TLB Refill handler)
 */
LUTLBHandler:
	/* lw	k1, BIOS_PC_AREA_BASE */
  li t0,BIOS_PC_AREA_BASE // load BIOS_PC_AREA_BASE in to t1
  lw t1,0(t0) // so we can do t0 = mem[t1]
  WRITEK1(t1)
	j	LSaveExcptAndJmp

/*
 * General exception handler
 *
 * k0 and k1 registers are always available (never have live
 * values outside BIOS)
 */
LEXCHandler:
/* TODO: riscrivere */
/* check if trap or interrupt */
  csrrw t0,mcause,zero
  srli t0,t0,31
  addi t0,t0,-1
  beq t0,zero,LIntHandler

  csrrw t0,mcause,zero
  addi t0,t0,-9
  beq t0,zero,LBreakHandler

	/* lw	k1, BIOS_PC_AREA_BASE */
  li t0,BIOS_PC_AREA_BASE // load BIOS_PC_AREA_BASE in to t1
  lw t1,0(t0) // so we can do t0 = mem[t1]
  addi t1,t1,8
  WRITEK1(t1)
	j	LSaveExcptAndJmp


LIntHandler:
  addi t0,t0,0

LBreakHandler:

	/* any other break is passed up to SYS handler */
	/* li	k1, BIOS_PC_AREA_BASE */
  /*
  li t1, BIOS_PC_AREA_BASE
  WRITEK1(t1)
  */
	/* addi	k1, k1, 8 */
  /*
  READK1(t1)
  addi t1, t1, 8
  WRITEK1(t1)
  */
  // TODO: check if in kernel mode

  addi t0,a0,-1
  beq t0, zero, LLDST

	/* If in user mode, pass along to kernel */
	/* li	k1, BUS_REG_RAM_BASE */
  li t1, BUS_REG_RAM_BASE
	/* addi	k1, k1, 8 */
  addi t1, t1, 8
  WRITEK1(t1)
	j	LSaveExcptAndJmp

LisKernel:
	/* 0 is LDCXT(stackPtr, STATUS, PC) */
	/* beq	0, k1, LLDCXT */
  READK1(t1)
  beq zero, t1, LLDCXT

	/* addi	k1, -1 */
  READK1(t1)
  addi t1, t1, -1
  WRITEK1(t1)

	/* 1 is LDST */
  /* beq	0, k1, LLDST */
  READK1(t1)
  beq zero, t1, LLDST

	/* addi	k1, -1 */
  READK1(t1)
  addi t1, t1, -1
  WRITEK1(t1)

	/* 2 is PANIC routine */
	/* beq	0, k1, LPanic */
  READK1(t1)
  beq zero, t1, LPanic 

	/* addi	k1, -1 */
  READK1(t1)
  addi t1, t1, -1
  WRITEK1(t1)

	/* 3 is HALT routine */
	/* beq	0, k1, LHalt */
  READK1(t1)
  beq zero, t1, LHalt

	/* any other break is passed up to SYS handler */
	/* li	k1, BIOS_PC_AREA_BASE */
  li t1, BIOS_PC_AREA_BASE
  WRITEK1(t1)
	/* addi	k1, k1, 8 */
  READK1(t1)
  addi t1, t1, 8
  WRITEK1(t1)
	j	LSaveExcptAndJmp

LLDCXT:
	# a1 is stackPtr, a2 is new STATUS, a3 is PC

	# STATUS preparation
  /* move	k1, a2 */
  WRITEK1(a2)
	# this for BIOS safety: no KU or IE bits on
	/* li	k0, SAFESTATUSMASK */
  li t0, SAFESTATUSMASK
  WRITEK0(t0)
	/* and	k1, k1, k0 */
  READK0(t0)
  READK1(t1)
  and t1, t1, t0
  WRITEK1(t1)

	# STATUS loading
	/* mtc0	k1, CP0_Status // CP0 not enabled */

	# stackPtr loading
	move	sp, a1

	# get new PC and jump
	/* move	k1, a3 */
  WRITEK1(a3)
  /* TODO: check if it is useful */
	/* move	t9, a3 */

	//.set	noreorder
	//.set	nomacro
	/* jr	k1 */
  READK1(t1)
  jr t1
	mret
	//.set	reorder
	//.set	macro

LLDST:
	# this means load from physical address in a1
	/* move	k0, a1 */
  WRITEK0(a1)
	j	LLoadStart

/*
 * Save cpu (exception) state into BIOS data page, load PC and SP
 *  and pass along handling to the kernel
 *    Kernel handler address (PC) is in k1 and SP is in k1 + 4
 */
LSaveExcptAndJmp:
	li	t0, BIOS_EXCPT_VECT_BASE
  lw t0,0(t0)
  WRITEK0(t0)
LSave:
  /* addi sp, sp, -33*4 */
  READK0(t0)

  /* salvare
    entry_hi
    cause
    status
    pc_epc
    gpr (fatto)
    hi
    lo
  */

	sw	ra, 20(t0)
	sw	sp, 24(t0)
	sw 	gp, 28(t0)
	sw	tp, 32(t0)
	sw	t1, 40(t0)
	sw	t2, 44(t0)
	sw	t3, 48(t0)
	sw	t4, 52(t0)
	sw	t5, 56(t0)
	sw	t6, 60(t0)
	sw	s0, 64(t0)
	sw	s1, 68(t0)
	sw	s2, 72(t0)
	sw	s3, 76(t0)
	sw	s4, 80(t0)
	sw	s5, 84(t0)
	sw	s6, 88(t0)
	sw	s7, 92(t0)
	sw	s8, 96(t0)
	sw	s9, 100(t0)
	sw	s10, 104(t0)
	sw	s11, 108(t0)
	sw	a0, 112(t0)
	sw	a1, 116(t0)
	sw	a2, 120(t0)
	sw	a3, 124(t0)
	sw	a4, 128(t0)
	sw	a5, 132(t0)
	sw	a6, 136(t0)
	sw	a7, 140(t0)

  csrrw t1,mcause,zero
	sw	t1, 4(t0)

  // epc
  csrrw t1,mepc,zero
	sw	t1, 12(t0)

  READK0(t0)
  sw t1, 40(t0)
	sw	t0, 36(t0)

  READK1(t1)
  lw sp, 4(t1)

	/* jr	k1 */
  lw t1,0(t1)
  jr t1
	nop

/*
 * Load CPU/CP0 registers from (state_t*) k0
 */
LLoadStart:

  /* salvare
    entry_hi
    cause
    status
    pc_epc
    gpr (fatto)
    hi
    lo
  */

  /* pc_epc */
  /*
	lw	t0, 16(t0)
  csrrw zero,mepc,t0
  */

  READK0(t0)

  /* gpr */
	lw	ra, 20(t0)
	lw	sp, 24(t0)
	lw 	gp, 28(t0)
	lw	tp, 32(t0)
	lw	t1, 40(t0)
	lw	t2, 44(t0)
	lw	t3, 48(t0)
	lw	t4, 52(t0)
	lw	t5, 56(t0)
	lw	t6, 60(t0)
	lw	s0, 64(t0)
	lw	s1, 68(t0)
	lw	s2, 72(t0)
	lw	s3, 76(t0)
	lw	s4, 80(t0)
	lw	s5, 84(t0)
	lw	s6, 88(t0)
	lw	s7, 92(t0)
	lw	s8, 96(t0)
	lw	s9, 100(t0)
	lw	s10, 104(t0)
	lw	s11, 108(t0)
	lw	a0, 112(t0)
	lw	a1, 116(t0)
	lw	a2, 120(t0)
	lw	a3, 124(t0)
	lw	a4, 128(t0)
	lw	a5, 132(t0)
	lw	a6, 136(t0)
	lw	a7, 140(t0)

  lw t0, 12(t0) // epc
  WRITEK1(t0)
  addi t6,t0,0 // TODO: mhhh (return address)

  // TODO: save status etc....

  READK0(t0)
	lw	t0, 36(t0)

  jr t6

	mret
	//.set	reorder
	//.set	macro

	.end	jios
	.size	jios, . - bios
