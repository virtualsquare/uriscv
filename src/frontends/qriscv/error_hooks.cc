// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QMessageBox>

#include "qriscv/application.h"
#include "uriscv/error.h"

void Panic(const char *message) {
  QMessageBox::critical(0, "PANIC", QString("PANIC: %1").arg(message));
  Appl()->quit();
}
