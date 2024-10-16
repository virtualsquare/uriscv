// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
// SPDX-FileCopyrightText: 2020 Mattia Biondi
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_PROCESSOR_WINDOW_H
#define QRISCV_PROCESSOR_WINDOW_H

#include <QMainWindow>

#include "uriscv/types.h"

class QToolBar;
class Processor;
class DebugSession;
class QLabel;
class CodeView;
class QLayout;
class QDockWidget;
class RegisterSetWidget;

class ProcessorWindow : public QMainWindow {
  Q_OBJECT

public:
  ProcessorWindow(Word cpuId, QWidget *parent = 0);

protected:
  virtual void closeEvent(QCloseEvent *event);

private:
  void createMenu();
  void createToolBar();
  QLayout *createInstrPanel();
  void createDockableWidgets();

  DebugSession *const dbgSession;
  const Word cpuId;
  Processor *cpu;

  QToolBar *toolBar;
  QLabel *statusLabel;
  CodeView *codeView;

  QLabel *prevPCLabel;
  QLabel *prevInstructionLabel;
  QLabel *pcLabel;
  QLabel *instructionLabel;

  QLabel *bdIndicator;
  QLabel *ldIndicator;

  RegisterSetWidget *regView;
  QDockWidget *tlbWidget;

private Q_SLOTS:
  void onMachineReset();
  void updateStatusInfo();
  void updateTLBViewTitle(bool topLevel);
};

#endif // QRISCV_PROCESSOR_WINDOW_H
