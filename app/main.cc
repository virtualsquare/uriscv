#include "gdb/gdb.h"
#include "uriscv/config.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/mybus.h"
#include "uriscv/processor.h"
#include "uriscv/stoppoint.h"
#include "uriscv/symbol_table.h"
#include "uriscv/utility.h"
#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace po = boost::program_options;

void Panic(const char *message) { ERROR(message); }

int main(int argc, char **argv) {
  std::cout << "Main\n";

  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")("debug", "enable debug")(
      "iter", po::value<int>(), "iterations")("gdb", "start gdb server");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }

  std::string error;
  MachineConfig *config =
      MachineConfig::LoadFromFile("config_machine.json", error);

  SymbolTable *stab;
  stab = new SymbolTable(config->getSymbolTableASID(),
                         config->getROM(ROM_TYPE_STAB).c_str());
  Machine *mac = new Machine(config, NULL, NULL, NULL);
  mac->setStab(stab);

  /* TODO: should be in a different thread */
  if (vm.count("gdb")) {
    GDBServer *gdb = new GDBServer(mac);
    gdb->StartServer();
  }

  int iter = -1;
  if (vm.count("debug")) {
    DEBUG = true;
  }
  bool unlimited = false;
  if (vm.count("iter"))
    iter = vm["iter"].as<int>();
  else
    unlimited = true;

  bool stopped = false;
  for (int i = 0; i < iter || unlimited; i++) {
    mac->step(&stopped);
    if (stopped) {
      ERROR("Error in step\n");
    }
  }
  return 0;
}
