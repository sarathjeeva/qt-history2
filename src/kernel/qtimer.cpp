/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qtimer.cpp#21 $
**
** Implementation of QTimer class
**
** Created : 931111
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtimer.h"
#include "qsignal.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qtimer.cpp#21 $");


/*!
  \class QTimer qtimer.h
  \brief The QTimer class provides timer signals and single-shot timers.

  \ingroup time
  \ingroup event

  It uses \link QTimerEvent timer events\endlink internally to provide a
  more versatile timer.	 QTimer is very easy to use, create a QTimer, call
  start() to start it and connect its timeout() to the appropriate slots,
  then when the time is up it will emit timeout().

  Note that a QTimer object is destroyed automatically when its parent
  object is destroyed.

  Example:
  \code
    QTimer *timer = new QTimer( myObject );
    timer->start( 2000, TRUE );			// 2 seconds single-shot
    connect( timer, SIGNAL(timeout()),
	     myObject, SLOT(timerDone()) );
  \endcode

  An alternative is to call QObject::startTimer() for your object and
  reimplement the QObject::timerEvent() event handler in your class (it
  must inherit QObject).  The advantage is that you can have multiple
  running timers, each having a unique identifier.  The disadvantage is
  that it does not support such high-level features as single-shot timers
  or signals.
*/


const int INV_TIMER = -1;			// invalid timer id


/*!
  Constructs a timer with a \e parent and a \e name.

  Notice that the destructor of the parent object will destroy this timer
  object.
*/

QTimer::QTimer( QObject *parent, const char *name )
    : QObject( parent, name )
{
    initMetaObject();
    id = INV_TIMER;
}

/*!
  Destroys the timer.
*/

QTimer::~QTimer()
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
}


/*!
  \fn void QTimer::timeout()
  This signal is emitted when the timer is activated.
*/

/*!
  \fn bool QTimer::isActive() const
  Returns TRUE if the timer is running (pending), or FALSE is the timer is
  idle.
*/


/*!
  Starts the timer with a \e msecs milliseconds timeout.

  If \e sshot is TRUE, the timer will be activated only once,
  otherwise it will continue until it is stopped.

  Any pending timer will be stopped.

  \sa stop(), changeInterval(), isActive()
*/

int QTimer::start( int msec, bool sshot )
{
    if ( id != INV_TIMER )			// stop running timer
	stop();
    single = sshot;
    return id = startTimer( msec );
}


/*!
  Changes the timeout interval to \e msec milliseconds.

  If the timer signal is pending, it will be stopped and restarted,
  otherwise it will be started.

  \sa start(), isActive()
*/

void QTimer::changeInterval( int msec )
{
    if ( id == INV_TIMER ) {			// create new timer
	start( msec );
    } else {
	killTimer( id );			// restart timer
	id = startTimer( msec );
    }
}

/*!
  Stops the timer.
  \sa start()
*/

void QTimer::stop()
{
    if ( id != INV_TIMER ) {
	killTimer( id );
	id = INV_TIMER;
    }
}


/*!
  Handles timer events.	 Emits timeout() when a timer event is received.
*/

bool QTimer::event( QEvent *e )
{
    if ( e->type() != Event_Timer )		// ignore all other events
	return FALSE;
    if ( single )				// stop single shot timer
	stop();
    emit timeout();				// emit timeout signal
    return TRUE;
}


/*
  The QSingleShotTimer class is an internal class for implementing
  QTimer::singleShot(). It starts a timer and emits the signal
  and kills itself when it gets the timeout.
*/

class QSingleShotTimer : public QSignal
{
public:
    bool    start( int msec, QObject *r, const char *m );
protected:
    bool    event( QEvent * );
private:
    int	    timerId;
};

int  qStartTimer( int interval, QObject *obj ); // implemented in qapp_xxx.cpp
bool qKillTimer( int id );

bool QSingleShotTimer::start( int msec, QObject *r, const char *m )
{
    timerId = 0;
    if ( connect(r, m) )
	timerId = qStartTimer( msec, (QObject *)this );
    return timerId != 0;
}

bool QSingleShotTimer::event( QEvent * )
{
    activate();					// emit the signal
    qKillTimer( timerId );			// no more timeouts
    delete this;				// kill itself
    return TRUE;
}


/*!
  This static function creates a single shot timer and connects it
  to a slot. Returns TRUE if successful or FALSE if it could not
  create a timer.

  It is very convenient to use this function because you do not need to
  bother with a \link QObject::timerEvent() timerEvent\endlink or
  to create a local QTimer object.

  Example:
  \code
      #include <qapp.h>
      #include <qtimer.h>

      int main( int argc, char **argv )
      {
          QApplication a( argc, argv );
	  QTimer::singleShot( 10*60*1000, &a, SLOT(quit()) );
	   ... // create your widgets
          return a.exec();
      }
  \endcode

  This sample program automatically terminates after 10 minutes (i.e.
  600000 milliseconds).
*/

bool QTimer::singleShot( int msec, QObject *receiver, const char *member )
{
    QSingleShotTimer *sst = new QSingleShotTimer;
    if ( !sst->start(msec, receiver, member) ) { // could not start
	delete sst;
	return FALSE;
    }
    return TRUE;
}
