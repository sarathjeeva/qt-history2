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

#include <qwidget.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qpoint.h>
#include "qdnd_p.h"


/*!
    \class QDrag
    \brief The QDrag class provides support for MIME-based drag and drop data
    transfer.

    Drag and drop is an intuitive way for users to copy or move data around in an
    application, and is used in many desktop environments as a mechanism for copying
    data between applications. Drag and drop support in Qt is centered around the
    QDrag class that handles most of the details of a drag and drop operation.

    The data to be transferred by the drag and drop operation is contained in a
    QMimeData object. This is specified with the setMimeData() function in the
    following way:

    \quotefromfile snippets/dragging/mainwindow.cpp
    \skipto mouseMoveEvent
    \skipto QDrag
    \printuntil setMimeData

    Note that setMimeData() assigns ownership of the QMimeData object to the
    QDrag object. The QDrag must be constructed on the heap and given a parent
    QObject to ensure that Qt can perform the necessary memory management
    operations on the objects the next time the event loop is entered.

    A pixmap can be used to represent the data while the drag is in progress, and
    will move with the cursor to the drop target. This pixmap typically shows an
    icon that represents the MIME type of the data being transferred, but any
    pixmap can be set with setPixmap(). Care must be taken to ensure that the
    pixmap is not too large. The cursor's hot spot can be given a position relative
    to the top-left corner of the pixmap with the setHotSpot() function. The
    following code positions the pixmap so that the cursor's hot spot points to
    the center of its bottom edge:

    \quotefromfile snippets/separations/finalwidget.cpp
    \skipto setHotSpot
    \printuntil setHotSpot

    The source and target widgets can be found with source() and target().
    These functions are often used to determine whether drag and drop operations
    started and finished at the same widget, so that special behavior can be
    implemented.

    QDrag only deals with the drag and drop operation itself. It is up to the
    developer to decide when a drag operation begins, and how a QDrag object should
    be constructed and used. For a given widget, it is often necessary to
    reimplement \l{QWidget::mousePressEvent()}{mousePressEvent()} to determine
    whether the user has pressed a mouse button, and reimplement
    \l{QWidget::mouseMoveEvent()}{mouseMoveEvent()} to check whether a QDrag is
    required.

    \sa \link dnd.html Drag and Drop\endlink QClipboard QMimeData
*/

/*!
    \enum QDrag::DropAction

    \value CopyAction       Copy the data to the target.
    \value MoveAction       Move the date from the source to the target.
    \value LinkAction       Create a link from the source to the target.
    \value ActionMask       ###
    \value TargetMoveAction ###
    \value IgnoreAction     Ignore the action (do nothing with the data).
*/

/*!
    Constructs a new drag object for the widget specified by \a dragSource.
*/
QDrag::QDrag(QWidget *dragSource)
    : QObject(*new QDragPrivate, dragSource)
{
    Q_D(QDrag);
    d->source = dragSource;
    d->target = 0;
    d->data = 0;
    d->hotspot = QPoint(-10, -10);
    d->possible_actions = QDrag::CopyAction;
    d->executed_action = QDrag::IgnoreAction;
}

/*!
    Destroys the drag object.
*/
QDrag::~QDrag()
{
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object == this)
        manager->cancel(false);
}

/*!
    Sets the data to be sent to the given MIME \a data.
*/
void QDrag::setMimeData(QMimeData *data)
{
    Q_D(QDrag);
    d->data = data;
}

/*!
    Returns the MIME data that is encapsulated by the drag object.
*/
QMimeData *QDrag::mimeData() const
{
    Q_D(const QDrag);
    return d->data;
}

/*!
    Sets \a pixmap as the pixmap used to represent the data in a drag
    and drop operation.
*/
void QDrag::setPixmap(const QPixmap &pixmap)
{
    Q_D(QDrag);
    d->pixmap = pixmap;
}

/*!
    Returns the pixmap used to represent the data in a drag and drop operation.
*/
QPixmap QDrag::pixmap() const
{
    Q_D(const QDrag);
    return d->pixmap;
}

/*!
    Sets the position of the hot spot relative to the top-left corner of the
    pixmap used to the point specified by \a hotspot.
*/
void QDrag::setHotSpot(const QPoint& hotspot)
{
    Q_D(QDrag);
    d->hotspot = hotspot;
}

/*!
    Returns the position of the hot spot relative to the top-left corner of the
    cursor.
*/
QPoint QDrag::hotSpot() const
{
    Q_D(const QDrag);
    return d->hotspot;
};

/*!
    Returns the source of the drag object. This is the widget where the drag
    and drop operation originated.
*/
QWidget *QDrag::source() const
{
    Q_D(const QDrag);
    return d->source;
}

/*!
    Returns the target of the drag and drop operation. This is the widget where
    the drag object was dropped.
*/
QWidget *QDrag::target() const
{
    Q_D(const QDrag);
    return d->target;
}

/*!
    Starts the drag and drop operation. The actions that are available to the
    user when the drag and drop operation is completed are specified in
    \a request.
*/
QDrag::DropAction QDrag::start(QDrag::DropActions request)
{
    Q_D(QDrag);
    QDragManager *manager = QDragManager::self();
    d->possible_actions = request;
    if (manager)
        d->executed_action = manager->drag(this);
    return d->executed_action;
}
