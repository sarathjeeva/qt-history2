/****************************************************************************
** $Id: //depot/qt/main/examples/forever/forever.cpp#2 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qtimer.h>
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>				// defines rand() function

#include "forever.h"


//
// Forever - a widget that draws rectangles forever.
//

//
// Constructs a Forever widget.
//

Forever::Forever( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    for (int a=0; a<numColors; a++) {
	colors[a] = QColor( rand()&255,
			    rand()&255,
			    rand()&255 );
    }
    rectangles = 0;
    startTimer( 0 );				// run continuous timer
    QTimer * counter = new QTimer( this );
    connect( counter, SIGNAL(timeout()),
	     this, SLOT(updateCaption()) );
    counter->start( 1000 );
}


void Forever::updateCaption()
{
    QString s;
    s.sprintf( "forever - %d rectangles/second", rectangles );
    rectangles = 0;
    setCaption( s );
}


//
// Handles paint events for the Forever widget.
//

void Forever::paintEvent( QPaintEvent * )
{
    QPainter paint( this );			// painter object
    int w = width();
    int h = height();
    paint.setPen( NoPen );			// do not draw outline
    paint.setBrush( colors[rand() % numColors]);// set random brush color
    QPoint p1( rand()%w, rand()%h );	// p1 = top left
    QPoint p2( rand()%w, rand()%h );	// p2 = bottom right
    QRect r( p1, p2 );
    paint.drawRect( r );			// draw filled rectangle
}

//
// Handles timer events for the Forever widget.
//

void Forever::timerEvent( QTimerEvent * )
{
    for ( int i=0; i<100; i++ ) {
	repaint( FALSE );			// repaint, don't erase
	rectangles++;
    }
}


//
// Create and display Forever widget.
//

int main( int argc, char **argv )
{
    QApplication a( argc, argv );		// create application object
    Forever always;				// create widget
    a.setMainWidget( &always );			// set as main widget
    always.show();				// show widget
    return a.exec();				// run event loop
}
