/****************************************************************************
**
** Definition of QSqlSelectCursor class.
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

#include "qsqlselectcursor.h"
#include "qsqldriver.h"

#ifndef QT_NO_SQL

class QSqlSelectCursorPrivate
{
public:
    QSqlSelectCursorPrivate() : populated( FALSE ) {}
    QString query;
    bool populated : 1;
};

/*!
    \class QSqlSelectCursor qsqlselectcursor.h
    \brief The QSqlSelectCursor class provides browsing of general SQL
    SELECT statements.

    \ingroup database
    \module sql

    QSqlSelectCursor is a convenience class that makes it possible to
    display result sets from general SQL \c SELECT statements in
    data-aware Qt widgets. QSqlSelectCursor is read-only and does not
    support \c INSERT, \c UPDATE or \c DELETE operations.

    Pass the query in at construction time, or use the
    QSqlSelectCursor::exec() function.

    Example:
    \code
    ...
    QSqlSelectCursor* cur = new QSqlSelectCursor( "SELECT id, firstname, lastname FROM author" );
    QDataTable* table = new QDataTable( this );
    table->setSqlCursor( cur, TRUE, TRUE );
    table->refresh();
    ...
    cur->exec( "SELECT * FROM books" );
    table->refresh();
    ...
    \endcode
*/

/*!
    Constructs a read only cursor on database \a db using the query \a query.
 */
QSqlSelectCursor::QSqlSelectCursor( const QString& query, QSqlDatabase* db )
    : QSqlCursor( QString(), FALSE, db )
{
    d = new QSqlSelectCursorPrivate;
    d->query = query;
    QSqlCursor::setMode( ReadOnly );
    if ( !query.isEmpty() )
	exec( query );
}

/*! Constructs a copy of \a other */
QSqlSelectCursor::QSqlSelectCursor( const QSqlSelectCursor& other )
    : QSqlCursor( other )
{
    d = new QSqlSelectCursorPrivate;
    d->query = other.d->query;
    d->populated = other.d->populated;
}

/*! Destroys the object and frees any allocated resources */
QSqlSelectCursor::~QSqlSelectCursor()
{
    delete d;
}

/*! \reimp */
bool QSqlSelectCursor::exec( const QString& query )
{
    d->query = query;
    bool ret = QSqlCursor::exec( query );
    if ( ret ) {
	QSqlCursor::clear();
	populateCursor();
    }
    return ret;
}

/*! \fn bool QSqlSelectCursor::select()
    \reimp
*/

/*! \reimp */
bool QSqlSelectCursor::select( const QString&, const QSqlIndex& )
{
    bool ret = QSqlCursor::exec( d->query );
    if ( ret && !d->populated )
	populateCursor();
    return ret;
}

/*! \internal */
void QSqlSelectCursor::populateCursor()
{
    QSqlRecordInfo inf = driver()->recordInfo( *(QSqlQuery*)this );
    for ( QSqlRecordInfo::const_iterator it = inf.begin(); it != inf.end(); ++it )
	QSqlCursor::append( *it );
    d->populated = TRUE;
}

/*! \fn QSqlIndex QSqlSelectCursor::primaryIndex( bool ) const
    \reimp
*/

/*! \fn QSqlIndex QSqlSelectCursor::index( const QStringList& ) const
    \reimp
*/

/*! \fn QSqlIndex QSqlSelectCursor::index( const QString& ) const
    \reimp
*/

/*! \fn QSqlIndex QSqlSelectCursor::index( const char* ) const
    \reimp
*/

/*! \fn void QSqlSelectCursor::setPrimaryIndex( const QSqlIndex& )
    \reimp
*/

/*! \fn void QSqlSelectCursor::append( const QSqlFieldInfo& )
    \reimp
*/

/*! \fn void QSqlSelectCursor::insert( int, const QSqlFieldInfo& )
    \reimp
*/

/*! \fn void QSqlSelectCursor::remove( int )
    \reimp
*/

/*! \fn void QSqlSelectCursor::clear()
    \reimp
*/

/*! \fn void QSqlSelectCursor::setGenerated( const QString&, bool )
    \reimp
*/

/*! \fn void QSqlSelectCursor::setGenerated( int, bool )
    \reimp
*/

/*! \fn QSqlRecord* QSqlSelectCursor::editBuffer( bool )
    \reimp
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeInsert()
    \reimp
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeUpdate()
    \reimp
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeDelete()
    \reimp
*/

/*! \fn int QSqlSelectCursor::insert( bool )
    \reimp
*/

/*! \fn int QSqlSelectCursor::update( bool )
    \reimp
*/

/*! \fn int QSqlSelectCursor::del( bool )
    \reimp
*/

/*! \fn void QSqlSelectCursor::setMode( int )
    \reimp
*/

/*! \fn void QSqlSelectCursor::setSort( const QSqlIndex& )
    \reimp
*/

/*! \fn QSqlIndex QSqlSelectCursor::sort() const
    \reimp
*/

/*! \fn void QSqlSelectCursor::setFilter( const QString& )
    \reimp
*/

/*! \fn QString QSqlSelectCursor::filter() const
    \reimp
*/

/*! \fn void QSqlSelectCursor::setName( const QString&, bool )
    \reimp
*/

/*! \fn QString QSqlSelectCursor::name() const
    \reimp
*/

/*! \fn QString QSqlSelectCursor::toString( const QString&, const QString& ) const
    \reimp
*/
#endif // QT_NO_SQL
