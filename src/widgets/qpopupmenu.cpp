/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.cpp#217 $
**
** Implementation of QPopupMenu class
**
** Created : 941128
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#define	 INCLUDE_MENUITEM_DEF
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qaccel.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtimer.h"
#include "qwhatsthis.h"
#include <ctype.h>

// Motif style parameters

static const int motifItemFrame		= 2;	// menu item frame width
static const int motifSepHeight		= 2;	// separator item height
static const int motifItemHMargin	= 3;	// menu item hor text margin
static const int motifItemVMargin	= 2;	// menu item ver text margin
static const int motifArrowHMargin	= 6;	// arrow horizontal margin
static const int motifArrowVMargin	= 2;	// arrow vertical margin
static const int motifTabSpacing	= 12;	// space between text and tab
static const int motifCheckMarkHMargin	= 2;	// horiz. margins of check mark


/*

+-----------------------------
|      PopupFrame
|   +-------------------------
|   |	   ItemFrame
|   |	+---------------------
|   |	|
|   |	|			   \
|   |	|   ^	T E X T	  ^	    | ItemVMargin
|   |	|   |		  |	   /
|   |	      ItemHMargin
|

*/




// used for internal communication - to be replaced with a class
// members in 2.0
static QPopupMenu * syncMenu = 0;
static int syncMenuId = 0;

// Used to detect motion prior to mouse-release
static int motion;

// used to provide ONE single-shot timer
static QTimer * singleSingleShot = 0;

static bool supressAboutToShow = FALSE;

static void popupSubMenuLater( int msec, QObject * receiver ) {
    if ( !singleSingleShot )
	singleSingleShot = new QTimer( qApp, "popup submenu timer" );
    singleSingleShot->disconnect( SIGNAL(timeout()) );
    QObject::connect( singleSingleShot, SIGNAL(timeout()),
		      receiver, SLOT(subMenuTimer()) );
    singleSingleShot->start( msec, TRUE );
}




/*!
  \class QPopupMenu qpopupmenu.h
  \brief The QPopupMenu class provides a popup menu widget.

  \ingroup realwidgets

  menu/menu.cpp is a typical example of QMenuBar and QPopupMenu use.

  \important insertItem clear text pixmap

  <img src=qpopmenu-m.gif> <img src=qpopmenu-w.gif>

  \sa QMenuBar
  <a href="guibooks.html#fowler">GUI Design Handbook: Menu, Drop-Down and
  Pop-Up</a>
*/


/*! \fn void QPopupMenu::aboutToShow()

  This signal is emitted just before the popup menu is displayed.  You
  can connect it to any slot that sets up the menu contents (e.g. to
  ensure that the right items are enabled).

  \sa setItemEnabled() setItemChecked() insertItem() removeItem()
*/



//
// Creates an accelerator string for the key k.
// For instance CTRL+Key_O gives "Ctrl+O".
//

QString QPopupMenu::accelString( int k )
{
    QString s;
    if ( (k & SHIFT) == SHIFT )
	s = tr( "Shift" );
    if ( (k & CTRL) == CTRL ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Ctrl" );
    }
    if ( (k & ALT) == ALT ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Alt" );
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( (k & ASCII_ACCEL) == ASCII_ACCEL ) {
	k &= ~ASCII_ACCEL;
	p.sprintf( "%c", (k & 0xff) );
    } else if ( k >= Key_F1 && k <= Key_F24 ) {
	p = tr( "F%1" ).arg(k - Key_F1 + 1);
    } else if ( k > Key_Space && k <= Key_AsciiTilde ) {
	p.sprintf( "%c", k );
    } else {
	switch ( k ) {
	    case Key_Space:
		p = tr( "Space" );
		break;
	    case Key_Escape:
		p = tr( "Esc" );
		break;
	    case Key_Tab:
		p = tr( "Tab" );
		break;
	    case Key_Backtab:
		p = tr( "Backtab" );
		break;
	    case Key_Backspace:
		p = tr( "Backspace" );
		break;
	    case Key_Return:
		p = tr( "Return" );
		break;
	    case Key_Enter:
		p = tr( "Enter" );
		break;
	    case Key_Insert:
		p = tr( "Ins" );
		break;
	    case Key_Delete:
		p = tr( "Del" );
		break;
	    case Key_Pause:
		p = tr( "Pause" );
		break;
	    case Key_Print:
		p = tr( "Print" );
		break;
	    case Key_SysReq:
		p = tr( "SysReq" );
		break;
	    case Key_Home:
		p = tr( "Home" );
		break;
	    case Key_End:
		p = tr( "End" );
		break;
	    case Key_Left:
		p = tr( "Left" );
		break;
	    case Key_Up:
		p = tr( "Up" );
		break;
	    case Key_Right:
		p = tr( "Right" );
		break;
	    case Key_Down:
		p = tr( "Down" );
		break;
	    case Key_Prior:
		p = tr( "PgUp" );
		break;
	    case Key_Next:
		p = tr( "PgDown" );
		break;
	    case Key_CapsLock:
		p = tr( "CapsLock" );
		break;
	    case Key_NumLock:
		p = tr( "NumLock" );
		break;
	    case Key_ScrollLock:
		p = tr( "ScrollLock" );
		break;
	    default:
		p.sprintf( "<%d?>", k );
		break;
	}
    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += tr( "+" );
	s += p;
    }
    return s;
}


/*****************************************************************************
  QPopupMenu member functions
 *****************************************************************************/

/*!
  Constructs a popup menu with a parent and a widget name.

  Although a popup menu is always a top level widget, if a parent is
  passed, the popup menu will be deleted on destruction of that parent
  (as with any other QObject).

*/

QPopupMenu::QPopupMenu( QWidget *parent, const char *name )
    : QTableView( parent, name, WType_Popup )
{
    isPopupMenu	  = TRUE;
    parentMenu	  = 0;
    selfItem	  = 0;
    autoaccel	  = 0;
    accelDisabled = FALSE;
    popupActive	  = -1;
    tabCheck	  = 0;
    hasDoubleItem = FALSE;
    maxPMWidth = 0;

    setTabMark( 0 );
    setNumCols( 1 );				// set number of table columns
    setNumRows( 0 );				// set number of table rows
    style().polishPopupMenu( this );
    setCheckableFlag( style() != WindowsStyle );
    setBackgroundMode( PaletteButton );
}

/*!
  Destroys the popup menu.
*/

QPopupMenu::~QPopupMenu()
{
    if ( syncMenu == this ) {
	qApp->exit_loop();
	syncMenu = 0;
    }
	
    delete autoaccel;
    if ( parentMenu )
	parentMenu->removePopup( this );	// remove from parent menu
}


void QPopupMenu::updateItem( int id )		// update popup menu item
{
    updateRow( indexOf(id) );
}


// Double use of tabMark / checkingEnabled until 2.0

/*!
  Enables or disables display of check marks by the menu items.

  Notice that checking is always enabled when in windows-style.

  \sa isCheckable(), QMenuData::setItemChecked()
*/

void QPopupMenu::setCheckable( bool enable )
{
    bool oldState = isCheckable();
    bool newState = (style() == WindowsStyle) || enable;
    if ( oldState != newState ) {
	setCheckableFlag( newState );
	if ( !newState ) {
	    // turning off isCheckable; must look for pixmaps
	    updateSize();
	}
    }
}


/*!
  Does the internal magic necessary to set the checkable flag.
 */
void QPopupMenu::setCheckableFlag( bool enable )
{
    bool oldState = isCheckable();
    bool newState = (style() == WindowsStyle) || enable;
    if ( oldState != newState ) {
	if ( newState ) {
	    setNumCols( 2 );
	    tabCheck |= 0x80000000;
	} else {
	    setNumCols( 1 );
	    tabCheck &= 0x7FFFFFFF;
	}
	badSize = TRUE;
	update();
    }
}




/*!
  Returns whether display of check marks by the menu items is enabled.

  \sa setCheckable(), QMenuData::setItemChecked()
*/

bool QPopupMenu::isCheckable() const
{
    return (tabCheck & 0x80000000) != 0;
}


void QPopupMenu::setTabMark( int t )
{
    bool e = isCheckable();
    tabCheck = t;
    if ( e )
	tabCheck |= 0x80000000;
}


int QPopupMenu::tabMark()
{
    return tabCheck & 0x7FFFFFFF;
}


void QPopupMenu::menuContentsChanged()
{
    badSize = TRUE;				// might change the size
    updateAccel( 0 );
    updateSize(); // ### SLOW, needs some rework
    if ( isVisible() ) {
	//	updateSize();
	repaint();
    }
}

void QPopupMenu::menuStateChanged()
{
    repaint();
}

void QPopupMenu::menuInsPopup( QPopupMenu *popup )
{
    popup->parentMenu = this;			// set parent menu
    connect( popup, SIGNAL(activatedRedirect(int)),
	     SLOT(subActivated(int)) );
    connect( popup, SIGNAL(highlightedRedirect(int)),
	     SLOT(subHighlighted(int)) );
}

void QPopupMenu::menuDelPopup( QPopupMenu *popup )
{
    popup->parentMenu = 0;
    popup->disconnect( SIGNAL(activatedRedirect(int)), this,
		       SLOT(subActivated(int)) );
    popup->disconnect( SIGNAL(highlightedRedirect(int)), this,
		       SLOT(subHighlighted(int)) );
}


void QPopupMenu::frameChanged()
{
    menuContentsChanged();
}


/*!
  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().
*/

void QPopupMenu::popup( const QPoint &pos, int indexAtPoint )
{
    //avoid circularity
    if ( isVisible() )
	return;

    if (parentMenu && parentMenu->actItem == -1){
	//reuse
	parentMenu->menuDelPopup( this );
	selfItem = 0;
	parentMenu = 0;
    }
    // #### should move to QWidget - anything might need this functionality,
    // #### since anything can have WType_Popup window flag.

    if ( mitems->count() == 0 )			// oops, empty
	insertSeparator();			// Save Our Souls
    if ( badSize )
	updateSize();
    QWidget *desktop = QApplication::desktop();
    int sw = desktop->width();			// screen width
    int sh = desktop->height();			// screen height
    int x  = pos.x();
    int y  = pos.y();
    if ( indexAtPoint > 0 )			// don't subtract when < 0
	y -= itemPos( indexAtPoint );		// (would subtract 2 pixels!)
    int w  = width();
    int h  = height();
    if ( x+w > sw )				// the complete widget must
	x = sw - w;				//   be visible
    if ( y+h > sh )
	y = sh - h;
    if ( x < 0 )
	x = 0;
    if ( y < 0 )
	y = 0;
    move( x, y );
    show();
    motion=0;
}

/*!
  \fn void QPopupMenu::activated( int id )

  This signal is emitted when a menu item is selected; \a id is the id
  of the selected item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa highlighted(), QMenuData::insertItem()
*/

/*!
  \fn void QPopupMenu::highlighted( int id )

  This signal is emitted when a menu item is highlighted; \a id is the
  id of the highlighted item.

  Normally, you will connect each menu item to a single slot using
  QMenuData::insertItem(), but sometimes you will want to connect
  several items to a single slot (most often if the user selects from
  an array).  This signal is handy in such cases.

  \sa activated(), QMenuData::insertItem()
*/

/*! \fn void QPopupMenu::highlightedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

/*! \fn void QPopupMenu::activatedRedirect( int id )
  \internal
  Used internally to connect submenus to their parents.
*/

void QPopupMenu::subActivated( int id )
{
    emit activatedRedirect( id );
}

void QPopupMenu::subHighlighted( int id )
{
    emit highlightedRedirect( id );
}

void QPopupMenu::accelActivated( int id )
{
    QMenuItem *mi = findItem( id );
    if ( mi && mi->isEnabled() ) {
	if ( mi->signal() ) // activate signal
	    mi->signal()->activate();
	actSig( mi->id() );
    }
}

void QPopupMenu::accelDestroyed()		// accel about to be deleted
{
    autoaccel = 0;				// don't delete it twice!
}


void QPopupMenu::actSig( int id, bool inwhatsthis )
{
    bool sync = FALSE;
    QPopupMenu * p = this;
    while( p && !sync ) {
	if ( p == syncMenu )
	    sync = TRUE;
	else if ( p->parentMenu && p->parentMenu->isPopupMenu )
	    p = (QPopupMenu*)(p->parentMenu);
	else
	    p = 0;
    }
    if ( sync && qApp ) {
	qApp->exit_loop();
	syncMenu = 0;
    }

    if ( !inwhatsthis )
	emit activated( id );
    else {
	int y = itemPos( indexOf( id ) ) + cellHeight( indexOf( id ) );
	QWhatsThis::leaveWhatsThisMode( findItem( id )->whatsThis(), mapToGlobal( QPoint(0,y) ) );
    }

    emit activatedRedirect( id );
}

void QPopupMenu::hilitSig( int id )
{
    emit highlighted( id );
    emit highlightedRedirect( id );
}


void QPopupMenu::setFirstItemActive()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    actItem = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( !mi->isSeparator() ) {
	    repaint( FALSE );
	    hilitSig( mi->id() );
	    return;
	}
	actItem++;
    }
    actItem = -1;
}

/*!
  \internal
  Hides all popup menus (in this menu tree) that are currently open.
*/

void QPopupMenu::hideAllPopups()
{
    register QMenuData *top = this;		// find top level popup
    while ( top->parentMenu && top->parentMenu->isPopupMenu )
	top = top->parentMenu;
    ((QPopupMenu*)top)->hide();			// cascade from top level
}

/*!
  \internal
  Hides all popup sub-menus.
*/

void QPopupMenu::hidePopups()
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() && mi->popup()->parentMenu == this ) //avoid circularity
	    mi->popup()->hide();
    }
    popupActive = -1;				// no active sub menu
}


/*!
  \internal
  Sends the event to the menu bar.
*/

bool QPopupMenu::tryMenuBar( QMouseEvent *e )
{
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    return top->isMenuBar ?
	((QMenuBar *)top)->tryMouseEvent( this, e ) : FALSE;
}

/*!
  \internal
  Tells the menu bar to go back to idle state.
*/

void QPopupMenu::byeMenuBar()
{
    hideAllPopups();
    register QMenuData *top = this;		// find top level
    while ( top->parentMenu )
	top = top->parentMenu;
    if ( top->isMenuBar )
	((QMenuBar *)top)->goodbye();
}


/*!
  \internal
  Return the item at \e pos, or -1 if there is no item there, or if
  it is a separator item.
*/

int QPopupMenu::itemAtPos( const QPoint &pos ) const
{
    int row = findRow( pos.y() );		// ask table for row
    int col = findCol( pos.x() );		// ask table for column
    int r = -1;
    if ( row != -1 && col != -1 ) {
	QMenuItem *mi = mitems->at(row);
	if ( !mi->isSeparator() )
	    r = row;				// normal item
    }
    return r;
}

/*!
  \internal
  Returns the y (top) position of item number \e index.
*/

int QPopupMenu::itemPos( int index )		// get y coord for item
{
    int y;
    if ( rowYPos( index, &y ) )			// ask table for position
	return y;
    else
	return 0;				// return 0 if not visible
}


/*!
  \internal
  Calculates and sets the size of the popup menu, based on the size
  of the items.
*/

void QPopupMenu::updateSize()
{
    int height	     = 0;
    int max_width    = 10;
    GUIStyle gs	  = style();
    const QFontMetrics & fm = fontMetrics();
#if 0
    QFontMetrics fm( font() );
#endif
    QMenuItemListIt it( *mitems );
    register QMenuItem *mi;
    bool hasSubMenu = FALSE;
    int cellh = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    int tab_width = 0;
    maxPMWidth = 0;
    bool oneHasIconSet = FALSE;
    while ( (mi=it.current()) ) {
	bool thisHasIconSet = mi->iconSet() != 0;
	oneHasIconSet = oneHasIconSet || thisHasIconSet;
	int w = 0;
	int itemHeight = internalCellHeight( mi );
	if ( mi->popup() )
	    hasSubMenu = TRUE;
	if ( mi->isSeparator() ) {
	}
	else if ( mi->pixmap() ) {
	    w = mi->pixmap()->width();	// pixmap only
	}
	
	if ( !mi->text().isNull() && !mi->isSeparator() ) {
	    if ( itemHeight < cellh )
		itemHeight = cellh;
	    QString s = mi->text();
	    int t;
	    if ( (t=s.find('\t')) >= 0 ) {	// string contains tab
		w = fm.width( s, t );
		int tw = fm.width( s.mid(t+1) );
		if ( tw > tab_width )
		    tab_width = tw;
	    } else {
		w += fm.width( s );
	    }
	}
	
	if ( thisHasIconSet ) {
	    maxPMWidth = QMAX( maxPMWidth,
			       mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() );
	}

	height += itemHeight;
#if defined(CHECK_NULL)
	if ( mi->text().isNull() && !mi->pixmap() && !mi->isSeparator() )
	    warning( "QPopupMenu: (%s) Popup has invalid menu item",
		     name( "unnamed" ) );
#endif
	if ( max_width < w )
	    max_width = w;
	++it;
    }

    if ( gs == MotifStyle ) {
	setCheckableFlag( isCheckable() || oneHasIconSet );
    }
    int extra_width = 0;
    if ( tab_width ) {
	extra_width = tab_width + motifTabSpacing;
	setTabMark( max_width + motifTabSpacing );
    }
    else
	setTabMark( 0 );

    max_width  += 2*motifItemHMargin;

    if ( isCheckable() )
	max_width += style().widthOfPopupCheckColumn( maxPMWidth ) + motifItemFrame;
    else
	max_width += 2*motifItemFrame;

    if ( hasSubMenu ) {
	if ( fm.ascent() + motifArrowHMargin > extra_width )
	    extra_width = fm.ascent() + motifArrowHMargin;
    }
    max_width += extra_width;
    setNumRows( mitems->count() );
    resize( max_width+2*frameWidth(), height+2*frameWidth() );
    badSize = FALSE;
}



/*!
  \internal
  The \e parent is 0 when it is updated when a menu item has
  changed a state, or it is something else if called from the menu bar.
*/

void QPopupMenu::updateAccel( QWidget *parent )
{
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    if ( parent == 0 && autoaccel == 0 )
	return;
    if ( autoaccel )				// build it from scratch
	autoaccel->clear();
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->key() ) {
	    if ( !autoaccel ) {
		autoaccel = new QAccel( parent );
		CHECK_PTR( autoaccel );
		connect( autoaccel, SIGNAL(activated(int)),
			 SLOT(accelActivated(int)) );
		connect( autoaccel, SIGNAL(destroyed()),
			 SLOT(accelDestroyed()) );
		if ( accelDisabled )
		    autoaccel->setEnabled( FALSE );
	    }
	    int k = mi->key();
	    int id = autoaccel->insertItem( k, mi->id() );
	    autoaccel->setWhatsThis( id, mi->whatsThis() );
	    if ( !mi->text().isNull() ) {
		QString s = mi->text();
		int i = s.find('\t');
		QString t = accelString( k );
		if ( i >= 0 )
		    s.replace( i+1, s.length()-i, t );
		else {
		    s += '\t';
		    s += t;
		}
		if ( s != mi->text() ) {
		    mi->setText( s );
		    badSize = TRUE;
		}
	    }
	}
	if ( mi->popup() && parent ) {		// call recursively
	    // reuse
	    QPopupMenu* popup = mi->popup();
	    if (!popup->avoid_circularity) {
		popup->avoid_circularity = 1;
		if (popup->parentMenu)
		    popup->parentMenu->menuDelPopup(popup);
		popup->selfItem  = mi;
		menuInsPopup(popup);
		popup->updateAccel( parent );
		popup->avoid_circularity = 0;
	    }
	}
    }
}

/*!
  \internal
  It would be better to check in the slot.
*/

void QPopupMenu::enableAccel( bool enable )
{
    if ( autoaccel )
	autoaccel->setEnabled( enable );
    else
	accelDisabled = TRUE;			// rememeber when updateAccel
    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {		// do the same for sub popups
	++it;
	if ( mi->popup() )			// call recursively
	    mi->popup()->enableAccel( enable );
    }
}


/*!
  Reimplements QWidget::setFont() to be able to refresh the popup menu
  when its font changes.
*/

void QPopupMenu::setFont( const QFont &font )
{
    QWidget::setFont( font );
    badSize = TRUE;
    update();
}

/*!
  Reimplements QWidget::show() for internal purposes.
*/

void QPopupMenu::show()
{
    if ( testWState(WState_Visible) ){
	supressAboutToShow = FALSE;
	return;
    }
    if (!supressAboutToShow)
	emit aboutToShow();
    else
	supressAboutToShow = FALSE;
    if ( badSize )
	updateSize();
    QWidget::show();
    popupActive = -1;
}

/*!
  Reimplements QWidget::hide() for internal purposes.
*/

void QPopupMenu::hide()
{
    if ( !isVisible() )
	return;
    actItem = popupActive = -1;
    mouseBtDn = FALSE;				// mouse button up
    hidePopups();
    killTimers();
    QWidget::hide();
    if ( syncMenu == this && qApp ) {
	qApp->exit_loop();
	syncMenu = 0;
    }
}

#if 0
/*!
  Reimplements QWidget::setEnabled() for internal purposes.
*/

void QPopupMenu::setEnabled( bool enable )
{
    if ( enable == isEnabled() )
	return;
    if ( parentMenu ) {
	QMenuItem *mi = parentMenu->findPopup( this );
	if ( mi ) {
	    parentMenu->setItemEnabled( mi->id(), enable );
	}
    }
    QWidget::setEnabled( enable );
}
#endif

/*****************************************************************************
  Implementation of virtual QTableView functions
 *****************************************************************************/

/*! \reimp */

int QPopupMenu::cellHeight( int row )
{
    QMenuItem *mi = mitems->at( row );
    return internalCellHeight( mi );
}

/*!\internal
 */
int QPopupMenu::internalCellHeight( QMenuItem* mi)
{
    int h = 0;
    if ( mi->isSeparator() ) {			// separator height
	h = motifSepHeight;
    } else if ( mi->pixmap() ) {		// pixmap height
	h = mi->pixmap()->height() + 2*motifItemFrame;
    } else {					// text height
	const QFontMetrics & fm = fontMetrics();
	h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
    }
    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
	h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height() + 2*motifItemFrame );
	if ( style() == MotifStyle )
 	    h += 2;				// Room for check rectangle
	const QFontMetrics & fm = fontMetrics();
	int h2 = fm.height() + 2*motifItemVMargin + 2*motifItemFrame;
	if ( h2 > h )
	    h = h2;
    }
    return h;
}

/*! \reimp */

int QPopupMenu::cellWidth( int col )
{
    if ( isCheckable() ) {
	if ( col == 0 )
	    return style().widthOfPopupCheckColumn( maxPMWidth );
	else
	    return width() - (2*frameWidth()+style().widthOfPopupCheckColumn( maxPMWidth ) );
    }	
    else
	return width() - 2*frameWidth();	
}


/*! \reimp */

void QPopupMenu::paintCell( QPainter *p, int row, int col )
{
    const QColorGroup & g = colorGroup();
    QMenuItem *mi = mitems->at( row );		// get menu item
    int cellh	  = cellHeight( row );
    int cellw	  = cellWidth( col );
    GUIStyle gs	  = style();
    bool act	  = row == actItem;
    bool dis	  = (selfItem && !selfItem->isEnabled()) || !mi->isEnabled();
    QColorGroup itemg = dis ? palette().disabled()
			: act ? palette().active()
			: palette().normal();

    if ( !mi->isDirty() )
	return;

    int rw = isCheckable() ? totalWidth() : cellw;

    if ( col == 0 ) {
	if ( mi->isSeparator() ) {			// draw separator
	    p->setPen( g.dark() );
	    p->drawLine( 0, 0, rw, 0 );
	    p->setPen( g.light() );
	    p->drawLine( 0, 1, rw, 1 );
	    return;
	}

	int cm = gs == MotifStyle ? 2 : 0;	// checkable margin

	if ( mi->isChecked() ) {
	    if ( gs == WindowsStyle && act && !dis ) {
		qDrawShadePanel( p, cm, cm, cellw-2*cm, cellh-2*cm,
				 g, TRUE, 1, &g.brush( QColorGroup::Mid ) );
	    } else if ( gs == WindowsStyle ||
			mi->iconSet() ) {
		qDrawShadePanel( p, cm, cm, cellw-2*cm, cellh-2*cm,
				 g, TRUE, 1, &g.brush( QColorGroup::Button ) );
	    }
	} else if ( !act ) {
 	    p->fillRect(cm, cm, cellw - 2*cm, cellh - 2*cm,
			g.brush( QColorGroup::Button ));
	}		

	if ( mi->iconSet() ) {		// draw iconset
	    QIconSet::Mode mode = dis?QIconSet::Disabled:QIconSet::Normal;
	    if ( style() == MotifStyle )
		mode = QIconSet::Normal; // no disabled icons in Motif
	    if (act && !dis )
		mode = QIconSet::Active;
	    QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    if ( gs == MotifStyle ) {
		if ( act && !dis ) {			// active item frame
		    if (style().defaultFrameWidth() > 1)
			qDrawShadePanel( p, 0, 0, rw, cellh, g, FALSE,
					 motifItemFrame,
					 &g.brush( QColorGroup::Button ) );
		    else
			qDrawShadePanel( p, 1, 1, rw-2, cellh-2, g, TRUE, 1,
					 &g.brush( QColorGroup::Button ) );
		}
		else				// incognito frame
		    p->fillRect(0,0,rw, cellh, g.brush( QColorGroup::Button ));
		// 		    qDrawPlainRect( p, 0, 0, rw, cellh, g.button(),
		// 				    motifItemFrame, &g.fillButton() );
	    } else {
		if ( act && !dis ) {
		    if ( !mi->isChecked() )
			qDrawShadePanel( p, 0, 0, cellw, cellh, g, FALSE, 1 );
		}
	    }
	    QRect cr( cm, cm, cellw-2*cm, cellh-2*cm );
	    QRect pmr( 0, 0, pixw, pixh );
	    pmr.moveCenter( cr.center() );
	    p->setPen( itemg.text() );
	    p->drawPixmap( pmr.topLeft(), pixmap );
	    if ( gs == WindowsStyle ) {
		QBrush fill = act? g.brush( QColorGroup::Highlight ) :
			      g.brush( QColorGroup::Button );
		p->fillRect( cellw + 1, 0, rw - cellw - 1, cellh, fill);
	    }
	    return;
	}

	int pw = motifItemFrame;
	if ( gs != MotifStyle )
	    pw = 1;
	if ( gs == WindowsStyle ) {
	    QBrush fill = act? g.brush( QColorGroup::Highlight ) :
			  g.brush( QColorGroup::Button );
	    if ( mi->isChecked() )
		p->fillRect( cellw + 1, 0, rw - cellw - 1, cellh, fill);
	    else
		p->fillRect( 0, 0, rw, cellh, fill);
	} else if ( gs == MotifStyle ) {
	    if ( act && !dis ) {			// active item frame
		if (style().defaultFrameWidth() > 1)
		    qDrawShadePanel( p, 0, 0, rw, cellh, g, FALSE, pw,
				     &g.brush( QColorGroup::Button ) );
		else
		    qDrawShadePanel( p, 1, 1, rw-2, cellh-2, g, TRUE, 1,
				     &g.brush( QColorGroup::Button ) );
	    }
	    else				// incognito frame
		p->fillRect(0, 0, rw, cellh, g.brush( QColorGroup::Button ));
	}

	if ( isCheckable() ) {	// just "checking"...
	    int mw = cellw - ( 2*motifCheckMarkHMargin + motifItemFrame );
	    int mh = cellh - 2*motifItemFrame;
	    if ( mi->isChecked() ) {
		style().drawPopupCheckMark( p, motifItemFrame + motifCheckMarkHMargin,
				motifItemFrame, mw, mh, itemg, act, dis );
	    }
	    return;
	}
    }

    if ( gs == WindowsStyle )
	p->setPen( act ? g.highlightedText() : g.buttonText() );
    else
	p->setPen( g.buttonText() );

    QColor discol;
    if ( dis ) {
	discol = itemg.text();
	p->setPen( discol );
    }

    int x = motifItemHMargin + ( isCheckable() ? 0 : motifItemFrame);
    QString s = mi->text();
    if ( !s.isNull() ) {			// draw text
	int t = s.find( '\t' );
	int m = motifItemVMargin;
	const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
	if ( t >= 0 ) {				// draw text before tab
	    if ( gs == WindowsStyle && dis && !act ) {
		p->setPen( g.light() );
		p->drawText( x+1, m+1, cellw, cellh-2*m, text_flags, s, t );
		p->setPen( discol );
	    }
	    p->drawText( x, m, cellw, cellh-2*m, text_flags, s, t );
	    s = s.mid(t+1);
	    x = tabMark();
	}
	if ( gs == WindowsStyle && dis && !act ) {
	    p->setPen( g.light() );
	    p->drawText( x+1, m+1, cellw, cellh-2*m, text_flags, s );
	    p->setPen( discol );
	}
	p->drawText( x, m, cellw, cellh-2*m, text_flags, s );
    } else if ( mi->pixmap() ) {			// draw pixmap
	QPixmap *pixmap = mi->pixmap();
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( OpaqueMode );
	p->drawPixmap( x, motifItemFrame, *pixmap );
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = (cellh-2*motifItemFrame) / 2;
	if ( gs == WindowsStyle && row == actItem ) {
	    if ( !dis )
		discol = white;
	    QColorGroup g2( discol, g.highlight(),
			    white, white,
			    dis ? discol : white,
			    discol, white );
	    style().drawArrow( p, RightArrow, FALSE,
			       cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			       dim, dim, g2, TRUE );
	} else if ( row == actItem ) {
	    style().drawArrow( p, RightArrow,
			       gs == MotifStyle && mi->isEnabled(),
			       cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			       dim, dim, g,
			       gs == MotifStyle && mi->isEnabled() ||
			       gs == WindowsStyle );
	} else {
	    style().drawArrow( p, RightArrow,
			       FALSE,
			       cellw - motifArrowHMargin - dim,  cellh/2-dim/2,
			       dim, dim, g, mi->isEnabled() );
	}
    }
    mi->setDirty( FALSE );
}


void QPopupMenu::paintAll()
{
    QMenuItemListIt it(*mitems);
    QMenuItem *mi;
    int row = 0;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->isDirty() )			// this item needs a refresh
	    updateRow( row );
	++row;
    }
}


/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*!
  Handles paint events for the popup menu.
*/

void QPopupMenu::paintEvent( QPaintEvent *e )
{
    setAllDirty( TRUE );
    QTableView::paintEvent( e );
    setAllDirty( FALSE );
}

/*!
  Handles close events for the popup menu.
*/

void QPopupMenu::closeEvent( QCloseEvent * e) {
    e->ignore();
    hide();
     if ( parentMenu && parentMenu->isMenuBar )
 	byeMenuBar();
}


/*!
  Handles mouse press events for the popup menu.
*/

void QPopupMenu::mousePressEvent( QMouseEvent *e )
{
    mouseBtDn = TRUE;				// mouse button down
    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains(e->pos()) && !tryMenuBar(e) ) {
	    byeMenuBar();
	}
	return;
    }
    register QMenuItem *mi = mitems->at(item);
    if ( item != actItem ) {			// new item activated
	actItem = item;
	repaint( FALSE );
	hilitSig( mi->id() );
    }
    QPopupMenu *popup = mi->popup();
    if ( popup ) {
	if ( popup->isVisible() ) {		// sub menu already open
	    popup->actItem = -1;
	    popup->hidePopups();
	    popup->repaint( FALSE );
	} else {				// open sub menu
	    hidePopups();
	    popupSubMenuLater( 20, this );
	}
    } else {
	hidePopups();
    }
}

/*!
  Handles mouse release events for the popup menu.
*/

void QPopupMenu::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mouseBtDn && !parentMenu && actItem < 0 && motion < 5 )
	return;

    mouseBtDn = FALSE;

    int item = itemAtPos( e->pos() );
    if ( item == -1 ) {
	if ( !rect().contains( e->pos() ) && tryMenuBar(e) )
	    return;
    }
    repaint( FALSE );
    if ( actItem >= 0 ) {			// selected menu item!
	register QMenuItem *mi = mitems->at(actItem);
	QPopupMenu *popup = mi->popup();
	bool b = QWhatsThis::inWhatsThisMode();
	if ( !mi->isEnabled() ) {
	    if ( b ) {
		byeMenuBar();
		actSig( mi->id(), b);
	    }
	} else 	if ( popup ) {
	    popup->setFirstItemActive();
	} else {				// normal menu item
	    byeMenuBar();			// deactivate menu bar
	    if ( mi->isEnabled() ) {
		if ( mi->signal() && !b ) // activate signal
		    mi->signal()->activate();
		actSig( mi->id(), b );
	    }
	}
    } else {
	byeMenuBar();
    }
}

/*!
  Handles mouse move events for the popup menu.
*/

void QPopupMenu::mouseMoveEvent( QMouseEvent *e )
{
    motion++;
    if ( parentMenu && parentMenu->isPopupMenu &&
	 (parentMenu->actItem != ((QPopupMenu *)parentMenu)->popupActive ) ) {
	// hack it to work: if there's a parent popup, and its active
	// item is not the same as its popped-up child, make the
	// popped-up child active
	QPopupMenu * p = (QPopupMenu *)parentMenu;
	int lastActItem = p->actItem;
	p->actItem = p->popupActive;
	if ( lastActItem >= 0 )
	    p->updateRow( lastActItem );
	if ( p->actItem >= 0 )
	    p->updateRow( p->actItem );
    }

    if ( (e->state() & Qt::MouseButtonMask) == 0 &&
	 !hasMouseTracking() )
	return;

    int	 item = itemAtPos( e->pos() );
    if ( item == -1 ) {				// no valid item
	int lastActItem = actItem;
	actItem = -1;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );

	if ( !rect().contains( e->pos() ) && !tryMenuBar( e ) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();
    } else {					// mouse on valid item
	// but did not register mouse press
	if ( (e->state() & Qt::MouseButtonMask) && !mouseBtDn )
	    mouseBtDn = TRUE; // so mouseReleaseEvent will pop down

	register QMenuItem *mi = mitems->at( item );
	if ( actItem == item )
	    return;

	if ( mi->popup() || (popupActive >= 0 && popupActive != item) )
	    popupSubMenuLater( style() == WindowsStyle ? 256 : 96, this );
	else if ( singleSingleShot )
	    singleSingleShot->stop();

	int lastActItem = actItem;
	actItem = item;
	if ( lastActItem >= 0 )
	    updateRow( lastActItem );
	updateRow( actItem );
	hilitSig( mi->id() );
    }
}


/*!
  Handles key press events for the popup menu.
*/

void QPopupMenu::keyPressEvent( QKeyEvent *e )
{
    QMenuItem  *mi = 0;
    QPopupMenu *popup;
    int d = 0;
    bool ok_key = TRUE;

    switch ( e->key() ) {
    case Key_Tab:
	// ignore tab, otherwise it will be passed to the menubar
	break;
	
    case Key_Up:
	d = -1;
	break;

    case Key_Down:
	d = 1;
	break;

    case Key_Alt:
	if ( style() == WindowsStyle ) {
	    byeMenuBar();
	}
	break;

    case Key_Escape:
	// just hide one
	hide();
	if ( parentMenu && parentMenu->isMenuBar )
	    ((QMenuBar *)parentMenu)->setWindowsAltMode( FALSE, -1);
	break;

    case Key_Left:
	if ( parentMenu && parentMenu->isPopupMenu ) {
	    ((QPopupMenu *)parentMenu)->hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Right:
	if ( actItem >= 0 && (popup=mitems->at( actItem )->popup()) ) {
	    hidePopups();
	    if ( singleSingleShot )
		singleSingleShot->stop();
	    popup->setFirstItemActive();
	    subMenuTimer();
	} else {
	    ok_key = FALSE;
	}
	break;

    case Key_Space:
	if ( style() != MotifStyle )
	    break;
	// for motif, fall through

    case Key_Return:
    case Key_Enter:
	if ( actItem < 0 )
	    break;
	mi = mitems->at( actItem );
	popup = mi->popup();
	if ( popup ) {
	    hidePopups();
	    popupSubMenuLater( 20, this );
	    popup->setFirstItemActive();
	} else {
	    byeMenuBar();
	    bool b = QWhatsThis::inWhatsThisMode();
	    if ( mi->isEnabled() || b ) {
		if ( mi->signal() && !b ) // activate signal
		    mi->signal()->activate();
		actSig( mi->id(), b );
	    }
	}
	break;

    case Key_F1:
	if ( actItem < 0 || e->state() != ShiftButton)
	    break;
	mi = mitems->at( actItem );
	if ( !mi->whatsThis().isNull() ){
	    if ( !QWhatsThis::inWhatsThisMode() )
		QWhatsThis::enterWhatsThisMode();
	    int y = itemPos( actItem) + cellHeight( actItem );
	    QWhatsThis::leaveWhatsThisMode( mi->whatsThis(), mapToGlobal( QPoint(0,y) ) );
	}
    default:
	ok_key = FALSE;

    }
#if 1
    if ( !ok_key && !e->state() && e->key() >= Key_0 && e->key() <= Key_Z ) {
	char c = '0' + e->key() - Key_0;

	QMenuItemListIt it(*mitems);
	register QMenuItem *m;
	int indx = 0;
	while ( (m=it.current()) ) {
	    ++it;
	    QString s = m->text();
	    if ( !s.isEmpty() ) {
		int i = s.find( '&' );
		if ( i >= 0 &&
			( s[i+1].isLetter() || s[i+1].isNumber() ) ) {
		    if ( s[i+1].upper() == c ) {
			mi = m;
			ok_key = TRUE;
			break;
		    }
		}
	    }
	    indx++;
	}
	if ( mi ) {
	    popup = mi->popup();
	    if ( popup ) {
		setActiveItem( indx );
		hidePopups();
		popupSubMenuLater( 20, this );
		popup->setFirstItemActive();
	    } else {
		byeMenuBar();
		bool b = QWhatsThis::inWhatsThisMode();
		if ( mi->isEnabled() || b ) {
		    if ( mi->signal() && !b  ) // activate signal
			mi->signal()->activate();
		    actSig( mi->id(), b );
		}
	    }
	}
    }
#endif
    if ( !ok_key ) {				// send to menu bar
	register QMenuData *top = this;		// find top level
	while ( top->parentMenu )
	    top = top->parentMenu;
	if ( top->isMenuBar )
	    ((QMenuBar*)top)->tryKeyEvent( this, e );
    }

    if ( d && actItem < 0 ) {
	setFirstItemActive();
    } else if ( d ) {				// highlight next/prev
	register int i = actItem;
	int c = mitems->count();
	int n = c;
	while ( n-- ) {
	    i = i + d;
	    if ( i == c )
		i = 0;
	    else if ( i < 0 )
		i = c - 1;
	    mi = mitems->at( i );
	    if ( !mi->isSeparator()
		 && ( style() != MotifStyle || mi->isEnabled() ) )
		break;
	}
	if ( i != actItem ) {
	    int lastActItem = actItem;
	    actItem = i;
	    if ( mi->id() != 0 )
		hilitSig( mi->id() );
	    updateRow( lastActItem );
	    updateRow( actItem );
	}
    }
}


/*!
  Handles timer events for the popup menu.
*/

void QPopupMenu::timerEvent( QTimerEvent *e )
{
    QTableView::timerEvent( e );
}

/*!
  Reimplemented for internal purposes.
*/
void  QPopupMenu::styleChange( GUIStyle )
{
    style().polishPopupMenu( this );
    setCheckableFlag( style() != WindowsStyle );
}


/*! This private slot handles the delayed submenu effects */

void QPopupMenu::subMenuTimer() {
    if ( (actItem < 0 && popupActive < 0) || actItem == popupActive )
	return;

    if ( popupActive >= 0 ) {
	hidePopups();
	popupActive = -1;
    }

    QMenuItem *mi = mitems->at(actItem);
    if ( !mi || !mi->isEnabled() )
	return;

    QPopupMenu *popup = mi->popup();
    if ( !popup || !popup->isEnabled() )
	return;

    //avoid circularity
    if ( popup->isVisible() )
	return;

    if (popup->parentMenu != this ){
	// reuse
	if (popup->parentMenu)
	    popup->parentMenu->menuDelPopup(popup);
	popup->selfItem  = mi;
	menuInsPopup(popup);
    }

    emit popup->aboutToShow();
    supressAboutToShow = TRUE;

    QPoint p( width() - motifArrowHMargin, frameWidth() + motifArrowVMargin );
    for ( int i=0; i<actItem; i++ )
	p.setY( p.y() + (QCOORD)cellHeight( i ) );
    p = mapToGlobal( p );
    if ( popup->badSize )
	popup->updateSize();
    if (p.y() + popup->height() > QApplication::desktop()->height()
	&& p.y() - popup->height()
	+ (QCOORD)(popup->cellHeight( popup->count()-1)) >= 0)
	p.setY( p.y() - popup->height()
		+ (QCOORD)(popup->cellHeight( popup->count()-1)));
    popupActive = actItem;
    bool left = FALSE;
    if ( ( parentMenu && parentMenu->isPopupMenu &&
	   ((QPopupMenu*)parentMenu)->geometry().x() > geometry().x() ) ||
	 p.x() + popup->width() > QApplication::desktop()->width() )
	left = TRUE;
    if ( left && popup->width() > mapToGlobal( QPoint(0,0) ).x() )
	left = FALSE;
    if ( left )
	p.setX( mapToGlobal(QPoint(0,0)).x() - popup->width() + frameWidth() );
    popup->popup( p );
}


void QPopupMenu::updateRow( int row )
{
    updateCell( row, 0, FALSE );
    if ( isCheckable() )
	updateCell( row, 1, FALSE );
}


/*!  Execute this popup synchronously.

  Opens the popup menu so that the item number \a indexAtPoint will be
  at the specified \e global position \a pos.  To translate a widget's
  local coordinates into global coordinates, use QWidget::mapToGlobal().

  The return code is the ID of the selected item in either the popup
  menu or one of its submenus, or -1 if no item is selected (normally
  because the user presses Escape).

  Note that all signals are emitted as usual.  If you connect a menu
  item to a slot and call the menu's exec(), you get the result both
  via the signal-slot connection and in the return value of exec().

  Common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
  \sa popup()
*/

int QPopupMenu::exec( const QPoint & pos, int indexAtPoint )
{
    if ( !qApp )
	return -1;

    syncMenu = this;
    syncMenuId = -1;

    connectModal( this, TRUE );
    popup( pos, indexAtPoint );
    qApp->enter_loop();
    connectModal( this, FALSE );

    syncMenu = 0;
    return syncMenuId;
}



/*
  connect the popup and all its submenus to modalActivation() if
  \a doConnect is true, otherwise disconnect.
 */
void QPopupMenu::connectModal( QPopupMenu* receiver, bool doConnect )
{
    if ( doConnect )
	connect( this, SIGNAL(activated(int)),
		 receiver, SLOT(modalActivation(int)) );
    else
	disconnect( this, SIGNAL(activated(int)),
		    receiver, SLOT(modalActivation(int)) );

    QMenuItemListIt it(*mitems);
    register QMenuItem *mi;
    while ( (mi=it.current()) ) {
	++it;
	if ( mi->popup() && mi->popup() != receiver && mi->popup()->parentMenu == this ) //avoid circularity
	    mi->popup()->connectModal( receiver, doConnect );
    }
}


/*!  Execute this popup synchronously.

  Similar to the above function, but the position of the
  popup is not set, so you must choose an appropriate position.
  The function move the popup if it is partially off-screen.

  More common usage is to position the popup at the current
  mouse position:
  \code
      exec(QCursor::pos());
  \endcode
  or aligned to a widget:
  \code
      exec(somewidget.mapToGlobal(QPoint(0,0)));
  \endcode
*/

int QPopupMenu::exec()
{
    return exec(mapToGlobal(QPoint(0,0)));
}


/*!  Internal slot used for exec(). */

void QPopupMenu::modalActivation( int id )
{
    syncMenuId = id;
}


/*!  Sets the currently active item to \a i and repaints as necessary.
*/

void QPopupMenu::setActiveItem( int i )
{
    int lastActItem = actItem;
    actItem = i;
    if ( lastActItem >= 0 )
	updateRow( lastActItem );
    if ( i >= 0 && i != lastActItem )
	updateRow( i );
}


/*!
  Return the id of the item at \e pos, or -1 if there is no item
  there, or if it is a separator item.
 */
int QPopupMenu::idAt( const QPoint& pos ) const
{
    return idAt( itemAtPos( pos ) );
}


/*!\fn int QPopupMenu::idAt( int index ) const

  Returns the identifier of the menu item at position \a index in the internal
  list, or -1 if \a index is out of range.

  \sa QMenuData::setId(), QMenuData::indexOf()
*/


/*!\reimp
 */
bool QPopupMenu::customWhatsThis() const
{
    return TRUE;
}
