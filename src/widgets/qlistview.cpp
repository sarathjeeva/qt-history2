/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#119 $
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
#include "qapplication.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qdatetime.h"
#include "qptrdict.h"
#include "qvector.h"

#include <stdlib.h> // qsort
#include <ctype.h> // tolower

const int Unsorted = 16383;


struct QListViewPrivate
{
    // classes that are here to avoid polluting the global name space

    // the magical hidden mother of all items
    struct Root: public QListViewItem {
	Root( QListView * parent );

	void setHeight( int );
	void invalidateHeight();
	void setup();
	QListView * theListView() const;

	QListView * lv;
    };

    // for the stack used in drawContentsOffset()
    struct Pending {
	Pending( int level, int ypos, QListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l; // level of this item; root is -1 or 0
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

    struct ItemColumnInfo {
	ItemColumnInfo(): pm( 0 ), next( 0 ) {}
	~ItemColumnInfo() { delete pm; delete next; }
	QString text;
	QPixmap * pm;
	ItemColumnInfo * next;
    };

    struct ViewColumnInfo {
	ViewColumnInfo(): align(AlignLeft), sortable(TRUE), next( 0 ) {}
	~ViewColumnInfo() { delete next; }
	int align;
	bool sortable;
	ViewColumnInfo * next;
    };

    // private variables used in QListView
    ViewColumnInfo * vci;
    QHeader * h;
    Root * r;
    uint rootIsExpandable : 1;
    int margin;

    QListViewItem * currentSelected;
    QListViewItem * focusItem;

    QTimer * timer;
    QTimer * dirtyItemTimer;
    int levelWidth;

    // the list of drawables, and the range drawables covers entirely
    // (it may also include a few items above topPixel)
    QList<DrawableItem> * drawables;
    int topPixel;
    int bottomPixel;

    QPtrDict<void> * dirtyItems;

    bool multi;

    // TRUE if the widget should take notice of mouseReleaseEvent
    bool buttonDown;
    // TRUE if the widget should ignore a double-click
    bool ignoreDoubleClick;

    // Per-column structure for information not in the QHeader
    struct Column {
	QListView::WidthMode wmode;
    };
    QVector<Column> column;

    // sort column and order   #### may need to move to QHeader [subclass]
    int sortcolumn;
    bool ascending;

    // suggested height for the items
    int fontMetricsHeight;
    bool allColumnsShowFocus;

    // currently typed prefix for the keyboard interface, and the time
    // of the last key-press
    QString currentPrefix;
    QTime currentPrefixTime;

    // whether to select or deselect during this mouse press.
    bool select;
};



/*!
  \class QListViewItem qlistview.h
  \brief The QListViewItem class implements a list view item.

  A list viev item is a multi-column object capable of displaying
  itself.  Its design has the following main goals: <ul> <li> Work
  quickly and well for \e large sets of data. <li> Have a low usage
  threshold for simple use. </ul>

  The simplest way to use QListViewItem is to construct one with a few
  constant strings.  This creates an item which is a child of \e
  parent, with two fixed-content strings, and discards the pointer to
  it:

  \code
     (void) new QListViewItem( parent, "first column", "second column" );
  \endcode

  This object will be deleted when \e parent is deleted, as for \link
  QObject QObjects. \endlink

  The parent is either another QListViewItem or a QListView.  If the
  parent is a QListView, this item is a top-level item within than
  QListView.  If the parent is another QListViewItem, this item
  becomes a child of the parent item.

  If you keep the pointer, you can set or change the texts using
  setText(), add pixmaps using setPixmap(), change its mode using
  setSelectable(), setSelected(), setOpen() and setExpandable(),
  change its height using setHeight(), and do much tree traversal.

  You can traverse the tree as if it were a doubly linked list using
  itemAbove() and itemBelow(); they return pointers to the items
  directly above and below this item on the screen (even if none of
  the three are actually visible at the moment).

  You can also traverse it as a tree, using parent(), firstChild() and
  nextSibling().  This code does something to each of an item's
  children:

  \code
    QListViewItem * myChild = myItem->firstChild();
    while( myChild ) {
        doSomething( myChild );
	myChild = myChild->nextSibling();
    }
  \endcode

  Note that the order of the children will change when the sorting
  order changes, and is undefined if the items are not visible.  You
  can however call enforceSortOrder() at any time, and QListView will
  always call it before it needs to show an item.

  Many programs will need to reimplement QListViewItem.  The most
  commonly reimplemented functions are: <ul> <li> text() returns the
  text in a column.  Many subclasses will compute that on the
  fly. <li> key() is used for sorting. <li> setup() is called before
  showing the item, and whenever e.g. the font changes. <li>
  activate() is called whenever the user clicks on the item or presses
  space when the item is the currently highlighted item.</ul>

  Some subclasses call setExpandable( TRUE ) even when they have no
  children, and populate themselves when setup() is called.  The
  dirview/dirview.cpp example program uses precisely this technique to
  start up quickly: The files and subdirectories in a directory aren't
  entered into the tree until they need to.

  This example shows a number of root items in a QListView.  These
  items are actually subclassed from QListViewItem: The file size,
  type etc. are computed on the fly.

  <img src="listview.gif" width="518" height="82" alt="Example List View">

  The next example shows a fraction of the dirview example.  Again,
  the Direcotory/Symbolic Link column is computed on the fly.  None of
  the items are root items; the \e usr item is a child of the root and
  the \e X11 item is a child of the \e usr item.

  <img src="treeview.gif" width="256" height="216" alt="Example Tree View">
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



/*!  Creates a new list view item in the QListView \a parent,
  \a parent, with at most 8 constant strings as contents.

  \code
     (void)new QListViewItem( lv, "/", "Root directory" );
  \endcode
*/

QListViewItem::QListViewItem( QListView * parent,
			      const char * label1,
			      const char * label2,
			      const char * label3,
			      const char * label4,
			      const char * label5,
			      const char * label6,
			      const char * label7,
			      const char * label8 )
{
    init();
    parent->insertItem( this );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}


/*!  Creates a new list view item that's a child of the QListViewItem
  \a parent, with at most 8 constant strings as contents.  Possible
  example in a threaded news or e-mail reader:

  \code
     (void)new QListViewItem( parentMessage, author, subject );
  \endcode
*/

QListViewItem::QListViewItem( QListViewItem * parent,
			      const char * label1,
			      const char * label2,
			      const char * label3,
			      const char * label4,
			      const char * label5,
			      const char * label6,
			      const char * label7,
			      const char * label8 )
{
    init();
    parent->insertItem( this );

    setText( 0, label1 );
    setText( 1, label2 );
    setText( 2, label3 );
    setText( 3, label4 );
    setText( 4, label5 );
    setText( 5, label6 );
    setText( 6, label7 );
    setText( 7, label8 );
}

/*!  Performs the initializations that's common to the constructors. */

void QListViewItem::init()
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = FALSE;

    nChildren = 0;
    parentItem = 0;
    siblingItem = childItem = 0;

    columns = 0;

    selected = 0;

    lsc = Unsorted;
    lso = TRUE; // unsorted in ascending order :)
    configured = FALSE;
    expandable = FALSE;
    selectable = TRUE;
    is_root = FALSE;
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
    delete (QListViewPrivate::ItemColumnInfo *)columns;
}


/*!  Inserts \a newChild into its list of children.  Called by the
  constructor of \a newChild.
*/

void QListViewItem::insertItem( QListViewItem * newChild )
{
    invalidateHeight();
    newChild->siblingItem = childItem;
    childItem = newChild;
    nChildren++;
    newChild->parentItem = this;
    lsc = Unsorted;
    newChild->ownHeight = 0;
    newChild->configured = FALSE;
}


/*!  Removes \a tbg from this object's list of children.  This is
  normally called by tbg's destructor. */

void QListViewItem::removeItem( QListViewItem * tbg )
{
    invalidateHeight();

    QListView * lv = listView();

    if ( lv && lv->d->currentSelected ) {
	QListViewItem * c = lv->d->currentSelected;
	while( c && c != tbg )
	    c = c->parentItem;
	if ( c == tbg )
	    lv->d->currentSelected = 0;
    }

    if ( lv && lv->d->focusItem ) {
	const QListViewItem * c = lv->d->focusItem;
	while( c && c != tbg )
	    c = c->parentItem;
	if ( c == tbg )
	    lv->d->focusItem = 0;
    }

    nChildren--;

    QListViewItem ** nextChild = &childItem;
    while( nextChild && *nextChild && tbg != *nextChild )
	nextChild = &((*nextChild)->siblingItem);

    if ( nextChild && tbg == *nextChild )
	*nextChild = (*nextChild)->siblingItem;
    tbg->parentItem = 0;
}


/*!
  \fn const char * QListViewItem::key( int column, bool ascending ) const

  Returns a key that can be used for sorting by column \a column.
  The default implementation returns text().  Derived classes may
  also incorporate the order indicated by \a ascending into this
  key, although this is not recommended.

  QListViewItem immediately copies the return value of this function,
  so it's safe to return a pointer to a static variable.

  \sa sortChildItems()
*/

const char * QListViewItem::key( int column, bool ) const
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


/*!  Sorts the children of this item by the return values of
  key(\a column, \a ascending), in ascending order if \a ascending
  is TRUE and in descending order of \a descending is FALSE.

  Asks some of the children to sort their children.  (QListView and
  QListViewItem ensure that all on-screen objects are properly sorted,
  but may avoid or defer sorting other objects in order to be more
  responsive.)

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
	= new QListViewPrivate::SortableItem[nChildren];
    QListViewItem * s = childItem;
    int i = 0;
    while ( s && i<nChildren ) {
	siblings[i].key = s->key( column, ascending );
	siblings[i].i = s;
	s = s->siblingItem;
	i++;
    }

    // and do it.
    qsort( siblings, nChildren,
	   sizeof( QListViewPrivate::SortableItem ), cmp );

    // build the linked list of siblings, in the appropriate
    // direction, and finally set this->childItem to the new top
    // child.
    if ( ascending ) {
	for( i=0; i < nChildren-1; i++ )
	    siblings[i].i->siblingItem = siblings[i+1].i;
	siblings[nChildren-1].i->siblingItem = 0;
	childItem = siblings[0].i;
    } else {
	for( i=nChildren-1; i >0; i-- )
	    siblings[i].i->siblingItem = siblings[i-1].i;
	siblings[0].i->siblingItem = 0;
	childItem = siblings[nChildren-1].i;
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

    if ( !nChildren )
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
  object, and whenever the font, GUI style or colors of the list view
  change.

  The default calls widthChanged() and sets the item's height.
*/

void QListViewItem::setup()
{
    widthChanged();
    setHeight( listView()->d->fontMetricsHeight );
}

/*!
  This virtual function is called whenever the user clicks on this
  item or presses Space on it. The default implementation does
  nothing.
*/

void QListViewItem::activate()
{
}

/*! \fn bool QListViewItem::isSelectable() const

  Returns TRUE if the item is selectable (as it is by default) and
  FALSE if it isn't.

  \sa setSelectable()
*/


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

  Returns TRUE if this item is expandable even when it has no
  children.
*/

/*!  Sets this item to be expandable even if it has no children if \a
  enable is TRUE, and to be expandable only if it has children if \a
  enable is FALSE (the default).

  The dirview example uses this in the canonical fashion: It checks
  whether the directory is empty in setup() and calls
  setExpandable(TRUE) if not, and in setOpen() it reads the contents
  of the directory and inserts items accordingly.  This strategy means
  that dirview can display the entire file system without reading very
  much at start-up.

  \sa setSelectable()
*/

void QListViewItem::setExpandable( bool enable )
{
    expandable = enable;
}


/*!  Makes sure that this object's children are sorted appropriately.

  This only works if every item in the chain from the root item to
  this item is sorted appropriately.

  \sa sortChildItems()
*/


void QListViewItem::enforceSortOrder() const
{
    if( parentItem &&
	(parentItem->lsc != lsc || parentItem->lso != lso) &&
	(int)parentItem->lsc != Unsorted )
	((QListViewItem *)this)->sortChildItems( (int)parentItem->lsc,
						 (bool)parentItem->lso );
}


/*! \fn bool QListViewItem::isSelected() const

  Returns TRUE if this item is selected, or FALSE if it is not.

  \sa setSelection() selectionChanged() QListViewItem::setSelected()
*/


/*!  Sets this item to be selected \a s is TRUE, and to not be
  selected if \a o is FALSE.

  This function does not maintain any invariants or repaint anything -
  QListView::setSelected() does that.

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

  \sa height()
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

    if ( !that->isOpen() || !that->childCount() )
	return that->ownHeight;

    QListViewItem * child = that->childItem;
    while ( child != 0 ) {
	that->maybeTotalHeight += child->totalHeight();
	child = child->siblingItem;
    }
    return that->maybeTotalHeight;
}


/*!  Returns the text in column \a column, or else 0.

  The returned pointer must be copied or used at once so that
  reimplementations of this function are at liberty to e.g. return a
  pointer into a static buffer.

  \sa key() paintCell()
*/

const char * QListViewItem::text( int column ) const
{
    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;

    while( column && l ) {
	l = l->next;
	column--;
    }

    return l ? (const char *)(l->text) : 0;
}


/*!  Sets the text in column \a column to \a text, if \a column is a
  valid column number and \a text is non-null.

  If \a text() has been reimplemented, this function may be a no-op.

  \sa text() key()
*/

void QListViewItem::setText( int column, const char * text )
{
    if ( column < 0 )
	return;

    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;
    if ( !l ) {
	l = new QListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }
    while( column ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ItemColumnInfo;
	l = l->next;
	column--;
    }
    l->text = text;
    repaint();
}


/*!  Sets the pixmap in column \a column to \a pm, if \a pm is
  non-null and \a column is non-negative.

  \sa pixmap() setText()
*/

void QListViewItem::setPixmap( int column, const QPixmap & pm )
{
    if ( column < 0 )
	return;

    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;
    if ( !l ) {
	l = new QListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }
    while( column ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ItemColumnInfo;
	l = l->next;
	column--;
    }
    if ( l->pm )
	*(l->pm) = pm;
    else
	l->pm = new QPixmap( pm );
    repaint();
}


/*!  Returns a pointer to the pixmap for \a column, or a null pointer
  if there is no pixmap for \a column.

  \sa setText() setPixmap()
*/

const QPixmap * QListViewItem::pixmap( int column ) const
{
    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;

    while( column && l ) {
	l = l->next;
	column--;
    }

    return (l && l->pm) ? l->pm : 0;
}


/*!  This virtual function paints the contents of one column of one item.

  \a p is a QPainter open on the relevant paint device.  \a pa is
  translated so 0, 0 is the top left pixel in the cell and \a width-1,
  height()-1 is the bottom right pixel \e in the cell.  The other
  properties of \a p (pen, brush etc) are undefined.  \a cg is the
  color group to use.  \a column is the logical column number within
  the item that is to be painted; 0 is the column which may contain a
  tree.

  This function may use QListView::itemMargin() for readability
  spacing on the left and right sides of information such as text,
  and should honour isSelected() and QListView::allColumnsShowFocus().

  If you reimplement this function, you should also reimplement
  width().

  The rectangle to be painted is in an undefined state when this
  function is called, so you \e must draw on all the pixels.

  \sa paintBranches(), QListView::drawContentsOffset()
*/

void QListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, int align )
{
    // Change width() if you change this.

    if ( !p )
	return;

    QListView *lv = listView();
    int r = lv ? lv->itemMargin() : 2;
    const QPixmap * icon = pixmap( column );

    p->fillRect( 0, 0, width, height(), cg.base() );

    int marg = lv ? lv->itemMargin() : 2;

    if ( isSelected() &&
	 (column==0 || listView()->allColumnsShowFocus()) ) {
	if ( listView()->style() == WindowsStyle ) {
	    p->fillRect( r - marg, 0, width - r + marg, height(),
			 QApplication::winStyleHighlightColor() );
	    p->setPen( white ); // ###
	} else {
	    p->fillRect( r - marg, 0, width - r + marg,
			 height(), cg.text() );
	    p->setPen( cg.base() );
	}
    } else {
	p->setPen( cg.text() );
    }

    if ( icon && !column ) {
	p->drawPixmap( r, (height()-icon->height())/2, *icon );
	r += icon->width() + listView()->itemMargin();
    }

    const char * t = text( column );
    if ( t ) {
	// should do the ellipsis thing in drawText()
	p->drawText( r, 0, width-marg-r, height(),
		     align | AlignVCenter, t );
    }
}

/*!
  Returns the number of pixels of width required to draw column \a c
  of listview \a lv, using the metrics \a fm without cropping.
  The list view containing this item may use
  this information, depending on the QListView::WidthMode settings
  for the column.

  The default implementation returns the width of the bounding
  rectangle of the text of column \a c.

  \sa listView() widthChanged() QListView::setColumnWidthMode()
  QListView::itemMargin()
*/
int QListViewItem::width(const QFontMetrics& fm,
			 const QListView* lv, int c) const
{
    int w = -(fm.minLeftBearing()+fm.minRightBearing()) +
	    fm.width(text(c)) + lv->itemMargin() + 2;
    const QPixmap * pm = pixmap( c );
    if ( pm )
	w += pm->width() + lv->itemMargin(); // ### correct margin stuff?
    return w;
}


/*!  \fn void QListViewItem::paintFocus( QPainter *p, const QColorGroup & cg, const QRect & r )

  Paints a focus indication on the rectangle \a r using painter \a p
  and colors \a cg.

  \a p is already clipped.

  \sa paintCell() paintBranches() QListView::setAllColumnsShowFocus()
*/

void QListViewItem::paintFocus( QPainter *p, const QColorGroup &,
				const QRect & r )
{
    if ( listView()->style() == WindowsStyle ) {
	p->drawWinFocusRect( r );
    } else {
	p->setPen( black );
	p->drawRect( r );
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
				   int w, int y, int h, GUIStyle s )
{
    p->fillRect( 0, 0, w, h, cg.base() );

    QListViewItem * child = firstChild();
    int linetop = 0, linebot = 0;

    int dotoffset = y & 1;

    // each branch needs at most two lines, ie. four end points
    QPointArray dotlines( childCount() * 4 );
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
	if ( child->expandable || child->childCount() ) {
	    if ( s == WindowsStyle ) {
		// needs a box
		p->setPen( cg.dark() );
		p->drawRect( bx-4, linebot-4, 9, 9 );
		// plus or minus
		p->setPen( cg.foreground() ); // ### windows uses black
		p->drawLine( bx - 2, linebot, bx + 2, linebot );
		if ( !child->isOpen() )
		    p->drawLine( bx, linebot - 2, bx, linebot + 2 );
		// dotlinery
		dotlines[c++] = QPoint( bx, linetop );
		dotlines[c++] = QPoint( bx, linebot - 5 );
		dotlines[c++] = QPoint( bx + 6, linebot );
		dotlines[c++] = QPoint( w, linebot );
		linebot += 6;
		linetop = linebot;
	    } else {
		int x = bx - 4;
		int y = linebot - 4;
		int d = 9;
		QPointArray a;
		if ( child->isOpen() ) {
		    // DownArrow
		    a.setPoints( 3, x, y, x+d, y, x+d/2, y+d );
		} else {
		    //RightArrow
		    a.setPoints( 3, x, y, x, y+d, x+d, y+d/2 );
		}
		p->setPen( cg.foreground() );
		p->drawPolygon( a );
	    }
	} else if ( s == WindowsStyle ) {
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

    if ( s == WindowsStyle ) {
	// this could be done much faster on X11, but not on Windows.
	// oh well.  do it the hard way.

	// thought: keep around a 64*1 and a 1*64 bitmap such that
	// drawPixmap'ing them is equivalent to drawing a horizontal
	// or vertical line with the appropriate pen.
	
	QPointArray dots( (h+4)/2 + (childCount()*w+3)/4 );
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


QListView * QListViewPrivate::Root::theListView() const
{
    return lv;
}


void QListViewPrivate::Root::setup()
{
    // explicitly nothing
}


/*!
  \class QListView qlistview.h
  \brief The QListView class implements a list/tree view.
  \ingroup realwidgets

  It can display and control a hierarchy of multi-column items, and
  provides the ability to add new items at run-time, let the user
  select one or many items, sort the list in increasing or decreasing
  order by any column, and so on.

  The simplest mode of usage is to create a QListView, add some column
  headers using setColumn(), create one or more QListViewItem objects
  with the QListView as parent, set up the list view's geometry(), and
  show() it.

  The main setup functions are <ul>

  <li>addColumn() - adds a column, with text and perhaps width.

  <li>setColumnWidthMode() - sets the column to be resized
  automatically or not.

  <li>setMultiSelection() - decides whether one can select one or many
  objects in this list view.  The default is FALSE (selecting one item
  unselects any other selected item).

  <li>setAllColumnsShowFocus() - decides whether items should show
  keyboard focus using all columns, or just column 0.  The default is
  to show focus using just column 0.

  <li>setRootIsDecorated() - decides whether root items can be opened
  and closed by the user, and have open/close decoration to their left.
  The default is FALSE.

  <li>setTreeStepSize() - decides the how many pixels an item's
  children are indented relative to their parent.  The default is 20.
  This is mostly a matter of taste.

  <li>setSorting() - decides whether the items should be sorted in
  ascending or descending order, and by what column.</ul>

  There are also several functions for mapping between items and
  coordinates.  itemAt() returns the item at a position on-screen,
  itemRect() returns the rectangle an item occupies on the screen and
  itemPos() returns the position of any item (not on-screen, in the
  list view).  firstChild() returns the item at the top of the view
  (not necessarily on-screen) so you can iterate over the items using
  either QListViewItem::itemBelow() or a combination of
  QListViewItem::firstChild() and QListViewItem::nextSibling().

  Naturally, QListView provides a clear() function, as well as an
  explicit insertItem() for when QListViewItem's default insertion
  won't do.

  Since QListView offers multiple selection it has to display keyboard
  focus and selection state separately.  Therefore there are functions
  both to set the selection state of an item, setSelected(), and to
  select which item displays keyboard focus, setCurrentItem().

  QListView emits two groups of signals: One group signals changes in
  selection/focus state and one signals selection.  The first group
  consists of selectionChanged(), applicable to all list views, and
  selectionChanged( QListViewItem * ), applicable only to
  single-selection list view, and currentChanged( QListViewItem * ).
  The second group consists of doubleClicked( QListViewItem * ),
  returnPressed( QListViewItem * ) and rightButtonClicked(
  QListViewItem *, const QPoint&, int ).

  In Motif style, QListView deviates fairly strongly from the look and
  feel of the Motif hierarchical tree view.  This is done mostly to
  provide a usable keyboard interface and to make the list view look
  better with a white background.

  <img src="listview.gif" width="518" height="82" alt="Example List View">
  <br clear="all">
  Windows style, flat (from QFileDialog)

  <img src="treeview.gif" width="256" height="216" alt="Example Tree View">
  <br clear="all">
  Motif style, hierarchial (from the dirview/dirview.cpp example)

  \internal

  need to say stuff about the mouse and keyboard interface.
*/

/*!  Creates a new empty list view, with \a parent as a parent and \a
  name as object name. */

QListView::QListView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    d = new QListViewPrivate;
    d->vci = 0;
    d->timer = new QTimer( this );
    d->levelWidth = 20;
    d->r = 0;
    d->rootIsExpandable = 0;
    d->h = new QHeader( this, "list view header" );
    d->h->installEventFilter( this );
    d->currentSelected = 0;
    d->focusItem = 0;
    d->drawables = 0;
    d->dirtyItems = 0;
    d->dirtyItemTimer = new QTimer( this );
    d->margin = 2;
    d->multi = 0;
    d->sortcolumn = 0;
    d->ascending = TRUE;
    d->allColumnsShowFocus = FALSE;
    d->fontMetricsHeight = fontMetrics().height();
    d->h->setTracking(TRUE);
    d->buttonDown = FALSE;
    d->ignoreDoubleClick = FALSE;

    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(updateContents()) );
    connect( d->dirtyItemTimer, SIGNAL(timeout()),
	     this, SLOT(updateDirtyItems()) );

    connect( d->h, SIGNAL(sizeChange( int, int, int )),
	     this, SLOT(handleSizeChange( int, int, int )) );
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
    r->is_root = TRUE;
    d->r = r;
    d->r->setSelectable( FALSE );

    viewport()->setFocusProxy( this );
    setFocusPolicy( TabFocus );
}


/*!  Deletes the list view and all items in it, and frees all
  allocated resources.  */

QListView::~QListView()
{
    d->focusItem = 0;
    d->currentSelected = 0;
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

    if ( d->dirtyItems ) {
	// make a new clip region, including the dirty items
	QRect br( cx - ox, cy - oy, cw, ch );
	QRegion r( br );
	QPtrDictIterator<void> it( *(d->dirtyItems) );
	QListViewItem * i;
	while( (i=(QListViewItem *)(it.currentKey())) != 0 ) {
	    ++it;
	    QRect ir( itemRect( i ) );
	    if ( !ir.isEmpty() ) {
		br = br.unite( ir );
		r = r.unite( QRegion( ir ) );
	    }
	}
	p->setClipRegion( r );
	// and change the "arguments".  naughty.
	cx = br.left() + ox;
	cy = br.top() + oy;
	cw = br.width();
	ch = br.height();
	delete d->dirtyItems;
	d->dirtyItems = 0;
	d->dirtyItemTimer->stop();
    }

    p->setFont( font() );

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
		if ( !d->focusItem && hasFocus() )
		    d->focusItem = current->i;
	    }

	    x = fx;
	    c = fc;

            // draw to last interesting column
            while( c < lc ) {
		int i = d->h->mapToLogical( c );
                cs = d->h->cellSize( c );
                r.setRect( x - ox, current->y - oy, cs, ih );
		if ( i==0 && current->i->parentItem )
		    r.setLeft( r.left() + current->l * treeStepSize() );

		p->save();
                p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
                p->translate( r.left(), r.top() );
		int ac = d->h->mapToLogical( c );
		current->i->paintCell( p, colorGroup(), ac, r.width(),
				       columnAlignment( ac ) );
		p->restore();
		x += cs;
		c++;
	    }
	}

	if ( tx < 0 )
	    tx = d->h->cellPos( d->h->mapToActual( 0 ) );
	
	// do any children of current need to be painted?
	if ( ih != ith &&
	     (current->i != d->r || d->rootIsExpandable) &&
	     current->y + ith > cy &&
	     current->y + ih < cy + ch &&
	     tx + current->l * treeStepSize() < cx + cw &&
	     tx + (current->l+1) * treeStepSize() > cx ) {
	    // compute the clip rectangle the safe way

	    int rtop = current->y + ih;
	    int rbottom = current->y + ith;
	    int rleft = tx + current->l*treeStepSize();
	    int rright = rleft + treeStepSize();
	
	    int crtop = QMAX( rtop, cy );
	    int crbottom = QMIN( rbottom, cy+ch );
	    int crleft = QMAX( rleft, cx );
	    int crright = QMIN( rright, cx+cw );

	    r.setRect( crleft-ox, crtop-oy,
		       crright-crleft, crbottom-crtop );

	    if ( r.isValid() ) {
		p->save();
		p->setClipRect( r );
		p->translate( rleft-ox, crtop-oy );
		current->i->paintBranches( p, colorGroup(), treeStepSize(),
					   rtop - crtop, r.height(), style() );
		p->restore();
	    }
	}
	
	// does current need focus indication?
	if ( current->i == d->focusItem && hasFocus() ) {
	    p->save();
	    if ( d->allColumnsShowFocus ) {
		int x1 = ox > 0 ? -1 : 0;
		int x2 = d->h->width() - ox;
		int w = QMIN( viewport()->width(), x2-x1+1 );
		r.setRect( x1, current->y - oy, w, ih );
                p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
		current->i->paintFocus( p, colorGroup(), r );
	    } else {
		r.setRect( d->h->cellPos( d->h->mapToActual( 0 ) ) - ox,
			   current->y - oy,
			   d->h->cellSize( d->h->mapToActual( 0 ) ),
			   ih );
                p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
		current->i->paintFocus( p, colorGroup(), r );
	    }
	    p->restore();
	}
	
    }

    if ( d->r->totalHeight() < cy + ch )
	paintEmptyArea( p, QRect( cx - ox, d->r->totalHeight() - oy,
				  cw, cy + ch - d->r->totalHeight() ) );

    int c = d->h->count()-1;
    if ( c >= 0 &&
	 d->h->cellPos( c ) + d->h->cellSize( c ) < cx + cw ) {
	c = d->h->cellPos( c ) + d->h->cellSize( c );
	paintEmptyArea( p, QRect( c - ox, cy - oy, cx + cw - c, ch ) );
    }
}



/*!  Paints \a rect so that it looks like empty background using
  painter p.  \a rect is is widget coordinates, ready to be fed to \a
  p.

  The default function fills \a rect with colorGroup().base().
*/

void QListView::paintEmptyArea( QPainter * p, const QRect & rect )
{
    p->fillRect( rect, colorGroup().base() );
}


/*! Rebuilds the lis of drawable QListViewItems.  This function is
  const so that const functions can call it without requiring
  d->drawables to be mutable */

void QListView::buildDrawableList() const
{
    if ( ( (int)d->r->lsc != d->sortcolumn ||
	   (bool)d->r->lso != d->ascending ) &&
	 d->sortcolumn != Unsorted )
	d->r->sortChildItems( d->sortcolumn, d->ascending );

    QStack<QListViewPrivate::Pending> stack;
    stack.push( new QListViewPrivate::Pending( ((int)d->rootIsExpandable)-1,
					       0, d->r ) );

    // could mess with cy and ch in order to speed up vertical
    // scrolling
    int cy = contentsY();
    int ch = ((QListView *)this)->viewport()->height();
    // ### hack to help sizeHint().  if not visible, assume that we'll
    // ### use 200 pixels rather than whatever QSrollView.  this lets
    // ### sizeHint() base its width on a more realistic number of
    // ### items.
    if ( !isVisible() && ch < 200 )
	ch = 200;
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
  This number has meaning only for tree views.  The default is 20.

  \sa setTreeStepSize()
*/

int QListView::treeStepSize() const
{
    return d->levelWidth;
}


/*!  Sets the the number of pixels a child is offset from its parent,
  in a tree view to \a l.  The default is 20.

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
  generally do not need to call this; the QListViewItem constructor
  does it for you.
*/

void QListView::insertItem( QListViewItem * i )
{
    if ( d->r ) // not for d->r itself
	d->r->insertItem( i );
}


/*!  Remove and delete all the items in this list view, and trigger an
  update. \sa triggerUpdate() */

void QListView::clear()
{
    if ( d->drawables )
	d->drawables->clear();
    delete d->dirtyItems;
    d->dirtyItems = 0;
    d->dirtyItemTimer->stop();

    setSelected( d->currentSelected, FALSE );
    d->focusItem = 0;

    // if it's down its downness makes no sense, so undown it
    d->buttonDown = FALSE;

    QListViewItem *c = (QListViewItem *)d->r->firstChild();
    QListViewItem *n;
    while( c ) {
	n = (QListViewItem *)c->nextSibling();
	delete c;
	c = n;
    }
    resizeContents( d->h->sizeHint().width(), 0 );
    triggerUpdate();
}


/*!
  Adds a new column at the right end of the widget, with the header \a
  label, and returns the index of the column.

  If \a width is negative, the new column will have WidthMode Maximum,
  otherwise it will be Manual at \a width pixels wide.

  \sa setColumnText() setColumnWidth() setColumnWidthMode()
*/
int QListView::addColumn( const char * label, int width )
{
    int c = d->h->addLabel( label, width );
    d->column.resize( c+1 );
    d->column.insert( c, new QListViewPrivate::Column );
    d->column[c]->wmode = width >=0 ? Manual : Maximum;
    return c;
}

/*!
  Sets the heading text of column \a column to \a label.  The leftmost
  colum is number 0.
*/
void QListView::setColumnText( int column, const char * label )
{
    ASSERT( column < d->h->count() );
    d->h->setLabel( column, label );
}

/*!
  Sets the width of column \a column to \a w pixels.  Note that if the
  column has a WidthMode other than Manual, this width setting may be
  subsequently overridden.  The leftmost colum is number 0.
*/
void QListView::setColumnWidth( int column, int w )
{
    ASSERT( column < d->h->count() );
    if ( d->h->cellSize( column ) != w ) {
	d->h->setCellSize( column, w );
	d->h->update(); // ##### paul, QHeader::setCellSize should do this.
    }
}

/*!
  Returns the text for the heading of column \a c.
*/
const char* QListView::columnText( int c ) const
{
    return d->h->label(c);
}

/*!
  Returns the width of the heading of column \a c.
*/
int QListView::columnWidth( int c ) const
{
    return d->h->cellSize(c);
}

/*!
  Sets column \c to behave according to \a mode, which is one of:

\define QListView::WidthMode

  <ul>
   <li> \c Manual - the column width does not change automatically
   <li> \c Maximum - the column is automatically sized according to the
	    widths of all items in the column.
   </ul>

   \bug doesn't shrink back yet when items shrink or close

   </ul>

  \sa QListViewItem::width()
*/
void QListView::setColumnWidthMode( int c, WidthMode mode )
{
    d->column[c]->wmode = mode;
}

/*!
  Returns the currently set WidthMode for column \a c.
  \sa setColumnWidthMode()
*/
QListView::WidthMode QListView::columnWidthMode( int c ) const
{
    return d->column[c]->wmode;
}


/*!

*/

void QListView::setColumnAlignment( int column, int align )
{
    if ( column < 0 )
	return;
    if ( !d->vci )
	d->vci = new QListViewPrivate::ViewColumnInfo;
    QListViewPrivate::ViewColumnInfo * l = d->vci;
    while( column ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ViewColumnInfo;
	l = l->next;
	column--;
    }
    if ( l->align == align )
	return;
    l->align = align;
    triggerUpdate();
}


/*!

*/

int QListView::columnAlignment( int column ) const
{
    if ( column < 0 || !d->vci )
	return AlignLeft;
    QListViewPrivate::ViewColumnInfo * l = d->vci;
    while( column ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ViewColumnInfo;
	l = l->next;
	column--;
    }
    return l ? l->align : AlignLeft;
}



/*!  Reimplemented to set the correct background mode and viewed area
  size. */

void QListView::show()
{
    if ( !isVisible() ) {
	QWidget * v = viewport();
	if ( v )
	    v->setBackgroundMode( NoBackground );

	reconfigureItems();
	updateGeometries();
    }
    QScrollView::show();
}


/*!  Updates the sizes of the viewport, header, scrollbars and so on.
  Don't call this directly; call triggerUpdates() instead.
*/

void QListView::updateContents()
{
    viewport()->update();
    updateGeometries();
}

void QListView::updateGeometries()
{
    QSize hs( d->h->sizeHint() );
    resizeContents( hs.width(), d->r->totalHeight() );
    if ( d->h->testWFlags( WState_DoHide ) ) {
	setMargins( 0, 0, 0, 0 );
    } else {
	setMargins( 0, hs.height(), 0, 0 );
	d->h->setGeometry( viewport()->x(), viewport()->y()-hs.height(),
			   viewport()->width(), hs.height() );
    }
}

/*!
  Updates the display when a section has changed size.
*/
void QListView::handleSizeChange( int section, int, int )
{
    updateGeometries();
    int left = d->h->cellPos(d->h->mapToActual( section ));
    viewport()->repaint( left, 0, viewport()->width()-left,
                         viewport()->height(), FALSE );
}


/*!  Very smart internal slot that'll repaint JUST the items that need
  to be repainted.  Don't use this directly; call repaintItem() and
  this slot gets called by a null timer.
*/

void QListView::updateDirtyItems()
{
    if ( d->timer->isActive() )
	return;
    if ( d->dirtyItems ) {
	QPtrDictIterator<void> it( *(d->dirtyItems) );
	QListViewItem * i;
	while( (i=(QListViewItem *)(it.currentKey())) != 0 ) {
	    ++it;
	    QRect ir( itemRect( i ) );
	    if ( !ir.isEmpty() ) {
		// we now have a rectangle to give to repaint() - so do it
		viewport()->update( ir );
		return;
	    }
	}
    }
}


/*!  Ensures that the header is correctly sized and positioned.
*/

void QListView::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    d->h->resize( viewport()->width(), d->h->height() );
}

void QListView::enabledChange( bool )
{
    triggerUpdate();
}


/*!  Triggers a size, geometry and contentual update during the next
  iteration of the event loop.  Cleverly makes sure that there'll be
  just one update, to avoid flicker. */

void QListView::triggerUpdate()
{
    if ( !isVisible() )
	return; // it will update when shown.

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
	    if ( me2.button() == RightButton ) {
		mousePressEvent( &me2 );
		return TRUE;
	    }
	    break;
	case Event_MouseButtonDblClick:
	    if ( me2.button() == RightButton )
		return TRUE;
	    break;
	case Event_MouseMove:
	    if ( me2.state() & RightButton ) {
		mouseMoveEvent( &me2 );
		return TRUE;
	    }
	    break;
	case Event_MouseButtonRelease:
	    if ( me2.button() == RightButton ) {
		mouseReleaseEvent( &me2 );
		return TRUE;
	    }
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
	    return TRUE;
	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( me );
	    return TRUE;
	case Event_MouseMove:
	    mouseMoveEvent( me );
	    return TRUE;
	case Event_MouseButtonRelease:
	    mouseReleaseEvent( me );
	    return TRUE;
	case Event_FocusIn:
	    focusInEvent( fe );
	    return TRUE;
	case Event_FocusOut:
	    focusOutEvent( fe );
	    return TRUE;
	default:
	    // nothing
	    break;
	}
    }
    return QScrollView::eventFilter( o, e );
}

/*! Returns a pointer to the listview containing this item.
*/

QListView * QListViewItem::listView() const
{
    const QListViewItem* c = this;
    while (c && !c->is_root)
	c = c->parentItem;
    if (!c) return 0;
    return ((QListViewPrivate::Root*)c)->theListView();
}

/*!
  Returns the depth of this item.
*/
int QListViewItem::depth() const
{
    return parentItem ? parentItem->depth()+1 : -1; // -1 == the hidden root
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

/*! Returns a pointer to the first (top) child of this item.

  Note that the children are not guaranteed to be sorted properly.
  QListView and QListViewItem try to postpone or avoid sorting to the
  greatest degree possible, in order to keep the user interface
  snappy.

  \sa nextSibling()
*/

QListViewItem* QListViewItem::firstChild () const
{
    enforceSortOrder();
    return childItem;
}


/*! Returns a pointer to the parent of this item.

  \sa firstChild(), nextSibling()
*/

QListViewItem* QListViewItem::parent () const
{
    if ( !parentItem || parentItem->is_root ) return 0;
    return parentItem;
}


/*! \fn QListViewItem* QListViewItem::nextSibling () const

  Returns a pointer to the next sibling (below this one) of this
  item.

  Note that the siblings are not guaranteed to be sorted properly.
  QListView and QListViewItem try to postpone or avoid sorting to the
  greatest degree possible, in order to keep the user interface
  snappy.

  \sa firstChild()
*/

/*! \fn int QListViewItem::childCount () const

  Returns the current number of children of this item.
*/


/*! \fn int QListViewItem::height () const

  Returns the height of this item in pixels.  This does not include
  the height of any children; totalHeight() returns that.
*/

/*!
  Call this function when the value of width() may have changed
  for column \a c.  Normally, you should call this if text(c) changes.
  Passing -1 for \a c indicates all columns may have changed.
  For efficiency, you should do this if more than one
  call to widthChanged() is required.

  \sa width()
*/
void QListViewItem::widthChanged(int c) const
{
    listView()->widthChanged(this, c);
}

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

    if ( e->button() == RightButton ) {
	QListViewItem * i;
	if ( viewport()->rect().contains( e->pos() ) )
	    i = itemAt( e->pos() );
	else
	    i = d->currentSelected;

	if ( i ) {
	    int c = d->h->mapToLogical( d->h->cellAt( e->pos().x() ) );
	    emit rightButtonPressed( i, viewport()->mapToGlobal( e->pos() ),
				     c );
	}
	return;
    }

    if ( e->button() != LeftButton )
	return;

    d->ignoreDoubleClick = FALSE;
    d->buttonDown = TRUE;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    if ( (i->isExpandable() || i->childCount()) &&
	 d->h->mapToLogical( d->h->cellAt( e->pos().x() ) ) == 0 ) {
	int x1 = e->pos().x() +
		 d->h->offset() -
		 d->h->cellPos( d->h->mapToActual( 0 ) );
	QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );
	while( it.current() && it.current()->i != i )
	    ++it;

	if ( it.current() ) {
	    x1 -= treeStepSize() * (it.current()->l - 1);
	    if ( x1 >= 0 && ( !i->isSelectable() || x1 < treeStepSize() ) ) {
		setOpen( i, !i->isOpen() );
		if ( !d->currentSelected )
		    setCurrentItem( i );
		d->buttonDown = FALSE;
		d->ignoreDoubleClick = TRUE;
		d->buttonDown = FALSE;
		return;
	    }
	}
    }

    d->select = isMultiSelection() ? !i->isSelected() : TRUE;

    if ( i->isSelectable() )
	setSelected( i, d->select );

    i->activate();

    setCurrentItem( i );

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
	setSelected( i, d->select );

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

    // ensure that the following mouse moves and eventual release is
    // ignored.
    d->buttonDown = FALSE;

    if ( d->ignoreDoubleClick ) {
	d->ignoreDoubleClick = FALSE;
	return;
    }

    QListViewItem * i = itemAt( e->pos() );

    if ( !i ) {
	// nothing
    } else if ( i->isSelectable() )
	emit doubleClicked( i );
    else if ( !i->isOpen() && (i->isExpandable() || i->childCount()) )
	setOpen( i, TRUE );
    else if ( i->isOpen() && i->childItem )
	setOpen( i, FALSE );
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

    if ( isMultiSelection() && d->currentSelected ) {
	// also (de)select the ones in between
	QListViewItem * b = d->currentSelected;
	bool below = ( itemPos( i ) > itemPos( b ) );
	while( b && b != i ) {
	    if ( b->isSelectable() )
		setSelected( b, d->select );
	    b = below ? b->itemBelow() : b->itemAbove();
	}
    }

    if ( i->isSelectable() )
	setSelected( i, d->select );

    setCurrentItem( i );
}


/*!  Handles focus in events on behalf of viewport().  Since
  viewport() is this widget's focus proxy by default, you can think of
  this function as handling this widget's focus in events.

  \sa setFocusPolicy() setFocusProxy() focusOutEvent()
*/

void QListView::focusInEvent( QFocusEvent * )
{
    if ( d->focusItem )
	repaintItem( d->focusItem );
    else
	triggerUpdate();
    return;
}


/*!  Handles focus out events on behalf of viewport().  Since
  viewport() is this widget's focus proxy by default, you can think of
  this function as handling this widget's focus in events.

  \sa setFocusPolicy() setFocusProxy() focusInEvent()
*/

void QListView::focusOutEvent( QFocusEvent * )
{
    if ( d->focusItem )
	repaintItem( d->focusItem );
    else
	triggerUpdate();
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
	d->currentPrefix.truncate( 0 );
	// do NOT accept.  QDialog.
	return;
    case Key_Down:
	i = i->itemBelow();
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Up:
	i = i->itemAbove();
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Next:
	i2 = itemAt( QPoint( 0, viewport()->height()-1 ) );
	if ( i2 == i || !r.isValid() ||
	     viewport()->height() <= itemRect( i ).bottom() ) {
	    if ( i2 )
		i = i2;
	    int left = viewport()->height();
	    while( (i2 = i->itemBelow()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    i = i2;
	}
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Prior:
	i2 = itemAt( QPoint( 0, 0 ) );
	if ( i == i2 || !r.isValid() || r.top() <= 0 ) {
	    if ( i2 )
		i = i2;
	    int left = viewport()->height();
	    while( (i2 = i->itemAbove()) != 0 && left > i2->height() ) {
		left -= i2->height();
		i = i2;
	    }
	} else {
	    i = i2;
	}
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Right:
	if ( i->isOpen() && i->childItem )
	    i = i->childItem;
	else if (  !i->isOpen() && (i->isExpandable() || i->childCount()) )
	    setOpen( i, TRUE );
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Left:
	if ( i->isOpen() && i->childItem )
	    setOpen( i, FALSE );
	else if ( i->parentItem && i->parentItem != d->r )
	    i = i->parentItem;
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Space:
	i->activate();
	d->currentPrefix.truncate( 0 );
	e->accept();
	break;
    case Key_Escape:
	break;
    default:
	if ( e->ascii() ) {
	    QString input( d->currentPrefix );
	    input.detach();
	    QListViewItem * keyItem = i;
	    QTime now( QTime::currentTime() );
	    while( keyItem ) {
		// try twice, first with the previous string and this char
		input += (char)tolower( e->ascii() );
		const char * keyItemKey;
		QString prefix;
		while( keyItem ) {
		    // Look for text in column 0, then left-to-right
		    keyItemKey = keyItem->text(0);
		    for (int col=0; col < d->h->count() && !keyItemKey; col++ )
			keyItemKey = keyItem->text( d->h->mapToLogical(col) );
		    if ( keyItemKey && *keyItemKey ) {
			prefix = keyItemKey;
			prefix.truncate( input.length() );
			prefix = prefix.lower();
			if ( prefix == input ) {
			    d->currentPrefix = input;
			    d->currentPrefixTime = now;
			    i = keyItem;
			     // nonoptimal double-break...
			    keyItem = 0;
			    input.detach();
			    input.truncate( 0 );
			}
		    }
		    if ( keyItem )
			keyItem = keyItem->itemBelow();
		}
		// then, if appropriate, with just this character
		if ( input.length() > 1 &&
		     d->currentPrefixTime.msecsTo( now ) > 1500 ) {
		    input.truncate( 0 );
		    keyItem = d->r;
		}
	    }
	    e->accept();
	} else {
	    return;
	}
    }

    if ( !i )
	return;

    if ( i->isSelectable() &&
	 ((e->state() & ShiftButton) || !isMultiSelection()) )
	setSelected( i, d->currentSelected
		     ? d->currentSelected->isSelected()
		     : TRUE );

    setCurrentItem( i );

    ensureItemVisible( i );
}


/*!  Returns a pointer to the QListViewItem at \a screenPos.  Note
  that \a screenPos is in the coordinate system of viewport(), not in
  the listview's own, much larger, coordinate system.

  itemAt() returns 0 if there is no such item.

  \sa itemPos() itemRect()
*/

QListViewItem * QListView::itemAt( const QPoint & screenPos ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();
    int g = screenPos.y() + contentsY();

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

int QListView::itemPos( const QListViewItem * item )
{
    const QListViewItem * i = item;
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
  TRUE, the present selected item is unselected and made current.
  Unlike QListViewItem::setSelected(), this function updates the list
  view as necessary and emits the selectionChanged() signals.

  \sa isSelected() setMultiSelection() isMultiSelection() setCurrentItem()
*/

void QListView::setSelected( QListViewItem * item, bool selected )
{
    if ( !item || item->isSelected() == selected || !item->isSelectable() )
	return;

    if ( selected && !isMultiSelection() && d->currentSelected ) {
	d->currentSelected->setSelected( FALSE );
	repaintItem( d->currentSelected );
    }

    if ( item->isSelected() != selected ) {
	item->setSelected( selected );
	d->currentSelected = selected ? item : 0;
	repaintItem( item );
    }

    if ( item && !isMultiSelection() && selected && d->focusItem != item )
	setCurrentItem( item );

    if ( !isMultiSelection() )
	emit selectionChanged( item );
    emit selectionChanged();
}


/*!  Returns i->isSelected().

  Provided only because QListView provides setSelected() and trolls
  are neat creatures and like neat, orthogonal interfaces.
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
    d->focusItem = i;

    if ( i != prev ) {
	emit currentChanged( i );
	repaintItem( i );
	if ( prev )
	    repaintItem( prev );
    }
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

QRect QListView::itemRect( const QListViewItem * i ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != i )
	c = d->drawables->next();

    if ( c && c->i == i ) {
	int y = c->y - contentsY();
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
    if ( d->sortcolumn == column && d->ascending == ascending )
	return;

    d->ascending = ascending;
    d->sortcolumn = column;
    triggerUpdate();
}


/*!  Changes the column the list view is sorted by. */

void QListView::changeSortColumn( int column )
{
    setSorting( d->h->mapToLogical( column ), d->ascending );
}

/*!
  Sets an advisory spacing which list items may use.
  \sa QListViewItem::paintCell().
*/
void QListView::setItemMargin( int m )
{
    d->margin = m;
}

/*!
  Returns an advisory spacing which list items may use.
  \sa QListViewItem::paintCell().
*/
int QListView::itemMargin() const
{
    return d->margin;
}


/*! \fn void QListView::rightButtonClicked( QListViewItem *, const QPoint&, int )
void QListView::rightButtonPressed (QListViewItem *, const QPoint &, int)

  This signal is emitted when the right button is clicked (ie. when
  it's released).  The arguments are the relevant QListViewItem (may
  be 0), the point in global coordinates and the relevant column.
*/


/*! \fn void QListView::rightButtonPressed (QListViewItem *, const QPoint &, int)

  This signal is emitted when the right button is pressed.  Then
  arguments are the relevant QListViewItem (may be 0), the point in
  global coordinates and the relevant column.
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

/*!
  Ensures the width mode of column \a c is updated according
  to the width of \a item.
*/
void QListView::widthChanged(const QListViewItem* item, int c)
{
    ASSERT( c < d->h->count() );

    if ( c < 0 ) {
	// Can we stop early?
	int col = 0;
	while ( col < d->h->count() && d->column[col]->wmode == Manual )
	    col++;
	if ( col == d->h->count() )
	    return; // All have mode Manual
    }

    if ( c < 0 || d->column[c]->wmode == Maximum ) {
	QFontMetrics fm = fontMetrics();
	int col = c < 0 ? 0 : c;
	int indent = treeStepSize() * item->depth();
	do {
	    int w = item->width( fm, this, col ) + indent;
	    if ( w > columnWidth(col) )
		setColumnWidth( col, w );
	    if ( c >= 0 )
		break; // Only one
	    indent = 0; // Only col 0 has indent
	    col++;
	} while ( col < d->h->count() );
    }
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

QListViewItem * QListView::firstChild() const
{
    return d->r->childItem;
}


/*!  Repaints this item on the screen, if it is currently visible. */

void QListViewItem::repaint() const
{
    listView()->repaintItem( this );
}


/*!  Repaints \a item on the screen, if \a item is currently visible.
  Takes care to avoid multiple repaints. */

void QListView::repaintItem( const QListViewItem * item ) const
{
    d->dirtyItemTimer->start( 0, TRUE );
    if ( !d->dirtyItems )
	d->dirtyItems = new QPtrDict<void>();
    d->dirtyItems->remove( (void *)item );
    d->dirtyItems->insert( (void *)item, (void *)item );
}



/*!
  \class QCheckListItem qlistview.h
  \brief The QCheckListItem class implements checkable list view items.

  There are three types of check list items: CheckBox, RadioButton and Controller.


  Checkboxes may be inserted at top level in the list view. A radio button must
  be child of a controller.
*/


/* XPM */
static const char * def_item_xpm[] = {
"16 16 4 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"o	c #C71BC30BC71B",
"                ",
"                ",
" ..........     ",
" .XXXXXXXX.     ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" ..........oo   ",
"   oooooooooo   ",
"   oooooooooo   ",
"                ",
"                "};




static    QPixmap * tempIcon = 0; // ########### temporary!
static const int BoxSize = 16;


/*!
  Constructs a checkable item with parent \a parent, text \a text and type
  \a tt. Note that a RadioButton must be child of a Controller, otherwise
  it will not toggle.
 */
QCheckListItem::QCheckListItem( QCheckListItem *parent, const char *text,
				Type tt )
    : QListViewItem( parent, text, 0 )
{
    myType = tt;
    pix = 0;
    init();
    if ( myType == RadioButton ) {
	if ( parent->type() != Controller )
	    warning( "QCheckListItem::QCheckListItem(), radio button must be "
		     "child of a controller" );
	else
	    exclusive = parent;
    }
}

/*!
  Constructs a checkable item with parent \a parent, text \a text and type
  \a tt. Note that \a tt must not be RadioButton, if so
  it will not toggle.
 */
QCheckListItem::QCheckListItem( QListView *parent, const char *text,
				Type tt )
    : QListViewItem( parent, text, 0 )
{
    myType = tt;
    if ( tt == RadioButton )
	warning( "QCheckListItem::QCheckListItem(), radio button must be "
		 "child of a QCheckListItem" );
    pix = 0;
    init();
}

/*!
  Constructs a Controller item with parent \a parent, text \a text and pixmap
  \a p.
 */
QCheckListItem::QCheckListItem( QListView *parent, const char *text,
				const QPixmap & p )
    : QListViewItem( parent, text, 0 )
{
    myType = Controller;
    pix = new QPixmap(p);
    init();
}

/*!
  Constructs a Controller item with parent \a parent, text \a text and pixmap
  \a p.
 */
QCheckListItem::QCheckListItem( QListViewItem *parent, const char *text,
				const QPixmap & p )
    : QListViewItem( parent, text, 0 )
{
    myType = Controller;
    pix = new QPixmap(p);
    init();
}

void QCheckListItem::init()
{
    on = FALSE;
    if ( !tempIcon )
	tempIcon = new QPixmap( def_item_xpm );
    if ( myType == Controller ) {
	if ( !pix )
	    pix = tempIcon; //#####
    } else {
	pix = 0;
    }
    exclusive = 0;
}


/*! \fn QCheckListItem::Type QCheckListItem::type() const

  Returns the type of this item.
*/

/*! \fn  bool QCheckListItem::isOn() const
  Returns TRUE if this item is toggled on, FALSE otherwise.
*/


/*! \fn const char *QCheckListItem::text() const

  Returns the text of this item.
*/


/*!
  If this is a Controller that has RadioButton children, turn off the
  child that is on.
 */
void QCheckListItem::turnOffChild()
{
    if ( myType == Controller && exclusive )
	exclusive->setOn( FALSE );
}

/*!
  Toggle checkbox, or set radiobutton on.
 */
void QCheckListItem::activate()
{
    if ( myType == CheckBox ) {
	setOn( !on );
    } else if ( myType == RadioButton ) {
	setOn( TRUE );
    }
}

/*!
  Sets this button on of \a b is TRUE, off otherwise. Maintains radiobutton
  exclusivity.
 */
void QCheckListItem::setOn( bool b  )
{
    if ( b == on )
	return;
    if ( myType == CheckBox ) {
	on = b;
	stateChange( b );
    } else if ( myType == RadioButton ) {
	if ( b ) {
	    if ( exclusive && exclusive->exclusive != this )
		exclusive->turnOffChild();
	    on = TRUE;
	    if ( exclusive )
		exclusive->exclusive = this;
	} else {
	    if ( exclusive && exclusive->exclusive == this )
		exclusive->exclusive = 0;
	    on = FALSE;
	}
	stateChange( b );
    }
    repaint();
}


/*!
  This virtual function is called when the item changes its on/off state.
 */
void QCheckListItem::stateChange( bool )
{
}

/*!
  Performs setup.
 */
void QCheckListItem::setup()
{
    QListViewItem::setup();
    int h = height();
    if ( myType == Controller && pix )
	 h = QMAX( pix->height(), h );
    h = QMAX( BoxSize, h );
    setHeight( h );
}


int QCheckListItem::width( const QFontMetrics& fm, const QListView* lv, int column) const
{
    int r = QListViewItem::width( fm, lv, column );
    if ( column == 0 ) {
	r += lv->itemMargin();
	if ( myType == Controller && pix ) {
	    r += pix->width();
	} else {	
	    r += BoxSize + 4;
	}
    }
    return r;
}

/*!
  Paints this item.
 */
void QCheckListItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, int align )
{
    if ( !p )
	return;

    p->fillRect( 0, 0, width, height(), cg.base() );

    if ( column != 0 ) {
	// The rest is text, or for subclasses to change.
	QListViewItem::paintCell( p, cg, column, width, align );
	return;
    }

    QListView *lv = listView();
    if ( !lv )
	return;
    int marg = lv->itemMargin();
    int r = marg;

    bool winStyle = lv->style() == WindowsStyle;

    if ( myType == Controller && pix ) {
	p->drawPixmap( 0, (height()-pix->height())/2, *pix );
	r += pix->width();
    } else {	
	ASSERT( lv ); //###
	//	QFontMetrics fm( lv->font() );
	//	int d = fm.height();
	int x = 0;
	int y = (height() - BoxSize) / 2;
	//	p->setPen( QPen( cg.text(), winStyle ? 2 : 1 ) );
	if ( myType == CheckBox ) {
	    p->setPen( QPen( cg.text(), 2 ) );
	    p->drawRect( x+marg, y+2, BoxSize-4, BoxSize-4 );
	    /////////////////////
	    x++;
	    y++;
	    if ( on ) {
		QPointArray a( 7*2 );
		int i, xx, yy;
		xx = x+1+marg;
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
		p->setPen( black );
		p->drawLineSegments( a );
	    }
	    ////////////////////////
	} else { //radiobutton look
	    if ( winStyle ) {
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

		static QCOORD pts1[] = {		// dark lines
		    1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
		static QCOORD pts2[] = {		// black lines
		    2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
		static QCOORD pts3[] = {		// background lines
		    2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
		static QCOORD pts4[] = {		// white lines
		    2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
		    11,4, 10,3, 10,2 };
		// static QCOORD pts5[] = {		// inner fill
		//    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
		//QPointArray a;
		//	p->eraseRect( x, y, w, h );

		p->setPen( cg.text() );
		QPointArray a( QCOORDARRLEN(pts1), pts1 );
		a.translate( x, y );
		//p->setPen( cg.dark() );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts2), pts2 );
		a.translate( x, y );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts3), pts3 );
		a.translate( x, y );
		//		p->setPen( black );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts4), pts4 );
		a.translate( x, y );
		//			p->setPen( blue );
		p->drawPolyline( a );
		//		a.setPoints( QCOORDARRLEN(pts5), pts5 );
		//		a.translate( x, y );
		//	QColor fillColor = isDown() ? g.background() : g.base();
		//	p->setPen( fillColor );
		//	p->setBrush( fillColor );
		//	p->drawPolygon( a );
		if ( on     ) {
		    p->setPen( NoPen );
		    p->setBrush( cg.text() );
		    p->drawRect( x+5, y+4, 2, 4 );
		    p->drawRect( x+4, y+5, 4, 2 );
		}

	    } else { //motif
		p->setPen( QPen( cg.text() ) );
		QPointArray a;
		int cx = BoxSize/2 - 1;
		int cy = height()/2;
		int e = BoxSize/2 - 1;
		for ( int i = 0; i < 3; i++ ) { //penWidth 2 doesn't quite work
		    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e );
		    p->drawPolygon( a );
		    e--;
		}
		if ( on ) {
		    p->setPen( QPen( cg.text()) );
		    QBrush   saveBrush = p->brush();
		    p->setBrush( cg.text() );
		    e = e - 2;
		    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e );
		    p->drawPolygon( a );
		    p->setBrush( saveBrush );
		}
	    }
	}
	r += BoxSize + 4;
    }

    p->translate( r, 0 );
    QListViewItem::paintCell( p, cg, column, width - r, align );
}

/*!
  Fills the rectangle. No decoration is drawn.
 */
void QCheckListItem::paintBranches( QPainter * p, const QColorGroup & cg,
			    int w, int, int h, GUIStyle)
{
    p->fillRect( 0, 0, w, h, cg.base() );

}


/*!  Returns a size suitable for this scroll view.  This is as wide as
  mostly upon QHeader's sizeHint() recommends and tall enough for
  perhaps 10 items.
*/

QSize QListView::sizeHint() const
{
    if ( !isVisibleToTLW() &&
	 (!d->drawables || d->drawables->isEmpty()) ) {
	// force the column widths to sanity, if possible
	buildDrawableList();

	QListViewPrivate::DrawableItem * c = d->drawables->first();

	while( c && c->i ) {
	    c->i->setup();
	    c = d->drawables->next();
	}
    }

    QSize s( d->h->sizeHint() );
    QListViewItem * l = d->r;
    while( l && !l->height() )
	l = l->childItem ? l->childItem : l->siblingItem;

    if ( l && l->height() )
	s.setHeight( s.height() + 10 * l->height() );
    else
	s.setHeight( s.height() + 140 );

    if ( s.width() > s.height() * 3 )
	s.setHeight( s.width() / 3 );
    else if ( s.width() > s.height() * 2 )
	s.setHeight( s.width() / 2 );
    else if ( s.width() * 2 > s.height() * 3 )
	s.setHeight( s.width() * 3 / 2 );

    return s;
}


/*!  Sets \a item to be open if \a open is TRUE and \item is
  expandable, and to be closed if \a open is FALSE.  Repaints
  accordingly.

  Does nothing if \a item is not expandable.

  \sa QListViewItem::setOpen() QListViewItem::setExpandable()
*/

void QListView::setOpen( QListViewItem * item, bool open )
{
    if ( !item ||
	 item->isOpen() == open ||
	 (open && !item->childCount() && !item->isExpandable()) )
	return;

    item->setOpen( open );
    if ( d->drawables )
	d->drawables->clear();
    buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != item )
	c = d->drawables->next();

    if ( c && c->i == item ) {
	d->dirtyItemTimer->start( 0, TRUE );
	if ( !d->dirtyItems )
	    d->dirtyItems = new QPtrDict<void>();
	while( c && c->i ) {
	    d->dirtyItems->insert( (void *)(c->i), (void *)(c->i) );
	    c = d->drawables->next();
	}
    }
}


/*!  Identical to \a item->isOpen().  Provided for completeness.

  \sa setOpen()
*/

bool QListView::isOpen( QListViewItem * item ) const
{
    return item->isOpen();
}


/*!  Sets this list view to show open/close signs on root items if \a
  enable is TRUE, and to not show such signs if \a enable is FALSE.

  Open/close signs is a little + or - in windows style, an arrow in
  Motif style.
*/

void QListView::setRootIsDecorated( bool enable )
{
    if ( enable != (bool)d->rootIsExpandable ) {
	d->rootIsExpandable = enable;
	if ( isVisible() )
	    triggerUpdate();
    }
}


/*!  Returns TRUE if root items can be opened and closed by the user,
  FALSE if not.
*/

bool QListView::rootIsDecorated() const
{
    return d->rootIsExpandable;
}


/*!  Ensures that \a i is makde visible, scrolling the list view
  vertically as required.

  \sa itemRect() QSCrollView::ensureVisible()
*/

void QListView::ensureItemVisible( const QListViewItem * i )
{
    if ( !i )
	return;

    int h = (i->height()+1)/2;
    ensureVisible( contentsX(), itemPos( i )+h, 0, h );
}


/*! \base64 listview.gif

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

*/

/*! \fn const char * QCheckListItem::text( int n ) const

  \reimp
*/

/*! \base64 treeview.gif

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
uMtYT2+pOb/YDDCrA93GxdKbVese971blW997/uh/wZ4wCUaX00+u75oVrS5yN3TTa66vivd
rIcvJcYWT8u1ZvW0TEONYpsmm+JDZrbj/sDWWy3TmNt5nfeyT1htk+YaaLt+ONZUnjdUe+2s
xg42ND/e3Ca//I0/b+nOx4vdaxMPhSrO+LdNjmyik9pl8i1lv7mc8HcX9tOGtbHKB97yRjGc
6+DkOdi3ftixZ9bsAPw62lv1dYwDc6QKh1bbf02/084qtaUWYeDcLcGpx92eG58t1lPcdUPN
3efJmt4sk+6qkj/15LdOubIR70zlqpflzbZ14Qt1eMQf+Voyx2SxQX/stJb9XJ4npuUHjPnw
JtfpvRYytsHNa0VmUdN2de+62x71Jzau9niuOt/LDW+xEnjyY8+32tfe7+WbferOX/vxpf++
6FN/yY1ao4mH/u/Z/3Vyk48GKfvI3VRnezbiVz60wyNdTcWSstljy/HFZ4bf1+V4Ygn7HsN1
Wn+6+1T78KJ4qGZz0qZqhNdBvqZcXBV/XjaAJyRNpbdyrQRPz+Vq2tRV2eZVUiZsyOR+Swdr
dHR77cV0led6zCNOyuZl6xR/IviADriCBTiCCAhyWTRi3vdG/9NuuvRu9odjpPd4M3h9YQc2
1sdHfpcoRXhHR4iEQjg/SdiE5AWF1jNRZwNS75Nod7doyhR+c2R3Eod312RqjYZ/S0iDzfNp
sjV4Hnd6WSiB3LNtmxeCPJaBMQOCUYhTYohOoUc2UTZD5sd4TIhYbqiHgAh00VYtrQtIh9jX
hrYjg2XIZIe4UzoHPPJmN9iGXoV4c7e3XI4IZpW4O1GXf3vIgHBndXkyeOh3hwAmhesndqxY
KM33ivxyYLKYULVoi7dYUJ8jYUf0iNmXcG0od1v4hek3Sqn4fVpYVJfoe2gYSxoXWxyXJ4Qn
bhMVcllmes44h64mO3ZYdILohjI4inLYehqoedSoUnlIiMfIh6OXiNv4iQyUjrnHTnhEjkJ3
V3FIKAEBADs=

*/


/*!  Returns a pointer to the QHeader object that manages this list
  view's columns.  Please don't modify the header behind the list
  view's back.

  \sa setHeader()
*/

QHeader * QListView::header() const
{
    return d->h;
}
