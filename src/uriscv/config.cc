#include "uriscv/config.h"

Config::Config() {}

void Config::setRomPath(std::string romPath) { this->romPath = romPath; }
std::string Config::getRomPath() { return this->romPath; }
