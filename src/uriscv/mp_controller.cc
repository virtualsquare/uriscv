// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uriscv/mp_controller.h"

#include <boost/bind/bind.hpp>

#include "base/lang.h"
#include "uriscv/arch.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"
#include "uriscv/processor.h"
#include "uriscv/systembus.h"

using namespace boost::placeholders;

MPController::MPController(const MachineConfig *config, Machine *machine)
    : config(config), machine(machine), bootPC(MCTL_DEFAULT_BOOT_PC),
      bootSP(MCTL_DEFAULT_BOOT_SP) {}

Word MPController::Read(Word addr, const Processor *cpu) const {
  UNUSED_ARG(cpu);

  switch (addr) {
  case MCTL_NCPUS:
    return config->getNumProcessors();

  case MCTL_BOOT_PC:
    return bootPC;

  case MCTL_BOOT_SP:
    return bootSP;

  default:
    return 0;
  }
}

void MPController::Write(Word addr, Word data, const Processor *cpu) {
  UNUSED_ARG(cpu);

  Word cpuId;

  switch (addr) {
  case MCTL_RESET_CPU:
    cpuId = data & MCTL_RESET_CPU_CPUID_MASK;
    if (cpuId < config->getNumProcessors())
      machine->getBus()->scheduleEvent(kCpuResetDelay * config->getClockRate(),
                                       boost::bind(&Processor::Reset,
                                                   machine->getProcessor(cpuId),
                                                   bootPC, bootSP));
    break;

  case MCTL_BOOT_PC:
    bootPC = data;
    break;

  case MCTL_BOOT_SP:
    bootSP = data;
    break;

  case MCTL_HALT_CPU:
    cpuId = data & MCTL_RESET_CPU_CPUID_MASK;
    if (cpuId < config->getNumProcessors())
      machine->getBus()->scheduleEvent(
          kCpuHaltDelay * config->getClockRate(),
          boost::bind(&Processor::Halt, machine->getProcessor(cpuId)));
    break;

  case MCTL_POWER:
    if (data == 0x0FF)
      machine->getBus()->scheduleEvent(kPoweroffDelay * config->getClockRate(),
                                       boost::bind(&Machine::Halt, machine));
    break;

  default:
    break;
  }
}
