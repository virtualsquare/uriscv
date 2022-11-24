#include "uriscv/bus.h"
#include "uriscv/const.h"
#include "uriscv/utility.h"

const Word DEFAULT_RAM_SIZE = 64;

RAM::RAM(Word size, uint8_t *memory) {
  this->size = size;
  this->memory = memory;
}

Word RAM::Read(Word addr) const {
  return this->memory[addr] | this->memory[addr + 1] << 8 |
         this->memory[addr + 2] << 16 | this->memory[addr + 3] << 24;
}

void RAM::Write(Word addr, Word value) { this->memory[addr] = value; }
Word RAM::Size() const { return this->size; }

Bus::Bus(Config *config) {
  std::cout << "[-] SystemBus init" << std::endl;
  char *buffer = NULL;
  Word ramSize;
  Utility::readFile(config->getRomPath(), buffer, &ramSize);
  this->ram = new RAM(DEFAULT_RAM_SIZE * FRAMESIZE, (uint8_t *)buffer);
}

bool Bus::Read(Word addr, Word *dst) {
  *dst = 0;
  if (INBOUNDS(addr, RAMBASE, RAMBASE + this->ram->Size())) {
    *dst = this->ram->Read(addr - RAMBASE);
    return false;
  }
  return true;
}
