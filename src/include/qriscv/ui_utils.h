// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_UI_UTILS_H
#define QRISCV_UI_UTILS_H

#include <QString>

#include "uriscv/types.h"

class SymbolTable;
class QTreeView;

const char *GetSymbolicAddress(const SymbolTable *symbolTable, Word asid,
                               Word address, bool onlyFunctions, SWord *offset);

QString FormatAddress(Word address);

void SetFirstColumnSpanned(QTreeView *treeView, bool setting = true);

#endif // QRISCV_UI_UTILS_H
