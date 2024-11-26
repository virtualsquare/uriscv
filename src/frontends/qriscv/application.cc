// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/application.h"

#include <cassert>

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include "qriscv/monitor_window.h"
#include "uriscv/error.h"

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv), settings("uriscv"), config(NULL) {
  setApplicationName("µRISC-V");
  setWindowIcon(QIcon(":/icons/uriscv-48.svg"));

  dbgSession.reset(new DebugSession);

  monitorWindow.reset(new MonitorWindow);
  monitorWindow->show();

  if (argc > 1)
    LoadConfig(argv[1]);
}

Application::~Application() {}

void Application::CreateConfig(const QString &path) {
  MachineConfig *newConfig;
  try {
    newConfig = MachineConfig::Create(QFile::encodeName(path).constData());
  } catch (FileError &e) {
    newConfig = NULL;
    QMessageBox::critical(
        monitorWindow.get(), QString("%1: Error").arg(applicationName()),
        QString("<b>Could not create machine configuration:</b> %1")
            .arg(e.what()));
  }
  if (newConfig)
    setCurrentConfig(path, newConfig);
}

void Application::LoadConfig(const QString &path) {
  std::string error;
  MachineConfig *newConfig =
      MachineConfig::LoadFromFile(QFile::encodeName(path).constData(), error);
  if (newConfig)
    setCurrentConfig(path, newConfig);
  else
    QMessageBox::critical(
        monitorWindow.get(), QString("%1: Error").arg(applicationName()),
        QString("<b>Error loading machine configuration:</b> %1")
            .arg(error.c_str()));
}

void Application::LoadRecentConfig(unsigned int i) {
  QStringList recent = settings.value("RecentFiles").toStringList();
  assert(i < (unsigned int)recent.size());
  LoadConfig(recent[i]);
}

MachineConfig *Application::getConfig() { return config.get(); }

QWidget *Application::getApplWindow() { return monitorWindow.get(); }

QFont Application::getMonospaceFont() {
  QFont monospaceFont("monospace");
  monospaceFont.setStyleHint(QFont::TypeWriter);
  return monospaceFont;
}

QFont Application::getBoldFont() {
  QFont font;
  font.setBold(true);
  return font;
}

void Application::setCurrentConfig(const QString &path,
                                   MachineConfig *newConfig) {
  dbgSession->halt();

  config.reset(newConfig);

  QFileInfo info(path);
  document = info.fileName();
  dir = info.absolutePath();
  QDir::setCurrent(dir);
  QString absolutePath = info.absoluteFilePath();

  QStringList recentFiles = settings.value("RecentFiles").toStringList();
  recentFiles.removeAll(absolutePath);
  recentFiles.prepend(absolutePath);
  while ((unsigned int)recentFiles.size() > kMaxRecentConfigs)
    recentFiles.removeLast();
  settings.setValue("RecentFiles", recentFiles);

  Q_EMIT MachineConfigChanged();
}

Application *Appl() { return static_cast<Application *>(qApp); }
