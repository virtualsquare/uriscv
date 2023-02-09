#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x0220000f,
	0x8330000f,
	0x0250000f,
	0x0f80000f,
	0x0320000f,
	0x0420000f,
	0x0100000f,
	0x0ff0000f,
	0x0330000f,
	0x02f0000f
};

int main(int argc, char **argv) {
	for(int i = 0; i < 10; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
