/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qbuilder.h#1 $
**
** Definition of QBuilder class
**
** Created : 980830
**
** Copyright (C) 1998 Troll Tech AS.  All rights reserved.
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

#ifndef QBUILDER_H
#define QBUILDER_H

#ifndef QT_H
#include "qmainwindow.h"
#include "qlistview.h"
#endif // QT_H

class QBuilderPrivate;

class Q_EXPORT QBuilder : public QMainWindow
{
    Q_OBJECT
public:
    QBuilder();
   ~QBuilder();

private:
    QBuilderPrivate* d;
    friend QApplication;
    void addTopLevelWidget(QWidget*);

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QBuilder( const QBuilder & );
    QBuilder &operator=( const QBuilder & );
#endif
};

class QBuilderObjectItem : public QObject, public QListViewItem {
    Q_OBJECT
    QObject* object;

public:
    QBuilderObjectItem( QListView * parent, QObject* o );
    QBuilderObjectItem( QListViewItem * parent, QObject* o );

    void setup();

protected:
    bool eventFilter(QObject*, QEvent*);

private slots:
    void objectDestroyed();
};

#endif // QFILEDIALOG_H
