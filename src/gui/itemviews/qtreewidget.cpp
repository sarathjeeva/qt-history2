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

#include "qtreewidget.h"
#include <private/qtreeview_p.h>

class QTreeModel : public QAbstractItemModel
{
    friend class QTreeWidget;
    friend class QTreeWidgetItem;

public:
    QTreeModel(int columns = 0, QObject *parent = 0);
    ~QTreeModel();

    virtual void setColumnCount(int columns);
    int columnCount() const;

    virtual void setColumnText(int column, const QString &text);
    virtual void setColumnIcon(int column, const QIconSet &icon);
    virtual void setColumnData(int column, int role, const QVariant &value);
    
    QString columnText(int column) const;
    QIconSet columnIcon(int column) const;
    QVariant columnData(int column, int role) const;

    QTreeWidgetItem *item(const QModelIndex &index) const;

    QModelIndex index(QTreeWidgetItem *item) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex::Null,
                      QModelIndex::Type type = QModelIndex::View) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool isSelectable(const QModelIndex &index) const;
    bool isEditable(const QModelIndex &index) const;

protected:
    void append(QTreeWidgetItem *item);
    void emitRowsInserted(QTreeWidgetItem *item);

private:
    int c;
    QList<QTreeWidgetItem*> tree;
    mutable QTreeWidgetItem topHeader;
};

/*
  \class QTreeModel qtreewidget.h

  \brief The QTreeModel class manages the items stored in a tree view.

  \ingroup model-view
    \mainclass
*/

/*!
  \internal

  Constructs a tree model with a \a parent object and the given
  number of \a columns.
*/

QTreeModel::QTreeModel(int columns, QObject *parent)
    : QAbstractItemModel(parent), c(0)
{
    setColumnCount(columns);
}

/*!
  \internal

  Destroys this tree model.
*/

QTreeModel::~QTreeModel()
{
    for (int i = 0; i < tree.count(); ++i)
        delete tree.at(i);
}

/*!
  \internal

  Sets the number of \a columns in the tree model.
*/

void QTreeModel::setColumnCount(int columns)
{
    if (c == columns)
        return;
    int _c = c;
    c = columns;
    if (c < _c)
        emit columnsRemoved(QModelIndex::Null, qMax(_c - 1, 0), qMax(c - 1, 0));
    topHeader.setColumnCount(c);
    for (int i = _c; i < c; ++i)
        topHeader.setText(i, QString::number(i));
    if (c > _c)
        emit columnsInserted(QModelIndex::Null, qMax(_c - 1, 0), qMax(c - 1, 0));
}

/*!
  \internal

  Returns the number of columns in the tree model.
*/

int QTreeModel::columnCount() const
{
    return c;
}

/*!
  \internal

  Sets the column text for the \a column to the given \a text.
*/

void QTreeModel::setColumnText(int column, const QString &text)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::DisplayRole, text);
}

/*!
  \internal

  Sets the icon set for the \a column to the icon set specified by
  \a icon.
*/

void QTreeModel::setColumnIcon(int column, const QIconSet &icon)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, QAbstractItemModel::DecorationRole, icon);
}

/*!
  \internal

  Sets the value for the \a column and \a role to the value specified by \a value.
*/

void QTreeModel::setColumnData(int column, int role, const QVariant &value)
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    setData(index, role, value);
}

/*!
  \internal

  Returns the text for the given \a column in the tree model.
*/

QString QTreeModel::columnText(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::DisplayRole).toString();
}

/*!
  \internal

  Returns the icon set for the given \a column.
*/

QIconSet QTreeModel::columnIcon(int column) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, QAbstractItemModel::DecorationRole).toIcon();
}

/*!
  \internal

  Returns the value set for the given \a column and \a role.
*/

QVariant QTreeModel::columnData(int column, int role) const
{
    QModelIndex index = createIndex(0, column, 0, QModelIndex::HorizontalHeader);
    return data(index, role);
}

/*!
  \internal

  Returns the tree view item corresponding to the \a index given.

  \sa QModelIndex
*/

QTreeWidgetItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.type() != QModelIndex::View)
        return &topHeader;
    return static_cast<QTreeWidgetItem *>(index.data());
}

/*!
  \internal

  Returns the model index that refers to the tree view \a item.
*/

QModelIndex QTreeModel::index(QTreeWidgetItem *item) const
{
    if (!item)
        return QModelIndex::Null;
    const QTreeWidgetItem *par = item->parent();
    int row = par ? par->children.indexOf(item) : tree.indexOf(item);
    return createIndex(row, 0, item);
}

/*!
  \internal

  Returns the model index with the given \a row, \a column, \a type,
  and \a parent.
*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent,
                              QModelIndex::Type type) const
{
    int r = tree.count();
    if (row < 0 || row >= r || column < 0 || column >= c)
        return QModelIndex::Null;
    if (!parent.isValid()) {// toplevel
        QTreeWidgetItem *itm = const_cast<QTreeModel*>(this)->tree.at(row);
        if (itm)
            return createIndex(row, column, itm, type);
        return QModelIndex::Null;
    }
    QTreeWidgetItem *parentItem = item(parent);
    if (parentItem && row < parentItem->childCount()) {
        QTreeWidgetItem *itm = static_cast<QTreeWidgetItem *>(parentItem->child(row));
        if (itm)
            return createIndex(row, column, itm, type);
        return QModelIndex::Null;
    }
    return QModelIndex::Null;
}

/*!
  \internal

  Returns the parent model index of the index given as the \a child.
*/

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex::Null;
    const QTreeWidgetItem *itm = reinterpret_cast<const QTreeWidgetItem *>(child.data());
    if (!itm)
        return QModelIndex::Null;
    QTreeWidgetItem *parent = const_cast<QTreeWidgetItem *>(itm->parent()); // FIXME
    return index(parent);
}

/*!
  \internal

  Returns the number of rows in the \a parent model index.
*/

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        QTreeWidgetItem *parentItem = item(parent);
        if (parentItem)
            return parentItem->childCount();
    }
    return tree.count();
}

/*!
  \internal

  Returns the number of columns in the item referred to by the given
  \a index.
*/

int QTreeModel::columnCount(const QModelIndex &) const
{
    return c;
}

/*!
  \internal

  Returns the data corresponding to the given model \a index and
  \a role.
*/

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    QTreeWidgetItem *itm = item(index);
    if (itm)
        return itm->data(index.column(), role);
    return QVariant();
}

/*!
  \internal

  Sets the data for the item specified by the \a index and \a role
  to that referred to by the \a value.

  Returns true if successful; otherwise returns false.
*/

bool QTreeModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid())
        return false;
    QTreeWidgetItem *itm = item(index);
    if (itm) {
        itm->setData(index.column(), role, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
  \internal

  Inserts a tree view item into the \a parent item at the given
  \a row. Returns true if successful; otherwise returns false.

  If no valid parent is given, the item will be inserted into this
  tree model at the row given.
*/

bool QTreeModel::insertRows(int row, const QModelIndex &parent, int)
{
    if (parent.isValid()) {
        QTreeWidgetItem *p =  item(parent);
        if (p) {
            p->children.insert(row, new QTreeWidgetItem(p));
            return true;
        }
        return false;
    }
    tree.insert(row, new QTreeWidgetItem());
    return true;
}

/*!
  \internal

  Removes the given \a row from the \a parent item, and returns true
  if successful; otherwise false is returned.
*/

bool QTreeModel::removeRows(int row, const QModelIndex &parent, int)
{
    if (parent.isValid()) {
        QTreeWidgetItem *p = item(parent);
        if (p) {
            p->children.removeAt(row);
            return true;
        }
        return false;
    }
    tree.removeAt(row);
    return true;
}

/*!
  \internal

  Returns true if the item at the \a index given is selectable;
  otherwise returns false.
*/

bool QTreeModel::isSelectable(const QModelIndex &) const
{
    return true;
}

/*!
  \internal

  Returns true if the item at the \a index given is editable;
  otherwise returns false.
*/

bool QTreeModel::isEditable(const QModelIndex &) const
{
    return true;
}

/*!
  \internal

  Appends the tree view \a item to the tree model.*/

void QTreeModel::append(QTreeWidgetItem *item)
{
    int r = tree.count();
    tree.push_back(item);
    emit rowsInserted(QModelIndex::Null, r, r);
}

/*!
\internal

Emits the rowsInserted() signal for the rows containing the given \a item.

\sa rowsInserted()*/

void QTreeModel::emitRowsInserted(QTreeWidgetItem *item)
{
    QModelIndex idx = index(item);
    QModelIndex parentIndex = parent(idx);
    emit rowsInserted(parentIndex, idx.row(), idx.row());
}

// QTreeWidgetItem

/*!
  \class QTreeWidgetItem qtreewidget.h

  \brief The QTreeWidgetItem class provides an item for use with the
  predefined QTreeWidget class.

  \ingroup model-view

  The QTreeWidgetItem class provides a familiar interface for items displayed
  in a QTreeWidget widget.

  \sa QTreeWidget QTreeModel
*/

/*!
    \fn const QTreeWidgetItem *QTreeWidgetItem::parent() const

    Returns this tree widget item's parent (which is 0 if this item is
    a top-level item).

    \sa childCount() child()
*/


/*!
    \fn const QTreeWidgetItem *QTreeWidgetItem::child(int index) const

    Returns this tree widget item's \a{index}-th child, or 0 if there
    is no such child.

    \sa childCount() parent()
*/


/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::child(int index)

    \overload
*/


/*!
    \fn int QTreeWidgetItem::childCount() const

    Returns the number of children this tree widget item has; it may
    be 0.

    \sa parent() child()
*/


/*!
    \fn int QTreeWidgetItem::columnCount() const

    Returns the number of columns that this item occupies.

    \sa text() icon()
*/


/*!
    \fn QString QTreeWidgetItem::text(int column) const

    Returns the text from the given \a column.

    \sa setText() icon() columnCount()
*/


/*!
    \fn QIconSet QTreeWidgetItem::icon(int column) const

    Returns the iconset from the given \a column.

    \sa setIcon() text() columnCount()
*/


/*!
    \fn bool QTreeWidgetItem::isEditable() const

    Returns true if this tree widget item is editable; otherwise
    returns false.

    \sa setEditable()
*/


/*!
    \fn bool QTreeWidgetItem::isSelectable() const

    Returns true if this tree widget item is selectable; otherwise
    returns false.

    \sa setSelectable()
*/


/*!
    \fn void QTreeWidgetItem::setText(int column, const QString &text)

    Sets the given \a column to hold the given \a text.

    \sa text() setIcon()
*/


/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIconSet &icon)

    Sets the given \a column to hold the given \a icon.

    \sa icon() setText()
*/


/*!
    \fn void QTreeWidgetItem::setEditable(bool editable)

    If \a editable is true, sets this tree widget item to be editable;
    otherwise sets it to be read-only.

    \sa isEditable()
*/


/*!
    \fn void QTreeWidgetItem::setSelectable(bool selectable)

    If \a selectable is true, sets this tree widget item to be
    selectable; otherwise sets it to be impossible for the user to
    select.

    \sa isSelectable()
*/


/*!
    \fn bool QTreeWidgetItem::operator==(const QTreeWidgetItem &other) const

    Returns true if this tree widget item is the same as the \a other
    tree widget item, i.e. has the same text and iconset in every
    column; otherwise returns false.
*/


/*!
    \fn bool QTreeWidgetItem::operator!=(const QTreeWidgetItem &other) const

    Returns true if this tree widget item has at least one column
    where its text or iconset is different from the \a other tree
    widget item; otherwise returns false.
*/



/*!
  Constructs a tree widget item. The item must be inserted
  into a tree view.

  \sa QTreeModel::append() QTreeWidget::append()
*/

QTreeWidgetItem::QTreeWidgetItem()
    : par(0), view(0), columns(0), editable(true), selectable(true)
{
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view)

    Constructs a tree widget item and inserts it into the given tree
    \a view.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *v)
    : par(0), view(v), columns(0), editable(true), selectable(true)
{
    if (view)
        view->append(this);
}

/*!
    Constructs a tree widget item with the given \a parent.
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent)
    : par(parent), view(parent->view), columns(0), editable(true), selectable(true)
{
    if (parent)
        parent->children.push_back(this);
    QTreeModel *model = ::qt_cast<QTreeModel*>(view->model());
    model->emitRowsInserted(this);
}

/*!
  Destroys this tree widget item.
*/

QTreeWidgetItem::~QTreeWidgetItem()
{
    for (int i = 0; i < children.count(); ++i)
        delete children.at(i);
}

/*!
    Sets the number of columns in the tree widget item to \a count.
*/

void QTreeWidgetItem::setColumnCount(int count)
{
    columns = count;
    values.resize(count);
}

/*!
    Returns the text stored in the \a column.

  \sa data() QAbstractItemModel::Role
*/

QString QTreeWidgetItem::text(int column) const
{
    const QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i)
        if (column_values.at(i).role == QAbstractItemModel::DisplayRole)
            return column_values.at(i).value.toString();
    return QString::null;
}

/*!
    Returns the icon stored in the \a column.

  \sa data() QAbstractItemModel::Role
*/

QIconSet QTreeWidgetItem::icon(int column) const
{
    const QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i)
        if (column_values.at(i).role == QAbstractItemModel::DecorationRole)
            return column_values.at(i).value.toIcon();
    return QIconSet();
}

/*!
    Sets the text for the item specified by the \a column to the given \a text.

    \sa text() setIcon()
*/

void QTreeWidgetItem::setText(int column, const QString &text)
{
    if (column >= columns)
        setColumnCount(column + 1);
    QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i)
        if (column_values.at(i).role == QAbstractItemModel::DisplayRole) {
            values[column][i].value = text;
            return;
        }
    values[column].append(Data(QAbstractItemModel::DisplayRole, text));
}

/*!
    Sets the icon for the item specified by the \a column to the given \a icon.

    \sa icon() setText()
*/

void QTreeWidgetItem::setIcon(int column, const QIconSet &icon)
{
    if (column >= columns)
        setColumnCount(column + 1);
    QVector<Data> column_values = values.at(column);
    for (int i = 0; i < column_values.count(); ++i)
        if (column_values.at(i).role == QAbstractItemModel::DecorationRole) {
            values[column][i].value = icon;
            return;
        }
    values[column].append(Data(QAbstractItemModel::DecorationRole, icon));
}


/*!
    Returns the data stored in the \a column with the given \a role.

  \sa QAbstractItemModel::Role
*/

QVariant QTreeWidgetItem::data(int column, int role) const
{
    if (column < 0 || column >= columns)
        return QVariant();
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    switch (role) {
    case QAbstractItemModel::DisplayRole:
        return text(column);
    case QAbstractItemModel::DecorationRole:
        return icon(column);
    }
    return QVariant();
}

/*!
    Sets the data for the item specified by the \a column and \a role
    to the given \a value.
*/

void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
    role = (role == QAbstractItemModel::EditRole ? QAbstractItemModel::DisplayRole : role);
    switch (role) {
    case QAbstractItemModel::DisplayRole:
        setText(column, value.toString());
        break;
    case QAbstractItemModel::DecorationRole:
        setIcon(column, value.toIconSet());
        break;
    }
}

class QTreeWidgetPrivate : public QTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeWidget)
public:
    QTreeWidgetPrivate() : QTreeViewPrivate() {}
    inline QTreeModel *model() const { return ::qt_cast<QTreeModel*>(q_func()->model()); }
};

#define d d_func()
#define q q_func()

/*!
  \class QTreeWidget qtreewidget.h

  \brief The QTreeWidget class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view

  The QTreeWidget class is a convenience class that replaces the \c QListView
  class. It provides a list view widget that takes advantage of Qt's
  model-view architecture.

  This class uses a default model to organize the data represented in the
  tree view, but also uses the QTreeWidgetItem class to provide a familiar
  interface for simple list structures.

  \omit
  In its simplest form, a tree view can be constructed and populated in
  the familiar way:

  \code
    QTreeWidget *view = new QTreeWidget(parent);

  \endcode
  \endomit

  \sa \link model-view-programming.html Model/View Programming\endlink QTreeModel QTreeWidgetItem
*/

/*!
  Constructs a tree view with the given \a parent widget, using the default
  model
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeViewPrivate(), parent)
{
    setModel(new QTreeModel(0, this));
}

/*!
  Retuns the number of header columns in the view.
*/

int QTreeWidget::columnCount() const
{
    return d->model()->columnCount();
}

/*!
  Sets the number of header \a columns in the tree view.
*/

void QTreeWidget::setColumnCount(int columns)
{
    d->model()->setColumnCount(columns);
}


/*!
  Returns the text for the given header \a column in the tree view.
*/

QString QTreeWidget::columnText(int column) const
{
    return d->model()->columnText(column);
}

/*!
  Sets the text for the header \a column to the \a text given.
*/

void QTreeWidget::setColumnText(int column, const QString &text)
{
    d->model()->setColumnText(column, text);
}

/*!
  Returns the icon set for the given header \a column in the tree view.
*/

QIconSet QTreeWidget::columnIcon(int column) const
{
    return d->model()->columnIcon(column);
}

/*!
  Sets the icon set for the header \a column to that specified by \a icon.
*/

void QTreeWidget::setColumnIcon(int column, const QIconSet &icon)
{
    d->model()->setColumnIcon(column, icon);
}

/*!
  Returns the value set for the given header \a column and \a role in the tree view.
*/

QVariant QTreeWidget::columnData(int column, int role) const
{
    return d->model()->columnData(column, role);
}

/*!
  Sets the value for the given header \a column and \a role to that specified by \a value.
*/

void QTreeWidget::setColumnData(int column, int role, const QVariant &value)
{
    d->model()->setColumnData(column, role, value);
}
    
/*!
  Appends a tree view \a item to the tree view.
*/

void QTreeWidget::append(QTreeWidgetItem *item)
{
    d->model()->append(item);
}
