/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTITEMMODEL_H
#define QABSTRACTITEMMODEL_H

#ifndef QT_H
#include <qobject.h>
#include <qvariant.h>
#endif

class QAbstractItemModel;

class Q_GUI_EXPORT QModelIndex
{
    friend class QAbstractItemModel;
public:
    enum Type { View, HorizontalHeader, VerticalHeader };
    inline QModelIndex() : r(-1), c(-1), d(0), t(View) {}
    inline QModelIndex(const QModelIndex &other)
        : r(other.row()), c(other.column()), d(other.data()), t(other.t) {}
    inline ~QModelIndex() { d = 0; }
    inline int row() const { return r; }
    inline int column() const { return c; }
    inline void *data() const { return d; }
    inline Type type() const { return t; }
    inline bool isValid() const { return (r >= 0) && (c >= 0); }
    inline bool operator==(const QModelIndex &other) const
    { return (other.r == r && other.c == c && other.d == d && other.t == t); }
    inline bool operator!=(const QModelIndex &other) const { return !(*this == other); }
private:
    inline QModelIndex(int row, int column, void *data, Type type)
        : r(row), c(column), d(data), t(type) {}
    int r, c;
    void *d;
    Type t;
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QModelIndex &);
#endif

class QPersistentModelIndexData;

class Q_GUI_EXPORT QPersistentModelIndex
{
public:
    QPersistentModelIndex();
    QPersistentModelIndex(const QModelIndex &index, QAbstractItemModel *model);
    QPersistentModelIndex(const QPersistentModelIndex &other);
    ~QPersistentModelIndex();
    bool operator<(const QPersistentModelIndex &other) const;
    void operator=(const QPersistentModelIndex &other);
    operator const QModelIndex&() const;
    int row() const;
    int column() const;
    void *data() const;
    QModelIndex::Type type() const;
    bool isValid() const;
    bool operator==(const QModelIndex &other) const;
    bool operator!=(const QModelIndex &other) const;
private:
    QPersistentModelIndexData *d;
};

template<typename T>
class QList;
typedef QList<QModelIndex> QModelIndexList;

class QWidget;
class QDropEvent;
class QDragObject;
class QMimeSource;
class QVariant;
class QAbstractItemModelPrivate;
template <class Key, class T>
class QMap;


class Q_GUI_EXPORT QAbstractItemModel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractItemModel)

public:
    enum Role {
        Display = 0,
        Decoration = 1,
        Edit = 2,
        ToolTip = 3,
        StatusTip = 4,
        WhatsThis = 5,
        User = 32
    };

    enum Match {
        MatchContains = 0,
        MatchFromStart = 1,
        MatchFromEnd = 2,
        MatchExactly = MatchFromStart | MatchFromEnd,
        MatchCase = 4,
        MatchWrap = 8,
        MatchDefault = MatchFromStart | MatchWrap
    };

    Q_DECLARE_FLAGS(MatchFlags, Match);

    QAbstractItemModel(QObject *parent = 0);
    virtual ~QAbstractItemModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex(),
                              QModelIndex::Type type = QModelIndex::View) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    inline QModelIndex topLeft(const QModelIndex &parent = QModelIndex()) const
        { return index(0, 0, parent); }
    inline QModelIndex bottomRight(const QModelIndex &parent = QModelIndex()) const
        { return index(rowCount(parent) - 1, columnCount(parent) - 1, parent); }
    inline QModelIndex sibling(int row, int column, const QModelIndex &idx) const
        { return index(row, column, parent(idx), idx.type()); }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
    virtual bool hasChildren(const QModelIndex &parent) const;

    virtual bool canDecode(QMimeSource *src) const;
    virtual bool decode(QDropEvent *e, const QModelIndex &parent = QModelIndex());
    virtual QDragObject *dragObject(const QModelIndexList &indices, QWidget *dragSource);

    virtual QVariant data(const QModelIndex &index, int role = Display) const = 0;
    virtual bool setData(const QModelIndex &index, int role, const QVariant &value);
    inline bool setData(const QModelIndex &index, const QVariant &value)
        { return setData(index, Edit, value); }

    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

    virtual bool insertRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);
    virtual bool insertColumns(int column, const QModelIndex &parent = QModelIndex(), int count = 1);
    virtual bool removeRows(int row, const QModelIndex &parent = QModelIndex(), int count = 1);
    virtual bool removeColumns(int column, const QModelIndex &parent = QModelIndex(), int count = 1);

    virtual bool isSelectable(const QModelIndex &index) const;
    virtual bool isEditable(const QModelIndex &index) const;
    virtual bool isDragEnabled(const QModelIndex &index) const;
    virtual bool isDropEnabled(const QModelIndex &index) const;

    virtual bool isSortable() const;
    virtual void sort(int column, Qt::SortOrder order);

    virtual bool equal(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    inline bool greaterThan(const QModelIndex &left, const QModelIndex &right) const
        { return lessThan(right, left); }

    virtual QModelIndex buddy(const QModelIndex &index) const;

    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value,
                                  int hits = 1, MatchFlags flags = MatchDefault) const;
public slots:
    virtual void fetchMore();

signals:
    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
    QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent);

    inline QModelIndex createIndex(int row = -1, int column = -1, void *data = 0,
                                   QModelIndex::Type type = QModelIndex::View) const
        { return QModelIndex(row, column, data, type); }

    void invalidatePersistentIndexes(const QModelIndex &parent = QModelIndex());
    int persistentIndexesCount() const;
    QModelIndex persistentIndexAt(int position) const;
    void setPersistentIndex(int position, const QModelIndex &index);

    friend class QPersistentModelIndexData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemModel::MatchFlags);

#endif
