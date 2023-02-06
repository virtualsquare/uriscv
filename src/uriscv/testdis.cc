#include <stdio.h>

#include "uriscv/disassemble.h"

const unsigned int testInstrs[] = {
	0x03298bb3,
	0x026210b3,
	0x030ca333,
	0x02eeb933,
	0x0320c1b3,
	0x03fe5e33,
	0x02eee333,
	0x0361f133,
	0x02b644b3,
	0x024404b3,
	0x402655b3,
	0x00982eb3,
	0x41308233,
	0x00744b33,
	0x407b08b3,
	0x01d18c33,
	0x41070cb3,
	0x00a015b3,
	0x00fb94b3,
	0x41ea8c33
};

int main(int argc, char **argv) {
	for(int i = 0; i < 20; i++) {
		printf("%s\n", StrInstr(testInstrs[i])); 
	} 
}
