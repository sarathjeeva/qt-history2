#ifndef QABSTRACTTEXTDOCUMENTLAYOUT_H
#define QABSTRACTTEXTDOCUMENTLAYOUT_H

#ifndef QT_H
#include <qobject.h>
#include <qtextlayout.h>
#include <qtextdocument.h>
#include <qtextcursor.h>
#include <qpalette.h>
#endif

class QRect;
class QRegion;
class QAbstractTextDocumentLayoutPrivate;
class QTextBlockIterator;
class QTextObjectInterface;

class Q_GUI_EXPORT QAbstractTextDocumentLayout : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractTextDocumentLayout)
    friend class QTextDocument;

public:
    struct PaintContext
    {
        PaintContext() { showCursor = false; textColorFromPalette = false; }
        QTextCursor cursor;
        QPalette palette;
        bool showCursor;
        bool textColorFromPalette;
    };

    QAbstractTextDocumentLayout();

    virtual void draw(QPainter *painter, const PaintContext &context) = 0;
    virtual int hitTest(const QPoint &point, QText::HitTestAccuracy accuracy) const = 0;

    virtual void documentChange(int from, int oldLength, int length) = 0;

    virtual int numPages() const = 0;

    void registerHandler(int objectType, QObject *component);
    QTextObjectInterface *handlerForObject(int objectType) const;

    virtual void setSize(QTextObject item, const QTextFormat &format);
    virtual void layoutObject(QTextObject item, const QTextFormat &format);
    virtual void drawObject(QPainter *painter, const QRect &rect, QTextObject object, const QTextFormat &format,
                            QTextLayout::SelectionType selection);

    virtual void setPageSize(const QSize &size) = 0;
    virtual QSize pageSize() const = 0;

protected:
    QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &);

    void invalidate(const QRect &r);
    void invalidate(const QRegion &r);

    QTextBlockIterator findBlock(int pos) const;
    QTextBlockIterator begin() const;
    QTextBlockIterator end() const;

    int formatIndex(int pos);
    QTextCharFormat format(int pos);

private slots:
    void handlerDestroyed(QObject *obj);
};

class QTextObjectInterface
{
public:
    virtual QSize intrinsicSize(const QTextFormat &format) = 0;
    virtual void drawObject(QPainter *painter, const QRect &rect, const QTextFormat &format) = 0;
};
Q_DECLARE_INTERFACE(QTextObjectInterface, "http://trolltech.com/Qt/QTextObjectInterface")

#endif
