/****************************************************************************
**
** Definition of QPaintEngine class.
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

#include "qpaintengine.h"
#include "qpaintengine_p.h"
#include "qpainter_p.h"

#include <private/qfontengine_p.h>

#define d d_ptr()
#define q q_ptr()

QPaintEngine::QPaintEngine(PaintEngineFeatures caps)
    : dirtyFlag(0),
      changeFlag(0),
      active(0),
      state(0),
      gccaps(caps),
      d_ptr(new QPaintEnginePrivate)
{
    d_ptr->q_ptr = this;
}

QPaintEngine::QPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures caps)
    : dirtyFlag(0),
      changeFlag(0),
      active(0),
      state(0),
      gccaps(caps),
      d_ptr(&dptr)
{
    d_ptr->q_ptr = this;
}

QPaintEngine::~QPaintEngine()
{
    delete d_ptr;
}

QPainter *QPaintEngine::painter() const
{
    return state->painter;
}

void QPaintEngine::updateInternal(QPainterState *s, bool updateGC)
{
    if (!s || !updateGC) {
        state = s;
        return;
    }

    // Same state, do minimal update...
    if (s==state) {
        if (testDirty(DirtyPen))
            updatePen(s->pen);
        if (testDirty(DirtyBrush))
            updateBrush(s->brush, s->bgOrigin);
        if (testDirty(DirtyFont))
            updateFont(s->font);
        if (testDirty(DirtyBackground))
            updateBackground(s->bgMode, s->bgBrush);
        if (testDirty(DirtyTransform))
            updateXForm(s->matrix);
        if (testDirty(DirtyClip)) {
            updateClipRegion(s->clipRegion, s->clipEnabled);
        }
        // Same painter, restoring old state.
    } else if (state && s->painter == state->painter) {
        if ((changeFlag&DirtyPen)!=0)
            updatePen(s->pen);
        if ((changeFlag&DirtyBrush)!=0)
            updateBrush(s->brush, s->bgOrigin);
        if ((changeFlag&DirtyFont)!=0)
            updateFont(s->font);
        if ((changeFlag&DirtyBackground)!=0)
            updateBackground(s->bgMode, s->bgBrush);
        if ((changeFlag&DirtyTransform)!=0)
            updateXForm(s->matrix);
        if ((changeFlag&DirtyClip)!=0 || (changeFlag&DirtyClip) != 0)
            updateClipRegion(s->clipRegion, s->clipEnabled);
        changeFlag = 0;
        // Different painter or state == 0 which is true for first time call
    } else {
        changeFlag = 0;
        updatePen(s->pen);
        updateBrush(s->brush, s->bgOrigin);
        updateFont(s->font);
        updateBackground(s->bgMode, s->bgBrush);
        updateXForm(s->matrix);
        updateClipRegion(s->clipRegion, s->clipEnabled);
    }
    dirtyFlag = 0;
    state = s;
}

/*! Default implementation does nothing...
 */
void QPaintEngine::drawPath(const QPainterPath &path)
{

}

void QPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    bool useFontEngine = true;
    if (hasFeature(QPaintEngine::UsesFontEngine)) {
        if (state->txop > QPainter::TxTranslate) {
            useFontEngine = false;
            QFontEngine *fe = ti.fontEngine;
            QFontEngine::FECaps fecaps = fe->capabilites();
            if (state->txop == QPainter::TxRotShear) {
                useFontEngine = (fecaps == QFontEngine::FullTransformations);
                if (!useFontEngine
                    && state->matrix.m11() == state->matrix.m22()
                    && state->matrix.m12() == -state->matrix.m21())
                    useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
            } else if (state->txop == QPainter::TxScale) {
                useFontEngine = (fecaps & QFontEngine::Scale);
            }
        }
        if (useFontEngine) {
            ti.fontEngine->draw(this, p.x(),  p.y(), ti, textFlags);
        }
    }

    if (!useFontEngine) {
        // Fallback: rasterize into a pixmap and draw the pixmap
        QPixmap pm(ti.width, ti.ascent + ti.descent);
        pm.fill(white);

        QPainter painter;
        painter.begin(&pm);
        painter.setPen(black);
        painter.drawTextItem(0, ti.ascent, ti, textFlags);
        painter.end();

        QImage img = pm;
        if (img.depth() != 32)
            img = img.convertDepth(32);
        img.setAlphaBuffer(true);
        int i = 0;
        while (i < img.height()) {
            uint *p = (uint *) img.scanLine(i);
            uint *end = p + img.width();
            while (p < end) {
                *p = ((0xff - qGray(*p)) << 24) | (state->pen.color().rgb() & 0x00ffffff);
                ++p;
            }
            ++i;
        }

        pm = img;
        state->painter->drawPixmap(p.x(), p.y() - ti.ascent, pm);
    }
}

void QPaintEngine::drawRects(const QList<QRect> &)
{
}

QPainter::RenderHints QPaintEngine::supportedRenderHints() const
{
    return 0;
}

QPainter::RenderHints QPaintEngine::renderHints() const
{
    return 0;
}

void QPaintEngine::setRenderHint(QPainter::RenderHint, bool)
{
}


void QWrapperPaintEngine::updatePen(const QPen &pen) { wrap->updatePen(pen); }
void QWrapperPaintEngine::updateBrush(const QBrush &brush, const QPoint &pt)
{ wrap->updateBrush(brush, pt); }
void QWrapperPaintEngine::updateFont(const QFont &font) { wrap->updateFont(font); }
void QWrapperPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{ wrap->updateBackground(bgMode, bgBrush); }
void QWrapperPaintEngine::updateXForm(const QWMatrix &matrix) { wrap->updateXForm(matrix); }
void QWrapperPaintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{ wrap->updateClipRegion(clipRegion, clipEnabled); }
void QWrapperPaintEngine::drawLine(const QPoint &p1, const QPoint &p2) { wrap->drawLine(p1, p2); }
void QWrapperPaintEngine::drawRect(const QRect &r) { wrap->drawRect(r); }
void QWrapperPaintEngine::drawPoint(const QPoint &p) { wrap->drawPoint(p); }
void QWrapperPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{ wrap->drawPoints(pa, index, npoints); }
void QWrapperPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{ wrap->drawRoundRect(r, xRnd, yRnd); }
void QWrapperPaintEngine::drawEllipse(const QRect &r)
{ wrap->drawEllipse(r); }
void QWrapperPaintEngine::drawArc(const QRect &r, int a, int alen)
{ wrap->drawArc(r, a, alen); }
void QWrapperPaintEngine::drawPie(const QRect &r, int a, int alen)
{ wrap->drawPie(r, a, alen); }
void QWrapperPaintEngine::drawChord(const QRect &r, int a, int alen)
{ wrap->drawChord(r, a, alen); }
void QWrapperPaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{ wrap->drawLineSegments(a, index, nlines); }
void QWrapperPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{ wrap->drawPolyline(pa, index, npoints); }
void QWrapperPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{ wrap->drawPolygon(pa, winding, index, npoints); }

void QWrapperPaintEngine::drawConvexPolygon(const QPointArray &a, int index, int npoints)
{ wrap->drawConvexPolygon(a, index, npoints); }
#ifndef QT_NO_BEZIER
void QWrapperPaintEngine::drawCubicBezier(const QPointArray &a, int index)
{ wrap->drawCubicBezier(a, index); }
#endif

void QWrapperPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask)
{ wrap->drawPixmap(r, pm, sr, imask); }
void QWrapperPaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{ wrap->drawTextItem(p, ti, textflags); }
void QWrapperPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s, bool optim)
{ wrap->drawTiledPixmap(r, pixmap, s, optim); }

