/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qstatusbar.h#7 $
**
** Definition of QStatusBar class
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

#ifndef QSTATUSBAR_H
#define QSTATUSBAR_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QStatusBarPrivate;


class Q_EXPORT QStatusBar: public QWidget
{
    Q_OBJECT
public:
    QStatusBar( QWidget * parent = 0, const char *name = 0 );
    ~QStatusBar();

    void addWidget( QWidget *, int, bool = FALSE );
    void removeWidget( QWidget * );

public slots:
    void message( const QString &);
    void message( const QString &, int );
    void clear();

protected:
    void paintEvent( QPaintEvent * );

    void reformat();
    void hideOrShow();

private:
    QStatusBarPrivate * d;
};


#endif
