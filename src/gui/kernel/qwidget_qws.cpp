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

#include "qcursor.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qhash.h"
#include "qstack.h"
#include "qdragobject.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qcursor.h"
#include "qinputcontext_p.h"
#include "qdesktopwidget.h"
#include "qcleanuphandler.h"

#include "qwsdisplay_qws.h"
#include "qscreen_qws.h"
#include "qwsmanager_qws.h"
#include "qwsregionmanager_qws.h"
#include "qinputcontext_p.h"

#include "qpaintengine_qws.h"

#include "qwidget_p.h"
#define d d_func()
#define q q_func()


// Paint event clipping magic
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

extern bool qt_xdnd_enable(QWidget* w, bool on);

extern int *qt_last_x;
extern int *qt_last_y;
extern WId qt_last_cursor;
extern bool qws_overrideCursor;
extern QWidget *qt_pressGrab;
extern QWidget *qt_mouseGrb;

extern QRect qt_maxWindowRect;

extern void qwsUpdateActivePainters();

static QWidget *keyboardGrb = 0;

static int takeLocalId()
{
    static int n=-1000;
    return --n;
}

// This repaints all children within a widget.

static void paint_children(QWidget * p,const QRegion& r, bool update)
{
    if(!p)
        return;
    QObjectList childObjects=p->children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject * o = childObjects.at(i);

        if(o->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(o);
                if (w->testWState(Qt::WState_Visible)) {
                    QRegion wr(QRegion(w->geometry()) & r);
                    if (!wr.isEmpty()) {
                        wr.translate(-w->x(),-w->y());
                        if (update || w->testWState(Qt::WState_InPaintEvent))
                            w->update(wr);
                        else
                            w->repaint(wr);
                        paint_children(w,wr,update);
                    }
                }
            }
    }

}

// Paint the widget and its children

static void paint_hierarchy(QWidget *w, bool update)
{
    if (w && w->testWState(Qt::WState_Visible)) {
        if (update)
            w->update(w->rect());
        else
            w->repaint(w->rect());

        QObjectList childObjects = w->children();
        for (int i = 0; i < childObjects.size(); ++i) {
            QObject *o = childObjects.at(i);
            if(o->isWidgetType())
                paint_hierarchy(static_cast<QWidget *>(o),update);
        }
    }
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

void QWidget::create(WId window, bool initializeWindow, bool /*destroyOldWindow*/)
{
    if (testWState(Qt::WState_Created) && window == 0)
        return;
    setWState(Qt::WState_Created);                        // set created flag

    if (!parentWidget() || parentWidget()->isDesktop())
        setWFlags(Qt::WType_TopLevel);                // top-level widget

    data->alloc_region_index = -1;
    data->alloc_region_revision = -1;
    isSettingGeometry = false;
    data->overlapping_children = -1;

    bool topLevel = testWFlags(Qt::WType_TopLevel);
    bool popup = testWFlags(Qt::WType_Popup);
    bool dialog = testWFlags(Qt::WType_Dialog);
    bool desktop = testWFlags(Qt::WType_Desktop);
    WId           id;
    QWSDisplay* dpy = qwsDisplay();

    if (!window)                                // always initialize
        initializeWindow = true;

    if (popup) {
        setWFlags(Qt::WStyle_Tool); // a popup is a tool window
        setWFlags(Qt::WStyle_StaysOnTop); // a popup stays on top
    }
    if (topLevel && parentWidget()) {
        // if our parent has Qt::WStyle_StaysOnTop, so must we
        QWidget *ptl = parentWidget()->topLevelWidget();
        if (ptl && ptl->testWFlags(Qt::WStyle_StaysOnTop))
            setWFlags(Qt::WStyle_StaysOnTop);
    }

    int sw = dpy->width();
    int sh = dpy->height();

    if (dialog || popup || desktop) {                // these are top-level, too
        topLevel = true;
        setWFlags(Qt::WType_TopLevel);
    }

    if (desktop) {                                // desktop widget
        dialog = popup = false;                        // force these flags off
        data->crect.setRect(0, 0, sw, sh);
    } else if (topLevel) {                        // calc pos/size from screen
        data->crect.setRect(0, 0, sw/2, 4*sh/10);
    } else {                                        // child widget
        data->crect.setRect(0, 0, 100, 30);
    }

    if (window) {                                // override the old window
        id = window;
        setWinId(window);
    } else if (desktop) {                        // desktop widget
        id = (WId)-2;                                // id = root window
        QWidget *otherDesktop = find(id);        // is there another desktop?
        if (otherDesktop && otherDesktop->testWFlags(Qt::WPaintDesktop)) {
            otherDesktop->setWinId(0);        // remove id from widget mapper
            setWinId(id);                        // make sure otherDesktop is
            otherDesktop->setWinId(id);        //   found first
        } else {
            setWinId(id);
        }
    } else {
        id = topLevel ? dpy->takeId() : takeLocalId();
        setWinId(id);                                // set widget id/handle + hd
    }

    if (!topLevel) {
        if (!testWFlags(Qt::WStyle_Customize))
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_MinMax | Qt::WStyle_SysMenu );
    } else if (!(desktop || popup)) {
        if (testWFlags(Qt::WStyle_Customize)) {        // customize top-level widget
            if (testWFlags(Qt::WStyle_NormalBorder)) {
                // XXX ...
            } else {
                if (!testWFlags(Qt::WStyle_DialogBorder)) {
                    // XXX ...
                }
            }
            if (testWFlags(Qt::WStyle_Tool)) {
                // XXX ...
            }
        } else {                                // normal top-level widget
            setWFlags(Qt::WStyle_NormalBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu |
                       Qt::WStyle_MinMax);
        }
    }

    data->alloc_region_dirty=false;
    data->paintable_region_dirty=false;

    if (!initializeWindow) {
        // do no initialization
    } else if (popup) {                        // popup widget
    } else if (topLevel && !desktop) {        // top-level widget
        QWidget *p = parentWidget();        // real parent
        if (p)
            p = p->topLevelWidget();
        if (testWFlags(Qt::WStyle_DialogBorder)
             || testWFlags(Qt::WStyle_StaysOnTop)
             || testWFlags(Qt::WStyle_Dialog)
             || testWFlags(Qt::WStyle_Tool)) {
            // XXX ...
        }

        // find the real client leader, i.e. a toplevel without parent
        while (p && p->parentWidget()) {
            p = p->parentWidget()->topLevelWidget();
        }

        // XXX ...
    }

    if (initializeWindow) {
    }

    setAttribute(Qt::WA_MouseTracking, true);
    setMouseTracking(false);                        // also sets event mask
    if (desktop) {
        setWState(Qt::WState_Visible);
    } else if (topLevel) {                        // set X cursor
        //QCursor *oc = QApplication::overrideCursor();
        if (initializeWindow) {
            //XXX XDefineCursor(dpy, winid, oc ? oc->handle() : cursor().handle());
        }
        setAttribute(Qt::WA_SetCursor);
#ifndef QT_NO_WIDGET_TOPEXTRA
        qwsDisplay()->nameRegion(winId(), objectName(), windowTitle());
#else
        qwsDisplay()->nameRegion(winId(), objectName(), QString::null);
#endif
    }

    if (topLevel) {
#ifndef QT_NO_WIDGET_TOPEXTRA
        d->createTLExtra();
#endif
#ifndef QT_NO_QWS_MANAGER
        if (testWFlags(Qt::WStyle_DialogBorder)
             || testWFlags(Qt::WStyle_NormalBorder))
        {
            // get size of wm decoration and make the old crect the new frect
            QRect cr = data->crect;
            QRegion r = QApplication::qwsDecoration().region(this, cr);
            QRect br(r.boundingRect());
            d->extra->topextra->fleft = cr.x() - br.x();
            d->extra->topextra->ftop = cr.y() - br.y();
            d->extra->topextra->fright = br.right() - cr.right();
            d->extra->topextra->fbottom = br.bottom() - cr.bottom();
            data->crect.addCoords(d->extra->topextra->fleft, d->extra->topextra->ftop,
                                  -d->extra->topextra->fright, -d->extra->topextra->fbottom);
            d->topData()->qwsManager = new QWSManager(this);
        } else if (d->topData()->qwsManager) {
            delete d->topData()->qwsManager;
            d->topData()->qwsManager = 0;
            data->crect.moveBy(-d->extra->topextra->fleft, -d->extra->topextra->ftop);
            d->extra->topextra->fleft = d->extra->topextra->ftop =
                d->extra->topextra->fright = d->extra->topextra->fbottom = 0;
        }
#endif
        // declare the widget's object name as window role

        qt_fbdpy->addProperty(id,QT_QWS_PROPERTY_WINDOWNAME);
        qt_fbdpy->setProperty(id,QT_QWS_PROPERTY_WINDOWNAME,0,objectName().latin1());

        // If we are session managed, inform the window manager about it
        if (d->extra && !d->extra->mask.isEmpty()) {
            data->req_region = d->extra->mask;
            data->req_region.translate(data->crect.x(),data->crect.y());
            data->req_region &= data->crect; //??? this is optional
        } else {
            data->req_region = data->crect;
        }
        data->req_region = qt_screen->mapToDevice(data->req_region, QSize(qt_screen->width(), qt_screen->height()));
    } else {
        if (d->extra && d->extra->topextra)        { // already allocated due to reparent?
            d->extra->topextra->fleft = 0;
            d->extra->topextra->ftop = 0;
            d->extra->topextra->fright = 0;
            d->extra->topextra->fbottom = 0;
        }
        //updateRequestedRegion(mapToGlobal(QPoint(0,0)));
    }
}


void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    deactivateWidgetCleanup();
    if (testWState(Qt::WState_Created)) {
        clearWState(Qt::WState_Created);
        QObjectList childObjects =  children();
        for (int i = 0; i < childObjects.size(); ++i) {
            QObject *obj = childObjects.at(i);
            if (obj->isWidgetType())
                static_cast<QWidget*>(obj)->destroy(destroySubWindows,
                                                     destroySubWindows);
        }
        releaseMouse();
        if (qt_pressGrab == this)
          qt_pressGrab = 0;

        if (keyboardGrb == this)
            releaseKeyboard();
        if (testWFlags(Qt::WShowModal))                // just be sure we leave modal
            qt_leave_modal(this);
        else if (testWFlags(Qt::WType_Popup))
            qApp->closePopup(this);
        if (testWFlags(Qt::WType_Desktop)) {
        } else {
            if (parentWidget() && parentWidget()->testWState(Qt::WState_Created)) {
                hide_sys();
            }
            if (destroyWindow && isTopLevel())
                qwsDisplay()->destroyRegion(winId());
        }
        setWinId(0);
    }
}


void QWidget::setParent_sys(QWidget *parent, Qt::WFlags f)
{
#ifndef QT_NO_CURSOR
    QCursor oldcurs;
    bool setcurs=testAttribute(Qt::WA_SetCursor);
    if (setcurs) {
        oldcurs = cursor();
        unsetCursor();
    }
#endif

    WId old_winid = data->winid;
    if (testWFlags(Qt::WType_Desktop))
        old_winid = 0;

    if (!isTopLevel() && parentWidget() && parentWidget()->testWState(Qt::WState_Created))
        hide_sys();

    setWinId(0);

    if (d->parent != parent) {
        QWidget *oldparent = parentWidget();
        QObject::setParent_helper(parent);
        if (oldparent) {
            oldparent->setChildrenAllocatedDirty();
            oldparent->data->paintable_region_dirty = true;
        }
        if (parent) {
            parent->setChildrenAllocatedDirty();
            parent->data->paintable_region_dirty = true;
        }
    }
    bool     enable = isEnabled();                // remember status
    Qt::FocusPolicy fp = focusPolicy();
    QSize    s            = size();
    //QBrush   bgc    = background();                        // save colors
#ifndef QT_NO_WIDGET_TOPEXTRA
    QString capt = windowTitle();
#endif
    data->widget_flags = f;
    clearWState(Qt::WState_Created | Qt::WState_Visible | Qt::WState_Hidden | Qt::WState_ExplicitShowHide);
    create();
    if (isTopLevel() || (!parent || parent->isVisible()))
        setWState(Qt::WState_Hidden);
    setGeometry(0, 0, s.width(), s.height());
    setEnabled(enable);
    setFocusPolicy(fp);
#ifndef QT_NO_WIDGET_TOPEXTRA
    if (!capt.isNull()) {
        d->extra->topextra->caption = QString::null;
        setWindowTitle(capt);
    }
#endif
    if ((int)old_winid > 0)
        qwsDisplay()->destroyRegion(old_winid);
#ifndef QT_NO_CURSOR
    if (setcurs) {
        setCursor(oldcurs);
    }
#endif
}


QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    int           x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
        x += w->data->crect.x();
        y += w->data->crect.y();
        w = w->isTopLevel() ? 0 : w->parentWidget();
    }
    return QPoint(x, y);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    int           x=pos.x(), y=pos.y();
    const QWidget* w = this;
    while (w) {
        x -= w->data->crect.x();
        y -= w->data->crect.y();
        w = w->isTopLevel() ? 0 : w->parentWidget();
    }
    return QPoint(x, y);
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height,
                                 bool text, QFont *)
{
    if (QRect(x, y, width, height) != microFocusHint()) {
        d->createExtra();
        d->extra->micro_focus_hint.setRect(x, y, width, height);
    }
#ifndef QT_NO_QWS_IM
    if (text) {
        QWidget *tlw = topLevelWidget();
        int winid = tlw->winId();
        QPoint p(x, y + height);
        QPoint gp = mapToGlobal(p);

        QRect r = QRect(mapToGlobal(QPoint(0,0)),
                         size());

        r.setBottom(tlw->geometry().bottom());

        //qDebug("QWidget::setMicroFocusHint %d %d %d %d", r.x(),
        //        r.y(),  r.width(), r.height());
        QInputContext::setMicroFocusWidget(this);

        qwsDisplay()->setIMInfo(winid, gp.x(), gp.y(), r);

        //send font info,  ###if necessary
        qwsDisplay()->setInputFont(winid, font());
    }
#endif
}


void QWidgetPrivate::setFont_sys(QFont *)
{
}

void QWidgetPrivate::updateSystemBackground() {}

#ifndef QT_NO_CURSOR

void QWidget::setCursor(const QCursor &cursor)
{
    d->createExtra();
    delete d->extra->curs;
    d->extra->curs = new QCursor(cursor);
    setAttribute(Qt::WA_SetCursor);
    if (isVisible())
        updateCursor(paintableRegion());
}

void QWidget::unsetCursor()
{
    if (d->extra) {
        delete d->extra->curs;
        d->extra->curs = 0;
    }
    setAttribute(Qt::WA_SetCursor, false);
    if (isVisible())
        updateCursor(paintableRegion());
}
#endif //QT_NO_CURSOR

/*!
    \property QWidget::windowModified
    \brief whether the window's surface has been modified

    \internal
*/

void QWidget::setWindowModified(bool mod)
{
    setAttribute(Qt::WA_WindowModified, mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(Qt::WA_WindowModified);
}

#ifndef QT_NO_WIDGET_TOPEXTRA
void QWidget::setWindowTitle(const QString &caption)
{
    if (d->extra && d->extra->topextra && d->extra->topextra->caption == caption)
        return; // for less flicker
    d->createTLExtra();
    d->extra->topextra->caption = caption;
    qwsDisplay()->setWindowCaption(this, caption);
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidgetPrivate::setWindowIcon_sys(const QPixmap &unscaledPixmap)
{
     QTLWExtra* x = d->topData();
     delete x->icon;
     x->icon = 0;
    QBitmap mask;
    if (unscaledPixmap.isNull()) {
    } else {
        QImage unscaledIcon = unscaledPixmap.convertToImage();
        QPixmap pixmap;
#ifndef QT_NO_IMAGE_SMOOTHSCALE
        pixmap.convertFromImage(unscaledIcon.smoothScale(16, 16));
#else
        pixmap.convertFromImage(unscaledIcon);
#endif
        x->icon = new QPixmap(pixmap);
        mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
}


void QWidget::setWindowIconText(const QString &iconText)
{
    d->createTLExtra();
    d->extra->topextra->iconText = iconText;
    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
}
#endif

void QWidget::grabMouse()
{
    if (qt_mouseGrb)
        qt_mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,true);

    qt_mouseGrb = this;
    qt_pressGrab = 0;
}

#ifndef QT_NO_CURSOR
void QWidget::grabMouse(const QCursor &cursor)
{
    if (qt_mouseGrb)
        qt_mouseGrb->releaseMouse();

    qwsDisplay()->grabMouse(this,true);
    qwsDisplay()->selectCursor(this, (unsigned int)cursor.handle());
    qt_mouseGrb = this;
    qt_pressGrab = 0;
}
#endif

void QWidget::releaseMouse()
{
    if (qt_mouseGrb == this) {
        qwsDisplay()->grabMouse(this,false);
        qt_mouseGrb = 0;
    }
}

void QWidget::grabKeyboard()
{
    if (keyboardGrb)
        keyboardGrb->releaseKeyboard();
    qwsDisplay()->grabKeyboard(this, true);
    keyboardGrb = this;
}

void QWidget::releaseKeyboard()
{
    if (keyboardGrb == this) {
        qwsDisplay()->grabKeyboard(this, false);
        keyboardGrb = 0;
    }
}


QWidget *QWidget::mouseGrabber()
{
    if (qt_mouseGrb)
        return qt_mouseGrb;
    return qt_pressGrab;
}


QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if (tlw->isVisible()) {
        qwsDisplay()->requestFocus(tlw->winId(), true);
    }
}


void QWidget::update()
{
    if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible)
        QApplication::postEvent(this, new QWSUpdateEvent(clipRegion()));
}

void QWidget::update(const QRegion &rgn)
{
     if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible)
         QApplication::postEvent(this, new QWSUpdateEvent(rgn&clipRegion()));
}

void QWidget::update(int x, int y, int w, int h)
{
    if (w && h && (data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) == Qt::WState_Visible) {
        if (w < 0)
            w = data->crect.width()  - x;
        if (h < 0)
            h = data->crect.height() - y;
        if (w != 0 && h != 0)
            QApplication::postEvent(this,
                    new QWSUpdateEvent(clipRegion().intersect(QRect(x,y,w,h))));
    }
}

struct QWSDoubleBuffer
{
    enum {
        MaxWidth = SHRT_MAX,
        MaxHeight = SHRT_MAX
    };

    QPixmap *hd;
    int depth;
};

static QWSDoubleBuffer *qt_global_double_buffer = 0;
static bool qt_global_double_buffer_active = false;

static void qt_discard_double_buffer(QWSDoubleBuffer **db)
{
    if (!*db)
        return;

    delete (*db)->hd;
    delete *db;
    *db = 0;
}

void qt_discard_double_buffer()
{
    qt_discard_double_buffer(&qt_global_double_buffer);
}

static QWSDoubleBuffer *qt_qws_create_double_buffer(int width, int height, int depth)
{
    QWSDoubleBuffer *db = new QWSDoubleBuffer;
    db->depth = depth;
    db->hd = new QPixmap(width, height, db->depth);
    Q_ASSERT(db->hd);
    return db;
}

static void qt_qws_get_double_buffer(QWSDoubleBuffer **db, int width, int height, int depth)
{
    // the db should consist of 128x128 chunks
    width  = qMin(((width / 128) + 1) * 128, int(QWSDoubleBuffer::MaxWidth));
    height = qMin(((height / 128) + 1) * 128, int(QWSDoubleBuffer::MaxHeight));

    if (qt_global_double_buffer_active) {
        *db = qt_qws_create_double_buffer(width, height, depth);
        return;
    }

    qt_global_double_buffer_active = true;

    if (qt_global_double_buffer) {
        if (qt_global_double_buffer->hd->width() >= width
            && qt_global_double_buffer->hd->height() >= height) {
            *db = qt_global_double_buffer;
            return;
        }

        width  = qMax(qt_global_double_buffer->hd->width(), width);
        height = qMax(qt_global_double_buffer->hd->height(), height);

        qt_discard_double_buffer(&qt_global_double_buffer);
    }

    qt_global_double_buffer = qt_qws_create_double_buffer(width, height, depth);
    *db = qt_global_double_buffer;
};

static void qt_qws_release_double_buffer(QWSDoubleBuffer **db)
{
    if (*db != qt_global_double_buffer)
        qt_discard_double_buffer(db);
    else
        qt_global_double_buffer_active = false;
}

void QWidget::repaint(const QRegion& rgn)
{
    if (testWState(Qt::WState_InPaintEvent))
        qWarning("QWidget::repaint: recursive repaint detected.");

    if ((data->widget_state & (Qt::WState_Visible|Qt::WState_BlockUpdates)) != Qt::WState_Visible)
        return;

    if (rgn.isEmpty())
        return;

    setWState(Qt::WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    bool do_clipping = (br != QRect(0, 0, data->crect.width(), data->crect.height()));
    bool double_buffer = (!testAttribute(Qt::WA_PaintOnScreen)
                          && !testAttribute(Qt::WA_NoSystemBackground)
                          && br.width()  <= QWSDoubleBuffer::MaxWidth
                          && br.height() <= QWSDoubleBuffer::MaxHeight
                          && !QPainter::redirected(this));

    QPoint redirectionOffset;
    QWSDoubleBuffer *qDoubleBuffer = 0;
    if (double_buffer) {
        qt_qws_get_double_buffer(&qDoubleBuffer, br.width(), br.height(), qwsDisplay()->depth());
        redirectionOffset = br.topLeft();
        QPainter::setRedirected(this, qDoubleBuffer->hd, redirectionOffset);
    }

    QPainter p; // We'll use it several times

    // Set clipping
    if (do_clipping) {
        if (redirectionOffset.isNull()) {
            qt_set_paintevent_clipping(this, rgn);
        } else {
            QRegion redirectionRegion(rgn);
            redirectionRegion.translate(-redirectionOffset);
            qt_set_paintevent_clipping(this, redirectionRegion);
        }
    }

    if (!testAttribute(Qt::WA_NoBackground) && !testAttribute(Qt::WA_NoSystemBackground)) {
#ifndef QT_NO_PALETTE
        QBrush bg = palette().brush(d->bg_role);
#else
        QBrush bg(red); //##########
#endif

        // Contents propagation list
        QPoint offset;
        QStack<QWidget*> parents;
        QWidget *w = this;
        while (w->d->isBackgroundInherited()) {
            offset += w->pos();
            w = w->parentWidget();
            parents += w;
        }

        // Erase background
        bool was_unclipped = testAttribute(Qt::WA_PaintUnclipped);
        setAttribute(Qt::WA_PaintUnclipped, false);
        p.begin(this);
        if(was_unclipped)
            setAttribute(Qt::WA_PaintUnclipped);
        p.setClipRegion(rgn);
        if(bg.pixmap())
            p.drawTiledPixmap(br,*bg.pixmap(), QPoint(br.x()+(offset.x()%bg.pixmap()->width()),
                                                      br.y()+(offset.y()%bg.pixmap()->height())));
        else
            p.fillRect(br, bg.color());
        p.end();

        // Actual contents propagation (..parentparentparent->parentparent->parent)
        if (!parents.isEmpty()) {
            w = parents.pop();
            for (;;) {
                if (w->testAttribute(Qt::WA_ContentsPropagated)) {
                    if (double_buffer)
                        QPainter::setRedirected(w, qDoubleBuffer->hd, offset+redirectionOffset);
                    else
                        QPainter::setRedirected(w, this, offset);
                    QRect rr = rect();
                    rr.moveBy(offset);
                    QPaintEvent e(rr);
                    bool was_in_paint_event = w->testWState(Qt::WState_InPaintEvent);
                    w->setWState(Qt::WState_InPaintEvent);
                    QApplication::sendEvent(w, &e);
                    if(!was_in_paint_event) {
                        w->clearWState(Qt::WState_InPaintEvent);
                        if(!w->testAttribute(Qt::WA_PaintOutsidePaintEvent) && w->paintingActive())
                            qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");
                    }
                    QPainter::restoreRedirected(w);
                }
                if (parents.isEmpty())
                    break;
                w = parents.pop();
                offset -= w->pos();
            }
        }
    }

    // Send paint event to self
    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(this, &e);

    // Clear the clipping again
    if (do_clipping)
        qt_clear_paintevent_clipping();

    // Flush double buffer, if used
    if (double_buffer) {
        QPainter::restoreRedirected(this);

        p.begin(this);
        QVector<QRect> rects = rgn.rects();
        for (int i = 0; i < rects.size(); ++i) {
            QRect rr = rects.at(i);
            p.drawPixmap(rr.topLeft(), *(qDoubleBuffer->hd),
                         QRect(rr.topLeft() - redirectionOffset, rr.size()));
        }
        p.end();

        qt_qws_release_double_buffer(&qDoubleBuffer);

        // Delete double buffer if not used within timeout
        if (!qApp->active_window) {
            extern int qt_double_buffer_timer;
            if (qt_double_buffer_timer)
                qApp->killTimer(qt_double_buffer_timer);
            qt_double_buffer_timer = qApp->startTimer(500);
        }
    }
    clearWState(Qt::WState_InPaintEvent);

    if(!testAttribute(Qt::WA_PaintOutsidePaintEvent) && paintingActive())
        qWarning("It is dangerous to leave painters active on a widget outside of the PaintEvent");

    if (testAttribute(Qt::WA_ContentsPropagated))
        d->updatePropagatedBackground(&rgn);
}

void QWidget::show_sys()
{
    if (testWFlags(Qt::WType_TopLevel)) {
        updateRequestedRegion(mapToGlobal(QPoint(0,0)));
        QRegion r(data->req_region);
#ifndef QT_NO_QWS_MANAGER
        if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
            QRegion wmr = d->extra->topextra->qwsManager->region();
            wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
            r += wmr;
        }
#endif
        qwsDisplay()->requestRegion(winId(), r);
        if (!testWFlags(Qt::WStyle_Tool)) {
            qwsDisplay()->requestFocus(winId(),true);
        }
        qwsDisplay()->setAltitude(winId(),
                testWFlags(Qt::WStyle_StaysOnTop) ? 1 : 0, true);

    } else if (!topLevelWidget()->data->in_show) {
        updateRequestedRegion(mapToGlobal(QPoint(0,0)));
        QWidget *p = parentWidget();
        p->setChildrenAllocatedDirty(geometry(), this);
        p->data->paintable_region_dirty = true;
        p->data->overlapping_children = -1;
        paint_hierarchy(this, true);
    }
}


void QWidget::hide_sys()
{
    deactivateWidgetCleanup();

    if (data->req_region.isEmpty())        // Already invisible?
        return;

    if (testWFlags(Qt::WType_TopLevel)) {
        releaseMouse();
        qwsDisplay()->requestRegion(winId(), QRegion());
        qwsDisplay()->requestFocus(winId(),false);
    } else {
        QWidget *p = parentWidget();
        if (p) {
            p->setChildrenAllocatedDirty(geometry(), this);
            p->data->paintable_region_dirty = true;
            if (p->data->overlapping_children)
                p->data->overlapping_children = -1;
            if (p->isVisible()) {
                p->update(geometry());
                paint_children(p,geometry(),true);
            }
        }
    }
    updateRequestedRegion(mapToGlobal(QPoint(0,0)));
}


static uint effectiveState(uint state)
{
    if (state & Qt::WState_Minimized)
        return Qt::WState_Minimized;
    else if (state & Qt::WState_FullScreen)
        return Qt::WState_FullScreen;
    if (state & Qt::WState_Maximized)
        return Qt::WState_Maximized;

    return 0;
}

void QWidget::setWindowState(uint newstate)
{
    data->in_set_window_state = 1;
    uint oldstate = effectiveState(data->widget_state);

    data->widget_state &= ~(Qt::WState_Minimized | Qt::WState_Maximized | Qt::WState_FullScreen);
    if (newstate & Qt::WindowMinimized)
        data->widget_state |= Qt::WState_Minimized;
    if (newstate & Qt::WindowMaximized)
        data->widget_state |= Qt::WState_Maximized;
    if (newstate & Qt::WindowFullScreen)
        data->widget_state |= Qt::WState_FullScreen;

    uint state = effectiveState(data->widget_state);

    bool needShow = false;
    if (isTopLevel() && state != oldstate) {
        d->createTLExtra();
        if (oldstate == 0) { //normal
            d->topData()->normalGeometry = geometry();
        } else if (oldstate == Qt::WState_FullScreen) {
            setParent(0, d->topData()->savedFlags);
            needShow = true;
        } else if (oldstate == Qt::WState_Minimized) {
            needShow = true;
        }

        if (state == Qt::WState_Minimized) {
            //### not ideal...
            hide();
            needShow = false;
        } else if (state == Qt::WState_FullScreen) {
            d->topData()->savedFlags = getWFlags();
            setParent(0, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_NoBorder |
                         // preserve some widget flags
                      (getWFlags() & 0xffff0000));
            const QRect screen = qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this));
            move(screen.topLeft());
            resize(screen.size());
            raise();
            needShow = true;
        } else if (state == Qt::WState_Maximized) {
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
                d->extra->topextra->qwsManager->maximize();
            else
#endif
                setGeometry(qt_maxWindowRect);
        } else { //normal
            QRect r = d->topData()->normalGeometry;
            if (r.width() >= 0) {
                d->topData()->normalGeometry = QRect(0,0,-1,-1);
                setGeometry(r);
            }
        }
    }
    data->in_set_window_state = 0;

    if (needShow)
        show();

    if (newstate & Qt::WindowActive)
        setActiveWindow();

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::raise()
{
    QWidget *p = parentWidget();
    if (p && p->d->children.contains(this)) {
        p->d->children.removeAll(this);
        p->d->children.append(this);
    }
    if (isTopLevel()) {
#ifdef QT_NO_WINDOWGROUPHINT
        if (!testWFlags(Qt::WStyle_Tool))
            setActiveWindow();
        qwsDisplay()->setAltitude(winId(), 0);
#else
        QWidget* act=0;
        if (!testWFlags(Qt::WStyle_Tool))
            act=this;
        qwsDisplay()->setAltitude(winId(), 0);

        QObjectList childObjects =  children();
        if (!childObjects.isEmpty()) {
            QWidgetList toraise;
            for (int i = 0; i < childObjects.size(); ++i) {
                QObject *obj = childObjects.at(i);
                if (obj->isWidgetType()) {
                    QWidget* w = static_cast<QWidget*>(obj);
                    if (w->isTopLevel())
                        toraise.append(w);
                }
            }

            for (int i = 0; i < toraise.size(); ++i) {
                QWidget *w = toraise.at(i);

                if (w->isVisible()) {
                    bool wastool = w->testWFlags(Qt::WStyle_Tool);
                    w->setWFlags(Qt::WStyle_Tool); // avoid setActiveWindow flicker
                    w->raise();
                    if (!wastool) {
                        w->clearWFlags(Qt::WStyle_Tool);
                        act = w;
                    }
                }
            }
        }
        if (act)
            act->setActiveWindow();
#endif // QT_NO_WINDOWGROUPHINT
    } else if (p) {
        p->setChildrenAllocatedDirty(geometry(), this);
        paint_hierarchy(this, true);
    }
}

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if (p && p->d->children.contains(this)) {
        p->d->children.removeAll(this);
        p->d->children.insert(0, this);
    }
    if (isTopLevel()) {
        qwsDisplay()->setAltitude(winId(), -1);
    } else if (p) {
        p->setChildrenAllocatedDirty(geometry());
        paint_children(p,geometry(),true);
    }
}

void QWidget::stackUnder(QWidget* w)
{
    QWidget *p = parentWidget();
    if (!p || !w || isTopLevel() || p != w->parentWidget())
        return;
    int loc = p->d->children.indexOf(w);
    if (loc >= 0 && p->d->children.contains(this)) {
        p->d->children.removeAll(this);
        loc = p->d->children.indexOf(w);
        p->d->children.insert(loc, this);
    }
    if (p) {
        // #### excessive repaints
        p->setChildrenAllocatedDirty();
        paint_children(p,geometry(),true);
        paint_children(p,w->geometry(),true);
    }
}


void QWidget::setGeometry_sys(int x, int y, int w, int h, bool isMove)
{
    if (d->extra) {                                // any size restrictions?
        w = qMin(w,d->extra->maxw);
        h = qMin(h,d->extra->maxh);
        w = qMax(w,d->extra->minw);
        h = qMax(h,d->extra->minh);
    }
    if (isTopLevel()) {
        w = qMax(1, w);
        h = qMax(1, h);
    }

    QPoint oldp = geometry().topLeft();
    QSize olds = size();
    QRect r(x, y, w, h);

    bool isResize = olds != r.size();

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if (r.size() == olds && oldp == r.topLeft())
        return;

    QRegion oldAlloc;
    if (!isTopLevel() && isMove && (w==olds.width() && h==olds.height())) {
        oldAlloc = allocatedRegion();
    }

    if (!data->in_set_window_state) {
        clearWState(Qt::WState_Maximized);
        clearWState(Qt::WState_FullScreen);
        if (isTopLevel())
            d->topData()->normalGeometry = QRect(0, 0, -1, -1);
    }
    QPoint oldPos = pos();
    data->crect = r;

    if (testWFlags(Qt::WType_Desktop))
        return;

    if (isTopLevel()) {
        //### ConfigPending not implemented, do we need it?
        //setWState(Qt::WState_ConfigPending);
        if (isMove && (w==olds.width() && h==olds.height())) {
            // just need to translate current region
            QSize s(qt_screen->width(), qt_screen->height());
            QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
            QPoint td2 = qt_screen->mapToDevice(QPoint(x - oldp.x(),y - oldp.y()), s);
            QPoint dd = QPoint(td2.x()-td1.x(), td2.y()-td1.y());
            data->req_region.translate(dd.x(), dd.y());
        } else {
            if (d->extra && !d->extra->mask.isEmpty()) {
                data->req_region = d->extra->mask;
                data->req_region.translate(data->crect.x(),data->crect.y());
                data->req_region &= data->crect; //??? this is optional
            } else {
                data->req_region = data->crect;
            }
            data->req_region = qt_screen->mapToDevice(data->req_region, QSize(qt_screen->width(), qt_screen->height()));
        }
        if (isVisible()) {
            if (isMove && !isResize && data->alloc_region_index >= 0) {
                qwsDisplay()->moveRegion(winId(), x - oldp.x(), y - oldp.y());
                setChildrenAllocatedDirty();
            } else {
                QRegion rgn(data->req_region);
#ifndef QT_NO_QWS_MANAGER
                if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                    QRegion wmr = d->extra->topextra->qwsManager->region();
                    wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
                    rgn += wmr;
                }
#endif
                qwsDisplay()->requestRegion(winId(), rgn);
                if (d->extra && d->extra->topextra) {
                    QRect br(rgn.boundingRect());
                    br = qt_screen->mapFromDevice(br, QSize(qt_screen->deviceWidth(), qt_screen->deviceHeight()));
                    d->extra->topextra->fleft = data->crect.x()-br.x();
                    d->extra->topextra->ftop = data->crect.y()-br.y();
                    d->extra->topextra->fright = br.right()-data->crect.right();
                    d->extra->topextra->fbottom = br.bottom()-data->crect.bottom();
                }
            }
        }
    }

    if (isVisible()) {
        isSettingGeometry = true;
        if (isMove) {
            QMoveEvent e(pos(), oldPos);
            QApplication::sendEvent(this, &e);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager)
                QApplication::sendEvent(d->extra->topextra->qwsManager, &e);
#endif
        }
        if (isResize) {
            QResizeEvent e(r.size(), olds);
            QApplication::sendEvent(this, &e);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                QResizeEvent e(r.size(), olds);
                QApplication::sendEvent(d->topData()->qwsManager, &e);
            }
#endif
        }

        updateRequestedRegion(mapToGlobal(QPoint(0,0)));

        QWidget *p = parentWidget();
        if (!isTopLevel() || isResize) {
            if (p && !isTopLevel()) {
                p->data->paintable_region_dirty = true;
                QRegion oldr(QRect(oldp, olds));
                dirtyChildren.translate(x, y);
                if (p->isSettingGeometry) {
                    if (oldp != r.topLeft()) {
                        QRegion upd((QRegion(r) | oldr) & p->rect());
                        dirtyChildren |= upd;
                    } else {
                        dirtyChildren |= QRegion(r) - oldr;
                        update(rect());
                    }
                    p->dirtyChildren |= dirtyChildren;
                } else {
                    QRegion upd((QRegion(r) | oldr) & p->rect());
                    dirtyChildren |= upd;
                    QRegion paintRegion = dirtyChildren;
#define FAST_WIDGET_MOVE
#ifdef FAST_WIDGET_MOVE
                    if (isMove && (w==olds.width() && h==olds.height())) {
                        QSize s(qt_screen->width(), qt_screen->height());

                        QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
                        QPoint td2 = qt_screen->mapToDevice(QPoint(x - oldp.x(),y - oldp.y()), s);
                        QPoint dd = QPoint(td2.x()-td1.x(), td2.y()-td1.y());
                        oldAlloc.translate(dd.x(), dd.y());

                        QRegion alloc(allocatedRegion());

                        QRegion scrollRegion(alloc & oldAlloc);
                        if (!scrollRegion.isEmpty()) {
                            bool was_unclipped = testAttribute(Qt::WA_PaintUnclipped);
                            setAttribute(Qt::WA_PaintUnclipped);

                            QWSPaintEngine * engine=static_cast<QWSPaintEngine*>(paintEngine());
                            engine->begin(this);

                            engine->setClipDeviceRegion(scrollRegion);
                            engine->scroll(x,y,w,h,oldp.x(),oldp.y());
                            engine->end();
                            if (!was_unclipped)
                                setAttribute(Qt::WA_PaintUnclipped,false);

                            QSize ds(qt_screen->deviceWidth(), qt_screen->deviceHeight());
                            scrollRegion = qt_screen->mapFromDevice(scrollRegion, ds);
                            QPoint gp = p->mapToGlobal(QPoint(0,0));
                            scrollRegion.translate(-gp.x(), -gp.y());
                            paintRegion -= scrollRegion;
                        }
                    }
#endif
                    if (!oldr.isEmpty())
                        p->update(oldr);
                    p->setChildrenAllocatedDirty(dirtyChildren, this);
                    qwsUpdateActivePainters();
                    paint_children(p, paintRegion, isResize);
                }
                p->data->overlapping_children = -1;
            } else {
                if (oldp != r.topLeft()) {
                    qwsUpdateActivePainters();
                    paint_hierarchy(this, true);
                } else {
                    setChildrenAllocatedDirty(dirtyChildren);
                    qwsUpdateActivePainters();
                    QApplication::postEvent(this, new QWSUpdateEvent(rect()));
                    paint_children(this, dirtyChildren, true);
                }
            }
        } else {
            qwsUpdateActivePainters();
        }
#ifndef QT_NO_QWS_MANAGER
        if (isResize && d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
            QApplication::postEvent(d->topData()->qwsManager,
                                    new QPaintEvent(clipRegion()));
        }
#endif
        isSettingGeometry = false;
        dirtyChildren = QRegion();
    } else {
        if (isMove && pos() != oldPos)
            setAttribute(Qt::WA_PendingMoveEvent, true);
        if (isResize)
            setAttribute(Qt::WA_PendingResizeEvent, true);
    }
}


void QWidget::setMinimumSize(int minw, int minh)
{
    if (!qt_maxWindowRect.isEmpty()) {
        // This is really just a work-around. Layout shouldn't be asking
        // for minimum sizes bigger than the screen.
        if (minw > qt_maxWindowRect.width())
            minw = qt_maxWindowRect.width();
        if (minh > qt_maxWindowRect.height())
            minh = qt_maxWindowRect.height();
    }

    if (minw < 0 || minh < 0)
        qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if (d->extra->minw == minw && d->extra->minh == minh)
        return;
    d->extra->minw = minw;
    d->extra->minh = minh;
    if (minw > width() || minh > height())
        resize(qMax(minw,width()), qMax(minh,height()));
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if (maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
        qWarning("QWidget::setMaximumSize: (%s/%s) "
                "The largest allowed size is (%d,%d)",
                 objectName().isEmpty() ? "unnamed" : objectName().latin1(),
                 metaObject()->className(), QWIDGETSIZE_MAX,
                QWIDGETSIZE_MAX);
        maxw = qMin(maxw, QWIDGETSIZE_MAX);
        maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if (maxw < 0 || maxh < 0) {
        qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                "are not possible",
                 objectName().isEmpty() ? "unnamed" : objectName().latin1(),
                 metaObject()->className(), maxw, maxh);
        maxw = qMax(maxw, 0);
        maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if (d->extra->maxw == maxw && d->extra->maxh == maxh)
        return;
    d->extra->maxw = maxw;
    d->extra->maxh = maxh;
    if (maxw < width() || maxh < height())
        resize(qMin(maxw,width()), qMin(maxh,height()));
    updateGeometry();
}

void QWidget::setSizeIncrement(int w, int h)
{
    d->createTLExtra();
    QTLWExtra* x = d->extra->topextra;
    if (x->incw == w && x->inch == h)
        return;
    x->incw = w;
    x->inch = h;
    if (testWFlags(Qt::WType_TopLevel)) {
        // XXX ...
    }
}

void QWidget::setBaseSize(int basew, int baseh)
{
    d->createTLExtra();
    QTLWExtra* x = d->extra->topextra;
    if (x->basew == basew && x->baseh == baseh)
        return;
    x->basew = basew;
    x->baseh = baseh;
    if (testWFlags(Qt::WType_TopLevel)) {
        // XXX
    }
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    if (testWState(Qt::WState_BlockUpdates) && children().isEmpty())
        return;
    bool valid_rect = r.isValid();
    QRect sr = valid_rect?r:rect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if (dx > 0) {
        x1 = sr.x();
        x2 = x1+dx;
        w -= dx;
    } else {
        x2 = sr.x();
        x1 = x2-dx;
        w += dx;
    }
    if (dy > 0) {
        y1 = sr.y();
        y2 = y1+dy;
        h -= dy;
    } else {
        y2 = sr.y();
        y1 = y2-dy;
        h += dy;
    }

    if (dx == 0 && dy == 0)
        return;

    QSize s(qt_screen->width(), qt_screen->height());
    QRegion alloc = valid_rect ? paintableRegion() : allocatedRegion();

    QRegion dAlloc = alloc;
    QPoint td1 = qt_screen->mapToDevice(QPoint(0,0), s);
    QPoint td2 = qt_screen->mapToDevice(QPoint(dx,dy), s);
    dAlloc.translate(td2.x()-td1.x(), td2.y()-td1.y());

    QRegion scrollRegion(alloc & dAlloc);

    if (w > 0 && h > 0) {
        bool was_unclipped = testAttribute(Qt::WA_PaintUnclipped);
        setAttribute(Qt::WA_PaintUnclipped);

        QWSPaintEngine * engine=static_cast<QWSPaintEngine*>(paintEngine());
        engine->begin(this);

        engine->setClipDeviceRegion(scrollRegion);
        engine->scroll(x2,y2,w,h,x1,y1);
        engine->end();
        if (!was_unclipped)
            setAttribute(Qt::WA_PaintUnclipped,false);
    }
    data->paintable_region_dirty = true;

    QPoint gpos = mapToGlobal(QPoint());

    if (!valid_rect && children().size() > 0) {        // scroll children
        setChildrenAllocatedDirty();
        QPoint pd(dx, dy);
        QObjectList childObjects = children();
        for (int i = 0; i < childObjects.size(); ++i) { // move all children
            QObject *object = childObjects.at(i);
            if (object->isWidgetType()) {
                QWidget *w = static_cast<QWidget *>(object);
                QPoint oldp = w->pos();
                QRect  r(w->pos() + pd, w->size());
                w->data->crect = r;
                w->updateRequestedRegion(gpos + w->pos());
                QMoveEvent e(r.topLeft(), oldp);
                QApplication::sendEvent(w, &e);
            }
        }
    }

    QSize ds(qt_screen->deviceWidth(), qt_screen->deviceHeight());
    scrollRegion = qt_screen->mapFromDevice(scrollRegion, ds);
    scrollRegion.translate(-gpos.x(), -gpos.y());

    QRegion update(sr);
    update -= scrollRegion;
    if (dx) {
        int x = x2 == sr.x() ? sr.x()+w : sr.x();
        update |= QRect(x, sr.y(), QABS(dx), sr.height());
    }
    if (dy) {
        int y = y2 == sr.y() ? sr.y()+h : sr.y();
        update |= QRect(sr.x(), y, sr.width(), QABS(dy));
    }
    repaint(update);
    if (!valid_rect)
        paint_children(this, update, false);
}


int QWidget::metric(int m) const
{
    int val;
    if (m == QPaintDeviceMetrics::PdmWidth) {
        val = data->crect.width();
    } else if (m == QPaintDeviceMetrics::PdmWidthMM) {
        // 75 dpi is 3dpmm
        val = (data->crect.width()*100)/288;
    } else if (m == QPaintDeviceMetrics::PdmHeight) {
        val = data->crect.height();
    } else if (m == QPaintDeviceMetrics::PdmHeightMM) {
        val = (data->crect.height()*100)/288;
    } else if (m == QPaintDeviceMetrics::PdmDepth) {
        return qwsDisplay()->depth();
    } else if (m == QPaintDeviceMetrics::PdmDpiX || m == QPaintDeviceMetrics::PdmPhysicalDpiX) {
        return 72;
    } else if (m == QPaintDeviceMetrics::PdmDpiY || m == QPaintDeviceMetrics::PdmPhysicalDpiY) {
        return 72;
    } else {
        val = QPaintDevice::metric(m);// XXX
    }
    return val;
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
}

void QWidgetPrivate::deleteTLSysExtra()
{
}

bool QWidget::acceptDrops() const
{
    return testWState(Qt::WState_DND);
}

void QWidget::setAcceptDrops(bool on)
{
    if (!!testWState(Qt::WState_DND) != on) {
        if (1/*XXX qt_xdnd_enable(this, on)*/) {
            if (on)
                setWState(Qt::WState_DND);
            else
                clearWState(Qt::WState_DND);
        }
    }
}

void QWidget::updateOverlappingChildren() const
{
    if (data->overlapping_children != -1 || isSettingGeometry)
        return;

    QRegion r;
    QObjectList childObjects = children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject *ch = childObjects.at(i);
            if (ch->isWidgetType() && !((QWidget*)ch)->isTopLevel()) {
                QWidget *w = (QWidget *)ch;
                if (w->isVisible()) {
                    QRegion rr(w->data->req_region);
                    QRegion ir = r & rr;
                    if (!ir.isEmpty()) {
                        data->overlapping_children = 1;
                        return;
                    }
                    r |= rr;
                }
            }
    }

    data->overlapping_children = 0;
}

void QWidget::updateRequestedRegion(const QPoint &gpos)
{
    if (!isTopLevel()) {
        if (!testWState(Qt::WState_Visible) || testWState(Qt::WState_ForceHide)) {
            data->req_region = QRegion();
        } else {
            data->req_region = QRect(gpos,data->crect.size());
            if (d->extra && !d->extra->mask.isEmpty()) {
                QRegion maskr = d->extra->mask;
                maskr.translate(gpos.x(), gpos.y());
                data->req_region &= maskr;
            }
            data->req_region = qt_screen->mapToDevice(data->req_region, QSize(qt_screen->width(), qt_screen->height()));
        }
    }

    QObjectList childObjects = children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject *ch = childObjects.at(i);
            if (ch->isWidgetType() && !((QWidget*)ch)->isTopLevel()) {
                QWidget *w = static_cast<QWidget *>(ch);
                w->updateRequestedRegion(gpos + w->pos());
            }
    }

}

QRegion QWidget::requestedRegion() const
{
    return data->req_region;
}

void QWidget::setChildrenAllocatedDirty()
{
    QObjectList childObjects = children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject *ch = childObjects.at(i);
        if (ch->isWidgetType()) {
            static_cast<QWidget *>(ch)->data->alloc_region_dirty = true;
        }
    }
}

void QWidget::setChildrenAllocatedDirty(const QRegion &r, const QWidget *dirty)
{
    QObjectList childObjects = children();
    for (int i = 0; i < childObjects.size(); ++i) {
        QObject *ch = childObjects.at(i);
        if (ch->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(ch);
            if (r.boundingRect().intersects(w->geometry()))
                w->data->alloc_region_dirty = true;
            if (w == dirty)
                break;
        }
    }
}

// check my hierarchy for dirty allocated regions
bool QWidget::isAllocatedRegionDirty() const
{
    if (isTopLevel())
        return false;

    if (data->alloc_region_dirty)
        return true;

    return parentWidget()->isAllocatedRegionDirty();
}

inline bool QRect::intersects(const QRect &r) const
{
    return (qMax(x1, r.x1) <= qMin(x2, r.x2) &&
             qMax(y1, r.y1) <= qMin(y2, r.y2));
}

QRegion QWidget::allocatedRegion() const
{
    if (isVisible()) {
        if (isTopLevel()) {
            return data->alloc_region;
        } else {
            if (isAllocatedRegionDirty()) {
                QRegion r(data->req_region);
                r &= parentWidget()->allocatedRegion();
                parentWidget()->updateOverlappingChildren();
                if (parentWidget()->data->overlapping_children) {
                    QObjectList siblings = parentWidget()->children();
                    bool clip=false;
                    for (int i = 0; i < siblings.size(); ++i) {
                        QObject *ch = siblings.at(i);
                        if (ch->isWidgetType()) {
                            QWidget *w = static_cast<QWidget*>(ch);
                            if (w == this)
                                clip=true;
                            else if (clip && !w->isTopLevel() && w->isVisible()) {
                                if (w->geometry().intersects(geometry()))
                                        r -= w->data->req_region;
                            }
                        }
                    }

                }

                // if I'm dirty, so are my chlidren.
                QObjectList childObjects = children();
                for (int i = 0; i < childObjects.size(); ++i) {
                    QObject *ch = childObjects.at(i);
                    if (ch->isWidgetType()) {
                        QWidget *w = static_cast<QWidget *>(ch);
                        if (!w->isTopLevel())
                            w->data->alloc_region_dirty = true;
                    }
                }

                data->alloc_region = r;
                data->alloc_region_dirty = false;
                data->paintable_region_dirty = true;
            }
            return data->alloc_region;
        }
    } else {
        return QRegion();
    }
}

QRegion QWidget::paintableRegion() const
{
    if (isVisible()) {
        if (data->paintable_region_dirty || isAllocatedRegionDirty()) {
            data->paintable_region = allocatedRegion();
            QObjectList childObjects = children();
            for (int i = 0; i < childObjects.size(); ++i) {
                QObject *ch = childObjects.at(i);
                if (ch->isWidgetType()) {
                    QWidget *w = static_cast<QWidget *>(ch);
                    if (!w->isTopLevel() && w->isVisible())
                        data->paintable_region -= w->data->req_region;
                }
            }

            data->paintable_region_dirty = false;
#ifndef QT_NO_CURSOR
            // The change in paintable region may have result in the
            // cursor now being within my region.
            updateCursor(data->paintable_region);
#endif
        }
        if (!isTopLevel())
            return data->paintable_region;
        else {
            QRegion r(data->paintable_region);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra)
                r += d->extra->topextra->decor_allocated_region;
#endif
            return r;
        }
    }

    return QRegion();
}

void QWidget::setMask(const QRegion& region)
{
    data->alloc_region_dirty = true;

    d->createExtra();

    if (region.isEmpty() && d->extra->mask.isEmpty())
        return;

    d->extra->mask = region;

    if (isTopLevel()) {
        if (!region.isEmpty()) {
            data->req_region = d->extra->mask;
            data->req_region.translate(data->crect.x(),data->crect.y()); //###expensive?
            data->req_region &= data->crect; //??? this is optional
        } else
            data->req_region = QRegion(data->crect);
        data->req_region = qt_screen->mapToDevice(data->req_region, QSize(qt_screen->width(), qt_screen->height()));
    }
    if (isVisible()) {
        if (isTopLevel()) {
            QRegion rgn(data->req_region);
#ifndef QT_NO_QWS_MANAGER
            if (d->extra && d->extra->topextra && d->extra->topextra->qwsManager) {
                QRegion wmr = d->extra->topextra->qwsManager->region();
                wmr = qt_screen->mapToDevice(wmr, QSize(qt_screen->width(), qt_screen->height()));
                rgn += wmr;
            }
#endif
            qwsDisplay()->requestRegion(winId(), rgn);
        } else {
            updateRequestedRegion(mapToGlobal(QPoint(0,0)));
            parentWidget()->data->paintable_region_dirty = true;
            parentWidget()->repaint(geometry());
            paint_children(parentWidget(),geometry(),true);
        }
    }
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

/*!
    \internal
*/
unsigned char * QWidget::scanLine(int i) const
{
    // Should add widget x() here, maybe
    unsigned char * base=qwsDisplay()->frameBuffer();
    if(base)
        base+=i*bytesPerLine();
    return base;
}

/*!
    \internal
*/
int QWidget::bytesPerLine() const
{
    return qt_screen->linestep();
}

void QWidget::resetInputContext()
{
#ifndef QT_NO_QWS_IM
    QInputContext::reset();
#endif
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this;

    if(!isVisible() || isDesktop()) {
        that->data->fstrut_dirty = isVisible();
        return;
    }

    //FIXME: need to fill in frame strut info
}

#ifndef QT_NO_CURSOR
void QWidget::updateCursor(const QRegion &r) const
{
    if (qt_last_x && (!QWidget::mouseGrabber() || QWidget::mouseGrabber() == this) &&
            qt_last_cursor != (WId)cursor().handle() && !qws_overrideCursor) {
        QSize s(qt_screen->width(), qt_screen->height());
        QPoint pos = qt_screen->mapToDevice(QPoint(*qt_last_x, *qt_last_y), s);
        if (r.contains(pos))
            qwsDisplay()->selectCursor((QWidget*)this, (unsigned int)cursor().handle());
    }
}
#endif


void QWidget::setWindowOpacity(double)
{
}

double QWidget::windowOpacity() const
{
    return 1.0;
}

static QSingleCleanupHandler<QWSPaintEngine> qt_paintengine_cleanup_handler;
static QWSPaintEngine *qt_widget_paintengine = 0;
/*!
    Returns the widget's paint engine. (This defaults to the
    QQWSPaintEngine.)
*/
QPaintEngine *QWidget::paintEngine() const
{
    if (!qt_widget_paintengine) {
        qt_widget_paintengine = new QWSPaintEngine();
        qt_paintengine_cleanup_handler.set(&qt_widget_paintengine);
    }
    if (qt_widget_paintengine->isActive()) {
        QPaintEngine *engine = new QWSPaintEngine();
        engine->setAutoDestruct(true);
        return engine;
    }
    return qt_widget_paintengine;
}
