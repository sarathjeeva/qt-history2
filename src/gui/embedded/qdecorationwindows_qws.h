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

#ifndef QDECORATIONWINDOWS_QWS_H
#define QDECORATIONWINDOWS_QWS_H

#ifndef QT_H
#include "qdecorationdefault_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_WINDOWS_WM_STYLE


class QDecorationWindows : public QDecorationDefault
{
public:
    QDecorationWindows();
    virtual ~QDecorationWindows();

    virtual QRegion region(const QWidget *, const QRect &rect, Region);
    virtual void paint(QPainter *, const QWidget *);
    virtual void paintButton(QPainter *, const QWidget *, Region, int state);
protected:
    virtual int getTitleWidth(const QWidget *);
//    virtual int getTitleHeight(const QWidget *);
    virtual const char **menuPixmap();
    virtual const char **closePixmap();
    virtual const char **minimizePixmap();
    virtual const char **maximizePixmap();
    virtual const char **normalizePixmap();
};

#endif // QT_NO_QWS_WINDOWS_WM_STYLE

#endif // QDECORATIONWINDOWS_QWS_H
