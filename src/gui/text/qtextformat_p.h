#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include "qtextformat.h"
#include <private/qobject_p.h>
#include <qvector.h>
#include <qmap.h>

class QTextPieceTable;

class QTextFormatProperty
{
public:
    inline QTextFormatProperty() : type(QTextFormat::Undefined) {}

    inline QTextFormatProperty(bool value) : type(QTextFormat::Bool)
    { data.boolValue = value; }

    inline QTextFormatProperty(int value) : type(QTextFormat::Integer)
    { data.intValue = value; }

    inline QTextFormatProperty(float value) : type(QTextFormat::Float)
    { data.floatValue = value; }

    QTextFormatProperty(const QString &value);

    QTextFormatProperty &operator=(const QTextFormatProperty &rhs);
    inline QTextFormatProperty(const QTextFormatProperty &rhs) : type(QTextFormat::Undefined)
    { (*this) = rhs; }


    inline ~QTextFormatProperty()
    { free(); }

    bool operator==(const QTextFormatProperty &rhs) const;

    QTextFormat::PropertyType type;
    union {
        bool boolValue;
        int intValue;
        float floatValue;
        mutable void *ptr;
    } data;

    inline QString stringValue() const
    { return *reinterpret_cast<QString *>(&data.ptr); }

private:
    void free();
};

QDataStream &operator<<(QDataStream &stream, const QTextFormatProperty &prop);
QDataStream &operator>>(QDataStream &stream, QTextFormatProperty &prop);

class QTextFormatPrivate : public QSharedData
{
public:
    // keep Q_INT* types here, so we can safely stream to a datastream
    typedef QMap<Q_INT32, QTextFormatProperty> PropertyMap;

    PropertyMap properties;
    Q_INT32 type;

    inline bool operator==(const QTextFormatPrivate &rhs) const {
        if (type != rhs.type)
            return false;

        return properties == rhs.properties;
    }
};


class Q_GUI_EXPORT QTextFormatCollection
{
public:
    QTextFormatCollection( QTextPieceTable *_pieceTable = 0 ) : pieceTable(_pieceTable) { ref = 0; }
    ~QTextFormatCollection();

    QTextFormatCollection(const QTextFormatCollection &rhs);
    QTextFormatCollection &operator=(const QTextFormatCollection &rhs);


    QTextGroup *createGroup(const QTextFormat &newFormat);
    QTextGroup *group(int groupIndex) const;
    int indexForGroup(QTextGroup *group);

    int indexForFormat(const QTextFormat &f);
    bool hasFormatCached(const QTextFormat &format) const;

    QTextFormat format(int idx) const;

    inline QTextBlockFormat blockFormat(int index) const
    { return format(index).toBlockFormat(); }
    inline QTextCharFormat charFormat(int index) const
    { return format(index).toCharFormat(); }
    inline QTextListFormat listFormat(int index) const
    { return format(index).toListFormat(); }
    inline QTextTableFormat tableFormat(int index) const
    { return format(index).toTableFormat(); }
    inline QTextImageFormat imageFormat(int index) const
    { return format(index).toImageFormat(); }

    inline int numFormats() const { return formats.count(); }

    QTextGroup *createGroup(int index);

    mutable QAtomic ref;

    QTextPieceTable *pieceTable;
    const QVector<QTextGroup *> &formatGroups() const { return groups; }
private:

    mutable QVector<QSharedDataPointer<QTextFormatPrivate> > formats;
    QVector<QTextGroup *> groups;
};

class QTextGroupPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTextGroup);
public:
    QTextPieceTable *pieceTable() const { return collection->pieceTable; }
    QTextFormatCollection *collection;
    int index;

    typedef QList<QTextBlockIterator> BlockList;
    BlockList blocks;
};

class QTextFramePrivate : public QTextGroupPrivate
{
    Q_DECLARE_PUBLIC(QTextFrame);
public:
    int fragment_start;
    int fragment_end;

    QTextFrame *parentFrame;
    QList<QTextFrame *> childFrames;
};


#endif // QTEXTFORMAT_P_H
