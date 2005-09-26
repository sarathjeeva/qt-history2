/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DISPLAYSHAPE_H
#define DISPLAYSHAPE_H

#include <QBrush>
#include <QFont>
#include <QHash>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QSizeF>
#include <QWidget>

class DisplayShape
{
public:
    DisplayShape(const QPointF &position, const QSizeF &maxSize);
    virtual ~DisplayShape() {};

    virtual bool animate();
    virtual bool contains(const QString &key) const;
    virtual bool isInteractive() const;
    virtual QVariant metaData(const QString &key) const;
    virtual void paint(QPainter *painter) const;
    virtual QPointF position() const;
    virtual QPointF target() const;
    virtual QRectF rect() const;
    virtual QSizeF size() const;
    virtual void removeMetaData(const QString &key);
    virtual void setInteractive(bool enable);
    virtual void setMetaData(const QString &key, const QVariant &value);
    virtual void setPosition(const QPointF &point);
    virtual void setSize(const QSizeF &size);
    virtual void setTarget(const QPointF &point);

protected:
    QHash<QString,QVariant> meta;
    QImage image;
    QPointF pos;
    QPointF targetPos;
    QSizeF maxSize;
    bool interactive;
};

class PanelShape : public DisplayShape
{
public:
    PanelShape(const QPainterPath &path, const QBrush &normal,
              const QBrush &highlighted, const QPen &pen,
              const QPointF &position, const QSizeF &maxSize);

    bool animate();
    void paint(QPainter *painter) const;
    QRectF rect() const;

private:
    QBrush brush;
    QBrush highlightedBrush;
    QBrush normalBrush;
    QPainterPath path;
    QPen pen;
};

class TitleShape : public DisplayShape
{
public:
    TitleShape(const QString &text, const QFont &font, const QPen &pen,
               const QPointF &position, const QSizeF &maxSize,
               Qt::Alignment alignment = Qt::AlignVCenter | Qt::AlignLeft);

    bool animate();
    void paint(QPainter *painter) const;
    QRectF rect() const;

private:
    QFont font;
    QString text;
    QPen pen;
    QPointF baselineStart;
    QRectF textRect;
    Qt::Alignment alignment;
};

class ImageShape : public DisplayShape
{
public:
    ImageShape(const QImage &original, const QPointF &position,
               const QSizeF &maxSize, int alpha = 0,
               Qt::Alignment alignment = Qt::AlignCenter);

    bool animate();
    void paint(QPainter *painter) const;
    QRectF rect() const;

private:
    void redraw();

    int alpha;
    QImage source;
    QPointF offset;
    Qt::Alignment alignment;
};

class DocumentShape : public DisplayShape
{
public:
    DocumentShape(const QString &text, const QFont &font,
                  const QPointF &position, const QSizeF &maxSize,
                  int alpha = 0);
    ~DocumentShape();

    bool animate();
    void paint(QPainter *painter) const;
    QRectF rect() const;

private:
    void redraw();

    QImage source;
    int alpha;
    QTextDocument textDocument;
};

#endif
