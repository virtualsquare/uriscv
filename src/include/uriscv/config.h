#ifndef URISCV_CONFIG_H
#define URISCV_CONFIG_H

#include "support/liburiscv/const.h"
#include "support/liburiscv/types.h"
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
