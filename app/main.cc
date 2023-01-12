#include "uriscv/config.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/mybus.h"
#include "uriscv/processor.h"
#include "uriscv/stoppoint.h"
#include "uriscv/symbol_table.h"
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

  MachineConfig *config = MachineConfig::Create("config_machine.json");
  config->setDeviceFile(EXT_IL_INDEX(IL_TERMINAL), 1, "term1.uriscv");
  config->setDeviceEnabled(EXT_IL_INDEX(IL_TERMINAL), 1, true);
  SymbolTable *stab;
  stab = new SymbolTable(config->getSymbolTableASID(),
                         config->getROM(ROM_TYPE_STAB).c_str());
  Machine *mac = new Machine(config, NULL, NULL, NULL);
  mac->setStab(stab);

  // Processor *processor = new Processor(config, bus);
  // processor->Init(2, ENTRYPOINT, 0);
  bool stopped = false;
  // for (int i = 0; i < 4460; i++) {
  for (;;) {
    // printf("%d ", i);
    mac->step(&stopped);
    if (stopped) {
      ERROR("Error in step\n");
    }
  }
  return 0;
}
