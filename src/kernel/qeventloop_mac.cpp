/****************************************************************************
**
** Implementation of timers, socket notifiers, and event handling
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "qplatformdefs.h"
#include "qeventloop.h"
#include "qapplication.h"
#include "qevent.h"
#include "qt_mac.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#ifdef Q_WS_MAC9
#  define QMAC_EVENT_NOWAIT 0.01
#else
#  define QMAC_EVENT_NOWAIT kEventDurationNoWait
#endif

#include "qeventloop_p.h"
#define d d_func()
#define q q_func()

//Externals
void qt_event_request_timer(TimerInfo *); //qapplication_mac.cpp
TimerInfo *qt_event_get_timer(EventRef); //qapplication_mac.cpp
void qt_event_request_select(QEventLoop *); //qapplication_mac.cpp
void qt_event_request_sockact(QEventLoop *); //qapplication_mac.cpp
void qt_event_request_updates(); //qapplication_mac.cpp
void qt_event_request_wakeup(); //qapplication_mac.cpp
bool qt_mac_send_event(QEventLoop::ProcessEventsFlags, EventRef, WindowPtr =NULL); //qapplication_mac.cpp
extern bool qt_is_gui_used; //qapplication.cpp

//Local Statics

//pre/post select callbacks
typedef void (*VFPTR)();
typedef QList<VFPTR> QVFuncList;
void qt_install_preselect_handler(VFPTR);
void qt_remove_preselect_handler(VFPTR);
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler(VFPTR);
void qt_remove_postselect_handler(VFPTR);
static QVFuncList *qt_postselect_handler = 0;
void qt_install_preselect_handler(VFPTR handler)
{
    if(!qt_preselect_handler)
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append(handler);
}
void qt_remove_preselect_handler(VFPTR handler)
{
    if(qt_preselect_handler) {
	QVFuncList::Iterator it = qt_preselect_handler->find(handler);
	if(it != qt_preselect_handler->end())
	    qt_preselect_handler->remove(it);
    }
}
void qt_install_postselect_handler(VFPTR handler)
{
    if(!qt_postselect_handler)
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend(handler);
}
void qt_remove_postselect_handler(VFPTR handler)
{
    if(qt_postselect_handler) {
	QVFuncList::Iterator it = qt_postselect_handler->find(handler);
	if(it != qt_postselect_handler->end())
	    qt_postselect_handler->remove(it);
    }
}

/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
class QMacSockNotPrivate {
public:
    CFSocketRef socknot;
    CFRunLoopSourceRef event_source;
};
QSockNotType::QSockNotType()
{
    list.setAutoDelete(true);
    FD_ZERO(&select_fds);
    FD_ZERO(&enabled_fds);
    FD_ZERO(&pending_fds);
}

QSockNot::~QSockNot()
{
    if(mac_d) {
	if(mac_d->socknot)
	    CFRelease(mac_d->socknot);
	if(mac_d->event_source)
	    CFRelease(mac_d->event_source);
    }
}

/*****************************************************************************
  Timers stuff
 *****************************************************************************/

/* timer code */
struct TimerInfo {
    int id;
    QObject *obj;
    bool pending;
    //type switches
    enum TimerType { TIMER_ZERO, TIMER_QT, TIMER_MAC, TIMER_ANY } type;
    union {
	EventLoopTimerRef mac_timer;
	struct {
	    timeval  interval;
	    timeval  timeout;
	} qt_timer;
    } u;
};
static int zero_timer_count = 0;
typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs
static TimerList *timerList	= 0;		// timer list
static EventLoopTimerUPP timerUPP = NULL;       //UPP

#ifdef Q_OS_DARWIN
static inline bool operator<(const timeval &t1, const timeval &t2)
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}
static inline bool operator==( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
}
static inline timeval &operator+=(timeval &t1, const timeval &t2)
{
    t1.tv_sec += t2.tv_sec;
    if((t1.tv_usec += t2.tv_usec) >= 1000000) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}
static inline timeval operator+(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if((tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}
static inline timeval operator-(const timeval &t1, const timeval &t2)
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if((tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}
static timeval	watchtime;			// watch if time is turned back
timeval	*qt_wait_timer_max = 0;
static inline void getTime(timeval &t)	// get time of day
{
    gettimeofday(&t, 0);
    while(t.tv_usec >= 1000000) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while(t.tv_usec < 0) {
	if(t.tv_sec > 0) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}
static void repairTimer(const timeval &time)	// repair broken timer
{
    if(!timerList)				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while(t) {				// repair all timers
	if(t->type == TimerInfo::TIMER_QT)
	    t->u.qt_timer.timeout = t->u.qt_timer.timeout - diff;
	else
	    qWarning("Qt: internal: %s:%d This can't happen!", __FILE__, __LINE__);
	t = timerList->next();
    }
}
static timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if(timerList && timerList->count()) {	// there are waiting timers
	getTime(currentTime);
	if(first) {
	    if(currentTime < watchtime)	// clock was turned back
		repairTimer(currentTime);
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if(t->type == TimerInfo::TIMER_QT) {
	    if(currentTime < t->u.qt_timer.timeout) {	// time to wait
		tm = t->u.qt_timer.timeout - currentTime;
	    } else {
		tm.tv_sec  = 0;			// no time to wait
		tm.tv_usec = 0;
	    }
	    if(qt_wait_timer_max && *qt_wait_timer_max < tm)
		tm = *qt_wait_timer_max;
	} else {
	    qWarning("Qt: internal: %s:%d Unexpected condition reached.", __FILE__, __LINE__);
	}
	return &tm;
    }
    if(qt_wait_timer_max) {
	tm = *qt_wait_timer_max;
	return &tm;
    }
    return 0;					// no timers
}
#endif

static void insertTimer(const TimerInfo *ti)	// insert timer info into list
{
    int index = 0;
    for(TimerInfo *t = timerList->first(); t; index++) {	// list is sorted by timeout
	if(t->type == TimerInfo::TIMER_QT && ti->u.qt_timer.timeout < t->u.qt_timer.timeout)
	    break;
	t = timerList->next();
    }
    timerList->insert(index, ti);		// inserts sorted
}

/* timer call back */
QMAC_PASCAL static void qt_activate_mac_timer(EventLoopTimerRef, void *data)
{
    TimerInfo *tmr = ((TimerInfo *)data);
    if(tmr->pending)
	return;
    if(tmr->type != TimerInfo::TIMER_MAC) { //can't really happen, can it?
	qWarning("Qt: internal %s: %d WH0A", __FILE__, __LINE__);
	return;
    }
    if(QMacBlockingFunction::blocking()) { //just send it immediately
	/* someday this is going to lead to an infite loop, I just know it. I should be marking the
	   pending here, and unmarking, but of course single shot timers are removed between now
	   and the return (down 4 lines) */
	QTimerEvent e(tmr->id);
	QApplication::sendEvent(tmr->obj, &e);
	QApplication::flush();
	return;
    }
    tmr->pending = TRUE;
    qt_event_request_timer(tmr);
}

//central cleanup
QMAC_PASCAL static Boolean find_timer_event(EventRef event, void *data)
{
    return (qt_event_get_timer(event) == ((TimerInfo *)data));
}

static bool killTimer(TimerInfo *t, bool remove=TRUE)
{
    t->pending = TRUE;
    if(t->type == TimerInfo::TIMER_MAC) {
	RemoveEventLoopTimer(t->u.mac_timer);
	if(t->pending) {
	    EventComparatorUPP fnc = NewEventComparatorUPP(find_timer_event);
	    FlushSpecificEventsFromQueue(GetMainEventQueue(), fnc, (void *)t);
	    DisposeEventComparatorUPP(fnc);
	}
    } else if(t->type == TimerInfo::TIMER_ZERO) {
	zero_timer_count--;
    }
    return remove ? timerList->remove() : TRUE;
}

//
// Timer initialization and cleanup routines
//
static void initTimers()			// initialize timers
{
    timerUPP = NewEventLoopTimerUPP(qt_activate_mac_timer);
    Q_CHECK_PTR(timerUPP);
    timerList = new TimerList;
    timerList->setAutoDelete(TRUE);
    zero_timer_count = 0;
}

static void cleanupTimers()			// cleanup timer data structure
{
    zero_timer_count = 0;
    if(timerList) {
	for(register TimerInfo *t = timerList->first(); t; t = timerList->next())
	    killTimer(t, FALSE);
	delete timerList;
	timerList = 0;
    }
    if(timerUPP) {
	DisposeEventLoopTimerUPP(timerUPP);
	timerUPP = NULL;
    }
}

static int qt_activate_timers(TimerInfo::TimerType types = TimerInfo::TIMER_ANY)
{
    if(types == TimerInfo::TIMER_ZERO) {
	if(!zero_timer_count)
	    return 0;
	int ret = 0;
	for(register TimerInfo *t = timerList->first();
	    ret != zero_timer_count && t; t = timerList->next()) {
	    if(t->type == TimerInfo::TIMER_ZERO) {
		ret++;
		QTimerEvent e(t->id);
		QApplication::sendEvent(t->obj, &e);	// send event
	    }
	}
	return ret;
    }

    if(!timerList || !timerList->count())	// no timers
	return 0;
    int n_act = 0;
#ifdef Q_OS_DARWIN
    bool first = TRUE;
    int maxCount = timerList->count();
    timeval currentTime;
    TimerInfo *begin = 0;
    register TimerInfo *t;
    for ( ;; ) {
	if ( ! maxCount )
	    break;
	getTime(currentTime);			// get current time
	if(first) {
	    if(currentTime < watchtime)	// clock was turned back
		repairTimer(currentTime);
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if(t->type == TimerInfo::TIMER_QT) {
	    if(!t || currentTime < t->u.qt_timer.timeout) // no timer has expired
		break;
	    if ( ! begin ) {
		begin = t;
	    } else if ( begin == t ) {
		// avoid sending the same timer multiple times
		break;
	    } else if ( t->u.qt_timer.interval <  begin->u.qt_timer.interval ||
			t->u.qt_timer.interval == begin->u.qt_timer.interval ) {
		begin = t;
	    }
	    timerList->take();			// unlink from list
	    t->u.qt_timer.timeout += t->u.qt_timer.interval;
	    if(t->u.qt_timer.timeout < currentTime)
		t->u.qt_timer.timeout = currentTime + t->u.qt_timer.interval;
	    insertTimer(t);			// relink timer
	    if(t->u.qt_timer.interval.tv_usec > 0 || t->u.qt_timer.interval.tv_sec > 0)
		n_act++;
	    else
		maxCount--;
	    QTimerEvent e(t->id);
	    QApplication::sendEvent(t->obj, &e);	// send event
	    if ( timerList->findRef( begin ) == -1 )
		begin = 0;
	}
    }
#endif
    return n_act;
}

int qStartTimer(int interval, QObject *obj)
{
    if(!timerList)				// initialize timer data
	initTimers();
    TimerInfo *t = new TimerInfo;		// create timer
    t->obj = obj;
    t->pending = TRUE;
#ifdef Q_OS_DARWIN
    if(!qt_is_gui_used) {
	t->type = TimerInfo::TIMER_QT;
	t->u.qt_timer.interval.tv_sec  = interval/1000;
	t->u.qt_timer.interval.tv_usec = (interval%1000)*1000;
	timeval currentTime;
	getTime(currentTime);
	t->u.qt_timer.timeout = currentTime + t->u.qt_timer.interval;
    } else
#endif
    if(!interval) {
	t->type = TimerInfo::TIMER_ZERO;
	zero_timer_count++;
    } else {
	t->type = TimerInfo::TIMER_MAC;
	EventTimerInterval mint = (((EventTimerInterval)interval) / 1000);
	if(InstallEventLoopTimer(GetMainEventLoop(), mint, mint,
				 timerUPP, t, &t->u.mac_timer)) {
	    delete t;
	    return 0;
	}
    }
    static int serial_id = 666;
    t->id = serial_id++;
    t->pending = FALSE;
    insertTimer(t);
    return t->id;
}

bool qKillTimer(int id)
{
    if(!timerList || id <= 0)
	return FALSE;				// not init'd or invalid timer
    register TimerInfo *t = timerList->first();
    while(t && (t->id != id)) // find timer info in list
	t = timerList->next();
    if(t)					// id found
	return killTimer(t);
    return FALSE; // id not found
}

bool qKillTimer(QObject *obj)
{
    if(!timerList)				// not initialized
	return FALSE;
    register TimerInfo *t = timerList->first();
    while(t) {				// check all timers
	if(t->obj == obj) {			// object found
	    killTimer(t);
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}


/*****************************************************************************
  QEventLoop Implementation
 *****************************************************************************/

// Initializations / Cleanup
void QEventLoop::init()
{
#ifdef Q_OS_UNIX
    if(!qt_is_gui_used) {
	pipe(d->thread_pipe);
	fcntl(d->thread_pipe[0], F_SETFD, FD_CLOEXEC);
	fcntl(d->thread_pipe[1], F_SETFD, FD_CLOEXEC);
    }
    d->select_timer = NULL;
    d->sn_highest = -1;
#endif
}

void QEventLoop::cleanup()
{
    if(!qt_is_gui_used) {
	close(d->thread_pipe[0]);
	close(d->thread_pipe[1]);
    }
    cleanupTimers();
    if(d->select_timer) {
	RemoveEventLoopTimer(d->select_timer);
	d->select_timer = NULL;
    }
}

void QEventLoop::macHandleTimer(TimerInfo *t)
{
    if(!timerList)
	return;
    if(t && t->pending) {
	t->pending = FALSE;
	QTimerEvent e(t->id);
	QApplication::sendEvent(t->obj, &e);	// send event
    }
}

/**************************************************************************
    Socket Notifiers
 *************************************************************************/
static EventLoopTimerUPP mac_select_timerUPP = NULL;
static void qt_mac_select_cleanup()
{
    DisposeEventLoopTimerUPP(mac_select_timerUPP);
    mac_select_timerUPP = NULL;
}
QMAC_PASCAL void qt_mac_select_timer_callbk(EventLoopTimerRef, void *me)
{
    QEventLoop *eloop = (QEventLoop*)me;
    if(QMacBlockingFunction::blocking()) { //just send it immediately
	timeval tm;
	memset(&tm, '\0', sizeof(tm));
	eloop->macHandleSelect(&tm);
    } else {
	qt_event_request_select(eloop);
    }
}
/* This function just exists so that I can have the function as a friend, and not have to put all the types
   that go into the real callback into qwindowdefs.h  --Sam */
void qt_mac_internal_select_callbk(int sock, int type, QEventLoop *eloop)
{
#if 0
     for(int i=0; i<3; i++) {
	if(!eloop->d->sn_vec[i].list)
	    continue;
	QList<QSockNot *> &list = eloop->d->sn_vec[i].list;
	for(int i = 0; i < list.size(); ++i) {
	    QSockNot *sn = list.at(i);
	    if(sn->fd == sock && sn->obj->type() == type)
		eloop->setSocketNotifierPending(sn->obj);
	}
    }
     if(QMacBlockingFunction::blocking()) //just send it immediately
	 eloop->activateSocketNotifiers();
     else
	 qt_event_request_sockact(eloop);
#else
     /* This is a bit of a shame, but CFSockets can starve each other out, however we have special code
	to prevent that from happening in our general select() case, so we'll we'll just use the moment
	we know data is there to fire the 'old timer case' and all will be happy (sockets will fire faster than
	as through the timer, but our fancy handling will prevent socket starvation */
     Q_UNUSED(sock);
     Q_UNUSED(type);
     qt_mac_select_timer_callbk(0, eloop);
#endif
}
static void qt_mac_select_callbk(CFSocketRef s, CFSocketCallBackType t, CFDataRef, const void *, void *me)
{
    int in_type = 0;
    if(t == kCFSocketReadCallBack) {
	in_type = QSocketNotifier::Read;
    } else if(t == kCFSocketWriteCallBack) {
	in_type = QSocketNotifier::Write;
    } else {
	qWarning("Qt: internal: %s:%d This can't happen!", __FILE__, __LINE__);
	return;
    }
    qt_mac_internal_select_callbk(CFSocketGetNative(s), in_type, (QEventLoop*)me);
}

int QEventLoop::macHandleSelect(timeval *tm)
{
    if(qt_preselect_handler) {
	QVFuncList::Iterator end = qt_preselect_handler->end();
	for(QVFuncList::Iterator it = qt_preselect_handler->begin(); it != end; ++it)
	    (**it)();
    }
#ifdef Q_OS_DARWIN
    int highest = 0;
    if(d->sn_highest >= 0) {			// has socket notifier(s)
	if(!d->sn_vec[0].list.isEmpty())
	    d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
	else
	    FD_ZERO(&d->sn_vec[0].select_fds);
	if(!d->sn_vec[1].list.isEmpty())
	    d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
	else
	    FD_ZERO(&d->sn_vec[1].select_fds);
	if(!d->sn_vec[2].list.isEmpty())
	    d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
	else
	    FD_ZERO(&d->sn_vec[2].select_fds);
    } else {
	FD_ZERO(&d->sn_vec[0].select_fds);
	FD_ZERO(&d->sn_vec[1].select_fds);
	FD_ZERO(&d->sn_vec[2].select_fds);
    }
    highest = d->sn_highest;

    if(!qt_is_gui_used) {
	FD_SET(d->thread_pipe[0], &d->sn_vec[0].select_fds);
	highest = QMAX(highest, d->thread_pipe[0]);
    }
    int nsel = select(highest + 1, &d->sn_vec[0].select_fds,
		      !d->sn_vec[1].list.isEmpty() ? &d->sn_vec[1].select_fds : 0,
		      !d->sn_vec[2].list.isEmpty() ? &d->sn_vec[2].select_fds : 0, tm);
#endif
    if(qt_postselect_handler) {
	QVFuncList::Iterator end = qt_postselect_handler->end();
	for(QVFuncList::Iterator it = qt_postselect_handler->begin(); it != end; ++it)
	    (**it)();
    }
#ifdef Q_OS_DARWIN
    if(nsel == -1) {
	if(errno == EINTR || errno == EAGAIN)
	    errno = 0;
    } else if(nsel > 0 && highest >= 0) {
	if(qt_is_gui_used) {
	    qt_event_request_updates();
	} else if(FD_ISSET(d->thread_pipe[0], &d->sn_vec[0].select_fds)) {
	    char c;
	    ::read(d->thread_pipe[0], &c, 1);
	}
	// if select says data is ready on any socket, then set the socket notifier
	// to pending
	for(int i=0; i<3; i++) {
	    QList<QSockNot *> &list = d->sn_vec[i].list;
	    for (int j = 0; j < list.size(); ++j) {
		QSockNot *sn = list.at(j);
		if(FD_ISSET(sn->fd, &d->sn_vec[i].select_fds))
		    setSocketNotifierPending(sn->obj);
	    }
	}
	return activateSocketNotifiers();
    }
    return 0;
#endif
}

void QEventLoop::registerSocketNotifier(QSocketNotifier *notifier)
{
#ifdef Q_OS_UNIX
    int sockfd = notifier->socket();
    int type = notifier->type();
    if(sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
	qWarning("Qt: QSocketNotifier: Internal error");
	return;
    }

    QList<QSockNot *> &list = d->sn_vec[type].list;
    fd_set *fds  = &d->sn_vec[type].enabled_fds;

    QSockNot *sn = new QSockNot;
    sn->mac_d = 0;
    sn->obj = notifier;
    sn->fd = sockfd;
    sn->queue = &d->sn_vec[type].pending_fds;

    int i;
    for (i = 0; i < list.size(); ++i) {
	QSockNot *p = list.at(i);
	if (p->fd < sockfd )
	    break;
	if ( p->fd == sockfd ) {
	    static const char *t[] = { "read", "write", "exception" };
	    qWarning( "QSocketNotifier: Multiple socket notifiers for "
		      "same socket %d and type %s", sockfd, t[type] );
	}
    }
    list.insert( i, sn );

    FD_SET(sockfd, fds);
    d->sn_highest = QMAX(d->sn_highest, sockfd);
    if(qt_is_gui_used) {
	if(type == QSocketNotifier::Read || type == QSocketNotifier::Write) {
	    CFSocketContext context;
	    memset(&context, '\0', sizeof(context));
	    context.info = this;
	    sn->mac_d = new QMacSockNotPrivate;
	    sn->mac_d->socknot = CFSocketCreateWithNative(NULL, sockfd, kCFSocketReadCallBack|kCFSocketWriteCallBack,
							  qt_mac_select_callbk, &context);
	    if(!sn->mac_d->socknot) {
		delete sn->mac_d;
		sn->mac_d = NULL;
	    } else {
		sn->mac_d->event_source = CFSocketCreateRunLoopSource(NULL, sn->mac_d->socknot, 0);
		CFRunLoopAddSource(CFRunLoopGetCurrent(), sn->mac_d->event_source, kCFRunLoopCommonModes);
	    }
	} 
	if(!d->select_timer) {
	    if(!mac_select_timerUPP) {
		mac_select_timerUPP = NewEventLoopTimerUPP(qt_mac_select_timer_callbk);
		qAddPostRoutine(qt_mac_select_cleanup);
	    }
	    InstallEventLoopTimer(GetMainEventLoop(), 0.1, 0.1,
				  mac_select_timerUPP, (void *)this, &d->select_timer);
	}
    }
#endif
}

void QEventLoop::unregisterSocketNotifier(QSocketNotifier *notifier)
{
#ifdef Q_OS_UNIX
    int sockfd = notifier->socket();
    int type = notifier->type();
    if(sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
	qWarning("Qt: QSocketNotifier: Internal error");
	return;
    }

    QList<QSockNot *> &list = d->sn_vec[type].list;
    fd_set *fds = &d->sn_vec[type].enabled_fds;

    QSockNot *sn;
    int i;
    for (i = 0; i < list.size(); ++i) {
	sn = list.at(i);
	if(sn->obj == notifier && sn->fd == sockfd)
	    break;
    }
    if(i == list.size()) // not found
	return;

    FD_CLR(sockfd, fds);			// clear fd bit
    FD_CLR(sockfd, sn->queue);
    d->sn_pending_list.remove(sn);		// remove from activation list
    list.remove(sn);				// remove notifier found above

    if(d->sn_highest == sockfd) {		// find highest fd
	d->sn_highest = -1;
	for(int i=0; i<3; i++) {
	    if(!d->sn_vec[i].list.isEmpty())
		d->sn_highest = QMAX(d->sn_highest,  // list is fd-sorted
				      d->sn_vec[i].list.first()->fd);
	}
    }
    if(d->sn_highest == -1 && d->select_timer) {
	RemoveEventLoopTimer(d->select_timer);
	d->select_timer = NULL;
    }
#endif
}

int QEventLoop::activateSocketNotifiers()
{
    if(d->sn_pending_list.isEmpty())
	return 0;

    // activate entries
    int n_act = 0;
    QEvent event(QEvent::SockAct);
    while (!d->sn_pending_list.isEmpty()) {
	QSockNot *sn = d->sn_pending_list.takeAt(0);
	if(FD_ISSET(sn->fd, sn->queue)) {
	    if(sn->obj->type() == QSocketNotifier::Read && sn->mac_d) {
		//we manually re-enable when we send so that we can be sure we want the next read (otherwise
		//we'll get it from the timer) and we don't drive up CPU usage
		CFSocketEnableCallBacks(sn->mac_d->socknot, kCFSocketReadCallBack);
	    }
	    FD_CLR(sn->fd, sn->queue);
	    QApplication::sendEvent(sn->obj, &event);
	    n_act++;
	}
    }
    return n_act;
}

void QEventLoop::setSocketNotifierPending(QSocketNotifier *notifier)
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if(sockfd < 0 || type < 0 || type > 2 || notifier == 0) {
	qWarning("Qt: QSocketNotifier: Internal error");
	return;
    }

    QList<QSockNot *> &list = d->sn_vec[type].list;
    QSockNot *sn = NULL;
    for(int i = 0; i < list.size(); ++i) {
	sn = list.at(i);
	if (sn->obj == notifier && sn->fd == sockfd)
	    break;
    }
    if(!sn) // not found
	return;

    // We choose a random activation order to be more fair under high load.
    // If a constant order is used and a peer early in the list can
    // saturate the IO, it might grab our attention completely.
    // Also, if we're using a straight list, the callback routines may
    // delete other entries from the list before those other entries are
    // processed.
    if(!FD_ISSET(sn->fd, sn->queue)) {
	d->sn_pending_list.insert((rand() & 0xff) % (d->sn_pending_list.count()+1), sn);
	FD_SET(sn->fd, sn->queue);
    }
}

bool QEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || (qt_is_gui_used && GetNumEventsInQueue(GetMainEventQueue()));
}

bool QEventLoop::processEvents(ProcessEventsFlags flags)
{
#if 0
    //TrackDrag says you may not use the EventManager things..
    if(qt_mac_in_drag) {
	qWarning("Qt: Cannot process events whilst dragging!");
	return FALSE;
    }
#endif
    int	   nevents = 0;
#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker(QApplication::qt_mutex);
#endif

    if(qt_is_gui_used) {
	if(!qt_mac_safe_pdev) { //create an empty widget and this can be used for a port anytime
	    QWidget *tlw = new QWidget(NULL, "empty_widget", Qt::WDestructiveClose);
	    tlw->hide();
	    qt_mac_safe_pdev = tlw;
	}

	QApplication::sendPostedEvents();
	qt_activate_timers(TimerInfo::TIMER_ZERO); //try to send null timers..

	EventRef event;
	do {
	    do {
		if(ReceiveNextEvent(0, 0, QMAC_EVENT_NOWAIT, TRUE, &event) != noErr)
		    break;
		if(qt_mac_send_event(flags, event))
		    nevents++;
		ReleaseEvent(event);
	    } while(GetNumEventsInQueue(GetMainEventQueue()));
	    QApplication::sendPostedEvents();
	} while(GetNumEventsInQueue(GetMainEventQueue()));
    }
    if(d->quitnow || d->exitloop)
	return FALSE;

    QApplication::sendPostedEvents();
    bool canWait = d->exitloop || d->quitnow ? FALSE : (flags & WaitForMore);

    if(!qt_is_gui_used) {
	timeval *tm = NULL;
	if(!canWait) { 		// no time to wait
	    static timeval zerotm;
	    if(!tm)
		tm = &zerotm;
	    tm->tv_sec  = 0;
	    tm->tv_usec = 0;
	} else {
	    tm = qt_wait_timer();
	}
	if(!(flags & ExcludeSocketNotifiers)) {
	    emit aboutToBlock();
#if defined(QT_THREAD_SUPPORT)
	    locker.mutex()->unlock();
#endif
	    nevents += macHandleSelect(tm);
#if defined(QT_THREAD_SUPPORT)
	    locker.mutex()->lock();
#endif
	}

	// we are awake, broadcast it
	emit awake();

	nevents += qt_activate_timers();
    } else if(canWait && !zero_timer_count) {
	emit aboutToBlock();
#if defined(QT_THREAD_SUPPORT)
	locker.mutex()->unlock();
#endif
#if defined( QMAC_USE_APPLICATION_EVENT_LOOP )
	RunApplicationEventLoop();
#else
	while(CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e20, true) == kCFRunLoopRunTimedOut);
#endif
#if defined(QT_THREAD_SUPPORT)
	locker.mutex()->lock();
#endif

	// we are awake, broadcast it
	emit awake();
    }
    return nevents > 0;
}

int QEventLoop::timeToWait() const
{
    if(qt_is_gui_used)
	return -1;
    if(timeval *tm = qt_wait_timer())
	return (tm->tv_sec*1000) + (tm->tv_usec/1000);
    return -1;
}

int QEventLoop::activateTimers()
{
    if (qt_is_gui_used)
	return 0;
    return qt_activate_timers();
}

void QEventLoop::wakeUp()
{
    if(qt_is_gui_used) {
	qt_event_request_wakeup();
	return;
    }
    char c = 0;
    ::write(d->thread_pipe[1], &c, 1);
}



/* This allows the eventloop to block, and will keep things going - including keeping away
   the evil spinning cursor */
class QMacBlockingFunction::Object : public QObject
{
    QAtomic ref;
public:
    Object() { startTimer(1); }

    void addRef() { ++ref; }
    bool subRef() { return (--ref); }

protected:
    void timerEvent(QTimerEvent *)
    {
	if(qt_activate_timers(TimerInfo::TIMER_ZERO))
	    QApplication::flush();
    }
};
QMacBlockingFunction::Object *QMacBlockingFunction::block = NULL;
QMacBlockingFunction::QMacBlockingFunction()
{
    if(!block) 
	block = new QMacBlockingFunction::Object;
    block->addRef();
}
QMacBlockingFunction::~QMacBlockingFunction()
{
    Q_ASSERT(block);
    if(!block->subRef())
	delete block;
}
