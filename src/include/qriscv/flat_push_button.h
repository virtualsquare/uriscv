// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_FLAT_PUSH_BUTTON_H
#define QRISCV_FLAT_PUSH_BUTTON_H

#include <QPushButton>

class FlatPushButton : public QPushButton {
  Q_OBJECT

public:
  FlatPushButton(QWidget *parent = 0);
  FlatPushButton(const QString &text, QWidget *parent = 0);
  FlatPushButton(const QIcon &icon, const QString &text, QWidget *parent = 0);

protected:
  virtual void enterEvent(QEvent *event);
  virtual void leaveEvent(QEvent *event);
};

#endif // QRISCV_FLAT_PUSH_BUTTON_H
