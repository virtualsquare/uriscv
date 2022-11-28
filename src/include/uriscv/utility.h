#ifndef URISCV_UTILITY_H
#define URISCV_UTILITY_H

#include "support/liburiscv/const.h"
#include "support/liburiscv/types.h"
#include <fstream>
#include <iostream>
#include <string>

namespace Utility {
void readFile(std::string filename, char *&dst, Word *size);
// Assumes little endian
void printb(size_t const size, void const *const ptr);
}; // namespace Utility

#endif
