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

#endif
