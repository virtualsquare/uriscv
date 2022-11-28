#ifndef URISCV_BUS_H
#define URISCV_BUS_H

#include "config.h"

class RAM {
public:
  // size as DEFAULT_RAM_SIZE * FRAMESIZE
  // (so every frame, not every mem cell per frame)
  RAM(Word size, uint8_t *ram);
  virtual ~RAM(){};

  exception_t Read(Word addr, Word *dst) const;
  exception_t Write(Word addr, Word value);
  Word Size() const;

private:
  uint8_t *memory;
  Word size;
};

class Bus {
public:
  Bus(Config *config);
  virtual ~Bus(){};

  exception_t Read(Word addr, Word *dst);
  exception_t Write(Word addr, Word value);

private:
  RAM *ram;
};

#endif
