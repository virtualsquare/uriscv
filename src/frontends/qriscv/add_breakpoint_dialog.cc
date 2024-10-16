// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/add_breakpoint_dialog.h"

#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QTreeView>

#include "qriscv/address_line_edit.h"
#include "qriscv/application.h"
#include "qriscv/symbol_table_model.h"
#include "uriscv/symbol_table.h"

AddBreakpointDialog::AddBreakpointDialog(QWidget *parent)
    : QDialog(parent), stab(Appl()->getDebugSession()->getSymbolTable()) {
  QGridLayout *layout = new QGridLayout(this);

  layout->addWidget(new QLabel("ASID:"), 0, 0);
  asidEditor = new AsidLineEdit;
  layout->addWidget(asidEditor, 0, 1);
  asidEditor->setMinimumWidth(asidEditor->fontMetrics().horizontalAdvance("0") *
                              5);
  asidEditor->setMaximumWidth(asidEditor->fontMetrics().horizontalAdvance("0") *
                              6);

  layout->setColumnMinimumWidth(2, 12);

  layout->addWidget(new QLabel("Address:"), 0, 3);
  addressEditor = new AddressLineEdit;
  layout->addWidget(addressEditor, 0, 4);

  QAbstractTableModel *stabModel = new SymbolTableModel(this);
  proxyModel = new SortFilterSymbolTableModel(Symbol::TYPE_FUNCTION, this);
  proxyModel->setSourceModel(stabModel);

  QTreeView *symbolTableView = new QTreeView;
  symbolTableView->setSortingEnabled(true);
  symbolTableView->sortByColumn(0, Qt::AscendingOrder);
  symbolTableView->setAlternatingRowColors(true);
  symbolTableView->setModel(proxyModel);
  symbolTableView->resizeColumnToContents(0);
  symbolTableView->hideColumn(SymbolTableModel::COLUMN_END_ADDRESS);

  connect(
      symbolTableView->selectionModel(),
      SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      this, SLOT(onSelectionChanged(const QItemSelection &)));

  layout->addWidget(symbolTableView, 1, 0, 1, 5);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  layout->addWidget(buttonBox, 2, 0, 1, 5);

  setWindowTitle("Add Breakpoint");
  resize(kInitialWidth, kInitialHeight);

  // connect(addressEditor, SIGNAL(textChanged(const QString&)), this,
  // SLOT(debugg()));
}

Word AddBreakpointDialog::getStartAddress() const {
  return addressEditor->getAddress();
}

Word AddBreakpointDialog::getASID() const { return asidEditor->getAsid(); }

void AddBreakpointDialog::onSelectionChanged(const QItemSelection &selected) {
  QModelIndexList indexes = selected.indexes();
  if (!indexes.isEmpty()) {
    int row = proxyModel->mapToSource(indexes[0]).row();
    const Symbol *symbol = stab->Get(row);
    addressEditor->setAddress(symbol->getStart());
    asidEditor->setAsid(Appl()->getConfig()->getSymbolTableASID());
  }
}
