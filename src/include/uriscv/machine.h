// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_MACHINE_H
#define URISCV_MACHINE_H

#include <vector>

#include "base/lang.h"
#include "uriscv/device.h"
#include "uriscv/machine_config.h"
#include "uriscv/stoppoint.h"
#include "uriscv/symbol_table.h"
#include "uriscv/systembus.h"

enum StopCause {
  SC_USER = 1 << 0,
  SC_BREAKPOINT = 1 << 1,
  SC_SUSPECT = 1 << 2,
  SC_EXCEPTION = 1 << 3,
  SC_UTLB_KERNEL = 1 << 4,
  SC_UTLB_USER = 1 << 5
};

class Processor;
class SystemBus;
class Device;
class StoppointSet;

class Machine {
public:
  Machine(const MachineConfig *config, StoppointSet *breakpoints,
          StoppointSet *suspects, StoppointSet *tracepoints);
  ~Machine();

  void step(bool *stopped = NULL);
  void step(unsigned int steps, unsigned int *stepped = NULL,
            bool *stopped = NULL);

  uint32_t idleCycles() const;
  void skip(uint32_t cycles);

  void Halt();
  bool IsHalted() const { return halted; }

  Processor *getProcessor(unsigned int cpuId);
  Device *getDevice(unsigned int line, unsigned int devNo);
  SystemBus *getBus();

  void setStopMask(unsigned int mask);
  unsigned int getStopMask() const;

  unsigned int getStopCause(unsigned int cpuId) const;
  unsigned int getActiveBreakpoint(unsigned int cpuId) const;
  unsigned int getActiveSuspect(unsigned int cpuId) const;

  bool ReadMemory(Word physAddr, Word *data);
  bool WriteMemory(Word paddr, Word data);

  void HandleBusAccess(Word pAddr, Word access, Processor *cpu);
  void HandleVMAccess(Word asid, Word vaddr, Word access, Processor *cpu);

  void setStab(SymbolTable *stab);
  SymbolTable *getStab();

private:
  struct ProcessorData {
    unsigned int stopCause;
    unsigned int breakpointId;
    unsigned int suspectId;
  };

  void onCpuStatusChanged(const Processor *cpu);
  void onCpuException(unsigned int, Processor *cpu);

  unsigned int stopMask;

  const MachineConfig *const config;

  scoped_ptr<SystemBus> bus;

  typedef std::vector<Processor *> CpuVector;
  std::vector<Processor *> cpus;

  ProcessorData pd[MachineConfig::MAX_CPUS];

  bool halted;
  bool stopRequested;
  bool pauseRequested;

  StoppointSet *breakpoints;
  StoppointSet *suspects;
  StoppointSet *tracepoints;

  SymbolTable *stab;
};

#endif // URISCV_MACHINE_H
