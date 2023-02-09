#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x0015b5f3,
	0x06eee073,
	0x05235973,
	0x003b72f3,
	0x013a9673,
	0x04f110f3,
	0x050420f3,
	0x38e65bf3,
	0x15595ff3,
	0x004f7873
};

int main(int argc, char **argv) {
	for(int i = 0; i < 10; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
