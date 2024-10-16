// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MEMORY_VIEW_DELEGATE_H
#define QRISCV_MEMORY_VIEW_DELEGATE_H

class MemoryViewDelegate {
public:
  virtual void Refresh() = 0;
};

#endif // QRISCV_MEMORY_VIEW_DELEGATE_H
