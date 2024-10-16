// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TREE_VIEW_H
#define QRISCV_TREE_VIEW_H

#include <list>

#include <QTreeView>

class QAbstractItemModel;

class TreeView : public QTreeView {
  Q_OBJECT

public:
  TreeView(const QString &name, const std::list<int> &resizedToContents,
           bool persistItemState = false, QWidget *parent = 0);

  void setModel(QAbstractItemModel *model);

private Q_SLOTS:
  void sectionResized(int logicalIndex, int oldSize, int newSize);
  void saveItemState();

private:
  const std::list<int> resizedToContents;
  const bool persistItemState;
  QString itemStateKey;
};

#endif // QRISCV_TREE_VIEW_H
