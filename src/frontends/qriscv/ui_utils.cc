// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/ui_utils.h"

#include <QAbstractItemModel>
#include <QTreeView>

#include "uriscv/arch.h"
#include "uriscv/symbol_table.h"

static const char *const kBiosSpaceName = "ExecROM";
static const char *const kBootSpaceName = "BootROM";

const char *GetSymbolicAddress(const SymbolTable *symbolTable, Word asid,
                               Word address, bool onlyFunctions,
                               SWord *offset) {
  if (address >= RAM_BASE) {
    return symbolTable->Probe(asid, address, !onlyFunctions, offset);
  } else if (address >= KSEG0_BOOT_BASE) {
    *offset = address - KSEG0_BOOT_BASE;
    return kBootSpaceName;
  } else {
    *offset = address - KSEG0_BASE;
    return kBiosSpaceName;
  }
}

QString FormatAddress(Word addr) {
  return (QString("0x%1.%2")
              .arg((quint32)(addr >> 16), 4, 16, QChar('0'))
              .arg((quint32)(addr & 0x0000ffffU), 4, 16, QChar('0')));
}

void SetFirstColumnSpanned(QTreeView *treeView, bool setting) {
  QAbstractItemModel *model = treeView->model();
  QModelIndex rootIndex = treeView->rootIndex();
  int rows = model->rowCount(rootIndex);
  for (int i = 0; i < rows; i++)
    treeView->setFirstColumnSpanned(i, rootIndex, setting);
}
