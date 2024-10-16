// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_STOP_MASK_VIEW_H
#define QRISCV_STOP_MASK_VIEW_H

#include <QGroupBox>
#include <map>

#include "uriscv/machine.h"

class QWidget;
class QAction;

class StopMaskView : public QGroupBox {
  Q_OBJECT

public:
  StopMaskView(const std::map<StopCause, QAction *> &actions,
               QWidget *parent = 0);
};

#endif // QRISCV_STOP_MASK_VIEW_H
