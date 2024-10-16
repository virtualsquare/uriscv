// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_BASIC_TYPES_H
#define BASE_BASIC_TYPES_H

// Not everyone has stdint.h appearently. Guess who lags behind?
#ifndef _MSC_VER
// Just use C99 `stdint.h'. Should be "anywhere it matters".
#include <stdint.h>
#else
#include <boost/cstdint.hpp>
// Pollute namespace as need arises!
using boost::int32_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::uint8_t;
#endif

#include <cstddef>
using std::size_t;

#endif // BASE_BASIC_TYPES_H
