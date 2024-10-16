// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/mac_id_edit.h"

#include <QValidator>

#include "base/lang.h"
#include "uriscv/utility.h"

class Validator : public QValidator {
public:
  Validator(QObject *parent = 0) : QValidator(parent) {}

  virtual State validate(QString &input, int &pos) const;
};

QValidator::State Validator::validate(QString &input, int &pos) const {
  UNUSED_ARG(pos);

  input.replace(' ', '0');
  if (input.left(2).toUInt(0, 16) % 2)
    return Invalid;
  else
    return Acceptable;
}

MacIdEdit::MacIdEdit(QWidget *parent) : QLineEdit(parent) {
  setText("00:00:00:00:00:00");
  setInputMask("HH:HH:HH:HH:HH:HH");
  setValidator(new Validator);
}

uint8_t *MacIdEdit::getMacId(uint8_t *id) const {
  return ParseMACId(qPrintable(text()), id);
}

void MacIdEdit::setMacId(const uint8_t *id) {
  setText(MACIdToString(id).c_str());
}
