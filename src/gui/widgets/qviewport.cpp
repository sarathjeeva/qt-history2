/****************************************************************************
**
** Implementation of QViewport widget class.
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

#include "qviewport.h"
#include "qscrollbar.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qevent.h"

#include "qviewport_p.h"
#include <qwidget.h>
#define d d_func()
#define q q_func()



/*!
    \class QViewport qviewport.h

    \brief The QViewport widget provides a scrolling area with
    on-demand scroll bars.

    \ingroup abstractwidgets

    QViewport is a low-level abstraction of a scrolling area. It gives
    you full control of the scroll bars, at the cost of simplicity. In
    most cases, using a QWidgetView is preferable.

    QViewport's central child widget is the scrolling area itself,
    called viewport(). The viewport widget uses all available
    space. Next to the viewport is a vertical scroll bar (accessible
    with verticalScrollBar()), and below a horizontal scroll bar
    (accessible with horizontalScrollBar()). Each scroll bar can be
    either visible or hidden, depending on the scroll bar's policy
    (see \l verticalScrollBarPolicy and \l horizontalScrollBarPolicy).
    When a scroll bar is hidden, the viewport expands in order to
    cover all available space. When a scroll bar becomes visible
    again, the viewport shrinks in order to make room for the scroll
    bar.

    With a scroll bar policy of Qt::ScrollBarAsNeeded (the default),
    QViewport shows scroll bars when those provide a non-zero
    scrolling range, and hides them otherwise. You control the range
    of each scroll bar with QAbstractSlider::setRange().

    In order to track scroll bar movements, reimplement the virtual
    function scrollContentsBy(). In order to fine-tune scrolling
    behavior, connect to a scroll bar's
    QAbstractSlider::actionTriggered() signal and adjust the \l
    QAbstractSlider::sliderPosition as you wish.

    It is possible to reserve a margin area around the viewport, see
    setViewportMargins(). The feature is mostly used to place a
    QHeader widget above or aside the scrolling area.

    For convience, QViewport makes all viewport events available in
    the virtual viewportEvent()-handler.  QWidget's specialised
    handlers are remapped to viewport events in the cases where this
    makes sense. The remapped specialised handlers are: paintEvent(),
    mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),
    mouseMoveEvent(), wheelEvent(), dragEnterEvent(), dragMoveEvent(),
    dragLeaveEvent(), dropEvent(), contextMenuEvent().  and
    resizeEvent().

*/


/*!
    \enum Qt::ScrollBarPolicy

    This enum type describes the various modes of QViewport's scroll
    bars.

    \value ScrollBarAsNeeded QViewport shows a scroll bar when the
    content is too large to fit and not otherwise. This is the
    default.

    \value ScrollBarAlwaysOff QViewport never shows a scroll bar.

    \value ScrollBarAlwaysOn  QViewport always shows a scroll bar.

    (The modes for the horizontal and vertical scroll bars are
    independent.)
*/

inline  bool QViewportPrivate::viewportEvent(QEvent *e) { return q->viewportEvent(e); }


class QViewportHelper : public QWidget
{
public:
    QViewportHelper(QWidget *parent):QWidget(parent){}
    bool event(QEvent *e);
    friend class QViewport;
};
bool QViewportHelper::event(QEvent *e) {
    if (QViewport* viewport = qt_cast<QViewport*>(parentWidget()))
        return ((QViewportPrivate*)((QViewportHelper*)viewport)->d_ptr)->viewportEvent(e);
    return QWidget::event(e);
}

QViewportPrivate::QViewportPrivate()
    :hbar(0), vbar(0), vbarpolicy(ScrollBarAsNeeded), hbarpolicy(ScrollBarAsNeeded),
     viewport(0), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0)
{
}


void QViewportPrivate::init()
{
    q->setFocusPolicy(QWidget::WheelFocus);
    q->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbar = new QScrollBar(Horizontal,  q);
    QObject::connect(hbar, SIGNAL(valueChanged(int)), q, SLOT(hslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), QueuedConnection);
    vbar = new QScrollBar(Vertical, q);
    QObject::connect(vbar, SIGNAL(valueChanged(int)), q, SLOT(vslide(int)));
    QObject::connect(hbar, SIGNAL(rangeChanged(int,int)), q, SLOT(showOrHideScrollBars()), QueuedConnection);
    viewport = new QViewportHelper(q);
    viewport->setBackgroundRole(QPalette::Base);
    QApplication::sendEvent(viewport, new QEvent(QEvent::User));
}

void QViewportPrivate::layoutChildren()
{
    bool needh = (hbarpolicy == ScrollBarAlwaysOn
                  || (hbarpolicy == ScrollBarAsNeeded && hbar->minimum() < hbar->maximum()));

    bool needv = (vbarpolicy == ScrollBarAlwaysOn
                  || (vbarpolicy == ScrollBarAsNeeded && vbar->minimum() < vbar->maximum()));

    int hsbExt = hbar->sizeHint().height();
    int vsbExt = vbar->sizeHint().width();

    bool reverse = QApplication::reverseLayout();
    reverse = true;
    QRect vr = q->rect();
    if (q->style().styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents)) {
        QRect fr = vr;
        if (needh) {
            fr.setBottom(fr.bottom() - hsbExt);
            hbar->setGeometry(QStyle::visualRect(QRect(0, fr.bottom() + 1, fr.width() - (needv?vsbExt:0), hsbExt), q));
        }
        if (needv) {
            fr.setRight(fr.right() - vsbExt);
            vbar->setGeometry(QStyle::visualRect(QRect(fr.right() + 1, 0, vsbExt, fr.height()), q));
        }
        q->setFrameRect(QStyle::visualRect(fr, q));
        vr = q->contentsRect();
    } else {
        q->setFrameRect(vr);
        vr = q->contentsRect();
        if (needh) {
            vr.setBottom(vr.bottom() - hsbExt);
            hbar->setGeometry(QStyle::visualRect(QRect(vr.left(), vr.bottom() + 1, vr.width() - (needv?vsbExt:0), hsbExt), q));
        }
        if (needv) {
            vr.setRight(vr.right() - vsbExt);
            vbar->setGeometry(QStyle::visualRect(QRect(vr.right() + 1, vr.top(), vsbExt, vr.height()), q));
        }
        vr = QStyle::visualRect(vr, q);
    }
    hbar->setShown(needh);
    vbar->setShown(needv);
    vr.addCoords(left, top, -right, -bottom);
    viewport->setGeometry(vr); // resize the viewport last
}

QViewport::QViewport(QViewportPrivate &dd, QWidget *parent)
    :QFrame(dd, parent)
{
    d->init();
}

/*!
    Constructs a viewport.

    The \a parent arguments is sent to the QWidget constructor.
*/
QViewport::QViewport(QWidget *parent)
    :QFrame(*new QViewportPrivate, parent)
{
    d->init();
}


/*!
  Destroys the viewport.
 */
QViewport::~QViewport()
{
}

/*! Returns the viewport's viewport
 */
QWidget *QViewport::viewport() const
{
    return d->viewport;
}


/*!
... still thinking about the name ###

Returns the size of the viewport as if the scroll bars had no valid
scrolling range.
*/
QSize QViewport::maximumViewportSize() const
{
    int hsbExt = d->hbar->sizeHint().height();
    int vsbExt = d->vbar->sizeHint().width();

    int f = 2 * d->frameWidth;
    QSize max = size() - QSize(f,f);
    if (d->vbarpolicy == ScrollBarAlwaysOn)
        max.rwidth() -= vsbExt;
    if (d->hbarpolicy == ScrollBarAlwaysOn)
        max.rheight() -= hsbExt;
    return max;
}


/*!
    \property QViewport::verticalScrollBarPolicy
    \brief the policy for the vertical scroll bar

    The default policy is \c Qt::ScrollBarAsNeeded

    \sa horizontalScrollBarPolicy
*/

Qt::ScrollBarPolicy QViewport::verticalScrollBarPolicy() const
{
    return d->vbarpolicy;
}

void QViewport::setVerticalScrollBarPolicy(ScrollBarPolicy policy)
{
    d->vbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}


/*!
  Returns the vertical scroll bar.

  \sa verticalScrollBarPolicy,  verticalScrollBar()
 */
QScrollBar *QViewport::verticalScrollBar() const
{
    return d->vbar;
}

/*!
    \property QViewport::horizontalScrollBarPolicy
    \brief the policy for the horizontal scroll bar

    The default policy is \c Qt::ScrollBarAsNeeded

    \sa verticalScrollBarPolicy
*/

Qt::ScrollBarPolicy QViewport::horizontalScrollBarPolicy() const
{
    return d->hbarpolicy;
}

void QViewport::setHorizontalScrollBarPolicy(ScrollBarPolicy policy)
{
    d->hbarpolicy = policy;
    if (isVisible())
        d->layoutChildren();
}

/*!
  Returns the horizontal scroll bar.

  \sa horizontalScrollBarPolicy,  verticalScrollBar()
 */
QScrollBar *QViewport::horizontalScrollBar() const
{
    return d->hbar;
}

/*!
    Sets the margins around the scrolling area to \a left, \a top, \a
    right and \a bottom. This is useful for applications such as
    spreadsheets with "locked" rows and columns. The marginal space is
    is left blank; put widgets in the unused area.

    By default all margins are zero.

*/
void QViewport::setViewportMargins(int left, int top, int right, int bottom)
{
    d->left = left;
    d->top = top;
    d->right = right;
    d->bottom = bottom;
    d->layoutChildren();
}

/*!  The main event handler for the QViewport widget (\e not the
  scrolling area viewport()).
*/
bool QViewport::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
            d->layoutChildren();
            break;
    case QEvent::Paint:
        QFrame::paintEvent((QPaintEvent*)e);
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
    case QEvent::ContextMenu:
        return false;
    case QEvent::StyleChange:
        d->layoutChildren();
        // fall through
    default:
        return QFrame::event(e);
    }
    return true;
}

/*!  The main event handler for the scrolling area (the viewport()
  widget). It handles event \a e.

  You can reimplement this function in a subclass, but we recommend
  using one of the specialized event handlers instead.

  Specialised handlers for viewport events are: paintEvent(),
  mousePressEvent(), mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), wheelEvent(), dragEnterEvent(), dragMoveEvent(),
  dragLeaveEvent(), dropEvent(), contextMenuEvent(), and
  resizeEvent().

 */
bool QViewport::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Resize:
    case QEvent::Paint:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::ContextMenu:
#ifndef QT_NO_DRAGANDDROP
    case QEvent::Drop:
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
#endif
        return QFrame::event(e);
    case QEvent::Wheel:
        if (!QFrame::event(e)) {
            if (static_cast<QWheelEvent*>(e)->orientation() == Horizontal)
                return QApplication::sendEvent(d->hbar, e);
            return QApplication::sendEvent(d->vbar, e);
        }
    default:
        return static_cast<QViewportHelper*>(d->viewport)->QWidget::event(e);
    }
    return true;
}

/*!
    This event handler can be reimplemented in a subclass to receive
    resize events for the viewport() widget. When resizeEvent() is
    called, the viewport already has its new geometry. The old size is
    accessible through QResizeEvent::oldSize().

    \sa QWidget::resizeEvent()
 */
void QViewport::resizeEvent(QResizeEvent *)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    paint events for the viewport() widget.

    Note: If you open a painter, make sure to open it on the
    viewport().

    \sa QWidget::paintEvent()
*/
void QViewport::paintEvent(QPaintEvent*)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse press events for the viewport() widget.

    \sa QWidget::mousePressEvent()
*/
void QViewport::mousePressEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse release events for the viewport() widget.

    \sa QWidget::mouseReleaseEvent()
*/
void QViewport::mouseReleaseEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse double click events for the viewport() widget.

    \sa QWidget::mouseDoubleClickEvent()
*/
void QViewport::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse move events for the viewport() widget.

    \sa QWidget::mouseMoveEvent()
*/
void QViewport::mouseMoveEvent(QMouseEvent *e)
{
    e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    wheel events for the viewport() widget.

    \sa QWidget::wheelEvent()
*/
#ifndef QT_NO_WHEELEVENT
void QViewport::wheelEvent(QWheelEvent *e)
{
    e->ignore();
}
#endif

/*!
    This event handler can be reimplemented in a subclass to receive
    context menu events for the viewport() widget.

    \sa QWidget::contextMenuEvent()
*/
void QViewport::contextMenuEvent(QContextMenuEvent *e)
{
    QFrame::contextMenuEvent(e);
}

void QViewport::keyPressEvent(QKeyEvent * e)
{
    switch (e->key()) {
    case Key_PageUp:
        d->vbar->triggerAction(QScrollBar::SliderPageStepSub);
        break;
    case Key_PageDown:
        d->vbar->triggerAction(QScrollBar::SliderPageStepAdd);
        break;
    case Key_Up:
        d->vbar->triggerAction(QScrollBar::SliderSingleStepSub);
        break;
    case Key_Down:
        d->vbar->triggerAction(QScrollBar::SliderSingleStepAdd);
        break;
    case Key_Left:
        d->hbar->triggerAction(QScrollBar::SliderSingleStepSub);
        break;
    case Key_Right:
        d->hbar->triggerAction(QScrollBar::SliderSingleStepAdd);
        break;
    default:
        e->ignore();
        return;
    }
    e->accept();
}


#ifndef QT_NO_DRAGANDDROP
/*!
    This event handler can be reimplemented in a subclass to receive
    drag enter events for the viewport() widget.

    \sa QWidget::dragEnterEvent()
*/
void QViewport::dragEnterEvent(QDragEnterEvent *)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    drag move events for the viewport() widget.

    \sa QWidget::dragMoveEvent()
*/
void QViewport::dragMoveEvent(QDragMoveEvent *)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    drag leave events for the viewport() widget.

    \sa QWidget::dragLeaveEvent()
*/
void QViewport::dragLeaveEvent(QDragLeaveEvent *)
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    drop events for the viewport() widget.

    \sa QWidget::dropEvent()
*/
void QViewport::dropEvent(QDropEvent *)
{
}


#endif

void QViewport::scrollContentsBy(int, int)
{
    viewport()->update();
}

void QViewportPrivate::hslide(int x)
{
    int dx = xoffset - x;
    xoffset = x;
    q->scrollContentsBy(dx, 0);
}

void QViewportPrivate::vslide(int y)
{
    int dy = yoffset - y;
    yoffset = y;
    q->scrollContentsBy(0, dy);
}

void QViewportPrivate::showOrHideScrollBars()
{
    layoutChildren();
}

QSize QViewport::minimumSizeHint() const
{
    int h = fontMetrics().height();
    if (h < 10)
        h = 10;
    int f = 2 * d->frameWidth;
    return QSize((6 * h) + f, (4 * h) + f);
}

#include "moc_qviewport.cpp"
