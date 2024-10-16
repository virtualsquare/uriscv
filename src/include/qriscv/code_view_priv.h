// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_CODE_VIEW_PRIV_H
#define QRISCV_CODE_VIEW_PRIV_H

#include <QWidget>

#include "uriscv/types.h"

class CodeView;

class CodeViewMargin : public QWidget {
  Q_OBJECT

public:
  static const int kMarkerSize = 16;

  CodeViewMargin(CodeView *codeView);

  virtual QSize sizeHint() const;

protected:
  bool event(QEvent *event);
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);

  // For some reason we have to proxy this to QAbstractScollArea
  // _explicitely_! Cannot figure out why, since by default the
  // parent should handle it automatically if ignored.
  void wheelEvent(QWheelEvent *event);

private:
  int indexAt(const QPoint &pos) const;

  CodeView *const codeView;
};

#endif // QRISCV_CODE_VIEW_PRIV_H
