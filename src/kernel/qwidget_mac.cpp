/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_mac.cpp $
**
** Implementation of QWidget and QWindow classes for mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qt_mac.h"

#include "qimage.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qabstractlayout.h"
#include "qtextcodec.h"
#include <stdio.h>
#include <qcursor.h>

// NOT REVISED

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/* THIS IS FOR PASCAL STYLE STRINGS, WE NEED TO FIGURE OUT IF THESE ARE FREED WHEN HANDED OVER
   TO THE OS FIXME FIXME FIXME FIXME */
//this function really sucks, re-write me when you'r edone figuring out
const unsigned char * p_str(const char * c)
{
    unsigned char * ret=new unsigned char[qstrlen(c)+2];
    ret[0]=qstrlen(c);
    qstrcpy(((char *)ret)+1,c);
    return ret;
}

QPoint posInWindow(QWidget *w)
{
  int x = 0, y = 0;
  for(QWidget *p = w; p && !p->isTopLevel(); p = p->parentWidget()) {
    x += p->x();
    y += p->y();
  }
  return QPoint(x, y);
}

static void paint_children(QWidget * p,const QRegion& r)
{
    if(!p || r.isEmpty())
	return;

#if 0
    Rect rect;
    SetRect( &rect, p->x(), p->y(), p->x()+p->width(), p->y()+p->height() );
    InvalWindowRect( (WindowRef)p->topLevelWidget()->winId(), &rect );
#else
    QApplication::postEvent(p,new QPaintEvent(r, !p->testWFlags(QWidget::WRepaintNoErase) ) );

    QObjectList * childObjects=(QObjectList*)p->children();
    if(childObjects) {
	QObject * o;
	for(o=childObjects->first();o!=0;o=childObjects->next()) {
	    if( o->isWidgetType() ) {
		QWidget *w = (QWidget *)o;
		if ( w->testWState(Qt::WState_Visible) ) {
		    QRegion wr = QRegion(w->geometry()) & r;
		    if ( !wr.isEmpty() ) {
			wr.translate(-w->x(),-w->y());
			paint_children(w,wr);
		    }
		}
	    }
	}
    }
#endif
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping( QPaintDevice *dev );

static WId serial_id = 0;
static WId parentw;
WId myactive = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;

static WId qt_root_win() {
    WindowPtr ret = NULL; 
#if 0
    //my desktop hacks, trying to figure out how to get a desktop, this doesn't work
    //but I'm going to leave it for now so I can test some more FIXME!!!
    GetCWMgrPort(ret);
#else
    //FIXME NEED TO FIGURE OUT HOW TO GET DESKTOP ON MACX
#endif
    return (WId) ret;
}

//FIXME How can I create translucent windows? (Need them for pull down menus)
//FIXME Is this even possible with the Carbon API? (You can't do it on OS9)
//FIXME Perhaps we need to access the lower level Quartz API?
//FIXME Documentation on Quartz, where is it?
void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow  )
{
    bg_pix = 0;
    own_id = 0;
    WId root_win = qt_root_win();
    WId destroyw = 0;
    setWState( WState_Created );                        // set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );            // top-level widget

    static int sw = -1, sh = -1;                // screen size
    bool topLevel = testWFlags( WType_TopLevel );
    bool popup = testWFlags( WType_Popup );
    bool modal = testWFlags( WType_Modal );
    bool desktop = testWFlags( WType_Desktop );
    WId    id;

    if ( !window )                              // always initialize
	initializeWindow=TRUE;

    //FIXME need to query the display for characteristics
    if ( sw < 0 ) {
	sw = 1024;    // Make it up
	sh = 768;
    }

    bg_col = pal.normal().background();

    if ( modal || popup || desktop ) {          // these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    Rect boundsRect;

    if ( desktop ) {                            // desktop widget
	modal = popup = FALSE;                  // force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {                    // calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {                                    // child widget
	crect.setRect( 0, 0, 100, 30 );
    }
    fpos = crect.topLeft();                     // default frame rect

    parentw = topLevel ? root_win : parentWidget()->winId();

    SetRect( &boundsRect, crect.left(), crect.top(),
	     crect.right(), crect.bottom());

    if ( !topLevel ) {
	if ( !testWFlags(WStyle_Customize) )
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );
    } else if ( !(desktop || popup) ) {
	if ( testWFlags(WStyle_Customize) ) {	// customize top-level widget
	    if ( testWFlags(WStyle_NormalBorder) ) {
		// XXX ...
	    } else {
		if ( !testWFlags(WStyle_DialogBorder) ) {
		    // XXX ...
		}
	    }
	    if ( testWFlags(WStyle_Tool) ) {
		// XXX ...
	    }
	} else {				// normal top-level widget
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }

    char title[2];
    title[0]=0;
    title[1]='\0';
    unsigned char visible=0;
    short procid;
    if ( popup || testWFlags(WStyle_Tool ) ) {
	procid = plainDBox;
    } else {
	procid = zoomDocProc;
    }

    WindowPtr behind = (WindowPtr)-1;
    unsigned char goaway=true;

    if ( window ) {				// override the old window
	if ( destroyOldWindow && own_id )
	    destroyw = winid;
	own_id = 1; //it has become mine!
	id = window;
	hd = (void *)id;
	setWinId(id);
    } else if ( desktop ) {			// desktop widget
	id = (WId)parentw;			// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else if( !parentWidget() || (popup || modal) ) {
	own_id = 1; //I created it, I own it
	SetRect( &boundsRect, 50, 50, 600, 200 );
	id = (WId)NewCWindow( nil, &boundsRect, (const unsigned char*)title,
			      visible, procid, behind, goaway, 0);
	hd = (void *)id;
	qDebug("Creating %s %d", name(), id);
	setWinId(id);
    } else {
	while(QWidget::find(++serial_id));
	setWinId(serial_id);
	id = serial_id;
	hd = topLevelWidget()->hd;
	setWinId(id);
    }

    bg_col = pal.normal().background();

    const char *c = name();
    if( c && isTopLevel()) {
	setCaption( QString( c ));
    }

    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );                  // also sets event mask
    clearWState(WState_Visible);
    dirtyClippedRegion(TRUE);
    macDropEnabled = false;

    if ( destroyw ) 
	DisposeWindow((WindowPtr)destroyw);
}

void qt_mac_destroy_widget(QWidget *w);

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
    if ( testWState(WState_Created) ) {
	dirtyClippedRegion(TRUE);
        clearWState( WState_Created );
        if ( children() ) {
            QObjectListIt it(*children());
            register QObject *obj;
            while ( (obj=it.current()) ) {      // destroy all widget children
                ++it;
                if ( obj->isWidgetType() )
                    ((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
            }
        }
	if ( mac_mouse_grabber == this )
	    releaseMouse();
	if ( mac_keyboard_grabber == this )
	    releaseKeyboard();
	if ( acceptDrops() )
	    setAcceptDrops(FALSE);

        if ( testWFlags(WType_Modal) )          // just be sure we leave modal
            qt_leave_modal( this );
        else if ( testWFlags(WType_Popup) )
            qApp->closePopup( this );
	if ( testWFlags(WType_Desktop) ) {
	} else {
	    if ( destroyWindow && isTopLevel() && hd && own_id) 
	        DisposeWindow( (WindowPtr)hd );
	}

    }
    QWidget * mya;
    mya=QWidget::find(myactive);
    if(mya==this) {
	myactive=0;
    }
    hd=0;
    setWinId( 0 );
    qt_mac_destroy_widget(this);
}

void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    QCursor oldcurs;
    bool setcurs=testWState(WState_OwnCursor);
    if ( setcurs ) {
	oldcurs = cursor();
	unsetCursor();
    }

    WId old_winid = (WId)hd;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;

    reparentFocusWidgets( parent );		// fix focus chains

    setWinId( 0 );
    if ( parentObj ) {				// remove from parent
	QObject *oldp = parentObj;
	parentObj->removeChild( this );
	if(oldp->isWidgetType()) 
	    paint_children( ((QWidget *)oldp),geometry() );
    }

    if ( old_winid && own_id && isTopLevel() ) 
	DisposeWindow( (WindowPtr)old_winid );

    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    if ( isTopLevel() || (!parent || parent->isVisibleTo( 0 ) ) )
	setWState( WState_ForceHide );	// new widgets do not show up in already visible parents 
    if(dropable) 
	setAcceptDrops(FALSE);
    create();
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    setFocusPolicy( fp );
    setAcceptDrops(dropable);
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( setcurs ) {
	setCursor(oldcurs);
    }

    QObjectList	*accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
	++it;
	((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
    if ( !parent ) {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
    }
    QEvent e( QEvent::Reparent );
    QApplication::sendEvent( this, &e );
}


QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
  Point mac_p;
  QPoint mp(posInWindow(((QWidget *)this)));
  mac_p.h = mp.x() + pos.x();
  mac_p.v = mp.y() + pos.y();
  if(handle()) {
    SetPortWindowPort((WindowPtr)handle());
    LocalToGlobal(&mac_p);
  }
  return QPoint(mac_p.h, mac_p.v);
}


QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
  Point mac_p;
  mac_p.h = pos.x();
  mac_p.v = pos.y();
  if(handle()) {
    SetPortWindowPort((WindowPtr)handle());
    GlobalToLocal(&mac_p);
  }
  for(const QWidget *p = this; p && !p->isTopLevel(); p = p->parentWidget()) {
    mac_p.h -= p->x();
    mac_p.v -= p->y();
  }
  return QPoint(mac_p.h, mac_p.v);
}


void QWidget::setMicroFocusHint(int, int, int, int, bool )
{
}

void QWidget::setFontSys()
{
}


void QWidget::setBackgroundColorDirect( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    backgroundColorChange( old );
}

static int allow_null_pixmaps = 0;

void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	QPixmap pm = pixmap;
	if (!pm.isNull()) {
	    if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
		pm = QPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pm );
    }
    if ( !allow_null_pixmaps ) {
	backgroundPixmapChange( old );
    }
}


void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}


void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle() || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
	extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
}


void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( WState_OwnCursor );
    }
}

void QWidget::setCaption( const QString &cap )
{
    if ( extra && extra->topextra && extra->topextra->caption == cap )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = cap;
    if(isTopLevel())
	SetWTitle((WindowPtr)hd, p_str(cap.latin1()));
    QEvent e( QEvent::CaptionChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    QBitmap mask;
    if ( pixmap.isNull() ) {
    } else {
	extra->topextra->icon = new QPixmap( pixmap );
	mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
    }
}

void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
}


void QWidget::grabMouse()
{
    mac_mouse_grabber=this;
}

void QWidget::grabMouse( const QCursor & )
{
    mac_mouse_grabber=this;
}

void QWidget::releaseMouse()
{
    mac_mouse_grabber = NULL;
}

void QWidget::grabKeyboard()
{
    mac_keyboard_grabber = this;
}

void QWidget::releaseKeyboard()
{
    mac_keyboard_grabber = NULL;
}


QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}


void QWidget::setActiveWindow()
{
    QWidget *widget = QWidget::find( myactive );
    if(!widget || !widget->isVisible())
	return;

    // FIXME: This is likely to flicker
    if ( widget->isPopup() )
        widget = NULL;

    if ( isTopLevel() ) {
	SelectWindow( (WindowPtr)hd );  // FIXME: Also brings to front - naughty?
	update();
	myactive = (WId) hd;
    }
    if ( widget && !isPopup() )
	widget->setActiveWindow();
}


void QWidget::update()
{
    update( 0, 0, width(), height() );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( !testWState(WState_BlockUpdates) && testWState( WState_Visible ) && isVisible() ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w && h ) {
#if 0
		QPoint mp(posInWindow(this));
		x += mp.x();
		y += mp.y();

		Rect r;
		SetRect( &r, x, y, x+w, y+h );
		InvalWindowRect( (WindowRef)hd, &r );
#else
		QApplication::postEvent( this, new QPaintEvent( QRect(x, y, w, h), !testWFlags( WRepaintNoErase ) ) );
#endif
	}
    }
}

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    QRect r(x,y,w,h);
    if ( r.isEmpty() )
	return; // nothing to do
    repaint(QRegion(r), erase); //general function..
}

void QWidget::repaint( const QRegion &reg , bool erase )
{
    if ( !testWState(WState_BlockUpdates) && testWState(WState_Visible) && isVisible() ) {
	qt_set_paintevent_clipping( this, reg );
	if ( erase )
	    this->erase(reg);

	QPaintEvent e( reg );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping( this );
    }

#if 0 //this is good for debugging and not much else, do not leave this in production
    GWorldPtr cgworld;
    GDHandle cghandle;
    GetGWorld(&cgworld, &cghandle);
    QDFlushPortBuffer(cgworld, (RgnHandle)reg.handle());
#endif
}

void QWidget::showWindow()
{
    dirtyClippedRegion(TRUE);
    if ( isTopLevel() ) {
	ShowHide( (WindowPtr)winid, 1 );
	setActiveWindow();
    }
    update();
}

void QWidget::hideWindow()
{
    deactivateWidgetCleanup();
    dirtyClippedRegion(TRUE);

    if ( isTopLevel() ) {
	ShowHide((WindowPtr)winid,0);
    } else {
	bool v = testWState(WState_Visible);
	clearWState(WState_Visible);
	paint_children( parentWidget(),geometry() );
	if ( v )
	    setWState(WState_Visible);
    }
}



bool QWidget::isMinimized() const
{
    return FALSE;
}

bool QWidget::isMaximized() const
{
    return testWState(WState_Maximized);
}


void QWidget::showMinimized()
{
    if ( isTopLevel() ) {
	if ( isVisible() ) { 
	    qDebug("showMinimized need to do this %s:%d", __FILE__, __LINE__);
        } else {
	    topData()->showMode = 1;
	    show();
	    clearWState( WState_Visible );
	    sendHideEventsToChildren(TRUE);
	}
    }
    QEvent e( QEvent::ShowMinimized );
    QApplication::sendEvent( this, &e );
}

void QWidget::showMaximized()
{
    if ( testWFlags(WType_TopLevel) ) {
	qDebug("showMaximized need to do this %s:%d", __FILE__, __LINE__);
    }
    show();
    QEvent e( QEvent::ShowMaximized );
    QApplication::sendEvent( this, &e );
    setWState(WState_Maximized);
}

void QWidget::showNormal()
{
    if ( isTopLevel() ) {
	if ( topData()->fullscreen ) {
	    reparent( 0, WType_TopLevel, QPoint(0,0) );
	    topData()->fullscreen = 0;
	}
	QRect r = topData()->normalGeometry;
	if ( r.width() >= 0 ) {
	    // the widget has been maximized
	    topData()->normalGeometry = QRect(0,0,-1,-1);
	    resize( r.size() );
	    move( r.topLeft() );
	}
    }
    show();
    QEvent e( QEvent::ShowNormal );
    QApplication::sendEvent( this, &e );
}


void QWidget::raise()
{
    if(isTopLevel())
	SelectWindow((WindowPtr)hd);
}

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    if ( isTopLevel() )
	; //how do I lower a window?? FIXME
    else if(p)
	update(geometry());
}


void QWidget::stackUnder( QWidget *w )
{
    QWidget *p = parentWidget();
    if ( !p || !w || isTopLevel() || p != w->parentWidget() )
	return;
    int loc = p->childObjects->findRef(w);
    if ( loc >= 0 && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( loc, p->childObjects->take() );
    // #### excessive repaints
    update( geometry() );
    w->update ( w->geometry() );
}


void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    if ( testWFlags(WType_Desktop) )
	return;
    dirtyClippedRegion(TRUE);

    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);

	// Deal with size increment
	if ( extra->topextra ) {
	    if ( extra->topextra->incw ) {
		w = w/extra->topextra->incw;
		w = w*extra->topextra->incw;
	    }
	    if ( extra->topextra->inch ) {
		h = h/extra->topextra->inch;
		h = h*extra->topextra->inch;
	    }
	}
    }

    if ( w < 1 )                                // invalid size
	w = 1;
    if ( h < 1 )
	h = 1;

    QPoint oldp = pos();
    QSize  olds = size();

    QRect  r( x, y, w, h );
    setCRect( r );
    if (!isTopLevel() && size() == olds && oldp == pos() )
	return;

    if ( isTopLevel() && isMove && own_id )
	MoveWindow( (WindowPtr)winid, x, y, 1);

    bool isResize = (olds != size());
    if ( isTopLevel() && winid && own_id )
	SizeWindow( (WindowPtr)winid, w, h, 1);

    if ( isVisible() ) {
	if ( isMove ) {
	    QMoveEvent e( pos(), oldp );
	    QApplication::sendEvent( this, &e );
	    update();
	}
	if ( isResize ) {
	    QResizeEvent e( size(), olds );
	    QApplication::sendEvent( this, &e );
	    update();
	}
	if( parentWidget() && (isMove || isResize) ) {
	    QRegion upd = QRegion(r) | QRegion(( QRect(oldp, olds) ));
	    paint_children( parentWidget(), upd );
	}
    } else {
	if ( isMove ) 
	    QApplication::postEvent( this, new QMoveEvent( pos(), oldp ) );
	if ( isResize )
	    QApplication::postEvent( this, new QResizeEvent( size(), olds ) );
    }
}

void QWidget::setMinimumSize( int minw, int minh)
{
    //I'm not happy to be doing this, but apparently this helps (ie on a mainwindow, so the
    //status bar doesn't fall of the bottom) this might need a FIXME!!!
    if(isTopLevel()) {
	minw+=10;
	minh+=10;
    }

#if defined(QT_CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMAX(minw,width()), QMAX(minh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize( int maxw, int maxh)
{
#if defined(QT_CHECK_RANGE)
    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX );
	maxw = QMIN( maxw, QWIDGETSIZE_MAX );
	maxh = QMIN( maxh, QWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
	maxw = QMAX( maxw, 0 );
	maxh = QMAX( maxh, 0 );
    }
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}


void QWidget::setSizeIncrement( int, int )
{
}

void QWidget::setBaseSize( int, int )
{
}


void QWidget::erase( int x, int y, int w, int h )
{
    erase( QRegion( x, y, w, h ) );
}

void QWidget::erase( const QRegion& reg )
{
    if ( backgroundMode() == NoBackground )
	return;

    QRect rr(reg.boundingRect());

    int xoff = 0;
    int yoff = 0;
    if ( !isTopLevel() && backgroundOrigin() == QWidget::ParentOrigin ) {
	xoff = x();
	yoff = y();
    }

    qt_set_paintevent_clipping( this, reg );
    QPainter p;
    p.begin(this);
    if ( extra && extra->bg_pix ) {
	if ( !extra->bg_pix->isNull() ) {
	    QPoint point(xoff%extra->bg_pix->width(), yoff%extra->bg_pix->height());
	    p.drawTiledPixmap(rr,*extra->bg_pix, point);
	} 
    } else {
	p.fillRect(rr, bg_col);
    }
    p.end();
    qt_clear_paintevent_clipping( this );
}


void QWidget::scroll( int dx, int dy)
{
    scroll( dx, dy, QRect() );
}

void QWidget::scroll( int dx, int dy, const QRect& r )
{
    if ( testWState( WState_BlockUpdates ) )
	return;

    bool valid_rect = r.isValid();
    QRect sr = valid_rect?r:rect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if ( dx > 0 ) {
	x1 = sr.x();
	x2 = x1+dx;
	w -= dx;
    } else {
	x2 = sr.x();
	x1 = x2-dx;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = sr.y();
	y2 = y1+dy;
	h -= dy;
    } else {
	y2 = sr.y();
	y1 = y2-dy;
	h += dy;
    }

    if ( dx == 0 && dy == 0 )
	return;
    if ( w > 0 && h > 0 ) {
	bitBlt(this,x2,y2,this,x1,y1,w,h);
    }

    if ( !valid_rect && children() ) {	// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->pos() + pd );
	    }
	    ++it;
	}
    }

    QRegion copied = (QRegion(0, 0, width(), height()) ^ clippedRegion());
    QPoint p = mapToGlobal( QPoint() );
    copied.translate( -p.x(), -p.y() );
    copied &= QRegion(sr);
    copied.translate(dx,dy);
    QRegion exposed = QRegion(sr) - copied;
    repaint( exposed, !testWFlags(WRepaintNoErase) );
}


void QWidget::drawText( int x, int y, const QString &str )
{
    if ( testWState(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


int QWidget::metric( int m ) const
{
    WindowPtr p = (WindowPtr)winid;
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	if ( !isTopLevel() ) {
	    return crect.width();
	} else {
  	    Rect windowBounds;
            GetPortBounds( GetWindowPort( p ), &windowBounds );
            return windowBounds.right;
	}
    } else if( m == QPaintDeviceMetrics::PdmHeight ) {
	if ( !isTopLevel() ) {
	    return crect.height();
	} else {
  	    Rect windowBounds;
            GetPortBounds( GetWindowPort( p ), &windowBounds );
            return windowBounds.bottom;
	}
    } else if ( m == QPaintDeviceMetrics::PdmWidthMM ) {
	return metric( QPaintDeviceMetrics::PdmWidth );
    } else if ( m == QPaintDeviceMetrics::PdmHeightMM ) {
	return metric( QPaintDeviceMetrics::PdmHeight );
    } else if ( m == QPaintDeviceMetrics::PdmNumColors ) {
	return 16;
    } else if ( m == QPaintDeviceMetrics::PdmDepth ) {
	// FIXME : this is a lie in most cases
	return 16;
    } else {
        // FIXME: Handle this case
	qWarning("QWidget::metric unhandled parameter %d",m);
    }
    return 0;
}

void QWidget::createSysExtra()
{
    //do we really need this? Will this need to go back when it crashes again? These are the questions..
//    font().handle(); // force QFont::load call
    extra->macDndExtra = 0;
}

void QWidget::deleteSysExtra()
{
}

void QWidget::createTLSysExtra()
{
}

void QWidget::deleteTLSysExtra()
{
}

bool QWidget::acceptDrops() const
{
    return macDropEnabled;
}

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra ); //dnd_mac
void qt_macdnd_register( QWidget *widget, QWExtra *extra ); //dnd_mac

void QWidget::setAcceptDrops( bool on )
{
    if ( (on && macDropEnabled) || (!on && !macDropEnabled) )
	return;

    if ( on ) {
	topLevelWidget()->createExtra();
	QWExtra *extra = topLevelWidget()->extraData();
	qt_macdnd_register( topLevelWidget(), extra );
	macDropEnabled = true;
    } else {
	macDropEnabled = false;
	QWExtra *extra = topLevelWidget()->extraData();
	qt_macdnd_unregister( topLevelWidget(), extra );
    }
}

void QWidget::setMask( const QRegion &region )
{
    dirtyClippedRegion(TRUE);
    if ( isVisible() && !isTopLevel() )
	update();

    createExtra();
    if ( region.isNull() && extra->mask.isNull() )
	return;
    extra->mask = region;
}

void QWidget::setMask( const QBitmap &bitmap )
{
    setMask( QRegion( bitmap ) );
}


void QWidget::clearMask()
{
    setMask( QRegion() );
}

void QWidget::setName( const char * )
{
}


//FIXME: this function still accepts an x and y, so we can do dirty children later.
void QWidget::propagateUpdates(int , int , int w, int h)
{
    SetPortWindowPort((WindowPtr)handle());
    QRect paintRect( 0, 0, w, h );

    if(!testWFlags(WRepaintNoErase))
	erase(paintRect);

    setWState( WState_InPaintEvent );
    QPaintEvent e( paintRect );
    QApplication::sendEvent( this, &e );
    clearWState( WState_InPaintEvent );

    QWidget *childWidget;
    const QObjectList *childList = children();
    if ( childList ) {
	for(QObjectListIt it(*childList); it.current(); ++it) {
	    if ( (*it)->isWidgetType() ) {
		childWidget = (QWidget *)(*it);
		if(childWidget->isVisible())
		    childWidget->propagateUpdates( 0, 0, childWidget->width(), childWidget->height() );
	    }
	}	
    }
}

void QWidget::dirtyClippedRegion(bool tell_parent)
{
    if(extra)
	extra->clip_dirty = TRUE;
    if(tell_parent && parentWidget())
	parentWidget()->dirtyClippedRegion(FALSE);
}

bool QWidget::isClippedRegionDirty()
{
    if(!extra) {
	return TRUE;
    }
    return extra->clip_dirty || (isTopLevel() || (parentWidget() && parentWidget()->isClippedRegionDirty()));
}

QRegion QWidget::clippedRegion()
{
    if(!isClippedRegionDirty())
	return extra->clip_saved;

    createExtra();
    extra->clip_dirty = FALSE;
    QPoint mp = posInWindow(this);
    QRegion mr(mp.x(), mp.y(), width(), height());
    extra->clip_saved = QRegion();

    QPoint tmp;
    //clip out my children
    if(const QObjectList *chldnlst=children()) {
	for(QObjectListIt it(*chldnlst); it.current(); ++it) {
	    if((*it)->isWidgetType()) {
		QWidget *cw = (QWidget *)(*it);
		if( cw->isVisible() ) {
		    QRegion childrgn(mp.x()+cw->x(), mp.y()+cw->y(), cw->width(), cw->height());
		    if(cw->extra && !cw->extra->mask.isNull()) {
			QRegion mask = cw->extra->mask;
			mask.translate(mp.x()+cw->x(), mp.y()+cw->y());
			childrgn &= mask;
		    }
		    extra->clip_saved += childrgn;
		}
	    }
	}
    }

    //clip away my siblings
    if(!isTopLevel() && parentWidget()) {
	if(const QObjectList *siblst = parentWidget()->children()) {
	    //loop to this because its in zorder, and i don't care about people behind me
	    QObjectListIt it(*siblst);
	    for(it.toLast(); it.current() && it.current() != this; --it) {
		if((*it)->isWidgetType()) {
		    QWidget *sw = (QWidget *)(*it);
		    tmp = posInWindow(sw);
		    QRect sr(tmp.x(), tmp.y(), sw->width(), sw->height());
		    if(sw->isVisible() && mr.contains(sr)) {
			QRegion sibrgn(sr);
			if(sw->extra && !sw->extra->mask.isNull()) {
			    QRegion mask = sw->extra->mask;
			    mask.translate(tmp.x(), tmp.y());
			    sibrgn &= mask;
			}
			extra->clip_saved += sibrgn;
		    }
		}
	    }
	}
    }

    extra->clip_saved = (mr & extra->clip_saved) ^ mr;
    //clip my rect with my mask
    if(extra && !extra->mask.isNull()) {
	QRegion mask = extra->mask;
	mask.translate(mp.x(), mp.y());
	extra->clip_saved &= mask;
    }
    return extra->clip_saved;
}


