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

#ifndef QPAINTENGINE_X11_H
#define QPAINTENGINE_X11_H

#include "qpaintengine.h"

class QX11PaintEnginePrivate;
class QPainterState;

class QX11PaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QX11PaintEngine)

public:
    QX11PaintEngine();
    ~QX11PaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPoint &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateXForm(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, bool clipEnabled);

    virtual void drawLine(const QPoint &p1, const QPoint &ps);
    virtual void drawRect(const QRect &r);
    virtual void drawRects(const QList<QRect> &rects);
    virtual void drawPoint(const QPoint &p);
    virtual void drawPoints(const QPointArray &pa);
    virtual void drawEllipse(const QRect &r);
    virtual void drawLineSegments(const QPointArray &);
    virtual void drawPolygon(const QPointArray &pa, PolygonDrawMode mode);

    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr,
                            Qt::PixmapDrawingMode mode);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
				 Qt::PixmapDrawingMode mode);

    virtual Qt::HANDLE handle() const;
    inline Type type() const { return QPaintEngine::X11; }

    static void initialize();
    static void cleanup();

protected:
    QX11PaintEngine(QX11PaintEnginePrivate &dptr);

    friend void qt_cleanup();
    friend void qt_draw_transformed_rect(QPaintEngine *pp,  int x, int y, int w,  int h, bool fill);
    friend void qt_draw_background(QPaintEngine *pp, int x, int y, int w,  int h);
    friend class QPixmap;
    friend class QFontEngineBox;
    friend class QFontEngineXft;
    friend class QFontEngineXLFD;

private:
#if defined(Q_DISABLE_COPY)
    QX11PaintEngine(const QX11PaintEngine &);
    QX11PaintEngine &operator=(const QX11PaintEngine &);
#endif
};

#endif // QPAINTENGINE_X11_H
