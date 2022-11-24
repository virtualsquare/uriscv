#include "uriscv/config.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  std::cout << "Main\n";
  Config *config = new Config();
  config->setRomPath("test.bin");
  // config->setRomPath("add-addi.bin");

  Processor *processor = new Processor(config);
  processor->Init(RAMBASE, 0);
  for (int i = 0; i < 40; i++) {
    Word instr = processor->Fetch();
    processor->Excecute(instr);
  }
  return 0;
}
