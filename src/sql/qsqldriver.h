/****************************************************************************
**
** Definition of QSqlDriver class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLDRIVER_H
#define QSQLDRIVER_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qsqlerror.h"
#include "qsqlquery.h"
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qstringlist.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlDatabase;
class QSqlDriverPrivate;

class QM_EXPORT_SQL QSqlDriver : public QObject
{
    friend class QSqlDatabase;
    Q_OBJECT
public:
    enum DriverFeature { Transactions, QuerySize, BLOB, Unicode, PreparedQueries,
			 NamedPlaceholders, PositionalPlaceholders };

    QSqlDriver( QObject * parent=0, const char * name=0 );
    ~QSqlDriver();
    virtual bool	isOpen() const;
    bool		isOpenError() const;

    virtual bool		beginTransaction();
    virtual bool		commitTransaction();
    virtual bool		rollbackTransaction();
    virtual QStringList		tables( const QString& tableType ) const;
    virtual QSqlIndex		primaryIndex( const QString& tableName ) const;
    virtual QSqlRecord		record( const QString& tableName ) const;
    virtual QSqlRecord		record( const QSqlQuery& query ) const;
    virtual QSqlRecordInfo	recordInfo( const QString& tablename ) const;
    virtual QSqlRecordInfo	recordInfo( const QSqlQuery& query ) const;
    virtual QString		nullText() const;
    virtual QString		formatValue( const QSqlField* field, bool trimStrings = FALSE ) const;
    QSqlError			lastError() const;

    virtual bool		hasFeature( DriverFeature f ) const = 0;
    virtual void		close() = 0;
    virtual QSqlQuery		createQuery() const = 0;

    virtual bool			open( const QString& db,
				      const QString& user = QString(),
				      const QString& password = QString(),
				      const QString& host = QString(),
				      int port = -1,
				      const QString& connOpts = QString() ) = 0;
protected:
    virtual void		setOpen( bool o );
    virtual void		setOpenError( bool e );
    virtual void		setLastError( const QSqlError& e );
private:
    QSqlDriverPrivate* d;
#if defined(Q_DISABLE_COPY)
    QSqlDriver( const QSqlDriver & );
    QSqlDriver &operator=( const QSqlDriver & );
#endif
};

#endif	// QT_NO_SQL
#endif
