// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_HEX_VIEW_H
#define QRISCV_HEX_VIEW_H

#include <QPlainTextEdit>

#include "qriscv/memory_view_delegate.h"
#include "uriscv/arch.h"
#include "uriscv/stoppoint.h"

class HexViewMargin;

class HexView : public QPlainTextEdit, public MemoryViewDelegate {
  Q_OBJECT
  Q_PROPERTY(bool reversedByteOrder READ HasReversedByteOrder WRITE
                 setReversedByteOrder)

public:
  enum ByteOrder { BYTE_ORDER_NATIVE, BYTE_ORDER_REVERSED };

  HexView(Word start, Word end, QWidget *parent = 0);

  bool HasReversedByteOrder() const { return revByteOrder; }
  void setReversedByteOrder(bool setting);

public Q_SLOTS:
  void Refresh();

protected:
  void resizeEvent(QResizeEvent *event);

  bool canInsertFromMimeData(const QMimeData *source) const;
  void insertFromMimeData(const QMimeData *source);

  void keyPressEvent(QKeyEvent *event);
  void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
  void updateMargin(const QRect &rect, int dy);
  void onCursorPositionChanged();

private:
  enum {
    COL_HI_NIBBLE = 0,
    COL_LO_NIBBLE = 1,
    COL_SPACING = 2,
    N_COLS_PER_BYTE
  };

  static const unsigned int kWordsPerRow = 2;
  static const unsigned int kCharsPerWord = WS * N_COLS_PER_BYTE;
  static const unsigned int kCharsPerRow = kWordsPerRow * kCharsPerWord;
  static const unsigned int kHorizontalSpacing = 1;

  static const unsigned int kInvalidLocationChar = 0x2592;

  unsigned int currentWord(const QTextCursor &cursor = QTextCursor()) const;
  unsigned int currentByte(const QTextCursor &cursor = QTextCursor()) const;
  unsigned int currentNibble(const QTextCursor &cursor = QTextCursor()) const;

  unsigned char byteValue(unsigned int word, unsigned int byte) const;
  Word dataAtCursor() const;

  void moveCursor(QTextCursor::MoveOperation operation, int n = 1);
  void setPoint(unsigned int word, unsigned int byte = 0,
                unsigned int nibble = 0);

  void paintMargin(QPaintEvent *event);
  void highlightWord();

  friend class HexViewMargin;

  const Word start;
  const Word end;
  const Word length;

  bool revByteOrder;

  const QString invalidByteRepr;

  HexViewMargin *margin;
};

#endif // QRISCV_HEX_VIEW_H
