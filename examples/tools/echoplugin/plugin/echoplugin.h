/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ECHOPLUGIN_H
#define ECHOPLUGIN_H

#include <QObject>
#include "echoplugin.h"
#include "echointerface.h"

class EchoPlugin : public QObject, EchoInterface
{
    Q_OBJECT
    Q_INTERFACES(EchoInterface)

public:
    QString echo(const QString &message);
};

#endif
