/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>
#include <QtNetwork/qsslcipher.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qtcpserver.h>
#include <QtTest/QtTest>

Q_DECLARE_METATYPE(QAbstractSocket::SocketState)
Q_DECLARE_METATYPE(QAbstractSocket::SocketError)
#ifndef QT_NO_OPENSSL
Q_DECLARE_METATYPE(QSslSocket::Mode)
#endif

class tst_QSslSocket : public QObject
{
    Q_OBJECT

public:
    tst_QSslSocket();
    virtual ~tst_QSslSocket();

    static void enterLoop(int secs)
    {
        ++loopLevel;
        QTestEventLoop::instance().enterLoop(secs);
    }

    static bool timeout()
    {
        return QTestEventLoop::instance().timeout();
    }

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL
private slots:
    void constructing();
    void simpleConnect();
    void simpleConnectWithIgnore();

    // API tests
    void addCaCertificate();
    void addCaCertificates();
    void addCaCertificates2();
    void ciphers();
    void connectToHostEncrypted();
    void currentCipher();
    void flush();
    void isEncrypted();
    void localCertificate();
    void mode();
    void peerCertificate();
    void peerCertificateChain();
    void privateKey();
    void protocol();
    void setCaCertificates();
    void setLocalCertificate();
    void setPrivateKey();
    void setProtocol();
    void setSocketDescriptor();
    void waitForEncrypted();
    void startClientHandShake();
    void startServerHandShake();
    void addGlobalCaCertificate();
    void addGlobalCaCertificates();
    void addGlobalCaCertificates2();
    void globalCaCertificates();
    void globalCiphers();
    void resetGlobalCiphers();
    void setGlobalCaCertificates();
    void setGlobalCiphers();
    void supportedCiphers();
    void supportsSsl();
    void systemCaCertificates();

    static void exitLoop()
    {
        // Safe exit - if we aren't in an event loop, don't
        // exit one.
        if (loopLevel > 0) {
            --loopLevel;
            QTestEventLoop::instance().exitLoop();
        }
    }

protected slots:
    void ignoreErrorSlot()
    {
        socket->ignoreSslErrors();
    }

private:
    QSslSocket *socket;
#endif // QT_NO_OPENSSL
private:
    static int loopLevel;
};

int tst_QSslSocket::loopLevel = 0;

tst_QSslSocket::tst_QSslSocket()
{
#ifndef QT_NO_OPENSSL
    qRegisterMetaType<QList<QSslError> >("QList<QSslError>");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QSslSocket::Mode");
#endif
}

tst_QSslSocket::~tst_QSslSocket()
{

}

void tst_QSslSocket::initTestCase_data()
{
}

void tst_QSslSocket::init()
{
}

void tst_QSslSocket::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslSocket::constructing()
{
    QSslSocket socket;

    QCOMPARE(socket.state(), QSslSocket::UnconnectedState);
    QCOMPARE(socket.mode(), QSslSocket::PlainMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(socket.bytesAvailable(), qint64(0));
    QCOMPARE(socket.bytesToWrite(), qint64(0));
    QVERIFY(!socket.canReadLine());
    QVERIFY(socket.atEnd());
    QCOMPARE(socket.localCertificate(), QSslCertificate());
    QCOMPARE(socket.errorString(), QString("Unknown error"));
    char c = '\0';
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
    QVERIFY(!socket.getChar(&c));
    QCOMPARE(c, '\0');
    QVERIFY(!socket.isOpen());
    QVERIFY(!socket.isReadable());
    QVERIFY(socket.isSequential());
    QVERIFY(!socket.isTextModeEnabled());
    QVERIFY(!socket.isWritable());
    QCOMPARE(socket.openMode(), QIODevice::NotOpen);
    QVERIFY(socket.peek(2).isEmpty());
    QCOMPARE(socket.pos(), qint64(0));
    QVERIFY(!socket.putChar('c'));
    QVERIFY(socket.read(2).isEmpty());
    QCOMPARE(socket.read(0, 0), qint64(-1));
    QVERIFY(socket.readAll().isEmpty());
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::readLine: Called with maxSize < 2");
    QCOMPARE(socket.readLine(0, 0), qint64(-1));
    char buf[10];
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device"); // readLine is based on getChar
    QCOMPARE(socket.readLine(buf, sizeof(buf)), qint64(-1));
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::seek: The device is not open");
    QVERIFY(!socket.reset());
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::seek: The device is not open");
    QVERIFY(!socket.seek(2));
    QCOMPARE(socket.size(), qint64(0));
    QVERIFY(!socket.waitForBytesWritten(10));
    QVERIFY(!socket.waitForReadyRead(10));
    QCOMPARE(socket.write(0, 0), qint64(-1));
    QCOMPARE(socket.write(QByteArray()), qint64(-1));
    QCOMPARE(socket.error(), QAbstractSocket::UnknownSocketError);
    QVERIFY(!socket.flush());
    QVERIFY(!socket.isValid());
    QCOMPARE(socket.localAddress(), QHostAddress());
    QCOMPARE(socket.localPort(), quint16(0));
    QCOMPARE(socket.peerAddress(), QHostAddress());
    QVERIFY(socket.peerName().isEmpty());
    QCOMPARE(socket.peerPort(), quint16(0));
    QCOMPARE(socket.proxy().type(), QNetworkProxy::DefaultProxy);
    QCOMPARE(socket.readBufferSize(), qint64(0));
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE(socket.socketType(), QAbstractSocket::TcpSocket);
    QVERIFY(!socket.waitForConnected(10));
    QTest::ignoreMessage(QtWarningMsg, "QSslSocket::waitForDisconnected() is not allowed in UnconnectedState");
    QVERIFY(!socket.waitForDisconnected(10));
    QCOMPARE(socket.protocol(), QSslSocket::SslV3);
}

void tst_QSslSocket::simpleConnect()
{
    QSslSocket socket;
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy hostFoundSpy(&socket, SIGNAL(hostFound()));
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy connectionEncryptedSpy(&socket, SIGNAL(encrypted()));
    QSignalSpy sslErrorsSpy(&socket, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&socket, SIGNAL(connected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(modeChanged(QSslSocket::Mode)), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(encrypted()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(exitLoop()));

    // Start connecting
    socket.connectToHost("imap.troll.no", 993);
    QCOMPARE(socket.state(), QAbstractSocket::HostLookupState);
    enterLoop(10);

    // Entered connecting state
    QCOMPARE(socket.state(), QAbstractSocket::ConnectingState);
    QCOMPARE(connectedSpy.count(), 0);
    QCOMPARE(hostFoundSpy.count(), 1);
    QCOMPARE(disconnectedSpy.count(), 0);
    enterLoop(10);

    // Entered connected state
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(socket.mode(), QSslSocket::PlainMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(hostFoundSpy.count(), 1);
    QCOMPARE(disconnectedSpy.count(), 0);

    // Enter encrypted mode
    socket.startClientHandShake();
    QCOMPARE(socket.mode(), QSslSocket::SslClientMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(connectionEncryptedSpy.count(), 0);
    QCOMPARE(sslErrorsSpy.count(), 0);

    // Starting handshake
    enterLoop(10);
    QCOMPARE(sslErrorsSpy.count(), 1);
    QCOMPARE(connectionEncryptedSpy.count(), 0);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
}

void tst_QSslSocket::simpleConnectWithIgnore()
{
    QSslSocket socket;
    this->socket = &socket;
    QSignalSpy encryptedSpy(&socket, SIGNAL(encrypted()));
    QSignalSpy sslErrorsSpy(&socket, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&socket, SIGNAL(readyRead()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(encrypted()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(connected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(exitLoop()));

    // Start connecting
    socket.connectToHost("imap.troll.no", 993);
    QCOMPARE(socket.state(), QAbstractSocket::HostLookupState);
    enterLoop(10);

    // Start handshake
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    socket.startClientHandShake();
    enterLoop(10);

    // Done; encryption should be enabled.
    QCOMPARE(sslErrorsSpy.count(), 1);
    QVERIFY(socket.isEncrypted());
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(encryptedSpy.count(), 1);

    // Wait for incoming data
    if (!socket.canReadLine())
        enterLoop(10);

    QCOMPARE(socket.readAll(), QByteArray("* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n"));

    socket.disconnectFromHost();
}

void tst_QSslSocket::addCaCertificate()
{
}

void tst_QSslSocket::addCaCertificates()
{
}

void tst_QSslSocket::addCaCertificates2()
{
}

void tst_QSslSocket::ciphers()
{
    QSslSocket socket;
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());
    socket.setCiphers(QList<QSslCipher>());
    QVERIFY(socket.ciphers().isEmpty());
    socket.resetCiphers();
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());
    socket.resetCiphers();
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());
}

void tst_QSslSocket::connectToHostEncrypted()
{
    QSslSocket socket;

    socket.addGlobalCaCertificates(QLatin1String("certs/fluke.ca.pem"));
    socket.connectToHostEncrypted("fluke.troll.no", 443);

    // This should pass unconditionally when using fluke's CA certificate.
    QVERIFY(socket.waitForEncrypted(10000));

    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected());

    QCOMPARE(socket.mode(), QSslSocket::SslClientMode);

    socket.connectToHost("fluke.troll.no", 13);

    QCOMPARE(socket.mode(), QSslSocket::PlainMode);

    QVERIFY(socket.waitForDisconnected());
}

void tst_QSslSocket::currentCipher()
{
    QSslSocket socket;
    this->socket = &socket;
    connect(&socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(ignoreErrorSlot()));
    QVERIFY(socket.currentCipher().isNull());
    socket.connectToHost("fluke.troll.no", 443 /* https */);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(socket.currentCipher().isNull());
    socket.startClientHandShake();
    QVERIFY(socket.waitForEncrypted(5000));
    QVERIFY(!socket.currentCipher().isNull());
    QVERIFY(QSslSocket::supportedCiphers().contains(socket.currentCipher()));
    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected());
}

void tst_QSslSocket::flush()
{
}

void tst_QSslSocket::isEncrypted()
{
}

void tst_QSslSocket::localCertificate()
{
}

void tst_QSslSocket::mode()
{
}

void tst_QSslSocket::peerCertificate()
{
}

void tst_QSslSocket::peerCertificateChain()
{
}

void tst_QSslSocket::privateKey()
{
}

void tst_QSslSocket::protocol()
{
    QSslSocket socket;
    QCOMPARE(socket.protocol(), QSslSocket::SslV3);
    {
        // Fluke allows TLSV1.
        socket.setProtocol(QSslSocket::TlsV1);
        QCOMPARE(socket.protocol(), QSslSocket::TlsV1);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        socket.abort();
    }
    {
        // Fluke allows SSLV2.
        socket.setProtocol(QSslSocket::SslV2);
        QCOMPARE(socket.protocol(), QSslSocket::SslV2);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY(socket.waitForEncrypted());
        socket.abort();
    }
    {
        // Fluke allows SSLV3, so it allows Compat.
        socket.setProtocol(QSslSocket::Compat);
        QCOMPARE(socket.protocol(), QSslSocket::Compat);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY(socket.waitForEncrypted());
    }
}

void tst_QSslSocket::setCaCertificates()
{
    QSslSocket socket;
    QCOMPARE(socket.caCertificates(), QSslSocket::globalCaCertificates());
    socket.setCaCertificates(QSslCertificate::fromPath("certs/fluke.ca.pem"));
    QCOMPARE(socket.caCertificates().size(), 1);
    socket.resetCaCertificates();
    QCOMPARE(socket.caCertificates(), QSslSocket::globalCaCertificates());
}

void tst_QSslSocket::setLocalCertificate()
{
}

void tst_QSslSocket::setPrivateKey()
{
}

void tst_QSslSocket::setProtocol()
{
}

class SslServer : public QTcpServer
{
    Q_OBJECT
public:
    SslServer() : socket(0) { }
    QSslSocket *socket;

protected:
    void incomingConnection(int socketDescriptor)
    {
        socket = new QSslSocket;
        connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));

        QList<QSslCertificate> localCert = QSslCertificate::fromPath("qsslsocket.pem");
        QVERIFY(!localCert.isEmpty());
        socket->setLocalCertificate(localCert.first());

        QVERIFY(socket->setSocketDescriptor(socketDescriptor, QAbstractSocket::ConnectedState));
        socket->startServerHandShake();
    }

protected slots:
    void ignoreErrorSlot()
    {
        socket->ignoreSslErrors();
    }
};

void tst_QSslSocket::setSocketDescriptor()
{
    /*
    SslServer server;
    QVERIFY(server.listen());

    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));

    QSslSocket *client = new QSslSocket;
    socket = client;
    connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    connect(client, SIGNAL(encrypted()), &loop, SLOT(quit()));

    client->connectToHostEncrypted(QHostAddress(QHostAddress::LocalHost).toString(), server.serverPort());

    loop.exec();

    QCOMPARE(client->state(), QAbstractSocket::ConnectedState);
    QVERIFY(client->isEncrypted());
    */
}

void tst_QSslSocket::waitForEncrypted()
{
    QSslSocket socket;
    this->socket = &socket;

    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    socket.connectToHostEncrypted("imap.troll.no", 993);

    QVERIFY(socket.waitForEncrypted(10000));
}

void tst_QSslSocket::startClientHandShake()
{
}

void tst_QSslSocket::startServerHandShake()
{
}

void tst_QSslSocket::addGlobalCaCertificate()
{
    // Reset the global CA chain
    QSslSocket::setGlobalCaCertificates(QSslSocket::systemCaCertificates());

    QList<QSslCertificate> flukeCerts = QSslCertificate::fromPath("certs/fluke.ca.pem");
    QCOMPARE(flukeCerts.size(), 1);
    QList<QSslCertificate> globalCerts = QSslSocket::globalCaCertificates();
    QVERIFY(!globalCerts.contains(flukeCerts.first()));
    QSslSocket::addGlobalCaCertificate(flukeCerts.first());
    QCOMPARE(QSslSocket::globalCaCertificates().size(), globalCerts.size() + 1);
    QVERIFY(QSslSocket::globalCaCertificates().contains(flukeCerts.first()));

    // Restore the global CA chain
    QSslSocket::setGlobalCaCertificates(QSslSocket::systemCaCertificates());
}

void tst_QSslSocket::addGlobalCaCertificates()
{
}

void tst_QSslSocket::addGlobalCaCertificates2()
{
}

void tst_QSslSocket::globalCaCertificates()
{
    QList<QSslCertificate> certs = QSslSocket::globalCaCertificates();
    QVERIFY(certs.size() > 1);
    QCOMPARE(certs, QSslSocket::systemCaCertificates());
}

void tst_QSslSocket::globalCiphers()
{
}

void tst_QSslSocket::resetGlobalCiphers()
{
}

void tst_QSslSocket::setGlobalCaCertificates()
{
}

void tst_QSslSocket::setGlobalCiphers()
{
}

void tst_QSslSocket::supportedCiphers()
{
    QList<QSslCipher> ciphers = QSslSocket::supportedCiphers();
    QVERIFY(ciphers.size() > 1);

    QSslSocket socket;
    QCOMPARE(socket.supportedCiphers(), ciphers);
    QCOMPARE(socket.globalCiphers(), ciphers);
    QCOMPARE(socket.ciphers(), ciphers);
}

void tst_QSslSocket::supportsSsl()
{
    QVERIFY(QSslSocket::supportsSsl());
}

void tst_QSslSocket::systemCaCertificates()
{
    QList<QSslCertificate> certs = QSslSocket::systemCaCertificates();
    QVERIFY(certs.size() > 1);
    QCOMPARE(certs, QSslSocket::globalCaCertificates());
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslSocket)
#include "tst_qsslsocket.moc"
