/****************************************************************************
**
** Implementation of QSocketDevice class.
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

#include "qplatformdefs.h"

// Almost always the same. If not, specify in qplatformdefs.h.
#if !defined(QT_SOCKOPTLEN_T)
# define QT_SOCKOPTLEN_T QT_SOCKLEN_T
#endif

// Tru64 redefines accept -> _accept with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_accept(int s, struct sockaddr *addr, QT_SOCKLEN_T *addrlen)
{ return ::accept(s, addr, addrlen); }
#if defined(accept)
# undef accept
#endif

// Solaris redefines bind -> __xnet_bind with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_bind(int s, struct sockaddr *addr, QT_SOCKLEN_T addrlen)
{ return ::bind(s, addr, addrlen); }
#if defined(bind)
# undef bind
#endif

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_connect(int s, struct sockaddr *addr, QT_SOCKLEN_T addrlen)
{ return ::connect(s, addr, addrlen); }
#if defined(connect)
# undef connect
#endif

// UnixWare 7 redefines listen -> _listen
static inline int qt_socket_listen(int s, int backlog)
{ return ::listen(s, backlog); }
#if defined(listen)
# undef listen
#endif

// UnixWare 7 redefines socket -> _socket
static inline int qt_socket_socket(int domain, int type, int protocol)
{ return ::socket(domain, type, protocol); }
#if defined(socket)
# undef socket
#endif

#include "qsocketdevice.h"

#ifndef QT_NO_NETWORK

#include "qwindowdefs.h"

#include <errno.h>

static inline QSocketDevice::Protocol qt_socket_getportaddr( struct sockaddr *sa,
		       Q_UINT16 *port, QHostAddress *addr )
{
#if !defined(QT_NO_IPV6)
    if ( sa->sa_family == AF_INET6 ) {
	struct sockaddr_in6 *sa6 = ( struct sockaddr_in6 * )sa;
	*port = ntohs( sa6->sin6_port );

	Q_IPV6ADDR tmp;
	memcpy( &tmp, &sa6->sin6_addr.s6_addr, sizeof(tmp) );
	*addr = QHostAddress( tmp );

	return QSocketDevice::Ipv6;
    } else
#endif
    {
	struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
	*port = ntohs( sa4->sin_port );
	*addr = QHostAddress( ntohl( sa4->sin_addr.s_addr ) );
	return QSocketDevice::Ipv4;
    }
}


//#define QSOCKETDEVICE_DEBUG

// internal
void QSocketDevice::init()
{
}


QSocketDevice::Protocol QSocketDevice::getProtocol( int socket )
{
#if !defined (QT_NO_IPV6)
    if (socket != -1) {
	struct sockaddr_storage ss;
	QT_SOCKLEN_T sslen = sizeof( ss );
	if ( getsockname(socket, (struct sockaddr *)&ss, &sslen) == 0 ) {
	    switch ( ss.ss_family ) {
		case AF_INET:
		    return Ipv4;
		case AF_INET6:
		    return Ipv6;
		default:
		    break;
	    }
	}
    }
#endif
    return Ipv4;
}

/*!
    Creates a new socket identifier. Returns -1 if there is a failure
    to create the new identifier; error() explains why.

    \sa setSocket()
*/

int QSocketDevice::createNewSocket()
{
#if !defined(QT_NO_IPV6)
    int s = qt_socket_socket( protocol() == Ipv6 ? AF_INET6 : AF_INET,
			      t == Datagram ? SOCK_DGRAM : SOCK_STREAM, 0 );
#else
    int s = qt_socket_socket( AF_INET, t==Datagram?SOCK_DGRAM:SOCK_STREAM, 0 );
#endif
    if ( s < 0 ) {
	switch( errno ) {
	case EPROTONOSUPPORT:
	    e = InternalError; // 0 is supposed to work for both types
	    break;
	case ENFILE:
	    e = NoFiles; // special case for this
	    break;
	case EACCES:
	    e = Inaccessible;
	    break;
	case ENOBUFS:
	case ENOMEM:
	    e = NoResources;
	    break;
	case EINVAL:
	    e = Impossible;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
    } else {
	return s;
    }
    return -1;
}

/*!
    \reimp

    Closes the socket and sets the socket identifier to -1 (invalid).

    (This function ignores errors; if there are any then a file
    descriptor leakage might result. As far as we know, the only error
    that can arise is EBADF, and that would of course not cause
    leakage. There may be OS-specific errors that we haven't come
    across, however.)

    \sa open()
*/
void QSocketDevice::close()
{
    if ( fd == -1 || !isOpen() )		// already closed
	return;
    setFlags( IO_Sequential );
    resetStatus();
    setState( 0 );
    ::close( fd );
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::close: Closed socket %x", fd );
#endif
    fd = -1;
    fetchConnectionParameters();
}


/*!
    Returns TRUE if the socket is valid and in blocking mode;
    otherwise returns FALSE.

    Note that this function does not set error().

    \warning On Windows, this function always returns TRUE since the
    ioctlsocket() function is broken.

    \sa setBlocking(), isValid()
*/
bool QSocketDevice::blocking() const
{
    if ( !isValid() )
	return TRUE;
    int s = fcntl( socket(), F_GETFL, 0 );
    return !(s >= 0 && ((s & O_NDELAY) != 0));
}


/*!
    Makes the socket blocking if \a enable is TRUE or nonblocking if
    \a enable is FALSE.

    Sockets are blocking by default, but we recommend using
    nonblocking socket operations, especially for GUI programs that
    need to be responsive.

    \warning On Windows, this function should be used with care since
    whenever you use a QSocketNotifier on Windows, the socket is
    immediately made nonblocking.

    \sa blocking(), isValid()
*/
void QSocketDevice::setBlocking( bool enable )
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::setBlocking( %d )", enable );
#endif
    int tmp = ::fcntl( socket(), F_GETFL, 0);
    if ( tmp >= 0 )
	tmp = ::fcntl( socket(), F_SETFL, enable ? (tmp&~O_NDELAY) : (tmp|O_NDELAY) );
    if ( tmp >= 0 )
	return;
    if ( e )
	return;
    switch( errno ) {
    case EACCES:
    case EBADF:
	e = Impossible;
	break;
    case EFAULT:
    case EAGAIN:
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
    case EDEADLK:
    case EINTR:
    case EINVAL:
    case EMFILE:
    case ENOLCK:
    case EPERM:
    default:
	e = UnknownError;
    }
}


/*!
    Returns the value of the socket option \a opt.
*/
int QSocketDevice::option( Option opt ) const
{
    int n = -1;
    int v = -1;
    switch ( opt ) {
    case Broadcast:
	n = SO_BROADCAST;
	break;
    case ReceiveBuffer:
	n = SO_RCVBUF;
	break;
    case ReuseAddress:
	n = SO_REUSEADDR;
	break;
    case SendBuffer:
	n = SO_SNDBUF;
	break;
    }
    if ( n != -1 ) {
	QT_SOCKOPTLEN_T len;
	len = sizeof(v);
	int r = ::getsockopt( socket(), SOL_SOCKET, n, (char*)&v, &len );
	if ( r >= 0 )
	    return v;
	if ( !e ) {
	    QSocketDevice *that = (QSocketDevice*)this; // mutable function
	    switch( errno ) {
	    case EBADF:
	    case ENOTSOCK:
		that->e = Impossible;
		break;
	    case EFAULT:
		that->e = InternalError;
		break;
	    default:
		that->e = UnknownError;
		break;
	    }
	}
	return -1;
    }
    return v;
}


/*!
    Sets the socket option \a opt to \a v.
*/
void QSocketDevice::setOption( Option opt, int v )
{
    int n = -1; // for really, really bad compilers
    switch ( opt ) {
    case Broadcast:
	n = SO_BROADCAST;
	break;
    case ReceiveBuffer:
	n = SO_RCVBUF;
	break;
    case ReuseAddress:
	n = SO_REUSEADDR;
	break;
    case SendBuffer:
	n = SO_SNDBUF;
	break;
    default:
	return;
    }
    if ( ::setsockopt( socket(), SOL_SOCKET, n, (char*)&v, sizeof(v)) < 0 &&
	 e == NoError ) {
	switch( errno ) {
	case EBADF:
	case ENOTSOCK:
	    e = Impossible;
	    break;
	case EFAULT:
	    e = InternalError;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
    }
}


/*!
    Connects to the IP address and port specified by \a addr and \a
    port. Returns TRUE if it establishes a connection; otherwise returns FALSE.
    If it returns FALSE, error() explains why.

    Note that error() commonly returns NoError for non-blocking
    sockets; this just means that you can call connect() again in a
    little while and it'll probably succeed.
*/
bool QSocketDevice::connect( const QHostAddress &addr, Q_UINT16 port )
{
    struct sockaddr_in a4;

    struct sockaddr *aa;
    QT_SOCKLEN_T aalen;
#if !defined(QT_NO_IPV6)
    struct sockaddr_in6 a6;

    if ( addr.isIp6Addr() ) {
	memset( &a6, 0, sizeof(a6) );
	a6.sin6_family = AF_INET6;
	a6.sin6_port = htons( port );
	Q_IPV6ADDR ip6 = addr.ip6Addr();
	memcpy( &a6.sin6_addr.s6_addr, &ip6, sizeof(ip6) );

	setProtocol( Ipv6 );
	aalen = sizeof( a6 );
	aa = (struct sockaddr *)&a6;
    } else
#endif
    {
	if ( !addr.isIp4Addr() ) {
	    qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	    e = Impossible;
	    return FALSE;
	}

	memset( &a4, 0, sizeof(a4) );
	a4.sin_family = AF_INET;
	a4.sin_port = htons( port );
	a4.sin_addr.s_addr = htonl( addr.ip4Addr() );

	setProtocol(Ipv4);
	aalen = sizeof(a4);
	aa = (struct sockaddr *)&a4;
    }

    int r = qt_socket_connect( socket(), aa, aalen );
    if ( r == 0 ) {
	fetchConnectionParameters();
	return TRUE;
    }
    if ( errno == EISCONN || errno == EALREADY || errno == EINPROGRESS ) {
	fetchConnectionParameters();
	return TRUE;
    }
    if ( e != NoError || errno == EAGAIN || errno == EWOULDBLOCK ) {
	return FALSE;
    }
    switch( errno ) {
    case EBADF:
    case ENOTSOCK:
	e = Impossible;
	break;
    case EFAULT:
    case EAFNOSUPPORT:
	e = InternalError;
	break;
    case ECONNREFUSED:
	e = ConnectionRefused;
	break;
    case ETIMEDOUT:
    case ENETUNREACH:
	e = NetworkFailure;
	break;
    case EADDRINUSE:
	e = NoResources;
	break;
    case EACCES:
    case EPERM:
	e = Inaccessible;
	break;
    default:
	e = UnknownError;
	break;
    }
    return FALSE;
}


/*!
    Assigns a name to an unnamed socket. The name is the host address
    \a address and the port number \a port. If the operation succeeds,
    bind() returns TRUE; otherwise it returns FALSE without changing
    what port() and address() return.

    bind() is used by servers for setting up incoming connections.
    Call bind() before listen().
*/
bool QSocketDevice::bind( const QHostAddress &address, Q_UINT16 port )
{
    if ( address.isIp6Addr() )
	setProtocol( Ipv6 );
    else if ( address.isIp4Addr() )
	setProtocol( Ipv4 );

    int r;
#if !defined(QT_NO_IPV6)
    struct sockaddr_in a4;
    struct sockaddr_in6 a6;

    if ( address.isIp6Addr() ) {
	memset( &a6, 0, sizeof(a6) );
	a6.sin6_family = AF_INET6;
	a6.sin6_port = htons( port );
	Q_IPV6ADDR tmp = address.ip6Addr();
	memcpy( &a6.sin6_addr.s6_addr, &tmp, sizeof(tmp) );
	setProtocol( Ipv6 );

	r = qt_socket_bind( socket(), (struct sockaddr *)&a6, sizeof(a6) );
    } else
#endif
    {
	if ( !address.isIp4Addr() ) {
	    qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	    e = Impossible;
	    return FALSE;
	}

	memset( &a4, 0, sizeof(a4) );
	a4.sin_family = AF_INET;
	a4.sin_port = htons( port );
	a4.sin_addr.s_addr = htonl( address.ip4Addr() );
	setProtocol( Ipv4 );

	r = qt_socket_bind( socket(), (struct sockaddr*)&a4, sizeof(a4) );
    }

    if ( r < 0 ) {
	switch( errno ) {
	case EADDRINUSE:
	case EINVAL:
	    e = AlreadyBound;
	    break;
	case EACCES:
	    e = Inaccessible;
	    break;
	case ENOMEM:
	    e = NoResources;
	    break;
	case EFAULT: // a was illegal
	case ENAMETOOLONG: // sz was wrong
	    e = InternalError;
	    break;
	case EBADF: // AF_UNIX only
	case ENOTSOCK: // AF_UNIX only
	case EROFS: // AF_UNIX only
	case ENOENT: // AF_UNIX only
	case ENOTDIR: // AF_UNIX only
	case ELOOP: // AF_UNIX only
	    e = Impossible;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
	return FALSE;
    }
    fetchConnectionParameters();
    return TRUE;
}


/*!
    Specifies how many pending connections a server socket can have.
    Returns TRUE if the operation was successful; otherwise returns
    FALSE. A \a backlog value of 50 is quite common.

    The listen() call only applies to sockets where type() is \c
    Stream, i.e. not to \c Datagram sockets. listen() must not be
    called before bind() or after accept().

    \sa bind(), accept()
*/
bool QSocketDevice::listen( int backlog )
{
    if ( !isValid() )
	return FALSE;
    if ( qt_socket_listen( socket(), backlog ) >= 0 )
	return TRUE;
    if ( !e )
	e = Impossible;
    return FALSE;
}


/*!
    Extracts the first connection from the queue of pending
    connections for this socket and returns a new socket identifier.
    Returns -1 if the operation failed.

    \sa bind(), listen()
*/
int QSocketDevice::accept()
{
    if ( !isValid() )
	return -1;

#if !defined (QT_NO_IPV6)
    struct sockaddr_storage aa;
#else
    struct sockaddr aa;
#endif
    QT_SOCKLEN_T l = sizeof( aa );
    bool done;
    int s;
    do {
        s = qt_socket_accept( socket(), (struct sockaddr*)&aa, &l );
        // we'll blithely throw away the stuff accept() wrote to aa
        done = TRUE;
        if ( s < 0 && e == NoError ) {
            switch( errno ) {
            case EINTR:
                done = FALSE;
                break;
#if defined(EPROTO)
	    case EPROTO:
#endif
#if defined(ENONET)
	    case ENONET:
#endif
	    case ENOPROTOOPT:
	    case EHOSTDOWN:
	    case EOPNOTSUPP:
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		// in all these cases, an error happened during connection
		// setup.  we're not interested in what happened, so we
		// just treat it like the client-closed-quickly case.
	    case EPERM:
		// firewalling wouldn't let us accept.  we treat it like
		// the client-closed-quickly case.
	    case EAGAIN:
#if EAGAIN != EWOULDBLOCK
	    case EWOULDBLOCK:
#endif
		// the client closed the connection before we got around
		// to accept()ing it.
		break;
	    case EBADF:
	    case ENOTSOCK:
		e = Impossible;
		break;
	    case EFAULT:
		e = InternalError;
		break;
	    case ENOMEM:
	    case ENOBUFS:
		e = NoResources;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    } while (!done);
    return s;
}


/*!
    Returns the number of bytes available for reading, or -1 if an
    error occurred.
*/
Q_LONG QSocketDevice::bytesAvailable() const
{
    if ( !isValid() )
	return -1;

    /*
      Apparently, there is not consistency among different operating
      systems on how to use FIONREAD.

      FreeBSD, Linux and Solaris all expect the 3rd argument to
      ioctl() to be an int, which is normally 32-bit even on 64-bit
      machines.

      IRIX, on the other hand, expects a size_t, which is 64-bit on
      64-bit machines.

      So, the solution is to use size_t initialized to zero to make
      sure all bits are set to zero, preventing underflow with the
      FreeBSD/Linux/Solaris ioctls.
    */
    size_t nbytes = 0;
    // gives shorter than true amounts on Unix domain sockets.
    if ( ::ioctl(socket(), FIONREAD, (char*)&nbytes) < 0 )
	return -1;
    return (Q_LONG) *((int *) &nbytes);
}


/*!
    Wait up to \a msecs milliseconds for more data to be available. If
    \a msecs is -1 the call will block indefinitely.

    Returns the number of bytes available for reading, or -1 if an
    error occurred.

    If \a timeout is non-null and no error occurred (i.e. it does not
    return -1): this function sets \a *timeout to TRUE, if the reason
    for returning was that the timeout was reached; otherwise it sets
    \a *timeout to FALSE. This is useful to find out if the peer
    closed the connection.

    \warning This is a blocking call and should be avoided in event
    driven applications.

    \sa bytesAvailable()
*/
Q_LONG QSocketDevice::waitForMore( int msecs, bool *timeout ) const
{
    if ( !isValid() )
	return -1;

    fd_set fds;
    struct timeval tv;

    FD_ZERO( &fds );
    FD_SET( socket(), &fds );

    tv.tv_sec = msecs / 1000;
    tv.tv_usec = (msecs % 1000) * 1000;

    int rv = select( socket() + 1, &fds, 0, 0, msecs < 0 ? 0 : &tv );

    if ( rv < 0 )
	return -1;

    if ( timeout ) {
	if ( rv == 0 )
	    *timeout = TRUE;
	else
	    *timeout = FALSE;
    }

    return bytesAvailable();
}


/*!
    Reads \a maxlen bytes from the socket into \a data and returns the
    number of bytes read. Returns -1 if an error occurred.
*/
Q_LONG QSocketDevice::readBlock( char *data, Q_ULONG maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
	qWarning( "QSocketDevice::readBlock: Null pointer error" );
    }
    if ( !isValid() ) {
	qWarning( "QSocketDevice::readBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	qWarning( "QSocketDevice::readBlock: Device is not open" );
	return -1;
    }
    if ( !isReadable() ) {
	qWarning( "QSocketDevice::readBlock: Read operation not permitted" );
	return -1;
    }
    bool done = FALSE;
    int r = 0;
    while ( done == FALSE ) {
	if ( t == Datagram ) {
#if !defined(QT_NO_IPV6)
            struct sockaddr_storage aa;
#else
            struct sockaddr_in aa;
#endif
	    memset( &aa, 0, sizeof(aa) );
	    QT_SOCKLEN_T sz;
	    sz = sizeof( aa );
	    r = ::recvfrom( socket(), data, maxlen, 0,
			    (struct sockaddr *)&aa, &sz );

	    setProtocol( qt_socket_getportaddr( (struct sockaddr *)&aa, &pp, &pa) );

	} else {
	    r = ::read( socket(), data, maxlen );
	}
	done = TRUE;
	if ( r >= 0 || errno == EAGAIN || errno == EWOULDBLOCK ) {
	    // nothing
	} else if ( errno == EINTR ) {
	    done = FALSE;
	} else if ( e == NoError ) {
	    switch( errno ) {
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
#if defined(ENONET)
	    case ENONET:
#endif
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    case EPIPE:
	    case ECONNRESET:
		// connection closed
		close();
		r = 0;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    }
    return r;
}


/*!
    Writes \a len bytes to the socket from \a data and returns the
    number of bytes written. Returns -1 if an error occurred.

    This is used for \c QSocketDevice::Stream sockets.
*/
Q_LONG QSocketDevice::writeBlock( const char *data, Q_ULONG len )
{
    if ( data == 0 && len != 0 ) {
	qWarning( "QSocketDevice::writeBlock: Null pointer error" );
	return -1;
    }
    if ( !isValid() ) {
	qWarning( "QSocketDevice::writeBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	qWarning( "QSocketDevice::writeBlock: Device is not open" );
	return -1;
    }
    if ( !isWritable() ) {
	qWarning( "QSocketDevice::writeBlock: Write operation not permitted" );
	return -1;
    }
    bool done = FALSE;
    int r = 0;
    bool timeout;
    while ( !done ) {
	r = ::write( socket(), data, len );
	done = TRUE;
	if ( r < 0 && e == NoError &&
	     errno != EAGAIN && errno != EWOULDBLOCK ) {
	    switch( errno ) {
	    case EINTR: // signal - call read() or whatever again
		done = FALSE;
		break;
	    case EPIPE:
		// connection closed
		close();
		r = 0;
		break;
	    case ENOSPC:
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
#if defined(ENONET)
	    case ENONET:
#endif
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	} else if ( waitForMore( 0, &timeout ) == 0 ) {
	    if ( !timeout ) {
		// connection closed
		close();
	    }
	}
    }
    return r;
}


/*!
    \overload

    Writes \a len bytes to the socket from \a data and returns the
    number of bytes written. Returns -1 if an error occurred.

    This is used for \c QSocketDevice::Datagram sockets. You must
    specify the \a host and \a port of the destination of the data.
*/
Q_LONG QSocketDevice::writeBlock( const char * data, Q_ULONG len,
			       const QHostAddress & host, Q_UINT16 port )
{
    if ( t != Datagram ) {
	qWarning( "QSocketDevice::sendBlock: Not datagram" );
	return -1; // for now - later we can do t/tcp
    }

    if ( data == 0 && len != 0 ) {
	qWarning( "QSocketDevice::sendBlock: Null pointer error" );
	return -1;
    }
    if ( !isValid() ) {
	qWarning( "QSocketDevice::sendBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	qWarning( "QSocketDevice::sendBlock: Device is not open" );
	return -1;
    }
    if ( !isWritable() ) {
	qWarning( "QSocketDevice::sendBlock: Write operation not permitted" );
	return -1;
    }
#if !defined(QT_NO_IPV6)
    struct sockaddr_in a4;
    struct sockaddr_in6 a6;
    struct sockaddr *aa;
    QT_SOCKLEN_T slen;

    if ( host.isIp6Addr() ) {
	memset( &a6, 0, sizeof(a6) );
	a6.sin6_family = AF_INET6;
	a6.sin6_port = htons( port );

	Q_IPV6ADDR tmp = host.ip6Addr();
	memcpy( &a6.sin6_addr.s6_addr, &tmp, sizeof(tmp) );
	slen = sizeof( a6 );
	aa = (struct sockaddr *)&a6;

	setProtocol( Ipv6 );
    } else
#endif
    {
	if ( !host.isIp4Addr() ) {
	    qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	    e = Impossible;
	    return -1;
	}

	memset( &a4, 0, sizeof(a4) );
	a4.sin_family = AF_INET;
	a4.sin_port = htons( port );
	a4.sin_addr.s_addr = htonl( host.ip4Addr() );
	slen = sizeof(a4);
	aa = (struct sockaddr *)&a4;

	setProtocol( Ipv4 );
    }

    // we'd use MSG_DONTWAIT + MSG_NOSIGNAL if Stevens were right.
    // but apparently Stevens and most implementors disagree
    bool done = FALSE;
    int r = 0;
    while ( !done ) {
	r = ::sendto( socket(), data, len, 0, aa, slen);
	done = TRUE;
	if ( r < 0 && e == NoError &&
	     errno != EAGAIN && errno != EWOULDBLOCK ) {
	    switch( errno ) {
	    case EINTR: // signal - call read() or whatever again
		done = FALSE;
		break;
	    case ENOSPC:
	    case EPIPE:
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
#if defined(ENONET)
	    case ENONET:
#endif
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    }
    return r;
}


/*!
    Fetches information about both ends of the connection: whatever is
    available.
*/
void QSocketDevice::fetchConnectionParameters()
{
    if ( !isValid() ) {
	p = 0;
	a = QHostAddress();
	pp = 0;
	pa = QHostAddress();
	return;
    }
#if !defined(QT_NO_IPV6)
    struct sockaddr_storage sa;
#else
    struct sockaddr_in sa;
#endif
    memset( &sa, 0, sizeof(sa) );
    QT_SOCKLEN_T sz;
    sz = sizeof( sa );
    if ( !::getsockname( socket(), (struct sockaddr *)(&sa), &sz ) )
	setProtocol( qt_socket_getportaddr( (struct sockaddr *)&sa, &p, &a ) );

    sz = sizeof( sa );
    if ( !::getpeername( socket(), (struct sockaddr *)(&sa), &sz ) )
	setProtocol( qt_socket_getportaddr( (struct sockaddr *)&sa, &pp, &pa ) );
}


/*!
    Returns the port number of the port this socket device is
    connected to. This may be 0 for a while, but is set to something
    sensible as soon as a sensible value is available.

    Note that for Datagram sockets, this is the source port of the
    last packet received, and that it is in native byte order.
*/
Q_UINT16 QSocketDevice::peerPort() const
{
    return pp;
}


/*!
    Returns the address of the port this socket device is connected
    to. This may be 0.0.0.0 for a while, but is set to something
    sensible as soon as a sensible value is available.

    Note that for Datagram sockets, this is the source port of the
    last packet received.
*/
QHostAddress QSocketDevice::peerAddress() const
{
    return pa;
}

#endif //QT_NO_NETWORK
