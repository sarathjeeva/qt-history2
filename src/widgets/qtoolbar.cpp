/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtoolbar.cpp#50 $
**
** Implementation of QToolBar class
**
** Created : 980315
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

#include "qtoolbar.h"

#include "qmainwindow.h"
#include "qpushbutton.h"
#include "qtooltip.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"

#ifdef QT_BUILDER
#include "qdom.h"
#include "qtoolbutton.h"
#include "qwhatsthis.h"
#endif

/*! \class QToolBar qtoolbar.h

  \brief The QToolBar class provides a simple tool bar.

  \ingroup realwidgets
  \ingroup application

  To use QToolBar, you simply create a QToolBar as child of a
  QMainWindow, create a number of QToolButton widgets (or other
  widgets) in left to right (or top to bottom) order, call
  addSeparator() when you want a separator, and that's all.

  The application/application.cpp example does precisely this.

  Each QToolBar lives in a \link QMainWindow dock \endlink in a
  QMainWindow, and can optionally start a new line in its dock.  Tool
  bars that start a new line are always positioned at the left end or
  top of the tool bar dock; others are placed next to the previous
  tool bar and word-wrapped as necessary.

  The tool bar is able to arrange its buttons horizontally or
  vertically (see setOrientation() for details) and draws the
  appropriate frames around the tool button in Windows and Motif
  style.  Generally, QMainWindow will set the orientation correctly
  for you.

  \sa QToolButton QMainWindow
  <a href="http://www.iarchitect.com/visual.htm">Parts of Isys on Visual Design,</a>
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui192.htm">Microsoft Style Guide,</a>
  <a href="http://www.microsoft.com/win32dev/uiguide/uigui196.htm">some common buttons (NB: read the license),</a>
  <a href="guibooks.html#fowler">GUI Design Handbook: Tool Bar.</a>
*/


/*!  Constructs an empty tool bar which is a child of \a parent and
  managed by \a parent, initially in \a dock, labelled \a and starting
  a new line in the dock if \a newLine is TRUE.  \a name is the object
  name, as usual.
*/

QToolBar::QToolBar( const QString &label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    mw = parent;
    sw = 0;
    o = (dock == QMainWindow::Left || dock == QMainWindow::Right )
	? Vertical : Horizontal;
    parent->addToolBar( this, label, dock, newLine );
    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
}


/*!  Constructs an empty horizontal tool bar which is a parent of \a
  parent and managed by \a mainWindow.  The \a label and \a newLine
  are passed straight to QMainWindow::addToolBar().  \a name is the
  object name and \a f is the widget flags.

  This is the constructor to use if you want to create torn-off
  toolbars, or toolbars in the status bar.
*/

QToolBar::QToolBar( const QString &label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QWidget( parent, name, f )
{
    d = 0;
    b = 0;
    mw = mainWindow;
    sw = 0;
    o = Horizontal;
    mainWindow->addToolBar( this, label, QMainWindow::Unmanaged, newLine );
    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
}


/*!  Constructs an empty tool bar in the top dock of its parent,
  without any label and without requiring a newline.  This is mostly
  useless. */

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QWidget( parent, name )
{
    d = 0;
    b = 0;
    o = Horizontal;
    sw = 0;
    mw = parent;
    parent->addToolBar( this, QString::null, QMainWindow::Top );
    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
}


/*! Destroys the object and frees any allocated resources. */

QToolBar::~QToolBar()
{
    delete b;
    b = 0;
    // delete d; as soon as there is a d
}


/*!  Adds a separator to the end of the toolbar. */

void QToolBar::addSeparator()
{
    QFrame * f = new QFrame( this, "tool bar separator" );
    f->setFrameStyle( QFrame::NoFrame ); // old-style whatevers
}


/*!  Sets this toolbar to organize its content vertically if \a
  newOrientation is \c Vertical and horizontally if \a newOrientation
  is \c Horizontal.
*/

void QToolBar::setOrientation( Orientation newOrientation )
{
    if ( o != newOrientation ) {
	o = newOrientation;
	setUpGM();
    }
}


/*! \fn Orientation QToolBar::orientation() const

  Returns the current orientation of the toolbar.
*/

/*!  Reimplemented to set up geometry management. */

void QToolBar::show()
{
    setUpGM();
    QWidget::show();
}


/*!  Sets up geometry management for this toolbar. */

void QToolBar::setUpGM()
{
    delete b;
    b = new QBoxLayout( this, orientation() == Vertical
			? QBoxLayout::Down : QBoxLayout::LeftToRight,
			style() == WindowsStyle ? 2 : 1, 0 );

    b->addSpacing( 9 );

    const QObjectList * c = children();
    QObjectListIt it( *c );
    QObject *obj;
    while( (obj=it.current()) != 0 ) {
	++it;
	if ( obj->isWidgetType() ) {
	    QWidget * w = (QWidget *)obj;
	    if ( !isVisible() || w->isVisible() ) {
		if ( !qstrcmp( "tool bar separator", obj->name() ) &&
		     !qstrcmp( "QFrame", obj->className() ) ) {
		    QFrame * f = (QFrame *)obj;
		    if ( orientation() == Vertical ) {
			f->setMinimumSize( 0, 6 );
			f->setMaximumSize( QWIDGETSIZE_MAX, 6 );
			if ( style() == WindowsStyle )
			    f->setFrameStyle( QFrame::HLine + QFrame::Sunken );
			else
			    f->setFrameStyle( QFrame::NoFrame );
		    } else {
			f->setMinimumSize( 6, 0 );
			f->setMaximumSize( 6, QWIDGETSIZE_MAX );
			if ( style() == WindowsStyle )
			    f->setFrameStyle( QFrame::VLine + QFrame::Sunken );
			else
			    f->setFrameStyle( QFrame::NoFrame );
		    }
		} else if ( w->maximumSize() == QSize(QWIDGETSIZE_MAX,
						      QWIDGETSIZE_MAX)
			    && w->minimumSize() == QSize(0,0) ) {
		    QSize s( w->sizeHint() );
		    if ( s.width() > 0 && s.height() > 0 )
			w->setMinimumSize( s );
		    else if ( s.width() > 0 )
			w->setMinimumWidth( s.width() );
		    else if ( s.height() > 0 )
			w->setMinimumHeight( s.width() );
		}
		b->addWidget( w, w == sw ? 42 : 0 );
	    }
	}
    }
    b->activate();
    updateGeometry();
}


/*!  Paint the handle.  The Motif style is rather close to Netscape
  and even closer to KDE. */

void QToolBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    qDrawShadePanel( &p, 0, 0, width(), height(),
		     colorGroup(), FALSE, 1, 0 );
    if ( style() == MotifStyle ) {
	QColor dark( colorGroup().dark() );
	QColor light( colorGroup().light() );
	unsigned int i;
	if ( orientation() == Vertical ) {
	    int w = width();
	    if ( w > 6 ) {
		QPointArray a( 2 * ((w-6)/3) );
		
		int x = 3 + (w%3)/2;
		p.setPen( dark );
		p.drawLine( 1, 8, w-2, 8 );
		for( i=0; 2*i < a.size(); i ++ ) {
		    a.setPoint( 2*i, x+1+3*i, 6 );
		    a.setPoint( 2*i+1, x+2+3*i, 3 );
		}
		p.drawPoints( a );
		p.setPen( light );
		p.drawLine( 1, 9, w-2, 9 );
		for( i=0; 2*i < a.size(); i++ ) {
		    a.setPoint( 2*i, x+3*i, 5 );
		    a.setPoint( 2*i+1, x+1+3*i, 2 );
		}
		p.drawPoints( a );
	    }
	} else {
	    int h = height();
	    if ( h > 6 ) {
		QPointArray a( 2 * ((h-6)/3) );
		int y = 3 + (h%3)/2;
		p.setPen( dark );
		p.drawLine( 8, 1, 8, h-2 );
		for( i=0; 2*i < a.size(); i ++ ) {
		    a.setPoint( 2*i, 5, y+1+3*i );
		    a.setPoint( 2*i+1, 2, y+2+3*i );
		}
		p.drawPoints( a );
		p.setPen( light );
		p.drawLine( 9, 1, 9, h-2 );
		for( i=0; 2*i < a.size(); i++ ) {
		    a.setPoint( 2*i, 4, y+3*i );
		    a.setPoint( 2*i+1, 1, y+1+3*i );
		}
		p.drawPoints( a );
	    }
	}
    } else {
	if ( orientation() == Vertical ) {
	    if ( width() > 4 ) {
		qDrawShadePanel( &p, 2, 4, width() - 4, 3,
				 colorGroup(), FALSE, 1, 0 );
		qDrawShadePanel( &p, 2, 7, width() - 4, 3,
				 colorGroup(), FALSE, 1, 0 );
	    }
	} else {
	    if ( height() > 4 ) {
		qDrawShadePanel( &p, 4, 2, 3, height() - 4,
				 colorGroup(), FALSE, 1, 0 );
		qDrawShadePanel( &p, 7, 2, 3, height() - 4,
				 colorGroup(), FALSE, 1, 0 );
	    }
	}
    }
}



/*!  Returns a pointer to the QMainWindow which controls this tool bar.
*/

QMainWindow * QToolBar::mainWindow()
{
    return mw;
}


/*!  Sets \a w to be expanded if this toolbar is requested to stretch
  (because QMainWindow right-justifies the dock it's in).
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
}


/*! \reimp */

bool QToolBar::event( QEvent * e )
{
    if ( e->type() == QEvent::LayoutHint ) {
	setUpGM();
    } else if ( e->type() == QEvent::ChildInserted ) {
	QObject * child = ((QChildEvent*)e)->child();
	if ( child && child->isWidgetType() )
	    child->installEventFilter( this );
    }
    return QWidget::event( e );
}


/*! \reimp */

bool QToolBar::eventFilter( QObject * obj, QEvent * e )
{
    if ( obj && e && obj->isWidgetType() &&
	 ((QWidget *)obj)->parentWidget() == this &&
	 ( e->type() == QEvent::Show ||
	   e->type() == QEvent::Hide ) ) {
	QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
    }
    return QWidget::eventFilter( obj, e );
}


/*!  Sets the label of this tool bar to \a label.  The label is not
currently used; it will be used in a forthcoming tool bar
configuration dialog.

\sa label()
*/

void QToolBar::setLabel( const QString & label )
{
    l = label;
}


/*!  Returns the label of this tool bar.

  \sa setLabel()
*/

QString QToolBar::label() const
{
    return l;
}


/*!
  Clears the toolbar, deleting all childwidgets.
 */
void QToolBar::clear()
{
    if ( !children() )
	return;
    QObjectList list = *children();
    for (QObjectListIt it(list); it.current(); ++it) {
        if ( it.current()->isWidgetType() )
            delete it.current();
    }
}

#ifdef QT_BUILDER
bool QToolBar::setConfiguration( const QDomElement& element )
{
  // When this code changes then the code in the builder
  // must be changed, too. Unfortunately it had to be copied.
  QDomElement r = element.firstChild().toElement();
  for( ; !r.isNull(); r = r.nextSibling().toElement() )
  {
    if ( r.tagName() == "Widget" )
    {
      if ( !r.firstChild().toElement().toWidget( this ) )
	return FALSE;
    }
    else if ( r.tagName() == "Separator" )
      addSeparator();
    else if ( r.tagName() == "WhatsThis" )
    {
      QToolButton *tb = QWhatsThis::whatsThisButton( this );
      // Changing that name would break the builder
      tb->setName( "whatsthis button" );
    }
  }

  // Dont call QWidget configure: For example we dont
  // allow for layouts
  if ( !QObject::setConfiguration( element ) )
    return FALSE;

  return TRUE;
}

QObject* QToolBar::factory( QObject* parent )
{
  if ( !parent->inherits("QMainWindow") )
  {
    qDebug( "The parent of a toolbar must always be a QMainWindow.\n" );
    return 0;
  }

  return new QToolBar( (QMainWindow*)parent );
}

#endif // QT_BUILDER

/* from chaunsee:

1.  Toolbars should contain only high-frequency functions.  Avoid putting
things like About and Exit on a toolbar unless they are frequent functions.

2.  All toolbar buttons must have some keyboard access method (it can be a
menu or shortcut key or a function in a dialog box that can be accessed
through the keyboard).

3.  Make toolbar functions as efficient as possible (the common example is to
Print in Microsoft applications, it doesn't bring up the Print dialog box, it
prints immediately to the default printer).

4.  Avoid turning toolbars into graphical menu bars.  To me, a toolbar should
be efficient. Once you make almost all the items in a toolbar into graphical
pull-down menus, you start to loose efficiency.

5.  Make sure that adjacent icons are distinctive. There are some toolbars
where you see a group of 4-5 icons that represent related functions, but they
are so similar that you can't differentiate among them.  These toolbars are
often a poor attempt at a "common visual language".

6.  Use any de facto standard icons of your platform (for windows use the
cut, copy, and paste icons provided in dev kits rather than designing your
own).

7.  Avoid putting a highly destructive toolbar button (delete database) by a
safe, high-frequency button (Find) -- this will yield 1-0ff errors).

8.  Tooltips in many Microsoft products simply reiterate the menu text even
when that is not explanatory.  Consider making your tooltips slightly more
verbose and explanatory than the corresponding menu item.

9.  Keep the toolbar as stable as possible when you click on different
objects. Consider disabling toolbar buttons if they are used in most, but not
all contexts.

10.  If you have multiple toolbars (like the Microsoft MMC snap-ins have),
put the most stable toolbar to at the left with less stable ones to the
right. This arrangement (stable to less stable) makes the toolbar somewhat
more predictable.

11.  Keep a single toolbar to fewer than 20 items divided into 4-7 groups of
items.
*/
