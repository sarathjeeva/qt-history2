/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#include <QtGui/qpixmap.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class Q_GUI_EXPORT QPixmapCache
{
public:
    static int cacheLimit();
    static void setCacheLimit(int);
    static QPixmap *find(const QString &key);
    static bool find(const QString &key, QPixmap&);
    static bool insert(const QString &key, const QPixmap&);
    static void remove(const QString &key);
    static void clear();
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPIXMAPCACHE_H
