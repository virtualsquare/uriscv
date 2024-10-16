// SPDX-FileCopyrightText: 2004 Mauro Morsiani
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uriscv/blockdev_params.h"
#include "uriscv/const.h"
#include "uriscv/types.h"
#include <fstream>

// This class implements the block devices' 4096 byte sectors/flash devices
// blocks. Each object contains a single buffer; methods are provided to
// read/write these blocks from/to real files and to access to the word-sized
// contents. This class is provided primarily to make DMA transfer easier and to
// standardize block handling.

class Block {
public:
  // This method returns an empty (unitialized) 4096 byte Block
  Block();

  // Object deletion is done by default handler

  // This method fills a Block with file contents starting at "offset"
  // bytes from file start, as computed by caller.
  // Returns TRUE if read does not succeed, FALSE otherwise
  bool ReadBlock(FILE *blkFile, SWord offset);

  // This method writes Block contents in a file, starting at "offset"
  // bytes from file start, as computed by caller. Returns TRUE if
  // write does not succeed, FALSE otherwise
  bool WriteBlock(FILE *blkFile, SWord offset);

  // This method returns the Word contained in the Block at ofs (Word
  // items) offset, range [0..BLOCKSIZE - 1]. Warning: in-bounds
  // checking is leaved to caller
  Word getWord(unsigned int ofs);

  // This method fills with "value" the Word contained in the Block at
  // ofs (Word items) offset, range [0..BLOCKSIZE - 1]. Warning:
  // in-bounds checking is leaved to caller
  void setWord(unsigned int ofs, Word value);

private:
  // Block contents
  Word blkBuf[BLOCKSIZE];
};

/****************************************************************************/

// This class contains the simulated disk drive geometry and performance
// parameters. They are filled by mkdev utility and used by DiskDevice class
// for detailed disk performance simulation.
// Position, min, max and default values, where applicable, are defined in
// h/blockdev.h header file.
//
// Parameters are:
// number of cylinders;
// number of heads;
// number of sectors per track;
// disk rotation time in microseconds;
// average track-to-track seek time in microseconds;
// data % of sector (to compute inter-sector gap)

class DiskParams {
public:
  // This method reads disk parameters from file header, builds a
  // DiskParams object, and returns the disk sectors start offset:
  // this allows to modify the parameters' size without changing the
  // caller.  If fileOfs returned is 0, something has gone wrong; file
  // is rewound after use
  DiskParams(FILE *diskFile, SWord *fileOfs);

  // Object deletion is done by default handler

  // These methods return the corresponding geometry or performance
  // figure
  unsigned int getCylNum(void);
  unsigned int getHeadNum(void);
  unsigned int getSectNum(void);
  unsigned int getRotTime(void);
  unsigned int getSeekTime(void);
  unsigned int getDataSect(void);

private:
  // parameter buffer
  unsigned int parms[DISKPNUM];
};

// This class contains the simulated flash device geometry and performance
// parameters. They are filled by mkdev utility and used by FlashDevice class
// for detailed flash device performance simulation.
// Position, min, max and default values, where applicable, are defined in
// h/blockdev.h header file.
//
// Parameters are:
// number of blocks;
// average read/write time in microseconds;

class FlashParams {
public:
  // This method reads flash device parameters from file header, builds a
  // FlashParams object, and returns the flash device blocks start offset:
  // this allows to modify the parameters' size without changing the
  // caller. If fileOfs returned is 0, something has gone wrong; file
  // is rewound after use
  FlashParams(FILE *flashFile, SWord *fileOfs);

  // Object deletion is done by default handler

  // These methods return the corresponding geometry or performance
  // figure

  unsigned int getBlocksNum(void);
  unsigned int getWTime(void);

private:
  // parameter buffer
  unsigned int parms[FLASHPNUM];
};
