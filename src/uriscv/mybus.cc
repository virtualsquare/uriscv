#include "uriscv/mybus.h"
#include "uriscv/arch.h"
#include "uriscv/const.h"
#include "uriscv/utility.h"
#include <cassert>
#include <cstdint>

const Word DEFAULT_RAM_SIZE = 64;

RAM::RAM(Word size, uint8_t *memory) {
  this->size = size;
  this->memory = new uint8_t[Word(this->size)];
}
RAM::RAM(Word size, uint8_t *memory, Word memorySize, Word offset) {
  this->size = size;
  this->memory = new uint8_t[Word(this->size)];
  if (memory != NULL) {
    Word i = 0;
    while (memory != NULL && i < memorySize) {
      this->memory[i + offset] = *memory;
      memory++;
      i++;
    }
  }
}

exception_t RAM::Read(Word addr, Word *dst) const {
  if (addr < 0 || addr + 3 > this->size) {
    return EXC_LOAD_ADDR_MIS;
  }
  *dst = this->memory[addr] | this->memory[addr + 1] << 8 |
         this->memory[addr + 2] << 16 | this->memory[addr + 3] << 24;
  return EXC_OK;
}
exception_t RAM::Write(Word addr, Word value) {
  if (addr < 0 || addr + 3 > this->size) {
    return EXC_LOAD_ADDR_MIS;
  }
  this->memory[addr] = value & 0xFF;
  this->memory[addr + 1] = (value >> 8) & 0xFF;
  this->memory[addr + 2] = (value >> 16) & 0xFF;
  this->memory[addr + 3] = (value >> 24) & 0xFF;
  return EXC_OK;
}

Word RAM::Size() { return this->size; }

MyBus::MyBus(Config *config) {
  std::cout << "[-] MyBus init" << std::endl;
  char *buffer = NULL;
  Word ramSize;
  Utility::readFile(config->getRomPath(), buffer, &ramSize);
  this->ram = new RAM(DEFAULT_RAM_SIZE * FRAMESIZE, (uint8_t *)buffer, ramSize,
                      CORE_TEXT_VADDR - RAMBASE);
  this->biosdata = new RAM(BIOSDATASIZE, NULL);
}

exception_t MyBus::Read(Word addr, Word *dst) {
  *dst = 0;
  if (INBOUNDS(addr, RAMBASE, RAMBASE + this->ram->Size())) {
    return this->ram->Read(addr - RAMBASE, dst);
  } else if (INBOUNDS(addr, BIOSDATABASE,
                      BIOSDATABASE + this->biosdata->Size())) {
    return this->biosdata->Read(addr - BIOSDATABASE, dst);
  } else if (INBOUNDS(addr, MMIO_BASE, MMIO_END)) {
    return this->BusRegRead(addr, dst);
  }
  return EXC_LOAD_ACC_FAULT;
}

exception_t MyBus::BusRegRead(Word addr, Word *dst) {
  if (INBOUNDS(addr, DEV_REG_START, DEV_REG_END)) {
    ERROR("Dev not implemented");
  } else {
    switch (addr) {
    case BUS_REG_RAM_BASE: {
      *dst = RAMBASE;
      return EXC_OK;
    }
    case BUS_REG_BIOS_BASE: {
      *dst = BIOSBASE;
      return EXC_OK;
    }
    case BUS_REG_BOOT_BASE: {
      *dst = BOOTBASE;
      return EXC_OK;
    }
    }
  }
  return EXC_LOAD_ACC_FAULT;
}

exception_t MyBus::Write(Word addr, Word value) {
  if (INBOUNDS(addr, RAMBASE, RAMBASE + this->ram->Size())) {
    return this->ram->Write(addr - RAMBASE, value);
  } else if (INBOUNDS(addr, BIOSDATABASE,
                      BIOSDATABASE + this->biosdata->Size())) {
    return this->biosdata->Write(addr - BIOSDATABASE, value);
  } else if (INBOUNDS(addr, MMIO_BASE, MMIO_END)) {
    return this->BusRegWrite(addr, value);
  }
  return EXC_LOAD_ACC_FAULT;
}

exception_t MyBus::BusRegWrite(Word addr, Word value) {
  switch (addr) {}
  return EXC_LOAD_ACC_FAULT;
}
