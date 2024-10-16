// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/trackable_mixin.h"

#include "base/lang.h"

TrackableMixin::~TrackableMixin() {
  for (sigc::connection &c : connections)
    c.disconnect();
}

void TrackableMixin::RegisterSigc(sigc::connection c) {
  connections.push_back(c);
}
