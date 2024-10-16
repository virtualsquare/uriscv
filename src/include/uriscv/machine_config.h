// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_MACHINE_CONFIG_H
#define URISCV_MACHINE_CONFIG_H

#include <list>
#include <string>

#include "base/basic_types.h"
#include "base/lang.h"
#include "uriscv/arch.h"
#include "uriscv/const.h"
#include "uriscv/types.h"
#include "uriscv/utility.h"

enum ROMType {
  ROM_TYPE_BOOT,
  ROM_TYPE_BIOS,
  ROM_TYPE_CORE,
  ROM_TYPE_STAB,
  N_ROM_TYPES
};

class MachineConfig {
public:
  static const Word MIN_RAM = 8;
  static const Word MAX_RAM = 512;
  static const Word DEFAUlT_RAM_SIZE = 64;

  static const unsigned int MIN_CPUS = 1;
  static const unsigned int MAX_CPUS = 16;
  static const unsigned int DEFAULT_NUM_CPUS = 1;

  static const unsigned int MIN_CLOCK_RATE = 1;
  static const unsigned int MAX_CLOCK_RATE = 99;
  static const unsigned int DEFAULT_CLOCK_RATE = 1;

  static const Word MIN_TLB = 4;
  static const Word MAX_TLB = 64;
  static const Word DEFAULT_TLB_SIZE = 16;

  static constexpr Word TLB_FLOOR_ADDRESS[4] = {MINWORDVAL, 0x40000000,
                                                0x80000000, MAXWORDVAL};
  static const Word DEFAULT_TLB_FLOOR_ADDRESS = MAXWORDVAL;

  static const Word MIN_ASID = 0;
  static const Word MAX_ASID = 64;

  static MachineConfig *LoadFromFile(const std::string &fileName,
                                     std::string &error);
  static MachineConfig *Create(const std::string &fileName);

  const std::string &getFileName() const { return fileName; }

  void Save();

  bool Validate(std::list<std::string> *errors) const;

  void setLoadCoreEnabled(bool setting) { loadCoreFile = setting; }
  bool isLoadCoreEnabled() const { return loadCoreFile; }

  void setRamSize(Word size);
  Word getRamSize() const { return ramSize; }

  void setNumProcessors(unsigned int value);
  unsigned int getNumProcessors() const { return cpus; }

  void setClockRate(unsigned int value);
  unsigned int getClockRate() const { return clockRate; }

  void setTLBSize(Word size);
  Word getTLBSize() const { return tlbSize; }

  void setTLBFloorAddress(Word addr);
  Word getTLBFloorAddress() const { return tlbFloorAddress; }

  void setROM(ROMType type, const std::string &fileName);
  const std::string &getROM(ROMType type) const;

  void setSymbolTableASID(Word asid);
  Word getSymbolTableASID() const { return symbolTableASID; }

  unsigned int getDeviceType(unsigned int il, unsigned int devNo) const;
  bool getDeviceEnabled(unsigned int il, unsigned int devNo) const;
  void setDeviceEnabled(unsigned int il, unsigned int devNo, bool setting);
  void setDeviceFile(unsigned int il, unsigned int devNo,
                     const std::string &fileName);
  const std::string &getDeviceFile(unsigned int il, unsigned int devNo) const;
  const uint8_t *getMACId(unsigned int devNo) const;
  void setMACId(unsigned int devNo, const uint8_t *value);

private:
  MachineConfig(const std::string &fileName);

  void resetToFactorySettings();
  bool validFileMagic(Word tag, const char *fName);

  std::string fileName;

  bool loadCoreFile;

  Word ramSize;
  unsigned int cpus;
  unsigned int clockRate;
  Word tlbSize;
  Word tlbFloorAddress;

  std::string romFiles[N_ROM_TYPES];
  Word symbolTableASID;

  std::string devFiles[N_EXT_IL][N_DEV_PER_IL];
  bool devEnabled[N_EXT_IL][N_DEV_PER_IL];
  scoped_array<uint8_t> macId[N_DEV_PER_IL];

  static const char *const deviceKeyPrefix[N_EXT_IL];
};

#endif // URISCV_MACHINE_CONFIG_H
