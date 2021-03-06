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

#ifndef QPAINTER_P_H
#define QPAINTER_P_H

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

#include "QtGui/qbrush.h"
#include "QtGui/qfont.h"
#include "QtGui/qpen.h"
#include "QtGui/qregion.h"
#include "QtGui/qmatrix.h"
#include "QtGui/qpainter.h"
#include "QtGui/qpainterpath.h"
#include "QtGui/qpaintengine.h"
#include "QtCore/qvector.h"

QT_BEGIN_NAMESPACE

class QPaintEngine;

class QPainterClipInfo
{
public:
    enum ClipType { RegionClip, PathClip };

    QPainterClipInfo(const QPainterPath &p, Qt::ClipOperation op, const QTransform &m) :
        clipType(PathClip), matrix(m), operation(op), path(p) { }

    QPainterClipInfo(const QRegion &r, Qt::ClipOperation op, const QTransform &m) :
        clipType(RegionClip), matrix(m), operation(op), region(r) { }

    ClipType clipType;
    QTransform matrix;
    Qt::ClipOperation operation;
    QPainterPath path;
    QRegion region;
};


class QPainterState : public QPaintEngineState
{
public:
    QPainterState();
    QPainterState(const QPainterState *s);
    ~QPainterState();
    void init(QPainter *p);

    QPointF bgOrigin;
    QFont font;
    QFont deviceFont;
    QPen pen;
    QBrush brush;
    QBrush bgBrush;             // background brush
    QRegion clipRegion;
    QPainterPath clipPath;
    Qt::ClipOperation clipOperation;
    QPainter::RenderHints renderHints;
    QList<QPainterClipInfo> clipInfo;
    QTransform worldMatrix;       // World transformation matrix, not window and viewport
    QTransform matrix;            // Complete transformation matrix,
    int txop;
    int wx, wy, ww, wh;         // window rectangle
    int vx, vy, vw, vh;         // viewport rectangle
    qreal opacity;

    uint WxF:1;                 // World transformation
    uint VxF:1;                 // View transformation
    uint clipEnabled:1;

    Qt::BGMode bgMode;
    QPainter *painter;
    Qt::LayoutDirection layoutDirection;
    QPainter::CompositionMode composition_mode;
    uint emulationSpecifier;
    uint changeFlags;
};


class QPainterPrivate
{
    Q_DECLARE_PUBLIC(QPainter)
public:
    QPainterPrivate(QPainter *painter)
        : q_ptr(painter), txinv(0), emptyState(true), device(0)
        , original_device(0), engine(0), fillrect_func(0)
    {
        states.push_back(new QPainterState());
        state = states.back();
    }

    ~QPainterPrivate()
    {
        for (int i=0; i<states.size(); ++i)
            delete states.at(i);
    }

    QPainter *q_ptr;

    QPoint redirection_offset;

    QPainterState *state;
    QVector<QPainterState*> states;

    QTransform invMatrix;
    uint txinv:1;
    uint emptyState:1;

    enum DrawOperation { StrokeDraw        = 0x1,
                         FillDraw          = 0x2,
                         StrokeAndFillDraw = 0x3
    };

    void updateEmulationSpecifier(QPainterState *s);
    void updateState(QPainterState *state);

    void draw_helper(const QPainterPath &path, DrawOperation operation = StrokeAndFillDraw);
    void drawStretchToDevice(const QPainterPath &path, DrawOperation operation);
    void drawOpaqueBackground(const QPainterPath &path, DrawOperation operation);

    void updateMatrix();
    void updateInvMatrix();
    void init();

    int rectSubtraction() const {
        return state->pen.style() != Qt::NoPen && state->pen.width() == 0 ? 1 : 0;
    }

    QTransform viewTransform() const;

    QPaintDevice *device;
    QPaintDevice *original_device;
    QPaintEngine *engine;

    typedef void (QPaintEngine::*FillRectBackdoor)(const QRect&, const QBrush&);
    FillRectBackdoor fillrect_func;
};

QString qt_generate_brush_key(const QBrush &brush);

QT_END_NAMESPACE

#endif // QPAINTER_P_H
