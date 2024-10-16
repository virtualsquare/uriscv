// SPDX-FileCopyrightText: 2004 Renzo Davoli
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef URISCV_VDE_NETWORK_H
#define URISCV_VDE_NETWORK_H

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "uriscv/libvdeplug_dyn.h"

class netblockq;

#define PROMISQ 0x4
#define INTERRUPT 0x2
#define NAMED 0x1

unsigned int testnetinterface(const char *name);

class netinterface {
public:
  netinterface(const char *name, const char *addr, int intnum);

  ~netinterface(void);

  unsigned int readdata(char *buf, int len);
  unsigned int writedata(char *buf, int len);
  unsigned int polling();
  void setaddr(char *iethaddr);
  void getaddr(char *pethaddr);
  void setmode(int imode);
  unsigned int getmode();

private:
  VDECONN *vdeconn;
  char ethaddr[6];
  char mode;
  struct pollfd polldata;
  class netblockq *queue;
};

#endif // URISCV_VDE_NETWORK_H
