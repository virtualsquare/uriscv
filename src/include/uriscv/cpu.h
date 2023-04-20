#ifndef URISCV_CPU_H
#define URISCV_CPU_H

#define CAUSE_EXCCODE_MASK 0x0000007c
#define CAUSE_EXCCODE_BIT 2
#define CAUSE_GET_EXCCODE(x) (((x)&CAUSE_EXCCODE_MASK) >> CAUSE_EXCCODE_BIT)

/* Exception codes - naming follows standard MIPS mnemonics */
#define EXC_INT 0
#define EXC_MOD 1
#define EXC_TLBL 2
#define EXC_TLBS 3
#define EXC_ADEL 4
#define EXC_ADES 5
#define EXC_IBE 6
#define EXC_DBE 7
#define EXC_SYS 8
#define EXC_BP 9
#define EXC_RI 10
#define EXC_CPU 11
#define EXC_OV 12

#define EXC_UTLBL 13
#define EXC_UTLBS 14

#define STATUS_IM_MASK 0x0000ff00
#define STATUS_IM(line) (1U << (8 + (line)))
#define STATUS_IM_BIT(line) (8 + (line))

#define CAUSE_IP_MASK 0x0000ff00
#define CAUSE_IP(line) (1U << (8 + (line)))
#define CAUSE_IP_BIT(line) (8 + (line))

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
