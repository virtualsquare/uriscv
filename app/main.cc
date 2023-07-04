#include "gdb/gdb.h"
#include "uriscv/config.h"
#include "uriscv/error.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
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
#include <sys/stat.h>

namespace po = boost::program_options;

char defFileName[] = "config_machine.json";

void Panic(const char *message) { ERROR(message); }

bool fileExists(const char *filename) {
  struct stat buf;
  return (stat(filename, &buf) == 0);
}

int main(int argc, char **argv) {

  po::positional_options_description p;
  p.add("config", -1);

  // Declare the supported options.
  po::options_description desc("Syntax");
  desc.add_options()("help", "show this help")(
      "config", po::value<std::string>()->default_value(defFileName))(
      "debug", "enable debug")("disass", "enable disassembler")(
      "iter", po::value<int>(), "iterations")("gdb", "start gdb server");

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << desc << "\n";
    return EXIT_FAILURE;
  }

  const char *filename = vm["config"].as<std::string>().c_str();
  if (!fileExists(filename)) {
    std::cerr << "Config file not exists\n";
    return EXIT_FAILURE;
  }

  std::string error;
  MachineConfig *config = MachineConfig::LoadFromFile(filename, error);

  if (error != "")
    Panic(error.c_str());

  SymbolTable *stab;
  stab = new SymbolTable(config->getSymbolTableASID(),
                         config->getROM(ROM_TYPE_STAB).c_str());
  Machine *mac = new Machine(config, NULL, NULL, NULL);
  mac->setStab(stab);

  int iter = -1;
  if (vm.count("debug"))
    DEBUG = true;
  if (vm.count("disass"))
    DISASS = true;

  bool unlimited = false;
  if (vm.count("iter"))
    iter = vm["iter"].as<int>();
  else
    unlimited = true;

  if (vm.count("gdb")) {
    GDBServer *gdb = new GDBServer(mac);
    gdb->StartServer();
  } else {
    bool stopped = false;
    for (int i = 0; i < iter || unlimited; i++) {
      mac->step(&stopped);
      if (stopped) {
        Panic("Error in step\n");
      }
    }
  }
  return EXIT_SUCCESS;
}
