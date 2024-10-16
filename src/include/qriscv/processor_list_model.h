// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_PROCESSOR_LIST_MODEL_H
#define QRISCV_PROCESSOR_LIST_MODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>

class MachineConfig;
class DebugSession;
class CpuStatusMap;

class ProcessorListModel : public QAbstractTableModel {
  Q_OBJECT

public:
  enum { COLUMN_CPU_ID, COLUMN_CPU_STATUS, COLUMN_CPU_ADDRESS, N_COLUMNS };

  ProcessorListModel(QObject *parent = 0);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;

private:
  static const char *headers[N_COLUMNS];

  const MachineConfig *const config;
  const DebugSession *const dbgSession;
  const CpuStatusMap *const cpuStatusMap;

private Q_SLOTS:
  void notifyStatusChanged();
};

#endif // QRISCV_PROCESSOR_LIST_MODEL_H
