/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.cpp#9 $
**
** Implementation of QFrame widget class
**
** Author  : Haavard Nord
** Created : 950201
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qframe.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qframe.cpp#9 $";
#endif

/*! \class QFrame qframe.h

  \brief This class provides decorative borders and border-like effects.

  Here are the types of borders QFrame knows about: <dl compact>
  <dt>NoFrame <dd> No visible frame, but the frame will still be as
  wide as you tell it to be (See setFrameWidth()).
  <dt>Box<dd> The frame is a simple box, as wide as setFrameWidth()
  specifies.
  <dt>Panel <dd> I'm sure hanord will fill this in.
  <dt> HLine <dd>
  <dt> VLine <dd>
  <dt> MType <dd>
  <dt> Plain <dd>
  <dt> Raised <dd>
  <dt> Sunken <dd>
  <dt> MStyle <dd>
  </dl> */

/*! Creates a frame with frame style NoFrame, 1 pixel frame width, and */

QFrame::QFrame( QWidget *parent, const char *name ) : QWidget( parent, name )
{
    initMetaObject();
    frect  = QRect( 0, 0, 0, 0 );
    fstyle = NoFrame;				// set default frame style
    fwidth = 1;
    mwidth = 0;
}

/*! Returns the geometry of the rectangle inside the border. \sa
  QRect, setFrameWidth(), setMidLineWidth().  */

QRect QFrame::contentsRect() const
{
    QRect r = frameRect();
    int tw = fwidth+mwidth;
    r.setRect( r.x()+tw, r.y()+tw, r.width()-tw*2, r.height()-tw*2 );
    return r;
}


void QFrame::setFrame( int f )
{
    fstyle = (short)f;
}

void QFrame::setFrameWidth( int fw )
{
    fwidth = fw;
}

void QFrame::setMidLineWidth( int mw )
{
    mwidth = mw;
}


void QFrame::setFrameRect( const QRect &r )
{
    frect = r;
}


void QFrame::paintEvent( QPaintEvent * )
{
    QPainter paint;
    paint.begin( this );
    drawFrame( &paint );
    drawContents( &paint );
    paint.end();
}


void QFrame::drawFrame( QPainter *p )
{
    QRect       r = frameRect();
    QPoint      p1, p2;
    QPainter   *paint = p;
    int	        type  = fstyle & MType;
    int	        style = fstyle & MStyle;
    QColorGroup g     = colorGroup();
    QColor      fg    = g.foreground();
    QColor	light = g.light();
    QColor	dark  = g.dark();
    QColor	mid   = g.mid();

    switch ( type ) {

	case Box:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, fwidth );
		    break;
		case Raised:
		    paint->drawShadeRect( r, light, dark, fwidth, mid, mwidth);
		    break;
		case Sunken:
		    paint->drawShadeRect( r, dark, light, fwidth, mid, mwidth);
		    break;
	    }
	    break;

	case Panel:
	    switch ( style ) {
		case Plain:
		    paint->drawShadePanel( r, fg, fg, fwidth );
		    break;
		case Raised:
		    paint->drawShadePanel( r, light, dark, fwidth );
		    break;
		case Sunken:
		    paint->drawShadePanel( r, dark, light, fwidth );
		    break;
	    }
	    break;

	case HLine:
	case VLine:
	    if ( type == HLine ) {
		p1 = QPoint( r.x(), r.height()/2 );
		p2 = QPoint( r.x()+r.width(), p1.y() );
	    }
	    else {
		p1 = QPoint( r.x()+r.width()/2, 0 );
		p2 = QPoint( p1.x(), r.height() );
	    }
	    switch ( style ) {
		case Plain:
		    paint->drawShadeLine( p1, p2, fg, fg, fwidth, fg, mwidth );
		    break;
		case Raised:
		    paint->drawShadeLine( p1, p2, light, dark,
					  fwidth, mid, mwidth );
		    break;
		case Sunken:
		    paint->drawShadeLine( p1, p2, dark, light,
					  fwidth, mid, mwidth );
		    break;
	    }
	    break;
    }
}


void QFrame::drawContents( QPainter * )
{
}
