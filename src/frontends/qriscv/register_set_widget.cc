// SPDX-FileCopyrightText: 2010, 2011 Tomislav Jonjic
// SPDX-FileCopyrightText: 2023 Davide Cioni
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qriscv/register_set_widget.h"

#include <boost/assign.hpp>

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QVBoxLayout>

#include "qriscv/application.h"
#include "qriscv/register_item_delegate.h"
#include "qriscv/register_set_snapshot.h"
#include "qriscv/tree_view.h"
#include "qriscv/ui_utils.h"

RegisterSetWidget::RegisterSetWidget(Word cpuId, QWidget *parent)
    : QDockWidget("Registers", parent),
      model(new RegisterSetSnapshot(cpuId, this)), cpuId(cpuId),
      delegateKey(QString("RegisterSetWidget%1/delegate").arg(cpuId)) {
  connect(this, &RegisterSetWidget::topLevelChanged, this,
          &RegisterSetWidget::updateWindowTitle);

  QWidget *widget = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout(widget);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  setWidget(widget);

  QActionGroup *displayGroup = new QActionGroup(this);
  QToolBar *toolBar = new QToolBar;

  addDisplayAction("Hex", new RIDelegateHex(this), displayGroup, toolBar);
  addDisplayAction("Signed Decimal", new RIDelegateSignedDecimal(this),
                   displayGroup, toolBar);
  addDisplayAction("Unsigned Decimal", new RIDelegateUnsignedDecimal(this),
                   displayGroup, toolBar);
  addDisplayAction("Binary", new RIDelegateBinary(this), displayGroup, toolBar);

  connect(displayGroup, &QActionGroup::triggered, this,
          &RegisterSetWidget::setDisplayType);

  layout->addWidget(toolBar, 0, Qt::AlignRight);
  QFont toolBarFont = toolBar->font();
  toolBarFont.setPointSizeF(toolBarFont.pointSizeF() * .75);
  toolBar->setFont(toolBarFont);
  toolBar->setStyleSheet("QToolButton { padding: 0; }");

  treeView = new TreeView(
      QString("RegisterSetView%1").arg(cpuId),
      boost::assign::list_of<int>(RegisterSetSnapshot::COL_REGISTER_MNEMONIC),
      true);
  treeView->setItemDelegateForColumn(RegisterSetSnapshot::COL_REGISTER_VALUE,
                                     delegates[currentDelegate()]);
  treeView->setAlternatingRowColors(true);
  treeView->setModel(model);
  SetFirstColumnSpanned(treeView, true);
  layout->addWidget(treeView);

  setAllowedAreas(Qt::AllDockWidgetAreas);
  setFeatures(DockWidgetClosable | DockWidgetMovable | DockWidgetFloatable);
}

void RegisterSetWidget::updateWindowTitle() {
  if (isFloating())
    setWindowTitle(QString("Processor %1 Registers").arg(cpuId));
  else
    setWindowTitle("Registers");
}

void RegisterSetWidget::setDisplayType(QAction *action) {
  int i = action->data().toInt();
  treeView->setItemDelegateForColumn(RegisterSetSnapshot::COL_REGISTER_VALUE,
                                     delegates[i]);
  Appl()->settings.setValue(delegateKey, i);
}

void RegisterSetWidget::addDisplayAction(const QString &text,
                                         QStyledItemDelegate *delegate,
                                         QActionGroup *group,
                                         QToolBar *toolBar) {
  QAction *action = new QAction(text, group);
  action->setCheckable(true);
  int index = delegates.size();
  action->setData(QVariant::fromValue(index));
  action->setChecked(currentDelegate() == index);
  delegates.push_back(delegate);
  toolBar->addAction(action);
}

int RegisterSetWidget::currentDelegate() const {
  return Appl()->settings.value(delegateKey).toInt();
}
