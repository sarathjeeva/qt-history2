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

#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include "element.h"
#include "canvastext.h"

#include <qcanvas.h>


class QPoint;


class CanvasView : public QCanvasView
{
    Q_OBJECT
public:
    CanvasView( QCanvas *canvas, ElementVector *elements,
		QWidget* parent = 0, const char* name = "canvas view",
		WFlags f = 0 )
	: QCanvasView( canvas, parent, name, f ), m_movingItem(0),
	  m_elements( elements ) {}

protected:
    void viewportResizeEvent( QResizeEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsContextMenuEvent( QContextMenuEvent *e );

private:
    QCanvasItem *m_movingItem;
    QPoint m_pos;
    ElementVector *m_elements;
};


#endif
