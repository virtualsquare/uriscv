// SPDX-FileCopyrightText: 2010 Tomislav Jonjic
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QRISCV_TRACE_BROWSER_H
#define QRISCV_TRACE_BROWSER_H

#include <boost/function.hpp>

#include <QPointer>
#include <QWidget>

#include "base/lang.h"
#include "qriscv/hex_view.h"
#include "qriscv/stoppoint_list_model.h"
#include "uriscv/types.h"

class QAction;
class QListView;
class DebugSession;
class QItemSelection;
class QComboBox;
class QSplitter;
class QStackedWidget;
class TracepointListModel;

class TraceBrowser : public QWidget {
  Q_OBJECT

public:
  TraceBrowser(QAction *insertAction, QAction *removeAction,
               QWidget *parent = 0);
  ~TraceBrowser();

  bool AddTracepoint(Word start, Word end);

private Q_SLOTS:
  void onMachineStarted();
  void onMachineHalted();

  void onTracepointAdded();
  void removeTracepoint();
  void onSelectionChanged(const QItemSelection &);
  void onDelegateTypeChanged(int index);
  void refreshView();

private:
  static const int kDefaultViewDelegate = 0;

  struct ViewDelegateInfo {
    int type;
    QPointer<QWidget> widget;
  };

  typedef boost::function<QWidget *(Word, Word)> DelegateFactoryFunc;

  struct ViewDelegateType {
    ViewDelegateType(const char *name, DelegateFactoryFunc func)
        : name(name), ctor(func) {}
    const char *name;
    DelegateFactoryFunc ctor;
  };

  Stoppoint *selectedTracepoint() const;

  static QWidget *createHexView(Word start, Word end, bool nativeOrder);

  DebugSession *const dbgSession;

  scoped_ptr<TracepointListModel> tplModel;

  QComboBox *delegateTypeCombo;
  QSplitter *splitter;
  QListView *tplView;
  QStackedWidget *viewStack;

  typedef std::map<unsigned int, ViewDelegateInfo> ViewDelegateMap;
  ViewDelegateMap viewMap;

  std::vector<ViewDelegateType> delegateFactory;
};

#endif // QRISCV_TRACE_BROWSER_H
