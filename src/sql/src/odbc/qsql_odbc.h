/****************************************************************************
**
** Definition of ODBC driver classes
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <qmap.h>
#include <qstring.h>
#include <qsqldriver.h>
#include <qsqlfield.h>
#include <qsqlresult.h>
#include <qsqlindex.h>

#include <sql.h>
#include <sqlext.h>

class QODBCPrivate;
class QODBCDriver;
class QODBCResult : public QSqlResult
{
    friend class QODBCDriver;
public:
    QODBCResult( const QODBCDriver * db, QODBCPrivate* p );
    ~QODBCResult();

    SQLHANDLE   statement();

protected:
    bool	fetchFirst();
    bool	fetchLast();
    bool	fetch(int i);
    bool	reset ( const QString& query );
    QVariant	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();
private:
    QODBCPrivate*	d;
    typedef QMap<int,QVariant> FieldCache;
    FieldCache fieldCache;
    typedef QMap<int,bool> NullCache;
    NullCache nullCache;
};

class QODBCDriver : public QSqlDriver
{
public:
    QODBCDriver( QObject * parent=0, const char * name=0 );
    ~QODBCDriver();
    bool	        hasTransactionSupport() const;
    bool                hasQuerySizeSupport() const;
    bool                canEditBinaryFields() const;
    bool                open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null );
    void	        close();
    QSqlQuery	        createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    SQLHANDLE           environment();
    SQLHANDLE           connection();

protected:
    bool                beginTransaction();
    bool                commitTransaction();
    bool                rollbackTransaction();
private:
    void init();
    bool endTrans();
    void cleanup();
    QODBCPrivate* d;
};

#endif
