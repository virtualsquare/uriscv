// SPDX-FileCopyrightText: 2004 Mauro Morsiani
// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uriscv/time_stamp.h"

#include <boost/format.hpp>

namespace TimeStamp {

std::string toString(uint64_t ts) {
  using boost::format;
  using boost::str;
  return str(format("0x%08lx.%08lx") % ((unsigned long)getHi(ts)) %
             ((unsigned long)getLo(ts)));
}

} // namespace TimeStamp
