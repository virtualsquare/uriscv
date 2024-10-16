// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_ADDRESS_LINE_EDIT_H
#define QRISCV_ADDRESS_LINE_EDIT_H

#include <QLineEdit>

#include "uriscv/types.h"

class AddressLineEdit : public QLineEdit {
  Q_OBJECT

public:
  AddressLineEdit(QWidget *parent = 0);
  Word getAddress() const;
  void setAddress(Word address);
};

class AsidLineEdit : public QLineEdit {
  Q_OBJECT

public:
  AsidLineEdit(QWidget *parent = 0);
  Word getAsid() const;
  void setAsid(Word asid);
};

#endif // QRISCV_ADDRESS_LINE_EDIT_H
