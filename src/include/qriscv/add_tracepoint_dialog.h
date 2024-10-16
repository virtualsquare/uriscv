// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_ADD_TRACEPOINT_DIALOG_H
#define QRISCV_ADD_TRACEPOINT_DIALOG_H

#include <QDialog>

#include "uriscv/types.h"

class AddressLineEdit;
class SymbolTable;
class SortFilterSymbolTableModel;
class QItemSelection;
class QLabel;
class QPushButton;

class AddTracepointDialog : public QDialog {
  Q_OBJECT

public:
  static const Word kMaxTracedRangeSize = 1025;

  AddTracepointDialog(QWidget *parent = 0);

  Word getStartAddress() const;
  Word getEndAddress() const;

private:
  static const int kInitialWidth = 430;
  static const int kInitialHeight = 340;

  AddressLineEdit *startAddressEdit;
  AddressLineEdit *endAddressEdit;

  const SymbolTable *const stab;
  SortFilterSymbolTableModel *proxyModel;

  QLabel *inputErrorLabel;
  QPushButton *okButton;

private Q_SLOTS:
  void validate();
  void onSelectionChanged(const QItemSelection &selected);
};

#endif // QRISCV_ADD_TRACEPOINT_DIALOG_H
