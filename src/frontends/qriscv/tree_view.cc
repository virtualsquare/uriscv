// SPDX-FileCopyrightText: 2011 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/tree_view.h"

#include <QHeaderView>

#include "base/lang.h"
#include "qriscv/application.h"
#include "qriscv/ui_utils.h"

TreeView::TreeView(const QString &name, const std::list<int> &resizedToContents,
                   bool persistItemState, QWidget *parent)
    : QTreeView(parent), resizedToContents(resizedToContents),
      persistItemState(persistItemState) {
  setObjectName(name);

  connect(header(), SIGNAL(sectionResized(int, int, int)), this,
          SLOT(sectionResized(int, int, int)));

  if (persistItemState) {
    itemStateKey = QString("%1/ExpandedItems").arg(objectName());
    connect(this, SIGNAL(expanded(const QModelIndex &)), this,
            SLOT(saveItemState()));
    connect(this, SIGNAL(collapsed(const QModelIndex &)), this,
            SLOT(saveItemState()));
  }
}

void TreeView::setModel(QAbstractItemModel *model) {
  QTreeView::setModel(model);

  if (model == NULL)
    return;

  // Not really sure if this should be here, but otoh cannot think
  // of a _single case_ where it would be undesired.
  header()->setSectionsMovable(false);

  bool resizeCols = true;
  for (int i = 0; i < model->columnCount(); ++i) {
    QVariant v = Appl()->settings.value(
        QString("%1/Section%2Size").arg(objectName()).arg(i));
    if (v.canConvert<int>() && v.toInt()) {
      header()->resizeSection(i, v.toInt());
      resizeCols = false;
    }
  }

  if (resizeCols) {
    for (int col : resizedToContents)
      resizeColumnToContents(col);
  }

  if (persistItemState) {
    QVariant var = Appl()->settings.value(itemStateKey);
    for (const QString &s : var.toStringList()) {
      bool ok;
      int row = s.toInt(&ok);
      if (!ok)
        continue;
      QModelIndex idx = model->index(row, 0);
      if (idx.isValid())
        setExpanded(idx, true);
    }
  }
}

void TreeView::sectionResized(int logicalIndex, int oldSize, int newSize) {
  UNUSED_ARG(oldSize);
  Appl()->settings.setValue(
      QString("%1/Section%2Size").arg(objectName()).arg(logicalIndex), newSize);
}

void TreeView::saveItemState() {
  QStringList list;

  int nr = model()->rowCount();
  for (int i = 0; i < nr; i++)
    if (isExpanded(model()->index(i, 0)))
      list << QString::number(i);

  Appl()->settings.setValue(itemStateKey, list);
}
