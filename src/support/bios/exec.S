/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2011 Tomislav Jonjic
 * Copyright (C) 2020 Mattia Biondi, Mikey Goldweber
 * Copyright (C) 2023 Gianmaria Rovelli
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
#include "uriscv/cpu.h"
#include "uriscv/bios.h"
#include "uriscv/const.h"

/* Constants */

#define ASIDSTEP    0x00000040
#define VPNMASK     0xFFFFF000
#define VECTSIZE    140

/* NOP padding: 30 nops - string length (32 bytes) */
#define PANIC_PADDING    (30 * 4) - 32
#define ENDSTRADDR        0x00000008
#define PANICSTRADDR      (ENDSTRADDR + 16)

/* Terminal device related stuff */
#define TERM0COMMAND      DEV_REG_ADDR(IL_TERMINAL, 0) + 0xC
#define BUSYCODE          3
#define BYTELEN           8
#define PRINTCHR          2

#define CAUSENEGMASK      0xFFFFFF83
#define KUPSTATUSMASK     0x00000008
/* #define SAFESTATUSMASK    0xFFFFFFFC */
#define SAFESTATUSMASK 0xFFFFFFF0
#define MSTATUSMPPM 0x1800

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
bios:

	/*
	 * 0x00000000 address
	 * This is the entry point for UTLB type exceptions.
	 */
	j	LEXCHandler
	nop

EndStr:
	.string	"System halted \n"
PanicStr:
	.string "kernel panic()\n"

	.space	PANIC_PADDING

	/*
	 * 0x00000080: panic()
	 * Print a message on terminal 0 and loop forever
	 */

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
 * Handle TLBL/TLBS refill misses
 * k1 is address of kernel branch address (TLB Refill handler)
 */
LUTLBHandler:
  csrrw t0,mcause,zero
  li t1,11
  sub t0,t0,t1
  csrrw zero,mcause,t0
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

  csrrw t0,mcause,zero
  srli t0,t0,31
  addi t0,t0,-1
  beq t0,zero,LIntHandler

  csrrw t0,mcause,zero
  li t1,0x7FFFFFFF
  and t0,t0,t1 
  li t1,EXC_UTLBL
  bgeu t0,t1,LUTLBHandler

  csrrw t0,mcause,zero
  addi t0,t0,-3
  beq t0,zero,LBreakHandler

  li t0,BIOS_PC_AREA_BASE // load BIOS_PC_AREA_BASE in to t1
  lw t1,0(t0) // so we can do t0 = mem[t1]
  addi t1,t1,8
  WRITEK1(t1)
	j	LSaveExcptAndJmp

LIntHandler:
  li t0,BIOS_PC_AREA_BASE // load BIOS_PC_AREA_BASE in to t1
  lw t1,0(t0) // so we can do t0 = mem[t1]
  addi t1,t1,8
  WRITEK1(t1)
	j	LSaveExcptAndJmp

LBreakHandler:

	/* 0 is LDCXT(stackPtr, STATUS, PC) */
  add t1, a0,0
  beq zero, t1, LLDCXT

  addi t1, t1, -1

	/* 1 is LDST */
  beq zero, t1, LLDST

  addi t1, t1, -1

	/* 2 is PANIC routine */
  beq zero, t1, LPanic 

  addi t1, t1, -1

	/* 3 is HALT routine */
  beq zero, t1, LHalt

  li t1, BUS_REG_RAM_BASE
  addi t1, t1, 8
  WRITEK1(t1)
	j	LSaveExcptAndJmp

LLDCXT:
	# a1 is stackPtr, a2 is new STATUS, a3 is PC

	# STATUS preparation
  move t1, a2
	# this for BIOS safety: no KU or IE bits on
  li t0, SAFESTATUSMASK
  and t1, t1, t0

	# STATUS loading
  csrrw zero,mstatus,t1

	# stackPtr loading
	move	sp, a1

	# get new PC and jump
  move t1, a3
  csrrw zero,mepc,a3

	mret
  jr t1

LLDST:
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
  READK0(t0)

	sw	ra, 24(t0)
	sw	sp, 28(t0)
	sw 	gp, 32(t0)
	sw	tp, 36(t0)
	sw	t1, 44(t0)
	sw	t2, 48(t0)
	sw	t3, 52(t0)
	sw	t4, 56(t0)
	sw	t5, 60(t0)
	sw	t6, 64(t0)
	sw	s0, 68(t0)
	sw	s1, 72(t0)
	sw	s2, 76(t0)
	sw	s3, 80(t0)
	sw	s4, 84(t0)
	sw	s5, 88(t0)
	sw	s6, 92(t0)
	sw	s7, 96(t0)
	sw	s8, 100(t0)
	sw	s9, 104(t0)
	sw	s10, 108(t0)
	sw	s11, 112(t0)
	sw	a0, 116(t0)
	sw	a1, 120(t0)
	sw	a2, 124(t0)
	sw	a3, 128(t0)
	sw	a4, 132(t0)
	sw	a5, 136(t0)
	sw	a6, 140(t0)
	sw	a7, 144(t0)

  csrrw t1,CSR_ENTRYHI,zero
  sw t1,0(t0)

  csrrw t1,mcause,zero
	sw	t1, 4(t0)

  csrrw t1,mstatus,zero
	sw	t1, 8(t0)

  csrrw t1,mepc,zero
	sw	t1, 12(t0)

  csrrw t1,mie,zero
	sw	t1, 16(t0)

  READK0(t0)
  sw t1, 44(t0)
	sw	t0, 40(t0)

  READK1(t1)
  lw sp, 4(t1)

  lw t1,0(t1)
  csrrw zero,mepc,t1

  jr t1
	nop

/*
 * Load CPU/CP0 registers from (state_t*) k0
 */
LLoadStart:

  READK0(t0)

  /* gpr */
	lw	ra, 24(t0)
	lw	sp, 28(t0)
	lw 	gp, 32(t0)
	lw	tp, 36(t0)
	lw	t1, 44(t0)
	lw	t2, 48(t0)
	lw	t3, 52(t0)
	lw	t4, 56(t0)
	lw	t5, 60(t0)
	lw	t6, 64(t0)
	lw	s0, 68(t0)
	lw	s1, 72(t0)
	lw	s2, 76(t0)
	lw	s3, 80(t0)
	lw	s4, 84(t0)
	lw	s5, 88(t0)
	lw	s6, 92(t0)
	lw	s7, 96(t0)
	lw	s8, 100(t0)
  lw	s9, 104(t0)
  lw	s10, 108(t0)
	lw	s11, 112(t0)
	lw	a0, 116(t0)
	lw	a1, 120(t0)
	lw	a2, 124(t0)
	lw	a3, 128(t0)
	lw	a4, 132(t0)
	lw	a5, 136(t0)
	lw	a6, 140(t0)
	lw	a7, 144(t0)

  lw t6, 0(t0)
  csrrw zero,CSR_ENTRYHI,t6


  lw t6, 4(t0)
  csrrw zero,mcause,t6

	/* This is for avoiding trouble if STATUS has (erroneously)
	 * set bit 0/1. It would cause an immediate EXC trap or expose
	 * BIOS to interrupts. */
  lw t6, 8(t0)
	srl 	t6,t6, 4 
	sll 	t6,t6, 4 
  csrrw zero,mstatus,t6

  // has to be the last one
  lw t6, 12(t0) 
  csrrw zero,mepc,t6
  WRITEK1(t6)

  lw t6, 16(t0)
  csrrw zero,mie,t6

  READK0(t0)
	lw	t6, 64(t0)
	lw	t0, 40(t0)

  READK1(t6)

	mret

	.end	jios
	.size	jios, . - bios
