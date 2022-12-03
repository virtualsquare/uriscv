#ifndef URISCV_BUS_H
#define URISCV_BUS_H

#include "uriscv/config.h"
#include "uriscv/device.h"

class RAM {
public:
  // size as DEFAULT_RAM_SIZE * FRAMESIZE
  // (so every frame, not every mem cell per frame)
  RAM(Word size, uint8_t *memory);
  RAM(Word size, uint8_t *memory, Word memorySize, Word offset);
  virtual ~RAM(){};

  exception_t Read(Word addr, Word *dst) const;
  exception_t Write(Word addr, Word value);
  Word Size();

private:
  uint8_t *memory;
  Word size;
};

class MyBus {
public:
  MyBus(Config *config);
  virtual ~MyBus(){};

  exception_t Read(Word addr, Word *dst);
  exception_t Write(Word addr, Word value);

  exception_t BusRegRead(Word addr, Word *dst);
  exception_t BusRegWrite(Word addr, Word value);

private:
  RAM *ram;
  RAM *biosdata;

  // device handling & interrupt generation tables
  Device *devTable[DEVINTUSED][DEVPERINT];
  Word instDevTable[DEVINTUSED];
};

#endif
