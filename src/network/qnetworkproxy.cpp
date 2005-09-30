/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qnetworkproxy.h"
#include "qsocks5socketengine_p.h"
#include "qmutex.h"

class QGlobalNetworkProxy
{
public:
    QGlobalNetworkProxy()
        : socks5SocketEngineHandler(0)
    {
    }

    ~QGlobalNetworkProxy()
    {
        delete socks5SocketEngineHandler;
    }

    void init()
    {
        QMutexLocker lock(&mutex);
        if (!socks5SocketEngineHandler)
            socks5SocketEngineHandler = new QSocks5SocketEngineHandler();
    }

    void setProxy(const QNetworkProxy &networkProxy)
    {
        QMutexLocker lock(&mutex);
        globalProxy = networkProxy;
    }

    QNetworkProxy proxy()
    {
        QMutexLocker lock(&mutex);
        return globalProxy;
    }

private:
    QMutex mutex;
    QNetworkProxy globalProxy;
    QSocks5SocketEngineHandler *socks5SocketEngineHandler;
};

Q_GLOBAL_STATIC(QGlobalNetworkProxy, globalNetworkProxy);

class QNetworkProxyPrivate
{
    Q_DECLARE_PUBLIC(QNetworkProxy)

public:
    QNetworkProxy::Type type;
    QString userName;
    QString password;
    QHostAddress address;
    quint16 port;

    QNetworkProxy *q_ptr;
};

QNetworkProxy::QNetworkProxy()
 : d_ptr(new QNetworkProxyPrivate)
{
    Q_D(QNetworkProxy);
    d->q_ptr = this;
    d->type = AutoProxy;
    d->port = 0;
}

void QNetworkProxy::setType(QNetworkProxy::Type type)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->type = type;
}

QNetworkProxy::Type QNetworkProxy::type() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->type;
}

void QNetworkProxy::setUserName(const QString &userName)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->userName = userName;
}

QString QNetworkProxy::userName() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->userName;
}

void QNetworkProxy::setPassword(const QString &password)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->password = password;
}

QString QNetworkProxy::password() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->password;
}

void QNetworkProxy::setAddress(const QHostAddress &address)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->address = address;
}

QHostAddress QNetworkProxy::address() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->address;
}

void QNetworkProxy::setPort(quint16 port)
{
    Q_D(QNetworkProxy);

    globalNetworkProxy()->init();

    d->port = port;
}

quint16 QNetworkProxy::port() const
{
    Q_D(const QNetworkProxy);

    globalNetworkProxy()->init();

    return d->port;
}

void QNetworkProxy::setProxy(const QNetworkProxy &networkProxy)
{
    if (globalNetworkProxy())
        globalNetworkProxy()->setProxy(networkProxy);
}

QNetworkProxy QNetworkProxy::proxy()
{
    if (globalNetworkProxy())
        return globalNetworkProxy()->proxy();
    return QNetworkProxy();
}
