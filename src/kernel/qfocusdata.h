/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.h#5 $
**
** Definition of internal QFocusData class
**
** Created : 980405
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

#ifndef QFOCUSDATA_H
#define QFOCUSDATA_H

#ifndef QT_H
#include "qlist.h"
#include "qwidget.h"
#endif // QT_H


class QFocusData {
public:
    QWidget* focusWidget() const { return it.current(); }

    // List-iteration
    QWidget* home();
    QWidget* next();
    QWidget* prev();
    int count() const { return focusWidgets.count(); }

private:
    friend class QWidget;
    QFocusData()
	: it(focusWidgets) {}
    QList<QWidget> focusWidgets;
    QListIterator<QWidget> it;
};

#endif // QFOCUSDATA_H
