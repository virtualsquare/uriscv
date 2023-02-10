#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x003100d3,
	0x183110d3,
	0x103120d3,
	0x103130d3,
	0x083140d3,
	0x183170d3,
	0x580100d3,
	0x214584d3,
	0x214594d3,
	0x2145a4d3,
	0x29fc8653,
	0x29971453,
	0xc00100d3,
	0xc0181653,
	0xe00d8253,
	0xe00397d3,
	0xa1fca0d3,
	0xa1971653,
	0xa0310a53,
	0x2145a4d3,
	0xd00080d3,
	0xd01694d3,
	0xf00984d3,
	0xf00f8553,
	0xf0000e53
};

int main(int argc, char **argv) {
	for(int i = 0; i < 25; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
