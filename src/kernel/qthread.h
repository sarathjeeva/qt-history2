/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qthread.h#6 $
**
** Definition of QThread class
**
** Created : 961223
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QTHREAD_H
#define QTHREAD_H

#ifndef QT_H
#include "qglobal.h"
#include "qwindowdefs.h"
#endif // QT_H


#if (defined(MT) || defined(REENTRANT) || defined(PTHREADS)) && defined(UNIX)
#define QTHREAD_POSIX
#endif

#if defined(_MT) && defined(_OS_WIN32_)
#define QTHREAD_WIN32
#endif

#if defined(_OS_WIN32_)
typedef HANDLE QThreadID;
#elif defined(UNIX)
typedef int QThreadID;
#else
typedef int QThreadID;
#endif

const QThreadID invalidQThreadID = (QThreadID)-1;

typedef void (*QThreadFunction)(void*);


class QThread
{
public:
    QThread( QThreadID id = invalidQThreadID );
    QThread( QThreadFunction func, void *args=0, int stackSize=-1 );
    QThread( const QThread & );
   ~QThread();

    QThread    &operator=( const QThread & );

    bool	isValid() const;

    QThreadID	id() const;

    int		priority() const;
    void	setPriority( int );

    void	suspend();
    void	resume();
    void	terminate();

  // Static thread functions
    static QThread   currentThread();
    static QThreadID currentThreadId();
    static QThreadID start( QThreadFunction func, void *args=0,
			    int stackSize=-1 );
    static void	exit( int retcode=0 );

    static void	suspend( QThreadID );
    static void	resume( QThreadID );
    static void	terminate( QThreadID );

private:
    QThreadID	tid;
};


inline QThread::QThread( QThreadID id )
    : tid(id)
{
}

inline QThread::QThread( QThreadFunction func, void *args,
			 int stackSize )
{
    tid = QThread::start( func, args, stackSize );
}

inline QThread::QThread( const QThread &t )
    : tid(t.tid)
{
}

inline QThread &QThread::operator=( const QThread &t )
{
    tid = t.tid;
    return *this;
}

inline bool QThread::isValid() const
{
    return tid >= 0;
}

inline QThreadID QThread::id() const
{
    return tid;
}

inline void QThread::suspend()
{
    suspend( id() );
}

inline void QThread::resume()
{
    resume( id() );
}

inline void QThread::terminate()
{
    terminate( id() );
}


#endif // QTHREAD_H
