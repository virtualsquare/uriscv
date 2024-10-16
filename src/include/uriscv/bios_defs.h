// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BIOS_DEFS_H
#define BIOS_DEFS_H

/*
 * BIOS services, invoked via break traps
 * FIXME: Are all of these safe to use (abuse?) as break codes?
 */
#define BIOS_SRV_LDCXT 0
#define BIOS_SRV_LDST 1
#define BIOS_SRV_PANIC 2
#define BIOS_SRV_HALT 3

/*
 * We use the BIOS-reserved registers as pointers to BIOS related data
 * structures (the exception vector and the PC/SP exception handlers)
 */
#define BIOS_EXCPT_VECT_BASE CPUCTL_BIOS_RES_0
#define BIOS_PC_AREA_BASE CPUCTL_BIOS_RES_1

/* BIOS Data Page base address */
#define BIOS_DATA_PAGE_BASE 0x0FFFF000
#define BIOS_EXEC_HANDLERS_ADDRS 0x0FFFF900

#endif /* BIOS_DEFS_H */
