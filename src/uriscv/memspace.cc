// SPDX-FileCopyrightText: 2004 Mauro Morsiani
//
// SPDX-License-Identifier: GPL-3.0-or-later

/****************************************************************************
 *
 * This module implements the classes which represents memory spaces:
 * RamSpace (for RAM) and BiosSpace (for ROMs), completely under control of
 * SystemBus.
 *
 ****************************************************************************/

#include "uriscv/memspace.h"

#include <stdio.h>
#include <stdlib.h>

#include <boost/format.hpp>

#include "uriscv/arch.h"
#include "uriscv/blockdev_params.h"
#include "uriscv/const.h"
#include "uriscv/error.h"

// This method creates a RamSpace object of a given size (in words) and
// fills it with core file contents if needed
RamSpace::RamSpace(Word size_, const char *fName)
    : ram(new Word[size_]), size(size_) {
  if (fName != NULL && *fName) {
    FILE *cFile;
    if ((cFile = fopen(fName, "r")) == NULL)
      throw FileError(fName);

    // Check validity
    Word tag;
    if (fread((void *)&tag, WORDLEN, 1, cFile) != 1 || tag != COREFILEID) {
      fclose(cFile);
      throw InvalidCoreFileError(fName, "Invalid core file");
    }

    if (fread((void *)ram.get(), WORDLEN, size, cFile) != size)
      if (ferror(cFile)) {
        fclose(cFile);
        throw ReadingError();
      }

    if (!feof(cFile)) {
      fclose(cFile);
      throw CoreFileOverflow();
    }

    fclose(cFile);
  }
}

/****************************************************************************/

// This method creates a BiosSpace object, filling with .rom file contents
BiosSpace::BiosSpace(const char *fileName) {
  assert(fileName != NULL && *fileName);

  FILE *file;

  if ((file = fopen(fileName, "r")) == NULL)
    throw FileError(fileName);

  Word tag;
  if ((fread((void *)&tag, WS, 1, file) != 1) || (tag != BIOSFILEID) ||
      (fread((void *)&size, WS, 1, file) != 1)) {
    fclose(file);
    throw InvalidFileFormatError(fileName, "ROM file expected");
  }

  memPtr.reset(new Word[size]);
  if (fread((void *)memPtr.get(), WS, size, file) != size) {
    fclose(file);
    throw InvalidFileFormatError(fileName, "Wrong ROM file size");
  }

  fclose(file);
}

// This method returns the value of Word at ofs address
// (SystemBus must assure that ofs is in range)
Word BiosSpace::MemRead(Word ofs) {
  assert(ofs < size);
  return memPtr[ofs];
}

// This method returns BiosSpace size in bytes
Word BiosSpace::Size() { return size * WORDLEN; }
