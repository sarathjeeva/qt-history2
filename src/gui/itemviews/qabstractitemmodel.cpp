#include "qabstractitemmodel.h"
#include <qdatastream.h>
#include <qdebug.h>

/*!
  \class QModelIndex qgenericitemmodel.h

  \brief class used to access data in the data model.

  This class is used as an index into QAbstractItemModel derived data models.
  The index is used by itemviews, delegates and selection models to represent an item in the model.
  QModelIndex objects are created by the model.
*/

/*!
  \class QAbstractItemModel qgenericitemmodel.h

  \brief Abstract class used to provide abitrary data.

  This class is an abstract class which provides data.  The data model is a hierarchy of rows. The data is accessed
  by calling data() with a QModelIndex object.  Functions rowCunt() and columnCount() returns
  the size of the data source which is provided by the model on each leve in the heierarchy.

  fetchMore() can be reimplemented by models where it is hard  to determine the final size of the data (e.g. data streams).
  The model is also responsible for sorting its data through the virtual functions
  sort() and isSortable().

  To notify about changes (e.g. an item changed, new data was added, etc.), the QAbstractItemModel emits the contentsChanged,
  contentsInserted and/or contentsRemoved signals.
*/

QAbstractItemModel::QAbstractItemModel(QObject *parent)
    : QObject(parent)
{
}

QAbstractItemModel::QAbstractItemModel(QObjectPrivate &dp, QObject *parent)
    : QObject(dp, parent)
{
}

QAbstractItemModel::~QAbstractItemModel()
{
}

QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
        return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

QModelIndex QAbstractItemModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

void QAbstractItemModel::fetchMore()
{
    // do nothing
}

const char *QAbstractItemModel::format(int i) const
{
    if (i == 0)
        return "application/x-qgenericmodeldatalist";
    return 0;
}

QByteArray QAbstractItemModel::encodedData(const char *mime, const QModelIndexList &indices) const
{
    if (indices.count() <= 0 || QString(mime) != format(0))
        return QByteArray();

    QByteArray encoded;
    QDataStream stream(&encoded, IO_WriteOnly);
    QModelIndexList::ConstIterator it = indices.begin();
    for (; it != indices.end(); ++it) {
        QMap<int, QVariant> roles = itemData((*it));
        for (QMap<int, QVariant>::ConstIterator r = roles.begin(); r != roles.end(); ++r)
            stream << r.key() << r.value();
        stream << -1;
    }
    return encoded;
}

bool QAbstractItemModel::canDecode(QMimeSource *src) const
{
    return src->provides(format(0));
}

bool QAbstractItemModel::decode(QMimeSource *src)
{
    if (!canDecode(src))
        return false;

    QByteArray encoded = src->encodedData(format(0));
    QDataStream stream(&encoded, IO_ReadOnly);
    bool newItem = true;
    int role;
    QVariant variantData;
    QModelIndex insertedItem;
    while (!stream.atEnd()) {
        stream >> role;
        if (role > -1) {
            if (newItem) {
                insertedItem = insertItem();
                newItem = false;
            }
            stream >> variantData;
            setData(insertedItem, role, variantData);
        } else {
            newItem = true;
        }
    }
    return true;
}

/*!
  Returns a map with values for all predefined roles in the model. Must be reimplemented if you extend the model
  with customized roles.

  \sa Role
*/

QMap<int, QVariant> QAbstractItemModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> roles;
    for (int i=0; i<User; ++i) {
        QVariant variantData = data(index, i);
        if (variantData != QVariant::Invalid)
            roles.insert(i, variantData);
    }
    return roles;
}

void QAbstractItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    // do nothing - read only
}

void QAbstractItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
        setData(index, it.key(), it.value());
}

bool QAbstractItemModel::insertRow(const QModelIndex &parent, int row)
{
    return false;
}

bool QAbstractItemModel::insertColumn(const QModelIndex &parent, int column)
{
    return false;
}

bool QAbstractItemModel::removeRow(const QModelIndex &parent, int row)
{
    return false;
}

bool QAbstractItemModel::removeColumn(const QModelIndex &parent, int column)
{
    return false;
}

bool QAbstractItemModel::isSelectable(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isEditable(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isDragEnabled(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isDropEnabled(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isSortable() const
{
    return false;
}

void QAbstractItemModel::sort(int, SortOrder)
{
    // do nothing
}

bool QAbstractItemModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return left == right;
}

bool QAbstractItemModel::greater(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.row() == right.row())
        return left.column() > right.column();
    return left.row() > right.row();
}

QModelIndexList QAbstractItemModel::match(const QModelIndex &start, int role, const QVariant &value, int hits) const
{
    QModelIndexList result;
    QString val = value.toString();
    QModelIndex idx;
    QModelIndex par = parent(start);
    int hit = 0;
    int col = start.column();
    for (int row = start.row(); row < rowCount(par) && hit < hits; ++row) {
        idx = index(row, col, par);
        if (data(idx, role).toString().startsWith(val)) {
            result.append(idx);
            ++hit;
        }
    }
    return result;
}

#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QModelIndex &idx)
{
    dbg.nospace() << "QModelIndex(" << idx.row() << "," << idx.column() << ",";
    switch (idx.type()) {
    case QModelIndex::View:
        dbg.nospace() << "View";
        break;
    case QModelIndex::HorizontalHeader:
        dbg.nospace() << "HorizontalHeader";
        break;
    case QModelIndex::VerticalHeader:
        dbg.nospace() << "VerticalHeader";
        break;
    }
    dbg.nospace() << ")";
    return dbg.space();
}
#endif
#endif

