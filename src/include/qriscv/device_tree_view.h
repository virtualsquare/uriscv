// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_DEVICE_TREE_VIEW_H
#define QRISCV_DEVICE_TREE_VIEW_H

#include <QTreeView>

class QAbstractItemModel;

class DeviceTreeView : public QTreeView {
  Q_OBJECT

public:
  DeviceTreeView(QWidget *parent = 0);
  virtual void setModel(QAbstractItemModel *model);

private Q_SLOTS:
  void sectionResized(int logicalIndex, int oldSize, int newSize);
};

#endif // QRISCV_DEVICE_TREE_VIEW_H
