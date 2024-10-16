// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/flat_push_button.h"

#include "base/lang.h"

FlatPushButton::FlatPushButton(QWidget *parent) : QPushButton(parent) {
  setFlat(true);
}

FlatPushButton::FlatPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent) {
  setFlat(true);
}

FlatPushButton::FlatPushButton(const QIcon &icon, const QString &text,
                               QWidget *parent)
    : QPushButton(icon, text, parent) {
  setFlat(true);
}

void FlatPushButton::enterEvent(QEvent *event) {
  UNUSED_ARG(event);
  setFlat(false);
}

void FlatPushButton::leaveEvent(QEvent *event) {
  UNUSED_ARG(event);
  setFlat(true);
}
