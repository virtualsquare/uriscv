/*
 * uRISCV - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include "qriscv/register_set_snapshot.h"

#include <boost/bind/bind.hpp>
#include <cassert>

#include "base/lang.h"
#include "qriscv/application.h"
#include "uriscv/disassemble.h"
#include "uriscv/systembus.h"

using namespace boost::placeholders;

const char *const RegisterSetSnapshot::headers[RegisterSetSnapshot::N_COLUMNS] =
    {"Register", "Value"};

const char *const RegisterSetSnapshot::registerTypeNames
    [RegisterSetSnapshot::kNumRegisterTypes] = {
        "CPU Registers", "CSR Registers", "Other Registers"};

const int relevantCsrRegisters[kNumRelevantCSRRegisters] = {
    CYCLE,       MCYCLE,      CYCLEH,    MCYCLEH,    TIME,  TIMEH,
    INSTRET,     MINSTRET,    INSTRETH,  MINSTRETH,  UTVEC, SEDELEG,
    MEDELEG,     SIDELEG,     MIDELEG,   STVEC,      MTVEC, USCRATCH,
    SSCRATCH,    MSCRATCH,    UEPC,      SEPC,       MEPC,  UCAUSE,
    SCAUSE,      MCAUSE,      UTVAL,     STVAL,      MTVAL, MSTATUS,
    CSR_ENTRYLO, CSR_ENTRYHI, CSR_INDEX, CSR_RANDOM, UIE,   UIP,
    SIE,         SIP,         MIE,       MIP};

RegisterSetSnapshot::RegisterSetSnapshot(Word cpuId, QObject *parent)
    : QAbstractItemModel(parent), cpuId(cpuId) {
  connect(debugSession, SIGNAL(MachineStopped()), this, SLOT(updateCache()));
  connect(debugSession, SIGNAL(MachineRan()), this, SLOT(updateCache()));
  connect(debugSession, SIGNAL(DebugIterationCompleted()), this,
          SLOT(updateCache()));

  connect(debugSession, SIGNAL(MachineReset()), this, SLOT(reset()));

  topLevelFont.setBold(true);

  for (Word &v : gprCache)
    v = 0;
  for (Word &v : csrCache)
    v = 0;

  reset();
}

QModelIndex RegisterSetSnapshot::index(int row, int column,
                                       const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent) || column >= N_COLUMNS)
    return QModelIndex();

  if (parent.isValid()) {
    switch (parent.row() + 1) {
    case RT_GENERAL:
      if ((unsigned int)row < Processor::kNumCPURegisters)
        return createIndex(row, column, (quint32)RT_GENERAL);
      break;
    case RT_CSR:
      if ((unsigned int)row < kNumRelevantCSRRegisters)
        return createIndex(row, column, (quint32)RT_CSR);
      break;
    case RT_OTHER:
      if ((unsigned int)row < sprCache.size())
        return createIndex(row, column, (quint32)RT_OTHER);
      break;
    }
  } else if (row < kNumRegisterTypes) {
    return createIndex(row, column, (quintptr)0);
  }

  // Fallback case - clearly something bogus
  return QModelIndex();
}

QModelIndex RegisterSetSnapshot::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return QModelIndex();

  if (index.internalId() == 0)
    return QModelIndex();
  else
    return createIndex(index.internalId() - 1, 0, (quintptr)0);
}

int RegisterSetSnapshot::rowCount(const QModelIndex &parent) const {
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid()) {
    // It's the root index, so return the number of toplevel
    // items.
    return kNumRegisterTypes;
  } else if (parent.internalId() == 0) {
    // 2nd level items
    switch (parent.row() + 1) {
    case RT_GENERAL:
      return Processor::kNumCPURegisters;
    case RT_CSR:
      return kNumRelevantCSRRegisters;
    case RT_OTHER:
      return sprCache.size();
    default:
      // Assert not reached
      assert(0);
      // Error
      return -1;
    }
  } else {
    // Leaf items have no children.
    return 0;
  }
}

int RegisterSetSnapshot::columnCount(const QModelIndex &parent) const {
  UNUSED_ARG(parent);
  return N_COLUMNS;
}

QVariant RegisterSetSnapshot::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return headers[section];
  else
    return QVariant();
}

QVariant RegisterSetSnapshot::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (index.internalId() == 0) {
    if (index.column() != 0)
      return QVariant();
    if (role == Qt::DisplayRole)
      return registerTypeNames[index.row()];
    if (role == Qt::FontRole)
      return topLevelFont;
  } else {
    if (role == Qt::DisplayRole) {
      switch (index.column()) {
      case COL_REGISTER_MNEMONIC:
        switch (index.internalId()) {
        case RT_GENERAL:
          return RegName(index.row());
        case RT_CSR:
          return CSRRegName(relevantCsrRegisters[index.row()]);
        case RT_OTHER:
          return sprCache[index.row()].name;
        default:
          return QVariant();
        }

      case COL_REGISTER_VALUE:
        switch (index.internalId()) {
        case RT_GENERAL:
          return gprCache[index.row()];
        case RT_CSR:
          return csrCache[index.row()];
          break;
        case RT_OTHER:
          return sprCache[index.row()].value;
          break;
        default:
          return QVariant();
        }

      default:
        // Assert not reached
        assert(0);
      }
    } else if (role == Qt::FontRole) {
      return Appl()->getMonospaceFont();
    }
  }

  return QVariant();
}

bool RegisterSetSnapshot::setData(const QModelIndex &index,
                                  const QVariant &variant, int role) {
  if (!(index.isValid() && role == Qt::EditRole && variant.canConvert<Word>() &&
        index.internalId() && index.column() == COL_REGISTER_VALUE)) {
    return false;
  }

  int r = index.row();

  switch (index.internalId()) {
  case RT_GENERAL:
    cpu->setGPR(r, variant.value<Word>());
    if (gprCache[r] != (Word)cpu->getGPR(r)) {
      gprCache[r] = cpu->getGPR(r);
      Q_EMIT dataChanged(index, index);
    }
    break;

  case RT_CSR:
    cpu->csrWrite(relevantCsrRegisters[r], variant.value<Word>());
    if (csrCache[r] != cpu->csrRead(relevantCsrRegisters[r])) {
      csrCache[r] = cpu->csrRead(relevantCsrRegisters[r]);
      Q_EMIT dataChanged(index, index);
    }
    break;

  case RT_OTHER:
    if (sprCache[r].setter) {
      sprCache[r].setter(variant.value<Word>());
      if (sprCache[r].value != sprCache[r].getter()) {
        sprCache[r].value = sprCache[r].getter();
        Q_EMIT dataChanged(index, index);
      }
    }
    break;

  default:
    // Assert not reached
    assert(0);
  }

  return true;
}

Qt::ItemFlags RegisterSetSnapshot::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::NoItemFlags;

  if (index.internalId() == 0)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  switch (index.column()) {
  case COL_REGISTER_MNEMONIC:
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  case COL_REGISTER_VALUE:
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  default:
    // Assert not reached
    assert(0);
  }

  return Qt::NoItemFlags;
}

void RegisterSetSnapshot::reset() {
  cpu = debugSession->getMachine()->getProcessor(cpuId);

  sprCache.clear();
  sprCache.push_back(
      SpecialRegisterInfo("nextPC", boost::bind(&Processor::getNextPC, cpu),
                          boost::bind(&Processor::setNextPC, cpu, _1)));
  sprCache.push_back(
      SpecialRegisterInfo("succPC", boost::bind(&Processor::getSuccPC, cpu),
                          boost::bind(&Processor::setSuccPC, cpu, _1)));
  sprCache.push_back(SpecialRegisterInfo(
      "prevPhysPC", boost::bind(&Processor::getPrevPPC, cpu)));
  sprCache.push_back(SpecialRegisterInfo(
      "currPhysPC", boost::bind(&Processor::getCurrPPC, cpu)));

  SystemBus *bus = debugSession->getMachine()->getBus();
  sprCache.push_back(
      SpecialRegisterInfo("Timer", boost::bind(&SystemBus::getTimer, bus),
                          boost::bind(&SystemBus::setTimer, bus, _1)));

  updateCache();
}

void RegisterSetSnapshot::updateCache() {
  for (unsigned int i = 0; i < Processor::kNumCPURegisters; i++) {
    Word value = cpu->getGPR(i);
    if (gprCache[i] != value) {
      gprCache[i] = value;
      QModelIndex index = createIndex(i, COL_REGISTER_VALUE, RT_GENERAL);
      Q_EMIT dataChanged(index, index);
    }
  }

  for (unsigned int i = 0; i < kNumRelevantCSRRegisters; i++) {
    Word value = cpu->csrRead(relevantCsrRegisters[i]);
    if (csrCache[i] != value) {
      csrCache[i] = value;
      QModelIndex index = createIndex(i, COL_REGISTER_VALUE, RT_CSR);
      Q_EMIT dataChanged(index, index);
    }
  }

  int row = 0;
  for (SpecialRegisterInfo &sr : sprCache) {
    Word value = sr.getter();
    if (value != sr.value) {
      sr.value = value;
      QModelIndex index = createIndex(row, COL_REGISTER_VALUE, RT_OTHER);
      Q_EMIT dataChanged(index, index);
    }
    row++;
  }
}
