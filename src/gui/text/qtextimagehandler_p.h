#ifndef QTEXTIMAGEHANDLER_P_H
#define QTEXTIMAGEHANDLER_P_H

#ifndef QT_H
#include <qobject.h>
#include <qabstracttextdocumentlayout.h>

#include "qtextdocument_p.h"
#endif // QT_H

class QTextImageFormat;

class QTextImageHandler : public QObject,
                          public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    QTextImageHandler(QObject *parent = 0);

    virtual QSize intrinsicSize(const QTextFormat &format);
    virtual void drawObject(QPainter *p, const QRect &rect, const QTextFormat &format);

    typedef QImage (*ExternalImageLoaderFunction)(const QString &name, const QString &context);
    static ExternalImageLoaderFunction externalLoader;
};

#endif // QTEXTIMAGEHANDLER_P_H
