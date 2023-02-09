#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x0015b5f3,
	0x06eee073,
	0x00000073,
	0x00100073,
	0x013a9673,
	0x04f110f3,
	0x00000073,
	0x38e65bf3,
	0x15595ff3,
	0x004f7873
};

int main(int argc, char **argv) {
	for(int i = 0; i < 10; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
