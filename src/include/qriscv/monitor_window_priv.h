// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MONITOR_WINDOW_PRIV_H
#define QRISCV_MONITOR_WINDOW_PRIV_H

#include <QWidget>

class QLabel;

class StatusDisplay : public QWidget {
  Q_OBJECT

public:
  StatusDisplay(QWidget *parent = 0);

private Q_SLOTS:
  void refreshAll();
  void refreshTod();

private:
  static const int kFieldSpacing = 10;

  QLabel *statusLabel;
  QLabel *todLabel;
};

#endif // QRISCV_MONITOR_WINDOW_PRIV_H
