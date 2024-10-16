// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_DEVICE_TREE_MODEL_H
#define QRISCV_DEVICE_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <sigc++/sigc++.h>

#include "uriscv/arch.h"

class Device;
class Machine;

class DeviceTreeModel : public QAbstractItemModel, public sigc::trackable {
  Q_OBJECT

public:
  enum {
    COLUMN_DEVICE_NUMBER = 0,
    COLUMN_DEVICE_CONDITION,
    COLUMN_DEVICE_STATUS,
    COLUMN_COMPLETION_TOD,
    N_COLUMNS
  };

  DeviceTreeModel(Machine *machine);

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

  Qt::ItemFlags flags(const QModelIndex &index) const;

private:
  void onDeviceStatusChanged(const char *status, Device *device);
  void onDeviceConditionChanged(bool operational, Device *device);

  Machine *const machine;

  static const char *const headerNames[N_COLUMNS];
  QIcon deviceTypeIcons[N_EXT_IL];
  static const char *const iconMap[N_EXT_IL];
};

#endif // QRISCV_DEVICE_TREE_MODEL_H
