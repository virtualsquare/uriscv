#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x7676bc93,
	0x41635613,
	0x88f48613,
	0xc79f0e93,
	0xbcd07313,
	0x19dfee13,
	0x01fd9b93,
	0x01bfd593,
	0x5ba2fe93,
	0x7131fa13,
	0x01b39113,
	0xd37f2e93,
	0xefe3e813,
	0x9047f493,
	0x003c9493
};

int main(int argc, char **argv) {
	for(int i = 0; i < 15; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
