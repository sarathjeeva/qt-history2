/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qserversocket.h#5 $
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qsocket.h"
#endif // QT_H


class QServerSocketPrivate;
class QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket( QObject *parent=0, const char *name=0 );
    QServerSocket( int port, QObject *parent=0, const char *name=0 );
    ~QServerSocket();

    int		 port() const;
    QSocket *socket() const;
    void	 setPort( int port );

    bool start(int backlog=4);

    virtual void newConnection( int socket );

protected:
    QSocketDevice *socketDevice();
    virtual bool accept() const;

protected slots:
    void	 incomingConnection( int socket );

private:
    QServerSocketPrivate *d;
    QSocket	*sd;
};


// inline int QServerSocket::port() const
// {
//     return sd->port();
// }

inline QSocket *QServerSocket::socket() const
{
    return sd;
}


#endif // QSERVERSOCKET_H
