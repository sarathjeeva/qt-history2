/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.h#18 $
**
** Definition of QMainWindow class
**
** Created : 980316
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QMenuBar;
class QToolBar;
class QStatusBar;
class QToolTipGroup;

class QMainWindowPrivate;


class QMainWindow: public QWidget
{
    Q_OBJECT
public:
    QMainWindow( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    ~QMainWindow();

    QMenuBar * menuBar() const;
    QStatusBar * statusBar() const;
    QToolTipGroup * toolTipGroup() const;

    virtual void setCentralWidget( QWidget * );
    QWidget * centralWidget() const;

    enum ToolBarDock { Unmanaged, TornOff, Top, Bottom, Right, Left };

    virtual void setDockEnabled( ToolBarDock dock, bool enable );
    bool isDockEnabled( ToolBarDock dock ) const;

    void addToolBar( QToolBar *, const char * label,
		     ToolBarDock = Top, bool newLine = FALSE );
    void removeToolBar( QToolBar * );

    void show();

    bool rightJustification() const;
    bool usesBigPixmaps() const;

    bool eventFilter( QObject*, QEvent* );

public slots:
    virtual void setRightJustification( bool );
    virtual void setUsesBigPixmaps( bool );

signals:
    void pixmapSizeChanged( bool );

protected slots:
    virtual void setUpLayout();

protected:
    void paintEvent( QPaintEvent * );
    bool event( QEvent * );

private:
    QMainWindowPrivate * d;
    void triggerLayout();
    void moveToolBar( QToolBar *, QMouseEvent * );

    virtual void setMenuBar( QMenuBar * );
    virtual void setStatusBar( QStatusBar * );
    virtual void setToolTipGroup( QToolTipGroup * );
};


#endif
