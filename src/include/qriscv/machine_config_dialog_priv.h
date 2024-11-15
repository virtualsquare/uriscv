// SPDX-FileCopyrightText: 2010, 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MACHINE_CONFIG_DIALOG_PRIV_H
#define QRISCV_MACHINE_CONFIG_DIALOG_PRIV_H

#include <QWidget>

#include "uriscv/arch.h"

class QLineEdit;
class QCheckBox;
class MacIdEdit;

class DeviceFileChooser : public QWidget {
  Q_OBJECT

public:
  DeviceFileChooser(const QString &deviceClassName, const QString &deviceName,
                    unsigned int line, QWidget *parent = 0);

  QString getDeviceFile(unsigned int devNo);
  bool IsDeviceEnabled(unsigned int devNo);

public Q_SLOTS:
  void Save();

private:
  unsigned int il;
  QString deviceName;
  QLineEdit *fileNameEdit[N_DEV_PER_IL];
  QCheckBox *enabledCB[N_DEV_PER_IL];

private Q_SLOTS:
  void browseDeviceFile(int devNo);
};

class NetworkConfigWidget : public QWidget {
  Q_OBJECT

public:
  NetworkConfigWidget(QWidget *parent = 0);

public Q_SLOTS:
  void Save();

private:
  QCheckBox *enabledCB[N_DEV_PER_IL];
  QLineEdit *fileEdit[N_DEV_PER_IL];
  QCheckBox *fixedMacId[N_DEV_PER_IL];
  MacIdEdit *macIdEdit[N_DEV_PER_IL];

private Q_SLOTS:
  void browseDeviceFile(int devNo);
};

#endif // QRISCV_MACHINE_CONFIG_DIALOG_PRIV_H
