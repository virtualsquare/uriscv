// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/application.h"

int main(int argc, char **argv) {
  Application application(argc, argv);

  return application.exec();
}
