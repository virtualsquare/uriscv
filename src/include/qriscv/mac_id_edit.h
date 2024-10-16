// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MAC_ID_EDIT_H
#define QRISCV_MAC_ID_EDIT_H

#include <QLineEdit>

#include "base/basic_types.h"

class MacIdEdit : public QLineEdit {
public:
  MacIdEdit(QWidget *parent = 0);

  uint8_t *getMacId(uint8_t *id) const;
  void setMacId(const uint8_t *id);
};

#endif // QRISCV_MAC_ID_EDIT_H
