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

#ifndef QABSTRACTPROXYMODEL_P_H
#define QABSTRACTPROXYMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QAbstractItemModel*.  This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include "private/qabstractitemmodel_p.h"

#ifndef QT_NO_PROXYMODEL

QT_BEGIN_NAMESPACE

class QAbstractProxyModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QAbstractProxyModel)
public:
    QAbstractProxyModelPrivate() : QAbstractItemModelPrivate(), model(0) {}
    QAbstractItemModel *model;
    void _q_sourceModelDestroyed();
};

QT_END_NAMESPACE

#endif // QT_NO_PROXYMODEL

#endif // QABSTRACTPROXYMODEL_P_H
