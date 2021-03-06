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

#include "qmouseyopy_qws.h"

#ifndef QT_NO_QWS_MOUSE_YOPY
#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qapplication.h"
#include "qscreen_qws.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

QT_BEGIN_NAMESPACE

class QWSYopyMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSYopyMouseHandlerPrivate(QWSYopyMouseHandler *h);
    ~QWSYopyMouseHandlerPrivate();

    void suspend();
    void resume();

private slots:
    void readMouseData();

private:
    int mouseFD;
    int prevstate;
    QSocketNotifier *mouseNotifier;
    QWSYopyMouseHandler *handler;
};

QWSYopyMouseHandler::QWSYopyMouseHandler(const QString &driver, const QString &device)
    : QWSMouseHandler(driver, device)
{
    d = new QWSYopyMouseHandlerPrivate(this);
}

QWSYopyMouseHandler::~QWSYopyMouseHandler()
{
    delete d;
}

void QWSYopyMouseHandler::resume()
{
    d->resume();
}

void QWSYopyMouseHandler::suspend()
{
    d->suspend();
}

QWSYopyMouseHandlerPrivate::QWSYopyMouseHandlerPrivate(QWSYopyMouseHandler *h)
    : handler(h)
{
    if ((mouseFD = open("/dev/ts", O_RDONLY)) < 0) {
        qWarning("Cannot open /dev/ts (%s)", strerror(errno));
        return;
    } else {
        sleep(1);
    }
    prevstate=0;
    mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read,
                                         this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
}

QWSYopyMouseHandlerPrivate::~QWSYopyMouseHandlerPrivate()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

#define YOPY_XPOS(d) (d[1]&0x3FF)
#define YOPY_YPOS(d) (d[2]&0x3FF)
#define YOPY_PRES(d) (d[0]&0xFF)
#define YOPY_STAT(d) (d[3]&0x01)

struct YopyTPdata {

  unsigned char status;
  unsigned short xpos;
  unsigned short ypos;

};

void QWSYopyMouseHandlerPrivate::suspend()
{
    mouseNotifier->setEnabled(false);
}


void QWSYopyMouseHandlerPrivate::resume()
{
    prevstate = 0;
    mouseNotifier->setEnabled(true);
}

void QWSYopyMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
        return;
    YopyTPdata data;

    unsigned int yopDat[4];

    int ret;

    ret=read(mouseFD,&yopDat,sizeof(yopDat));

    if(ret) {
        data.status= (YOPY_PRES(yopDat)) ? 1 : 0;
        data.xpos=YOPY_XPOS(yopDat);
        data.ypos=YOPY_YPOS(yopDat);
        QPoint q;
        q.setX(data.xpos);
        q.setY(data.ypos);
        if (data.status && !prevstate) {
          handler->mouseChanged(q,Qt::LeftButton);
        } else if(!data.status && prevstate) {
          handler->mouseChanged(q,0);
        }
        prevstate = data.status;
    }
    if(ret<0) {
        qDebug("Error %s",strerror(errno));
    }
}

QT_END_NAMESPACE

#include "qmouseyopy_qws.moc"

#endif //QT_NO_QWS_MOUSE_YOPY
