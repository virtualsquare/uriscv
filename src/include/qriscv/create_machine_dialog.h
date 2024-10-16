// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_CREATE_MACHINE_DIALOG_H
#define QRISCV_CREATE_MACHINE_DIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class CreateMachineDialog : public QDialog {
  Q_OBJECT

public:
  CreateMachineDialog(QWidget *parent = 0);

  QString getFileName() const;

private:
  bool hasValidInput() const;

  QLineEdit *dirEdit;
  QLineEdit *nameEdit;
  QPushButton *createButton;

private Q_SLOTS:
  void validate();
  void browseDir();
};

#endif // QRISCV_CREATE_MACHINE_DIALOG_H
