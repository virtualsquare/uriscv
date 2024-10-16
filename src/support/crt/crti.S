/*
 * SPDX-FileCopyrightText: 2011 Tomislav Jonjic
 * SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
