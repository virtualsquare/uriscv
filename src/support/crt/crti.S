/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2011 Tomislav Jonjic
 * Copyright (C) 2023 Gianmaria Rovelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * This module contains startup code for non-PIC o32 .aout
 * executables.
 *
 * PLEASE NOTE:
 *
 * - The stack pointer ($sp) must be initialized by the kernel!
 *
 * - The TERMINATE_SYSCALL (see below) must be set to the appropriate
 *   code, if the program is ever expected to return from main().
 */

#include <uriscv/aout.h>

#define TEXT_VADDR 0x80000000

/* Set this to a suitable value */
#define TERMINATE_SYSCALL 0

	.text
	.align	2

	.globl	__start
	.type	__start, @function
	.extern	main
__start:

	li	t0, TEXT_VADDR + (AOUT_HE_GP_VALUE << 2)
	lw	gp, 0(t0)

	/* Call main */
	addi	sp, sp, -16
	jal	main
	addi	sp, sp, 16

	li	a0, TERMINATE_SYSCALL
  ecall

	/* Not reached! */

	.end	__start
	.size	__start, .-__start
