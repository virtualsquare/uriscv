// SPDX-FileCopyrightText: 2010, 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_REGISTER_SET_WIDGET_H
#define QRISCV_REGISTER_SET_WIDGET_H

#include <vector>

#include <QDockWidget>

#include "uriscv/types.h"

class QAction;
class QTreeView;
class Processor;
class RegisterSetSnapshot;
class QStyledItemDelegate;
class QActionGroup;
class QToolBar;

class RegisterSetWidget : public QDockWidget {
  Q_OBJECT

public:
  RegisterSetWidget(Word cpuId, QWidget *parent = 0);

protected:
  RegisterSetSnapshot *model;

private Q_SLOTS:
  void updateWindowTitle();
  void setDisplayType(QAction *action);

private:
  void addDisplayAction(const QString &text, QStyledItemDelegate *delegate,
                        QActionGroup *group, QToolBar *toolBar);
  int currentDelegate() const;

  const Word cpuId;
  QTreeView *treeView;
  std::vector<QStyledItemDelegate *> delegates;
  const QString delegateKey;
};

#endif // QRISCV_REGISTER_SET_WIDGET_H
