/****************************************************************************
**
** Implementation of QPaintDevice class for X11.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qpaintengine_x11.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include <private/qt_x11_p.h>
#include "qx11info_x11.h"

/*!
    \class QPaintDevice qpaintdevice.h
    \brief The QPaintDevice class is the base class of objects that
    can be painted.

    \ingroup multimedia

    A paint device is an abstraction of a two-dimensional space that
    can be drawn using a QPainter. The drawing capabilities are
    implemented by the subclasses QWidget, QPixmap, QPicture and
    QPrinter.

    The default coordinate system of a paint device has its origin
    located at the top-left position. X increases to the right and Y
    increases downward. The unit is one pixel. There are several ways
    to set up a user-defined coordinate system using the painter, for
    example, using QPainter::setWorldMatrix().

    Example (draw on a paint device):
    \code
    void MyWidget::paintEvent(QPaintEvent *)
    {
        QPainter p;                       // our painter
        p.begin(this);                  // start painting the widget
        p.setPen(red);                  // red outline
        p.setBrush(yellow);             // yellow fill
        p.drawEllipse(10, 20, 100,100); // 100x100 ellipse at position (10, 20)
        p.end();                          // painting done
    }
    \endcode

    The bit block transfer is an extremely useful operation for
    copying pixels from one paint device to another (or to itself). It
    is implemented as the global function bitBlt().

    Example (scroll widget contents 10 pixels to the right):
    \code
    bitBlt(myWidget, 10, 0, myWidget);
    \endcode

    \warning Qt requires that a QApplication object exists before
    any paint devices can be created. Paint devices access window
    system resources, and these resources are not initialized before
    an application object is created.
*/

/*!
    Constructs a paint device with internal flags \a devflags. This
    constructor can be invoked only from QPaintDevice subclasses.
*/

QPaintDevice::QPaintDevice(uint devflags)
{
    if (!qApp) {                                // global constructor
        qFatal("QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice");
        return;
    }
    devFlags = devflags;
    painters = 0;
}

/*!
    Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
    if (paintingActive())
        qWarning("QPaintDevice: Cannot destroy paint device that is being "
                  "painted");
}

/*!
    \fn int QPaintDevice::devType() const

    \internal

    Returns the device type identifier, which is \c QInternal::Widget
    if the device is a QWidget, \c QInternal::Pixmap if it's a
    QPixmap, \c QInternal::Printer if it's a QPrinter, \c
    QInternal::Picture if it's a QPicture or \c
    QInternal::UndefinedDevice in other cases (which should never
    happen).
*/

/*!
    \fn bool QPaintDevice::isExtDev() const

    Returns true if the device is an external paint device; otherwise
    returns false.

    External paint devices cannot be bitBlt()'ed from. QPicture and
    QPrinter are external paint devices.
*/


/*!
    \fn bool QPaintDevice::paintingActive() const

    Returns true if the device is being painted, i.e. someone has
    called QPainter::begin() but not yet called QPainter::end() for
    this device; otherwise returns false.

    \sa QPainter::isActive()
*/

/*! \internal

    Returns the X11 Drawable of the paint device. 0 is returned if it
    can't be obtained.
*/

Drawable qt_x11Handle(const QPaintDevice *pd)
{
    Q_ASSERT(pd);
    if (pd->devType() == QInternal::Widget)
        return static_cast<const QWidget *>(pd)->handle();
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(pd)->handle();
    return 0;
}

/*!
    Returns the QX11Info structure for the \a pd paint device. 0 is
    returned if it can't be obtained.
*/
QX11Info *qt_x11Info(const QPaintDevice *pd)
{
    Q_ASSERT(pd);
    if (pd->devType() == QInternal::Widget)
        return static_cast<const QWidget *>(pd)->x11Info();
    else if (pd->devType() == QInternal::Pixmap)
        return static_cast<const QPixmap *>(pd)->x11Info();
    return 0;
}

/*!
    \internal

    Internal virtual function that returns paint device metrics.

    Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric(int) const
{
    qWarning("QPaintDevice::metrics: Device has no metric information");
    return 0;
}

//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//

static bool      init_mask_gc = false;
static const int max_mask_gcs = 11;                // suitable for hashing

struct mask_gc {
    GC        gc;
    int mask_no;
};

static mask_gc gc_vec[max_mask_gcs];


static void cleanup_mask_gc()
{
    Display *dpy = QX11Info::appDisplay();
    init_mask_gc = false;
    for (int i=0; i<max_mask_gcs; i++) {
        if (gc_vec[i].gc)
            XFreeGC(dpy, gc_vec[i].gc);
    }
}

static GC cache_mask_gc(Display *dpy, Drawable hd, int mask_no, Pixmap mask)
{
    if (!init_mask_gc) {                        // first time initialization
        init_mask_gc = true;
        qAddPostRoutine(cleanup_mask_gc);
        for (int i=0; i<max_mask_gcs; i++)
            gc_vec[i].gc = 0;
    }
    mask_gc *p = &gc_vec[mask_no % max_mask_gcs];
    if (!p->gc || p->mask_no != mask_no) {        // not a perfect match
        if (!p->gc) {                                // no GC
            p->gc = XCreateGC(dpy, hd, 0, 0);
            XSetGraphicsExposures(dpy, p->gc, False);
        }
        XSetClipMask(dpy, p->gc, mask);
        p->mask_no = mask_no;
    }
    return p->gc;
}

/*!
    \relates QPaintDevice

    Copies a block of pixels from \a src to \a dst. \a sx, \a sy
    is the top-left pixel in \a src (0, 0) by default, \a dx, \a dy is
    the top-left position in \a dst and \a sw, \a sh is the size of
    the copied block (all of \a src by default).

    If \a ignoreMask is false (the default) and \a src is a
    masked QPixmap, the entire blit is masked by \a{src}->mask().

    If \a src, \a dst, \a sw or \a sh is 0, bitBlt() does nothing. If
    \a sw or \a sh is negative bitBlt() copies starting at \a sx (and
    respectively, \a sy) and ending at the right end (respectively,
    bottom) of \a src.

    \a src must be a QWidget or QPixmap. You cannot blit from a
    QPrinter, for example. bitBlt() does nothing if you attempt to
    blit from an unsupported device.

    bitBlt() does nothing if \a src has a greater depth than \e dst.
    If you need to for example, draw a 24-bit pixmap on an 8-bit
    widget, you must use drawPixmap().
*/

void bitBlt(QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx, int sy, int sw, int sh,
             bool ignoreMask)
{
    if (!src || !dst) {
        Q_ASSERT(src != 0);
        Q_ASSERT(dst != 0);
        return;
    }
    if (!qt_x11Handle(src) || src->isExtDev())
        return;

    QPoint redirection_offset;
    const QPaintDevice *redirected = QPainter::redirected(dst, &redirection_offset);
    if (redirected) {
        dst = const_cast<QPaintDevice*>(redirected);
        dx -= redirection_offset.x();
        dy -= redirection_offset.y();
    }

    int ts = src->devType();                        // from device type
    int td = dst->devType();                        // to device type

    QX11Info *src_xf = qt_x11Info(src),
	     *dst_xf = qt_x11Info(dst);

    Q_ASSERT(src_xf != 0 && dst_xf != 0);

    Display *dpy = src_xf->display();

    if (sw <= 0) {                                // special width
        if (sw < 0)
            sw = src->metric(QPaintDeviceMetrics::PdmWidth) - sx;
        else
            return;
    }
    if (sh <= 0) {                                // special height
        if (sh < 0)
            sh = src->metric(QPaintDeviceMetrics::PdmHeight) - sy;
        else
            return;
    }

#if 0 // ### port
    if (dst->paintingActive() && dst->isExtDev()) {
        QPixmap *pm;                                // output to picture/printer
        bool         tmp_pm = true;
        if (ts == QInternal::Pixmap) {
            pm = (QPixmap*)src;
            if (sx != 0 || sy != 0 ||
                 sw != pm->width() || sh != pm->height() || ignoreMask) {
                QPixmap *tmp = new QPixmap(sw, sh, pm->depth());
                bitBlt(tmp, 0, 0, pm, sx, sy, sw, sh, Qt::CopyROP, true);
                if (pm->mask() && !ignoreMask) {
                    QBitmap mask(sw, sh);
                    bitBlt(&mask, 0, 0, pm->mask(), sx, sy, sw, sh,
                            Qt::CopyROP, true);
                    tmp->setMask(mask);
                }
                pm = tmp;
            } else {
                tmp_pm = false;
            }
        } else if (ts == QInternal::Widget) {// bitBlt to temp pixmap
            pm = new QPixmap(sw, sh);
            bitBlt(pm, 0, 0, src, sx, sy, sw, sh);
        } else {
            qWarning("bitBlt: Cannot bitBlt from device");
            return;
        }
        QPDevCmdParam param[3];
        QPoint p(dx,dy);
        param[0].point        = &p;
        param[1].pixmap = pm;
        dst->cmd(QPaintDevice::PdcDrawPixmap, 0, param);
        if (tmp_pm)
            delete pm;
        return;
    }
#endif

    switch (ts) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:                        // OK, can blt from these
        break;
    default:
        qWarning("bitBlt: Cannot bitBlt from device type %x", ts);
        return;
    }
    switch (td) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:                        // OK, can blt to these
        break;
    default:
        qWarning("bitBlt: Cannot bitBlt to device type %x", td);
        return;
    }

    if (qt_x11Handle(dst) == 0) {
        qWarning("bitBlt: Cannot bitBlt to device");
        return;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = false;
    bool graphics_exposure = false;
    QPixmap *src_pm;
    QBitmap *mask;

    if (ts == QInternal::Pixmap) {
        src_pm = (QPixmap*)src;
        if (src_pm->x11Info()->screen() != dst_xf->screen())
            src_pm->x11SetScreen(dst_xf->screen());
        mono_src = src_pm->depth() == 1;
        mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
        src_pm = 0;
        mono_src = false;
        mask = 0;
        include_inferiors = ((QWidget*)src)->testAttribute(Qt::WA_PaintUnclipped);
        graphics_exposure = td == QInternal::Widget;
    }
    if (td == QInternal::Pixmap) {
        if (dst_xf->screen() != src_xf->screen())
            ((QPixmap*)dst)->x11SetScreen(src_xf->screen());
        mono_dst = ((QPixmap*)dst)->depth() == 1;
        ((QPixmap*)dst)->detach();                // changes shared pixmap
    } else {
        mono_dst = false;
        include_inferiors = include_inferiors ||
                            ((QWidget*)dst)->testAttribute(Qt::WA_PaintUnclipped);
    }

    if (mono_dst && !mono_src) {        // dest is 1-bit pixmap, source is not
        qWarning("bitBlt: Incompatible destination pixmap");
        return;
    }

#ifndef QT_NO_XRENDER
    if (src_pm && !mono_src && src_pm->data->alphapm && !ignoreMask) {
        // use RENDER to do the blit
        QPixmap *alpha = src_pm->data->alphapm;
	Qt::HANDLE src_pict, dst_pict;
	if (src->devType() == QInternal::Widget)
	    src_pict = static_cast<const QWidget *>(src)->xftPictureHandle();
	else
	    src_pict = static_cast<const QPixmap *>(src)->xftPictureHandle();
	if (dst->devType() == QInternal::Widget)
	    dst_pict = static_cast<const QWidget *>(dst)->xftPictureHandle();
	else
	    dst_pict = static_cast<const QPixmap *>(dst)->xftPictureHandle();
        if (dst_pict && src_pict && alpha->xftPictureHandle()) {
            XRenderPictureAttributes pattr;
            ulong picmask = 0;
            if (include_inferiors) {
                pattr.subwindow_mode = IncludeInferiors;
                picmask |= CPSubwindowMode;
            }
            if (graphics_exposure) {
                pattr.graphics_exposures = true;
                picmask |= CPGraphicsExposure;
            }
            if (picmask)
                XRenderChangePicture(dpy, dst_pict, picmask, &pattr);
            XRenderComposite(dpy, PictOpOver, src_pict, alpha->xftPictureHandle(), dst_pict,
                             sx, sy, sx, sy, dx, dy, sw, sh);
            // restore attributes
            pattr.subwindow_mode = ClipByChildren;
            pattr.graphics_exposures = false;
            if (picmask)
                XRenderChangePicture(dpy, dst_pict, picmask, &pattr);
            return;
        }
    }
#endif

    GC gc;

    if (mask && !mono_src) {                        // fast masked blt
        bool temp_gc = false;
        if (mask->data->maskgc) {
            gc = (GC)mask->data->maskgc;        // we have a premade mask GC
        } else {
            if (false && src_pm->optimization() == QPixmap::NormalOptim) { // #### cache disabled
                // Compete for the global cache
                gc = cache_mask_gc(dpy, qt_x11Handle(dst),
                                    mask->data->ser_no,
                                    mask->handle());
            } else {
                // Create a new mask GC. If BestOptim, we store the mask GC
                // with the mask (not at the pixmap). This way, many pixmaps
                // which have a common mask will be optimized at no extra cost.
                gc = XCreateGC(dpy, qt_x11Handle(dst), 0, 0);
                XSetGraphicsExposures(dpy, gc, False);
                XSetClipMask(dpy, gc, mask->handle());
                if (src_pm->optimization() == QPixmap::BestOptim) {
                    mask->data->maskgc = gc;
                } else {
                    temp_gc = true;
                }
            }
        }
        XSetClipOrigin(dpy, gc, dx-sx, dy-sy);
        if (include_inferiors) {
            XSetSubwindowMode(dpy, gc, IncludeInferiors);
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
            XSetSubwindowMode(dpy, gc, ClipByChildren);
        } else {
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
        }

        if (temp_gc)                                // delete temporary GC
            XFreeGC(dpy, gc);
        return;
    }

    gc = qt_xget_temp_gc(dst_xf->screen(), mono_dst);                // get a reusable GC


    if (mono_src && mono_dst && src == dst) { // dst and src are the same bitmap
        XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
    } else if (mono_src) {                        // src is bitmap
        XGCValues gcvals;
        ulong          valmask = GCBackground | GCForeground | GCFillStyle |
                            GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
        if (td == QInternal::Widget) {        // set GC colors
            QWidget *w = (QWidget *)dst;
            gcvals.background = w->palette().color(w->backgroundRole()).pixel(dst_xf->screen());
            gcvals.foreground = w->palette().color(w->foregroundRole()).pixel(dst_xf->screen());
            if (include_inferiors) {
                valmask |= GCSubwindowMode;
                gcvals.subwindow_mode = IncludeInferiors;
            }
        } else if (mono_dst) {
            gcvals.background = 0;
            gcvals.foreground = 1;
        } else {
            gcvals.background = QColor(Qt::white).pixel(dst_xf->screen());
            gcvals.foreground = QColor(Qt::black).pixel(dst_xf->screen());
        }

        gcvals.fill_style  = FillOpaqueStippled;
        gcvals.stipple = qt_x11Handle(src);
        gcvals.ts_x_origin = dx - sx;
        gcvals.ts_y_origin = dy - sy;

        bool clipmask = false;
        if (mask) {
            if (((QPixmap*)src)->data->selfmask) {
                gcvals.fill_style = FillStippled;
            } else {
                XSetClipMask(dpy, gc, mask->handle());
                XSetClipOrigin(dpy, gc, dx-sx, dy-sy);
                clipmask = true;
            }
        }

        XChangeGC(dpy, gc, valmask, &gcvals);
        XFillRectangle(dpy, qt_x11Handle(dst), gc, dx, dy, sw, sh);

        valmask = GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
        gcvals.fill_style  = FillSolid;
        gcvals.ts_x_origin = 0;
        gcvals.ts_y_origin = 0;
        if (include_inferiors) {
            valmask |= GCSubwindowMode;
            gcvals.subwindow_mode = ClipByChildren;
        }
        XChangeGC(dpy, gc, valmask, &gcvals);

        if (clipmask) {
            XSetClipOrigin(dpy, gc, 0, 0);
            XSetClipMask(dpy, gc, XNone);
        }

    } else {                                        // src is pixmap/widget

        if (graphics_exposure)                // widget to widget
            XSetGraphicsExposures(dpy, gc, True);
        if (include_inferiors) {
            XSetSubwindowMode(dpy, gc, IncludeInferiors);
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
            XSetSubwindowMode(dpy, gc, ClipByChildren);
        } else {
            XCopyArea(dpy, qt_x11Handle(src), qt_x11Handle(dst), gc, sx, sy, sw, sh, dx, dy);
        }
        if (graphics_exposure)                // reset graphics exposure
            XSetGraphicsExposures(dpy, gc, False);
    }
}


/*!
    \relates QPaintDevice

    \fn void bitBlt(QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr)
    \overload

    Overloaded bitBlt() with the destination point \a dp and source
    rectangle \a sr.
*/

#ifdef QT_COMPAT

Display *QPaintDevice::x11Display() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->display();
    return QX11Info::appDisplay();
}

int QPaintDevice::x11Screen() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->screen();
    return QX11Info::appScreen();
}

void *QPaintDevice::x11Visual() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->visual();
    return QX11Info::appVisual();
}

int QPaintDevice::x11Depth() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
        return info->depth();
    return QX11Info::appDepth();
}

int QPaintDevice::x11Cells() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->cells();
    return QX11Info::appCells();
}

Qt::HANDLE QPaintDevice::x11Colormap() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->colormap();
    return QX11Info::appColormap();
}

bool QPaintDevice::x11DefaultColormap() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->defaultColormap();
    return QX11Info::appDefaultColormap();
}

bool QPaintDevice::x11DefaultVisual() const
{
    QX11Info *info = qt_x11Info(this);
    if (info)
	return info->defaultVisual();
    return QX11Info::appDefaultVisual();
}

void *QPaintDevice::x11AppVisual(int screen)
{ return QX11Info::appVisual(screen); }

Qt::HANDLE QPaintDevice::x11AppColormap(int screen)
{ return QX11Info::appColormap(screen); }

Display *QPaintDevice::x11AppDisplay()
{ return QX11Info::appDisplay(); }

int QPaintDevice::x11AppScreen()
{ return QX11Info::appScreen(); }

int QPaintDevice::x11AppDepth(int screen)
{ return QX11Info::appDepth(screen); }

int QPaintDevice::x11AppCells(int screen)
{ return QX11Info::appCells(screen); }

Qt::HANDLE QPaintDevice::x11AppRootWindow(int screen)
{ return QX11Info::appRootWindow(screen); }

bool QPaintDevice::x11AppDefaultColormap(int screen)
{ return QX11Info::appDefaultColormap(screen); }

bool QPaintDevice::x11AppDefaultVisual(int screen)
{ return QX11Info::appDefaultVisual(screen); }

/*!
    Sets the value returned by x11AppDpiX() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiY()
*/
void QPaintDevice::x11SetAppDpiX(int dpi, int screen)
{
    QX11Info::setAppDpiX(dpi, screen);
}

/*!
    Sets the value returned by x11AppDpiY() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiX()
*/
void QPaintDevice::x11SetAppDpiY(int dpi, int screen)
{
    QX11Info::setAppDpiY(dpi, screen);
}


/*!
    Returns the horizontal DPI of the X display (X11 only) for screen
    \a screen. Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiY(), x11SetAppDpiX(), QPaintDeviceMetrics::logicalDpiX()
*/
int QPaintDevice::x11AppDpiX(int screen)
{
    return QX11Info::appDpiX(screen);
}

/*!
    Returns the vertical DPI of the X11 display (X11 only) for screen
    \a screen.  Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
int QPaintDevice::x11AppDpiY(int screen)
{
    return QX11Info::appDpiY(screen);
}
#endif
