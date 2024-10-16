// SPDX-FileCopyrightText: 2010, 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_REGISTER_ITEM_DELEGATE_H
#define QRISCV_REGISTER_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

#include "uriscv/types.h"

class RegisterItemDelegate : public QStyledItemDelegate {
public:
  RegisterItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

  virtual QString displayText(const QVariant &value,
                              const QLocale &locale) const;

  virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

  virtual void updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const;

protected:
  virtual QString Text(Word value) const = 0;
};

class RIDelegateHex : public RegisterItemDelegate {
public:
  RIDelegateHex(QObject *parent = 0) : RegisterItemDelegate(parent) {}

  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const;

protected:
  virtual QString Text(Word value) const {
    return QString("0x%1").arg(value, 8, 16, QLatin1Char('0'));
  }
};

class RIDelegateSignedDecimal : public RegisterItemDelegate {
public:
  RIDelegateSignedDecimal(QObject *parent = 0) : RegisterItemDelegate(parent) {}

  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const;

protected:
  virtual QString Text(Word value) const {
    return QString::number((SWord)value, 10);
  }
};

class RIDelegateUnsignedDecimal : public RegisterItemDelegate {
public:
  RIDelegateUnsignedDecimal(QObject *parent = 0)
      : RegisterItemDelegate(parent) {}

  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const;

protected:
  virtual QString Text(Word value) const { return QString::number(value, 10); }
};

class RIDelegateBinary : public RegisterItemDelegate {
public:
  RIDelegateBinary(QObject *parent = 0) : RegisterItemDelegate(parent) {}

  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const;

  virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const;

protected:
  virtual QString Text(Word value) const;
};

#endif // QRISCV_REGISTER_ITEM_DELEGATE_H
