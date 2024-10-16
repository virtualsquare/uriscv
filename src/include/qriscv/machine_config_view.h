// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MACHINE_CONFIG_VIEW_H
#define QRISCV_MACHINE_CONFIG_VIEW_H

#include <QWidget>

class QLabel;

class MachineConfigView : public QWidget {
  Q_OBJECT

public:
  MachineConfigView(QWidget *parent = 0);

public Q_SLOTS:
  void Update();

private:
  static QString checkedFileName(const QString &fileName);

  QLabel *numCpusLabel;
  QLabel *clockRateLabel;
  QLabel *tlbSizeLabel;
  QLabel *ramSizeLabel;
  QLabel *ramtopLabel;
  QLabel *tlbFloorAddressLabel;

  QLabel *bootstrapROMLabel;
  QLabel *executionROMLabel;

  QLabel *loadCoreLabel;
  QLabel *coreFileLabel;

  QLabel *stabLabel;
  QLabel *stabAsidLabel;
};

#endif // QRISCV_MACHINE_CONFIG_VIEW_H
