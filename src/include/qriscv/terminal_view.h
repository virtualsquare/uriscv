// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TERMINAL_VIEW_H
#define QRISCV_TERMINAL_VIEW_H

#include <sigc++/sigc++.h>

#include <QByteArray>
#include <QPlainTextEdit>

class TerminalDevice;

class TerminalView : public QPlainTextEdit, public sigc::trackable {
  Q_OBJECT

public:
  TerminalView(TerminalDevice *terminal, QWidget *parent = 0);

protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);

  virtual bool canInsertFromMimeData(const QMimeData *source) const;
  virtual void insertFromMimeData(const QMimeData *source);

private:
  void flushInput();
  void onCharTransmitted(char c);

  TerminalDevice *const terminal;
  QByteArray input;
};

#endif // QRISCV_TERMINAL_VIEW_H
