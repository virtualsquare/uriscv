// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_APPLICATION_H
#define QRISCV_APPLICATION_H

#include <QApplication>
#include <QFont>
#include <QSettings>

#include "base/lang.h"
#include "qriscv/debug_session.h"
#include "uriscv/machine.h"
#include "uriscv/machine_config.h"

class MonitorWindow;
class QWidget;

class Application : public QApplication {
  Q_OBJECT

public:
  static const unsigned int kMaxRecentConfigs = 5;

  Application(int &argc, char **argv);
  ~Application();

  MachineConfig *getConfig();
  void CreateConfig(const QString &path);
  void LoadConfig(const QString &path);
  void LoadRecentConfig(unsigned int i);

  DebugSession *getDebugSession() { return dbgSession.get(); }
  QWidget *getApplWindow();

  const QString &getCurrentDir() const { return dir; }

  QFont getMonospaceFont();
  QFont getBoldFont();

  QSettings settings;

  QString document;

Q_SIGNALS:
  void MachineConfigChanged();

private:
  void setCurrentConfig(const QString &path, MachineConfig *newConfig);

  scoped_ptr<DebugSession> dbgSession;

  scoped_ptr<MachineConfig> config;
  QString dir;

  scoped_ptr<MonitorWindow> monitorWindow;
};

Application *Appl();

#define debugSession (Appl()->getDebugSession())

#endif // QRISCV_APPLICATION_H
