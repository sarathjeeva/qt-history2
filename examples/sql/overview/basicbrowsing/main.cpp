/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include "../connection.h"

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv, FALSE );

    if ( createConnections() ) {
	QSqlDatabase *oracledb = QSqlDatabase::database( "ORACLE" );
	// Copy data from the oracle database to the ODBC (default)
	// database
	QSqlQuery target;
	QSqlQuery query( "SELECT id, name FROM people", oracledb );
	if ( query.isActive() ) {
	    while ( query.next() ) {
		target.exec( "INSERT INTO people ( id, name ) VALUES ( " + 
			      query.value(0).toString() + 
			      ", '" + query.value(1).toString() +  "' )" );
	    }
	}
    }

    return 0;
}
