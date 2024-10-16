// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QAction>
#include <QCheckBox>
#include <QGridLayout>

#include "qriscv/stop_mask_view.h"

StopMaskView::StopMaskView(const std::map<StopCause, QAction *> &actions,
                           QWidget *parent)
    : QGroupBox("Stop Mask", parent) {
  QGridLayout *layout = new QGridLayout;

  std::map<StopCause, QAction *>::const_iterator it;
  int col = 0;
  for (it = actions.begin(); it != actions.end(); ++it) {
    QAction *action = it->second;
    QCheckBox *cb = new QCheckBox(action->text());
    cb->setChecked(action->isChecked());
    connect(action, SIGNAL(triggered(bool)), cb, SLOT(setChecked(bool)));
    connect(cb, SIGNAL(toggled(bool)), action, SLOT(setChecked(bool)));
    layout->addWidget(cb, 0, col++, Qt::AlignLeft);
  }
  --col;
  layout->setColumnStretch(col, 1);
  layout->setHorizontalSpacing(11);

  setLayout(layout);
}
