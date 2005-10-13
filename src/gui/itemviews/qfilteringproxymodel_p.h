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

#ifndef QFILTERINGPROXYMODEL_P_H
#define QFILTERINGPROXYMODEL_P_H

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

#include <QtCore/qnamespace.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <private/qmappingproxymodel_p.h>

class QFilteringProxyModelPrivate : private QMappingProxyModelPrivate
{
    Q_DECLARE_PUBLIC(QFilteringProxyModel)

public:
    QFilteringProxyModelPrivate() : QMappingProxyModelPrivate() {}
    // maps the parent to the  number of filtered rows and columns
    mutable QMap<QModelIndex, QPair<int,int> > filtered_count;
};

#endif // QFILTERINGPROXYMODEL_P_H
