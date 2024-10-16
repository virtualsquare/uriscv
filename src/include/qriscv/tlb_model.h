// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TLB_MODEL_H
#define QRISCV_TLB_MODEL_H

#include <sigc++/sigc++.h>

#include <QAbstractTableModel>

#include "uriscv/types.h"

class Processor;

class TLBModel : public QAbstractTableModel, public sigc::trackable {
  Q_OBJECT

public:
  enum Column { COLUMN_PTE_HI, COLUMN_PTE_LO, N_COLUMNS };

  TLBModel(Word cpuId, QObject *parent = 0);

  Qt::ItemFlags flags(const QModelIndex &index) const;

  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

private Q_SLOTS:
  void onMachineReset();

private:
  static const char *const detailsTemplate;

  void onTLBChanged(unsigned int index);
  QString tlbEntryDetails(unsigned int index) const;

  const Word cpuId;
  Processor *cpu;
};

#endif // QRISCV_TLB_MODEL_H
