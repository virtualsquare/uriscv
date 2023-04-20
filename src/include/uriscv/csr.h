#ifndef URISCV_CSR_H
#define URISCV_CSR_H

/* CSRs permissions */
#define NNR 0x1
#define NNW 0x2
#define NWW 0xA
#define RRR 0x15
#define WWW 0x2A

#define NUM_HARTS 2
#define MODE_USER 0
#define MODE_SUPERVISOR 1
#define MODE_MACHINE 2

#define CYCLE 0xC00 /* Clock	cycle	counter */
#define MCYCLE 0xB00
#define CYCLEH 0xC80 /* Upper half of cycle (RV32 only) */
#define MCYCLEH 0xB80
#define TIME 0xC01    /* Current time in ticks */
#define TIMEH 0xC81   /* Upper half of time (RV32 only) */
#define INSTRET 0xC02 /* Number of instructions retired */
#define MINSTRET 0xB02
#define INSTRETH 0xC82 /* Upper half of instret (RV32 only) */
#define MINSTRETH 0xB82
#define UTVEC 0x005   /* Trap	handler	base	address */
#define SEDELEG 0x102 /* Exception	delegation	register */
#define MEDELEG 0x302
#define SIDELEG 0x103 /* Interrupt delegation	register */
#define MIDELEG 0x303
#define STVEC 0x105
#define MTVEC 0x305
#define USCRATCH 0x40 /* Previous value of PC */
#define SSCRATCH 0x140
#define MSCRATCH 0x340
#define UEPC 0x41 /* Previous value of PC */
#define SEPC 0x141
#define MEPC 0x341
#define UCAUSE 0x42 /* Trap cause code */
#define SCAUSE 0x142
#define MCAUSE 0x342
#define UTVAL 0x43 /* Bad	address	or	bad	instruction */
#define STVAL 0x143
#define MTVAL 0x343

#define MSTATUS 0x300

/* Custom CSR */
#define CSR_ENTRYLO 0x800
#define CSR_ENTRYHI 0x801
#define CSR_INDEX 0x802
#define CSR_RANDOM 0x803
#define CSR_BADVADDR 0x804

#endif
