/****************************************************************************
** $Id: $
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qsocket.h>
#include <qapplication.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qlistbox.h>

#include "client.h"


ClientInfo::ClientInfo( const QString &host, Q_UINT16 port )
{
    connect( infoList, SIGNAL(selected(const QString&)), SLOT(selectItem(const QString&)) );
    connect( btnBack, SIGNAL(clicked()), SLOT(stepBack()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    socket = new QSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()), SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), SLOT(socketError(int)) );

    socket->connectToHost( host, port );
}


void ClientInfo::selectItem( const QString& item )
{
    // item in listBox selected, use LIST or GET depending of the node type.
    if ( item.endsWith( "/" ) ) {
	sendToServer( "LIST " + infoPath->text() + item );
    } else {
	sendToServer( "GET " + infoPath->text() + item );
    }
}

void ClientInfo::stepBack()
{
    // go back (up) in path hierarchy
    int i = infoPath->text().findRev( '/', -2 );
    if ( i > 0 ) {
	infoPath->setText( infoPath->text().left( i + 1 ) );
    } else
	infoPath->setText( "/" );
    sendToServer( "LIST " + infoPath->text() );
}


void ClientInfo::sendToServer( const QString& line )
{
    // send full command line to the server via socket
    if ( line.startsWith( "LIST" ) ) {
	QString path = line.mid( 5 );
	if ( !path.endsWith( "/" ) )
	    path += "/";
	if ( !path.startsWith( "/" ) )
	    path = "/" + path;
	infoPath->setText( path );
	infoList->clear();
    }
    infoText->clear();

    // write to the server
    QTextStream os(socket);
    os << line << "\r\n";
}

void ClientInfo::socketConnected()
{
    sendToServer( "LIST" );
}

void ClientInfo::socketReadyRead()
{
    // read from the server
    QTextStream stream( socket );
    QString line;
    while ( socket->canReadLine() ) {
	line = stream.readLine();
	if ( line.startsWith( "500" ) || line.startsWith( "550" ) ) {
	    infoText->append( "error: " + line.mid( 4 ) );
	} else if ( line.startsWith( "212+" ) ) {
	    infoList->insertItem( line.mid( 6 ) + QString( ( line[ 4 ] == 'D' ) ? "/" : "" ) );
	} else if ( line.startsWith( "213+" ) ) {
	    infoText->append( line.mid( 4 ) );
	}
    }
}


void ClientInfo::socketConnectionClosed()
{
    infoText->clear();
    infoText->append( "error: Connection closed by the server\n" );
}

void ClientInfo::socketError( int code )
{
    infoText->clear();
    infoText->append( QString( "error: Error number %1 occurred\n" ).arg( code ) );
}

