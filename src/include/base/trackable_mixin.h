// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_TRACKABLE_MIXIN_H
#define BASE_TRACKABLE_MIXIN_H

#include <list>
#include <sigc++/sigc++.h>

class TrackableMixin {
public:
  ~TrackableMixin();

  void RegisterSigc(sigc::connection c);

private:
  std::list<sigc::connection> connections;
};

#endif // BASE_TRACKABLE_MIXIN_H
