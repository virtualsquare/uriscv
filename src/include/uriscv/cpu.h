/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Mauro Morsiani
 * Copyright (C) 2020 Mattia Biondi
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#ifndef URISCV_CPU_H
#define URISCV_CPU_H

#define CAUSE_EXCCODE_MASK 0x0000007c
#define CAUSE_EXCCODE_BIT 2
#define CAUSE_GET_EXCCODE(x) (((x)&CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_BIT)

/* Exception codes as described in RISCV manual */
#define EXC_IAM 0  /* instruction address misaligned */
#define EXC_IAF 1  /* instruction access fault */
#define EXC_II 2   /* illegal instruction */
#define EXC_BP 3   /* breakpoint */
#define EXC_LAM 4  /* load address misaligned */
#define EXC_LAF 5  /* load address fault */
#define EXC_SAM 6  /* store address misaligned */
#define EXC_SAF 7  /* store access fault */
#define EXC_ECU 8  /* environment call from U-mode */
#define EXC_ECS 9  /* environment call from S-mode */
#define EXC_ECM 11 /* environment call from M-mode */
#define EXC_IPF 12 /* instruction page fault */
#define EXC_LPF 13 /* load page fault */
#define EXC_SPF 15 /* store page fault */

#define EXC_MOD 24   /* tbl bit d enable */
#define EXC_TLBL 25  /* tlb load fault */
#define EXC_TLBS 26  /* tlb store fault */
#define EXC_UTLBL 27 /* user tlb load fault */
#define EXC_UTLBS 28 /* user tlb store fault */

#define STATUS_IM_MASK 0x0000ff00
#define STATUS_IM(line) (1U << (8 + (line)))
#define STATUS_IM_BIT(line) (8 + (line))

#define CAUSE_IS_INT(cause) (cause & 0x80000000)

/* #define CAUSE_IP_MASK 0x0000ff00 */
#define CAUSE_IP(line) (1U << (line))
#define CAUSE_IP_BIT(line) (line)

#define CAUSE_CE_MASK 0x30000000
#define CAUSE_CE_BIT 28
#define CAUSE_GET_CE(x) (((x)&CAUSE_CE_MASK) >> CAUSE_CE_BIT)

#define CAUSE_BD 0x80000000
#define CAUSE_BD_BIT 31

#define ENTRYHI_SEGNO_MASK 0xc0000000
#define ENTRYHI_SEGNO_BIT 30
#define ENTRYHI_GET_SEGNO(x) (((x)&ENTRYHI_SEGNO_MASK) >> ENTRYHI_SEGNO_BIT)

#define ENTRYHI_VPN_MASK 0x3ffff000
#define ENTRYHI_VPN_BIT 12
#define ENTRYHI_GET_VPN(x) (((x)&ENTRYHI_VPN_MASK) >> ENTRYHI_VPN_BIT)

#define ENTRYHI_ASID_MASK 0x00000fc0
#define ENTRYHI_ASID_BIT 6
#define ENTRYHI_GET_ASID(x) (((x)&ENTRYHI_ASID_MASK) >> ENTRYHI_ASID_BIT)

#define ENTRYLO_PFN_MASK 0xfffff000
#define ENTRYLO_PFN_BIT 12
#define ENTRYLO_GET_PFN(x) (((x)&ENTRYLO_PFN_MASK) >> ENTRYLO_PFN_BIT)

#define ENTRYLO_DIRTY 0x00000400
#define ENTRYLO_DIRTY_BIT 10
#define ENTRYLO_VALID 0x00000200
#define ENTRYLO_VALID_BIT 9
#define ENTRYLO_GLOBAL 0x00000100
#define ENTRYLO_GLOBAL_BIT 8

#endif
