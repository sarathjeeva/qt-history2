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

#ifndef PROPERTY_H
#define PROPERTY_H

#include <qglobal.h>
#include <qstring.h>

QT_BEGIN_NAMESPACE

class QSettings;

class QMakeProperty
{
    QSettings *settings;
    void initSettings();
    QString keyBase(bool =true) const;
    QString value(QString, bool just_check);
public:
    QMakeProperty();
    ~QMakeProperty();

    bool hasValue(QString);
    QString value(QString v) { return value(v, false); }
    void setValue(QString, const QString &);

    bool exec();
};

QT_END_NAMESPACE

#endif // PROPERTY_H
