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

#ifndef QMIME_H
#define QMIME_H

#include <QtCore/qmimedata.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class Q_GUI_EXPORT QMimeSource
{
public:
    virtual ~QMimeSource();
    virtual const char* format(int n = 0) const = 0;
    virtual bool provides(const char*) const;
    virtual QByteArray encodedData(const char*) const = 0;
};


#if defined(Q_WS_WIN)

typedef struct tagFORMATETC FORMATETC;
typedef struct tagSTGMEDIUM STGMEDIUM;
struct IDataObject;

#include <QtCore/qvariant.h>

/*
  Encapsulation of conversion between MIME and Windows CLIPFORMAT.
  Not need on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QWindowsMime {
public:
    QWindowsMime();
    virtual ~QWindowsMime();

    // for converting from Qt
    virtual bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const = 0;
    virtual bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM * pmedium) const = 0;
    virtual QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const = 0;

    // for converting to Qt
    virtual bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const = 0;
    virtual QVariant convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const = 0;
    virtual QString mimeForFormat(const FORMATETC &formatetc) const = 0;

    static int registerMimeType(const QString &mime);

private:
    friend class QClipboardWatcher;
    friend class QDragManager;
    friend class QDropData;
    friend class QOleDataObject;

    static QWindowsMime *converterToMime(const QString &mimeType, IDataObject *pDataObj);
    static QStringList allMimesForFormats(IDataObject *pDataObj);
    static QWindowsMime *converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData);
    static QVector<FORMATETC> allFormatsForMime(const QMimeData *mimeData);
};

#endif
#if defined(Q_WS_MAC)

/*
  Encapsulation of conversion between MIME and Mac flavor.
  Not need on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QMacMime {
    char type;
public:
    enum QMacMimeType { MIME_DND=0x01, MIME_CLIP=0x02, MIME_QT_CONVERTOR=0x04, MIME_ALL=MIME_DND|MIME_CLIP };
    explicit QMacMime(char);
    virtual ~QMacMime();

    static void initialize();

    static QList<QMacMime*> all(uchar);
    static QMacMime *convertor(uchar, const QString &mime, QString flav);
    static QMacMime *convertor(uchar, const QString &mime, int flav);
    static QString flavorToMime(uchar, QString flav);
    static QString flavorToMime(uchar, int flav);

    virtual QString convertorName() = 0;
    virtual int countFlavors() = 0;

    virtual QString flavorUTI(int index);
    virtual int flavor(int) { Q_ASSERT(false); return 0; }

    virtual bool canConvert(const QString &mime, QString flav);
    virtual bool canConvert(const QString &, int) { Q_ASSERT(false); return false; }

    virtual QString mimeFor(QString flav);
    virtual QString mimeFor(int) { Q_ASSERT(false); return QString(); }

    virtual int flavorFor(const QString &) { Q_ASSERT(false); return 0; }
    virtual QString flavorUTIFor(const QString &mime);

    virtual QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav);
    virtual QVariant convertToMime(const QString &, QList<QByteArray>, int) { Q_ASSERT(false); return QVariant(); }

    virtual QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav);
    virtual QList<QByteArray> convertFromMime(const QString &, QVariant, int){ Q_ASSERT(false); return QList<QByteArray>(); }
};

#endif // Q_WS_MAC

QT_END_HEADER

#endif // QMIME_H
