// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_LANG_H
#define BASE_LANG_H

// Smart pointers (here we could just as well depend on TR1 and use
// its version of these i guess - the choice is arbitrary).

#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>

using boost::enable_shared_from_this;
using boost::scoped_array;
using boost::scoped_ptr;
using boost::shared_ptr;

// Make the nature of "reference"-type classes excplicit to readers
// and compilers by using the macro below somewhere in a private
// section of a class definition.
#define DISABLE_COPY_AND_ASSIGNMENT(Class)                                     \
  Class(const Class &);                                                        \
  Class &operator=(const Class &)

#define UNUSED_ARG(x) ((void)x)

#endif // BASE_LANG_H
