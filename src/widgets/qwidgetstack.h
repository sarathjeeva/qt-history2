/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwidgetstack.h#6 $
**
** Definition of QWidgetStack class
**
** Created : 980306
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QWIDGETSTACK_H
#define QWIDGETSTACK_H

#ifndef QT_H
#include "qframe.h"
#include "qintdict.h"
#endif // QT_H


class QWidgetStackPrivate;

class QGridLayout;


class QWidgetStack: public QFrame
{
    Q_OBJECT
public:
    QWidgetStack( QWidget * parent = 0, const char * name = 0 );
    ~QWidgetStack();

    void addWidget( QWidget *, int );

    void removeWidget( QWidget * );

    void show();

    QWidget * widget( int ) const;
    int id( QWidget * ) const;

    QWidget * visibleWidget() const;

signals:
    void aboutToShow( int );
    void aboutToShow( QWidget * );

public slots:
    void raiseWidget( int );
    void raiseWidget( QWidget * );

protected:
    void frameChanged();

    void setChildGeometries();

private:
    bool isMyChild( QWidget * );
    QWidgetStackPrivate * d;
    QIntDict<QWidget> * dict;
    QGridLayout * l;
    QWidget * topWidget;
};


#endif
