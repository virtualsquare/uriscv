/*
 * SPDX-FileCopyrightText: 2010, 2011 Tomislav Jonjic
 * SPDX-FileCopyrightText: 2020 Mattia Biondi
 * SPDX-FileCopyrightText: 2020 Mikey Goldweber
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * uRISCV machine-specific constants, most notably bus & device memory
 * mapped register addresses.
 *
 * IMPORTANT: Keep this header assembler-safe!
 */

#ifndef URISCV_ARCH_H
#define URISCV_ARCH_H

/*
 * Generalities
 */
#define WORD_SIZE 4
#define WS WORD_SIZE

#define MMIO_BASE 0x10000000
#define RAM_BASE 0x20000000

/* Segment-related constants */
#define KSEG0_BASE 0x00000000
#define KSEG0_BIOS_BASE 0x00000000
#define KSEG0_BOOT_BASE 0x1FC00000
#define KUSEG_BASE 0x80000000

/* Device register size */
#define DEV_REG_SIZE_W 4
#define DEV_REG_SIZE (DEV_REG_SIZE_W * WS)

/*
 * Interrupt lines
 */

#define N_INTERRUPT_LINES 8

#define N_IL N_INTERRUPT_LINES

/* Number of interrupt lines available to devices */
#define N_EXT_IL 5

/* Devices per interrupt line */
#define N_DEV_PER_IL 8

#define DEV_IL_START 17

#define IL_TIMER 3
#define IL_CPUTIMER 7

#define IL_IPI 16
#define IL_DISK 17
#define IL_FLASH 18
#define IL_ETHERNET 19
#define IL_PRINTER 20
#define IL_TERMINAL 21

#define EXT_IL_INDEX(il) ((il)-IL_DISK)

/*
 * Bus and device register definitions
 *
 * Device interrupt lines are identified by the range [3, 7],
 * i.e. their repsective physical interrupt lines. Keep this in mind
 * when using the macros below. This is slightly confusing, but so is
 * any alternative.
 */

/* Bus register space */
#define BUS_REG_RAM_BASE 0x10000000
#define BUS_REG_RAM_SIZE 0x10000004
#define BUS_REG_BIOS_BASE 0x10000008
#define BUS_REG_BIOS_SIZE 0x1000000C
#define BUS_REG_BOOT_BASE 0x10000010
#define BUS_REG_BOOT_SIZE 0x10000014
#define BUS_REG_TOD_HI 0x10000018
#define BUS_REG_TOD_LO 0x1000001C
#define BUS_REG_TIMER 0x10000020
#define BUS_REG_TIME_SCALE 0x10000024

/* TLB floor address */
#define TLB_FLOOR_ADDR 0x10000028

/* Installed devices bitmap */
#define IDEV_BITMAP_BASE 0x1000002C
#define IDEV_BITMAP_END (IDEV_BITMAP_BASE + N_EXT_IL * WS)
#define IDEV_BITMAP_ADDR(line) (IDEV_BITMAP_BASE + ((line)-DEV_IL_START) * WS)

/* Interrupting devices bitmap */
#define CDEV_BITMAP_BASE 0x10000040
#define CDEV_BITMAP_END (CDEV_BITMAP_BASE + N_EXT_IL * WS)
#define CDEV_BITMAP_ADDR(line) (CDEV_BITMAP_BASE + ((line)-DEV_IL_START) * WS)

/* Device register area */
#define DEV_REG_START 0x10000054
#define DEV_REG_ADDR(line, dev)                                                \
  (DEV_REG_START + ((line)-DEV_IL_START) * N_DEV_PER_IL * DEV_REG_SIZE +       \
   (dev)*DEV_REG_SIZE)

/* End of memory mapped external device registers area */
#define DEV_REG_END (DEV_REG_START + N_EXT_IL * N_DEV_PER_IL * DEV_REG_SIZE)

/*
 * Interrupt Routing Table (IRT)
 */
#define IRT_BASE 0x10000300
#define IRT_END                                                                \
  0x100003c0 /* (IRT_BASE + (N_EXT_IL + 1) * N_DEV_PER_IL * WS)                \
              */

#define IRT_ENTRY(line, dev)                                                   \
  (IRT_BASE + WS * (((line)-IL_TIMER) * N_DEV_PER_IL + dev))

#define IRT_ENTRY_POLICY_MASK 0x10000000
#define IRT_ENTRY_POLICY_BIT 28
#define IRT_ENTRY_GET_POLICY(x)                                                \
  (((x)&IRT_ENTRY_POLICY_MASK) >> IRT_ENTRY_POLICY_BIT)

#define IRT_ENTRY_DEST_MASK 0x0000ffff
#define IRT_ENTRY_DEST_BIT 0
#define IRT_ENTRY_GET_DEST(x) (((x)&IRT_ENTRY_DEST_MASK) >> IRT_ENTRY_DEST_BIT)

/* Interrupt routing policies */
#define IRT_POLICY_FIXED 0
#define IRT_POLICY_DYNAMIC 1

/*
 * Int. controller cpu inteface (banked) register set
 */
#define CPUCTL_INBOX 0x10000400

#define CPUCTL_INBOX_MSG_MASK 0x000000ff
#define CPUCTL_INBOX_MSG_BIT 0
#define CPUCTL_INBOX_GET_MSG(x)                                                \
  (((x)&CPUCTL_INBOX_MSG_MASK) >> CPUCTL_INBOX_MSG_BIT)

#define CPUCTL_INBOX_ORIGIN_MASK 0x00000f00
#define CPUCTL_INBOX_ORIGIN_BIT 8
#define CPUCTL_INBOX_GET_ORIGIN(x)                                             \
  (((x)&CPUCTL_INBOX_ORIGIN_MASK) >> CPUCTL_INBOX_ORIGIN_BIT)

#define CPUCTL_OUTBOX 0x10000404

#define CPUCTL_OUTBOX_MSG_MASK 0x000000ff
#define CPUCTL_OUTBOX_MSG_BIT 0
#define CPUCTL_OUTBOX_GET_MSG(x)                                               \
  (((x)&CPUCTL_OUTBOX_MSG_MASK) >> CPUCTL_OUTBOX_MSG_BIT)

#define CPUCTL_OUTBOX_RECIP_MASK 0x00ffff00
#define CPUCTL_OUTBOX_RECIP_BIT 8
#define CPUCTL_OUTBOX_GET_RECIP(x)                                             \
  (((x)&CPUCTL_OUTBOX_RECIP_MASK) >> CPUCTL_OUTBOX_RECIP_BIT)

#define CPUCTL_TPR 0x10000408

#define CPUCTL_TPR_PRIORITY_MASK 0x0000000f

#define CPUCTL_BIOS_RES_0 0x1000040c
#define CPUCTL_BIOS_RES_1 0x10000410

#define CPUCTL_BASE CPUCTL_INBOX
#define CPUCTL_END (CPUCTL_BIOS_RES_1 + WS)

/*
 * Machine control registers
 */
#define MCTL_NCPUS 0x10000500

#define MCTL_RESET_CPU 0x10000504
#define MCTL_RESET_CPU_CPUID_MASK 0x0000000f

/* Reset vector and initial $sp */
#define MCTL_BOOT_PC 0x10000508
#define MCTL_BOOT_SP 0x1000050c

#define MCTL_DEFAULT_BOOT_PC 0x1fc00000
#define MCTL_DEFAULT_BOOT_SP 0x00000000

#define MCTL_HALT_CPU 0x10000510
#define MCTL_POWER 0x10000514

#define MCTL_BASE MCTL_NCPUS
#define MCTL_END (MCTL_POWER + WS)

#define MMIO_END MCTL_END

#endif /* !defined(URISCV_ARCH_H) */
