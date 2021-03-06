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

#ifndef PROPARSER_H
#define PROPARSER_H

#include <QMap>
#include <QString>

QT_BEGIN_NAMESPACE

QMap<QString, QString> proFileTagMap( const QString& text, QString currentPath = QString() );
QString loadFile( const QString &fileName );

QT_END_NAMESPACE

#endif
