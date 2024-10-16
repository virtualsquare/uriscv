// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_HEX_VIEW_PRIV_H
#define QRISCV_HEX_VIEW_PRIV_H

#include <QWidget>

class HexView;

class HexViewMargin : public QWidget {
  Q_OBJECT

public:
  static const int kLeftPadding = 3;
  static const int kRightPadding = 5;

  HexViewMargin(HexView *hexView);

  virtual QSize sizeHint() const;

protected:
  void paintEvent(QPaintEvent *event);
  void wheelEvent(QWheelEvent *event);

private:
  HexView *const hexView;
};

#endif // QRISCV_HEX_VIEW_PRIV_H
