// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_MACHINE_CONFIG_DIALOG_H
#define QRISCV_MACHINE_CONFIG_DIALOG_H

#include <QDialog>

#include "uriscv/machine_config.h"

class QListWidget;
class QStackedLayout;
class QSpinBox;
class QComboBox;
class QLineEdit;
class QCheckBox;
class AsidLineEdit;

class MachineConfigDialog : public QDialog {
  Q_OBJECT

public:
  MachineConfigDialog(MachineConfig *config, QWidget *parent = 0);

private:
  QWidget *createGeneralTab();
  QWidget *createDeviceTab();
  void registerDeviceClass(const QString &label, const QString &icon,
                           unsigned int devClassIndex,
                           const QString &deviceClassName,
                           const QString &deviceName, bool selected = false);

  void setFileBrowser(QPushButton *button,
                      std::function<void(QString)> callback);

  MachineConfig *const config;

  QSpinBox *cpuSpinner;
  QSpinBox *clockRateSpinner;
  QComboBox *tlbSizeList;
  QComboBox *tlbFloorAddressList;
  QSpinBox *ramSizeSpinner;
  QCheckBox *coreBootCheckBox;
  AsidLineEdit *stabAsidEdit;

  struct {
    const char *description;
    QLineEdit *lineEdit;
  } romFileInfo[N_ROM_TYPES];

  QListWidget *devClassView;
  QStackedLayout *devFileChooserStack;

private Q_SLOTS:
  void getROMFileName(int index);

  void onDeviceClassChanged();

  void saveConfigChanges();
};

#endif // QRISCV_MACHINE_CONFIG_DIALOG_H
