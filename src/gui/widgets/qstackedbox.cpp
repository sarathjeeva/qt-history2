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

#include "qstackedbox.h"

#include <qstackedlayout.h>
#include <qevent.h>
#include <private/qframe_p.h>

#define d d_func()
#define q q_func()

class QStackedBoxPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QStackedBox)
public:
    QStackedBoxPrivate():layout(0), blockChildAdd(false){}
    QStackedLayout *layout;
    bool blockChildAdd;
};

class QBoolBlocker
{
    bool &block;
    bool reset;
public:
    inline QBoolBlocker(bool &b):block(b), reset(b){block = true;}
    inline ~QBoolBlocker(){block = reset; }
};

/*!
    \class QStackedBox qstackedbox.h
    \brief The  QStackedBox class provides a stack of widgets where only the top widget is visible.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    All the horizontal box's child widgets will be placed on top of
    each other, and only the top one will be visible to the user. This
    top widget is the currentWidget().  It can be changed by setting
    the \l currentIndex property, using setCurrentIndex(). The index
    of a given widget inside the stacked box is retrieved with
    indexOf(); widget() returns the widget at a given index position.

    If you just need a stacked layout (not a widget), use
    QStackedLayout instead.

    \sa QTabWidget
*/

/*!
    \fn void QStackedBox::currentChanged(int index)

    This signal is emitted when the current widget is changed. The
    parameter holds the \a index of the new current widget, or -1 if
    there isn't a new one (for example, if there are no widgets in the
    stacked box).
*/

/*!
    \fn void QStackedBox::widgetRemoved(int index)

    This signal is emitted when the widget at position \a index is
    removed.
*/


/*!
  Constructs a new QStackedBox as a child of \a parent.
*/

QStackedBox::QStackedBox(QWidget *parent)
    :QFrame(*new QStackedBoxPrivate, parent)
{
    d->layout = new QStackedLayout(this);
    connect(d->layout, SIGNAL(widgetRemoved(int)), this, SIGNAL(widgetRemoved(int)));
}

/*!
  Destroys the object and frees any allocated resources.
*/

QStackedBox::~QStackedBox()
{
}

/*!  Adds \a w to this box. The first widget added becomes the
  initial current widget.  Returns the index of \a w in this box.
*/
int QStackedBox::addWidget(QWidget *w)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->addWidget(w);
}

/*!  Inserts \a w to this box at position \a index. If \a index is out
  of range, the widget gets appened. The first widget added becomes
  the initial current widget.  Returns the index of \a w in this box.
*/
int QStackedBox::insertWidget(int index, QWidget *w)
{
    QBoolBlocker block(d->blockChildAdd);
    return d->layout->insertWidget(index, w);
}


/*!
    Removes widget \a w from this layout, but does not delete it.
*/
void QStackedBox::removeWidget(QWidget *w)
{
    d->layout->removeWidget(w);
}


/*!
    \property QStackedBox::currentIndex
    \brief The index position of the current widget

    The current index is -1 if there is no current widget. The widget
    at index position 0 is the one that is on top (i.e. the one that
    is visible).

    \sa currentWidget() indexOf()
*/

void QStackedBox::setCurrentIndex(int index)
{
    d->layout->setCurrentIndex(index);
    emit currentChanged(index);
}

int QStackedBox::currentIndex() const
{
    return d->layout->currentIndex();
}

/*!
  Returns the current widget, or 0 if there are no child widgets.
*/
QWidget *QStackedBox::currentWidget() const
{
    return d->layout->currentWidget();
}


/*!
  Returns the index of \a w, or -1 if \a w is not a child.
 */
int QStackedBox::indexOf(QWidget *w) const
{
    return d->layout->indexOf(w);
}

/*!
    Returns the widget at position \a index, or 0 if there is no such
    widget.
*/
QWidget *QStackedBox::widget(int index) const
{
    return d->layout->widget(index);
}

/*!
    \property QStackedBox::count
    \brief The number of widgets in this layout.
*/
int QStackedBox::count() const
{
    return d->layout->count();
}

/*!
  \reimp
*/
void QStackedBox::childEvent(QChildEvent *e)
{
    if (!e->child()->isWidgetType())
        return;
    QWidget *w = static_cast<QWidget*>(e->child());

    if (e->added() && !d->blockChildAdd && !w->isTopLevel() && d->layout->indexOf(w) < 0)
        d->layout->addWidget(w);
}
