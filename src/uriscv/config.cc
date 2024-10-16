// SPDX-FileCopyrightText: 2023 Gianmaria Rovelli
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uriscv/config.h"

Config::Config() {}

void Config::setRomPath(std::string romPath) { this->romPath = romPath; }
std::string Config::getRomPath() { return this->romPath; }
