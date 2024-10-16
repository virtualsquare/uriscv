// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/terminal_view.h"

#include <cassert>
#include <string>

#include <QFile>
#include <QFontMetrics>
#include <QTextStream>

#include "base/lang.h"
#include "qriscv/application.h"
#include "uriscv/device.h"

TerminalView::TerminalView(TerminalDevice *terminal, QWidget *parent)
    : QPlainTextEdit(parent), terminal(terminal) {
  terminal->SignalTransmitted.connect(
      sigc::mem_fun(this, &TerminalView::onCharTransmitted));

  QFont font = Appl()->getMonospaceFont();
  setFont(font);
  setCursorWidth(fontMetrics().horizontalAdvance("o"));

  // Disable features that look silly in a basic terminal widget
  setContextMenuPolicy(Qt::NoContextMenu);
  setUndoRedoEnabled(false);

  // Wrapping
  setWordWrapMode(QTextOption::WrapAnywhere);

  std::string devFile = Appl()->getConfig()->getDeviceFile(
      terminal->getInterruptLine(), terminal->getNumber());
  QFile file(devFile.c_str());
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    // TODO: fix this and another gazillion critical points
    assert(0);
  }
  QTextStream in(&file);
  setPlainText(in.readAll());

  moveCursor(QTextCursor::End);
}

void TerminalView::keyPressEvent(QKeyEvent *e) {
  if (e->modifiers() & ~Qt::ShiftModifier)
    return;

  int key = e->key();
  if (key == Qt::Key_Return || key == Qt::Key_Enter) {
    flushInput();
    QPlainTextEdit::keyPressEvent(e);
  } else if (key == Qt::Key_Backspace && !input.isEmpty()) {
    input.chop(1);
    QPlainTextEdit::keyPressEvent(e);
  } else if (!e->text().isEmpty() && Qt::Key_Space <= key &&
             key <= Qt::Key_nobreakspace) {
    input.append(e->text().toLatin1());
    QPlainTextEdit::keyPressEvent(e);
  }
}

void TerminalView::mousePressEvent(QMouseEvent *e) { UNUSED_ARG(e); }

bool TerminalView::canInsertFromMimeData(const QMimeData *source) const {
  UNUSED_ARG(source);
  return false;
}

void TerminalView::insertFromMimeData(const QMimeData *source) {
  // It seems that Qt forces us to reimplement (stub out) this if we
  // want to avoid drop and clipboard paste ops in all cases.
  UNUSED_ARG(source);
}

void TerminalView::flushInput() {
  terminal->Input(input.constData());
  input.clear();
}

void TerminalView::onCharTransmitted(char c) {
  insertPlainText(QString(c));

  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::End);
  setTextCursor(cursor);
}
