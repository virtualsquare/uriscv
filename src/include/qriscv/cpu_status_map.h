// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_CPU_STATUS_MAP_H
#define QRISCV_CPU_STATUS_MAP_H

#include <vector>

#include <QObject>
#include <QString>

class DebugSession;
class Machine;
class Processor;

class CpuStatusMap : public QObject {
  Q_OBJECT

public:
  CpuStatusMap(DebugSession *dbgSession);

  const QString &getStatus(unsigned int cpuId) const;
  const QString &getLocation(unsigned int cpuId) const;

Q_SIGNALS:
  void Changed();

private:
  struct StatusInfo {
    QString status;
    QString location;
  };

  void formatActiveCpuStatus(Processor *cpu);
  void formatActiveCpuLocation(Processor *cpu);

  DebugSession *const dbgSession;
  Machine *const machine;

  static const char *const statusTemplates[];

  std::vector<StatusInfo> statusMap;

private Q_SLOTS:
  void update();
};

#endif // QRISCV_CPU_STATUS_MAP_H
