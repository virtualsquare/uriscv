/*
 * SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef BIOS_H
#define BIOS_H

/*
 * BIOS services, invoked via break traps
 * FIXME: Are all of these safe to use (abuse?) as break codes?
 */
#define BIOS_SRV_LDCXT 0
#define BIOS_SRV_LDST 1
#define BIOS_SRV_PANIC 2
#define BIOS_SRV_HALT 3
#define BIOS_SRV_TLBWR 4
#define BIOS_SRV_TLBWI 5
#define BIOS_SRV_TLBP 6
#define BIOS_SRV_TLBR 7
#define BIOS_SRV_TLBCLR 8

/*
 * We use the BIOS-reserved registers as pointers to BIOS related data
 * structures (the exception vector and the PC/SP exception handlers)
 */
#define BIOS_EXCPT_VECT_BASE CPUCTL_BIOS_RES_0
#define BIOS_PC_AREA_BASE CPUCTL_BIOS_RES_1

/* BIOS Data Page base address */
#define BIOS_DATA_PAGE_BASE 0x0FFFF000
#define BIOS_EXEC_HANDLERS_ADDRS 0x0FFFF900

#endif /* BIOS_H */
