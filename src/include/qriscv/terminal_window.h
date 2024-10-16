// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TERMINAL_WINDOW_H
#define QRISCV_TERMINAL_WINDOW_H

#include <QMainWindow>

class TerminalDevice;
class QLabel;
class QVBoxLayout;
class TerminalView;
class TerminalStatusWidget;

class TerminalWindow : public QMainWindow {
  Q_OBJECT

public:
  TerminalWindow(unsigned int devNo, QWidget *parent = 0);

protected:
  virtual void closeEvent(QCloseEvent *event);

private Q_SLOTS:
  void onMachineReset();

private:
  static const int kDefaultCols = 60;
  static const int kDefaultRows = 20;

  static TerminalDevice *getTerminal(unsigned int devNo);

  const unsigned int devNo;

  QVBoxLayout *layout;
  TerminalView *terminalView;
  TerminalStatusWidget *statusWidget;
};

#endif // QRISCV_TERMINAL_WINDOW_H
