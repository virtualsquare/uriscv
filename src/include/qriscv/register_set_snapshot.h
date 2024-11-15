// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_REGISTER_SET_SNAPSHOT_H
#define QRISCV_REGISTER_SET_SNAPSHOT_H

#include <boost/function.hpp>
#include <vector>

#include <QAbstractItemModel>
#include <QFont>

#include "uriscv/processor.h"
#include "uriscv/types.h"

const int kNumRelevantCSRRegisters = 40;

class RegisterSetSnapshot : public QAbstractItemModel {
  Q_OBJECT

public:
  enum { COL_REGISTER_MNEMONIC = 0, COL_REGISTER_VALUE, N_COLUMNS };

  RegisterSetSnapshot(Word cpuId, QObject *parent = 0);

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant data(const QModelIndex &index, int role) const;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

  Qt::ItemFlags flags(const QModelIndex &index) const;

private:
  static const int kNumRegisterTypes = 3;

  enum RegisterType { RT_GENERAL = 1, RT_CSR = 2, RT_OTHER = 3 };

  struct SpecialRegisterInfo {
    SpecialRegisterInfo(const char *str, boost::function<Word()> get)
        : name(str), getter(get), value(0) {}
    SpecialRegisterInfo(const char *str, boost::function<Word()> get,
                        boost::function<void(Word)> set)
        : name(str), getter(get), setter(set), value(0) {}
    const char *name;
    boost::function<Word()> getter;
    boost::function<void(Word)> setter;
    Word value;
  };

  static const char *const headers[N_COLUMNS];

  static const char *const registerTypeNames[kNumRegisterTypes];

  const Word cpuId;
  Processor *cpu;

  Word gprCache[Processor::kNumCPURegisters];
  Word csrCache[kNumRelevantCSRRegisters];
  std::vector<SpecialRegisterInfo> sprCache;

  QFont topLevelFont;

private Q_SLOTS:
  void reset();
  void updateCache();
};

#endif // QRISCV_REGISTER_SET_SNAPSHOT_H
