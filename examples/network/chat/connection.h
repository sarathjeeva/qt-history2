/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QTime>

static const int MaxBufferSize = 1024000;

class Connection : public QTcpSocket
{
    Q_OBJECT
public:
    enum ConnectionState {
        WaitingForGreeting,
        ReadingGreeting,
        ReadyForUse
    };

    enum DataType {
        PlainText,
        Ping,
        Pong,
        Greeting,
        Undefined
    };

    Connection(QObject *parent = 0);
    ~Connection();

    void setName(const QString &name);
    QString name() const;

    void setGreetingMessage(const QString &message);
    bool sendMessage(const QString &message);

signals:
    void readyForUse();
    void newMessage(const QString &from, const QString &message);

protected:
    void timerEvent(QTimerEvent *timerEvent);

private slots:
    void processReadyRead();
    void sendPing();
    void sendGreetingMessage();

private:
    QString greetingMessage;
    QString username;
    QTimer pingTimer;
    QTime pongTime;
    QByteArray buffer;
    ConnectionState state;
    DataType currentDataType;
    int numBytesForCurrentDataType;
    int transferTimerId;
    bool isGreetingMessageSent;

    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();
};

#endif // CONNECTION_H

