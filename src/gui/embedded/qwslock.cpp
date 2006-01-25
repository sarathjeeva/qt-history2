#include "qwslock_p.h"

#include <qglobal.h>
#include <qdebug.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <time.h>
#include <linux/version.h>
#include <unistd.h>

#ifdef QT_NO_SEMAPHORE
#error QWSLock currently requires semaphores
#endif

QWSLock::QWSLock()
{
    semId = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    if (semId == -1) {
        perror("QWSLock::QWSLock");
        qFatal("Unable to create semaphore");
    }

    if (semctl(semId, BackingStore, SETVAL, 1) == -1) {
        perror("QWSLock::QWSLock");
        qFatal("Unable to initialize backingstore semaphore");
    }
    lockCount[BackingStore] = 0;

    if (semctl(semId, Communication, SETVAL, 1) == -1) {
        perror("QWSLock::QWSLock");
        qFatal("Unable to initialize communication semaphore");
    }
    lockCount[Communication] = 0;
}

QWSLock::QWSLock(int id)
{
    semId = id;
    lockCount[0] = lockCount[1] = 0;
}

QWSLock::~QWSLock()
{
    if (semId == -1)
        return;
    semctl(semId, 0, IPC_RMID, 0);
}

static bool forceLock(int semId, int semNum, int)
{
    int ret;
    do {
        sembuf sops = { semNum, -1, 0 };

        // As the BackingStore lock is a mutex, and only one process may own
        // the lock, it's safe to use SEM_UNDO. On the other hand, the
        // Communication lock is locked by the client but unlocked by the
        // server and therefore can't use SEM_UNDO.
        if (semNum == QWSLock::BackingStore)
            sops.sem_flg |= SEM_UNDO;

        ret = semop(semId, &sops, 1);
        if (ret == -1 && errno != EINTR)
            qDebug("QWSLock::lock: %s", strerror(errno));
    } while (ret == -1 && errno == EINTR);

    return (ret != -1);
}

bool QWSLock::lock(LockType type, int timeout)
{
    if (hasLock(type)) {
        ++lockCount[type];
        return true;
    }

    if (!forceLock(semId, type, timeout))
        return false;
    ++lockCount[type];
    return true;
}

bool QWSLock::hasLock(LockType type)
{
    return (lockCount[type] > 0);
}

void QWSLock::unlock(LockType type)
{
    if (hasLock(type)) {
        --lockCount[type];
        if (hasLock(type))
            return;
    }

    const int semNum = type;
    int ret;
    do {
        sembuf sops = {semNum, 1, 0};
        if (semNum == QWSLock::BackingStore)
            sops.sem_flg |= SEM_UNDO;

        ret = semop(semId, &sops, 1);
        if (ret == -1 && errno != EINTR)
            qDebug("QWSLock::unlock: %s", strerror(errno));
    } while (ret == -1 && errno == EINTR);
}

bool QWSLock::wait(LockType type, int timeout)
{
    bool ok = forceLock(semId, type, timeout);
    if (ok)
        unlock(type);
    return ok;
}

