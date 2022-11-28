#include "uriscv/bus.h"
#include "uriscv/utility.h"
#include <cassert>

const Word DEFAULT_RAM_SIZE = 64;

RAM::RAM(Word size, uint8_t *memory) {
  this->size = size;
  this->memory = memory;
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

Word RAM::Size() const { return this->size; }

Bus::Bus(Config *config) {
  std::cout << "[-] Bus init" << std::endl;
  char *buffer = NULL;
  Word ramSize;
  Utility::readFile(config->getRomPath(), buffer, &ramSize);
  this->ram = new RAM(DEFAULT_RAM_SIZE * FRAMESIZE, (uint8_t *)buffer);
}

exception_t Bus::Read(Word addr, Word *dst) {
  *dst = 0;
  if (INBOUNDS(addr, RAMBASE, RAMBASE + this->ram->Size())) {
    return this->ram->Read(addr - RAMBASE, dst);
  }
  return EXC_LOAD_ACC_FAULT;
}

exception_t Bus::Write(Word addr, Word value) {
  if (INBOUNDS(addr, RAMBASE, RAMBASE + this->ram->Size())) {
    return this->ram->Write(addr - RAMBASE, value);
  }
  return EXC_LOAD_ACC_FAULT;
}
