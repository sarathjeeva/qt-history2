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

#ifndef QTOOLBARLAYOUT_P_H
#define QTOOLBARLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qlayout.h>
#include <private/qlayoutengine_p.h>
#include <QVector>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLBAR

class QAction;
class QToolBarExtension;
class QMenu;

class Q_GUI_EXPORT QToolBarItem : public QWidgetItem
{
public:
    QToolBarItem(QWidget *widget);
    bool isEmpty() const;

    QAction *action;
    bool customWidget;
};

class Q_GUI_EXPORT QToolBarLayout : public QLayout
{
    Q_OBJECT

public:
    QToolBarLayout(QWidget *parent = 0);
    ~QToolBarLayout();

    void addItem(QLayoutItem *item);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    bool isEmpty() const;
    void invalidate();
    Qt::Orientations expandingDirections() const;

    void setGeometry(const QRect &r);
    QSize minimumSize() const;
    QSize sizeHint() const;

    void insertAction(int index, QAction *action);
    int indexOf(QAction *action) const;
    int indexOf(QWidget *widget) const { return QLayout::indexOf(widget); }

    QRect handleRect() const;

    bool layoutActions(const QSize &size);
    QSize expandedSize(const QSize &size) const;
    bool expanded, animating;

    void setUsePopupMenu(bool set); // Yeah, there's no getter, but it's internal.

    bool movable() const;
    void updateMarginAndSpacing();

public slots:
    void setExpanded(bool b);

private:
    QList<QToolBarItem*> items;
    QSize hint, minSize;
    bool dirty, expanding, empty;
    QVector<QLayoutStruct> geomArray;
    QRect handRect;
    QToolBarExtension *extension;

    void updateGeomArray() const;
    QToolBarItem *createItem(QAction *action);
    QMenu *popupMenu;
};

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE

#endif // QTOOLBARLAYOUT_P_H
