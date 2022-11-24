#ifndef URISCV_BUS_H
#define URISCV_BUS_H

#include "config.h"
#include "const.h"
#include "types.h"

class RAM {
public:
  // size as DEFAULT_RAM_SIZE * FRAMESIZE
  // (so every frame, not every mem cell per frame)
  RAM(Word size, uint8_t *ram);
  virtual ~RAM(){};

  Word Read(Word addr) const;
  void Write(Word addr, Word value);
  Word Size() const;

private:
  uint8_t *memory;
  Word size;
};

class Bus {
public:
  Bus(Config *config);
  virtual ~Bus(){};

  bool Read(Word addr, Word *dst);

private:
  RAM *ram;
};

#endif
