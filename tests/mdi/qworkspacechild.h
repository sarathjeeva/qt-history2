/****************************************************************************
** $Id: //depot/qt/main/tests/mdi/qworkspacechild.h#2 $
**
** Definition of the QChildWindow class
**
** Created : 990210
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWORKSPACECHILD_H
#define QWORKSPACECHILD_H

//
//  W A R N I N G
//  -------------
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

#include <qframe.h>
class QToolButton;
class QLabel;
class QWorkspace;


class Q_EXPORT QWorkspaceChildTitleBar : public QWidget
{
    Q_OBJECT
public:
    QWorkspaceChildTitleBar (QWorkspace* w, QWidget* parent, const char* name=0, bool iconMode = FALSE );
    ~QWorkspaceChildTitleBar();

    bool isActive() const;

 public slots:
    void setActive( bool );
    void setText( const QString& title );

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();

protected:
    void resizeEvent( QResizeEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QToolButton* closeB;
    QToolButton* maxB;
    QToolButton* iconB;
    QLabel* titleL;
    bool buttonDown;
    int mode;
    QPoint moveOffset;
    QWorkspace* workspace;
    bool imode;
    bool act;
};


class Q_EXPORT QWorkspaceChild : public QFrame
{
    Q_OBJECT
public:
    QWorkspaceChild( QWidget* client, QWorkspace *parent=0, const char *name=0 );
    ~QWorkspaceChild();

    void setActive( bool );
    bool isActive() const;

    QWidget* clientWidget() const;
    QWidget* iconWidget() const;

 public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void childEvent( QChildEvent* );

    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QWidget* clientw;
    bool buttonDown;
    int mode;
    QPoint moveOffset;
    bool act;
    QWorkspaceChildTitleBar* titlebar;
    QWorkspaceChildTitleBar* iconw;
    QSize clientSize;

};
#endif
