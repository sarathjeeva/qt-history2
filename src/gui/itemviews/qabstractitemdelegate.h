/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#include <QtCore/qobject.h>
#include <QtGui/qstyleoption.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_ITEMVIEWS

class QPainter;
class QModelIndex;
class QAbstractItemModel;
class QAbstractItemView;
class QHelpEvent;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
    Q_OBJECT

public:

    enum EndEditHint {
        NoHint,
        EditNextItem,
        EditPreviousItem,
        SubmitModelCache,
        RevertModelCache
    };

    explicit QAbstractItemDelegate(QObject *parent = 0);
    virtual ~QAbstractItemDelegate();

    // painting
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const = 0;

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const = 0;

    // editing
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;

    // for non-widget editors
    virtual bool editorEvent(QEvent *event,
                             QAbstractItemModel *model,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index);

    static QString elidedText(const QFontMetrics &fontMetrics, int width,
                              Qt::TextElideMode mode, const QString &text);

public Q_SLOTS:
    bool helpEvent(QHelpEvent *event,
                   QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index);

Q_SIGNALS:
    void commitData(QWidget *editor);
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint = NoHint);

protected:
    QAbstractItemDelegate(QObjectPrivate &, QObject *parent = 0);
private:
    Q_DISABLE_COPY(QAbstractItemDelegate)
};

#endif // QT_NO_ITEMVIEWS

QT_END_NAMESPACE

QT_END_HEADER

#endif // QABSTRACTITEMDELEGATE_H
