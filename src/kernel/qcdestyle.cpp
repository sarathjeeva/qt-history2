#include "qcdestyle.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qpalette.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include <limits.h>

/*!
  \class QCDEStyle qcdestyle.h
  \brief CDE Look and Feel
  
  This style provides a slightly improved Motif look similar to some
  versions of the Common Desktop Environment (CDE). The main
  difference are thinner frames and more modern radiobuttons and
  checkboxes.
*/

/*!
    Constructs a QCDEStyle
*/
QCDEStyle::QCDEStyle() : QMotifStyle()
{
}

/*!
  Reimplementation from QStyle

  \sa QStyle
  */

int QCDEStyle::defaultFrameWidth()
{
    return 1;
}


/*!
  Reimplementation from QMotifStyle

  \sa QMotifStyle
  */

void QCDEStyle::drawArrow( QPainter *p, ArrowType type, bool down,
		 int x, int y, int w, int h,
		 const QColorGroup &g, bool /* enabled */, const QBrush * /* fill */ )
{
    QPointArray bFill;				// fill polygon
    QPointArray bTop;				// top shadow.
    QPointArray bBot;				// bottom shadow.
    QPointArray bLeft;				// left shadow.
    QWMatrix	matrix;				// xform matrix
    bool vertical = type == UpArrow || type == DownArrow;
    bool horizontal = !vertical;
    int	 dim = w < h ? w : h;
    int	 colspec = 0x0000;			// color specification array

    if ( dim < 2 )				// too small arrow
	return;

    if ( dim > 3 ) {
	bFill.resize( dim & 1 ? 3 : 4 );
 	bTop.resize( 2 );
	bBot.resize( 2 );
	bLeft.resize( 2 );
	bLeft.putPoints( 0, 2, 0,0, 0,dim-1 );
	bTop.putPoints( 0, 2, 1,0, dim-1, dim/2);
	bBot.putPoints( 0, 2, 1,dim-1, dim-1, dim/2);

	if ( dim > 6 ) {			// dim>6: must fill interior
	    bFill.putPoints( 0, 2, 1,dim-1, 1,1 );
	    if ( dim & 1 )			// if size is an odd number
		bFill.setPoint( 2, dim - 2, dim / 2 );
	    else
		bFill.putPoints( 2, 2, dim-2,dim/2-1, dim-2,dim/2 );
	}
    }
    else {
	if ( dim == 3 ) {			// 3x3 arrow pattern
	    bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
	    bTop .setPoints( 2, 1,0, 1,0 );
	    bBot .setPoints( 2, 1,2, 2,1 );
	}
	else {					// 2x2 arrow pattern
	    bLeft.setPoints( 2, 0,0, 0,1 );
	    bTop .setPoints( 2, 1,0, 1,0 );
	    bBot .setPoints( 2, 1,1, 1,1 );
	}
    }

    if ( type == UpArrow || type == LeftArrow ) {
	matrix.translate( x, y );
	if ( vertical ) {
	    matrix.translate( 0, h - 1 );
	    matrix.rotate( -90 );
	} else {
	    matrix.translate( w - 1, h - 1 );
	    matrix.rotate( 180 );
	}
	if ( down )
	    colspec = horizontal ? 0x2334 : 0x2343;
	else
	    colspec = horizontal ? 0x1443 : 0x1434;
    }
    else if ( type == DownArrow || type == RightArrow ) {
	matrix.translate( x, y );
	if ( vertical ) {
	    matrix.translate( w-1, 0 );
	    matrix.rotate( 90 );
	}
	if ( down )
	    colspec = horizontal ? 0x2443 : 0x2434;
	else
	    colspec = horizontal ? 0x1334 : 0x1343;
    }

    QColor *cols[5];
    cols[0] = 0;
    cols[1] = (QColor *)&g.button();
    cols[2] = (QColor *)&g.mid();
    cols[3] = (QColor *)&g.light();
    cols[4] = (QColor *)&g.dark();
#define CMID	*cols[ (colspec>>12) & 0xf ]
#define CLEFT	*cols[ (colspec>>8) & 0xf ]
#define CTOP	*cols[ (colspec>>4) & 0xf ]
#define CBOT	*cols[ colspec & 0xf ]

    QPen     savePen   = p->pen();		// save current pen
    QBrush   saveBrush = p->brush();		// save current brush
    QWMatrix wxm = p->worldMatrix();
    QPen     pen( NoPen );
    QBrush brush = g.fillButton();

    p->setPen( pen );
    p->setBrush( brush );
    p->setWorldMatrix( matrix, TRUE );		// set transformation matrix
    p->drawPolygon( bFill );			// fill arrow
    p->setBrush( NoBrush );			// don't fill

    p->setPen( CLEFT );
    p->drawLineSegments( bLeft );
    p->setPen( CBOT );
    p->drawLineSegments( bBot );
    p->setPen( CTOP );
    p->drawLineSegments( bTop );

    p->setWorldMatrix( wxm );
    p->setBrush( saveBrush );			// restore brush
    p->setPen( savePen );			// restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT

}

/*!
  Reimplementation from QMotifStyle

  \sa QMotifStyle
  */
void QCDEStyle::drawIndicator( QPainter* p,
			       int x, int y, int w, int h, const QColorGroup &g,
			       bool on, bool down, bool /* enabled */ )
{
    bool showUp = !down && !on;
    QBrush fill =  down ? g.fillMid() : g.fillButton();
    qDrawShadePanel( p, x, y, w, h, g, !showUp, defaultFrameWidth(), &fill );

    if (on) {
	QPointArray a( 7*2 );
	int i, xx, yy;
	xx = x+3;
	yy = y+5;
	for ( i=0; i<3; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( i=3; i<7; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	p->setPen( g.foreground() );
	p->drawLineSegments( a );
    }
}


#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

/*!
  Reimplementation from QMotifStyle

  \sa QMotifStyle
  */
void QCDEStyle::drawExclusiveIndicator( QPainter* p,
				   int x, int y, int w, int h, const QColorGroup &g,
				   bool on, bool down, bool /* enabled */ )
{
    static QCOORD pts1[] = {		// up left  lines
	1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
    static QCOORD pts4[] = {		// bottom right  lines
	2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	11,4, 10,3, 10,2 };
    static QCOORD pts5[] = {		// inner fill
	4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };

    p->eraseRect( x, y, w, h );
    QPointArray a( QCOORDARRLEN(pts1), pts1 );
    a.translate( x, y );
    p->setPen( (down||on) ? g.dark() : g.light() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts4), pts4 );
    a.translate( x, y );
    p->setPen(  (down||on) ? g.light() : g.dark() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts5), pts5 );
    a.translate( x, y );
    QColor fillColor = on ? g.dark() : g.background();
    p->setPen( fillColor );
    p->setBrush( on ?  g.fillDark() : g.fillBackground() );
    p->drawPolygon( a );
}


