/****************************************************************************
**
** Definition of QIOEngine class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QIOENGINE_H__
#define __QIOENGINE_H__

#include "qiodevice.h"

class QIOEnginePrivate;
class QIOEngine
{
protected:
    QIOEnginePrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(QIOEngine)
public:
    virtual ~QIOEngine();

    virtual bool open(int flags) = 0;
    virtual bool close() = 0;
    virtual void flush() = 0;

    virtual QIODevice::Offset size() const = 0;
    virtual QIODevice::Offset at() const = 0;
    virtual bool seek(QIODevice::Offset) = 0;
    virtual bool atEnd() const;

    virtual bool isSequential() const = 0;

    virtual Q_LONG readBlock(char *data, Q_LONG maxlen) = 0;
    virtual Q_LONG writeBlock(const char *data, Q_LONG len) = 0;
    virtual Q_LONG readLine(char *data, Q_LONG maxlen);

    virtual int getch();
    virtual int putch(int);
    virtual int ungetch(int) = 0;

    virtual QIODevice::Status errorStatus() const;
    virtual QString errorMessage() const;

protected:
    QIOEngine();
    QIOEngine(QIOEnginePrivate &);
};

extern QString qt_errorstr(int errorCode);

#endif /* __QIOENGINE_H__ */
