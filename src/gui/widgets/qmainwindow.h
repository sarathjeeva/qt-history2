/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#include <qwidget.h>

class QDockWindow;
class QMainWindowPrivate;
class QMenuBar;
class QStatusBar;
class QToolBar;
class QMenu;

class Q_GUI_EXPORT QMainWindow : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMainWindow)

    Q_PROPERTY(Qt::IconSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle)

public:
    QMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QMainWindow();

    Qt::IconSize iconSize() const;
    void setIconSize(Qt::IconSize iconSize);

    Qt::ToolButtonStyle toolButtonStyle() const;
    void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

    QMenuBar *menuBar() const;
    void setMenuBar(QMenuBar *menubar);

    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *statusbar);

    QWidget *centralWidget() const;
    void setCentralWidget(QWidget *widget);

    void setCorner(Qt::Corner corner, Qt::DockWindowArea area);
    Qt::DockWindowArea corner(Qt::Corner corner) const;

    void addToolBarBreak(Qt::ToolBarArea area);
    void insertToolBarBreak(QToolBar *before);

    void addToolBar(Qt::ToolBarArea area, QToolBar *toolbar);
    void addToolBar(QToolBar *toolbar);
    void insertToolBar(QToolBar *before, QToolBar *toolbar);
    void removeToolBar(QToolBar *toolbar);

    Qt::ToolBarArea toolBarArea(QToolBar *toolbar) const;

    void addDockWindow(Qt::DockWindowArea area, QDockWindow *dockwindow);
    void extendDockWindowArea(Qt::DockWindowArea area, QDockWindow *dockwindow,
                              Qt::Orientation orientation);
    void splitDockWindow(QDockWindow *after, QDockWindow *dockwindow,
                         Qt::Orientation orientation);
    void removeDockWindow(QDockWindow *dockwindow);

    Qt::DockWindowArea dockWindowArea(QDockWindow *dockwindow) const;

    void setDockWindowState(const QString &state);
    QString dockWindowState() const;

    virtual QMenu *createPopupMenu();

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QMainWindow(QWidget *parent, const char *name, Qt::WFlags flags = 0);
#endif

signals:
    void iconSizeChanged(Qt::IconSize iconSize);
    void toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle);

protected:
    void childEvent(QChildEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    bool event(QEvent *event);
};

#endif // QMAINWINDOW_H
