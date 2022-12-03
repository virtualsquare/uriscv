#include "uriscv/config.h"
#include "uriscv/mybus.h"
#include "uriscv/processor.h"
#include "uriscv/utility.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#define ENTRYPOINT 0x2000124c

int main(int argc, char **argv) {
  std::cout << "Main\n";
  Config *config = new Config();
  // config->setRomPath("test.bin");
  // config->setRomPath("mytest.bin");
  // config->setRomPath("../src/tests/rv32ui-p-addi.bin");
  config->setRomPath("../tests/kernel.core.uriscv");
  // config->setRomPath("../src/tests/hello.bin");
  MyBus *bus = new MyBus(config);
  // config->setRomPath("add-addi.bin");

  Processor *processor = new Processor(config, bus);
  processor->Init(2, ENTRYPOINT, 0);
  for (int i = 0; i < 90; i++) {
    if (processor->Cycle() != EXC_OK)
      break;
  }
  return 0;
}
