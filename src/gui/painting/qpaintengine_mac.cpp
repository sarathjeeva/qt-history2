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

#include <qbitmap.h>
#include <qpaintdevice.h>
#include <qpaintdevicemetrics.h>
#include <private/qpaintengine_mac_p.h>
#include <qpainterpath.h>
#include <qpixmapcache.h>
#include <private/qprintengine_mac_p.h>
#include <qprinter.h>
#include <qstack.h>
#include <qtextcodec.h>
#include <qtextcodec.h>
#include <qwidget.h>

#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qpainter_p.h>
#include <private/qpainterpath_p.h>
#include <private/qpixmap_p.h>
#include <private/qt_mac_p.h>
#include <private/qwidget_p.h>

#include <string.h>

#define d d_func()
#define q q_func()

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

/*****************************************************************************
  External functions
 *****************************************************************************/
extern QPoint posInWindow(const QWidget *w); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern GrafPtr qt_macQDHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGContextRef qt_macCreateCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGImageRef qt_mac_create_cgimage(const QPixmap &, Qt::PixmapDrawingMode, bool); //qpixmap_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern const uchar *qt_patternForBrush(int, bool); //qbrush.cpp
extern QPixmap qt_pixmapForBrush(int, bool); //qbrush.cpp

// paintevent magic to provide Windows semantics on Qt/Mac
class paintevent_item
{
    QPaintDevice* dev;
    QRegion clipRegion;
public:
    paintevent_item(QPaintDevice *dv, QRegion r) : dev(dv), clipRegion(r) { }
    inline bool operator==(const QPaintDevice *rhs) const { return rhs == dev; }
    inline bool operator!=(const QPaintDevice *rhs) const { return !(this->operator==(rhs)); }
    inline QPaintDevice *device() const { return dev; }
    inline QRegion region() const { return clipRegion; }
};
QStack<paintevent_item*> paintevents;
static paintevent_item *qt_mac_get_paintevent() { return paintevents.isEmpty() ? 0 : paintevents.top(); }

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region)
{
    QRegion r = region;
    if(dev && dev->devType() == QInternal::Widget) {
        QWidget *w = (QWidget *)dev;
        QPoint mp(posInWindow(w));
        r.translate(mp.x(), mp.y());
    }
    if(paintevent_item *curr = qt_mac_get_paintevent()) {
        if(curr->device() == dev)
            r &= curr->region();
    }
    paintevents.push(new paintevent_item(dev, r));
}

void qt_clear_paintevent_clipping(QPaintDevice *dev)
{
    if(paintevents.isEmpty() || !((*paintevents.top()) == dev)) {
        qWarning("Qt: internal: WH0A, qt_clear_paintevent_clipping mismatch.");
        return;
    }
    delete paintevents.pop();
}

//Implemented for qt_mac_p.h
QMacCGContext::QMacCGContext(QPainter *p)
{
    QPaintEngine *pe = p->paintEngine();
    if(pe->type() == QPaintEngine::MacPrinter)
        pe = static_cast<QMacPrintEngine*>(pe)->paintEngine();
    Q_ASSERT(pe->type() == QPaintEngine::CoreGraphics);
    pe->syncState();
    context = static_cast<QCoreGraphicsPaintEngine*>(pe)->handle();
    CGContextRetain(context);
}

/*****************************************************************************
  QQuickDrawPaintEngine member functions
 *****************************************************************************/

inline static QPaintEngine::PaintEngineFeatures qt_mac_qd_features()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::UsesFontEngine|QPaintEngine::PixmapScale
        |QPaintEngine::AlphaPixmap
        );
}

QQuickDrawPaintEngine::QQuickDrawPaintEngine()
    : QPaintEngine(*(new QQuickDrawPaintEnginePrivate), qt_mac_qd_features())
{
}

QQuickDrawPaintEngine::QQuickDrawPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures devcaps)
    : QPaintEngine(dptr, (devcaps ? devcaps : qt_mac_qd_features()))
{
}

QQuickDrawPaintEngine::~QQuickDrawPaintEngine()
{
}

bool
QQuickDrawPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QQuickDrawPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    d->saved = new QMacSavedPortInfo;     //save the gworld now, we'll reset it in end()
    d->pdev = pdev;
    setActive(true);
    assignf(IsActive | DirtyFont);

    d->clip.serial = 0;
    d->paintevent = 0;
    d->clip.dirty = false;
    d->offx = d->offy = 0;
    bool unclipped = false;
    if(d->pdev->devType() == QInternal::Pixmap) {
        static_cast<QPixmap*>(d->pdev)->detach();  //detach it
    } else if(d->pdev->devType() == QInternal::Widget) {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        { //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
        bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if(!d->locked) {
            LockPortBits(GetWindowPort(qt_mac_window_for(w)));
            d->locked = true;
        }

        if(w->isDesktop()) {
            if(!unclipped)
                qWarning("QQuickDrawPaintEngine::begin: Does not support clipped desktop on MacOSX");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QQuickDrawPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = static_cast<QPixmap*>(d->pdev);
        if(pm->isNull()) {
            qWarning("QQuickDrawPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }
    d->unclipped = unclipped;
    if(type() != CoreGraphics)
        setupQDPort(true); //force setting paint device, this does unclipped fu
    return true;
}

bool
QQuickDrawPaintEngine::end()
{
    setActive(false);

    if(d->locked) {
        if(d->pdev->devType() == QInternal::Widget)
            UnlockPortBits(GetWindowPort(qt_mac_window_for(static_cast<QWidget*>(d->pdev))));
        d->locked = false;
    }

    delete d->saved;
    d->saved = 0;
    if(d->pdev->devType() == QInternal::Widget && static_cast<QWidget*>(d->pdev)->isDesktop())
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));

    d->pdev = 0;
    return true;
}

void
QQuickDrawPaintEngine::updatePen(const QPen &pen)
{
    d->current.pen = pen;
}


void
QQuickDrawPaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    d->current.brush = brush;
    d->current.bg.origin = origin;
}

void
QQuickDrawPaintEngine::updateFont(const QFont &)
{
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QQuickDrawPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = bgBrush;
}

void
QQuickDrawPaintEngine::updateMatrix(const QMatrix &)
{
}

void
QQuickDrawPaintEngine::setClippedRegionInternal(QRegion *rgn)
{
    if(rgn) {
        d->current.clip = *rgn;
        setf(ClipOn);
    } else {
        d->current.clip = QRegion();
        clearf(ClipOn);
    }
    d->clip.dirty = 1;
}

void
QQuickDrawPaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        setClippedRegionInternal(0);
    } else {
        QRegion clip = region;
        if(testf(ClipOn)) {
            if(op == Qt::IntersectClip)
                clip = d->current.clip.intersect(clip);
            else if(op == Qt::UniteClip)
                clip = d->current.clip.unite(clip);
        }
        setClippedRegionInternal(&clip);
    }
}

void
QQuickDrawPaintEngine::drawLine(const QLineF &line)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;
    setupQDPen();
    MoveTo(qRound(line.startX())+d->offx,qRound(line.startY())+d->offy);
    LineTo(qRound(line.endX())+d->offx,qRound(line.endY())+d->offy);
}

void
QQuickDrawPaintEngine::drawRect(const QRectF &r)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect rect;
    SetRect(&rect, qRound(r.x())+d->offx, qRound(r.y())+d->offy,
            qRound(r.x() + r.width())+d->offx, qRound(r.y() + r.height())+d->offy);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintRect(&rect);
        } else {
            QPixmap pm;
            if(d->brush_style_pix) {
                pm = *d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintRect(&rect);
                }
            } else {
                pm = d->current.brush.texture();
            }
            if(!pm.isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r.toRect());
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, pm, QPointF(r.x(), r.y()) - d->current.bg.origin, Qt::ComposePixmap);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameRect(&rect);
    }
}

void
QQuickDrawPaintEngine::drawPoint(const QPointF &pt)
{
    Q_ASSERT(isActive());
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        MoveTo(qRound(pt.x()) + d->offx, qRound(pt.y()) + d->offy);
        Line(0, 0);
    }
}

void
QQuickDrawPaintEngine::drawPoints(const QPolygon &p)
{
    Q_ASSERT(isActive());

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        QPointArray pa = p.toPointArray();
        for(int i=0; i < pa.size(); i++) {
            MoveTo(pa[i].x()+d->offx, pa[i].y()+d->offy);
            Line(0, 0);
        }
    }
}

void
QQuickDrawPaintEngine::drawEllipse(const QRectF &r)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect mac_r;
    SetRect(&mac_r, qRound(r.x()) + d->offx, qRound(r.y()) + d->offy,
            qRound(r.x() + r.width()) + d->offx, qRound(r.y() + r.height()) + d->offy);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintOval(&mac_r);
        } else {
            QPixmap pm = 0;
            if(d->brush_style_pix) {
                pm = *d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintOval(&mac_r);
                }
            } else {
                pm = d->current.brush.texture();
            }
            if(!pm.isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r.toRect(), QRegion::Ellipse);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, pm, QPointF(r.x(), r.y()) - d->current.bg.origin, Qt::ComposePixmap);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameOval(&mac_r);
    }
}

void
QQuickDrawPaintEngine::drawLines(const QList<QLineF> &lines)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    setupQDPen();
    for(int i = 0; i < lines.size(); i++) {
        const QPointF start = lines[i].start(), end = lines[i].end();
        MoveTo(qRound(start.x()) + d->offx, qRound(start.y()) + d->offy);
        LineTo(qRound(end.x()) + d->offx, qRound(end.y()) + d->offy);
    }
}

void
QQuickDrawPaintEngine::drawPolygon(const QPolygon &p, PolygonDrawMode mode)
{
    Q_ASSERT(isActive());
    if (mode == PolylineMode) {
        int x1, y1, x2, y2, xsave, ysave;
        QPointArray pa = p.toPointArray();
        if(pa.isEmpty())
            return;
        pa.point(pa.count()-2, &x1, &y1);      // last line segment
        pa.point(pa.count()-1, &x2, &y2);
        xsave = x2; ysave = y2;
        bool plot_pixel = false;
        if(x1 == x2) {                           // vertical
            if(y1 < y2)
                y2++;
            else
                y2--;
        } else if(y1 == y2) {                    // horizontal
            if(x1 < x2)
                x2++;
            else
                x2--;
        } else {
            plot_pixel = d->current.pen.style() == Qt::SolidLine; // plot last pixel
        }
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;

        setupQDPen();
        /* We draw 5000 chunks at a time because of limitations in QD */
        for(int chunk = 0; chunk < pa.count();) {
            //make a region of it
            PolyHandle poly = OpenPoly();
            MoveTo(pa[chunk].x()+d->offx, pa[chunk].y()+d->offy);
            for(int last_chunk=chunk+5000; chunk < last_chunk; chunk++) {
                if(chunk == pa.count())
                    break;
                LineTo(pa[chunk].x()+d->offx, pa[chunk].y()+d->offy);
            }
            ClosePoly();
            //now draw it
            FramePoly(poly);
            KillPoly(poly);
        }
    } else {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;

        QPointArray pa = p.toPointArray();
        PolyHandle polyHandle = OpenPoly();
        MoveTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
        for(int x = 1; x < pa.size(); x++)
            LineTo(pa[x].x()+d->offx, pa[x].y()+d->offy);
        LineTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
        ClosePoly();

        if(d->current.brush.style() != Qt::NoBrush) {
            setupQDBrush();
            if(d->current.brush.style() == Qt::SolidPattern) {
                PaintPoly(polyHandle);
            } else {
                QPixmap pm = 0;
                if(d->brush_style_pix) {
                    pm = *d->brush_style_pix;
                    if(d->current.bg.mode == Qt::OpaqueMode) {
                        ::RGBColor f;
                        f.red = d->current.bg.brush.color().red()*256;
                        f.green = d->current.bg.brush.color().green()*256;
                        f.blue = d->current.bg.brush.color().blue()*256;
                        RGBForeColor(&f);
                        PaintPoly(polyHandle);
                    }
                } else {
                    pm = d->current.brush.texture();
                }

                if(!pm.isNull()) {
                    //save the clip
                    bool clipon = testf(ClipOn);
                    QRegion clip = d->current.clip;

                    //create the region
                    QRegion newclip(pa);
                    if(clipon)
                        newclip &= clip;
                    setClippedRegionInternal(&newclip);

                    //draw the brush
                    QRect r(pa.boundingRect());
                    drawTiledPixmap(r, pm, r.topLeft() - d->current.bg.origin, Qt::ComposePixmap);

                    //restore the clip
                    setClippedRegionInternal(clipon ? &clip : 0);
                }
            }
        }
        if(d->current.pen.style() != Qt::NoPen) {
            setupQDPen();
            FramePoly(polyHandle);
        }
        KillPoly(polyHandle);
    }
}

void
QQuickDrawPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p,
				       Qt::PixmapDrawingMode mode)
{
    int yPos=qRound(r.y()), xPos, drawH, drawW, yOff=qRound(p.y()), xOff;
    int rBottom = qRound(r.y() + r.height());
    int rRight = qRound(r.x() + r.width());
    while(yPos < rBottom) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if(yPos + drawH > rBottom)        // Cropping last row
            drawH = rBottom - yPos;
        xPos = qRound(r.x());
        xOff = qRound(p.x());
        while(xPos < rRight) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if(xPos + drawW > rRight)    // Cropping last column
                drawW = rRight - xPos;
            drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH),
                       mode);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

void
QQuickDrawPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr,
                                  Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(isActive());
    if(pixmap.isNull())
        return;

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    //setup port
    ::RGBColor f;
    if(pixmap.depth() == 1) {
        f.red = d->current.pen.color().red()*256;
        f.green = d->current.pen.color().green()*256;
        f.blue = d->current.pen.color().blue()*256;
    } else {
        f.red = f.green = f.blue = 0;
    }
    RGBForeColor(&f);
    f.red = f.green = f.blue = ~0;
    RGBBackColor(&f);

    //get pixmap bits
    const BitMap *srcbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(&pixmap));
    const QPixmap *srcmask=0;
    if(mode == Qt::ComposePixmap) {
        if(pixmap.data->alphapm)
            srcmask = pixmap.data->alphapm;
        else
            srcmask = pixmap.mask();
    }

    //get pdev bits
    const BitMap *dstbitmap=0;
    switch(d->pdev->devType()) {
    case QInternal::Widget: {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        dstbitmap = GetPortBitMapForCopyBits(GetWindowPort(qt_mac_window_for(w)));
        break; }
    case QInternal::Printer:
    case QInternal::Pixmap: {
        dstbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(d->pdev));
        break; }
    }

    //get copy mode
    short copymode = srcCopy;
    if(srcmask && srcmask->depth() > 1)
        copymode = ditherCopy;

    //do the blt
    Rect srcr;
    SetRect(&srcr, qRound(sr.x()), qRound(sr.y()),
            qRound(sr.x() + sr.width()), qRound(sr.y()+sr.height()));
    Rect dstr;
    SetRect(&dstr, d->offx + qRound(r.x()), d->offy + qRound(r.y()),
            d->offx + qRound(r.x() + r.width()),
            d->offy + qRound(r.y() + r.height()));
    if(srcmask) {
        const BitMap *maskbits = GetPortBitMapForCopyBits(qt_macQDHandle(srcmask));
        if(d->pdev->devType() == QInternal::Printer) { //can't use CopyDeepMask on a printer
            QPixmap tmppix(qRound(r.width()), qRound(r.height()), pixmap.depth());
            Rect pixr;
            SetRect(&pixr, 0, 0, qRound(r.width()), qRound(r.height()));
            const BitMap *pixbits = GetPortBitMapForCopyBits((GWorldPtr)tmppix.handle());
            {
                QMacSavedPortInfo pi(&tmppix);
                EraseRect(&pixr);
                CopyDeepMask(srcbitmap, maskbits, pixbits, &srcr, &srcr, &pixr, copymode, 0);
            }
            setupQDPort(true);
            CopyBits(pixbits, dstbitmap, &pixr, &dstr, srcOr, 0); //use srcOr transfer, to "emulate" the mask
        } else {
            CopyDeepMask(srcbitmap, maskbits, dstbitmap, &srcr, &srcr, &dstr, copymode, 0);
        }
    } else {
        CopyBits(srcbitmap, dstbitmap, &srcr, &dstr, copymode, 0);
    }
}

void
QQuickDrawPaintEngine::initialize()
{
}

void
QQuickDrawPaintEngine::cleanup()
{
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDPen()
{
    //pen size
    int dot = d->current.pen.width();
    if(dot < 1)
        dot = 1;
    PenSize(dot, dot);

    //forecolor
    ::RGBColor f;
    f.red = d->current.pen.color().red()*256;
    f.green = d->current.pen.color().green()*256;
    f.blue = d->current.pen.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDBrush()
{
    //pattern
    delete d->brush_style_pix;
    d->brush_style_pix = 0;
    int bs = d->current.brush.style();
    if(bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        d->brush_style_pix = new QPixmap(8, 8);
        d->brush_style_pix->setMask(qt_pixmapForBrush(bs, true));
        d->brush_style_pix->fill(d->current.brush.color());
    } else if(bs == Qt::CustomPattern) {
        QPixmap texture = d->current.brush.texture();
        if(texture.isQBitmap()) {
            d->brush_style_pix = new QPixmap(texture.width(), texture.height());
            d->brush_style_pix->setMask(*((QBitmap*)&texture));
            d->brush_style_pix->fill(d->current.brush.color());
        }
    }

    //forecolor
    ::RGBColor f;
    f.red = d->current.brush.color().red()*256;
    f.green = d->current.brush.color().green()*256;
    f.blue = d->current.brush.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDFont()
{
    setupQDPen();
}

/*!
    \internal
*/
void QQuickDrawPaintEngine::setupQDPort(bool force, QPoint *off, QRegion *rgn)
{
    bool remade_clip = false;
    if(d->pdev->devType() == QInternal::Printer) {
        if(force) {
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, d->pdev->metric(QPaintDeviceMetrics::PdmWidth),
                                          d->pdev->metric(QPaintDeviceMetrics::PdmHeight));
        }
    } else if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        paintevent_item *pevent = qt_mac_get_paintevent();
        if(pevent && (*pevent) != d->pdev)
            pevent = 0;
        QWidget *w = (QWidget*)d->pdev;
        if(!(remade_clip = force)) {
            if(pevent != d->paintevent)
                remade_clip = true;
            else if(!w->isVisible())
                remade_clip = d->clip.serial;
            else
                remade_clip = (d->clip.serial != w->d_func()->clippedSerial(!d->unclipped));
        }
        if(remade_clip) {
            //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();

            if(!w->isVisible()) {
                d->clip.pdev = QRegion(0, 0, 0, 0); //make the clipped reg empty if not visible!!!
                d->clip.serial = 0;
            } else {
                d->clip.pdev = w->d_func()->clippedRegion(!d->unclipped);
                d->clip.serial = w->d_func()->clippedSerial(!d->unclipped);
            }
            if(pevent)
                d->clip.pdev &= pevent->region();
            d->paintevent = pevent;
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(force) {//clip out my bounding rect
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, pm->width(), pm->height());
        }
    }
    if(remade_clip || d->clip.dirty) {         //update clipped region
        remade_clip = true;
        if(!d->clip.pdev.isEmpty() && testf(ClipOn)) {
            d->clip.paintable = d->current.clip;
            d->clip.paintable.translate(d->offx, d->offy);
            d->clip.paintable &= d->clip.pdev;
        } else {
            d->clip.paintable = d->clip.pdev;
        }

        CGrafPtr ptr = qt_macQDHandle(d->pdev);
        if(RgnHandle rgn = d->clip.paintable.handle()) {
            QDAddRegionToDirtyRegion(ptr, rgn);
        } else {
            QRect qr = d->clip.paintable.boundingRect();
            Rect mr; SetRect(&mr, qr.x(), qr.y(), qr.right(), qr.bottom());
            QDAddRectToDirtyRegion(ptr, &mr);
        }
        d->clip.dirty = false;
    }
    { //setup the port
        QMacSavedPortInfo::setPaintDevice(d->pdev);
        if(type() != CoreGraphics)
            QMacSavedPortInfo::setClipRegion(d->clip.paintable);
    }
    if(off)
        *off = QPoint(d->offx, d->offy);
    if(rgn)
        *rgn = d->clip.paintable;
}

/*****************************************************************************
  QCoreGraphicsPaintEngine utility functions
 *****************************************************************************/

//colour conversion
inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }

//pattern handling (tiling)
struct QMacPattern {
    QMacPattern() : opaque(true), as_mask(false), image(0) { data.bytes = 0; }
    //input
    QColor background, foreground;
    bool opaque;
    bool as_mask;
    struct {
        QPixmap pixmap;
        const uchar *bytes;
    } data;
    //output
    CGImageRef image;
};
static void qt_mac_draw_pattern(void *info, CGContextRef c)
{
    QMacPattern *pat = (QMacPattern*)info;
    int w = 0, h = 0;
    if (!pat->image) {
        CGImageRef image = 0;
        if (pat->as_mask) {
            w = h = 8;
            CGDataProviderRef provider = CGDataProviderCreateWithData(0, pat->data.bytes, 64, 0);
            image = CGImageMaskCreate(w, h, 1, 1, 1, provider, 0, false);
            CGDataProviderRelease(provider);
        } else {
            w = pat->data.pixmap.width();
            h = pat->data.pixmap.height();
            image = qt_mac_create_cgimage(pat->data.pixmap, Qt::ComposePixmap,
                                          pat->data.pixmap.isQBitmap());
        }
        if(pat->opaque && CGImageIsMask(image)) {
            QPixmap tmp(w, h);
            CGRect rect = CGRectMake(0, 0, w, h);
            CGContextRef ctx = qt_macCreateCGHandle(&tmp);
            CGContextSetRGBFillColor(ctx, qt_mac_convert_color_to_cg(pat->background.red()),
                                     qt_mac_convert_color_to_cg(pat->background.green()),
                                     qt_mac_convert_color_to_cg(pat->background.blue()),
                                     qt_mac_convert_color_to_cg(pat->background.alpha()));
            CGContextFillRect(ctx, rect);
            CGContextSetRGBFillColor(ctx, qt_mac_convert_color_to_cg(pat->foreground.red()),
                                     qt_mac_convert_color_to_cg(pat->foreground.green()),
                                     qt_mac_convert_color_to_cg(pat->foreground.blue()),
                                     qt_mac_convert_color_to_cg(pat->foreground.alpha()));
            HIViewDrawCGImage(ctx, &rect, image);
            pat->image = qt_mac_create_cgimage(tmp, Qt::CopyPixmap, false);
            CGImageRelease(image);
        } else {
            pat->image = image;
        }
    } else {
        w = CGImageGetWidth(pat->image);
        h = CGImageGetHeight(pat->image);
    }
    CGRect rect = CGRectMake(0, 0, w, h);
    HIViewDrawCGImage(c, &rect, pat->image); //top left
}
static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    if(pat->image)
        CGImageRelease(pat->image);
    delete pat;
}

//gradiant callback
static void qt_mac_color_gradient_function(void *info, const float *in, float *out)
{
    QBrush *brush = static_cast<QBrush *>(info);
    const float red = qt_mac_convert_color_to_cg(brush->color().red());
    out[0] = red + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().red())-red);
    const float green = qt_mac_convert_color_to_cg(brush->color().green());
    out[1] = green + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().green())-green);
    const float blue = qt_mac_convert_color_to_cg(brush->color().blue());
    out[2] = blue + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().blue())-blue);
    const float alpha = qt_mac_convert_color_to_cg(brush->color().alpha());
    out[3] = alpha + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().alpha()) - alpha);
}

//clipping handling
static void qt_mac_clip_cg_reset(CGContextRef hd)
{
    //setup xforms
    CGAffineTransform old_xform = CGContextGetCTM(hd);
    CGContextConcatCTM(hd, CGAffineTransformInvert(old_xform));
    CGContextConcatCTM(hd, CGAffineTransformIdentity);

    //do the clip reset
    QRect qrect = QRect(0, 0, 99999, 999999);
    Rect qdr; SetRect(&qdr, qrect.left(), qrect.top(), qrect.right(),
                      qrect.bottom());
    ClipCGContextToRegion(hd, &qdr, QRegion(qrect).handle(true));

    //reset xforms
    CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
    CGContextConcatCTM(hd, old_xform);
}

static CGMutablePathRef qt_mac_compose_path(const QPainterPath &p)
{
    CGMutablePathRef ret = CGPathCreateMutable();
    QPointF startPt;
    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &elm = p.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (i > 0
                && p.elementAt(i - 1).x == startPt.x()
                && p.elementAt(i - 1).y == startPt.y())
                CGPathCloseSubpath(ret);
            startPt = QPointF(elm.x, elm.y);
            CGPathMoveToPoint(ret, 0, elm.x, elm.y);
            break;
        case QPainterPath::LineToElement:
            CGPathAddLineToPoint(ret, 0, elm.x, elm.y);
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(p.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(p.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            CGPathAddCurveToPoint(ret, 0,
                                  elm.x, elm.y,
                                  p.elementAt(i+1).x, p.elementAt(i+1).y,
                                  p.elementAt(i+2).x, p.elementAt(i+2).y);
            i+=2;
            break;
        default:
            qFatal("QCoreGraphicsPaintEngine::drawPath(), unhandled type: %d", elm.type);
            break;
        }
    }
    if (!p.isEmpty()
        && p.elementAt(p.elementCount() - 1).x == startPt.x()
        && p.elementAt(p.elementCount() - 1).y == startPt.y())
        CGPathCloseSubpath(ret);
    return ret;
}

static void qt_mac_clip_cg(CGContextRef hd, const QRegion &rgn, const QPoint *pt, CGAffineTransform *orig_xform)
{
    CGAffineTransform old_xform = CGAffineTransformIdentity;
    if(orig_xform) { //setup xforms
        old_xform = CGContextGetCTM(hd);
        CGContextConcatCTM(hd, CGAffineTransformInvert(old_xform));
        CGContextConcatCTM(hd, *orig_xform);
    }

    //do the clipping
    CGContextBeginPath(hd);
    if(rgn.isEmpty()) {
        CGContextAddRect(hd, CGRectMake(0, 0, 0, 0));
    } else {
        QVector<QRect> rects = rgn.rects();
        const int count = rects.size();
        for(int i = 0; i < count; i++) {
            const QRect &r = rects[i];
            CGRect mac_r = CGRectMake(r.x(), r.y(), r.width(), r.height());
            if(pt) {
                mac_r.origin.x -= pt->x();
                mac_r.origin.y -= pt->y();
            }
            CGContextAddRect(hd, mac_r);
        }
    }
    CGContextClip(hd);

    if(orig_xform) {//reset xforms
        CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
        CGContextConcatCTM(hd, old_xform);
    }
}

/*****************************************************************************
  QCoreGraphicsPaintEngine member functions
 *****************************************************************************/

inline static QPaintEngine::PaintEngineFeatures qt_mac_cg_features()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::CoordTransform|QPaintEngine::PenWidthTransform
        |QPaintEngine::PatternTransform|QPaintEngine::PixmapTransform
        |QPaintEngine::PainterPaths|QPaintEngine::PixmapScale
        |QPaintEngine::UsesFontEngine|QPaintEngine::LinearGradients
        |QPaintEngine::ClipTransform|QPaintEngine::AlphaStroke
        |QPaintEngine::AlphaFill|QPaintEngine::AlphaPixmap
        |QPaintEngine::FillAntialiasing|QPaintEngine::LineAntialiasing
        );
}

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine()
    : QQuickDrawPaintEngine(*(new QCoreGraphicsPaintEnginePrivate), qt_mac_cg_features())
{
}

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr)
    : QQuickDrawPaintEngine(dptr, qt_mac_cg_features())
{
}

QCoreGraphicsPaintEngine::~QCoreGraphicsPaintEngine()
{
}

bool
QCoreGraphicsPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QCoreGraphicsPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    //initialization
    d->offx = d->offy = 0; // (quickdraw compat!!)
    d->pdev = pdev;
    d->hd = qt_macCreateCGHandle(pdev);
    d->orig_xform = CGContextGetCTM(d->hd);
    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
    d->setClip(0);  //clear the context's clipping

    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
        ((QPixmap*)d->pdev)->detach();             // will modify it

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        { //offset painting in widget relative the tld (quickdraw compat!!!)
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
	bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if(w->isDesktop()) {
            if(!unclipped)
                qWarning("QCoreGraphicsPaintEngine::begin: Does not support clipped desktop on MacOSX");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QCoreGraphicsPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(pm->depth() == 1) {
            setRenderHint(QPainter::Antialiasing, false);
            setRenderHint(QPainter::TextAntialiasing, false);
        }

        if(pm->isNull()) {
            qWarning("QCoreGraphicsPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }
    return true;
}

bool
QCoreGraphicsPaintEngine::end()
{
    setActive(false);
    if(d->pdev->devType() == QInternal::Widget && ((QWidget*)d->pdev)->isDesktop())
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));
    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
    d->pdev = 0;
    if(d->hd) {
        CGContextSynchronize(d->hd);
        CGContextRelease(d->hd);
        d->hd = 0;
    }
    return true;
}

void
QCoreGraphicsPaintEngine::updatePen(const QPen &pen)
{
    Q_ASSERT(isActive());
    d->current.pen = pen;

    //pen style
    float *lengths = 0;
    int count = 0;
    if(pen.style() == Qt::DashLine) {
        static float inner_lengths[] = { 3, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DotLine) {
        static float inner_lengths[] = { 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DashDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DashDotDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1, 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    }
    CGContextSetLineDash(d->hd, 0, lengths, count);

    //pencap
    CGLineCap cglinecap = kCGLineCapButt;
    if(pen.capStyle() == Qt::SquareCap)
        cglinecap = kCGLineCapSquare;
    else if(pen.capStyle() == Qt::RoundCap)
        cglinecap = kCGLineCapRound;
    CGContextSetLineCap(d->hd, cglinecap);

    //penwidth
    CGContextSetLineWidth(d->hd, pen.width() <= 0 ? 1 : pen.width());

    //join
    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(pen.joinStyle() == Qt::BevelJoin)
        cglinejoin = kCGLineJoinBevel;
    else if(pen.joinStyle() == Qt::RoundJoin)
        cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin(d->hd, cglinejoin);

    //color
    const QColor &col = pen.color();
    CGContextSetRGBStrokeColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                               qt_mac_convert_color_to_cg(col.green()),
                               qt_mac_convert_color_to_cg(col.blue()),
                               qt_mac_convert_color_to_cg(col.alpha()));
}

void
QCoreGraphicsPaintEngine::updateBrush(const QBrush &brush, const QPointF &brushOrigin)
{
    Q_ASSERT(isActive());
    d->current.brush = brush;
    d->current.bg.origin = brushOrigin;

    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }

    //pattern
    Qt::BrushStyle bs = brush.style();
    if(bs == Qt::LinearGradientPattern) {
        CGFunctionCallbacks callbacks = { 0, qt_mac_color_gradient_function, 0 };
        CGFunctionRef fill_func = CGFunctionCreate(const_cast<void *>(reinterpret_cast<const void *>(&brush)), 1, 0, 4, 0, &callbacks);
        CGColorSpaceRef grad_colorspace = CGColorSpaceCreateDeviceRGB();
        const QPointF start = brush.gradientStart() * painter()->matrix(),
                       stop =  brush.gradientStop() * painter()->matrix();
        d->shading = CGShadingCreateAxial(grad_colorspace, CGPointMake(start.x(), start.y()),
                                          CGPointMake(stop.x(), stop.y()), fill_func, true, true);
        CGFunctionRelease(fill_func);
        CGColorSpaceRelease(grad_colorspace);
    } else if(bs != Qt::SolidPattern && bs != Qt::NoBrush) {
        int width = 0, height = 0;
        QMacPattern *qpattern = new QMacPattern;
        float components[4] = { 1.0, 1.0, 1.0, 1.0 };
        CGColorSpaceRef base_colorspace = 0;
        if (bs == Qt::CustomPattern) {
            qpattern->data.pixmap = brush.texture();
            if(qpattern->data.pixmap.isQBitmap()) {
                const QColor &col = brush.color();
                components[0] = qt_mac_convert_color_to_cg(col.red());
                components[1] = qt_mac_convert_color_to_cg(col.green());
                components[2] = qt_mac_convert_color_to_cg(col.blue());
                base_colorspace = CGColorSpaceCreateDeviceRGB();
            }
            width = qpattern->data.pixmap.width();
            height = qpattern->data.pixmap.height();
        } else {
            qpattern->as_mask = true;
            qpattern->data.bytes = qt_patternForBrush(bs, false);
            width = height = 8;
            const QColor &col = brush.color();
            components[0] = qt_mac_convert_color_to_cg(col.red());
            components[1] = qt_mac_convert_color_to_cg(col.green());
            components[2] = qt_mac_convert_color_to_cg(col.blue());
            base_colorspace = CGColorSpaceCreateDeviceRGB();
        }
        qpattern->opaque = (d->current.bg.mode == Qt::OpaqueMode);
        qpattern->foreground = brush.color();
        qpattern->background = d->current.bg.brush.color();

        CGColorSpaceRef fill_colorspace = CGColorSpaceCreatePattern(base_colorspace);
        CGContextSetFillColorSpace(d->hd, fill_colorspace);

        CGPatternCallbacks callbks;
        callbks.version = 0;
        callbks.drawPattern = qt_mac_draw_pattern;
        callbks.releaseInfo = qt_mac_dispose_pattern;
        CGPatternRef fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
                                                    CGContextGetCTM(d->hd), width, height,
                                                    kCGPatternTilingNoDistortion, !base_colorspace,
                                                    &callbks);
        CGContextSetFillPattern(d->hd, fill_pattern, components);

        CGPatternRelease(fill_pattern);
        CGColorSpaceRelease(fill_colorspace);
        if(base_colorspace)
            CGColorSpaceRelease(base_colorspace);
    } else if(bs != Qt::NoBrush) {
        const QColor &col = brush.color();
        CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                 qt_mac_convert_color_to_cg(col.green()),
                                 qt_mac_convert_color_to_cg(col.blue()),
                                 qt_mac_convert_color_to_cg(col.alpha()));
    }
}

void
QCoreGraphicsPaintEngine::updateFont(const QFont &)
{
    Q_ASSERT(isActive());
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QCoreGraphicsPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &brush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = brush;
}

void
QCoreGraphicsPaintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_ASSERT(isActive());
    d->setTransform(matrix.isIdentity() ? 0 : &matrix);
}

void
QCoreGraphicsPaintEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        clearf(ClipOn);
        d->current.clip = QRegion();
        d->setClip(0);
    } else {
        if(testf(ClipOn))
            op = Qt::ReplaceClip;
        setf(ClipOn);
        QRegion clipRegion(p.toFillPolygon().toPointArray(),
                           p.fillRule() == Qt::WindingFill);
        if(op == Qt::ReplaceClip) {
            d->current.clip = clipRegion;
            d->setClip(0);
        } else if(op == Qt::IntersectClip) {
            d->current.clip = d->current.clip.intersect(clipRegion);
        }
        if(op == Qt::UniteClip) {
            d->current.clip = d->current.clip.unite(clipRegion);
            d->setClip(&d->current.clip);
        } else {
            CGMutablePathRef path = qt_mac_compose_path(p);
            CGContextAddPath(d->hd, path);
            CGContextClip(d->hd);
            CGPathRelease(path);
        }
    }
}

void
QCoreGraphicsPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        clearf(ClipOn);
        d->current.clip = QRegion();
        d->setClip(0);
    } else {
        setf(ClipOn);
        if(op == Qt::IntersectClip)
            d->current.clip = d->current.clip.intersect(clipRegion);
        else if(op == Qt::ReplaceClip)
            d->current.clip = clipRegion;
        else if(op == Qt::UniteClip)
            d->current.clip = d->current.clip.unite(clipRegion);
        d->setClip(&d->current.clip);
    }
}

void
QCoreGraphicsPaintEngine::drawLine(const QLineF &line)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, line.startX(), line.startY()+1);
    CGContextAddLineToPoint(d->hd, line.endX(), line.endY()+1);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPath(const QPainterPath &p)
{
    CGMutablePathRef path = qt_mac_compose_path(p);
    uchar ops = QCoreGraphicsPaintEnginePrivate::CGStroke;
    if(p.fillRule() == Qt::WindingFill)
        ops |= QCoreGraphicsPaintEnginePrivate::CGFill;
    else
        ops |= QCoreGraphicsPaintEnginePrivate::CGEOFill;
    CGContextBeginPath(d->hd);
    d->drawPath(ops, path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawRect(const QRectF &r)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = 0;
    if(d->current.brush.style() == Qt::LinearGradientPattern) {
        path = CGPathCreateMutable();
        CGPathAddRect(path, 0, d->adjustedRect(r));
    } else {
        CGContextBeginPath(d->hd);
        CGContextAddRect(d->hd, d->adjustedRect(r));
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke,
                path);
    if(path)
        CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPoint(const QPointF &p)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, p.x(), p.y()+1);
    CGContextAddLineToPoint(d->hd, p.x(), p.y()+1);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPoints(const QPolygon &pa)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    for(int i=0; i < pa.size(); i++) {
        float x = pa[i].x(), y = pa[i].y();
        CGContextMoveToPoint(d->hd, x, y+1);
        CGContextAddLineToPoint(d->hd, x, y+1);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    }
}

void
QCoreGraphicsPaintEngine::drawEllipse(const QRectF &rr)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGRect r = d->adjustedRect(rr);
    CGAffineTransform transform = CGAffineTransformMakeScale(r.size.width / r.size.height, 1);
    CGPathAddArc(path, &transform,
                 (r.origin.x + (r.size.width / 2)) / (r.size.width / r.size.height),
                 r.origin.y + (r.size.height / 2), r.size.height / 2, 0, 2 * M_PI, false);
    CGContextBeginPath(d->hd);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill | QCoreGraphicsPaintEnginePrivate::CGStroke,
                path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPolygon(const QPolygon &a, PolygonDrawMode mode)
{
    Q_ASSERT(isActive());

    if (mode == PolylineMode) {
        CGContextMoveToPoint(d->hd, a[0].x(), a[0].y()+1);
        for(int x = 1; x < a.size(); ++x)
            CGContextAddLineToPoint(d->hd, a[x].x(), a[x].y()+1);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    } else {
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, 0, a[0].x(), a[0].y()+1);
        for(int x = 1; x < a.size(); ++x)
            CGPathAddLineToPoint(path, 0, a[x].x(), a[x].y()+1);
        if (a.first() != a.last())
            CGPathAddLineToPoint(path, 0, a[0].x(), a[0].y()+1);
        CGContextBeginPath(d->hd);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill
                    | QCoreGraphicsPaintEnginePrivate::CGStroke, path);
    }
}

void
QCoreGraphicsPaintEngine::drawLines(const QList<QLineF> &lines)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    for(int i = 0; i < lines.size(); i++) {
        const QPointF start = lines[i].start(), end = lines[i].end();
        CGContextMoveToPoint(d->hd, start.x(), start.y()+1);
        CGContextAddLineToPoint(d->hd, end.x(), end.y()+1);
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                                     Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(isActive());
    if(pm.isNull())
        return;

    //save
    CGContextSaveGState(d->hd);

    //setup
    bool asMask = pm.isQBitmap() || pm.depth() == 1;
    if(asMask) {     //set colour
        if(d->pdev->devType() == QInternal::Pixmap && static_cast<QPixmap*>(d->pdev)->isQBitmap()) {
            asMask = false; //if the destination is a bitmap no need for the colour tricks --Sam
        } else {
            const QColor &col = d->current.pen.color();
            CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                     qt_mac_convert_color_to_cg(col.green()),
                                     qt_mac_convert_color_to_cg(col.blue()),
                                     qt_mac_convert_color_to_cg(col.alpha()));
        }
    }
    //set clip
    QRegion rgn(r.toRect());
    qt_mac_clip_cg(d->hd, rgn, 0, 0);

    //draw
    const float sx = ((float)r.width())/sr.width(), sy = ((float)r.height())/sr.height();
    CGRect rect = CGRectMake(r.x()-(sr.x()*sx), r.y()-(sr.y()*sy), pm.width()*sx, pm.height()*sy);
    CGImageRef image = qt_mac_create_cgimage(pm, mode, asMask);
    HIViewDrawCGImage(d->hd, &rect, image); //top left
    CGImageRelease(image);

    //restore
    CGContextRestoreGState(d->hd);
}

void
QCoreGraphicsPaintEngine::initialize()
{
}

void
QCoreGraphicsPaintEngine::cleanup()
{
}

CGContextRef
QCoreGraphicsPaintEngine::handle() const
{
    return d->hd;
}

void
QCoreGraphicsPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap,
                                          const QPointF &p, Qt::PixmapDrawingMode)
{
    Q_ASSERT(isActive());

    //save the old state
    CGContextSaveGState(d->hd);
    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->data.pixmap = pixmap;
    qpattern->opaque = false;
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM(d->hd), width, height,
                                       kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(0);
    CGContextSetFillColorSpace(d->hd, cs);
    float component = 1.0; //just one
    CGContextSetFillPattern(d->hd, pat, &component);
    CGContextSetPatternPhase(d->hd, CGSizeMake(p.x()-r.x(), p.y()-r.y()));
    //fill the rectangle
    CGRect mac_rect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    CGContextFillRect(d->hd, mac_rect);
    //restore the state
    CGContextRestoreGState(d->hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
}

QPainter::RenderHints
QCoreGraphicsPaintEngine::supportedRenderHints() const
{
    return QPainter::RenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
}

void
QCoreGraphicsPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    CGContextSetShouldAntialias(d->hd, hints & QPainter::Antialiasing);
    CGContextSetShouldSmoothFonts(d->hd, hints & QPainter::TextAntialiasing);
}

CGRect
QCoreGraphicsPaintEnginePrivate::adjustedRect(const QRectF &r)
{
    float adjusted = 0;
    if(current.pen.style() != Qt::NoPen) {
        if(current.pen.width() <= 0)
            adjusted = 0.5;
        else
            adjusted = float(current.pen.width()) / 2;
    }
    return CGRectMake(r.x()+adjusted, r.y()+adjusted, r.width(), r.height());
}

void
QCoreGraphicsPaintEnginePrivate::setClip(const QRegion *rgn)
{
    if(hd) {
        qt_mac_clip_cg_reset(hd);
        QPoint mp(0, 0);
        if(d->pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget*>(pdev);
            mp = posInWindow(w);
            qt_mac_clip_cg(hd, w->d->clippedRegion(), &mp, &orig_xform);
        }
        if(paintevent_item *pevent = qt_mac_get_paintevent()) {
            if((*pevent) == pdev)
                qt_mac_clip_cg(hd, pevent->region(), &mp, &orig_xform);
        }
        if(rgn)
            qt_mac_clip_cg(hd, *rgn, 0, 0); //already device relative
    }
}

void QCoreGraphicsPaintEnginePrivate::drawPath(uchar ops, CGMutablePathRef path)
{
    Q_ASSERT((ops & (CGFill | CGEOFill)) != (CGFill | CGEOFill)); //can't really happen
    if ((ops & (CGFill | CGEOFill))) {
        if (current.brush.style() == Qt::LinearGradientPattern) {
            Q_ASSERT(path);
            CGContextAddPath(hd, path);
            CGContextSaveGState(hd);
            if (ops & CGFill)
                CGContextClip(hd);
            else if (ops & CGEOFill)
                CGContextEOClip(hd);
            CGContextDrawShading(hd, shading);
            CGContextRestoreGState(hd);
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        } else if (current.brush.style() == Qt::NoBrush) {
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        }
    }
    if ((ops & CGStroke) && current.pen.style() == Qt::NoPen)
        ops &= ~CGStroke;

    CGPathDrawingMode mode;
    if ((ops & (CGStroke | CGFill)) == (CGStroke | CGFill))
        mode = kCGPathFillStroke;
    else if ((ops & (CGStroke | CGEOFill)) == (CGStroke | CGEOFill))
        mode = kCGPathEOFillStroke;
    else if (ops & CGStroke)
        mode = kCGPathStroke;
    else if (ops & CGEOFill)
        mode = kCGPathEOFill;
    else if (ops & CGFill)
        mode = kCGPathFill;
    else //nothing to do..
        return;
    if(path)
        CGContextAddPath(hd, path);
    CGContextDrawPath(hd, mode);
}
