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

#ifndef QBUFFER_H
#define QBUFFER_H

#include "QtCore/qiodevice.h"
#include "QtCore/qbytearray.h"

class QObject;
class QBufferPrivate;

class Q_CORE_EXPORT QBuffer : public QIODevice
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif

public:
#ifndef QT_NO_QOBJECT
     explicit QBuffer(QObject *parent = 0);
     QBuffer(QByteArray *buf, QObject *parent = 0);
#else
     QBuffer();
     explicit QBuffer(QByteArray *buf);
#endif
    ~QBuffer();

    QByteArray &buffer();
    const QByteArray &buffer() const;
    void setBuffer(QByteArray *a);

    void setData(const QByteArray &data);
    inline void setData(const char *data, int len) { setData(QByteArray(data, len)); }
    const QByteArray &data() const;

    bool open(OpenMode openMode);

    void close();
    Q_LONGLONG size() const;
    Q_LONGLONG pos() const;
    bool seek(Q_LONGLONG off);
    bool atEnd() const;

protected:
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    Q_DECLARE_PRIVATE(QBuffer)
    Q_DISABLE_COPY(QBuffer)

    Q_PRIVATE_SLOT(d, void emitSignals())
};

#endif // QBUFFER_H
