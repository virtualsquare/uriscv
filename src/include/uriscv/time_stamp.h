// SPDX-FileCopyrightText: 2004 Mauro Morsiani
// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_TIME_STAMP_H
#define URISCV_TIME_STAMP_H

#include <string>

#include "base/basic_types.h"

namespace TimeStamp {

inline uint32_t getHi(uint64_t ts) { return ts >> 32; }

inline uint32_t getLo(uint64_t ts) { return (uint32_t)ts; }

inline void setHi(uint64_t &ts, uint32_t value) {
  ts = (uint64_t)getLo(ts) | (uint64_t)value << 32;
}

inline void setLo(uint64_t &ts, uint32_t value) {
  ts = (uint64_t)getHi(ts) | (uint64_t)value;
}

std::string toString(uint64_t ts);

} // namespace TimeStamp

#endif // URISCV_TIME_STAMP_H
