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

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#include <qobject.h>
#include <qstyleoption.h>

class QPainter;
class QModelIndex;
class QAbstractItemModel;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_OBJECT

public:
    QAbstractItemDelegate(QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    // painting
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QAbstractItemModel *model, const QModelIndex &index) const = 0;

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QAbstractItemModel *model,
                           const QModelIndex &index) const = 0;

    // editing
    virtual QWidget *editor(QWidget *parent,
                            const QStyleOptionViewItem &option,
                            const QAbstractItemModel *model,
                            const QModelIndex &index);

    virtual void releaseEditor(QWidget *editor);

    virtual void setEditorData(QWidget *editor,
                               const QAbstractItemModel *model,
                               const QModelIndex &index) const;

    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QAbstractItemModel* model,
                                      const QModelIndex &index) const;

    // for non-widget editors
    virtual bool editorEvent(QEvent *e, const QStyleOptionViewItem &option,
                             QAbstractItemModel* model, const QModelIndex &index);

signals:
    void commitData(QWidget *editor);
    void doneEditing(QWidget *editor);
    void editingAccepted();
    void editingAborted();
    void editNextItem();
    void editPreviousItem();

protected:
    QAbstractItemDelegate(QObjectPrivate &, QObject *parent = 0);
    QString ellipsisText(const QFontMetrics &fontMetrics, int width, int align,
                         const QString &org) const;
};

#endif
