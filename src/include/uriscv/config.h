// SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_CONFIG_H
#define URISCV_CONFIG_H

#include "uriscv/const.h"
#include "uriscv/types.h"
#include <string>

class Config {
public:
  Config();
  virtual ~Config() { this->romPath = ""; }

  void setRomPath(std::string romPath);
  std::string getRomPath();

private:
  std::string romPath;
};

#endif
