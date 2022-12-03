#ifndef URISCV_CONST_H
#define URISCV_CONST_H

// CSRs permissions
#define NNR 0x1
#define NNW 0x2
#define NWW 0xA
#define RRR 0x15
#define WWW 0x2A

#define NUM_HARTS 2
#define MODE_USER 0
#define MODE_SUPERVISOR 1
#define MODE_MACHINE 2

#define CYCLE 0xC00 // Clock	cycle	counter
#define MCYCLE 0xB00
#define CYCLEH 0xC80 // Upper half of cycle (RV32 only)
#define MCYCLEH 0xB80
#define TIME 0xC01    // Current time in ticks
#define TIMEH 0xC81   // Upper half of time (RV32 only)
#define INSTRET 0xC02 // Number of instructions retired
#define MINSTRET 0xB02
#define INSTRETH 0xC82 // Upper half of instret (RV32 only)
#define MINSTRETH 0xB82
#define UTVEC 0x005   // Trap	handler	base	address
#define SEDELEG 0x102 // Exception	delegation	register
#define MEDELEG 0x302
#define SIDELEG 0x103 // Interrupt delegation	register
#define MIDELEG 0x303
#define STVEC 0x105
#define MTVEC 0x305
#define USCRATCH 0x40 // Previous value of PC
#define SSCRATCH 0x140
#define MSCRATCH 0x340
#define UEPC 0x41 // Previous value of PC
#define SEPC 0x141
#define MEPC 0x341
#define UCAUSE 0x42 // Trap cause code
#define SCAUSE 0x142
#define MCAUSE 0x342
#define UTVAL 0x43 // Bad	address	or	bad	instruction
#define STVAL 0x143
#define MTVAL 0x343

// general configuration constants
#define MPSFILETYPE ".umps"
#define AOUTFILETYPE ".aout.umps"
#define BIOSFILETYPE ".rom.umps"
#define COREFILETYPE ".core.umps"
#define STABFILETYPE ".stab.umps"

// maximum area size for trace ranges: a little more than 4KB
// to avoid troubles in browser refresh if area is too large
#define MAXTRACESIZE (FRAMESIZE + 1)

/***************************************************************************/

// no more user-serviceable parts below this line

// some utility constants
#define HIDDEN static

#define EOS '\0'
#define EMPTYSTR ""
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

// host specific constants
#ifdef WORDS_BIGENDIAN
#define BIGENDIANCPU 1
#else
#define BIGENDIANCPU 0
#endif

// hardware constants

// physical memory page frame size (in words)
#define FRAMESIZE 1024

// KB per frame
#define FRAMEKB 4

// block device size in words
#define BLOCKSIZE FRAMESIZE

// eth packet size
#define PACKETSIZE 1514

// DMA transfer time
#define DMATICKS BLOCKSIZE

// miscellaneous MIPS alignment and size definitions needed by modules
// other by processor.cc

// number of ASIDs
#define MAXASID 64

// MIPS NOP instruction
#define NOP 0UL

// word length in bytes, byte length in bits, sign masks, etc.
#define WORDLEN 4
#define BYTELEN 8
#define WORDSHIFT 2
#define MINWORDVAL 0x00000000UL
#define MAXWORDVAL 0xFFFFFFFFUL
#define MAXSWORDVAL 0x7FFFFFFFUL
#define SIGNMASK 0x80000000UL
#define BYTEMASK 0x000000FFUL

// immediate/lower halfword part mask
#define IMMMASK 0x0000FFFFUL

// word alignment mask
#define ALIGNMASK 0x00000003UL

// halfword bit length
#define HWORDLEN 16

// exception type constants (simulator internal coding)
#define NOEXCEPTION 0
#define INTEXCEPTION 1
#define MODEXCEPTION 2
#define UTLBLEXCEPTION 3
#define TLBLEXCEPTION 4
#define UTLBSEXCEPTION 5
#define TLBSEXCEPTION 6
#define ADELEXCEPTION 7
#define ADESEXCEPTION 8
#define DBEXCEPTION 9
#define IBEXCEPTION 10
#define SYSEXCEPTION 11
#define BPEXCEPTION 12
#define RIEXCEPTION 13
#define CPUEXCEPTION 14
#define OVEXCEPTION 15

// interrupt handling related constants

// timer interrupt line
#define TIMERINT 2

// device starting interrupt line
#define DEVINTBASE 3

// device register length
#define DEVREGLEN 4

// interrupts available for registers
#define DEVINTUSED 5

// devices per interrupt line
#define DEVPERINT 8

// segments base addresses
#define KSEG0BASE 0x00000000UL
#define KSEG0TOP 0x20000000UL
#define KUSEGBASE 0x80000000UL

#define CORE_TEXT_VADDR 0x20001000

// bus memory mapping constants (BIOS/BIOS Data Page/device registers/BOOT/RAM)
#define BIOSBASE 0x00000000UL
#define BIOSDATABASE 0x0FFFF000UL
#define DEVBASE 0x10000000UL
#define BOOTBASE 0x1FC00000UL
#define RAMBASE 0x20000000UL

// size of bios data page (in words)
#define BIOSDATASIZE ((DEVBASE - BIOSDATABASE) / WORDLEN)

// Processor structure register numbers
#define CPUREGNUM 34
#define CPUGPRNUM 32
#define CP0REGNUM 10

// device type codes
#define NULLDEV 0
#define DISKDEV 1
#define FLASHDEV 2
#define ETHDEV 3
#define PRNTDEV 4
#define TERMDEV 5

// interrupt line offset used for terminals
// (lots of code must be modified if this changes)

#define TERMINT 4

// memory access types for brkpt/susp/trace ranges in watch.cc and appforms.cc
// modules
#define READWRITE 0x6
#define READ 0x4
#define WRITE 0x2
#define EXEC 0x1
#define EMPTY 0x0

#define REG_ZERO 0
#define REG_RA 1
#define REG_SP 2
#define REG_GP 3
#define REG_TP 4
#define REG_T0 5
#define REG_T1 6
#define REG_T2 7
#define REG_T3 28
#define REG_T4 29
#define REG_T5 30
#define REG_T6 31
#define REG_S0 8
#define REG_S1 9
#define REG_S2 18
#define REG_S3 19
#define REG_S4 20
#define REG_S5 21
#define REG_S6 22
#define REG_S7 23
#define REG_S8 24
#define REG_S9 25
#define REG_S10 26
#define REG_S11 27
#define REG_FP 8
#define REG_A0 10
#define REG_A1 11
#define REG_A2 12
#define REG_A3 13
#define REG_A4 14
#define REG_A5 15
#define REG_A6 16
#define REG_A7 17

// some useful macros

// recognizes bad (unaligned) virtual address
#define BADADDR(w) ((w & ALIGNMASK) != 0UL)

// returns the sign bit of a word
#define SIGNBIT(w) (w & SIGNMASK)

// returns 1 if the two strings are equal, 0 otherwise
#define SAMESTRING(s, t) (strcmp(s, t) == 0)

// returns 1 if a is in open-ended interval [b, c[, 0 otherwise
#define INBOUNDS(a, b, c) (a >= b && a < c)

#endif
