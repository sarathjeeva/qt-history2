/****************************************************************************
**
** Definition of QSqlError class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSQLERROR_H
#define QSQLERROR_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class Q_EXPORT QSqlError
{
public:
    enum Type {
	None,
	Connection,
	Statement,
	Transaction,
	Unknown
    };
    QSqlError(  const QString& driverText = QString::null,
		const QString& databaseText = QString::null,
		int type = QSqlError::None,
		int number = -1 );
    QSqlError( const QSqlError& other );
    QSqlError& operator=( const QSqlError& other );
    virtual ~QSqlError();

    QString	driverText() const;
    virtual void setDriverText( const QString& driverText );
    QString	databaseText() const;
    virtual void setDatabaseText( const QString& databaseText );
    int		type() const;
    virtual void setType( int type );
    int		number() const;
    virtual void setNumber( int number );
private:
    QString	driverError;
    QString	databaseError;
    int		errorType;
    int	errorNumber;
};

#endif // QT_NO_SQL
#endif
