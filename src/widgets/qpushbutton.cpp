/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#81 $
**
** Implementation of QPushButton class
**
** Created : 940221
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qdialog.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qdrawutl.h"
#include "qpixmap.h"
#include "qpmcache.h"
#include "qbitmap.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qpushbutton.cpp#81 $");


/*!
  \class QPushButton qpushbt.h
  \brief The QPushButton widget provides a push button with a text label.

  \ingroup realwidgets

  A default push button in a dialog emits the clicked signal if the user
  presses the Enter key.

  A push button has \c TabFocus as a default focusPolicy(), i.e. it can 
  get keyboard focus by tabbing but not by clicking.

*/

const int extraMotifWidth = 10;
const int extraMotifHeight = 10;


static bool extraSize( const QPushButton *b, int &wx, int &hx,
		       bool onlyWhenDefault )
{
    if ( onlyWhenDefault && !b->isDefault() ) {
	wx = hx = 0;
	return FALSE;
    }
    switch ( b->style() ) {
	case MotifStyle:			// larger def Motif buttons
	    wx = extraMotifWidth;
	    hx = extraMotifHeight;
	    break;
	default:
	    wx = hx = 0;
	    return FALSE;
    }
    return TRUE;
}

static void resizeDefButton( QPushButton *b )
{
    int wx, hx;
    if ( !extraSize( b, wx, hx, FALSE ) )
	return;
    if ( !b->isDefault() ) {			// not default -> shrink
	wx = -wx;
	hx = -hx;
    }
    QRect r = b->geometry();
    b->QWidget::setGeometry( r.x()-wx/2, r.y()-hx/2,
			     r.width()+wx, r.height()+hx );
}


/*!
  Constructs a push button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*!
  Constructs a push button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
*/

QPushButton::QPushButton( const char *text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

void QPushButton::init()
{
    initMetaObject();
    autoDefButton = defButton = lastDown = lastDef = lastEnabled = FALSE;
    setFocusPolicy( TabFocus );
}


/*!
  Makes the push button a toggle button if \e enable is TRUE, or a normal
  push button if \e enable is FALSE.

  Toggle buttons have an on/off state similar to \link QCheckBox check
  boxes. \endlink A push button is initially not a toggle button.

  \sa setOn(), toggle(), toggleButton() toggled()
*/

void QPushButton::setToggleButton( bool enable )
{
    QButton::setToggleButton( enable );
}


/*!
  Switches a toggle button on if \e enable is TRUE or off if \e enable is
  FALSE.
  \sa isOn(), toggle(), toggled(), toggleButton()
*/

void QPushButton::setOn( bool enable )
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( enable );
}


/*!
  Toggles the state of a toggle button.
  \sa isOn(), setOn(), toggled(), toggleButton()
*/

void QPushButton::toggle()
{
    if ( !isToggleButton() )
	return;
    QButton::setOn( !isOn() );
}


/*!
  \fn bool QPushButton::autoDefault() const
  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
*/

/*!
  Sets the push buttons to an auto-default button if \e enable is TRUE,
  or to a normal button if \e enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
}


/*!
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is default.

  \sa setDefault()
*/

/*!
  Sets the button to be the default button if \e enable is TRUE, or
  to be a normal button if \e enable is FALSE.

  A default push button in a \link QDialog dialog\endlink emits the
  QButton::clicked() signal if the user presses the Enter key.	Only
  one push button in the dialog can be default.

  Default push buttons are only allowed in dialogs.

  \sa isDefault(), setAutoDefault(), QDialog
*/

void QPushButton::setDefault( bool enable )
{
    if ( (defButton && enable) || !(defButton || enable) )
	return;					// no change
    QWidget *p = this;
    while ( p && p->parentWidget() )		// get the top level parent
	p = p->parentWidget();
    if ( !p->inherits("QDialog") )		// not a dialog
	return;
    defButton = enable;
    if ( defButton )
	((QDialog*)p)->setDefault( this );
    int gs = style();
    if ( gs != MotifStyle ) {
	if ( isVisible() )
	    repaint( FALSE );
    } else {
	resizeDefButton( (QPushButton*)this );
    }
}


/*!
  Returns a size which fits the contents of the push button.
*/

QSize QPushButton::sizeHint() const
{
    int w, h;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()	 + 6;
	h = pm->height() + 6;
    } else {
	QFontMetrics fm = fontMetrics();
	QRect br = fm.boundingRect( text() );
	w = br.width()	+ 6;
	h = fm.height() + 6;
	w += h/8 + 10;
	h += h/8 + 4;
    }
    return QSize( w, h );
}


/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( int x, int y )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::move( x-wx/2, y-hx/2 );
}

/*!
  Reimplements QWidget::move() for internal purposes.
*/

void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::resize( w+wx, h+hx );
}

/*!
  Reimplements QWidget::resize() for internal purposes.
*/

void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( int x, int y, int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::setGeometry( x-wx/2, y-hx/2, w+wx, h+hx );
}

/*!
  Reimplements QWidget::setGeometry() for internal purposes.
*/

void QPushButton::setGeometry( const QRect &r )
{
    setGeometry( r.x(), r.y(), r.width(), r.height() );
}


/*!
  Draws the push button, except its label.
  \sa drawButtonLabel()
*/

void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    bool	updated = isDown() != (bool)lastDown
			  || lastDef != defButton
			  || isEnabled() != (bool)lastEnabled;
    QColor	fillcol = g.background();
    int		x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "$qt_push_%d_%d_%d_%d_%d_%d_%d_%d_%d", gs,
		   palette().serialNumber(), isDown(), defButton, w, h,
		   isToggleButton(), isOn(), isEnabled() );
    QPixmap *pm = QPixmapCache::find( pmkey );
    QPainter pmpaint;
    if ( pm ) {					// pixmap exists
	QPixmap pm_direct = *pm;
	pmpaint.begin( &pm_direct );
	pmpaint.drawPixmap( 0, 0, *pm );
	if ( text() )
	    pmpaint.setFont( font() );
	drawButtonLabel( &pmpaint );
	pmpaint.end();
	p->drawPixmap( 0, 0, pm_direct );
	lastDown = isDown();
	lastDef = defButton;
	lastEnabled = isEnabled();
	if ( hasFocus() ) {
	    if ( style() == WindowsStyle ) {
		p->drawWinFocusRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	    } else {
		p->setPen( black );
		p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
	    }
	}
	return;
    }
    bool use_pm = TRUE;
    if ( use_pm ) {
	pm = new QPixmap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( fillcol );
	p->eraseRect( 0, 0, w, h );
    }
#endif

    p->setPen( g.foreground() );
    p->setBrush( QBrush(fillcol,NoBrush) );

    if ( gs == WindowsStyle ) {		// Windows push button
	if ( isDown() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    } else {
		qDrawWinButton( p, x1, y1, w, h, g, TRUE );
	    }
	} else if ( isToggleButton() && isOn() && isEnabled() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE );
	    p->setPen( NoPen );
	    p->setBrush( QBrush( white, Dense4Pattern ) );
	    p->drawRect( x1+2, y1+2, x2-x1-3, y2-y1-3 );
	    updated = FALSE;
	} else {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    qDrawWinButton( p, x1, y1, x2-x1+1, y2-y1+1, g, isOn() );
	}
	if ( updated ) {
	    p->fillRect( x1+1, y1+1, x2-x1-2, y2-y1-2, g.background() );
	}
    } else if ( gs == MotifStyle ) {		// Motif push button
	if ( defButton ) {			// default Motif button
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE );
	    x1 += extraMotifWidth/2;
	    y1 += extraMotifHeight/2;
	    x2 -= extraMotifWidth/2;
	    y2 -= extraMotifHeight/2;
	}
	QBrush fill( fillcol );
	if ( isDown() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2,
			     updated ? &fill : 0 );
	} else if ( isToggleButton() && isOn() ) {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE, 2, 0 );
	    if ( updated ) {
		p->setPen( NoPen );
		p->setBrush( g.mid() );
		p->drawRect( x1+2, y1+2, x2-x1-3, y2-y1-3 );
	    }
	} else {
	    qDrawShadePanel( p, x1, y1, x2-x1+1, y2-y1+1, g, FALSE, 2,
			     updated ? &fill : 0 );
	}
    }
    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( 0, 0, *pm );
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    drawButtonLabel( p );
    if ( gs == MotifStyle && hasFocus() ) {
	p->setPen( black );
	p->drawRect( x1+3, y1+3, x2-x1-5, y2-y1-5 );
    }
    lastDown = isDown();
    lastDef = defButton;
    lastEnabled = isEnabled();
}


/*!
  Draws the push button label.
  \sa drawButton()
*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    int		dt = 0;
    switch ( gs ) {
    case WindowsStyle:
	dt = 1;
	break;
    case MotifStyle:
	break;
    default:
	;
    }
    QRect r = rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( isDown() || isOn() ) {			// shift pixmap/text
	x += dt;
	y += dt;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    p->setPen( g.text() );
    if ( pixmap() ) {
	const QPixmap *pm = pixmap();
	x += w/2 - pm->width()/2;		// center
	y += h/2 - pm->height()/2;
	if ( pm->width() > w || pm->height() > h )
	    p->setClipRect( x, y, w, h );
	p->setBackgroundColor( g.background() );
	p->setBackgroundMode( TransparentMode );
	if ( gs == WindowsStyle && !isEnabled() ) {
	    if ( !pm->isQBitmap() ) {
		if ( pm->mask() == 0 ) {
		    // detach and build a goodish mask -- slow!
		    QPixmap masked = *pm;
		    masked.detach();
		    masked.setMask( masked.isQBitmap() ?
				    *((QBitmap*)(&masked)) :
				    masked.reasonableMask() );
		    setPixmap( masked );
		}
		pm = pm->mask();
	    }
	    p->setBackgroundMode( TransparentMode );
	    p->setPen( white );
	    p->drawPixmap( x+1, y+1, *pm );
	    p->setPen( g.foreground() );
	}
	p->drawPixmap( x, y, *pm );
	p->setClipping( FALSE );
    } else if ( text() ) {
	if ( gs == WindowsStyle && !isEnabled() ) {
	    p->setPen( white );
	    p->drawText( x+1, y+1, w, h, AlignCenter|ShowPrefix, text() );
	    p->setPen( g.foreground() );
	}
	p->drawText( x, y, w, h, AlignCenter|ShowPrefix, text() );
    }
}


/*!
  Handles focus in events for the push button.
*/

void QPushButton::focusInEvent( QFocusEvent *e )
{
    if ( autoDefButton )
	setDefault( TRUE );
    QButton::focusInEvent( e );
}
