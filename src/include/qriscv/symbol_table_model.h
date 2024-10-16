// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_SYMBOL_TABLE_MODEL_H
#define QRISCV_SYMBOL_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "uriscv/symbol_table.h"

class SymbolTableModel : public QAbstractTableModel {
  Q_OBJECT

public:
  enum Column {
    COLUMN_SYMBOL,
    COLUMN_START_ADDRESS,
    COLUMN_END_ADDRESS,
    N_COLUMNS
  };

  SymbolTableModel(QObject *parent = 0);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;

private:
  const SymbolTable *const table;
};

class SortFilterSymbolTableModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  SortFilterSymbolTableModel(Symbol::Type tableType, QObject *parent = 0);

  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role = Qt::DisplayRole) const;

protected:
  virtual bool filterAcceptsRow(int sourceRow,
                                const QModelIndex &sourceParent) const;

private:
  const SymbolTable *const table;
  const Symbol::Type tableType;
};

#endif // QRISCV_SYMBOL_TABLE_MODEL_H
