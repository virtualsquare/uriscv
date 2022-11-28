#include "uriscv/utility.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

void Utility::readFile(std::string filename, char *&dst, Word *size) {
  std::ifstream file(filename,
                     std::ios::in | std::ifstream::ate | std::ios::binary);
  std::string buffer, line;
  if (file.is_open()) {
    file.seekg(0, file.end);
    *size = file.tellg();
    file.seekg(0, file.beg);
    dst = new char[*size];
    file.read(dst, *size);
    file.close();
  } else {
    std::cout << "File not found!\n";
  }
}
void Utility::printb(size_t const size, void const *const ptr) {
  unsigned char *b = (unsigned char *)ptr;
  unsigned char byte;
  int i, j;

  for (i = size - 1; i >= 0; i--) {
    for (j = 7; j >= 0; j--) {
      byte = (b[i] >> j) & 1;
      printf("%u", byte);
    }
  }
  puts("");
}
