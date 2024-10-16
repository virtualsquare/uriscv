// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_BOOLEAN_ITEM_DELEGATE_H
#define QRISCV_BOOLEAN_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

class BooleanItemDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  BooleanItemDelegate(QObject *parent = 0);

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const;

  // We reimplement `editorEvent' as the implementation in
  // QStyledItemDelegate isn't of any use for us (i.e. we don't want
  // a default editor!).
  bool editorEvent(QEvent *event, QAbstractItemModel *model,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index);

private:
  static QRect buttonGeometry(const QRect &viewItemRect);
};

#endif // QRISCV_BOOLEAN_ITEM_DELEGATE_H
