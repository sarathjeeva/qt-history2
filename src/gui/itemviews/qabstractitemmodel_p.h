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

#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

#include <private/qobject_p.h>

class QPersistentModelIndexData
{
public:
    QPersistentModelIndexData() : model(0) { ref = 0; }
    QModelIndex index;
    QAtomic ref;
    const QAbstractItemModel *model;
    static QPersistentModelIndexData shared_null;
    static QPersistentModelIndexData *create(const QModelIndex &index);
    static void destroy(QPersistentModelIndexData *data);
};

class QAbstractItemModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemModel)

public:
    static QString i2s(QChar *buf, int size, int num);
    QList<QPersistentModelIndexData*> persistentIndexes;
};

#endif // QABSTRACTITEMMODEL_P_H
