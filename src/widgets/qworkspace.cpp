/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qworkspace.cpp#27 $
**
** Implementation of the QWorkspace class
**
** Created : 931107
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
#include "qworkspace.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qlayout.h"
#include "qtoolbutton.h"
#include "qlabel.h"
#include "qvbox.h"
#include "qaccel.h"
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qguardedptr.h"

#if defined(_WS_WIN_)
#include "qt_windows.h"
const bool win32 = TRUE;
#define TITLEBAR_HEIGHT 18
#define TITLEBAR_SEPARATION 2
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 14
#define BORDER 2
#define RANGE 16
#define OFFSET 20


static const char * close_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"....##....##....",
".....##..##.....",
"......####......",
".......##.......",
"......####......",
".....##..##.....",
"....##....##....",
"................",
"................",
"................",
"................",
"................"};

static const char*maximize_xpm[]={
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"...#########....",
"...#########....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#.......#....",
"...#########....",
"................",
"................",
"................",
"................"};


static const char * minimize_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"................",
"....######......",
"....######......",
"................",
"................",
"................",
"................"};

static const char * normalize_xpm[] = {
"16 16 2 1",
"# c #000000",
". c None",
"................",
"................",
"................",
".....######.....",
".....######.....",
".....#....#.....",
"...######.#.....",
"...######.#.....",
"...#....###.....",
"...#....#.......",
"...#....#.......",
"...######.......",
"................",
"................",
"................",
"................"};


#else // !_WS_WIN_

const bool win32 = FALSE;
#define TITLEBAR_HEIGHT 18
#define TITLEBAR_SEPARATION 2
#define BUTTON_WIDTH 18
#define BUTTON_HEIGHT 18
#define BORDER 2
#define RANGE 16
#define OFFSET 20

static const char * close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
"X      c #707070",
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};

static const char * maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
"X      c #707070",
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};


static const char * minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c white",
"X      c #707070",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * normalize_xpm[] = {
"16 16 3 1",
" 	s None	c None",
".	c white",
"X	c #707070",
"                ",
"                ",
"     ........   ",
"     .XXXXXXXX  ",
"     .X     .X  ",
"     .X     .X  ",
"  ....X...  .X  ",
"  .XXXXXXXX .X  ",
"  .X     .XXXX  ",
"  .X     .X     ",
"  .X     .X     ",
"  .X......X     ",
"  .XXXXXXXX     ",
"                ",
"                ",
"                "};

#endif // !_WS_WIN_




static bool resizeHorizontalDirectionFixed = FALSE;
static bool resizeVerticalDirectionFixed = FALSE;

class Q_EXPORT QWorkspaceChildTitleBar : public QWidget
{
    Q_OBJECT
public:
    QWorkspaceChildTitleBar (QWorkspace* w, QWidget* parent, const char* name=0, bool iconMode = FALSE );
    ~QWorkspaceChildTitleBar();

    bool isActive() const;

    QSize sizeHint() const;

 public slots:
    void setActive( bool );
    void setText( const QString& title );
    void setIcon( const QPixmap& icon );

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

protected:
    void resizeEvent( QResizeEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QToolButton* closeB;
    QToolButton* maxB;
    QToolButton* iconB;
    QLabel* titleL;
    QLabel* iconL;
    bool buttonDown;
    QPoint moveOffset;
    QWorkspace* workspace;
    bool imode;
    bool act;
};


class Q_EXPORT QWorkspaceChild : public QFrame
{
    Q_OBJECT
public:
    QWorkspaceChild( QWidget* client, QWorkspace *parent=0, const char *name=0 );
    ~QWorkspaceChild();

    void setActive( bool );
    bool isActive() const;

    void adjustToFullscreen();

    QWidget* clientWidget() const;
    QWidget* iconWidget() const;

    void doResize();
    void doMove();

signals:
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

 public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void childEvent( QChildEvent* );
    void keyPressEvent( QKeyEvent * );

    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );

private:
    QWidget* clientw;
    bool buttonDown;
    enum MousePosition {
	Nowhere, TopLeft , BottomRight, BottomLeft, TopRight, Top, Bottom, Left, Right, Center
    };
    MousePosition mode;
    bool moveResizeMode;
    void setMouseCursor( MousePosition m );
    bool isMove() const {
	return moveResizeMode && mode == Center;
    }
    bool isResize() const {
	return moveResizeMode && !isMove();
    }
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    bool act;
    QWorkspaceChildTitleBar* titlebar;
    QGuardedPtr<QWorkspaceChildTitleBar> iconw;
    QSize clientSize;

};


class QWorkspaceData {
public:
    QWorkspaceChild* active;
    QList<QWorkspaceChild> windows;
    QList<QWorkspaceChild> focus;
    QList<QWidget> icons;
    QWorkspaceChild* maxClient;
    QRect maxRestore;
    QFrame* maxcontrols;

    int px;
    int py;
    QWidget *becomeActive;
    QPopupMenu* popup;
    int menuId;
    int controlId;
};

// NOT REVISED

/*!
  \class QWorkspace qworkspace.h

  \brief The QWorkspace widget provides a workspace that can contain
  decorated windows as opposed to frameless child widgets.  QWorkspace
  makes it easy to implement a multidocument interface (MDI).

  \ingroup realwidgets

  An MDI application has one main window that has a menubar. The
  central widget of this main window is a workspace. The workspace
  itself contains zero, one or several document windows, each of which
  displays a document.

  The menubar and the toolbars act as a roadmap to the
  application. You get to keep the same map all the time, even if you
  open three different documents in three different child windows and
  switch around among them.

  The workspace itself is an ordinary Qt widget. It has a standard
  constructor that takes a parent widget and an object name.  Document
  windows, so-called MDI windows, are also ordinary Qt widgets.  They
  may even be main windows itself, in case your application design
  requires specific toolbars or statusbars for them. The only special
  thing about them is that you create them with the workspace as
  parent widget. The rest of the magic happens behind the scenes. All
  you have to do is call QWidget::show() or QWidget::showMaximized()
  (as you would do with normal toplevel windows) and the document
  window appears as MDI window inside the workspace.

  In addition to show, QWidget::hide(), QWidget::showMaximized(),
  QWidget::showNormal(), QWidget::showMinimized() also work as
  expected for the document windows.

  A document window becomes active when it gets the focus. This can be
  achieved by calling QWidget::setFocus(). The workspace emits a
  signal clientActivated() when it detects the activation change.  The
  active client itself can be obtained with activeClient().

  The convenience function clientList() returns a list of all document
  windows. This is especially useful to create a popup menu "&Windows"
  on the fly.

  If the user clicks on the frame of an MDI window, this window
  becomes active, i.e. it gets the focus. For that reason, all direct
  children of a QWorkspace have to be focus enabled. If your MDI
  window does not handle focus itself, use QWidget::setFocusProxy() to
  install a focus-enabled child widget as focus proxy.

  In case the toplevel window contains a menu bar and a MDI child
  window is maximized, QWorkspace places the child window's minimize,
  restore and close buttons on the menubar, in the same relative
  positions as in the title bar of the child window.  Futhermore, on
  the very left, the menubar will contain a window operation menu with
  the child window's icon as label.

  In general, modern GUI applications should be document-centric
  rather then application-centric. A single document interface (SDI)
  guarantees a bijective mapping between open documents and open
  windows on the screen. This makes the model very easy to understand
  and therefore the natural choice for applications targeted on
  inexperienced users. Typical examples are modern
  wordprocessors. Although most wordprocessors were MDI applications
  in the past, user interface studies showed that many users never
  really understood the model.

  If an application is supposed to be used mostly by experienced
  users, a multiple document interface may neverthless make sense.  A
  typical example is an integrated development environment (IDE). With
  an IDE, a document is a project. The project itself consists of an
  arbitrary number of subdocuments, mainly code files but also other
  data. MDI offers a good possibility to group these subdocuments
  together in one main window.  The menubar and the toolbars form a
  stable working context for the users to grasp and it is
  crystal-clear which subdocuments belong together. Furthermore, the
  user effort for window management tasks such as positioning,
  stacking and sizing is significantly reduced.

  An alternative to MDI with QWorkspace is a multipane structure. This
  can be achived by tiling the main window into separate panes with a
  \l QSplitter.

*/


/*!
  Creates a workspace with a \a parent and a \a name
 */
QWorkspace::QWorkspace( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QWorkspaceData;
    d->maxcontrols = 0;
    d->active = 0;
    d->maxClient = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;
    d->popup = new QPopupMenu;
    d->menuId = -1;
    d->controlId = -1;
    connect( d->popup, SIGNAL( aboutToShow() ), this, SLOT(operationMenuAboutToShow() ));
    connect( d->popup, SIGNAL( activated(int) ), this, SLOT( operationMenuActivated(int) ) );
    d->popup->insertItem(tr("&Restore"), 1);
    d->popup->insertItem(tr("&Move"), 2);
    d->popup->insertItem(tr("&Size"), 3);
    d->popup->insertItem(tr("Mi&nimize"), 4);
    d->popup->insertItem(tr("Ma&ximize"), 5);
    d->popup->insertSeparator();
    d->popup->insertItem(tr("Close")+"\t"+QAccel::keyToString( CTRL+Key_W),
		  this, SLOT( closeActiveClient() ) );

    QAccel* a = new QAccel( this );
    a->connectItem( a->insertItem( ALT + Key_Minus), this, SLOT( showOperationMenu() ) );
    a->connectItem( a->insertItem( ALT + Key_W), this, SLOT( closeActiveClient() ) );
    a->connectItem( a->insertItem( ALT + Key_F6), this, SLOT( activateNextClient() ) );
    a->connectItem( a->insertItem( CTRL + Key_Tab), this, SLOT( activateNextClient() ) );
    a->connectItem( a->insertItem( CTRL +  ALT + Key_Tab), this, SLOT( activateNextClient() ) );
    a->connectItem( a->insertItem( ALT + SHIFT + Key_F6), this, SLOT( activatePreviousClient() ) );
    a->connectItem( a->insertItem( CTRL + SHIFT + Key_Tab), this, SLOT( activatePreviousClient() ) );
    a->connectItem( a->insertItem( CTRL +  ALT + SHIFT + Key_Tab), this, SLOT( activatePreviousClient() ) );
}

/*!
  Destructor.
 */
QWorkspace::~QWorkspace()
{
    delete d;
}

/*!\reimp
 */
void QWorkspace::childEvent( QChildEvent * e)
{

    if (e->inserted() && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( w->testWFlags( WStyle_Customize | WStyle_NoBorder )
	      || d->icons.contains( w ) )
	    return; 	    // nothing to do

	QWorkspaceChild* child = new QWorkspaceChild( w, this );
	connect( child, SIGNAL( popupOperationMenu( const QPoint& ) ),
		 this, SLOT( popupOperationMenu( const QPoint& ) ) );
	connect( child, SIGNAL( showOperationMenu() ),
		 this, SLOT( showOperationMenu() ) );
	d->windows.append( child );
	d->focus.append( child );
	place( child );
	child->raise();
	activateClient( w );
    } else if (e->removed() ) {
	if ( d->windows.contains( (QWorkspaceChild*)e->child() ) ) {
	    d->windows.removeRef( (QWorkspaceChild*)e->child() );
	    d->focus.removeRef( (QWorkspaceChild*)e->child() );
	    if ( d->windows.isEmpty() )
		hideMaximizeControls();
	    if ( d->icons.contains( (QWidget*)e->child() ) ){
		d->icons.remove( (QWidget*)e->child() );
		layoutIcons();
	    }
	    if( e->child() == d->active )
		d->active = 0;

	    if (  !d->windows.isEmpty() ) {
		if ( e->child() == d->maxClient  ) {
		    d->maxClient = 0;
		    maximizeClient( d->windows.first()->clientWidget() );
		} else {
		    activateClient( d->windows.first()->clientWidget() );
		}
	    } else if ( e->child() == d->maxClient )
		d->maxClient = 0;
	}
    }
}



void QWorkspace::activateClient( QWidget* w, bool change_focus )
{
    if ( !isVisible() ) {
	d->becomeActive = w;
	return;
    }

    if ( d->active && d->active->clientWidget() == w )
	return;

    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	c->setActive( c->clientWidget() == w );
	if (c->clientWidget() == w)
	    d->active = c;
    }

    if (!d->active)
	return;

    if ( d->maxClient && d->maxClient != d->active )
	maximizeClient( d->active->clientWidget() );

    d->active->raise();

    if ( change_focus ) {
	d->focus.removeRef( d->active );
	d->focus.append( d->active );
    }
    emit clientActivated( w );
}


/*!
  Returns the active client, or 0 if no client is active.
 */
QWidget* QWorkspace::activeClient() const
{
    return d->active?d->active->clientWidget():0;
}


void QWorkspace::place( QWidget* w)
{
    int tx,ty;
    QRect maxRect = rect();
    if (d->px < maxRect.x())
	d->px = maxRect.x();
    if (d->py < maxRect.y())
	d->py = maxRect.y();

    d->px += OFFSET;
    d->py += 2*OFFSET;

    if (d->px > maxRect.width()/2)
	d->px =  maxRect.x() + OFFSET;
    if (d->py > maxRect.height()/2)
	d->py =  maxRect.y() + OFFSET;
    tx = d->px;
    ty = d->py;
    if (tx + w->width() > maxRect.right()){
	tx = maxRect.right() - w->width();
	if (tx < 0)
	    tx = 0;
	d->px =  maxRect.x();
    }
    if (ty + w->height() > maxRect.bottom()){
	ty = maxRect.bottom() - w->height();
	if (ty < 0)
	    ty = 0;
	d->py =  maxRect.y();
    }
    w->move( tx, ty );
}

void QWorkspace::insertIcon( QWidget* w )
{
    if (d->icons.contains(w) )
	return;
    d->icons.append( w );
    if (w->parentWidget() != this )
	w->reparent( this, 0, QPoint(0,0), FALSE);
    layoutIcons();
    if (isVisible())
	w->show();

}

void QWorkspace::removeIcon( QWidget* w)
{
    if (!d->icons.contains( w ) )
	return;
    d->icons.remove( w );
    w->hide();
 }

/*!\reimp
 */
void QWorkspace::resizeEvent( QResizeEvent * )
{
    if ( d->maxClient )
	d->maxClient->adjustToFullscreen();
    layoutIcons();
}

/*!\reimp
 */
void QWorkspace::showEvent( QShowEvent *e )
{
    QWidget::showEvent( e );
    if ( d->becomeActive )
	activateClient( d->becomeActive );
    else if ( d->windows.count() > 0 && !d->active )
	activateClient( d->windows.first()->clientWidget() );
}

void QWorkspace::layoutIcons()
{
    int x = 0;
    int y = height();
    for (QWidget* w = d->icons.first(); w ; w = d->icons.next() ) {

	if ( x > 0 && x + w->width() > width() ){
	    x = 0;
	    y -= w->height();
	}

	w->move(x, y-w->height());
	x = w->geometry().right();
    }
}

void QWorkspace::minimizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	c->hide();
	insertIcon( c->iconWidget() );
	if ( d->maxClient == c ) {
	    c->setGeometry( d->maxRestore );
	    d->maxClient = 0;
	    hideMaximizeControls();
	}
    }
}

void QWorkspace::normalizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( c ) {
	if ( c == d->maxClient ) {
	    c->setGeometry( d->maxRestore );
	    d->maxClient = 0;
	}
	else {
	    removeIcon( c->iconWidget() );
	    c->show();
	}
	hideMaximizeControls();
    }
}

void QWorkspace::maximizeClient( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );

    if ( c ) {
	if (d->icons.contains(c->iconWidget()) )
	    normalizeClient( w );
	QRect r( c->geometry() );
	c->adjustToFullscreen();
	c->show();
	c->raise();
	if ( d->maxClient && d->maxClient != c ) {
	    d->maxClient->setGeometry( d->maxRestore );
	}
	if ( d->maxClient != c ) {
	    d->maxClient = c;
	    d->maxRestore = r;
	}

	activateClient( w);
	showMaximizeControls();
    }
}

void QWorkspace::showClient( QWidget* w)
{
    if ( d->maxClient )
	maximizeClient( w );
    else
	normalizeClient( w );
}


QWorkspaceChild* QWorkspace::findChild( QWidget* w)
{
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	if (c->clientWidget() == w)
	    return c;
    }
    return 0;
}

/*!
  Returns a list of all clients.
 */
QWidgetList QWorkspace::clientList() const
{
    QWidgetList clients;
    for (QWorkspaceChild* c = d->windows.first(); c; c = d->windows.next() ) {
	clients.append( c->clientWidget() );
    }
    return clients;
}

/*!\reimp
 */
bool QWorkspace::eventFilter( QObject *o, QEvent * e)
{
    return QWidget::eventFilter( o, e);
}

void QWorkspace::showMaximizeControls()
{

    QObjectList * l = topLevelWidget()->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b = 0;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    delete l;
    
    if ( !b )
	return;
    
    if ( !d->maxcontrols ) {
	d->maxcontrols = new QFrame( topLevelWidget() );
	if ( !win32 )
	    d->maxcontrols->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	QHBoxLayout* l = new QHBoxLayout( d->maxcontrols, d->maxcontrols->frameWidth(), 0 );
	QToolButton* iconB = new QToolButton( d->maxcontrols, "iconify" );
	l->addWidget( iconB );
	iconB->setFocusPolicy( NoFocus );
	iconB->setIconSet( QPixmap( minimize_xpm ));
 	iconB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( iconB, SIGNAL( clicked() ), this, SLOT( minimizeActiveClient() ) );
	QToolButton* restoreB = new QToolButton( d->maxcontrols, "restore" );
	l->addWidget( restoreB );
	restoreB->setFocusPolicy( NoFocus );
	restoreB->setIconSet( QPixmap( normalize_xpm ));
 	restoreB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( restoreB, SIGNAL( clicked() ), this, SLOT( normalizeActiveClient() ) );
	
	l->addSpacing( 2 );
	QToolButton* closeB = new QToolButton( d->maxcontrols, "close" );
	l->addWidget( closeB );
	closeB->setFocusPolicy( NoFocus );
	closeB->setIconSet( QPixmap( close_xpm ) );
 	closeB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( closeB, SIGNAL( clicked() ), this, SLOT( closeActiveClient() ) );

	if ( !win32 ) {
	    iconB->setAutoRaise( TRUE );
	    restoreB->setAutoRaise( TRUE );
	    closeB->setAutoRaise( TRUE );
	}
	
	d->maxcontrols->setFixedSize( 3* BUTTON_WIDTH+2+2*d->maxcontrols->frameWidth(),
				    BUTTON_HEIGHT+2*d->maxcontrols->frameWidth());
    }

//     d->maxcontrols->move( b->mapToParent(b->rect().bottomRight()) 
// 			  - QPoint(4 + d->maxcontrols->width(), 4 + d->maxcontrols->height() ) );
    if ( d->controlId == -1 )
	d->controlId = b->insertItem( d->maxcontrols, -1, b->count() );
    if ( d->active && d->menuId == -1 ) {
	if ( d->active->clientWidget()->icon() )
	    d->menuId = b->insertItem( *d->active->clientWidget()->icon(), d->popup, -1, 0 );
	else {
	    QPixmap pm(10,12);
	    pm.fill( white );
	    d->menuId = b->insertItem( pm, d->popup, -1, 0 );
	}
    }
}

void QWorkspace::hideMaximizeControls()
{
    QObjectList * l = topLevelWidget()->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b = 0;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    delete l;
    if ( b ) {
	if ( d->menuId != -1 )
	    b->removeItem( d->menuId );
	if ( d->controlId != -1 )
	    b->removeItem( d->controlId );
    }
    d->maxcontrols = 0;
    d->menuId = -1;
    d->controlId = -1;
}

void QWorkspace::closeActiveClient()
{
    QWidget* w = activeClient();
    if ( w )
	w->close();
}

void QWorkspace::normalizeActiveClient()
{
    if  ( d->active )
	d->active->showNormal();
}

void QWorkspace::minimizeActiveClient()
{
    if  ( d->active )
	d->active->showMinimized();
}

void QWorkspace::showOperationMenu()
{
    if  ( !d->active )
	return;
    QPoint p( d->active->clientWidget()->mapToGlobal( QPoint(0,0) ) );
    if ( !d->active->isVisible() ) {
	p = d->active->iconWidget()->mapToGlobal( QPoint(0,0) );
	p.ry() -= d->popup->sizeHint().height();
    }
    popupOperationMenu( p );
}

void QWorkspace::popupOperationMenu( const QPoint&  p)
{
    d->popup->popup( p );
}


void QWorkspace::operationMenuAboutToShow()
{
    for ( int i = 1; i < 6; i++ )
	d->popup->setItemEnabled( i, d->active!= 0 );
	
    if ( !d->active )
	return;
    
    if ( d->active == d->maxClient ) {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 5, FALSE );
    } else if ( d->active->isVisible() ){
	d->popup->setItemEnabled( 1, FALSE );
    } else {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 4, FALSE );
    }
}

void QWorkspace::operationMenuActivated( int a )
{
    if ( !d->active )
	return;
    switch ( a ) {
    case 1:
	d->active->showNormal();
	break;
    case 2:
	d->active->doMove();
	break;
    case 3:
	d->active->doResize();
	break;
    case 4:
	d->active->showMinimized();
	break;
    case 5:
	d->active->showMaximized();
	break;
    default:
	break;
    }
}

void QWorkspace::activateNextClient()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	activateClient( d->focus.first()->clientWidget(), FALSE );
	return;
    }

    int a = d->focus.find( d->active );
    if ( a <= 0 )
	a = d->focus.count()-1;
    else
	a--;
    activateClient( d->focus.at( a )->clientWidget(), FALSE );
}

void QWorkspace::activatePreviousClient()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	activateClient( d->focus.last()->clientWidget(), FALSE );
	return;
    }

    int a = d->focus.find( d->active );
    if ( a < 0  || a >= int(d->focus.count())-1 )
	a = 0;
    else
	a++;
    activateClient( d->focus.at( a )->clientWidget(), FALSE );
}


/*!
  \fn void QWorkspace::clientActivated( QWidget* w )

  This signal is emitted when the client widget \a w becomes active.

  \sa activeClient(), clientList()
*/



QWorkspaceChildTitleBar::QWorkspaceChildTitleBar (QWorkspace* w, QWidget* parent,
						  const char* name, bool iconMode )
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder )
{
    workspace = w;
    buttonDown = FALSE;
    imode = iconMode;
    act = FALSE;

    titleL = new QLabel( this, "__workspace_child_title_bar" );
    titleL->setTextFormat( PlainText );
    titleL->setIndent( 10 );

    closeB = new QToolButton( this, "close" );
    closeB->setFocusPolicy( NoFocus );
    closeB->setIconSet( QPixmap( close_xpm ) );
    closeB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( closeB, SIGNAL( clicked() ),
	     this, SIGNAL( doClose() ) ) ;
    maxB = new QToolButton( this, "maximize" );
    maxB->setFocusPolicy( NoFocus );
    maxB->setIconSet( QPixmap( maximize_xpm ));
    maxB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
    connect( maxB, SIGNAL( clicked() ),
	     this, SIGNAL( doMaximize() ) );
    iconB = new QToolButton( this, "iconify" );
    iconB->setFocusPolicy( NoFocus );
    iconB->resize(BUTTON_WIDTH, BUTTON_HEIGHT);

    if ( !win32 ) {
	closeB->setAutoRaise( TRUE );
	maxB->setAutoRaise( TRUE );
	iconB->setAutoRaise( TRUE );
    }
    if ( imode ) {
	iconB->setIconSet( QPixmap( normalize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doNormal() ) );
    }
    else {
	iconB->setIconSet( QPixmap( minimize_xpm ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doMinimize() ) );
    }

    titleL->setMouseTracking( TRUE );
    titleL->installEventFilter( this );
    titleL->setAlignment( AlignVCenter | SingleLine );
    QFont fnt = font();
    fnt.setBold( TRUE );
    titleL->setFont( fnt );

    iconL = new QLabel( this, "icon" );
    iconL->setAlignment( AlignCenter );
    iconL->setFocusPolicy( NoFocus );
    iconL->installEventFilter( this );
}

QWorkspaceChildTitleBar::~QWorkspaceChildTitleBar()
{
}

void QWorkspaceChildTitleBar::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = TRUE;
	moveOffset = mapToParent( e->pos() );
	emit doActivate();
    } else if ( e->button() == RightButton ) {
	emit doActivate();
	emit popupOperationMenu( e->globalPos() );
    }
}

void QWorkspaceChildTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
    }
}

void QWorkspaceChildTitleBar::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown )
	return;
    QPoint p = workspace->mapFromGlobal( e->globalPos() );
    if ( !workspace->rect().contains(p) ) {
	if ( p.x() < 0 )
	    p.rx() = 0;
	if ( p.y() < 0 )
	    p.ry() = 0;
	if ( p.x() > workspace->width() )
	    p.rx() = workspace->width();
	if ( p.y() > workspace->height() )
	    p.ry() = workspace->height();
    }

    QPoint pp = p - moveOffset;

    parentWidget()->move( pp );
}


void QWorkspaceChildTitleBar::setText( const QString& title )
{
    titleL->setText( title );
}


void QWorkspaceChildTitleBar::setIcon( const QPixmap& icon )
{
    iconL->setPixmap( icon );
}


bool QWorkspaceChildTitleBar::eventFilter( QObject * o, QEvent * e)
{
    if ( o == titleL ) {
	if ( e->type() == QEvent::MouseButtonPress
	     || e->type() == QEvent::MouseButtonRelease
	     || e->type() == QEvent::MouseMove) {
	    QMouseEvent* me = (QMouseEvent*) e;
	    QMouseEvent ne( me->type(), titleL->mapToParent(me->pos()), me->button(), me->state() );

	    if (e->type() == QEvent::MouseButtonPress )
		mousePressEvent( &ne );
	    else if (e->type() == QEvent::MouseButtonRelease )
		mouseReleaseEvent( &ne );
	    else
		mouseMoveEvent( &ne );
	}
	else if ( ((QMouseEvent*)e)->button() == LeftButton && e->type() == QEvent::MouseButtonDblClick ) {
	    if ( imode )
		emit doNormal();
	    else
		emit doMaximize();
	}
    } else if ( o == iconL ) {
	if ( e->type() == QEvent::MouseButtonPress ) {
	    emit doActivate();
	    emit showOperationMenu();
	}
    }
    return FALSE;
}


void QWorkspaceChildTitleBar::resizeEvent( QResizeEvent * )
{
    int bo = ( height()- BUTTON_HEIGHT) / 2;
    closeB->move( width() - BUTTON_WIDTH - bo, bo  );
    maxB->move( closeB->x() - BUTTON_WIDTH - bo, closeB->y() );
    iconB->move( maxB->x() - BUTTON_WIDTH, maxB->y() );
    iconL->setGeometry( 0, 0, BUTTON_WIDTH, height() );
    if ( win32 || (imode && !isActive()) )
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0 ),
				    rect().bottomRight() ) );
    else
	titleL->setGeometry( QRect( QPoint( BUTTON_WIDTH, 0),
				    QPoint( iconB->geometry().left() - 1, rect().bottom() ) ) );

}


void QWorkspaceChildTitleBar::setActive( bool active )
{
    act = active;
    if ( active ) {
	if ( imode ){
	    iconB->show();
	    maxB->show();
	    closeB->show();
	}
	QColorGroup g = colorGroup();
	g.setColor( QColorGroup::Background,  colorGroup().color( QColorGroup::Highlight ) );
	g.setColor( QColorGroup::Text,  colorGroup().color( QColorGroup::HighlightedText) );
	if ( win32 ) {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	}
    }
    else {
	if ( imode ){
	    iconB->hide();
	    closeB->hide();
	    maxB->hide();
	}
	QColorGroup g = colorGroup();
	if ( win32 ) {
	    g.setColor( QColorGroup::Background,  colorGroup().color( QColorGroup::Dark ) );
	    g.setColor( QColorGroup::Text,  colorGroup().color( QColorGroup::Background) );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	    iconL->setPalette( QPalette( g, g, g), TRUE );
	} else {
	    titleL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	    titleL->setFrameStyle( QFrame::NoFrame );
	    titleL->setPalette( QPalette( g, g, g), TRUE );
	}
    }
    if ( imode )
	resizeEvent(0);
}

bool QWorkspaceChildTitleBar::isActive() const
{
    return act;
}


QSize QWorkspaceChildTitleBar::sizeHint() const
{
    return QSize( 196, QMAX( TITLEBAR_HEIGHT, fontMetrics().lineSpacing() ) );
}

class QWorkSpaceChildProtectedWidget : public QWidget
{
public:
    void reasonableFocus() { if ( !isFocusEnabled() )
	(void) focusNextPrevChild( TRUE );
    }
};


QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent,
				  const char *name )
    : QFrame( parent, name,
	      WStyle_Customize | WStyle_NoBorder  | WDestructiveClose )
{
    mode = Nowhere;
    buttonDown = FALSE;
    setMouseTracking( TRUE );
    act = FALSE;
    iconw = 0;

    titlebar = new QWorkspaceChildTitleBar( parent, this );
    connect( titlebar, SIGNAL( doActivate() ),
	     this, SLOT( activate() ) );
    connect( titlebar, SIGNAL( doClose() ),
	     window, SLOT( close() ) );
    connect( titlebar, SIGNAL( doMinimize() ),
	     this, SLOT( showMinimized() ) );
    connect( titlebar, SIGNAL( doMaximize() ),
	     this, SLOT( showMaximized() ) );
    connect( titlebar, SIGNAL( popupOperationMenu( const QPoint& ) ),
	     this, SIGNAL( popupOperationMenu( const QPoint& ) ) );
    connect( titlebar, SIGNAL( showOperationMenu() ),
	     this, SIGNAL( showOperationMenu() ) );

    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    setMinimumSize( 128, 96 );

    clientw = window;
    if (!clientw)
	return;

    titlebar->setText( clientw->caption() );
    if( clientw->icon() )
	titlebar->setIcon( *clientw->icon() );

    int th = titlebar->sizeHint().height();

    bool hasBeenResize = clientw->testWState( WState_Resized );
    clientw->reparent( this, QPoint( contentsRect().x()+BORDER,
				     th + BORDER + TITLEBAR_SEPARATION + contentsRect().y() ), TRUE  );

    if ( !hasBeenResize ) {
	QSize cs = clientw->sizeHint();
	QSize s( cs.width() + 2*frameWidth() + 2*BORDER,
		 cs.height() + 3*frameWidth() + th +TITLEBAR_SEPARATION+2*BORDER );
	resize( s );
    } else {
	resize( clientw->width() + 2*frameWidth() + 2*BORDER,
		clientw->height() + 2*frameWidth() + th +2*BORDER);
    }

    clientw->installEventFilter( this );
}

QWorkspaceChild::~QWorkspaceChild()
{
}


void QWorkspaceChild::resizeEvent( QResizeEvent * )
{
    QRect r = contentsRect();
    int th = titlebar->sizeHint().height();
    titlebar->setGeometry( r.x() + BORDER, r.y() + BORDER, r.width() - 2*BORDER, th+1);

    if (!clientw)
	return;

    QRect cr( r.x() + BORDER, r.y() + BORDER + TITLEBAR_SEPARATION + th,
			r.width() - 2*BORDER,
			  r.height() - 2*BORDER - TITLEBAR_SEPARATION - th);
    clientSize = cr.size();
    clientw->setGeometry( cr );
}

void QWorkspaceChild::activate()
{
    ((QWorkspace*)parentWidget())->activateClient( clientWidget() );
}


bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{

    if ( !isActive() ) {
	if ( e->type() == QEvent::MouseButtonPress || e->type() == QEvent::FocusIn ) {
	    activate();
	}
    }

    if (o != clientw)
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Show:
	if ( isVisible() )
	    break;
	if (( (QShowEvent*)e)->spontaneous() )
	    break;
	((QWorkspace*)parentWidget())->showClient( clientWidget() );
	break;
    case QEvent::ShowMaximized:
	showMaximized();
	break;
    case QEvent::ShowMinimized:
	showMinimized();
	break;
    case QEvent::ShowNormal:
	showNormal();
	break;
    case QEvent::Hide:
	if ( !clientw->isVisibleTo( this ) ) {
	    if (iconw) {
		delete iconw->parentWidget();
	    }
	    hide();
	}
	break;
#if QT_VERSION >= 210
    case QEvent::CaptionChange:
	titlebar->setText( clientw->caption() );
	break;
    case QEvent::IconChange:
	if ( clientw->icon() )
	    titlebar->setIcon( *clientw->icon() );
	else {
	    QPixmap pm;
	    titlebar->setIcon( pm );
	}
	break;
#endif
    case QEvent::LayoutHint:
	//layout()->activate();
	break;
    case QEvent::Resize:
	{
	    QResizeEvent* re = (QResizeEvent*)e;
	    if ( re->size() != clientSize ){
		int th = titlebar->sizeHint().height();
		QSize s( re->size().width() + 2*frameWidth() + 2*BORDER,
			 re->size().height() + 3*frameWidth() + th +TITLEBAR_SEPARATION+2*BORDER );
		resize( s );
	    }
	}
	break;
    default:
	break;
    }

    return FALSE;
}


void QWorkspaceChild::childEvent( QChildEvent*  e)
{
    if ( e->type() == QEvent::ChildRemoved && e->child() == clientw ) {
	clientw = 0;
	if ( iconw )
	    delete iconw->parentWidget();
	close();
    }
}

void QWorkspaceChild::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	activate();
	mouseMoveEvent( e );
	buttonDown = TRUE;
	moveOffset = e->pos();
	invertedMoveOffset = rect().bottomRight() - e->pos();
    }
}

void QWorkspaceChild::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
	releaseKeyboard();
    }
}

void QWorkspaceChild::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown ) {
	if ( e->pos().y() <= RANGE && e->pos().x() <= RANGE)
	    mode = TopLeft;
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() >= width()-RANGE)
	    mode = BottomRight;
	else if ( e->pos().y() >= height()-RANGE && e->pos().x() <= RANGE)
	    mode = BottomLeft;
	else if ( e->pos().y() <= RANGE && e->pos().x() >= width()-RANGE)
	    mode = TopRight;
	else if ( e->pos().y() <= RANGE )
	    mode = Top;
	else if ( e->pos().y() >= height()-RANGE )
	    mode = Bottom;
	else if ( e->pos().x() <= RANGE )
	    mode = Left;
	else if (  e->pos().x() >= width()-RANGE )
	    mode = Right;
	else
	    mode = Right;
	setMouseCursor( mode );
	return;
    }

    if ( testWState(WState_ConfigPending) )
	return;

    QPoint globalPos = parentWidget()->mapFromGlobal( e->globalPos() );
    QPoint p = globalPos + invertedMoveOffset;
    QPoint pp = globalPos - moveOffset;

    QPoint mp( QMIN( pp.x(), geometry().right() - minimumWidth() +1 ),
	       QMIN( pp.y(), geometry().bottom() - minimumHeight() + 1 ) );
    mp = QPoint( QMAX( mp.x(), geometry().right() - maximumWidth() +1 ),
		 QMAX( mp.y(), geometry().bottom() -maximumHeight() +1) );


    QRect geom = geometry();

    switch ( mode ) {
    case TopLeft:
	geom =  QRect( mp, geometry().bottomRight() ) ;
	break;
    case BottomRight:
	geom =  QRect( geometry().topLeft(), p ) ;
	break;
    case BottomLeft:
	geom =  QRect( QPoint(mp.x(), geometry().y() ), QPoint( geometry().right(), p.y()) ) ;
	break;
    case TopRight:
	geom =  QRect( QPoint(geometry().x(), mp.y() ), QPoint( p.x(), geometry().bottom()) ) ;
	break;
    case Top:
	geom =  QRect( QPoint( geometry().left(), mp.y() ), geometry().bottomRight() ) ;
	break;
    case Bottom:
	geom =  QRect( geometry().topLeft(), QPoint( geometry().right(), p.y() ) ) ;
	break;
    case Left:
	geom =  QRect( QPoint( mp.x(), geometry().top() ), geometry().bottomRight() ) ;
	break;
    case Right:
	geom =  QRect( geometry().topLeft(), QPoint( p.x(), geometry().bottom() ) ) ;
	break;
    case Center:
	geom.moveTopLeft( pp );
	break;
    default:
	break;
    }

    if ( parentWidget()->rect().intersects( geom ) )
	setGeometry( geom );

#ifdef _WS_WIN_
    MSG msg;
    while( PeekMessage( &msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
      ;
#endif
    QApplication::syncX();
}



void QWorkspaceChild::enterEvent( QEvent * )
{
}

void QWorkspaceChild::leaveEvent( QEvent * )
{
    if ( !buttonDown )
	setCursor( arrowCursor );
}


void QWorkspaceChild::setActive( bool b)
{
    if ( b == act || !clientw)
	return;

    act = b;

    titlebar->setActive( act );
    if (iconw )
	iconw->setActive( act );

    if (act) {
	QObjectList* ol = clientw->queryList( "QWidget" );
	QObject *o;
	for ( o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	bool hasFocus = FALSE;
	for ( o = ol->first(); o; o = ol->next() ) {
	    hasFocus |= ((QWidget*)o)->hasFocus();
	}
	if ( !hasFocus ) {
	    clientw->setFocus(); // insufficient, need toplevel semantics ########
	    ( (QWorkSpaceChildProtectedWidget*)clientw)->reasonableFocus();
	}
	delete ol;

    }
    else {
	QObjectList* ol = clientw->queryList( "QWidget" );
	for (QObject* o = ol->first(); o; o = ol->next() ) {
	    o->removeEventFilter( this );
	    o->installEventFilter( this );
	}
	delete ol;
    }
}

bool QWorkspaceChild::isActive() const
{
    return act;
}

QWidget* QWorkspaceChild::clientWidget() const
{
    return clientw;
}


QWidget* QWorkspaceChild::iconWidget() const
{
    if ( !iconw ) {
	QWorkspaceChild* that = (QWorkspaceChild*) this;
	QVBox* vbox = new QVBox;
	vbox->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	vbox->resize( 196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth() );
	that->iconw = new QWorkspaceChildTitleBar( (QWorkspace*)parentWidget(), vbox, 0, TRUE );
	iconw->setActive( isActive() );
	connect( iconw, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( iconw, SIGNAL( doClose() ),
		 clientWidget(), SLOT( close() ) );
	connect( iconw, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
	connect( iconw, SIGNAL( doMaximize() ),
		 this, SLOT( showMaximized() ) );
	connect( iconw, SIGNAL( popupOperationMenu( const QPoint& ) ),
		 this, SIGNAL( popupOperationMenu( const QPoint& ) ) );
	connect( iconw, SIGNAL( showOperationMenu() ),
		 this, SIGNAL( showOperationMenu() ) );
    }
    iconw->setText( clientWidget()->caption() );
    if ( clientWidget()->icon() )
	iconw->setIcon( *clientWidget()->icon() );
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    ((QWorkspace*)parentWidget())->minimizeClient( clientWidget() );
}

void QWorkspaceChild::showMaximized()
{
    ((QWorkspace*)parentWidget())->maximizeClient( clientWidget() );
}

void QWorkspaceChild::showNormal()
{
    ((QWorkspace*)parentWidget())->normalizeClient( clientWidget() );
    if (iconw) {
	delete iconw->parentWidget();
    }
}


void QWorkspaceChild::adjustToFullscreen()
{
    setGeometry( -clientw->x(), -clientw->y(),
		 parentWidget()->width() + width() - clientw->width(),
		 parentWidget()->height() + height() - clientw->height() );
}


/*!
  Sets an appropriate cursor shape for the logical mouse position \a m

  \sa QWidget::setCursor()
 */
void QWorkspaceChild::setMouseCursor( MousePosition m )
{
    switch ( m ) {
    case TopLeft:
    case BottomRight:
	setCursor( sizeFDiagCursor );
	break;
    case BottomLeft:
    case TopRight:
	setCursor( sizeBDiagCursor );
	break;
    case Top:
    case Bottom:
	setCursor( sizeVerCursor );
	break;
    case Left:
    case Right:
	setCursor( sizeHorCursor );
	break;
    default:
	setCursor( arrowCursor );
	break;
    }
}

void QWorkspaceChild::keyPressEvent( QKeyEvent * e )
{
    if ( !isMove() && !isResize() )
	return;
    bool is_control = e->state() & ControlButton;
    int delta = is_control?1:8;
    QPoint pos = QCursor::pos();
    QPoint invertedMoveOffset; //TODO
    switch ( e->key() ) {
    case Key_Left:
	pos.rx() -= delta;
	if ( pos.x() <= QApplication::desktop()->geometry().left() ) {
	    if ( mode == TopLeft || mode == BottomLeft ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomRight )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = TopLeft;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Right:
	pos.rx() += delta;
	if ( pos.x() >= QApplication::desktop()->geometry().right() ) {
	    if ( mode == TopRight || mode == BottomRight ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = BottomRight;
	    else if ( mode == TopLeft )
		mode = TopRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Up:
	pos.ry() -= delta;
	if ( pos.y() <= QApplication::desktop()->geometry().top() ) {
	    if ( mode == TopLeft || mode == TopRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = TopLeft;
	    else if ( mode == BottomRight )
		mode = TopRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Down:
	pos.ry() += delta;
	if ( pos.y() >= QApplication::desktop()->geometry().bottom() ) {
	    if ( mode == BottomLeft || mode == BottomRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == TopLeft )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = BottomRight;
	    setMouseCursor( mode );
	    grabMouse( cursor() );
	}
	break;
    case Key_Space:
    case Key_Return:
    case Key_Enter:
	moveResizeMode = FALSE;
	releaseMouse();
	releaseKeyboard();
	buttonDown = FALSE;
	break;
    default:
	return;
    }
    QCursor::setPos( pos );
}


void QWorkspaceChild::doResize()
{
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = mapFromGlobal( QCursor::pos() );
    if ( moveOffset.x() < width()/2) {
	if ( moveOffset.y() < height()/2)
	    mode = TopLeft;
	else
	    mode = BottomLeft;
    } else {
	if ( moveOffset.y() < height()/2)
	    mode = TopRight;
	else
	    mode = BottomRight;
    }
    invertedMoveOffset = rect().bottomRight() - moveOffset;
    setMouseCursor( mode );
    grabMouse( cursor()  );
    grabKeyboard();
    resizeHorizontalDirectionFixed = FALSE;
    resizeVerticalDirectionFixed = FALSE;
}

void QWorkspaceChild::doMove()
{
    mode = Center;
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = mapFromGlobal( QCursor::pos() );
    invertedMoveOffset = rect().bottomRight() - moveOffset;
    grabMouse( arrowCursor );
    grabKeyboard();
}

#include "qworkspace.moc"
