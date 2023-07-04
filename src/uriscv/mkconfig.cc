/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2023 Gianmaria Rovelli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/****************************************************************************
 *
 * This is a stand-alone program which produces a configuration file to
 * with specified performance figures and geometry, or assembles existing
 * data files into a single flash device image file.  Disk image files are
 * used to emulate disk devices; flash device image files are used to emulate
 * flash drive devices.
 *
 ****************************************************************************/
#include "uriscv/arch.h"
#include "uriscv/machine_config.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <stdio.h>
#include <string>

namespace po = boost::program_options;

HIDDEN char defFileName[] = "config_machine.json";
HIDDEN int defProcessorsNum = 1;
HIDDEN int defClockRate = 1;
HIDDEN int defTlbSize = 16;
HIDDEN int defTlbFloor = 0;
HIDDEN int defRamSize = 64;
HIDDEN char defBootBios[] = "/usr/share/uriscv/coreboot.rom.uriscv";
HIDDEN char defExecBios[] = "/usr/share/uriscv/exec.rom.uriscv";
HIDDEN char defCore[] = "kernel.core.uriscv";
HIDDEN bool defLoadCore = true;
HIDDEN char defStab[] = "kernel.stab.uriscv";
HIDDEN int defStabAsid = 0x40;

HIDDEN char deviceName[][20] = {"Disk", "Flash", "Network", "Printer",
                                "Terminal"};

std::string getInput() {
  std::string input;

  do {
    std::getline(std::cin, input); // First, gets a line and stores in input
  } while (strcmp(input.c_str(), "") &&
           input.find("\n") !=
               std::string::npos); // Checks if input is empty. If so, loop is
                                   // repeated. if not, exits from the loop

  return input;
}

void askForDevice(MachineConfig *config, unsigned int il) {
  std::string r;
  il = EXT_IL_INDEX(il);
  printf("Are there %s devices ? (y/N) : ", deviceName[il]);
  r = getInput();
  if (!strcmp(r.c_str(), "y")) {
    std::string file;
    std::string enabled;
    for (int i = 0; i < N_INTERRUPT_LINES; i++) {
      printf("Device N. %d\n\tFile: ", i);
      file = getInput();
      if (strcmp(file.c_str(), "")) {
        config->setDeviceFile(il, i, file);

        printf("\tEnabled (Y/n) : ");
        enabled = getInput();
        config->setDeviceEnabled(
            il, i,
            (!strcmp(enabled.c_str(), "y") || !strcmp(enabled.c_str(), "Y"))
                ? true
                : false);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;

  po::positional_options_description p;
  p.add("destination", -1);

  po::options_description desc("Syntax");
  desc.add_options()("help", "show this help")(
      "destination", po::value<std::string>()->default_value(defFileName),
      "Configuration file")("proc",
                            po::value<int>()->default_value(defProcessorsNum),
                            "Number of processors")(
      "clock-rate", po::value<int>()->default_value(defClockRate),
      "Clock rate in MHz")("tlb-size",
                           po::value<int>()->default_value(defTlbSize),
                           "Size of the TLB { 4 | 8 | 16 | 32 | 64 }")(
      "tlb-floor", po::value<int>()->default_value(defTlbFloor),
      "TLB floor { 0x0(RAMTOP) | 0x4000000 | 0x8000000 | 0xFFFFFFFF(OFF) }")(
      "ram-size", po::value<int>()->default_value(defRamSize),
      "Size of the RAM")("boot-bios",
                         po::value<std::string>()->default_value(defBootBios),
                         "Path of the ROM with the bootstrap instructions")(
      "exec-bios", po::value<std::string>()->default_value(defExecBios),
      "Path of the ROM with bios")(
      "core", po::value<std::string>()->default_value(defCore),
      "Path of the kernel")("load-core",
                            po::value<bool>()->default_value(defLoadCore),
                            "Load the core file")(
      "stab", po::value<std::string>()->default_value(defStab),
      "Path of the symbol table")("stab-asid",
                                  po::value<int>()->default_value(defStabAsid),
                                  "ASID of the symbol table");
  try {

    // Declare the supported options.

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cerr << desc << "\n";
      ret = EXIT_FAILURE;
    } else {
      MachineConfig *config =
          MachineConfig::Create(vm["destination"].as<std::string>().c_str());
      config->setNumProcessors(vm["proc"].as<int>());
      config->setClockRate(vm["clock-rate"].as<int>());
      config->setTLBSize(vm["tlb-size"].as<int>());
      config->setTLBFloorAddress(vm["tlb-floor"].as<int>());
      config->setRamSize(vm["ram-size"].as<int>());
      config->setRamSize(vm["ram-size"].as<int>());
      config->setROM(ROM_TYPE_BOOT, vm["boot-bios"].as<std::string>());
      config->setROM(ROM_TYPE_BIOS, vm["exec-bios"].as<std::string>());
      config->setROM(ROM_TYPE_CORE, vm["core"].as<std::string>());
      config->setLoadCoreEnabled(vm["load-core"].as<bool>());
      config->setROM(ROM_TYPE_STAB, vm["stab"].as<std::string>());
      config->setSymbolTableASID(vm["stab-asid"].as<int>());

      for (int i = IL_DISK; i < IL_DISK + N_EXT_IL; i++)
        askForDevice(config, i);

      std::list<std::string> errors;
      if (!config->Validate(&errors)) {
        std::cerr << "Errors occured\n";
        for (std::string n : errors)
          std::cerr << n << ", ";
        std::cerr << std::endl;
      }
      config->Save();
    }
  } catch (po::error &e) {
    std::cerr << "Error(s) while parsing the options" << std::endl;
    std::cerr << desc << "\n";
    ret = EXIT_FAILURE;
  }

  return ret;
}
