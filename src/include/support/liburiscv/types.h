#ifndef URISCV_TYPES_H
#define URISCV_TYPES_H

#include <stdint.h>

typedef uint8_t Byte;
typedef uint16_t HalfWord;
typedef uint32_t Word;

typedef int32_t SWord;

typedef struct csr_t {
  Word value;
  /*  UUSSMM
   *  U - user mode
   *  S - supervisor mode
   *  M - machine/kernel mode
   *  00 - no permission
   *  01 - read only
   *  10 - read/write
   */
  uint8_t perm;
} csr_t;

enum exception_t {
  EXC_OK = -1,
  EXC_INSTR_ADDR_MIS = 0,
  EXC_INSTR_ACC_FAULT = 1,
  EXC_ILL_INSTR = 2,
  EXC_BREAKPOINT = 3,
  EXC_LOAD_ADDR_MIS = 4,
  EXC_LOAD_ACC_FAULT = 5,
  EXC_MEM_OPER_MIS = 6,
  EXC_MEM_OPER_ACC_FAULT = 7,
  EXC_ENV_CALL_U = 8,
  EXC_ENV_CALL_S = 9,
  EXC_ENV_CALL_H = 10, // not used
  EXC_ENV_CALL_M = 11,
};

enum interrupt_t {
  INT_NONE = -1,
  INT_USIP = 0,
  INT_SSIP = 1,
  INT_HSIP = 2, // not used
  INT_MSIP = 3,
  INT_UTIP = 4,
  INT_STIP = 5,
  INT_HTIP = 6,
  INT_MTIP = 7,
  INT_UEIP = 8,
  INT_SEIP = 9,
  INT_HEIP = 10,
  INT_MEIP = 11,
};

/* Device register type for disks, flash devices and printers (dtp) */
typedef struct dtpreg {
  unsigned int status;
  unsigned int command;
  unsigned int data0;
  unsigned int data1;
} dtpreg_t;

/* Device register type for terminals */
typedef struct termreg {
  unsigned int recv_status;
  unsigned int recv_command;
  unsigned int transm_status;
  unsigned int transm_command;
} termreg_t;

typedef union devreg {
  dtpreg_t dtp;
  termreg_t term;
} devreg_t;

/* Bus register area */
typedef struct devregarea {
  unsigned int rambase;
  unsigned int ramsize;
  unsigned int execbase;
  unsigned int execsize;
  unsigned int bootbase;
  unsigned int bootsize;
  unsigned int todhi;
  unsigned int todlo;
  unsigned int intervaltimer;
  unsigned int timescale;
  unsigned int tlb_floor_addr;
  unsigned int inst_dev[5];
  unsigned int interrupt_dev[5];
  devreg_t devreg[5][8];
} devregarea_t;

/* Pass Up Vector */
typedef struct passupvector {
  unsigned int tlb_refill_handler;
  unsigned int tlb_refill_stackPtr;
  unsigned int exception_handler;
  unsigned int exception_stackPtr;
} passupvector_t;

#define STATE_GPR_LEN 29

/* Processor state */
typedef struct state {
  unsigned int entry_hi;
  unsigned int cause;
  unsigned int status;
  unsigned int pc_epc;
  unsigned int gpr[STATE_GPR_LEN];
  unsigned int hi;
  unsigned int lo;
} state_t;

#define reg_at gpr[0]
#define reg_v0 gpr[1]
#define reg_v1 gpr[2]
#define reg_a0 gpr[3]
#define reg_a1 gpr[4]
#define reg_a2 gpr[5]
#define reg_a3 gpr[6]
#define reg_t0 gpr[7]
#define reg_t1 gpr[8]
#define reg_t2 gpr[9]
#define reg_t3 gpr[10]
#define reg_t4 gpr[11]
#define reg_t5 gpr[12]
#define reg_t6 gpr[13]
#define reg_t7 gpr[14]
#define reg_s0 gpr[15]
#define reg_s1 gpr[16]
#define reg_s2 gpr[17]
#define reg_s3 gpr[18]
#define reg_s4 gpr[19]
#define reg_s5 gpr[20]
#define reg_s6 gpr[21]
#define reg_s7 gpr[22]
#define reg_t8 gpr[23]
#define reg_t9 gpr[24]
#define reg_gp gpr[25]
#define reg_sp gpr[26]
#define reg_fp gpr[27]
#define reg_ra gpr[28]
#define reg_HI gpr[29]
#define reg_LO gpr[30]

#define ETH_ALEN 6
#define ETH_PAYLOAD 1500

typedef struct packet {
  unsigned char dest[ETH_ALEN];
  unsigned char src[ETH_ALEN];
  unsigned char proto[2];
  unsigned char data[ETH_PAYLOAD];
  unsigned char dummy[2];
} packet_t;

#endif
