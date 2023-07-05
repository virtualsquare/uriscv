/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

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
