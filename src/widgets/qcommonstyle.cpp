/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcommonstyle.cpp#6 $
**
** Implementation of the QCommonStyle class
**
** Created : 981231
**
** Copyright (C) 1998-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcommonstyle.h"
#if !defined(QT_NO_STYLE_WINDOWS) || !defined(QT_NO_STYLE_MOTIF)
#define INCLUDE_MENUITEM_DEF
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qtoolbutton.h"
#include "qmenubar.h"
#include <limits.h>


// NOT REVISED
/*!
  \class QCommonStyle qcommonstyle.h
  \brief Encapsulates common Look and Feel of a GUI.

  \ingroup appearance

  This abstract class implements some of the widget's look and feel
  that is common to all GUI styles provided and shipped as part of Qt.
*/

/*!
  Constructs a QCommonStyle that provides the style \a s.  This determines
  the default behavior of the virtual functions.
*/
QCommonStyle::QCommonStyle(GUIStyle s) : QStyle(s)
{
}

 /*!
  Destructs the style.
*/
QCommonStyle::~QCommonStyle()
{
}


/*! \reimp
 */
void QCommonStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				  const QColorGroup &g, bool sunken,
			      bool /* editable */,
			      bool /*enabled */,
			      const QBrush *fill )
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}


/*! \reimp
 */
QRect QCommonStyle::comboButtonRect( int x, int y, int w, int h) const
{
    return buttonRect(x, y, w-21, h);
}

/*! \reimp
 */
QRect QCommonStyle::comboButtonFocusRect( int x, int y, int w, int h) const
{
    return buttonRect(x+2, y+2, w-4-21, h-4);
}

/*! \reimp
*/

void QCommonStyle::drawComboButtonMask( QPainter *p, int x, int y, int w, int h)
{
    drawButtonMask(p, x, y, w, h);
}


/*!\reimp
 */
void QCommonStyle::drawPushButtonLabel( QPushButton* btn, QPainter *p)
{
#ifndef QT_NO_COMPLEXWIDGETS
    QRect r = pushButtonContentsRect( btn );
    if ( btn->isDown() || btn->isOn() ){
	int sx = 0;
	int sy = 0;
	getButtonShift(sx, sy);
	r.moveBy( sx, sy );
    }
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( btn->isMenuButton() ) {
	int dx = menuButtonIndicatorWidth( btn->height() );
 	drawArrow( p, DownArrow, FALSE,
 	 	   x+w-dx, y+2, dx-4, h-4,
 		   btn->colorGroup(),
 		   btn->isEnabled() );
	w -= dx;
    }

    if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
	QIconSet::Mode mode = btn->isEnabled()
			      ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && btn->hasFocus() )
	    mode = QIconSet::Active;
	QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
	x += pixw + 4;
	w -= pixw + 4;
    }
    drawItem( p, x, y, w, h,
	      AlignCenter | ShowPrefix,
	      btn->colorGroup(), btn->isEnabled(),
	      btn->pixmap(), btn->text(), -1, &btn->colorGroup().buttonText() );
#endif
}



/*!\reimp
 */
void QCommonStyle::getButtonShift( int &x, int &y) const
{
    x = 0;
    y = 0;
}



/*!\reimp
 */
int QCommonStyle::defaultFrameWidth() const
{
    return 2;
}



/*!\reimp
 */
void QCommonStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe,
				  int& overlap) const
{
#ifndef QT_NO_COMPLEXWIDGETS
    overlap = 3;
    hframe = 24;
    vframe = 0;
    if ( t->shape() == QTabBar::RoundedAbove || t->shape() == QTabBar::RoundedBelow )
	vframe += 10;
#endif
}

/*!\reimp
 */
void QCommonStyle::drawTab( QPainter* p,  const  QTabBar* tb, QTab* t , bool selected )
{
#ifndef QT_NO_COMPLEXWIDGETS
    if ( tb->shape() == QTabBar::TriangularAbove || tb->shape() == QTabBar::TriangularBelow ) {
	// triangular, above or below
	int y;
	int x;
	QPointArray a( 10 );
	a.setPoint( 0, 0, -1 );
	a.setPoint( 1, 0, 0 );
	y = t->r.height()-2;
	x = y/3;
	a.setPoint( 2, x++, y-1 );
	a.setPoint( 3, x++, y );
	a.setPoint( 3, x++, y++ );
	a.setPoint( 4, x, y );

	int i;
	int right = t->r.width() - 1;
	for ( i = 0; i < 5; i++ )
	    a.setPoint( 9-i, right - a.point( i ).x(), a.point( i ).y() );

	if ( tb->shape() == QTabBar::TriangularAbove )
	    for ( i = 0; i < 10; i++ )
		a.setPoint( i, a.point(i).x(),
			    t->r.height() - 1 - a.point( i ).y() );

	a.translate( t->r.left(), t->r.top() );

	if ( selected )
	    p->setBrush( tb->colorGroup().base() );
	else
	    p->setBrush( tb->colorGroup().background() );
	p->setPen( tb->colorGroup().foreground() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
    }
#endif
}

/*!\reimp
 */
void QCommonStyle::drawTabMask( QPainter* p,  const  QTabBar* /* tb*/ , QTab* t, bool /* selected */ )
{
#ifndef QT_NO_COMPLEXWIDGETS
    p->drawRect( t->r );
#endif
}

/*!\reimp
 */
QStyle::ScrollControl QCommonStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p )
{
#ifndef QT_NO_COMPLEXWIDGETS
    if ( !sb->rect().contains( p ) )
	return NoScroll;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SubLine;
    if ( pos < sliderStart )
	return SubPage;
    if ( pos < sliderStart + sliderLength )
	return Slider;
    if ( pos < sliderMax + sliderLength )
	return AddPage;
    return AddLine;
#endif
}

/*!\reimp
 */
void
QCommonStyle::drawSliderMask( QPainter *p,
			int x, int y, int w, int h,
			Orientation, bool, bool )
{
    p->fillRect(x, y, w, h, color1);
}


/*!\reimp
 */
void
QCommonStyle::drawSliderGrooveMask( QPainter *p,
				   int x, int y, int w, int h,
				   QCOORD c,
				   Orientation orient )
{
    QColorGroup g(color1, color1, color1, color1, color1, color1, color1, color1, color0);
    drawSliderGroove( p, x, y, w, h, g, c, orient );
//      if ( orient == Horizontal )
// 	 p->fillRect(x, y, w, h, color1);
//     else
// 	p->fillRect(x, y, w, h, color1);
}


/*!\reimp
 */
int QCommonStyle::maximumSliderDragDistance() const
{
    return -1;
}



static const int motifArrowHMargin	= 6;	// arrow horizontal margin

/*! \reimp
 */
int QCommonStyle::popupSubmenuIndicatorWidth( const QFontMetrics& fm  ) const
{
    return fm.ascent() + motifArrowHMargin;
}

void QCommonStyle::drawMenuBarItem( QPainter* p, int x, int y, int w, int h,
				    QMenuItem* mi, QColorGroup& g,
				    bool enabled, bool )
{
#ifndef QT_NO_COMPLEXWIDGETS
    drawItem( p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
	    g, enabled, mi->pixmap(), mi->text(), -1, &g.buttonText() );
#endif
}

#if defined(_CC_GCC_)
#warning "doesn't really belong here... fix 3.0"
#endif

QRect QCommonStyle::pushButtonContentsRect( QPushButton* btn ) const
{
#ifndef QT_NO_COMPLEXWIDGETS
    int fw = 0;
    if ( btn->isDefault() || btn->autoDefault() )
	fw = buttonDefaultIndicatorWidth();

    return buttonRect( fw, fw, btn->width()-2*fw, btn->height()-2*fw );
#endif
}


void QCommonStyle::drawToolBarHandle( QPainter *p, const QRect &r, Qt::Orientation orientation,
				bool highlight, const QColorGroup &cg,
				bool )
{
    p->save();
    p->translate( r.x(), r.y() );

    if ( orientation == Qt::Vertical ) {
	if ( r.width() > 4 ) {
	    qDrawShadePanel( p, 2, 4, r.width() - 4, 3,
			     cg, highlight, 1, 0 );
	    qDrawShadePanel( p, 2, 7, r.width() - 4, 3,
			     cg, highlight, 1, 0 );
	}
    } else {
	if ( r.height() > 4 ) {
	    qDrawShadePanel( p, 4, 2, 3, r.height() - 4,
			     cg, highlight, 1, 0 );
	    qDrawShadePanel( p, 7, 2, 3, r.height() - 4,
			     cg, highlight, 1, 0 );
	}
    }
    p->restore();
}

void QCommonStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
		     const QColorGroup &g, bool sunken,
		     const QBrush *fill ) 
{
    QStyle::drawToolButton( p, x, y, w, h, g, sunken, fill );
}


void QCommonStyle::drawToolButton( QToolButton* btn, QPainter *p)
{
#ifndef QT_NO_COMPLEXWIDGETS
    if ( !btn )
	return;

    int x = 0;
    int y = 0;
    int w = btn->width();
    int h = btn->height();
    const QColorGroup &g = btn->colorGroup();
    bool sunken = ( btn->isOn() && !btn->son ) || btn->isDown();
    QBrush fill = btn->colorGroup().brush( btn->backgroundMode() == QToolButton::PaletteBackground ?
					   QColorGroup::Background : QColorGroup::Button );
    if ( guiStyle() == WindowsStyle && btn->isOn() )
	fill = QBrush( g.light(), Dense4Pattern );
#if defined(_WS_WIN_)
    if ( btn->uses3D() || btn->isDown() || ( btn->isOn() && !btn->son ) ) {
	// WIN2000 is really tricky here!
	bool drawRect = btn->isOn();
	if ( guiStyle() == WindowsStyle && btn->isOn() &&
	     ( QApplication::winVersion() == Qt::WV_2000 || QApplication::winVersion() == Qt::WV_98 ) &&
	     btn->uses3D() ) {
	    fill = btn->colorGroup().brush( QColorGroup::Button );
	    drawRect = FALSE;
	}
	if ( guiStyle() == WindowsStyle &&
	     ( QApplication::winVersion() == Qt::WV_2000 || QApplication::winVersion() == Qt::WV_98 ) &&
	     btn->autoRaise() ) {
	    drawPanel( p, x, y, w, h, g, sunken, 1, &fill );
	    if ( drawRect ) {
		p->setPen( QPen( g.color( QColorGroup::Button ) ) );
		p->drawRect( x + 1, y + 1, w - 2, h - 2 );
	    }
	} else {
	    drawToolButton( p, x, y, w, h, g, sunken, &fill );
	}
    } else if ( btn->parentWidget() && btn->parentWidget()->backgroundPixmap() &&
		!btn->parentWidget()->backgroundPixmap()->isNull() ) {
 	p->drawTiledPixmap( 0, 0, btn->width(), btn->height(),
			    *btn->parentWidget()->backgroundPixmap(),
			    btn->x(), btn->y() );
    }
#else
    if ( btn->uses3D() || btn->isDown() || ( btn->isOn() && !btn->son ) ) {
	drawToolButton( p, x, y, w, h, g, sunken, &fill );
    } else if ( btn->parentWidget() && btn->parentWidget()->backgroundPixmap() &&
	      !btn->parentWidget()->backgroundPixmap()->isNull() ) {
 	p->drawTiledPixmap( 0, 0, btn->width(), btn->height(),
			    *btn->parentWidget()->backgroundPixmap(),
			    btn->x(), btn->y() );
    } else {
	if ( btn->parentWidget() )
	    fill = QBrush( btn->parentWidget()->backgroundColor() );
	drawToolButton( p, x - 10, y - 10, w + 20, h + 20, g, sunken, &fill );
    }
#endif
#endif
}

#endif
