/****************************************************************************
** $Id: //depot/qt/main/examples/hello/hello.cpp#3 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "hello.h"
#include <qpushbutton.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qpixmap.h>


/*
  Constructs a Hello widget. Starts a 40 ms animation timer.
*/

Hello::Hello( const char *text, QWidget *parent, const char *name )
    : QWidget(parent,name), t(text), b(0)
{
    QTimer *timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), SLOT(animate()) );
    timer->start( 40 );

    resize( 260, 130 );
}


/*
  This private slot is called each time the timer fires.
*/

void Hello::animate()
{
    b = (b + 1) & 15;
    repaint( FALSE );
}


/*
  Handles mouse button release events for the Hello widget.

  We emit the clicked() signal when the mouse is released inside
  the widget.
*/

void Hello::mouseReleaseEvent( QMouseEvent *e )
{
    if ( rect().contains( e->pos() ) )
        emit clicked();
}


/*
  Handles paint events for the Hello widget.

  Flicker-free update using a double buffer.
*/

void Hello::paintEvent( QPaintEvent * )
{
    static int sin_tbl[16] = {
        0, 38, 71, 92, 100, 92, 71, 38,	0, -38, -71, -92, -100, -92, -71, -38};

    if ( t.isEmpty() )
        return;

    // 1: Compute some sizes, positions etc.
    QFontMetrics fm = fontMetrics();
    int w = fm.width(t) + 20;
    int h = fm.height() * 2;
    int pmx = width()/2 - w/2;
    int pmy = height()/2 - h/2;

    // 2: Create a double buffer for the area we wish to paint on
    QDoubleBuffer buffer( this, pmx, pmy, w, h );

    // 3: Paint into the buffer. Cool wave effect
    int x = 10;
    int y = h/2 + fm.descent();
    int i = 0;
    while ( !t[i].isNull() ) {
        int i16 = (b+i) & 15;
        buffer.painter()->setPen( QColor((15-i16)*16,255,255,QColor::Hsv) );
        buffer.painter()->drawText( pmx+x, pmy+y-sin_tbl[i16]*h/800, t.mid(i,1), 1 );
        x += fm.width( t[i] );
        i++;
    }

    // 4: The double buffer goes out of scope and automatically flushes its contents
}
