#include "uriscv/config.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/mybus.h"
#include "uriscv/processor.h"
#include "uriscv/stoppoint.h"
#include "uriscv/utility.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#define ENTRYPOINT 0x2000124c

void Panic(const char *message) { ERROR(message); }

int main(int argc, char **argv) {
  std::cout << "Main\n";
  // Config *config = new Config();
  // config->setRomPath("test.bin");
  // config->setRomPath("mytest.bin");
  // config->setRomPath("../src/tests/rv32ui-p-addi.bin");
  // config->setRomPath("../tests/kernel.core.uriscv");
  // config->setRomPath("../src/tests/hello.bin");
  // MyBus *bus = new MyBus(config);
  // config->setRomPath("add-addi.bin");

  MachineConfig *mac_config = MachineConfig::Create("config_machine.json");
  Machine *mac = new Machine(mac_config, NULL, NULL, NULL);
  mac->step();

  // Processor *processor = new Processor(config, bus);
  // processor->Init(2, ENTRYPOINT, 0);
  // for (int i = 0; i < 90; i++) {
  //   if (processor->Cycle() != EXC_OK)
  //     break;
  // }
  return 0;
}
