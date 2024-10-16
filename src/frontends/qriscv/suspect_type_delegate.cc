// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/suspect_type_delegate.h"

#include <QComboBox>

#include "base/lang.h"
#include "uriscv/stoppoint.h"

SuspectTypeDelegate::ItemInfo SuspectTypeDelegate::valueMap[] = {
    {AM_WRITE, "Write"}, {AM_READ, "Read"}, {AM_READ_WRITE, "Read/Write"}};

SuspectTypeDelegate::SuspectTypeDelegate(QWidget *parent)
    : QStyledItemDelegate(parent) {}

QWidget *SuspectTypeDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const {
  UNUSED_ARG(option);
  UNUSED_ARG(index);

  QComboBox *editor = new QComboBox(parent);
  for (unsigned int i = 0; i < kValidTypes; i++)
    editor->addItem(valueMap[i].label);
  return editor;
}

void SuspectTypeDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);

  unsigned int value = index.data(Qt::EditRole).toUInt();
  unsigned int i;
  for (i = 0; i < kValidTypes; i++)
    if (value == valueMap[i].value)
      break;
  comboBox->setCurrentIndex(i);
}

void SuspectTypeDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const {
  QComboBox *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, valueMap[comboBox->currentIndex()].value, Qt::EditRole);
}

void SuspectTypeDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option,
    const QModelIndex &index) const {
  editor->setGeometry(option.rect);
}
