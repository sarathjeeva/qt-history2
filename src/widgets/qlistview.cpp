/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#38 $
**
** Implementation of QListView widget class
**
** Created : 970809
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qlistview.h"
#include "qtimer.h"
#include "qheader.h"
#include "qpainter.h"
#include "qstack.h"
#include "qlist.h"
#include "qstrlist.h"
#include "qapp.h"
#include "qpixmap.h"
#include "qkeycode.h"

#include <stdarg.h> // va_list
#include <stdlib.h> // qsort

RCSTAG("$Id: //depot/qt/main/src/widgets/qlistview.cpp#38 $");


const int Unsorted = 32767;


struct QListViewPrivate
{
    // classes that are here to avoid polluting the global name space

    // the magical hidden mother of all items
    struct Root: public QListViewItem {
	Root( QListView * parent );

	void setHeight( int );
	void invalidateHeight();
	void setup();

	QListView * lv;
    };

    // for the stack used in drawContentsOffset()
    struct Pending {
	Pending( int level, int ypos, QListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l; // top pixel in this item, in list view coordinates
	int y; // level of this item in the tree
	QListViewItem * i; // the item itself
    };

    // to remember what's on screen
    struct DrawableItem {
	DrawableItem( Pending * pi ) { y=pi->y; l=pi->l; i=pi->i; };
	int y;
	int l;
	QListViewItem * i;
    };

    // for sorting
    struct SortableItem {
	QString key;
	QListViewItem * i;
    };

    // private variables used in QListView
    QHeader * h;
    Root * r;

    QListViewItem * currentSelected;
    QListViewItem * focusItem;

    QTimer * timer;
    int levelWidth;

    // the list of drawables, and the range drawables covers entirely
    // (it may also include a few items above topPixel)
    QList<DrawableItem> * drawables;
    int topPixel;
    int bottomPixel;

    bool multi;

    // TRUE if the widget should take notice of mouseReleaseEvent
    bool buttonDown;

    // sort column and order
    int column;
    bool ascending;

    // suggested height for the items
    int fontMetricsHeight;
    bool allColumnsShowFocus;
};



/*!
  \class QListViewItem qlistview.h
  \brief The QListViewItem class implements a listview item.

  This class is not finished.
 */



/*!  Create a new list view item in the QListView \a parent */

QListViewItem::QListViewItem( QListView * parent )
{
    init();
    parent->insertItem( this );
}


/*!  Create a new list view item which is a child of \a parent */

QListViewItem::QListViewItem( QListViewItem * parent )
{
    init();
    parent->insertItem( this );
}



/*!  Create a new list view item in the QListView \a parent,
  with the contents asdf asdf asdf ### sex vold ###

*/

QListViewItem::QListViewItem( QListView * parent,
			      const char * firstLabel, ... )
{
    init();
    parent->insertItem( this );

    columnTexts = new QStrList();
    columnTexts->append( firstLabel );
    va_list ap;
    const char * nextLabel;
    va_start( ap, firstLabel );
    while ( (nextLabel = va_arg(ap, const char *)) != 0 )
	columnTexts->append( nextLabel );
    va_end( ap );
}


/*!  Create a new list view item which is a child of \a parent,
  with the contents asdf asdf asdf ### sex vold ###

*/

QListViewItem::QListViewItem( QListViewItem * parent,
			      const char * firstLabel, ... )
{
    init();
    parent->insertItem( this );

    columnTexts = new QStrList();
    columnTexts->append( firstLabel );
    va_list ap;
    const char * nextLabel;
    va_start( ap, firstLabel );
    while ( (nextLabel = va_arg(ap, const char *)) != 0 )
	columnTexts->append( nextLabel );
    va_end( ap );
}

/*!  Perform the initializations that's common to the constructors. */

void QListViewItem::init()
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = FALSE;

    childCount = 0;
    parentItem = 0;
    siblingItem = childItem = 0;

    columnTexts = 0;

    selected = 0;

    lsc = Unsorted;
    lso = TRUE; // unsorted in ascending order :)
    configured = FALSE;
    expandable = FALSE;
    selectable = TRUE;
}


/*!  Deletes this item and all children of it, freeing up all
  allocated resources.
*/

QListViewItem::~QListViewItem()
{
    if ( parentItem )
	parentItem->removeItem( this );
    QListViewItem * nextChild = childItem;
    while ( childItem ) {
	nextChild = childItem->siblingItem;
	delete childItem;
	childItem = nextChild;
    }

}


/*!  Inserts \a newChild into its list of children.  Called by the
  constructor of \a newChild.
*/

void QListViewItem::insertItem( QListViewItem * newChild )
{
    invalidateHeight();
    newChild->siblingItem = childItem;
    childItem = newChild;
    childCount++;
    newChild->parentItem = this;
    lsc = Unsorted;
    newChild->ownHeight = 0;
    newChild->configured = FALSE;
}


/*!  Removes \a tbg from this object's list of children.
*/

void QListViewItem::removeItem( QListViewItem * tbg )
{
    invalidateHeight();

    if ( tbg == listView()->d->currentSelected )
	listView()->d->currentSelected = 0;

    if ( tbg == listView()->d->focusItem )
	listView()->d->focusItem = 0;

    childCount--;

    QListViewItem ** nextChild = &childItem;
    while( nextChild && *nextChild && tbg != *nextChild )
	nextChild = &((*nextChild)->siblingItem);

    if ( nextChild && tbg == *nextChild )
	*nextChild = (*nextChild)->siblingItem;
    tbg->parentItem = 0;
}


/*!  Returns a key that can be used for sorting by column \a column.
  The default implementation returns text().

  The return value is immediately copied.

  \sa sortChildItems()
*/

const char * QListViewItem::key( int column ) const
{
    return text( column );
}


static int cmp( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    return qstrcmp( ((QListViewPrivate::SortableItem *)n1)->key,
		    ((QListViewPrivate::SortableItem *)n2)->key );
}


/*!  Undocumented for the time being.  I'll redoc when its semantics
  have settled down.

  \sa key()
*/

void QListViewItem::sortChildItems( int column, bool ascending )
{
    // we try HARD not to sort.  if we're already sorted, don't.
    if ( column == (int)lsc && ascending == (bool)lso )
	return;

    // more dubiously - only sort if the child items "exist"
    if ( !isOpen() )
	return;

    lsc = column;
    lso = ascending;

    // and don't sort if we already have the right sorting order
    if ( childItem == 0 || childItem->siblingItem == 0 )
	return;

    // make an array we can sort in a thread-safe way using qsort()
    QListViewPrivate::SortableItem * siblings
	= new QListViewPrivate::SortableItem[childCount];
    QListViewItem * s = childItem;
    int i = 0;
    while ( s && i<childCount ) {
	siblings[i].key = s->key( column );
	siblings[i].i = s;
	s = s->siblingItem;
	i++;
    }

    // and do it.
    qsort( siblings, childCount,
	   sizeof( QListViewPrivate::SortableItem ), cmp );

    // build the linked list of siblings, in the appropriate
    // direction, and finally set this->childItem to the new top
    // child.
    if ( ascending ) {
	for( i=0; i < childCount-1; i++ )
	    siblings[i].i->siblingItem = siblings[i+1].i;
	siblings[childCount-1].i->siblingItem = 0;
	childItem = siblings[0].i;
    } else {
	for( i=childCount-1; i >0; i-- )
	    siblings[i].i->siblingItem = siblings[i-1].i;
	siblings[0].i->siblingItem = 0;
	childItem = siblings[childCount-1].i;
    }

    // we don't want no steenking memory leaks.
    delete[] siblings;
}


/*!  Sets this item's own height to \a height pixels.  This implictly
  changes totalHeight() too.

  Note that e.g. a font change causes this height to be overwitten
  unless you reimplement setup().

  \sa ownHeight() totalHeight() isOpen();
*/

void QListViewItem::setHeight( int height )
{
    if ( ownHeight != height ) {
	ownHeight = height;
	invalidateHeight();
    }
}


/*!  Invalidates the cached total height of this item including
  all open children.

  \sa setHeight() ownHeight() totalHeight()
*/

void QListViewItem::invalidateHeight()
{
    if ( maybeTotalHeight < 0 )
	return;
    maybeTotalHeight = -1;
    if ( parentItem && parentItem->isOpen() )
	parentItem->invalidateHeight();
}


/*!  Sets this item to be open (its children are visible) if \a o is
  TRUE, and to be closed (its children are not visible) if \a o is
  FALSE.

  Also does some bookeeping.

  \sa ownHeight() totalHeight()
*/

void QListViewItem::setOpen( bool o )
{
    if ( o == (bool)open )
	return;
    open = o;

    if ( !childCount )
	return;
    invalidateHeight();

    if ( !configured ) {
	QListViewItem * l = this;
	QStack<QListViewItem> s;
	while( l ) {
	    if ( l->open && l->childItem ) {
		s.push( l->childItem );
	    } else if ( l->childItem ) {
		// first invisible child is unconfigured
		QListViewItem * c = l->childItem;
		while( c ) {
		    c->configured = FALSE;
		    c = c->siblingItem;
		}
	    }
	    l->configured = TRUE;
	    l->setup();
	    l = (l == this) ? 0 : l->siblingItem;
	    if ( !l && !s.isEmpty() )
		l = s.pop();
	}
    }

    if ( !open )
	return;
    enforceSortOrder();

}


/*!  This virtual function is called before the first time QListView
  needs to know the height or any other graphical attribute of this
  class, and whenever the font, GUI style or colors of the list view
  change.

  The default sets the item's height.
*/

void QListViewItem::setup()
{
    setHeight( listView()->d->fontMetricsHeight );
}


/*! \fn bool QListViewItem::isSelectable() const

  Returns TRUE if the item is selectable (as it is by default) and
  FALSE if it isn't. \sa setSelectable() */


/*!  Sets this items to be selectable if \a enable is TRUE (the
  default) or not to be selectable if \a enable is FALSE.

  The user is not able to select a non-selectable item using either
  the keyboard or mouse.  The application programmer still can, of
  course.  \sa isSelectable() */

void QListViewItem::setSelectable( bool enable )
{
    selectable = enable;
}


/*! \fn bool QListViewItem::isExpandable() { return expnadable; }

  Returns TRUE if this item is selectable even when it has no
  children.
*/

/*!  Sets this item to be expandable even if it has no children if \a
  enable is TRUE, and to be expandable only if it has children if \a
  enable is FALSE (the default).

  The dirview example uses this in the canonical fashion: It checks
  whether the directory is empty in setup() and calls
  setExpandable(TRUE) if not, and in setOpen() it reads the contents
  of the directory and inserts items accordingly.
*/

void QListViewItem::setExpandable( bool enable )
{
    expandable = enable;
}


/*!  Enforce that this object's children are sorted appropriately.

  This only works if every item in the chain from the root item to
  this item is sorted appropriately.

  \sa sortChildItems()
*/


void QListViewItem::enforceSortOrder()
{
    if( parentItem && (parentItem->lsc != lsc || parentItem->lso != lso) )
	sortChildItems( (int)parentItem->lsc, (bool)parentItem->lso );
}


/*! \fn bool QListViewItem::isSelected() const

  Returns TRUE if this item is selected, or FALSE if it is not.

  \sa setSelection() selectionChanged() QListViewItem::setSelected()
*/


/*!  Sets this item to be selected \a s is TRUE, and to not be
  selected if \a o is FALSE.  Doesn't repaint anything in either case.

  \sa ownHeight() totalHeight() */

void QListViewItem::setSelected( bool s )
{
    selected = s ? 1 : 0;
}


/*!  Returns the total height of this object, including any visible
  children.  This height is recomputed lazily and cached for as long
  as possible.

  setOwnHeight() can be used to set the item's own height, setOpen()
  to show or hide its children, and invalidateHeight() to invalidate
  the cached height.
*/

int QListViewItem::totalHeight() const
{
    if ( maybeTotalHeight >= 0 )
	return maybeTotalHeight;
    QListViewItem * that = (QListViewItem *)this;
    if ( !that->configured ) {
	that->configured = TRUE;
	that->setup(); // ### virtual non-const function called in const
    }
    that->maybeTotalHeight = that->ownHeight;

    if ( !that->isOpen() || !that->children() )
	return that->ownHeight;

    QListViewItem * child = that->childItem;
    while ( child != 0 ) {
	that->maybeTotalHeight += child->totalHeight();
	child = child->siblingItem;
    }
    return that->maybeTotalHeight;
}


/*!  Returns the text in column \a column, or else 0.

  The returned string must be copied or used at once;
  reimplementations of this function are at liberty to e.g. return a
  pointer into a static buffer.

  \sa key() paintCell()
*/

const char * QListViewItem::text( int column ) const
{
    if ( columnTexts && (int)columnTexts->count() > column )
	return columnTexts->at( column );
    else
	return 0;
}


/*!  This virtual function paints the contents of one cell.

  \a p is a QPainter open on the relevant paint device.  \a pa is
  translated so 0, 0 is the top left pixel in the cell and \a width-1,
  height()-1 is the bottom right pixel \e in the cell.  The other
  properties of \a p (pen, brush etc) are undefined.  \a cg is the
  color group to use.  \a column is the logical column number within
  the item that is to be painted; 0 is the column which may contain a
  tree.  \a showFocus is TRUE if this item should indicate that the
  list view has keyboard focus, FALSE otherwise.

  The rectangle to be painted is in an undefined state when this
  function is called, so you \e must draw on all the pixels.

  \sa paintBranches(), QListView::drawContentsOffset()
*/

void QListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, bool showFocus ) const
{
    if ( !p )
	return;

    QListView *lv = listView();
    int r = 2;
    QPixmap * icon = 0; // ### temporary! to be replaced with an array

    p->fillRect( 0, 0, width, height(), cg.base() );

    if ( icon && !column ) {
	p->drawPixmap( 0, (height()-icon->height())/2, *icon );
	r += icon->width();
    }

    const char * t = text( column );
    if ( t ) {
	if ( lv )
	    p->setFont( lv->font() );

	if ( isSelected() && column==0 ) {
	    p->fillRect( r-2, 0, width - r + 2, height(),
			 QApplication::winStyleHighlightColor() );
	    p->setPen( white ); // ###
	} else {
	    p->setPen( cg.text() );
	}

	// should do the ellipsis thing here
	p->drawText( r, 0, width-2-r, height(), AlignLeft + AlignVCenter, t );
    }

    if ( showFocus && !column ) {
	if ( listView()->style() == WindowsStyle ) {
	    p->drawWinFocusRect( r-2, 0, width-r+2, height() );
	} else {
	    p->setPen( black );
	    p->drawRect( r-2, 0, width-r+2, height() );
	}
    }
}


/*!  Paints a set of branches from this item to (some of) its children.

  \a p is set up with clipping and translation so that you can draw
  only in the rectangle you need to; \a cg is the color group to use,
  0,top is the top left corner of the update rectangle, w-1,top is the
  top right corner, 0,bottom-1 is the bottom left corner and the
  bottom right corner is left as an excercise for the reader.

  The update rectangle is in an undefined state when this function is
  called; this function must draw on \e all of the pixels.

  \sa paintCell(), QListView::drawContentsOffset()
*/

void QListViewItem::paintBranches( QPainter * p, const QColorGroup & cg,
				   int w, int y, int h, GUIStyle s ) const
{
    p->fillRect( 0, 0, w, h, cg.base() );

    const QListViewItem * child = firstChild();
    int linetop = 0, linebot = 0;

    int dotoffset = y & 1;

    // each branch needs at most two lines, ie. four end points
    QPointArray dotlines( children() * 4 );
    int c = 0;

    // skip the stuff above the exposed rectangle
    while ( child && y + child->height() <= 0 ) {
	y += child->totalHeight();
	child = child->nextSibling();
    }

    int bx = w / 2;

    // paint stuff in the magical area
    while ( child && y < h ) {
	linebot = y + child->height()/2;
	if ( child->expandable || child->children() ) {
	    if ( s == WindowsStyle ) {
		// needs a box
		p->setPen( cg.dark() );
		p->drawRect( bx-4, linebot-4, 9, 9 );
		// plus or minus
		p->setPen( cg.foreground() ); // ### windows uses black
		p->drawLine( bx - 2, linebot, bx + 2, linebot );
		if ( !child->isOpen() )
		    p->drawLine( bx, linebot - 2, bx, linebot + 2 );

	    } else {
		// down or right arrow.  fucking ugly, but hey.
		::qDrawArrow( p, child->isOpen() ? DownArrow : RightArrow,
			      s, FALSE, bx - 5, linebot - 5, 11, 11,
			      cg, TRUE );
	    }
	    // dotlinery
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, linebot - 5 );
	    dotlines[c++] = QPoint( bx + 6, linebot );
	    dotlines[c++] = QPoint( w, linebot );
	    linebot += 6;
	    linetop = linebot;
	} else {
	    // just dotlinery
	    dotlines[c++] = QPoint( bx + 2, linebot ); // ### +2? +1?
	    dotlines[c++] = QPoint( w, linebot );
	}

	y += child->totalHeight();
	child = child->nextSibling();
    }

    if ( child ) // there's a child, so move linebot to edge of rectangle
	linebot = h;

    if ( linetop < linebot ) {
	dotlines[c++] = QPoint( bx, linetop );
	dotlines[c++] = QPoint( bx, linebot );
    }

    if ( s == MotifStyle ) {
	p->setPen( cg.foreground() );
	p->drawLineSegments( dotlines, 0, c/2 );
    } else {
	// this could be done much faster on X11, but not on Windows.
	// oh well.  do it the hard way.

	// thought: keep around a 64*1 and a 1*64 bitmap such that
	// drawPixmap'ing them is equivalent to drawing a horizontal
	// or vertical line with the appropriate pen.
	
	QPointArray dots( (h+4)/2 + (children()*w+3)/4 );
	// at most one dot for every second y coordinate, plus the
	// spillover at the top.  at most dot for every second x
	// coordinate for half of the width for every child.  both
	// divisions must be rounded up to the nearest integer.
	int i = 0; // index into dots
	int line; // index into dotlines
	int point; // relevant coordinate of current point
	int end; // same coordinate of the end of the current line
	int other; // the other coordinate of the current point/line
	for( line = 0; line < c; line += 2 ) {
	    // assumptions here: lines are horizontal or vertical.
	    // lines always start with the numerically lowest
	    // coordinate.
	    if ( dotlines[line].y() == dotlines[line+1].y() ) {
		end = dotlines[line+1].x();
		point = dotlines[line].x();
		other = dotlines[line].y();
		while( point < end ) {
		    dots[i++] = QPoint( point, other );
		    point += 2;
		}
	    } else {
		end = dotlines[line+1].y();
		point = dotlines[line].y();
		if ( (point & 1) != dotoffset )
		    point++;
		other = dotlines[line].x();
		while( point < end ) {
		    dots[i++] = QPoint( other, point );
		    point += 2;
		}
	    }
	}
	p->setPen( cg.dark() );
	p->drawPoints( dots, 0, i );
    }
}



QListViewPrivate::Root::Root( QListView * parent )
    : QListViewItem( parent )
{
    lv = parent;
    setHeight( 0 );
    setOpen( TRUE );
}


void QListViewPrivate::Root::setHeight( int )
{
    QListViewItem::setHeight( 0 );
}


void QListViewPrivate::Root::invalidateHeight()
{
    QListViewItem::invalidateHeight();
    lv->triggerUpdate();
}


void QListViewPrivate::Root::setup()
{
    // nothing
}


/*!
  \class QListView qlistview.h
  \brief The QListView class implements a tree/list view.

  Lots of things don't work yet.  The tree view functionality hasn't
  been tested since the last extensive changes, focus stuff doesn't
  work, input is ignored and so on.
*/

/*!  Creates a new empty list view, with \a parent as a parent and \a
  name as object name. */

QListView::QListView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    d = new QListViewPrivate;
    d->timer = new QTimer( this );
    d->levelWidth = 0;
    d->r = 0;
    d->h = new QHeader( this, "list view header" );
    d->h->installEventFilter( this );
    d->currentSelected = 0;
    d->focusItem = 0;
    d->drawables = 0;
    d->multi = 0;
    d->column = 0;
    d->ascending = TRUE;
    d->allColumnsShowFocus = FALSE;
    d->fontMetricsHeight = fontMetrics().height();

    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(updateContents()) );
    connect( d->h, SIGNAL(sizeChange( int, int )),
	     this, SLOT(triggerUpdate()) );
    connect( d->h, SIGNAL(sizeChange( int, int )),
	     this, SIGNAL(sizeChanged()) );
    connect( d->h, SIGNAL(moved( int, int )),
	     this, SLOT(triggerUpdate()) );
    connect( d->h, SIGNAL(sectionClicked( int )),
	     this, SLOT(changeSortColumn( int )) );
    connect( horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	     d->h, SLOT(setOffset(int)) );
    connect( horizontalScrollBar(), SIGNAL(valueChanged(int)),
	     d->h, SLOT(setOffset(int)) );

    // will access d->r
    QListViewPrivate::Root * r = new QListViewPrivate::Root( this );
    d->r = r;
    d->r->setSelectable( FALSE );

    setFocusProxy( viewport() );
}


/*!  Deletes the list view and all items in it, and frees all
  allocated resources.  */

QListView::~QListView()
{
    delete d->r;
    delete d;
}


/*!  Calls QListViewItem::paintCell() and/or
  QListViewItem::paintBranches() for all list view items that
  require repainting.  See the documentation for those functions for
  details.
*/

void QListView::drawContentsOffset( QPainter * p, int ox, int oy,
				    int cx, int cy, int cw, int ch )
{
    if ( !d->drawables ||
	 d->drawables->isEmpty() ||
	 d->topPixel > cy ||
	 d->bottomPixel < cy + ch - 1 ||
	 d->r->maybeTotalHeight < 0 )
	buildDrawableList();

    QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );

    QRect r;
    int l;
    int fx = -1, x, fc = 0, lc = 0;
    l = 0;
    int tx = -1;
    struct QListViewPrivate::DrawableItem * current;

    while ( (current = it.current()) != 0 ) {
	++it;

	int ih = current->i->height();
	int ith = current->i->totalHeight();
	int c;
	int cs;

	// need to paint current?
	if ( ih > 0 && current->y < cy+ch && current->y+ih >= cy ) {
	    if ( fx < 0 ) {
		// find first interesting column, once
		x = 0;
		c = 0;
		cs = d->h->cellSize( 0 );
		while ( x + cs <= cx && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		fx = x;
		fc = c;
		while( x < cx + cw && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		lc = c;
		// also make sure that the top item indicates focus,
		// if nothing would otherwise
		if ( !d->focusItem )
		    d->focusItem = current->i;
	    }

	    x = fx;
	    c = fc;

            // draw to last interesting column
            while( c < lc ) {
		int i = d->h->mapToLogical( c );
                cs = d->h->cellSize( c );
                r.setRect( x + ox, current->y + oy, cs, ih );
                if ( c + 1 == lc && x + cs < cx + cw )
                    r.setRight( cx + cw + ox - 1 );
		if ( i==0 && current->l > 0 )
		    r.setLeft( r.left() + (current->l-1) * treeStepSize() );

		p->save();
                p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
                p->translate( r.left(), r.top() );
		current->i->paintCell( p, colorGroup(),
				       d->h->mapToLogical( c ), r.width(),
				       hasFocus() &&
				       (d->allColumnsShowFocus ||
					current->i == d->focusItem) );
		p->restore();
		x += cs;
		c++;
	    }
	}

	if ( tx < 0 )
	    tx = d->h->cellPos( d->h->mapToActual( 0 ) );

	// do any children of current need to be painted?
	if ( current->i->isOpen() &&
	     current->y + ith > cy &&
	     current->y + ih < cy + ch &&
	     tx < cx + cw &&
	     tx + current->l * treeStepSize() > cx ) {
	    // compute the clip rectangle the safe way

	    int rtop = current->y + ih;
	    int rbottom = current->y + ith;
	    int rleft = tx + (current->l-1)*treeStepSize();
	    int rright = rleft + treeStepSize();

	    int crtop = QMAX( rtop, cy );
	    int crbottom = QMIN( rbottom, cy+ch );
	    int crleft = QMAX( rleft, cx );
	    int crright = QMIN( rright, cx+cw );

	    r.setRect( crleft+ox, crtop+oy,
		       crright-crleft, crbottom-crtop );

	    if ( r.isValid() ) {
		p->save();
		p->setClipRect( r );
		p->translate( rleft+ox, crtop+oy );
		current->i->paintBranches( p, colorGroup(), treeStepSize(),
					   rtop - crtop, r.height(), style() );
		p->restore();
	    }
	}
    }

    if ( d->r->totalHeight() < cy + ch ) {
	// really should call some virtual method, or at least use
	// something more configurable than colorGroup().base()
	p->fillRect( cx + ox, d->r->totalHeight() + oy,
		     cw, cy + ch - d->r->totalHeight(),
		     colorGroup().base() );
    }
}


/*! Rebuild the lis of drawable QListViewItems.  This function is
  const so that const functions can call it without requiring
  d->drawables to be mutable */

void QListView::buildDrawableList() const
{
    if ( (int)d->r->lsc != d->column || (bool)d->r->lso != d->ascending )
	d->r->sortChildItems( d->column, d->ascending );

    QStack<QListViewPrivate::Pending> stack;
    stack.push( new QListViewPrivate::Pending( 0, 0, d->r ) );

    // could mess with cy and ch in order to speed up vertical
    // scrolling
    int cy = -contentsY();
    int ch = ((QListView *)this)->viewport()->height();
    d->topPixel = cy + ch; // one below bottom
    d->bottomPixel = cy - 1; // one above top

    struct QListViewPrivate::Pending * cur;

    // used to work around lack of support for mutable
    QList<QListViewPrivate::DrawableItem> * dl;

    if ( d->drawables ) {
	dl = ((QListView *)this)->d->drawables;
	dl->clear();
    } else {
	dl = new QList<QListViewPrivate::DrawableItem>;
	dl->setAutoDelete( TRUE );
	((QListView *)this)->d->drawables = dl;
    }

    while ( !stack.isEmpty() ) {
	cur = stack.pop();

	int ih = cur->i->height();
	int ith = cur->i->totalHeight();

	// is this item, or its branch symbol, inside the viewport?
	if ( cur->y + ith >= cy && cur->y < cy + ch ) {
	    dl->append( new QListViewPrivate::DrawableItem(cur));
	    // perhaps adjust topPixel up to this item?  may be adjusted
	    // down again if any children are not to be painted
	    if ( cur->y < d->topPixel )
		d->topPixel = cur->y;
	    // bottompixel is easy: the bottom item drawn contains it
	    d->bottomPixel = cur->y + ih - 1;
	}

	// push younger sibling of cur on the stack?
	if ( cur->y + ith < cy+ch && cur->i->siblingItem )
	    stack.push( new QListViewPrivate::Pending(cur->l,
						      cur->y + ith,
						      cur->i->siblingItem));

	// do any children of cur need to be painted?
	if ( cur->i->isOpen() &&
	     cur->y + ith > cy &&
	     cur->y + ih < cy + ch ) {
	    cur->i->enforceSortOrder();

	    QListViewItem * c = cur->i->childItem;
	    int y = cur->y + ih;

	    // if any of the children are not to be painted, skip them
	    // and invalidate topPixel
	    while ( c && y + c->totalHeight() <= cy ) {
		y += c->totalHeight();
		c = c->siblingItem;
		d->topPixel = cy + ch;
	    }

	    // push one child on the stack, if there is at least one
	    // needing to be painted
	    if ( c && y < cy+ch )
		stack.push( new QListViewPrivate::Pending( cur->l + 1,
							   y, c ) );
	}

	delete cur;
    }
}




/*!  Returns the number of pixels a child is offset from its parent.
  This number has meaning only for tree views.

  \sa setTreeStepSize()
*/

int QListView::treeStepSize() const
{
    return d->levelWidth;
}


/*!  Set teh the number of pixels a child is offset from its parent,
  in a tree view.

  \sa treeStepSize()
*/

 void QListView::setTreeStepSize( int l )
{
    if ( l != d->levelWidth ) {
	d->levelWidth = l;
	// update
    }
}


/*!  Inserts a top-level QListViewItem into this list view.  You
  should not need to call this; the QListViewItem constructor does it
  for you.
*/

void QListView::insertItem( QListViewItem * i )
{
    if ( d->r ) // not for d->r itself
	d->r->insertItem( i );
}


/*!  Remove all the list view items from the list, and trigger an
  update. \sa triggerUpdate() */

void QListView::clear()
{
    if ( d->drawables )
	d->drawables->clear();

    d->currentSelected = 0;
    d->focusItem = 0;
    contentsResize( d->h->width(), viewport()->height() ); // ### ?

    // if it's down its downness makes no sense, so undown it
    d->buttonDown = FALSE;

    QListViewItem *c = (QListViewItem *)d->r->firstChild();
    QListViewItem *n;
    while( c ) {
	n = (QListViewItem *)c->nextSibling();
	delete c;
	c = n;
    }
}


/*!  Sets the header for column \a column to be labelled \a label and
  be \a size pixels wide.  If \a column is negative (as it is by
  default) setColumn() adds a new column at the right end. */

void QListView::setColumn( const char * label, int size, int column )
{
    if ( column < 0 )
	d->h->setCellSize( d->h->addLabel( label ), size );
    else
	d->h->setLabel( column, label, size );
}


/*!  Reimplemented to set the correct background mode and viewed area
  size. */

void QListView::show()
{
    QWidget * v = viewport();
    if ( v )
	v->setBackgroundMode( NoBackground );

    reconfigureItems();

    contentsResize( 250, d->r->totalHeight() ); // ### 250
    QScrollView::show();
}


/*!  Updates the sizes of the viewport, header, scrollbars and so on.
  Don't call this directly; call triggerUpdates() instead.
*/

void QListView::updateContents()
{
    int w = 0;
    for( int i=0; i<d->h->count(); i++ )
	w += d->h->cellSize( i );

    int h = d->h->sizeHint().height();
    d->h->setGeometry( frameWidth(), frameWidth(),
		       frameRect().width(), h );
    setMargins( 0, h, 0, 0 );

    contentsResize( w, d->r->totalHeight() );  // repaints
    viewport()->repaint();
}


/*!  Trigger a size-and-stuff update during the next iteration of the
  event loop.  Cleverly makes sure that there'll be just one. */

void QListView::triggerUpdate()
{
    if ( d && d->drawables ) {
	delete d->drawables;
	d->drawables = 0;
    }
    d->timer->start( 0, TRUE );
}


/*!  Redirects events for the viewport to mousePressEvent(),
  keyPressEvent() and friends. */

bool QListView::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return FALSE;

    if ( o == d->h &&
	 e->type() >= Event_MouseButtonPress &&
	 e->type() <= Event_MouseMove ) {
	QMouseEvent * me = (QMouseEvent *)e;
	QMouseEvent me2( me->type(),
			 QPoint( me->pos().x(),
				 me->pos().y() - d->h->height() ),
			 me->button(), me->state() );
	switch( me2.type() ) {
	case Event_MouseButtonPress:
	    mousePressEvent( &me2 );
	    break;
	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( &me2 );
	    break;
	case Event_MouseMove:
	    mouseMoveEvent( &me2 );
	    break;
	case Event_MouseButtonRelease:
	    mouseReleaseEvent( &me2 );
	    break;
	default:
	    break;
	}
    } else if ( o == viewport() ) {
	QMouseEvent * me = (QMouseEvent *)e;
	QFocusEvent * fe = (QFocusEvent *)e;

	switch( e->type() ) {
	case Event_MouseButtonPress:
	    mousePressEvent( me );
	    break;
	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( me );
	    break;
	case Event_MouseMove:
	    mouseMoveEvent( me );
	    break;
	case Event_MouseButtonRelease:
	    mouseReleaseEvent( me );
	    break;
	case Event_FocusIn:
	    focusInEvent( fe );
	    break;
	case Event_FocusOut:
	    focusOutEvent( fe );
	    break;
	default:
	    // nothing
	    break;
	}
    }
    return QScrollView::eventFilter( o, e );
}

/*!
  Returns the listview containing this item.
*/

QListView * QListViewItem::listView() const
{
   const QListViewItem * l = this;
    while( l && l->parentItem )
	l = l->parentItem;
    return ((QListViewPrivate::Root*)l)->lv;
}


/*!  Returns a pointer to the item immediately above this item on the
  screen.  This is usually the item's closest older sibling, but may
  also be its parent or its next older sibling's youngest child, or
  something else if anyoftheabove->height() returns 0.

  This function assumes that all parents of this item are open
  (ie. that this item is visible, or can be made visible by
  scrolling).

  \sa itemBelow() itemRect()
*/

QListViewItem * QListViewItem::itemAbove()
{
    if ( !parentItem )
	return 0;

    QListViewItem * c = parentItem;
    if ( c->childItem != this ) {
	c = c->childItem;
	while( c && c->siblingItem != this )
	    c = c->siblingItem;
	if ( !c )
	    return 0;
	while( c->isOpen() && c->childItem ) {
	    c = c->childItem;
	    while( c->siblingItem )
		c = c->siblingItem;		// assign c's sibling to c
	}
    }
    if ( c && !c->height() )
	return c->itemAbove();
    return c;
}


/*!  Returns a pointer to the item immediately below this item on the
  screen.  This is usually the item's eldest child, but may also be
  its next younger sibling, its parent's next younger sibling,
  granparent's etc., or something else if anyoftheabove->height()
  returns 0.

  This function assumes that all parents of this item are open
  (ie. that this item is visible, or can be made visible by
  scrolling).

  \sa itemAbove() itemRect() */

QListViewItem * QListViewItem::itemBelow()
{
    QListViewItem * c = 0;
    if ( isOpen() && childItem ) {
	c = childItem;
    } else if ( siblingItem ) {
	c = siblingItem;
    } else if ( parentItem ) {
	c = this;
	do {
	    c = c->parentItem;
	} while( c->parentItem && !c->siblingItem );
	if ( c )
	    c = c->siblingItem;
    }
    if ( c && !c->height() )
	return c->itemBelow();
    return c;
}


/*! \fn bool QListViewItem::isOpen () const

  Returns TRUE if this list view item has children \e and they are
  potentially visible, or FALSE if the item has no children or they
  are hidden.

  \sa setOpen()
*/

/*! \fn const QListViewItem* QListViewItem::firstChild () const

  Returns a pointer to the first (top) child of this item.

  NOTE that the children are not guaranteed to be sorted properly.
  QListView and QListViewItem try to postpone or avoid sorting to the
  greatest degree possible, in order to keep the user interface
  snappy.

  \sa nextSibling()
*/

/*! \fn const QListViewItem* QListViewItem::nextSibling () const

  Returns a pointer to the next sibling (below this one) of this
  item.

  NOTE that the siblings are not guaranteed to be sorted properly.
  QListView and QListViewItem try to postpone or avoid sorting to the
  greatest degree possible, in order to keep the user interface
  snappy.

  \sa fistChild()
*/

/*! \fn int QListViewItem::children () const

  Returns the current number of children of this item.
*/


/*! \fn int QListViewItem::height () const

  Returns the height of this item in pixels.  This does not include
  the height of any children; totalHeight() returns that.
*/

/*! \fn virtual int QListViewItem::compare (int column, const QListViewItem * with) const

  Defined to return less than 0, 0 or greater than 0 depending on
  whether this item is lexicograpically before, the same as, or after
  \a with when sorted by column \a column.
*/

/*! \fn void QListView::sizeChanged ()

  This signal is emitted when the list view changes width (or height?
  not at present).
*/


/*! \fn void QListView::selectionChanged()

  This signal is emitted whenever the set of selected items has
  changed (normally before the screen update).  It is available both
  in single-selection and multi-selection mode, but is most meaningful
  in multi-selection mode.

  \sa setSelected() QListViewItem::setSelected()
*/


/*! \fn void QListView::selectionChanged( QListViewItem * )

  This signal is emitted whenever the selected item has changed in
  single-selection mode (normally after the screen update).  The
  argument is the newly selected item.

  There is another signal which is more useful in multi-selection
  mode.

  \sa setSelected() QListViewItem::setSelected() currentChanged()
*/


/*! \fn void QListView::currentChanged( QListViewItem * )

  This signal is emitted whenever the current item has changed
  (normally after the screen update).  The current item is the item
  responsible for indicating keyboard focus.

  The argument is the newly current item.

  \sa setCurrentItem() currentItem()
*/


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mousePressEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    if ( e->button() != LeftButton )
	return;

    d->buttonDown = TRUE;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    if ( (i->isExpandable() || i->children()) &&
	 d->h->mapToLogical( d->h->cellAt( e->pos().x() ) == 0 ) ) {
	int x1 = e->pos().x() - d->h->cellPos( d->h->mapToActual( 0 ) );
	QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );
	while( it.current() && it.current()->i != i )
	    ++it;

	if ( it.current() ) {
	    x1 -= treeStepSize() * (it.current()->l - 2);
	    if ( x1 >= 0 && x1 < treeStepSize() )
		i->setOpen( !i->isOpen() );
	}
    }

    if ( i->isSelectable() )
	setSelected( i, isMultiSelection() ? !i->isSelected() : TRUE );

    setCurrentItem( i ); // repaints

    return;
}


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mouseReleaseEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    if ( e->button() == RightButton ) {
	QListViewItem * i;
	if ( viewport()->rect().contains( e->pos() ) )
	    i = itemAt( e->pos() );
	else
	    i = d->currentSelected;

	if ( i ) {
	    int c = d->h->mapToLogical( d->h->cellAt( e->pos().x() ) );
	    emit rightButtonClicked( i, viewport()->mapToGlobal( e->pos() ),
				     c );
	}
	return;
    }

    if ( e->button() != LeftButton || !d->buttonDown )
	return;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    if ( i->isSelectable() )
	setSelected( i, d->currentSelected
		     ? d->currentSelected->isSelected()
		     : TRUE );

    setCurrentItem( i ); // repaints

    return;
}


/*!  Processes mouse double-click events on behalf of the viewed
  widget; eventFilter() calls this function.  Note that the
  coordinates in \a e is in the coordinate system of viewport(). */

void QListView::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    // ensure that the next mouse moves and eventual release is
    // ignored.
    d->buttonDown = FALSE;

    QListViewItem * i = itemAt( e->pos() );
    if ( i )
	emit doubleClicked( i );
}


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mouseMoveEvent( QMouseEvent * e )
{
    if ( !e || !d->buttonDown )
	return;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    if ( i->isSelectable() )
	setSelected( i, d->currentSelected
		     ? d->currentSelected->isSelected()
		     : TRUE );

    setCurrentItem( i ); // repaints
    return;
}


/*!  Handles focus in events on behalf of viewport().  Since
  viewport() is this widget's focus proxy by default, you can think of
  this function as handling this widget's focus in events.

  \sa setFocusPolicy() setFocusProxy() focusOutEvent()
*/

void QListView::focusInEvent( QFocusEvent * )
{
    return;
}


/*!  Handles focus out events on behalf of viewport().  Since
  viewport() is this widget's focus proxy by default, you can think of
  this function as handling this widget's focus in events.

  \sa setFocusPolicy() setFocusProxy() focusInEvent()
*/

void QListView::focusOutEvent( QFocusEvent * )
{
    return;
}


/*!  Handles key press events on behalf of viewport().  Since
  viewport() is this widget's focus proxy by default, you can think of
  this function as handling this widget's keyboard input.
*/

void QListView::keyPressEvent( QKeyEvent * e )
{
    if ( !e )
	return; // subclass bug

    e->accept();
    if ( !currentItem() )
	return;

    QListViewItem * i = currentItem();

    if ( isMultiSelection() && i->isSelectable() && e->ascii() == ' ' ) {
	setSelected( i, !i->isSelected() );
	return;
    }

    QRect r( itemRect( i ) );
    QListViewItem * i2;

    switch( e->key() ) {
    case Key_Enter:
    case Key_Return:
	emit returnPressed( currentItem() );
	return;
    case Key_Down:
	i = i->itemBelow();
	break;
    case Key_Up:
	i = i->itemAbove();
	break;
    case Key_Next:
	i2 = itemAt( QPoint( 0, viewport()->height()-1 ) );
	if ( i2 == i || !r.isValid() ||
	     viewport()->height() <= itemRect( i ).bottom() ) {
	    i = i2;
	    int left = viewport()->height();
	    while( (i2 = i->itemBelow()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    i = i2;
	}
	break;
    case Key_Prior:
	i2 = itemAt( QPoint( 0, 0 ) );
	if ( i == i2 || !r.isValid() || r.top() <= 0 ) {
	    i = i2;
	    int left = viewport()->height();
	    while( (i2 = i->itemAbove()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    i = i2;
	}
	break;
    case Key_Right:
	if ( i->isOpen() && i->childItem ) {
	    i = i->childItem;
	} else if (  !i->isOpen() && (i->isExpandable() || i->children()) ) {
	    i->setOpen( TRUE );
	    triggerUpdate();
	}
	break;
    case Key_Left:
	if ( i->isOpen() && i->childItem ) {
	    i->setOpen( FALSE );
	    triggerUpdate();
	} else if ( i->parentItem && i->parentItem != d->r ) {
	    i = i->parentItem;
	}
	break;
    default:
	e->ignore();
	return;
    }

    if ( !i )
	return;

    int y = itemPos( i );
    if ( y < -contentsY() )
	verticalScrollBar()->setValue( y );
    else if ( y + i->height() > viewport()->height() - contentsY() )
	verticalScrollBar()->setValue( y+i->height() - viewport()->height() );

    if ( i->isSelectable() &&
	 ((e->state() & ShiftButton) || !isMultiSelection()) )
	setSelected( i, d->currentSelected
		     ? d->currentSelected->isSelected()
		     : TRUE  );

    setCurrentItem( i );
}


/*!  Returns a pointer to the QListViewItem at \a screenPos.  Note
  that \a screenPos is in the coordinate system of viewport(), not in
  the listview's own, much larger, coordinate system.

  itemAt() returns 0 if there is no such item.

  \sa itemPos() itemRect()
*/

QListViewItem * QListView::itemAt( QPoint screenPos ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();
    int g = screenPos.y() - contentsY();

    while( c && c->i && c->y + c->i->height() <= g )
	c = d->drawables->next();

    return (c && c->y <= g) ? c->i : 0;
}


/*!  Returns the y coordinate of \a item in the list view's
  coordinate system.  This functions is normally much slower than
  itemAt(), but it works for all items, while itemAt() normally works
  only for items on the screen.

  \sa itemAt() itemRect()
*/

int QListView::itemPos( QListViewItem * item )
{
    QListViewItem * i = item;
    QListViewItem * p;
    int a = 0;

    while( i && i->parentItem ) {
	p = i->parentItem;
	a += p->height();
	p = p->childItem;
	while( p && p != i ) {
	    a += p->totalHeight();
	    p = p->siblingItem;
	}
	i = i->parentItem;
    }
    return a;
}


/*!  Sets the list view to multi-selection mode if \a enable is TRUE,
  and to single-selection mode if \a enable is FALSE.

  \sa isMultiSelection()
*/

void QListView::setMultiSelection( bool enable )
{
    d->multi = enable ? TRUE : FALSE;
}


/*!  Returns TRUE if this list view is in multi-selection mode and
  FALSE if it is in single-selection mode.

  \sa setMultiSelection()
*/

bool QListView::isMultiSelection() const
{
    return d->multi;
}


/*!  Sets \a item to be selected if \a selected is TRUE, and to be not
  selected if \a selected is FALSE.

  If the list view is in single-selection mode and \a selected is
  TRUE, the present selected item is unselected.  Unlike
  QListViewItem::setSelected(), this function updates the list view as
  necessary and emits the selectionChanged() signals.

  \sa isSelected() setMultiSelection() isMultiSelection()
*/

void QListView::setSelected( QListViewItem * item, bool selected )
{
    if ( !item || item->isSelected() == selected )
	return;

    QRect r( 0,0, -1,-1 );

    if ( selected && !isMultiSelection() && d->currentSelected ) {
	d->currentSelected->setSelected( FALSE );
	r = r.unite( itemRect( d->currentSelected ) );
    }

    if ( item->isSelected() != selected ) {
	item->setSelected( selected );
	d->currentSelected = item;
	r = r.unite( itemRect( item ) );
    }

    if ( !d->allColumnsShowFocus ) {
	QRect col( d->h->cellPos(d->h->mapToActual(0)), r.top(),
		   d->h->cellSize(d->h->mapToActual(0)), r.height() );
	r = r.intersect( col );
    }

    viewport()->repaint( r, FALSE );
    if ( !isMultiSelection() )
	emit selectionChanged( item );
    emit selectionChanged();
}


/*!  Returns i->isSelected().

  Provided only because QListView provides setSelected() and I like
  completeness.
*/

bool QListView::isSelected( QListViewItem * i ) const
{
    return i ? i->isSelected() : FALSE;
}


/*!  Sets \a i to be the current highlighted item and repaints
  appropriately.  This highlighted item is used for keyboard
  navigation and focus indication; it doesn't mean anything else.

  \sa currentItem()
*/

void QListView::setCurrentItem( QListViewItem * i )
{
    QListViewItem * prev = d->focusItem;
    QRect r( 0, 0, -1, -1 );
    d->focusItem = i;
    if ( prev && prev != i )
	r = itemRect( prev );

    if ( i )
	r = r.unite( itemRect( i ) );

    if ( !d->allColumnsShowFocus ) {
	QRect col( d->h->cellPos(d->h->mapToActual(0)), r.top(),
		   d->h->cellSize(d->h->mapToActual(0)), r.height() );
	r = r.intersect( col );
    }

    viewport()->repaint( r, FALSE );
    if ( i != prev )
	emit currentChanged( i );

}


/*!  Returns a pointer to the currently highlighted item, or 0 if
  there isn't any.

  \sa setCurrentItem()
*/

QListViewItem * QListView::currentItem() const
{
    return d ? d->focusItem : 0;
}


/*!  Returns the rectangle on the screen \a i occupies in
  viewport()'s coordinates, or an invalid rectangle if \a i is a null
  pointer or is not currently visible.

  The rectangle returned does not include any children of the
  rectangle (ie. it uses QListViewItem::height() rather than
  QListViewItem::totalHeight()).  If you want the rectangle including
  children, you can use something like this code:

  \code
    QRect r( listView->itemRect( item ) );
    r.setHeight( (QCOORD)(QMIN( item->totalHeight(),
				listView->viewport->height() - r.y() ) ) )
  \endcode

  Note the way it avoids too-high rectangles.  totalHeight() can be
  much larger than the window system's coordinate system allows.

  itemRect() is comparatively slow.  It's best to call it only for
  items that are probably on-screen.
*/

QRect QListView::itemRect( QListViewItem * i ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != i )
	c = d->drawables->next();

    if ( c && c->i == i ) {
	int y = c->y + contentsY();
	if ( y + c->i->height() >= 0 &&
	     y < ((QListView *)this)->viewport()->height() ) {
	    QRect r( 0, y, d->h->width(), i->height() );
	    return r;
	}
    }

    return QRect( 0, 0, -1, -1 );
}


/*! \fn void QListView::doubleClicked( QListViewItem * )

  This signal is emitted whenever an item is double-clicked.  It's
  emitted on the second button press, not the second button release.
*/


/*! \fn void QListView::returnPressed( QListViewItem * )

  This signal is emitted when enter or return is pressed.  The
  argument is currentItem().
*/


/*!  Set the list view to be sorted by \a column and to be sorted
  in ascending order if \a ascending is TRUE or descending order if it
  is FALSE.
*/

void QListView::setSorting( int column, bool ascending )
{
    if ( d->column == column && d->ascending == ascending )
	return;

    d->ascending = ascending;
    d->column = column;
    triggerUpdate();
}


/*!  Changes the column the list view is sorted by. */

void QListView::changeSortColumn( int column )
{
    setSorting( column, d->ascending );
}


/*! \fn void QListView::rightButtonClicked( QListViewItem *, const QPoint&, int )

  This signal is emitted when the right button is clicked (ie. when
  it's released).  The arguments are the relevant QListViewItem (may
  be 0), the point in global coordinates and the relevant column.
*/


/*!  Reimplemented to let the list view items update themselves.  \a s
  is the new GUI style. */

void QListView::setStyle( GUIStyle s )
{
    d->h->setStyle( s );
    QScrollView::setStyle( s );
    reconfigureItems();
}


/*!  Reimplemented to let the list view items update themselves.  \a f
  is the new font. */

void QListView::setFont( const QFont & f )
{
    d->h->setFont( f );
    QScrollView::setFont( f );
    reconfigureItems();
}


/*!  Reimplemented to let the list view items update themselves.  \a p
  is the new palette. */

void QListView::setPalette( const QPalette & p )
{
    d->h->setPalette( p );
    QScrollView::setPalette( p );
    reconfigureItems();
}


/*!  Ensures that setup() are called for all currently visible items,
  and that it will be called for currently invisuble items as soon as
  their parents are opened.

  (A visible item, here, is an item whose parents are all open.  The
  item may happen to be offscreen.)

  \sa QListViewItem::setup()
*/

void QListView::reconfigureItems()
{
    d->fontMetricsHeight = fontMetrics().height();
    d->r->setOpen( FALSE );
    d->r->setOpen( TRUE );
}


/*!  Sets this list view to assume that the items show focus and
  selection state using all of their columns if \a enable is TRUE, or
  that they show it just using column 0 if \a enable is FALSE.

  The default is FALSE.

  Setting this to TRUE if it isn't necessary can cause noticeable
  flicker.

  \sa allColumnsShowFocus()
*/

void QListView::setAllColumnsShowFocus( bool enable )
{
    d->allColumnsShowFocus = enable;
}


/*!  Returns TRUE if the items in this list view indicate focus and
  selection state using all of their columns, else FALSE.

  \sa setAllColumnsShowFocus()
*/

bool QListView::allColumnsShowFocus() const
{
    return d->allColumnsShowFocus;
}


/*!  Returns the first item in this QListView.  You can use its \link
  QListViewItem::firstChild() firstChild() \endlink and \link
  QListViewItem::nextSibling() nextSibling() \endlink functions to
  traverse the entire tree of items.

  Returns 0 if there is no first item.

  \sa itemAt() itemBelow() itemAbove()
*/

const QListViewItem * QListView::firstChild() const
{
    return d->r->childItem;
}
