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
	0xf0000e53,
	0xff472607,
	0xfe4420a7,
	0x29b387c3,
	0x701f7a47,
	0xa12b374b,
	0x7965a94f,
	0xfe143207,
	0x069b3fa7,
	0x7b65a943,
	0x721f7a47,
	0xa32b374b,
	0x7b65a94f,
	0x023120d3,
	0x0a3110d3,
	0x123170d3,
	0x1a3130d3,
	0x5a02c653,
	0x234584d3,
	0x234594d3,
	0x2345a4d3,
	0x283100d3,
	0x283110d3,
	0x2a3100d3,
	0x2a3110d3,
	0xc20130d3,
	0xc211c153
};

int main(int argc, char **argv) {
	for(int i = 0; i < 51; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
