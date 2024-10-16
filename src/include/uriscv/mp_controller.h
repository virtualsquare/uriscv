// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_MP_CONTROLLER_H
#define URISCV_MP_CONTROLLER_H

#include "uriscv/types.h"

class MachineConfig;
class Machine;
class SystemBus;
class Processor;

class MPController {
public:
  MPController(const MachineConfig *config, Machine *machine);

  Word Read(Word addr, const Processor *cpu) const;
  void Write(Word addr, Word data, const Processor *cpu);

private:
  static const unsigned int kCpuResetDelay = 50;
  static const unsigned int kCpuHaltDelay = 50;
  static const unsigned int kPoweroffDelay = 1000;

  const MachineConfig *const config;
  Machine *const machine;

  Word bootPC;
  Word bootSP;
};

#endif // URISCV_MP_CONTROLLER_H
