/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwaitcondition.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qlist.h"
#include "qt_windows.h"

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>

//***********************************************************************
// QWaitConditionPrivate
// **********************************************************************

class QWaitConditionEvent
{
public:
    inline QWaitConditionEvent() : priority(0)
    {
	QT_WA ({
	    event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}, {
	    event = CreateEventA(NULL, TRUE, FALSE, NULL);
	});
    }
    inline ~QWaitConditionEvent() { CloseHandle(event); }
    int priority;
    HANDLE event;
};

typedef QList<QWaitConditionEvent *> EventQueue;

class QWaitConditionPrivate
{
public:
    QMutex mtx;
    EventQueue queue;
    EventQueue freeQueue;

    bool wait(QMutex *mutex, unsigned long time);
};

bool QWaitConditionPrivate::wait(QMutex *mutex, unsigned long time)
{
    bool ret = FALSE;

    mtx.lock();
    QWaitConditionEvent *wce =
	freeQueue.isEmpty() ? new QWaitConditionEvent : freeQueue.takeAt(0);
    wce->priority = GetThreadPriority(GetCurrentThread());

    // insert 'wce' into the queue (sorted by priority)
    QWaitConditionEvent *current = queue.first();
    int index;
    for (index = 0; index < queue.size(); ++index) {
	if (current->priority < wce->priority)
	    break;
    }
    queue.insert(index, wce);
    mtx.unlock();

    if (mutex) mutex->unlock();

    // wait for the event
    switch (WaitForSingleObject(wce->event, time)) {
    default: break;

    case WAIT_OBJECT_0:
	ret = TRUE;
	break;
    }

    if (mutex) mutex->lock();

    mtx.lock();
    // remove 'wce' from the queue
    queue.remove(wce);
    ResetEvent(wce->event);
    freeQueue.append(wce);
    mtx.unlock();

    return ret;
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
}

QWaitCondition::~QWaitCondition()
{
    Q_ASSERT(d->queue.isEmpty());

    for(EventQueue::Iterator it = d->freeQueue.begin(); it != d->freeQueue.end(); ++it)
	delete (*it);
    delete d;
}

bool QWaitCondition::wait( unsigned long time )
{
    return d->wait(0, time);
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time)
{
    if ( !mutex )
	return FALSE;

    if (mutex->d->recursive) {
#ifdef QT_CHECK_RANGE
	qWarning("QWaitCondition::wait: Cannot wait on recursive mutexes.");
#endif
	return FALSE;
    }
    return d->wait(mutex, time);
}

void QWaitCondition::wakeOne()
{
    // wake up the first thread in the queue
    QMutexLocker locker(&d->mtx);
    QWaitConditionEvent *first = d->queue.first();
    if (first)
	SetEvent(first->event);
}

void QWaitCondition::wakeAll()
{
    // wake up the all threads in the queue
    QMutexLocker locker(&d->mtx);
    for (int i = 0; i < d->queue.size(); ++i) {
	QWaitConditionEvent *current = d->queue.at(i);
	SetEvent(current->event);
    }
}
