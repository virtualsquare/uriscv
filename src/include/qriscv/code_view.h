// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_CODE_VIEW_H
#define QRISCV_CODE_VIEW_H

#include <boost/function.hpp>
#include <map>
#include <sigc++/sigc++.h>

#include <QPlainTextEdit>

#include "uriscv/types.h"

class QPaintEvent;
class QPixmap;
class CodeViewMargin;
class DebugSession;
class Processor;
class Symbol;
class SymbolTable;
class StoppointSet;
class StoppointListModel;

class CodeView : public QPlainTextEdit, public sigc::trackable {
  Q_OBJECT

public:
  CodeView(Word cpuId);

protected:
  // We need to handle the resizeEvent since we're responsible of
  // resizing our margin.
  void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
  void loadCode();
  void onMachineStopped();
  void updateMargin(const QRect &rect, int dy);
  void reset();

private:
  static const int TAB_STOP_CHARS = 8;

  void paintMargin(QPaintEvent *event);
  void ensureCurrentInstructionVisible();

  void onBreakpointInserted();
  void onBreakpointChanged(size_t);

  QString disassemble(Word instr, Word pc) const;
  QString disasmBranch(Word instr, Word pc) const;
  QString disasmJump(Word instr, Word pc) const;

  CodeViewMargin *codeMargin;

  DebugSession *const dbgSession;
  const Word cpuId;
  Processor *cpu;
  SymbolTable *symbolTable;
  StoppointSet *breakpoints;

  bool codeLoaded;
  Word startPC, endPC;

  StoppointListModel *bplModel;

  QPixmap pcMarkerPixmap;
  QPixmap enabledBpMarkerPixmap;
  QPixmap disabledBpMarkerPixmap;

  typedef boost::function<QString(Word, Word)> DisasmFunc;
  typedef std::map<unsigned int, DisasmFunc> DisasmMap;
  DisasmMap disasmMap;

  friend class CodeViewMargin;
};

#endif // QRISCV_CODE_VIEW_H
