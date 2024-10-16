/*
 * SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "uriscv/arch.h"
#include "uriscv/aout.h"
#include "uriscv/bios.h"
#include "uriscv/aout.h"

#define CORE_TEXT_VADDR 0x20001000
#define KERNELSTACKSIZE 0x00001000


	.text
	.align	2

	.globl	__start
	.type	__start, @function
  /*
	.ent	__start
  */
	.extern	main
__start:
/*
	.frame	sp, 16, ra
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
  */

	/* computes stack bottom (end of first page of RAM)
	   using s0 and s1 (s0 and s1) */
  li t0, BUS_REG_RAM_BASE
	lw	s0, 0(t0)
	li	s1, KERNELSTACKSIZE
	add	s0, s0, s1
	addi	sp, s0, -8

	/* Initialize gp */
	li	gp, CORE_TEXT_VADDR + (AOUT_HE_GP_VALUE << 2)

	addi	sp, sp, -16
	jal	main
	addi	sp, sp, 16

/*
	.set	noreorder
	.set	nomacro
  */
	li 	a0, BIOS_SRV_HALT
	nop
	ebreak
	nop
  /*
	.set	reorder
	.set	macro
  */

	.end	__start
	.size	__start, .-__start
