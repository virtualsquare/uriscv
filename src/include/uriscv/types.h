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

#endif
