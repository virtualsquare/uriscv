// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_DEBUG_SESSION_H
#define QRISCV_DEBUG_SESSION_H

#include <QObject>
#include <thread>

#include "base/lang.h"
#include "qriscv/cpu_status_map.h"
#include "qriscv/stoppoint_list_model.h"
#include "uriscv/machine.h"
#include "uriscv/stoppoint.h"
#include "uriscv/symbol_table.h"
#include "gdb/gdb.h"

enum MachineStatus { MS_HALTED, MS_RUNNING, MS_STOPPED };

class QAction;
class Machine;
class QTimer;

class DebugSession : public QObject {
  Q_OBJECT

public:
  static const int kNumSpeedLevels = 5;
  static const int kMaxSpeed = kNumSpeedLevels - 1;

  static const int kDefaultStopMask =
      (SC_BREAKPOINT | SC_SUSPECT | SC_EXCEPTION);

  DebugSession();

  MachineStatus getStatus() const { return status; }

  bool isStopped() const { return status == MS_STOPPED; }
  bool isStoppedByUser() const { return stoppedByUser; }
  bool isRunning() const { return status == MS_RUNNING; }
  bool isStarted() const { return status != MS_HALTED; }

  void halt();
  void killServer();

  unsigned int getStopMask() const { return stopMask; }
  int getSpeed() const { return speed; }

  Machine *getMachine() const { return machine.get(); }
  SymbolTable *getSymbolTable() { return symbolTable.get(); }

  StoppointSet *getBreakpoints() { return &breakpoints; }
  StoppointListModel *getBreakpointListModel() { return bplModel.get(); }

  StoppointSet *getSuspects() { return &suspects; }
  StoppointSet *getTracepoints() { return &tracepoints; }

  const CpuStatusMap *getCpuStatusMap() const { return cpuStatusMap.get(); }

  // Global actions
  QAction *startMachineAction;
  QAction *haltMachineAction;
  QAction *toggleMachineAction;
  QAction *resetMachineAction;

  QAction *debugContinueAction;
  QAction *debugStepAction;
  QAction *debugStopAction;
  QAction *debugToggleAction;

  //GDB stub
  void setGdbStatus(bool value);
  bool isGdbEnabled() { return GdbStatus; }

public Q_SLOTS:
  void setStopMask(unsigned int value);
  void setSpeed(int value);
  void stop();

Q_SIGNALS:
  void StatusChanged();
  void MachineStarted();
  void MachineStopped();
  void MachineRan();
  void MachineHalted();
  void MachineReset();
  void DebugIterationCompleted();

  void SpeedChanged(int);

private:
  static const uint32_t kMaxSkipped = 50000;

  void createActions();
  void setStatus(MachineStatus newStatus);

  void initializeMachine();

  void initializeThreadServer(MachineConfig*);

  void step(unsigned int steps);
  void runStepIteration();
  void runContIteration();

  void relocateStoppoints(const SymbolTable *newTable, StoppointSet &set);

  MachineStatus status;
  scoped_ptr<Machine> machine;

  scoped_ptr<SymbolTable> symbolTable;

  // We need a "proxy" stop mask here since it has to live through
  // machine reconfigurations, resets, etc.
  unsigned int stopMask;

  int speed;
  static const unsigned int kIterCycles[kNumSpeedLevels];
  static const unsigned int kIterInterval[kNumSpeedLevels];

  StoppointSet breakpoints;
  scoped_ptr<StoppointListModel> bplModel;
  StoppointSet suspects;
  StoppointSet tracepoints;

  scoped_ptr<CpuStatusMap> cpuStatusMap;

  bool stoppedByUser;

  bool stepping;
  unsigned int stepsLeft;

  QTimer *timer;
  QTimer *idleTimer;

  uint32_t idleSteps;

  bool GdbStatus = false;
  std::thread gts;
  GDBServer *gdb;

private Q_SLOTS:
  void onMachineConfigChanged();

  void startMachine();
  void onHaltMachine();
  void toggleMachine();
  void onResetMachine();
  void onContinue();
  void onStep();
  void toggleDebug();

  void updateActionSensitivity();

  void runIteration();
  void skip();
};

#endif // QRISCV_DEBUG_SESSION_H