/****************************************************************************
**
** Implementation of QHostAddress class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qhostaddress.h"
#include "qstringlist.h"

#ifndef QT_NO_NETWORK
class QHostAddressPrivate
{
public:
    QHostAddressPrivate( Q_UINT32 a_=0 ) : a(a_), isIp4(TRUE)
    {
    }
    QHostAddressPrivate( Q_UINT8 *a_ );
    ~QHostAddressPrivate()
    {
    }

    QHostAddressPrivate & operator=( const QHostAddressPrivate &from )
    {
	a = from.a;
	isIp4 = from.isIp4;
	for ( int i=0; i<16; i++ ) {
	    a6[i] = from.a6[i];
	}
	return *this;
    }

private:
    Q_UINT32 a;     // ip 4 address
    Q_UINT8 a6[16]; // ip 6 address
    bool isIp4;

    friend class QHostAddress;
};

QHostAddressPrivate::QHostAddressPrivate( Q_UINT8 *a_ ) : a(0), isIp4(FALSE)
{
    for ( int i=0; i<16; i++ ) {
	a6[i] = a_[i];
    }
}

/*!
    \class QHostAddress qhostaddress.h
    \brief The QHostAddress class provides an IP address.
\if defined(commercial_edition)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class contains an IP address in a platform and protocol
    independent manner. It stores both IPv4 and IPv6 addresses in a
    way that you can easily access on any platform. (Qt does not
    currently provide IPv6 support.)

    QHostAddress is normally used with the classes QSocket,
    QServerSocket and QSocketDevice to set up a server or to connect
    to a host.

    Host addresses may be set with setAddress() and retrieved with
    ip4Addr() or toString().

    \sa QSocket, QServerSocket, QSocketDevice
*/


/*!
    Creates a host address object with the IP address 0.0.0.0.
*/
QHostAddress::QHostAddress()
    : d( new QHostAddressPrivate )
{
}


/*!
    Creates a host address object for the IPv4 address \a ip4Addr.
*/
QHostAddress::QHostAddress( Q_UINT32 ip4Addr )
    : d( new QHostAddressPrivate( ip4Addr ) )
{
}


/*!
    Creates a host address object with the specified IPv6 address.

    \a ip6Addr must be a 16 byte array in network byte order
    (high-order byte first).
*/
QHostAddress::QHostAddress( Q_UINT8 *ip6Addr )
    : d( new QHostAddressPrivate( ip6Addr ) )
{
}

QHostAddress::QHostAddress(const QString &address)
    : d( new QHostAddressPrivate )
{
    setAddress( address );
}

/*!
    Creates a copy of \a address.
*/
QHostAddress::QHostAddress( const QHostAddress &address )
    : d( new QHostAddressPrivate )
{
    *d = *(address.d);
}


/*!
    Destroys the host address object.
*/
QHostAddress::~QHostAddress()
{
    delete d;
}


/*!
    Assigns another host address object \a address to this object and
    returns a reference to this object.
*/
QHostAddress & QHostAddress::operator=( const QHostAddress & address )
{
    *d = *(address.d);
    return *this;
}


/*!
    Set the IPv4 address specified by \a ip4Addr.
*/
void QHostAddress::setAddress( Q_UINT32 ip4Addr )
{
    delete d;
    d = new QHostAddressPrivate( ip4Addr );
}


/*!
    \overload

    Set the IPv6 address specified by \a ip6Addr.

    \a ip6Addr must be a 16 byte array in network byte order
    (high-order byte first).
*/
void QHostAddress::setAddress( Q_UINT8 *ip6Addr )
{
    delete d;
    d = new QHostAddressPrivate( ip6Addr );
}

#ifndef QT_NO_STRINGLIST
static bool parseIp4(const QString& address, Q_UINT32 *addr)
{
    QStringList ipv4 = QStringList::split(".", address, FALSE);
    if (ipv4.count() == 4) {
	int i = 0;
	bool ok = TRUE;
	while(ok && i < 4) {
	    uint byteValue = ipv4[i].toUInt(&ok);
	    if (byteValue > 255)
		ok = FALSE;
	    if (ok)
		*addr = (*addr << 8) + byteValue;
	    ++i;
	}
	if (ok)
	    return TRUE;
    }
    return FALSE;
}

/*!
    \overload

    Sets the IPv4 or IPv6 address specified by the string
    representation \a address (e.g. "127.0.0.1"). Returns TRUE and
    sets the address if the address was successfully parsed; otherwise
    returns FALSE and leaves the address unchanged.
*/
bool QHostAddress::setAddress(const QString& address)
{
    QString a = address.simplifyWhiteSpace();

    // try ipv4
    Q_UINT32 maybeIp4 = 0;
    if (parseIp4(address, &maybeIp4)) {
	setAddress(maybeIp4);
	return TRUE;
    }

    // try ipv6
    QStringList ipv6 = QStringList::split(":", a, TRUE);
    int count = ipv6.count();
    if (count < 3)
	return FALSE; // there must be at least two ":"
    if (count > 8)
	return FALSE; // maximum of seven ":" exceeded
    Q_UINT8 maybeIp6[16];
    int mc = 16;
    int fillCount = 9 - count;
    for (int i=count-1; i>=0; --i) {
	if ( mc <= 0 )
	    return FALSE;

	if (ipv6[i].isEmpty()) {
	    if (i==count-1) {
		// special case: ":" is last character
		if (!ipv6[i-1].isEmpty())
		    return FALSE;
		maybeIp6[--mc] = 0;
		maybeIp6[--mc] = 0;
	    } else if (i==0) {
		// special case: ":" is first character
		if (!ipv6[i+1].isEmpty())
		    return FALSE;
		maybeIp6[--mc] = 0;
		maybeIp6[--mc] = 0;
	    } else {
		for (int j=0; j<fillCount; ++j) {
		    if ( mc <= 0 )
			return FALSE;
		    maybeIp6[--mc] = 0;
		    maybeIp6[--mc] = 0;
		}
	    }
	} else {
	    bool ok = FALSE;
	    uint byteValue = ipv6[i].toUInt(&ok, 16);
	    if (ok && byteValue <= 0xffff) {
		maybeIp6[--mc] = byteValue & 0xff;
		maybeIp6[--mc] = (byteValue >> 8) & 0xff;
	    } else {
		if (i == count-1) {
		    // parse the ipv4 part of a mixed type
		    if (!parseIp4(ipv6[i], &maybeIp4))
			return FALSE;
		    maybeIp6[--mc] = maybeIp4 & 0xff;
		    maybeIp6[--mc] = (maybeIp4 >> 8) & 0xff;
		    maybeIp6[--mc] = (maybeIp4 >> 16) & 0xff;
		    maybeIp6[--mc] = (maybeIp4 >> 24) & 0xff;
		    --fillCount;
		} else {
		    return FALSE;
		}
	    }
	}
    }
    if (mc == 0) {
	setAddress(maybeIp6);
	return TRUE;
    }

    return FALSE;
}
#endif

/*!
    Returns TRUE if the host address represents an IPv4 address;
    otherwise returns FALSE.
*/
bool QHostAddress::isIp4Addr() const
{
    return d->isIp4;
}


/*!
    Returns the IPv4 address as a number.

    For example, if the address is 127.0.0.1, the returned value is
    2130706433 (i.e. 0x7f000001).

    This value is only valid when isIp4Addr() returns TRUE.

    \sa toString()
*/
Q_UINT32 QHostAddress::ip4Addr() const
{
    return d->a;
}

#ifndef QT_NO_SPRINTF
/*!
    Returns the address as a string.

    For example, if the address is the IPv4 address 127.0.0.1, the
    returned string is "127.0.0.1".

    \sa ip4Addr()
*/
QString QHostAddress::toString() const
{
    if ( d->isIp4 ) {
	Q_UINT32 i = ip4Addr();
	QString s;
	s.sprintf( "%d.%d.%d.%d", (i>>24) & 0xff, (i>>16) & 0xff,
		(i >> 8) & 0xff, i & 0xff );
	return s;
    } else {
	Q_UINT16 ugle[8];
	for ( int i=0; i<8; i++ ) {
	    ugle[i] = ( (Q_UINT16)( d->a6[2*i] ) << 8 ) |
		( (Q_UINT16)( d->a6[2*i+1] ) );
	}
	QString s;
	s.sprintf( "%X:%X:%X:%X:%X:%X:%X:%X",
		ugle[0], ugle[1], ugle[2], ugle[3],
		ugle[4], ugle[5], ugle[6], ugle[7] );
	return s;
    }
}
#endif


/*!
    Returns TRUE if this host address is the same as \a other;
    otherwise returns FALSE.
*/
bool QHostAddress::operator==( const QHostAddress & other ) const
{
    return d->a == other.d->a;
}


/*!
    Returns TRUE if this host address is null (INADDR_ANY or in6addr_any). The
    default constructor creates a null address, and that address isn't valid
    for any particular host or interface.
*/
bool QHostAddress::isNull() const
{
    if ( d->isIp4 )
	return d->a == 0;
    int i = 0;
    while( i < 16 ) {
	if ( d->a6[i++] != 0 )
	    return FALSE;
    }
    return TRUE;
}

#endif //QT_NO_NETWORK
