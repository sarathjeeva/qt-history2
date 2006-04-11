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

#ifndef QGRAPHICSITEMANIMATION_H
#define QGRAPHICSITEMANIMATION_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

class QGraphicsItem;
class QMatrix;
class QPointF;
class QTimeLine;

class QGraphicsItemAnimationPrivate;
class Q_GUI_EXPORT QGraphicsItemAnimation : public QObject
{
    Q_OBJECT
public:
    QGraphicsItemAnimation(QObject *parent = 0);
    virtual ~QGraphicsItemAnimation();

    QGraphicsItem *item() const;
    void setItem(QGraphicsItem *item);

    QTimeLine *timeLine() const;
    void setTimeLine(QTimeLine *timeLine);
    
    QPointF posAt(qreal step) const;
    void setPosAt(qreal step, const QPointF &pos);

    QMatrix matrixAt(qreal step) const;
    
    qreal rotationAt(qreal step) const;
    void setRotationAt(qreal step, qreal angle);

    qreal xTranslationAt(qreal step) const;
    qreal yTranslationAt(qreal step) const;
    void setTranslationAt(qreal step, qreal dx, qreal dy);
    
    qreal verticalScaleAt(qreal step) const;
    qreal horizontalScaleAt(qreal step) const;
    void setScaleAt(qreal step, qreal sx, qreal sy);

    qreal verticalShearAt(qreal step) const;
    qreal horizontalShearAt(qreal step) const;
    void setShearAt(qreal step, qreal sh, qreal sv);
    
    void clear();

public Q_SLOTS:
    void setStep(qreal x);
    void reset();

Q_SIGNALS:
    void newTransformation(const QPointF &pos, const QMatrix &matrix);
    
private:
    Q_DISABLE_COPY(QGraphicsItemAnimation)
    QGraphicsItemAnimationPrivate *d;
};

QT_END_HEADER

#endif
