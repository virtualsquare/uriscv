// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_ADD_SUSPECT_DIALOG_H
#define QRISCV_ADD_SUSPECT_DIALOG_H

#include <QDialog>

#include "uriscv/types.h"

class AddressLineEdit;
class AsidLineEdit;
class SymbolTable;
class SortFilterSymbolTableModel;
class QItemSelection;
class QPushButton;

class AddSuspectDialog : public QDialog {
  Q_OBJECT

public:
  AddSuspectDialog(QWidget *parent = 0);

  Word getStartAddress() const;
  Word getEndAddress() const;
  Word getASID() const;

private:
  static const int kInitialWidth = 380;
  static const int kInitialHeight = 340;

  AsidLineEdit *asidEditor;
  AddressLineEdit *startAddressEdit;
  AddressLineEdit *endAddressEdit;

  const SymbolTable *const stab;
  SortFilterSymbolTableModel *proxyModel;

  QPushButton *okButton;

private Q_SLOTS:
  void validate();
  void onSelectionChanged(const QItemSelection &selected);
};

#endif // QRISCV_ADD_SUSPECT_DIALOG_H
