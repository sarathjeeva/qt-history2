/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.cpp#6 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qws.h"
#include "qwscommand.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qimage.h>
#include <qsocket.h>
#include <qdatetime.h>

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

// make a unique window id
static int get_window_id()
{
    static int win_id = 0;
    return ++win_id;
}

/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

QWSClient::QWSClient( int socket, int shmid ) :
    QSocket(socket),
    s(socket),
    stream(this)
{
    stream.setByteOrder(QDataStream::LittleEndian); // XXX per client
    stream << SWIDTH << SHEIGHT << 32 << shmid;
    stream.device()->flush();
}

int QWSClient::socket() const
{
    return s;
}

void QWSClient::sendMouseEvent(const QPoint& pos, int state)
{
    int window = 0; // not used yet
    int time=timer.elapsed();
    stream << INT8('M') << window << (int)pos.x() << (int)pos.y() << state << time;
    stream.device()->flush();
}

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

QWSServer::QWSServer( QObject *parent=0, const char *name=0 ) :
    QServerSocket(QTFB_PORT,parent,name)
{
    shmid = shmget(IPC_PRIVATE, SWIDTH*SHEIGHT*sizeof(QRgb),
			IPC_CREAT|IPC_EXCL|0666);
    if ( shmid < 0 )
	perror("Cannot allocate shared memory.  Server already running?");
    framebuffer = (uchar*)shmat( shmid, 0, 0 );
    if ( framebuffer == (uchar*)-1 )
	perror("Cannot attach to shared memory.");
    int e=shmctl(shmid, IPC_RMID, 0);
    if ( e<0 )
	perror("shmctl IPC_RMID");

    if ( !start() )
	qFatal("Failed to bind to port %d",QTFB_PORT);
}

QWSServer::~QWSServer()
{
    // XXX destroy all clients
}

void QWSServer::newConnection( int socket )
{
    qDebug("New client...");
    client[socket] = new QWSClient(socket,shmid);
    connect( client[socket], SIGNAL(readyRead()),
	     this, SLOT(doClient()) );
}

void QWSServer::doClient()
{
    QWSClient* c = (QWSClient*)sender();
    int cmd = c->getch();
    QWSCommand *command = QWSCommand::getCommand( cmd, this, c );
    if ( !command ) {
	qWarning( "Protocol error - got: %c", cmd );
// 	qDebug( "got: %c", cmd );
// 	client[c->socket()] = 0;
// 	delete c;
	return;
    }

    command->readData();
    command->execute();
}

void QWSServer::sendMouseEvent(const QPoint& pos, int state)
{
    for (ClientIterator it = client.begin(); it != client.end(); ++it )
	(*it)->sendMouseEvent(pos,state);
}



class Main : public QWidget {
    QImage img;
    QWSServer server;

public:
    Main() :
	server( this )
    {
	img = QImage( server.frameBuffer(),
	    SWIDTH, SHEIGHT, 32, 0, 0, QImage::BigEndian );

	setMouseTracking(TRUE);
	resize(SWIDTH,SHEIGHT);
	startTimer(500);
    }
    void timerEvent(QTimerEvent*)
    {
	repaint(FALSE);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }
    void mouseReleaseEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }
    void mouseMoveEvent(QMouseEvent* e)
    {
	sendMouseEvent(e);
    }

    void sendMouseEvent(QMouseEvent* e)
    {
	server.sendMouseEvent(e->pos(), e->stateAfter());
    }

    void paintEvent(QPaintEvent* e)
    {
	QRect r = e->rect();
	bitBlt(this, r.x(), r.y(), &img, r.x(), r.y(), r.width(), r.height(),
	    OrderedDither);
    }
};

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    qwsRegisterCommands();

    Main m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
