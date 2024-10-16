// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TERMINAL_WINDOW_PRIV_H
#define QRISCV_TERMINAL_WINDOW_PRIV_H

#include <sigc++/sigc++.h>

#include <QIcon>
#include <QWidget>

class QLabel;
class QPushButton;
class TerminalDevice;
class QCheckBox;

class TerminalStatusWidget : public QWidget, public sigc::trackable {
  Q_OBJECT

public:
  TerminalStatusWidget(TerminalDevice *terminal, QWidget *parent = 0);

private:
  // We _have_ to set the minimumSize() property on dynamic labels
  // in resizable containers; the constant is here mainly as a
  // remainder for that :-)
  static const int kStatusLabelsMinimumWidth = 16;

  void updateStatus();
  void onConditionChanged(bool isWorking);

  TerminalDevice *const terminal;

  bool expanded;
  QWidget *statusAreaWidget;

  QLabel *rxStatusLabel;
  QLabel *rxCompletionTime;
  QLabel *txStatusLabel;
  QLabel *txCompletionTime;

  QIcon expandedIcon;
  QIcon collapsedIcon;

  QPushButton *expanderButton;

  QCheckBox *hwFailureCheckBox;

private Q_SLOTS:
  void onHardwareFailureButtonClicked(bool checked);
  void onExpanderButtonClicked();
};

#endif // QRISCV_TERMINAL_WINDOW_PRIV_H
