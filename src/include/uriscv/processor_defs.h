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

/****************************************************************************
 *
 * This header file contains constant & macro definitions for processor.cc
 * and disassemble.cc program modules.
 *
 * All names, bit positions and masks follow MIPS register formats: see
 * documentation.
 *
 * Instruction opcodes too may be retrieved from appropriate documentation:
 * codes are in octal format for easier coding by programmer.
 *
 ****************************************************************************/

// virtual exception handling base address
#define BOOTEXCBASE (BOOTBASE + 0x100UL)

// exception virtual offsets
#define TLBREFOFFS 0x000UL
#define OTHEREXCOFFS 0x080UL

// CPUEXCEPTION handling constant
#define COPEOFFSET 27

// used to decode TLBCLR instruction
#define CONTEXTREG 4

// and corresponding register index in Processor structure

// STATUS and CAUSE interrupt area mask
#define INTMASK 0x0000FF00UL

// TLB EntryHI handling masks and constants
#define VPNMASK 0xFFFFF000UL
#define ASIDMASK 0x00000FC0UL
#define ASIDOFFS 6
#define OFFSETMASK (~(VPNMASK))

// TLB EntryLO bit positions and mask
#define GBITPOS 8
#define VBITPOS 9
#define DBITPOS 10
#define ENTRYLOMASK 0xFFFFFF00UL

// RANDOM and INDEX register handling constants
#define RNDIDXOFFS 8
#define RANDOMBASE (1UL << RNDIDXOFFS)
#define RANDOMSTEP (1UL << RNDIDXOFFS)

// STATUS register handling constants:

// at reset, are set to 1: cop0 (bit 28), BEV (bit 22)
// all other bits are set to 0 (interrupts disabled, etc.)
#define STATUSRESET 0x10400000UL

// this mask forbids changes to some bits (CU3->1 usable bits, all DS field
// except BEV bit, 0 fixed parts)
#define STATUSMASK 0x1F40FF3FUL

// STATUS kernel/user, interrupt enable
#define KUOBITPOS 5
#define IEOBITPOS 4
#define KUCBITPOS 1
#define IECBITPOS 0

// instruction decode field masks and constants

#define OPTYPEMASK 0xE0000000UL
#define OPCODEMASK 0xFC000000UL
#define FUNCTMASK 0x0000003FUL
#define RSMASK 0x03E00000UL
#define RTMASK 0x001F0000UL
#define RDMASK 0x0000F800UL
#define SHAMTMASK 0x000007C0UL
#define PCUPMASK 0xF0000000UL
#define COPTYPEMASK 0x03E00000UL
#define COPCODEMASK 0x03FF0000UL
#define COPNUMMASK 0x0C000000UL
#define CALLMASK 0x03FFFFC0UL
#define IMMSIGNPOS 15
#define DWCOPBITPOS 28

#define LINKREG 31

#define OPCODEOFFS 26
#define RSOFFSET 21
#define RTOFFSET 16
#define RDOFFSET 11
#define SHAMTOFFS 6
#define COPTYPEOFFS 21
#define COPCODEOFFS 16

// instruction main opcodes

#define REGTYPE 077
#define IMMTYPE 010
#define COPTYPE 020
#define BRANCHTYPE 000
#define LOADTYPE 040
#define LOADCOPTYPE 060
#define STORECOPTYPE 070
#define STORETYPE 050

// REGTYPE (Special) function opcodes

#define SFN_ADD 040
#define SFN_ADDU 041
#define SFN_AND 044
#define SFN_BREAK 015
#define SFN_DIV 032
#define SFN_DIVU 033
#define SFN_JALR 011
#define SFN_JR 010
#define SFN_MFHI 020
#define SFN_MFLO 022
#define SFN_MTHI 021
#define SFN_MTLO 023
#define SFN_MULT 030
#define SFN_MULTU 031
#define SFN_NOR 047
#define SFN_OR 045
#define SFN_SLL 000
#define SFN_SLLV 004
#define SFN_SLT 052
#define SFN_SLTU 053
#define SFN_SRA 003
#define SFN_SRAV 007
#define SFN_SRL 002
#define SFN_SRLV 006
#define SFN_SUB 042
#define SFN_SUBU 043
#define SFN_SYSCALL 014
#define SFN_XOR 046
#define SFN_CAS 013

// IMMCOMPTYPE opcodes

#define ADDI 010
#define ADDIU 011
#define ANDI 014
#define LUI 017
#define ORI 015
#define SLTI 012
#define SLTIU 013
#define XORI 016

// BRANCHTYPE opcodes

#define BEQ 004

#define BGL 001
#define BGEZ 001
#define BGEZAL 021
#define BLTZ 000
#define BLTZAL 020

#define BGTZ 007
#define BLEZ 006
#define BNE 005
#define J 002
#define JAL 003

// COPTYPE opcodes

// detects if instruction refers to CP0
#define COP0SEL 020

#define CO0 020
#define RFE 020
#define TLBP 010
#define TLBR 001
#define TLBWI 002
#define TLBWR 006
#define COFUN_WAIT 040

#define BC0 010
#define BC0F 0400
#define BC0T 0401

#define MFC0 0
#define MTC0 04

// LOADTYPE opcodes

#define LB 040
#define LBU 044
#define LH 041
#define LHU 045
#define LW 043
#define LWL 042
#define LWR 046

// STORETYPE opcodes

#define SB 050
#define SH 051
#define SW 053
#define SWL 052
#define SWR 056

// LOADCOPTYPE opcodes
#define LWC0 060

// STORECOPTYPE opcodes
#define SWC0 070

// byte sign extension mask
#define BYTESIGNMASK 0x0000007FUL

// useful macros

// extracts VPN from address
#define VPN(w) ((w & VPNMASK))

// extracts ASID from address
#define ASID(w) ((w & ASIDMASK))

// applies interrupt mask to STATUS/CAUSE register
#define IM(r) ((r & INTMASK))

// aligns a virtual address (clears lowest bits)
#define ALIGN(va) (va & ~(ALIGNMASK))

// computes physical address from virtual address and PFN field
#define PHADDR(va, pa) ((va & OFFSETMASK) | (pa & VPNMASK))

// detects which byte is referred by a LB/LBU/LWL/LWR/SB/SWL/SWR instruction
#define BYTEPOS(va) (va & ALIGNMASK)

// detects which halfword is referred by a LH/LHU/SH instruction
#define HWORDPOS(va) ((va & ALIGNMASK) >> 1)

// instruction fields decoding macros
// #define OPCODE(i) ((i & OPCODEMASK) >> OPCODEOFFS)
// #define FUNCT(i) (i & FUNCTMASK)
// #define SHAMT(i) ((i & SHAMTMASK) >> SHAMTOFFS)
// #define REGSHAMT(r) (r & (SHAMTMASK >> SHAMTOFFS))
// #define RS(i) ((i & RSMASK) >> RSOFFSET)
// #define RT(i) ((i & RTMASK) >> RTOFFSET)
// #define RD(i) ((i & RDMASK) >> RDOFFSET)
#define OP_L 0x3
#define OP_LB 0x0
#define OP_LH 0x1
#define OP_LW 0x2
#define OP_LBU 0x4
#define OP_LHU 0x5

#define I_TYPE 0x13
#define OP_ADDI 0x0
#define INSTR_NOP 0x00000013
#define OP_SLLI 0x1
#define OP_SLTI 0x2
#define OP_SLTIU 0x3
#define OP_XORI 0x4
#define OP_SR 0x05
#define OP_SRLI_FUNC7 0x00
#define OP_SRAI_FUNC7 0x20
#define OP_ORI 0x6
#define OP_ANDI 0x7

#define I2_TYPE 0x73
#define OP_ECALL_EBREAK 0x0
#define ECALL_IMM 0x0
#define EBREAK_IMM 0x1
#define MRET_IMM 0x302
#define EWFI_IMM 0x105
#define OP_CSRRW 0x1
#define OP_CSRRS 0x2
#define OP_CSRRC 0x3
#define OP_CSRRWI 0x5
#define OP_CSRRSI 0x6
#define OP_CSRRCI 0x7
#define R_TYPE 0x33

#define OP_MUL_FUNC7 0x1
#define OP_MUL_FUNC3 0x0
#define OP_MULH_FUNC3 0x1
#define OP_MULH_FUNC7 0x1
#define OP_MULHSU_FUNC3 0x2
#define OP_MULHSU_FUNC7 0x1
#define OP_MULHU_FUNC3 0x3
#define OP_MULHU_FUNC7 0x1
#define OP_DIV_FUNC3 0x4
#define OP_DIV_FUNC7 0x1
#define OP_DIVU_FUNC3 0x5
#define OP_DIVU_FUNC7 0x1
#define OP_REM_FUNC3 0x6
#define OP_REM_FUNC7 0x1
#define OP_REMU_FUNC3 0x7
#define OP_REMU_FUNC7 0x1

#define OP_ADD_FUNC3 0x0
#define OP_ADD_FUNC7 0x0
#define OP_SUB_FUNC3 0x0
#define OP_SUB_FUNC7 0x20
#define OP_SLL_FUNC3 0x1
#define OP_SLL_FUNC7 0x0
#define OP_SLT_FUNC3 0x2
#define OP_SLT_FUNC7 0x0
#define OP_SLTU_FUNC3 0x3
#define OP_SLTU_FUNC7 0x0
#define OP_XOR_FUNC3 0x4
#define OP_XOR_FUNC7 0x0
#define OP_SRL_FUNC3 0x5
#define OP_SRL_FUNC7 0x0
#define OP_SRA_FUNC3 0x5
#define OP_SRA_FUNC7 0x20
#define OP_OR_FUNC3 0x6
#define OP_OR_FUNC7 0x0
#define OP_AND_FUNC3 0x7
#define OP_AND_FUNC7 0x0

#define B_TYPE 0x63
#define OP_BEQ 0x0
#define OP_BNE 0x1
#define OP_BLT 0x4
#define OP_BGE 0x5
#define OP_BLTU 0x6
#define OP_BGEU 0x7

#define S_TYPE 0x23
#define OP_SB 0x0
#define OP_SH 0x1
#define OP_SW 0x2

#define OP_AUIPC 0x17
#define OP_LUI 0x37
#define OP_JAL 0x6F
#define OP_JALR 0x67
#define INSTR_RET 0x00008067

#define OP_FENCE 0xF
#define OP_FENCE_TSO 0x8330000F
#define OP_PAUSE 0x100000F
#define FENCE_PERM(instr) ((instr >> 20) & 0xFF)
#define FENCE_SUCC(instr) ((instr >> 20) & 0xF)
#define FENCE_PRED(instr) ((instr >> 24) & 0xF)
#define FENCE_PI(instr) (FENCE_PRED(instr) & 0x1)
#define FENCE_PO(instr) ((FENCE_PRED(instr) >> 1) & 0x1)
#define FENCE_PR(instr) ((FENCE_PRED(instr) >> 2) & 0x1)
#define FENCE_PW(instr) ((FENCE_PRED(instr) >> 3) & 0x1)
#define FENCE_SI(instr) (FENCE_SUCC(instr) & 0x1)
#define FENCE_SO(instr) ((FENCE_SUCC(instr) >> 1) & 0x1)
#define FENCE_SR(instr) ((FENCE_SUCC(instr) >> 2) & 0x1)
#define FENCE_SW(instr) ((FENCE_SUCC(instr) >> 3) & 0x1)

#define OP_FLOAD 0x7
#define OP_FLW_FUNC3 0x2
#define OP_FLD_FUNC3 0x3
#define OP_FSAVE 0x27
#define OP_FSW_FUNC3 0x2
#define OP_FSD_FUNC3 0x3
#define OP_FMADDS 0x43
#define OP_FMSUBS 0x47
#define OP_FNMSUBS 0x4B
#define OP_FNMADDS 0x4F
#define OP_FLOAT_OP 0x53
#define OP_FADDS_FUNC7 0x0
#define OP_FADDD_FUNC7 0x1
#define OP_FSUBS_FUNC7 0x4
#define OP_FSUBD_FUNC7 0x5
#define OP_FMULS_FUNC7 0x8
#define OP_FMULD_FUNC7 0x9
#define OP_FDIVS_FUNC7 0xC
#define OP_FDIVD_FUNC7 0xD
#define OP_FSQRTS_FUNC7 0x2C
#define OP_FSQRTD_FUNC7 0x2D
#define OP_FSGNJS_FUNC7 0x10
#define OP_FSGNJD_FUNC7 0x11
#define OP_FSGNJ_FUNC3 0x0
#define OP_FSGNJN_FUNC3 0x1
#define OP_FSGNJX_FUNC3 0x2
#define OP_FMINMAXS_FUNC7 0x14
#define OP_FMINMAXD_FUNC7 0x15
#define OP_FMIN_FUNC3 0x0
#define OP_FMAX_FUNC3 0x1
#define OP_FCVTWS_FUNC7 0x60
#define OP_FCVTWD_FUNC7 0x61
#define OP_FCVTWS_FUNCRS2 0x0
#define OP_FCVTWUS_FUNCRS2 0x1
#define OP_FCVTWD_FUNCRS2 0x0
#define OP_FCVTWUD_FUNCRS2 0x1
#define OP_FMVCLASSS_FUNC7 0x70
#define OP_FCLASSD_FUNC7 0x71
#define OP_FMVXW_FUNC3 0x0
#define OP_FCLASSS_FUNC3 0x1
#define OP_FCOMPARES_FUNC7 0x50
#define OP_FCOMPARED_FUNC7 0x51
#define OP_FEQS_FUNC3 0x2
#define OP_FLTS_FUNC3 0x1
#define OP_FLES_FUNC3 0x0
#define OP_FCVTSW_FUNC7 0x68
#define OP_FCVTSD_FUNC7 0x20
#define OP_FCVTDS_FUNC7 0x21
#define OP_FCVTDW_FUNC7 0x69
#define OP_FCVTSW_FUNCRS2 0x0
#define OP_FCVTSWU_FUNCRS2 0x1
#define OP_FCVTDW_FUNCRS2 0x0
#define OP_FCVTDWU_FUNCRS2 0x1
#define OP_FMVWX_FUNC7 0x78

#define OPCODE(instr) (instr & 0x7F)
#define RD(instr) ((instr >> 7) & 0x1F)
#define FUNC2(instr) ((instr >> 25) & 0x3)
#define FUNC3(instr) ((instr >> 12) & 0x7)
#define RS1(instr) ((instr >> 15) & 0x1F)
#define RS2(instr) ((instr >> 20) & 0x1F)
#define RS3(instr) ((instr >> 27) & 0x1F)
#define FUNC7(instr) ((instr >> 25) & 0x7F)

#define I_IMM_SIZE 12
#define I_IMM(instr) (instr >> 20)
#define B_IMM(instr)                                                           \
  (((instr & 0xF00) >> 7) | ((instr & 0x80) << 4) |                            \
   ((instr & 0x80000000) >> 19) | ((instr & 0x7E000000) >> 20))
#define U_IMM_SIZE 20
#define U_IMM(instr) (instr >> 12)
#define S_IMM_SIZE 12
#define S_IMM(instr) (RD(instr) | (FUNC7(instr) << 5))
#define J_IMM_SIZE 20
#define J_IMM(instr)                                                           \
  ((((instr >> 31) & 0x1) << 20) | (((instr >> 21) & 0x3FF) << 1)) |           \
      (((instr >> 20) & 0x1) << 11) | (((instr >> 12) & 0xFF) << 12)

#define SIGN_BIT(value, bits) (((value) & (1 << (bits - 1))) >> (bits - 1))
#define SIGN_EXTENSION(value, bits)                                            \
  (SIGN_BIT(value, bits) ? (((value) | (((1 << (32 - bits)) - 1)) << bits))    \
                         : value)

// coprocessor instructions decoding macros
#define COPOPCODE(i) ((i & COPCODEMASK) >> COPCODEOFFS)
#define COPOPTYPE(i) ((i & COPTYPEMASK) >> COPTYPEOFFS)
#define COPNUM(i) ((i & COPNUMMASK) >> OPCODEOFFS)

// computes jump target from PC and J/JAL instruction itself
#define JUMPTO(pc, i) ((pc & PCUPMASK) | ((i & ~(OPCODEMASK)) << WORDSHIFT))

// zero-extends a immediate (halfword) quantity
#define ZEXTIMM(i) (i & IMMMASK)

// computes index value from INDEX register format:
// SIGNMASK is used to clear bit 31, which marks a failed TLB probe
#define RNDIDX(id) ((id & ~(SIGNMASK)) >> RNDIDXOFFS)

// decodes BREAK/SYSCALL argument inside instruction itself
#define CALLVAL(i) ((i & CALLMASK) >> SHAMTOFFS)
