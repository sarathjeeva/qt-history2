/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDROPSITE_H
#define QDROPSITE_H

#ifndef QT_H
#ifndef QT_H
#include "qglobal.h"
#endif // QT_H
#endif


class QWidget;


class Q_COMPAT_EXPORT QDropSite {
public:
    QDropSite( QWidget* parent );
    virtual ~QDropSite();
};


#endif  // QDROPSITE_H
