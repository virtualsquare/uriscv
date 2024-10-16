// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_SUSPECT_TYPE_DELEGATE_H
#define QRISCV_SUSPECT_TYPE_DELEGATE_H

#include <QStyledItemDelegate>

class SuspectTypeDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  SuspectTypeDelegate(QWidget *parent = 0);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

private:
  static const unsigned int kValidTypes = 3;

  struct ItemInfo {
    unsigned int value;
    const char *label;
  };

  static ItemInfo valueMap[kValidTypes];
};

#endif // QRISCV_SUSPECT_TYPE_DELEGATE_H
