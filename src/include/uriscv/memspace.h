// SPDX-FileCopyrightText: 2004 Mauro Morsiani
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_MEMSPACE_H
#define URISCV_MEMSPACE_H

#include "base/lang.h"
#include "uriscv/types.h"

// This class implements the RAM device. Any object allows reads and
// writes with random access to word-sized items using appropriate
// methods. Contents may be loaded from file at creation. SystemBus
// must do all bounds checking and address conversion for access

class RamSpace {
public:
  // This method creates a RamSpace object of a given size (in words)
  // and fills it with file contents if needed
  RamSpace(Word size_, const char *fName);

  // This method returns the value of Word at index
  Word MemRead(Word index) const { return ram[index]; }

  // This method allows to write data to a specified address (as word
  // offset). SystemBus must check address validity and make
  // byte-to-word address conversion)
  void MemWrite(Word index, Word data) { ram[index] = data; }

  // This method returns RamSpace size in bytes
  Word Size() const { return size << 2; }

private:
  scoped_array<Word> ram;

  // size of structure in words (C style addressing: [0..size - 1])
  Word size;
};

// This class implements ROM devices. The BIOS or Bootstrap ROM is read
// from a disk file, whose name is specified during object creation.
// Access is read-only, to word-sized items in it. SystemBus must do all
// bounds checking and address conversion for access

class BiosSpace {
public:
  // This method creates a BiosSpace object, filling with .rom file
  // contents
  BiosSpace(const char *name);

  // This method returns the value of Word at ofs address
  // (SystemBus must assure that ofs is in range)
  Word MemRead(Word ofs);

  // This method returns BiosSpace size in bytes
  Word Size();

private:
  scoped_array<Word> memPtr;

  // size of structure in Words (C style addressing: [0..size - 1])
  Word size;
};

#endif // URISCV_MEMSPACE_H
