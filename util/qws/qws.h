/****************************************************************************
** $Id: //depot/qt/main/util/qws/qws.h#7 $
**
** Definition of Qt/FB central server classes
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

#ifndef QTFB_H
#define QTFB_H

#include <qserversocket.h>
#include <qmap.h>
#include <qdatetime.h>

#include "qwsproperty.h"
#include "qwscommand.h"

const int QTFB_PORT=0x4642; // FB

class QWSClient;

/*********************************************************************
 *
 * Class: QWSServer
 *
 *********************************************************************/

class QWSServer : public QServerSocket
{
    Q_OBJECT

public:
    QWSServer( QObject *parent=0, const char *name=0 );
    ~QWSServer();
    void newConnection( int socket );

    uchar* frameBuffer() { return framebuffer; }

    void sendMouseEvent(const QPoint& pos, int state);
    QWSPropertyManager *properties() {
	return &propertyManager;
    }

private:
    void invokeCreate( QWSCreateCommand *cmd, QWSClient *client );
    void invokeRegion( QWSRegionCommand *cmd, QWSClient *client );
    void invokeAddProperty( QWSAddPropertyCommand *cmd );
    void invokeSetProperty( QWSSetPropertyCommand *cmd );
    void invokeRemoveProperty( QWSRemovePropertyCommand *cmd );
    
private slots:
    void doClient();

private:
    typedef QMapIterator<int,QWSClient*> ClientIterator;
    typedef QMap<int,QWSClient*> ClientMap;

    int shmid;
    uchar* framebuffer;
    ClientMap client;
    QWSPropertyManager propertyManager;
};

/*********************************************************************
 *
 * Class: QWSClient
 *
 *********************************************************************/

class QWSClient : public QSocket
{
public:
    QWSClient( int socket, int shmid );

    int socket() const;

    void sendMouseEvent(const QPoint& pos, int state);
    QWSCommand* readMoreCommand();

public:
    int s; // XXX QSocket::d::socket->socket() is this value
    QTime timer;
    QWSCommand* command;
};

#endif
