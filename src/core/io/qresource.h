/****************************************************************************
**
** Definition of QResource and QMetaResource classes.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QRESOURCE_H__
#define __QRESOURCE_H__

#include <qglobal.h>
#include <qlist.h>

class QMetaResource;
class QMetaResourcePrivate;
class QResourcePrivate;

class Q_CORE_EXPORT QResource {
private:
    QResourcePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QResource)

public:
    QString name() const;

    uint size() const;
    const uchar *data() const;

    bool isContainer() const;
    QList<QResource *> children() const;
    const QResource *parent() const;

    static QResource *find(const QString &path);

protected:
    friend class QMetaResource;
    QResource();
    ~QResource();
};

/* Don't use this */
class Q_CORE_EXPORT QMetaResource {
private:
    QMetaResourcePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QMetaResource)

public:
    QMetaResource(uchar *resource);
    ~QMetaResource();
};

#endif /* __QRESOURCE_H__ */
