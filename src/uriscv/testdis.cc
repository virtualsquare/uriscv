#include <stdio.h>

#include "uriscv/disassemble.h"

// Not working B instructions
const unsigned int brokenBInstrs[] = {
	0x6bb850e3,
	0x83459c63,
	0x968d75e3,
	0xdf750de3,
	0x094d06e3,
	0xdd1814e3,
	0x81308663,
	0xe292c7e3,
	0x50b88763,
	0x64ff46e3,
};

// Source
/*
bge x16, x27, .+3744
bne x11, x20, .-4040
bgeu x26, x8, .-1686
beq x10, x23, .-518
beq x26, x20, .+2188
bne x16, x17, .-568
beq x1, x19, .-4084
blt x5, x9, .-466
beq x17, x11, .+1294
blt x30, x15, .+3660
*/

// Expected objdump (don't mind the <*>)
/*
   0:	6bb850e3          	bge	a6,s11,ea0 <_start+0xea0>
   4:	83459c63          	bne	a1,s4,fffff03c <__global_pointer$+0xffffd814>
   8:	968d75e3          	bgeu	s10,s0,fffff972 <__global_pointer$+0xffffe14a>
   c:	df750de3          	beq	a0,s7,fffffe06 <__global_pointer$+0xffffe5de>
  10:	094d06e3          	beq	s10,s4,89c <_start+0x89c>
  14:	dd1814e3          	bne	a6,a7,fffffddc <__global_pointer$+0xffffe5b4>
  18:	81308663          	beq	ra,s3,fffff024 <__global_pointer$+0xffffd7fc>
  1c:	e292c7e3          	blt	t0,s1,fffffe4a <__global_pointer$+0xffffe622>
  20:	50b88763          	beq	a7,a1,52e <_start+0x52e>
  24:	64ff46e3          	blt	t5,a5,e70 <_start+0xe70>
*/

// Actual output
/*
Hex:
bge     a6,s11,fffffea0
bne     a1,s4,1038
bgeu    s10,s0,fffff96a
beq     a0,s7,fffffdfa
beq     s10,s4,fffff88c
bne     a6,a7,fffffdc8
beq     ra,s3,100c
blt     t0,s1,fffffe2e
beq     a7,a1,50e
blt     t5,a5,fffffe4c

Decimal:
bge     a6,s11,-352
bne     a1,s4,4152
bgeu    s10,s0,-1686
beq     a0,s7,-518
beq     s10,s4,-1908
bne     a6,a7,-568
beq     ra,s3,4108
blt     t0,s1,-466
beq     a7,a1,1294
blt     t5,a5,-436

*/

const unsigned int testInstrs[] = {
	0xfb452c23,
	0x90f71c23,
	0xf45f9723,
	0x06c12323,
	0x700917a3,
	0xf2d42f23,
	0x6967af23,
	0x07a98aa3,
	0xe4991323,
	0xea629a23,
};

int main(int argc, char **argv) {
	for(int i = 0; i < 10; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
