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

#if !defined( Q_WS_QWS ) || defined( QT_NO_QWS_MULTIPROCESS )
#define QLock QWSSemaphore
#undef QT_NO_QWS_MULTIPROCESS
#include "../../src/gui/embedded/qlock.cpp"
#else
#include "qlock_p.h"
#endif

#include "qvfbshmem.h"
#include "qvfbhdr.h"

#define QTE_PIPE "QtEmbedded-%1"

#include <QFile>
#include <QTimer>

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <asm/page.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#ifndef Q_WS_QWS
// Get the name of the directory where Qt/Embedded temporary data should
// live.
static QString qws_dataDir(int qws_display_id)
{
    QByteArray dataDir = QString("/tmp/qtembedded-%1").arg(qws_display_id).toLocal8Bit();
    if (mkdir(dataDir, 0700)) {
        if (errno != EEXIST) {
            qFatal("Cannot create Qt/Embedded data directory: %s", dataDir.constData());
        }
    }

    struct stat buf;
    if (lstat(dataDir, &buf))
        qFatal("stat failed for Qt/Embedded data directory: %s", dataDir.constData());

    if (!S_ISDIR(buf.st_mode))
        qFatal("%s is not a directory", dataDir.constData());
    if (buf.st_uid != getuid())
        qFatal("Qt/Embedded data directory is not owned by user %d", getuid());

    if ((buf.st_mode & 0677) != 0600)
        qFatal("Qt/Embedded data directory has incorrect permissions: %s", dataDir.constData());
    dataDir += "/";

    return QString(dataDir);
}
#endif

static QString displayPipe;
static QString displayPiped;
class DisplayLock
{
public:
    DisplayLock() : qlock(0) {
        if (QFile::exists(displayPiped)) {
            qlock = new QLock(displayPipe, 'd', false);
            qlock->lock(QLock::Read);
        }
    }
    ~DisplayLock() {
        if (qlock) {
            qlock->unlock();
            delete qlock;
            qlock = 0;
        }
    }
private:
    QLock *qlock;
};

QShMemViewProtocol::QShMemViewProtocol(int displayid, const QSize &s,
        int d, QObject *parent)
    : QVFbViewProtocol(displayid, parent), hdr(0), dataCache(0), lockId(-1)
{
    int actualdepth=d;

    switch ( d ) {
	case 12:
	    actualdepth=16;
	    break;
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
	    break;

	default:
	    qFatal( "Unsupported bit depth %d\n", d );
    }

    int w = s.width();
    int h = s.height();

    QString username = "unknown";
    const char *logname = getenv("LOGNAME");
    if ( logname )
        username = logname;

    QString oldPipe = "/tmp/qtembedded-" + username + "/" + QString( QTE_PIPE ).arg( displayid );
    int oldPipeSemkey = ftok( oldPipe.latin1(), 'd' );
    int oldPipeLockId = semget( oldPipeSemkey, 0, 0 );
    if (oldPipeLockId >= 0){
        sembuf sops;
        sops.sem_num = 0;
        sops.sem_op = 1;
        sops.sem_flg = SEM_UNDO;
        int rv;
        do {
            rv = semop(lockId,&sops,1);
        } while ( rv == -1 && errno == EINTR );
        qFatal("Cannot create lock file as an old version of QVFb has opened %s. Close other QVFb and try again", oldPipe.latin1());
    }

    kh = new QVFbKeyPipeProtocol(displayid);
    /* should really depend on receiving qt version, but how can
       one tell? */
    mh = new QVFbMousePipe(displayid);

    QString mousePipe = mh->pipeName();

    key_t key = ftok( mousePipe.latin1(), 'b' );

    int bpl;
    if ( d == 1 )
	bpl = (w*d+7)/8;
    else
	bpl = ((w*actualdepth+31)/32)*4;

    displaySize = bpl * h;

    unsigned char *data;
    uint data_offset_value = sizeof(QVFbHeader);

    int dataSize = bpl * h + data_offset_value;
    shmId = shmget( key, dataSize, IPC_CREAT|0666);
    if ( shmId != -1 )
	data = (unsigned char *)shmat( shmId, 0, 0 );
    else {
	struct shmid_ds shm;
	shmctl( shmId, IPC_RMID, &shm );
	shmId = shmget( key, dataSize, IPC_CREAT|0666);
	if ( shmId == -1 )
	    qFatal( "Cannot get shared memory 0x%08x", key );
	data = (unsigned char *)shmat( shmId, 0, 0 );
    }

    if ( (long)data == -1 ){
        delete kh;
        delete mh;
	qFatal( "Cannot attach to shared memory %d",shmId );
    }
    dataCache = (unsigned char *)malloc(displaySize);
    memset(dataCache, 0, displaySize);
    memset(data+sizeof(QVFbHeader), 0, displaySize);

    hdr = (QVFbHeader *)data;
    hdr->width = w;
    hdr->height = h;
    hdr->depth = actualdepth;
    hdr->linestep = bpl;
    hdr->numcols = 0;
    hdr->dataoffset = data_offset_value;
    hdr->update = QRect();

#ifdef Q_WS_QWS
    displayPipe = qws_dataDir() + QString( QTE_PIPE ).arg( displayid );
#else
    displayPipe = qws_dataDir(displayid) + QString( QTE_PIPE ).arg( displayid );
#endif
    displayPiped = displayPipe + 'd';


    mRefreshTimer = new QTimer( this );
    connect( mRefreshTimer, SIGNAL(timeout()), this, SLOT(flushChanges()) );
}

QShMemViewProtocol::~QShMemViewProtocol()
{
    struct shmid_ds shm;
    shmdt( (char*)hdr );
    shmctl( shmId, IPC_RMID, &shm );
    free(dataCache);
    delete kh;
    delete mh;
}

int QShMemViewProtocol::width() const
{
    return hdr->width;
}

int QShMemViewProtocol::height() const
{
    return hdr->height;
}

int QShMemViewProtocol::depth() const
{
    return hdr->depth;
}

int QShMemViewProtocol::linestep() const
{
    return hdr->linestep;
}

int  QShMemViewProtocol::numcols() const
{
    return hdr->numcols;
}

QRgb *QShMemViewProtocol::clut() const
{
    return hdr->clut;
}

unsigned char *QShMemViewProtocol::data() const
{
    return dataCache;
    //return ((unsigned char *)hdr)+hdr->dataoffset;
}

void QShMemViewProtocol::flushChanges()
{
    // based of dirty rect, copy changes from hdr to hdrcopy
    QRect r;
    {
        DisplayLock();
        if (hdr->dirty) {
            r = hdr->update;
            hdr->dirty = false;
            hdr->update = QRect();
            /* copy the memory area */
            /* for now, be inefficient. */
            memcpy(dataCache, ((char *)hdr) + hdr->dataoffset, displaySize);
        }
    }
    emit displayDataChanged(r);
}

void QShMemViewProtocol::setRate(int interval) 
{
    if (interval > 0)
        return mRefreshTimer->start(1000/interval);
    else
        mRefreshTimer->stop();
}

int QShMemViewProtocol::rate() const
{
    int i = mRefreshTimer->interval();
    if (i > 0)
        return 1000/i;
    else
        return 0;
}
