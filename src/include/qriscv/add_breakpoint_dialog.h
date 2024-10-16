// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_ADD_BREAKPOINT_DIALOG_H
#define QRISCV_ADD_BREAKPOINT_DIALOG_H

#include <QDialog>
#include <QItemSelection>

#include "uriscv/types.h"

class AddressLineEdit;
class AsidLineEdit;
class SymbolTable;
class SortFilterSymbolTableModel;

class AddBreakpointDialog : public QDialog {
  Q_OBJECT

public:
  AddBreakpointDialog(QWidget *parent = 0);

  Word getStartAddress() const;
  Word getASID() const;

private:
  static const int kInitialWidth = 380;
  static const int kInitialHeight = 340;

  AsidLineEdit *asidEditor;
  AddressLineEdit *addressEditor;

  const SymbolTable *const stab;
  SortFilterSymbolTableModel *proxyModel;

private Q_SLOTS:
  void onSelectionChanged(const QItemSelection &selected);
};

#endif // QRISCV_ADD_BREAKPOINT_DIALOG_H
