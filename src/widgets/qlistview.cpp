/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#249 $
**
** Implementation of QListView widget class
**
** Created : 970809
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

#include "qlistview.h"
#include "qtimer.h"
#include "qheader.h"
#include "qpainter.h"
#include "qstack.h"
#include "qlist.h"
#include "qstrlist.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qptrdict.h"
#include "qvector.h"
#include "qiconset.h"

#include "qpixmapcache.h"

#include <stdlib.h> // qsort
#include <ctype.h> // tolower

const int Unsorted = 16383;

static QBitmap * verticalLine = 0;
static QBitmap * horizontalLine = 0;

static void cleanupBitmapLines()
{
    delete verticalLine;
    delete horizontalLine;
    verticalLine = 0;
    horizontalLine = 0;
}

struct QListViewPrivate
{
    // classes that are here to avoid polluting the global name space

    // the magical hidden mother of all items
    class Root: public QListViewItem {
      public:
	Root( QListView * parent );

	void setHeight( int );
	void invalidateHeight();
	void setup();
	QListView * theListView() const;

	QListView * lv;
    };

    // for the stack used in drawContentsOffset()
    class Pending {
      public:
	Pending( int level, int ypos, QListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l; // level of this item; root is -1 or 0
	int y; // level of this item in the tree
	QListViewItem * i; // the item itself
    };

    // to remember what's on screen
    class DrawableItem {
      public:
	DrawableItem( Pending * pi ) { y=pi->y; l=pi->l; i=pi->i; };
	int y;
	int l;
	QListViewItem * i;
    };

    // for sorting
    class SortableItem {
      public:
	QString key;
	QListViewItem * i;
    };

    class ItemColumnInfo {
      public:
	ItemColumnInfo(): pm( 0 ), next( 0 ) {}
	~ItemColumnInfo() { delete pm; delete next; }
	QString text;
	QPixmap * pm;
	ItemColumnInfo * next;
    };

    class ViewColumnInfo {
      public:
	ViewColumnInfo(): align(Qt::AlignLeft), sortable(TRUE), next( 0 ) {}
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

    // holds a list of iterators
    QList<QListViewItemIterator> *iterators;

    QTimer *scrollTimer;
};



/*!
  \class QListViewItem qlistview.h
  \brief The QListViewItem class implements a list view item.

  A list viev item is a multi-column object capable of displaying
  itself.  Its design has the following main goals: <ul> <li> Work
  quickly and well for \e large sets of data. <li> Be easy to use in
  the simple case. </ul>

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
  parent is a QListView, this item is a top-level item within that
  QListView.  If the parent is another QListViewItem, this item
  becomes a child of the parent item.

  If you keep the pointer, you can set or change the texts using
  setText(), add pixmaps using setPixmap(), change its mode using
  setSelectable(), setSelected(), setOpen() and setExpandable(),
  change its height using setHeight(), and do much tree traversal.
  The set* functions in QListView also affect QListViewItem, of
  course.

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

  (No iterator class is provided for QListViewItem, as a simple
  iterator does not adequately express the choices QListViewItem
  necessitates: Would <code>++it</code> mean "next sibling", "child if
  any, else next sibling", "child if this object is open and has
  children, else next subling" or any of a few other possible
  meanings?)

  Note that the order of the children will change when the sorting
  order changes, and is undefined if the items are not visible.  You
  can however call enforceSortOrder() at any time, and QListView will
  always call it before it needs to show an item.

  Many programs will need to reimplement QListViewItem.  The most
  commonly reimplemented functions are: <ul> <li> text() returns the
  text in a column.  Many subclasses will compute that on the
  fly. <li> key() is used for sorting.  The default key() simply calls
  text(), but judicious use of key can be used to sort by e.g. date
  (as QFileDialog does).  <li> setup() is called before showing the
  item, and whenever e.g. the font changes. <li> activate() is called
  whenever the user clicks on the item or presses space when the item
  is the currently highlighted item.</ul>

  Some subclasses call setExpandable( TRUE ) even when they have no
  children, and populate themselves when setup() or setOpen( TRUE ) is
  called.  The dirview/dirview.cpp example program uses precisely this
  technique to start up quickly: The files and subdirectories in a
  directory aren't entered into the tree until they need to.

  This example shows a number of root items in a QListView.  These
  items are actually subclassed from QListViewItem: The file size,
  type etc. are computed on the fly.

  <img src="listview.png" width="518" height="82" alt="Example List View">

  The next example shows a fraction of the dirview example.  Again,
  the Directory/Symbolic Link column is computed on the fly.  None of
  the items are root items; the \e usr item is a child of the root and
  the \e X11 item is a child of the \e usr item.

  <img src="treeview.png" width="227" height="261" alt="Example Tree View">
*/

/*!  Creates a new top-level list view item in the QListView \a parent.
*/

QListViewItem::QListViewItem( QListView * parent )
{
    init();
    parent->insertItem( this );
}


/*!  Creates a new list view item which is a child of \a parent and first
  in the parent's list of children. */

QListViewItem::QListViewItem( QListViewItem * parent )
{
    init();
    parent->insertItem( this );
}




/*!  Constructs an empty list view item which is a child of \a parent
  and is after \a after in the parent's list of children */

QListViewItem::QListViewItem( QListView * parent, QListViewItem * after )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );
}


/*!  Constructs an empty list view item which is a child of \a parent
  and is after \a after in the parent's list of children */

QListViewItem::QListViewItem( QListViewItem * parent, QListViewItem * after )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );
}



/*!  Creates a new list view item in the QListView \a parent,
  \a parent, with at most 8 constant strings as contents.

  \code
     (void)new QListViewItem( lv, "/", "Root directory" );
  \endcode

  \sa setText()
*/

QListViewItem::QListViewItem( QListView * parent,
			      QString label1,
			      QString label2,
			      QString label3,
			      QString label4,
			      QString label5,
			      QString label6,
			      QString label7,
			      QString label8 )
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

  \sa setText()
*/

QListViewItem::QListViewItem( QListViewItem * parent,
			      QString label1,
			      QString label2,
			      QString label3,
			      QString label4,
			      QString label5,
			      QString label6,
			      QString label7,
			      QString label8 )
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

/*!  Creates a new list view item in the QListView \a parent,
  after item \a after, with at most 8 constant strings as contents.

  Note that the order is changed according to QListViewItem::key()
  unless the list view's sorting is disabled using
  QListView::setSorting( -1 ).

  \sa setText()
*/

QListViewItem::QListViewItem( QListView * parent, QListViewItem * after,
			      QString label1,
			      QString label2,
			      QString label3,
			      QString label4,
			      QString label5,
			      QString label6,
			      QString label7,
			      QString label8 )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );

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
  \a parent, after item \a after, with at most 8 constant strings as
  contents.

  Note that the order is changed according to QListViewItem::key()
  unless the list view's sorting is disabled using
  QListView::setSorting( -1 ).

  \sa setText()
*/

QListViewItem::QListViewItem( QListViewItem * parent, QListViewItem * after,
			      QString label1,
			      QString label2,
			      QString label3,
			      QString label4,
			      QString label5,
			      QString label6,
			      QString label7,
			      QString label8 )
{
    init();
    parent->insertItem( this );
    moveToJustAfter( after );

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
    QListView *lv = listView();

    if ( lv ) {
        if ( lv->d->iterators ) {
	    QListViewItemIterator *i = lv->d->iterators->first();
	    while ( i ) {
	        if ( i->current() == this )
		    i->currentRemoved();
		i = lv->d->iterators->next();
	    }
	}
    }

    if ( parentItem )
	parentItem->removeItem( this );
    QListViewItem * i = childItem;
    childItem = 0;
    while ( i ) {
	i->parentItem = 0;
	QListViewItem * n = i->siblingItem;
	delete i;
	i = n;
    }
    delete (QListViewPrivate::ItemColumnInfo *)columns;
}


/*!  Inserts \a newChild into its list of children.  You should not
  need to call this function; it is called automatically by the
  constructor of \a newChild.

  This function works even if this item is not contained in a list view.
*/

void QListViewItem::insertItem( QListViewItem * newChild )
{
    if ( !newChild || newChild->parentItem == this )
	return;
    if ( newChild->parentItem )
	newChild->parentItem->removeItem( newChild );
    if ( open )
	invalidateHeight();
    newChild->siblingItem = childItem;
    childItem = newChild;
    nChildren++;
    newChild->parentItem = this;
    lsc = Unsorted;
    newChild->ownHeight = 0;
    newChild->configured = FALSE;
}


/*!\obsolete
  Removes \a tbg from this object's list of children and causes an
  update of the screen display.  You should normally not need to call
  this function, as QListViewItem::~QListViewItem() calls it. The normal way
  to delete an item is \c delete.

  \warning This function leaves \a tbg and its children in a state
  where most member functions are unsafe.  Only the few functions that
  are explicitly documented to work in this state may be used then.

  \sa QListViewItem::insertItem()
*/
void QListViewItem::removeItem( QListViewItem * item )
{
    if ( !item )
	return;

    QListView * lv = listView();
    if ( lv ) {

        if ( lv->d->iterators ) {
	    QListViewItemIterator *i = lv->d->iterators->first();
	    while ( i ) {
   	        if ( i->current() == item )
		    i->currentRemoved();
		i = lv->d->iterators->next();
	    }
	}	

	invalidateHeight();

	if ( lv->d && lv->d->drawables ) {
	    delete lv->d->drawables;
	    lv->d->drawables = 0;
	}

	if ( lv->d->dirtyItems ) {
	    if ( item->childItem ) {
		delete lv->d->dirtyItems;
		lv->d->dirtyItems = 0;
		lv->d->dirtyItemTimer->stop();
		lv->triggerUpdate();
	    } else {
		lv->d->dirtyItems->take( (void *)item );
	    }
	}

	if ( lv->d->currentSelected ) {
	    QListViewItem * c = lv->d->currentSelected;
	    while( c && c != item )
		c = c->parentItem;
	    if ( c == item ) {
		lv->d->currentSelected = 0;
		emit lv->selectionChanged( 0 );
	    }
	}

	if ( lv->d->focusItem ) {
	    const QListViewItem * c = lv->d->focusItem;
	    while( c && c != item )
		c = c->parentItem;
	    if ( c == item )
		lv->d->focusItem = 0;
	}
    }

    nChildren--;

    QListViewItem ** nextChild = &childItem;
    while( nextChild && *nextChild && item != *nextChild )
	nextChild = &((*nextChild)->siblingItem);

    if ( nextChild && item == *nextChild )
	*nextChild = (*nextChild)->siblingItem;
    item->parentItem = 0;
}


/*!
  Removes \a item from this object's list of children and causes an
  update of the screen display.  You should normally not need to call
  this function, as QListViewItem::~QListViewItem() calls it. The normal way
  to delete an item is \c delete.

  \warning This function leaves \a item and its children in a state
  where most member functions are unsafe.  Only the few functions that
  are explicitly documented to work in this state may be used then.

  \sa QListViewItem::insertItem()
*/

void QListViewItem::takeItem( QListViewItem * item )
{
    removeItem( item );
}


/*!
  \fn QString QListViewItem::key( int column, bool ascending ) const

  Returns a key that can be used for sorting by column \a column.
  The default implementation returns text().  Derived classes may
  also incorporate the order indicated by \a ascending into this
  key, although this is not recommended.

  You can use this function to sort by non-alphabetic data.  This code
  excerpt sort by file modification date, for example

  \code
    if ( column == 3 ) {
        QDateTime epoch( QDate( 1980, 1, 1 ) );
	tmpString.sprintf( "%08d", epoch.secsTo( myFile.lastModified() ) );
    } else {
        // ....
    }
    return tmpString;
  \endcode

  \sa sortChildItems()
*/

QString QListViewItem::key( int column, bool ) const
{
    return text( column );
}


static int cmp( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    return ((QListViewPrivate::SortableItem *)n1)->key.
	    compare( ((QListViewPrivate::SortableItem *)n2)->key );
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

    if ( column < 0 )
	return;

    // more dubiously - only sort if the child items "exist"
    if ( !isOpen() || !childCount() )
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


/*!  Sets this item's own height to \a height pixels.  This implicitly
  changes totalHeight() too.

  Note that e.g. a font change causes this height to be overwritten
  unless you reimplement setup().

  For best results in Windows style, we suggest using an even number
  of pixels.

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

  This function works even if this item is not contained in a list view.

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

  Also does some bookkeeping.

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

  The default calls widthChanged() and sets the item's height to the
  height of a single line of text in the list view's font.  (If you
  use icons, multi-line text etc. you will probably need to call
  setHeight() yourself or reimplement this.)
*/

void QListViewItem::setup()
{
    widthChanged();
    QListView * v = listView();
    int h = v->d->fontMetricsHeight + 2*v->itemMargin();
    if ( h % 2 > 0 )
	h++;
    setHeight( h );
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


/*! \fn bool QListViewItem::isExpandable() const

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

  Note that root items are not expandable by the user unless
  QListView::setRootIsDecorated() is set to TRUE.

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
    else if ( !parentItem &&
	      ( (int)lsc != listView()->d->sortcolumn ||
		(bool)lso != listView()->d->ascending ) &&
	       listView()->d->sortcolumn != Unsorted )
	((QListViewItem *)this)->sortChildItems( listView()->d->sortcolumn,
						 listView()->d->ascending );
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


/*!  Returns the text in column \a column, or a
  \link QString::operator!() null string \endlink if there
  is no text in that column.

  This function works even if this item is not contained in a list
  view, but reimplementations of it are not required to work properly
  in that case.

  \sa key() paintCell()
*/

QString QListViewItem::text( int column ) const
{
    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;

    while( column && l ) {
	l = l->next;
	column--;
    }

    return l ? l->text : QString::null;
}


/*!  Sets the text in column \a column to \a text, if \a column is a
  valid column number and \a text is non-null.

  If \a text() has been reimplemented, this function may be a no-op.

  \sa text() key()
*/

void QListViewItem::setText( int column, const QString &text )
{
    if ( column < 0 )
	return;


    QListViewPrivate::ItemColumnInfo * l
	= (QListViewPrivate::ItemColumnInfo*) columns;
    if ( !l ) {
	l = new QListViewPrivate::ItemColumnInfo;
	columns = (void*)l;
    }
    for( int c=0; c<column; c++ ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ItemColumnInfo;
	l = l->next;
    }
    if ( l->text == text )
	return;

    l->text = text;
    if ( parent() )
	parent()->lsc = Unsorted;
    widthChanged( column );
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

    for( int c=0; c<column; c++ ) {
	if ( !l->next )
	    l->next = new QListViewPrivate::ItemColumnInfo;
	l = l->next;
    }

    if ( ( pm.isNull() && ( !l->pm || l->pm->isNull() ) ) ||
	 ( l->pm && pm.serialNumber() == l->pm->serialNumber() ) )
	return;

    if ( pm.isNull() ) {
	delete l->pm;
	l->pm = 0;
    } else {
	if ( l->pm )
	    *(l->pm) = pm;
	else
	    l->pm = new QPixmap( pm );
    }
    widthChanged( column );
    repaint();
}


/*!  Returns a pointer to the pixmap for \a column, or a null pointer
  if there is no pixmap for \a column.

  This function works even if this item is not contained in a list
  view, but reimplementations of it are not required to work properly
  in that case.

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
  and should honor isSelected() and QListView::allColumnsShowFocus().

  If you reimplement this function, you should also reimplement
  width().

  The rectangle to be painted is in an undefined state when this
  function is called, so you \e must draw on all the pixels.  The
  painter \a p has the right font on entry.

  \sa paintBranches(), QListView::drawContentsOffset()
*/

void QListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, int align )
{
    // Change width() if you change this.

    if ( !p )
	return;

    QListView *lv = listView();
    int r = lv ? lv->itemMargin() : 1;
    const QPixmap * icon = pixmap( column );

    p->fillRect( 0, 0, width, height(), cg.base() );

    int marg = lv ? lv->itemMargin() : 1;

    if ( isSelected() &&
	 (column==0 || listView()->allColumnsShowFocus()) ) {
	    p->fillRect( r - marg, 0, width - r + marg, height(),
			 cg.brush( QColorGroup::Highlight ) );
	    p->setPen( cg.highlightedText() );
    } else {
	p->setPen( cg.text() );
    }

    if ( icon ) {
	p->drawPixmap( r, (height()-icon->height())/2, *icon );
	r += icon->width() + listView()->itemMargin();
    }

    QString t = text( column );
    if ( !t.isEmpty() ) {
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
int QListViewItem::width( const QFontMetrics& fm,
			  const QListView* lv, int c ) const
{
    int w = fm.width( text( c ) ) + lv->itemMargin() * 2;
    const QPixmap * pm = pixmap( c );
    if ( pm )
	w += pm->width() + lv->itemMargin(); // ### correct margin stuff?
    return w;
}


/*!  Paints a focus indication on the rectangle \a r using painter \a p
  and colors \a cg.

  \a p is already clipped.

  \sa paintCell() paintBranches() QListView::setAllColumnsShowFocus()
*/

void QListViewItem::paintFocus( QPainter *p, const QColorGroup &cg,
				const QRect & r )
{
    listView()->style().drawFocusRect( p, r, cg, isSelected()? & cg.highlight() : & cg.base(), isSelected() );
}


/*!  Paints a set of branches from this item to (some of) its children.

  \a p is set up with clipping and translation so that you can draw
  only in the rectangle you need to; \a cg is the color group to use;
  the update rectangle is at 0, 0 and has size \a w, \a h.  The top of
  the rectangle you own is at \a y (which is never greater than 0 but
  can be outside the window system's allowed coordinate range).

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

    int dotoffset = (itemPos() + height() - y) %2;

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
	    // needs a box
	    p->setPen( cg.dark() );
	    p->drawRect( bx-4, linebot-4, 9, 9 );
	    p->setPen( cg.foreground() ); // ### windows uses black
	    if ( s == WindowsStyle ) {
		// plus or minus
		p->drawLine( bx - 2, linebot, bx + 2, linebot );
		if ( !child->isOpen() )
		    p->drawLine( bx, linebot - 2, bx, linebot + 2 );
	    } else {
		QPointArray a;
		if ( child->isOpen() )
		    a.setPoints( 3, bx-2, linebot-2,
				 bx, linebot+2,
				 bx+2, linebot-2 ); //RightArrow
		else
		    a.setPoints( 3, bx-2, linebot-2,
				 bx+2, linebot,
				 bx-2, linebot+2 ); //DownArrow
		p->setBrush( cg.foreground() );
		p->drawPolygon( a );
		p->setBrush( NoBrush );
	    }
	    // dotlinery
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, linebot - 5 );
	    dotlines[c++] = QPoint( bx + 5, linebot );
	    dotlines[c++] = QPoint( w, linebot );
	    linetop = linebot + 5;
	} else {
	    // just dotlinery
	    dotlines[c++] = QPoint( bx+1, linebot );
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

    p->setPen( cg.dark() );
    if ( s == WindowsStyle ) {
	if ( !verticalLine ) {
	    // make 128*1 and 1*128 bitmaps that can be used for
	    // drawing the right sort of lines.
	    verticalLine = new QBitmap( 1, 129, TRUE );
	    horizontalLine = new QBitmap( 128, 1, TRUE );
	    QPointArray a( 64 );
	    QPainter p;
	    p.begin( verticalLine );
	    int i;
	    for( i=0; i<64; i++ )
		a.setPoint( i, 0, i*2+1 );
	    p.setPen( color1 );
	    p.drawPoints( a );
	    p.end();
	    QApplication::flushX();
	    verticalLine->setMask( *verticalLine );
	    p.begin( horizontalLine );
	    for( i=0; i<64; i++ )
		a.setPoint( i, i*2+1, 0 );
	    p.setPen( color1 );
	    p.drawPoints( a );
	    p.end();
	    QApplication::flushX();
	    horizontalLine->setMask( *horizontalLine );
	    qAddPostRoutine( cleanupBitmapLines );
	}
	int line; // index into dotlines
	for( line = 0; line < c; line += 2 ) {
	    // assumptions here: lines are horizontal or vertical.
	    // lines always start with the numerically lowest
	    // coordinate.

	    // point ... relevant coordinate of current point
	    // end ..... same coordinate of the end of the current line
	    // other ... the other coordinate of the current point/line
	    if ( dotlines[line].y() == dotlines[line+1].y() ) {
		int end = dotlines[line+1].x();
		int point = dotlines[line].x();
		int other = dotlines[line].y();
		while( point < end ) {
		    int i = 128;
		    if ( i+point > end )
			i = end-point;
		    p->drawPixmap( point, other, *horizontalLine,
				   0, 0, i, 1 );
		    point += i;
		}
	    } else {
		int end = dotlines[line+1].y();
		int point = dotlines[line].y();
		int other = dotlines[line].x();
		int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;
		while( point < end ) {
		    int i = 128;
		    if ( i+point > end )
			i = end-point;
		    p->drawPixmap( other, point, *verticalLine,
				   0, pixmapoffset, 1, i );
		    point += i;
		}
	    }
	}
    } else {
	int line; // index into dotlines
	for( line = 0; line < c; line += 2 ) {
	    p->drawLine( dotlines[line].x(), dotlines[line].y(),
			 dotlines[line+1].x(), dotlines[line+1].y() );
	}
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

  <li>setSorting() - decides whether the items should be sorted,
  whether it should be in ascending or descending order, and by what
  column it should be sorted.</ul>

  To handle events such as mouse-presses on the listview, derived classes
  can override the QScrollView functions
\link QScrollView::contentsMousePressEvent() contentsMousePressEvent\endlink,
\link QScrollView::contentsMouseReleaseEvent() contentsMouseReleaseEvent\endlink,
\link QScrollView::contentsMouseDoubleClickEvent() contentsMouseDoubleClickEvent\endlink,
\link QScrollView::contentsMouseMoveEvent() contentsMouseMoveEvent\endlink,
\link QScrollView::contentsDragEnterEvent() contentsDragEnterEvent\endlink,
\link QScrollView::contentsDragMoveEvent() contentsDragMoveEvent\endlink,
\link QScrollView::contentsDragLeaveEvent() contentsDragLeaveEvent\endlink,
\link QScrollView::contentsDropEvent() contentsDropEvent\endlink, and
\link QScrollView::contentsWheelEvent() contentsWheelEvent\endlink.

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

  <img src="listview.png" width="518" height="82" alt="Example List View">
  <br clear="all">
  Windows style, flat (from QFileDialog)

  <img src="treeview.png" width="256" height="216" alt="Example Tree View">
  <br clear="all">
  Motif style, hierarchical (from the dirview/dirview.cpp example)

  \internal

  need to say stuff about the mouse and keyboard interface.
*/

/*!  Creates a new empty list view, with \a parent as a parent and \a
  name as object name. */

QListView::QListView( QWidget * parent, const char *name )
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
    d->margin = 1;
    d->multi = 0;
    d->sortcolumn = 0;
    d->ascending = TRUE;
    d->allColumnsShowFocus = FALSE;
    d->fontMetricsHeight = fontMetrics().height();
    d->h->setTracking(TRUE);
    d->buttonDown = FALSE;
    d->ignoreDoubleClick = FALSE;
    d->column.setAutoDelete( TRUE );
    d->iterators = 0;
    d->scrollTimer = 0;

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
    setFocusPolicy( StrongFocus );
}


/*!  Deletes the list view and all items in it, and frees all
  allocated resources.  */

QListView::~QListView()
{
    if ( d->iterators ) {
        QListViewItemIterator *i = d->iterators->first();
	while ( i ) {
	    i->listView = 0L;
	    i = d->iterators->next();
	}
	delete d->iterators;
    }

    d->focusItem = 0;
    d->currentSelected = 0;
    delete d->r;
    d->r = 0;
    delete d->dirtyItems;
    d->dirtyItems = 0;
    delete d->drawables;
    d->drawables = 0;
    delete d->vci;
    d->vci = 0;
    delete d;
    d = 0;
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
	QRect br( cx - ox, cy - oy, cw, ch );
	QPtrDictIterator<void> it( *(d->dirtyItems) );
	QListViewItem * i;
	while( (i=(QListViewItem *)(it.currentKey())) != 0 ) {
	    ++it;
	    QRect ir = itemRect( i ).intersect( viewport()->rect() );
	    if ( ir.isEmpty() || br.contains( ir ) )
		// we're painting this one, or it needs no painting: forget it
		d->dirtyItems->remove( (void *)i );
	}
	if ( d->dirtyItems->count() ) {
	    // there are still items left that need repainting
	    d->dirtyItemTimer->start( 0, TRUE );
	} else {
	    // we're painting all items that need to be painted
	    delete d->dirtyItems;
	    d->dirtyItems = 0;
	    d->dirtyItemTimer->stop();
	}
    }

    p->setFont( font() );

    QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );

    QRect r;
    int fx = -1, x, fc = 0, lc = 0;
    int tx = -1;
    QListViewPrivate::DrawableItem * current;

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
		cs = d->h->cellSize( 0 	);
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
		//WINDOWSBUG### should use this
                //p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
                p->translate( r.left(), r.top() );
		int ac = d->h->mapToLogical( c );
		current->i->paintCell( p, colorGroup(), ac, r.width(),
				       columnAlignment( ac ) );
		p->restore();
		if ( c == 0 && current->i == d->focusItem && hasFocus() &&
		     !d->allColumnsShowFocus ) {
		    p->save();
		    //WINDOWSBUG### should use this
		    //p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
		    current->i->paintFocus( p, colorGroup(), r );
		    p->restore();
		}
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
		//WINDOWSBUG### should use this
		//p->setClipRect( r );
		p->translate( rleft-ox, crtop-oy );
		current->i->paintBranches( p, colorGroup(), treeStepSize(),
					   rtop - crtop, r.height(), style() );
		p->restore();
	    }
	}

	// does current need focus indication?
	if ( current->i == d->focusItem && hasFocus() &&
	     d->allColumnsShowFocus ) {
	    p->save();
	    int x = contentsX() - ox;
	    int w = visibleWidth();
	    if ( contentsX() ) {
		x--;
		w++;
	    }
	    if ( contentsX() + visibleWidth() < contentsWidth() ) {
		w++;
	    }
	    r.setRect( x, current->y - oy, w, ih );
	    //WINDOWSBUG### should use this
	    //p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
	    current->i->paintFocus( p, colorGroup(), r );
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


/*! Rebuilds the list of drawable QListViewItems.  This function is
  const so that const functions can call it without requiring
  d->drawables to be mutable */

void QListView::buildDrawableList() const
{
    d->r->enforceSortOrder();

    QStack<QListViewPrivate::Pending> stack;
    stack.push( new QListViewPrivate::Pending( ((int)d->rootIsExpandable)-1,
					       0, d->r ) );

    // could mess with cy and ch in order to speed up vertical
    // scrolling
    int cy = contentsY();
    int ch = ((QListView *)this)->visibleHeight();
    // ### hack to help sizeHint().  if not visible, assume that we'll
    // ### use 200 pixels rather than whatever QScrollView thinks.
    // ### this lets sizeHint() base its width on a more realistic
    // ### number of items.
    if ( !isVisible() && ch < 200 )
	ch = 200;
    d->topPixel = cy + ch; // one below bottom
    d->bottomPixel = cy - 1; // one above top

    QListViewPrivate::Pending * cur;

    // used to work around lack of support for mutable
    QList<QListViewPrivate::DrawableItem> * dl;

    dl = new QList<QListViewPrivate::DrawableItem>;
    dl->setAutoDelete( TRUE );
    if ( d->drawables )
	delete ((QListView *)this)->d->drawables;
    ((QListView *)this)->d->drawables = dl;

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
	if ( cur->i->isOpen() && cur->i->childCount() &&
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


/*!  Inserts \a i into the list view as a top-level item.  You do not
  need to call this unless you've called removeItem( \a i ) or
  QListViewItem::removeItem( i ) and need to reinsert \a i elsewhere.

  \sa QListViewItem::removeItem() (important) removeItem()
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
    if ( d->iterators ) {
        QListViewItemIterator *i = d->iterators->first();
	while ( i ) {
	    i->curr = 0L;
	    i = d->iterators->next();
	}
    }	

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
int QListView::addColumn( const QString &label, int width )
{
    int c = d->h->addLabel( label, width );
    d->column.resize( c+1 );
    d->column.insert( c, new QListViewPrivate::Column );
    d->column[c]->wmode = width >=0 ? Manual : Maximum;
    return c;
}

/*!
  Adds a new column at the right end of the widget, with the header \a
  label and \a iconset, and returns the index of the column.

  If \a width is negative, the new column will have WidthMode Maximum,
  otherwise it will be Manual at \a width pixels wide.

  \sa setColumnText() setColumnWidth() setColumnWidthMode()
*/
int QListView::addColumn( const QIconSet& iconset, const QString &label, int width )
{
    int c = d->h->addLabel( iconset, label, width );
    d->column.resize( c+1 );
    d->column.insert( c, new QListViewPrivate::Column );
    d->column[c]->wmode = width >=0 ? Manual : Maximum;
    return c;
}

/*!
  Sets the heading text of column \a column to \a label.  The leftmost
  colum is number 0.
*/
void QListView::setColumnText( int column, const QString &label )
{
    if ( column < d->h->count() )
	d->h->setLabel( column, label );
}

/*!
  Sets the heading text of column \a column to \a iconset and \a
  label.  The leftmost colum is number 0.
*/
void QListView::setColumnText( int column, const QIconSet& iconset, const QString &label )
{
    if ( column < d->h->count() )
	d->h->setLabel( column, iconset, label );
}

/*!
  Sets the width of column \a column to \a w pixels.  Note that if the
  column has a WidthMode other than Manual, this width setting may be
  subsequently overridden.  The leftmost colum is number 0.
*/
void QListView::setColumnWidth( int column, int w )
{
    if ( column < d->h->count() && d->h->cellSize( column ) != w ) {
	d->h->setCellSize( column, w );
	d->h->update(); // ##### paul, QHeader::setCellSize should do this.
    }
}

/*!
  Returns the text for the heading of column \a c.
*/
QString QListView::columnText( int c ) const
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
  Configures the logical column \a column to have alignment \a align.
  The alignment is ultimately passed to QListViewItem::paintCell()
  for each item in the view.

  The display is automatically scheduled to be updated.
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
  Returns the alignment of logical column \a column.  The default
  is \c AlignLeft.
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
    ensureItemVisible( d->focusItem );
}

void QListView::updateGeometries()
{
    int th = d->r->totalHeight();
    int tw = d->h->cellPos( d->h->count()-1 ) +
	    d->h->cellSize( d->h->count()-1 );
    if ( d->h->offset() &&
	 tw < d->h->offset() + d->h->width() )
	horizontalScrollBar()->setValue( tw - QListView::d->h->width() );
    resizeContents( tw, th );
    if ( d->h->testWState( WState_ForceHide ) ) {
	setMargins( 0, 0, 0, 0 );
    } else {
	QSize hs( d->h->sizeHint() );
	setMargins( 0, hs.height(), 0, 0 );
	d->h->setGeometry( viewport()->x(), viewport()->y()-hs.height(),
			   visibleWidth(), hs.height() );
    }
}

/*!
  Updates the display when a section has changed size.
*/
void QListView::handleSizeChange( int section, int, int )
{
    updateGeometries();
    int left = d->h->cellPos( d->h->mapToActual( section ) - d->h->offset() );
    viewport()->repaint( left, 0, visibleWidth()-left,
                         visibleHeight(), FALSE );
}


/*!  Very smart internal slot that'll repaint JUST the items that need
  to be repainted.  Don't use this directly; call repaintItem() and
  this slot gets called by a null timer.
*/

void QListView::updateDirtyItems()
{
    if ( d->timer->isActive() || !d->dirtyItems)
        return;
    QRect ir;
    QPtrDictIterator<void> it( *(d->dirtyItems) );
    QListViewItem * i;
    while( (i=(QListViewItem *)(it.currentKey())) != 0 ) {
        ++it;
        ir = ir.unite( itemRect(i) );
    }
    if ( !ir.isEmpty() )                    // rectangle to be repainted
        viewport()->repaint( ir, FALSE );
}


/*!  Ensures that the header is correctly sized and positioned.
*/

void QListView::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    d->h->resize( visibleWidth(), d->h->height() );
}

void QListView::enabledChange( bool )
{
    triggerUpdate();
}


/*!  Triggers a size, geometry and content update during the next
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
	 e->type() >= QEvent::MouseButtonPress &&
	 e->type() <= QEvent::MouseMove ) {
	QMouseEvent * me = (QMouseEvent *)e;
	QMouseEvent me2( me->type(),
			 QPoint( me->pos().x(),
				 me->pos().y() - d->h->height() ),
			 me->button(), me->state() );
	switch( me2.type() ) {
	case QEvent::MouseButtonPress:
	    if ( me2.button() == RightButton ) {
		viewportMousePressEvent( &me2 );
		return TRUE;
	    }
	    break;
	case QEvent::MouseButtonDblClick:
	    if ( me2.button() == RightButton )
		return TRUE;
	    break;
	case QEvent::MouseMove:
	    if ( me2.state() & RightButton ) {
		viewportMouseMoveEvent( &me2 );
		return TRUE;
	    }
	    break;
	case QEvent::MouseButtonRelease:
	    if ( me2.button() == RightButton ) {
		viewportMouseReleaseEvent( &me2 );
		return TRUE;
	    }
	    break;
	default:
	    break;
	}
    } else if ( o == viewport() ) {
	QFocusEvent * fe = (QFocusEvent *)e;

	switch( e->type() ) {
	case QEvent::FocusIn:
	    focusInEvent( fe );
	    return TRUE;
	case QEvent::FocusOut:
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
    while ( c && !c->is_root )
	c = c->parentItem;
    if ( !c )
	return 0;
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
  grandparent's etc., or something else if anyoftheabove->height()
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


/*!
  Returns the height of this item in pixels.  This does not include
  the height of any children; totalHeight() returns that.
*/
int QListViewItem::height() const
{
    QListViewItem * that = (QListViewItem *)this;
    if ( !that->configured ) {
	that->configured = TRUE;
	that->setup(); // ### virtual non-const function called in const
    }

    return ownHeight;
}

/*!
  Call this function when the value of width() may have changed
  for column \a c.  Normally, you should call this if text(c) changes.
  Passing -1 for \a c indicates all columns may have changed.
  For efficiency, you should do this if more than one
  call to widthChanged() is required.

  \sa width()
*/
void QListViewItem::widthChanged( int c ) const
{
    listView()->widthChanged( this, c );
}

/*! \fn void QListView::selectionChanged()

  This signal is emitted whenever the set of selected items has
  changed (normally before the screen update).  It is available both
  in single-selection and multi-selection mode, but is most meaningful
  in multi-selection mode.

  Note that you may not delete any QListViewItem objects in slots
  connected to this signal.

  \sa setSelected() QListViewItem::setSelected()
*/


/*! \fn void QListView::selectionChanged( QListViewItem * )

  This signal is emitted whenever the selected item has changed in
  single-selection mode (normally after the screen update).  The
  argument is the newly selected item, or 0 if the change was to
  unselect the selected item.

  There is another signal which is more useful in multi-selection
  mode.

  Note that you may not delete any QListViewItem objects in slots
  connected to this signal.

  \sa setSelected() QListViewItem::setSelected() currentChanged()
*/


/*! \fn void QListView::currentChanged( QListViewItem * )

  This signal is emitted whenever the current item has changed
  (normally after the screen update).  The current item is the item
  responsible for indicating keyboard focus.

  The argument is the newly current item, or 0 if the change was to
  make no item current.  This can happen e.g. if all items in the list
  view are deleted.

  Note that you may not delete any QListViewItem objects in slots
  connected to this signal.

  \sa setCurrentItem() currentItem()
*/


/*!
  Processes mouse move events on behalf of the viewed widget.
*/
void QListView::contentsMousePressEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    QPoint vp = contentsToViewport(e->pos());

    if ( e->button() == RightButton ) {
	QListViewItem * i;
	if ( viewport()->rect().contains( vp ) )
	    i = itemAt( vp );
	else
	    i = d->currentSelected;

	if ( i ) {
	    int c = d->h->mapToLogical( d->h->cellAt( vp.x() ) );
	    emit rightButtonPressed( i, viewport()->mapToGlobal( vp ),
				     c );
	}
	return;
    }

    if ( e->button() != LeftButton )
	return;

    d->ignoreDoubleClick = FALSE;
    d->buttonDown = TRUE;

    QListViewItem * i = itemAt( vp );
    if ( !i )
	return;

    if ( (i->isExpandable() || i->childCount()) &&
	 d->h->mapToLogical( d->h->cellAt( vp.x() ) ) == 0 ) {
	int x1 = vp.x() +
		 d->h->offset() -
		 d->h->cellPos( d->h->mapToActual( 0 ) );
	QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );
	while( it.current() && it.current()->i != i )
	    ++it;

	if ( it.current() ) {
	    x1 -= treeStepSize() * (it.current()->l - 1);
	    if ( x1 >= 0 && ( !i->isSelectable() || x1 < treeStepSize() ) ) {
		setOpen( i, !i->isOpen() );
		if ( !d->currentSelected ) {
		    setCurrentItem( i );
		    // i may have been deleted here
		}
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


/*!
  Processes mouse move events on behalf of the viewed widget.
*/
void QListView::contentsMouseReleaseEvent( QMouseEvent * e )
{
  // delete and disconnect autoscroll timer, if we have one
    if ( d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL(timeout()),
		    this, SLOT(doAutoScroll()) );
        d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

    if ( !e )
	return;

    QPoint vp = contentsToViewport(e->pos());

    if ( e->button() == RightButton ) {
	QListViewItem * i;
	if ( viewport()->rect().contains( vp ) )
	    i = itemAt( vp );
	else
	    i = d->currentSelected;

	if ( i ) {
	    int c = d->h->mapToLogical( d->h->cellAt( vp.x() ) );
	    emit rightButtonClicked( i, viewport()->mapToGlobal( vp ),
				     c );
	}
	return;
    }

    if ( e->button() != LeftButton || !d->buttonDown )
	return;

    QListViewItem * i = itemAt( vp );
    if ( !i )
	return;

    if ( i->isSelectable() )
	setSelected( i, d->select );

    setCurrentItem( i ); // repaints

    return;
}


/*!
  Processes mouse double-click events on behalf of the viewed widget.
*/
void QListView::contentsMouseDoubleClickEvent( QMouseEvent * e )
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

    QPoint vp = contentsToViewport(e->pos());

    QListViewItem * i = itemAt( vp );

    if ( !i )
	return;

    if ( !i->isOpen() ) {
	if ( i->isExpandable() || i->childCount() )
	    setOpen( i, TRUE );
    } else if (i->childItem ) {
	setOpen( i, FALSE );
    }

    emit doubleClicked( i );
}


/*!
  Processes mouse move events on behalf of the viewed widget.
*/
void QListView::contentsMouseMoveEvent( QMouseEvent * e )
{
    if ( !e || !d->buttonDown )
	return;

    bool needAutoScroll = FALSE;

    QPoint vp = contentsToViewport(e->pos());

    // check, if we need to scroll
    if ( vp.y() > visibleHeight() || vp.y() < 0 )
	needAutoScroll = TRUE;
	
    // if we need to scroll and no autoscroll timer is started,
    // connect the timer
    if ( needAutoScroll && !d->scrollTimer ) {
        d->scrollTimer = new QTimer( this );
	connect( d->scrollTimer, SIGNAL(timeout()),
		 this, SLOT(doAutoScroll()) );
	d->scrollTimer->start( 100, FALSE );
	// call it once manually
	doAutoScroll();
    }

    // if we don't need to autoscroll
    if ( !needAutoScroll ) {
        // if there is a autoscroll timer, delete it
        if ( d->scrollTimer ) {
	    disconnect( d->scrollTimer, SIGNAL(timeout()),
			this, SLOT(doAutoScroll()) );
	    d->scrollTimer->stop();
	    delete d->scrollTimer;
	    d->scrollTimer = 0;
        }
	// call this to select an item
        doAutoScroll();
    }
}

void QListView::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );

    // do the scrolling
    if ( pos.y() > visibleHeight() ) {
        scrollBy( 0, verticalScrollBar()->lineStep() );
	pos.setY( visibleHeight() - 1 );
    }
	
    if ( pos.y() < 0 ) {
        scrollBy( 0, -verticalScrollBar()->lineStep() );
	pos.setY( 1 );
    }

    QListViewItem * i = itemAt( pos );
    if ( !i )
	return;

    if ( isMultiSelection() && d->focusItem ) {
	// also (de)select the ones in between
	QListViewItem * b = d->focusItem;
	bool down = ( itemPos( i ) > itemPos( b ) );
	while( b && b != i ) {
	    if ( b->isSelectable() )
		setSelected( b, d->select );
	    b = down ? b->itemBelow() : b->itemAbove();
	}
    }

    if ( i->isSelectable() )
	setSelected( i, d->select );

    setCurrentItem( i );
    ensureItemVisible( i );
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

    QListViewItem* oldCurrent = currentItem();
    if ( !oldCurrent )
        return;

    QListViewItem * i = currentItem();

    if ( isMultiSelection() && i->isSelectable() && e->ascii() == ' ' ) {
        setSelected( i, !i->isSelected() );
        d->currentPrefix.truncate( 0 );
        return;
    }

    QRect r( itemRect( i ) );
    QListViewItem * i2;

    switch( e->key() ) {
    case Key_Enter:
    case Key_Return:
        d->currentPrefix.truncate( 0 );
        if ( i && !i->isSelectable() &&
             ( i->childCount() || i->isExpandable() ) ) {
            i->setOpen( !i->isOpen() );
            return;
        }
        e->ignore();
        emit returnPressed( currentItem() );
        // do NOT accept.  QDialog.
        return;
    case Key_Down:
        i = i->itemBelow();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Up:
        i = i->itemAbove();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Home:
        i = firstChild();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_End:
        i = firstChild();
        while ( i->nextSibling() )
            i = i->nextSibling();
        while ( i->itemBelow() )
            i = i->itemBelow();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Next:
        i2 = itemAt( QPoint( 0, visibleHeight()-1 ) );
        if ( i2 == i || !r.isValid() ||
             visibleHeight() <= itemRect( i ).bottom() ) {
            if ( i2 )
                i = i2;
            int left = visibleHeight();
            while( (i2 = i->itemBelow()) != 0 && left > i2->height() ) {
                left -= i2->height();
                i = i2;
            }
        } else {
            i = i2;
        }
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Prior:
        i2 = itemAt( QPoint( 0, 0 ) );
        if ( i == i2 || !r.isValid() || r.top() <= 0 ) {
            if ( i2 )
                i = i2;
            int left = visibleHeight();
            while( (i2 = i->itemAbove()) != 0 && left > i2->height() ) {
                left -= i2->height();
                i = i2;
            }
        } else {
            i = i2;
        }
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Plus:
        if (  !i->isOpen() && (i->isExpandable() || i->childCount()) )
            setOpen( i, TRUE );
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Right:
        if ( i->isOpen() && i->childItem )
            i = i->childItem;
        else if ( !i->isOpen() && (i->isExpandable() || i->childCount()) )
            setOpen( i, TRUE );
        else if ( contentsX() + visibleWidth() < contentsWidth() )
            horizontalScrollBar()->addLine();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Minus:
        if ( i->isOpen() )
            setOpen( i, FALSE );
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Left:
        if ( i->isOpen() )
            setOpen( i, FALSE );
        else if ( i->parentItem && i->parentItem != d->r )
            i = i->parentItem;
        else if ( contentsX() )
            horizontalScrollBar()->subtractLine();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Space:
        i->activate();
        d->currentPrefix.truncate( 0 );
        break;
    case Key_Escape:
        e->ignore(); // For QDialog
        break;
    default:
        if ( e->ascii() && isalnum( e->ascii() ) ) {
            QString input( d->currentPrefix );
            QListViewItem * keyItem = i;
            QTime now( QTime::currentTime() );
            while( keyItem ) {
                // try twice, first with the previous string and this char
                input += (char)tolower( e->ascii() );
                QString keyItemKey;
                QString prefix;
                while( keyItem ) {
                    // Look for text in column 0, then left-to-right
                    keyItemKey = keyItem->text(0);
                    for (int col=0; col < d->h->count() && !keyItemKey; col++ )
                        keyItemKey = keyItem->text( d->h->mapToLogical(col) );
                    if ( !keyItemKey.isEmpty() ) {
                        prefix = keyItemKey;
                        prefix.truncate( input.length() );
                        prefix = prefix.lower();
                        if ( prefix == input ) {
                            d->currentPrefix = input;
                            d->currentPrefixTime = now;
                            i = keyItem;
                            // nonoptimal double-break...
                            keyItem = 0;
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
        } else {
            e->ignore();
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

    if ( oldCurrent ) {
        QRect r = itemRect( oldCurrent );
        r = r.unite( itemRect( currentItem() ) );
        viewport()->repaint( r.x(), r.y(), r.width(), r.height() );
    }
}


/*!  Returns a pointer to the QListViewItem at \a viewPos.  Note
  that \a viewPos is in the coordinate system of viewport(), not in
  the listview's own, much larger, coordinate system.

  itemAt() returns 0 if there is no such item.

  \sa itemPos() itemRect()
*/

QListViewItem * QListView::itemAt( const QPoint & viewPos ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();
    int g = viewPos.y() + contentsY();

    while( c && c->i && c->y + c->i->height() <= g )
	c = d->drawables->next();

    return (c && c->y <= g) ? c->i : 0;
}


/*!  Returns the y coordinate of \a item in the list view's
  coordinate system.  This functions is normally much slower than
  itemAt(), but it works for all items, while itemAt() normally works
  only for items on the screen.

  This is a thin wrapper around QListViewItem::itemPos().

  \sa itemAt() itemRect()
*/

int QListView::itemPos( const QListViewItem * item )
{
    return item ? item->itemPos() : 0;
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
  TRUE, the currently selected item is unselected and \a item made
  current.  Unlike QListViewItem::setSelected(), this function updates
  the list view as necessary and emits the selectionChanged() signals.

  \sa isSelected() setMultiSelection() isMultiSelection() setCurrentItem()
*/

void QListView::setSelected( QListViewItem * item, bool selected )
{
    if ( !item || item->isSelected() == selected || !item->isSelectable() )
	return;

    if ( d->currentSelected == item && !selected ) {
	d->currentSelected = 0;
	item->setSelected( FALSE );
	repaintItem( item );
    }

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
	emit selectionChanged( selected ? item : 0 );
    emit selectionChanged();
}


/*! Sets all items to be not selected, updates the list view as
necessary and emits the selectionChanged() signals.  Note that for
multi-selection list views, this function needs to iterate over \e all
items.

\sa setSelected(), setMultiSelection()
*/

void QListView::clearSelection()
{
    if ( isMultiSelection() ) {
	bool anything = FALSE;
	QListViewItem * i = firstChild();
	QStack<QListViewItem> s;
	while ( i ) {
	    if ( i->childItem )
		s.push( i->childItem );
	    if ( i->isSelected() ) {
		anything = TRUE;
		setSelected( i, FALSE );
	    }
	    i = i->siblingItem;
	    if ( !i )
		i = s.pop();
	}
	if ( anything ) {
	    if ( !isMultiSelection() )
		emit selectionChanged( i );
	    emit selectionChanged();
	}
    } else if ( d->currentSelected ) {
	QListViewItem * i = d->currentSelected;
	setSelected( i, FALSE );
	if ( !isMultiSelection() )
	    emit selectionChanged( i );
	emit selectionChanged();
    }
}


/*!  Returns \link QListViewItem::isSelected() i->isSelected(). \endlink

  Provided only because QListView provides setSelected() and trolls
  are neat creatures and like neat, orthogonal interfaces.
*/

bool QListView::isSelected( QListViewItem * i ) const
{
    return i ? i->isSelected() : FALSE;
}


/*!  Returns a pointer to the selected item, if the list view is in
single-selection mode and an item is selected.

If no items are selected or the list view is in multi-selection mode
this function returns 0.

\sa setSelected() setMultiSelection()
*/

QListViewItem * QListView::selectedItem() const
{
    return isMultiSelection() ? 0 : d->currentSelected;
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
	if ( i )
	    repaintItem( i );
	if ( prev )
	    repaintItem( prev );
	if ( i )
	    emit currentChanged( i );
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
	     y < ((QListView *)this)->visibleHeight() ) {
	    QRect r( -contentsX(), y, d->h->width(), i->height() );
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

  If \a column is -1, sorting is disabled.
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
    int lcol = d->h->mapToLogical( column );
    setSorting( lcol, d->sortcolumn == lcol ? !d->ascending : TRUE);
}

/*! Sets the advisory item margin which list items may use to \a m.

  The item margin defaults to one pixel and is the margin between the
  item's edges and the area where it draws its contents.
  QListViewItem::paintFocus() draws in the margin.

  \sa QListViewItem::paintCell()
*/
void QListView::setItemMargin( int m )
{
    if ( d->margin == m )
	return;
    d->margin = m;
    if ( isVisibleToTLW() ) {
	if ( d->drawables )
	    d->drawables->clear();
	triggerUpdate();
    }
}

/*! Returns the advisory item margin which list items may use.

  \sa QListViewItem::paintCell() setItemMargin()
*/
int QListView::itemMargin() const
{
    return d->margin;
}


/*! \fn void QListView::rightButtonClicked( QListViewItem *, const QPoint&, int )

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

void QListView::styleChange( GUIStyle /* s */ )
{
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
  and that it will be called for currently invisible items as soon as
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
void QListView::widthChanged( const QListViewItem* item, int c )
{
    if ( c >= d->h->count() )
	return;

    const QFontMetrics & fm = fontMetrics();
    int col = c < 0 ? 0 : c;
    while ( col == c || ( c < 0 && col < d->h->count() ) ) {
	if ( d->column[col]->wmode == Maximum ) {
	    int w = item->width( fm, this, col );
	    if ( col == 0 ) {
		int indent = treeStepSize() * item->depth();
		if ( rootIsDecorated() )
		    indent += treeStepSize();
		w += indent;
	    }
	    if ( w > columnWidth( col ) )
		setColumnWidth( col, w );
	}
	col++;
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
    d->r->enforceSortOrder();
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
    if ( !item )
	return;
    d->dirtyItemTimer->start( 0, TRUE );
    if ( !d->dirtyItems )
	d->dirtyItems = new QPtrDict<void>();
    d->dirtyItems->replace( (void *)item, (void *)item );
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




static QPixmap *defaultIcon = 0;
static const int BoxSize = 16;


/*!
  Constructs a checkable item with parent \a parent, text \a text and type
  \a tt. Note that a RadioButton must be child of a Controller, otherwise
  it will not toggle.
 */
QCheckListItem::QCheckListItem( QCheckListItem *parent, const QString &text,
				Type tt )
    : QListViewItem( parent, text, QString::null )
{
    myType = tt;
    init();
    if ( myType == RadioButton ) {
	if ( parent->type() != Controller )
	    qWarning( "QCheckListItem::QCheckListItem(), radio button must be "
		     "child of a controller" );
	else
	    exclusive = parent;
    }
}

/*!
  Constructs a checkable item with parent \a parent, text \a text and type
  \a tt. Note that this item must not be a a RadioButton. Radio buttons must
  be children on a Controller.
 */
QCheckListItem::QCheckListItem( QListViewItem *parent, const QString &text,
				Type tt )
    : QListViewItem( parent, text, QString::null )
{
    myType = tt;
    if ( myType == RadioButton ) {
      qWarning( "QCheckListItem::QCheckListItem(), radio button must be "
	       "child of a QCheckListItem" );
    }
    init();
}

/*!
  Constructs a checkable item with parent \a parent, text \a text and type
  \a tt. Note that \a tt must not be RadioButton, if so
  it will not toggle.
 */
QCheckListItem::QCheckListItem( QListView *parent, const QString &text,
				Type tt )
    : QListViewItem( parent, text )
{
    myType = tt;
    if ( tt == RadioButton )
	qWarning( "QCheckListItem::QCheckListItem(), radio button must be "
		 "child of a QCheckListItem" );
    init();
}

/*!
  Constructs a Controller item with parent \a parent, text \a text and pixmap
  \a p.
 */
QCheckListItem::QCheckListItem( QListView *parent, const QString &text,
				const QPixmap & p )
    : QListViewItem( parent, text )
{
    myType = Controller;
    setPixmap( 0, p );
    init();
}

/*!
  Constructs a Controller item with parent \a parent, text \a text and pixmap
  \a p.
 */
QCheckListItem::QCheckListItem( QListViewItem *parent, const QString &text,
				const QPixmap & p )
    : QListViewItem( parent, text )
{
    myType = Controller;
    setPixmap( 0, p );
    init();
}

void QCheckListItem::init()
{
    on = FALSE;
    reserved = 0;
    if ( !defaultIcon )
	defaultIcon = new QPixmap( def_item_xpm );
    if ( myType == Controller ) {
	if ( !pixmap(0) )
	    setPixmap( 0, *defaultIcon );
    }
    exclusive = 0;
}


/*! \fn QCheckListItem::Type QCheckListItem::type() const

  Returns the type of this item.
*/

/*! \fn  bool QCheckListItem::isOn() const
  Returns TRUE if this item is toggled on, FALSE otherwise.
*/


/*! \fn QString QCheckListItem::text() const

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
  Toggle checkbox, or set radio button on.
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
  Sets this button on if \a b is TRUE, off otherwise. Maintains radio button
  exclusivity.
 */
void QCheckListItem::setOn( bool b  )
{
    if ( listView() && !listView()->isEnabled() )
        return;

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
    h = QMAX( BoxSize, h );
    setHeight( h );
}


int QCheckListItem::width( const QFontMetrics& fm, const QListView* lv, int column) const
{
    int r = QListViewItem::width( fm, lv, column );
    if ( column == 0 ) {
	r += lv->itemMargin();
	if ( myType == Controller && pixmap( 0 ) ) {
	    //	     r += 0;
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

    if ( myType == Controller ) {
	if ( !pixmap( 0 ) )
	    r += BoxSize + 4;
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
		p->drawLineSegments( a );
	    }
	    ////////////////////////
	} else { //radio button look
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
  Draws the focus rectangle
*/
void QCheckListItem::paintFocus( QPainter *p, const QColorGroup & cg,
				 const QRect & r )
{
    if ( myType != Controller ) {
        QRect rect( r.x() + BoxSize + 5, r.y(), r.width() - BoxSize - 5,r.height() );
	QListViewItem::paintFocus(p, cg, rect);
    } else {
      QListViewItem::paintFocus(p, cg, r);
    }
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
  QHeader's sizeHint() recommends and tall enough for perhaps 10 items.
*/

QSize QListView::sizeHint() const
{
    if ( !isVisibleToTLW() &&
	 (!d->drawables || d->drawables->isEmpty()) )
	// force the column widths to sanity, if possible
	buildDrawableList();

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


/*!  Sets \a item to be open if \a open is TRUE and \a item is
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


/*!  Ensures that \a i is made visible, scrolling the list view
  vertically as required.

  \sa itemRect() QSCrollView::ensureVisible()
*/

void QListView::ensureItemVisible( const QListViewItem * i )
{
    if ( !i )
	return;
    if ( d->r->maybeTotalHeight < 0 )
	updateGeometries();
    int h = (i->height()+1)/2;
    ensureVisible( contentsX(), itemPos( i )+h, 0, h );
}


/*! \base64 listview.png

iVBORw0KGgoAAAANSUhEUgAAAgYAAABSBAMAAADdkQFbAAAAGFBMVEUAAACZmZnc3NzMzMzA
wMCAgID//////wCGUuu/AAAGRklEQVR4nO1au47cOgxVYbAfXCCfkD7Vti5m+zSZWgES9Rf7
/8A1X3pYkl/SjPdizdnMONaD5DEp8dg290vM2QZ8ArkwuDBAmTD49bXlgRj8NF9avhMGrqv8
PdSE8nimbu2RZQJj8Kdrdr3Xp1toQnk8U7f2yM5cGFwYoFwYXBigKAbjHf/6CNphTHG+qWna
jIbayAcO5NZqn3XdOHZA7YOYkFiygMHQFYOx4gSauKDngcO4vTh8k+77OKr7OzEYp56jwd8B
f1vknTCY/mEwmHkT4zOMY0HPg6/h3QxTUz1clnWTL1M4TYJKpg9Np6RgBQM0bTD0e0B9Yoe5
c3rNpiITGYOhoOehcTzy1zHd5Pd0BbwS/0U9sjFhPdAMKtm23467EV+rGOQAhTjogkFQIl/S
IxsTMKAIYgwWVq2tdsjVnE+FTaO0FfT4XMBraI5kJCrAaWcY4Kn1XOCVQOOgSd7/eD/nU6GJ
411Wn1xPmgvHdY95HNzvG3IB86dbLjAGw72SC4hBdT0QfA7nQowB+bN9PaB9YWTlzfsC1wfV
fQE9rewLlB/Ton58XyBXzDjo5jMgClv2hZ7yXp9OmyrVyFeqlWtV6VfCYLgwqMqFwbkYnH1n
96VSwaD5Vub/WhSD2+3b2aacJoLB77e3H2ebcpoIBv98fHzcbrdnaADnDDhr+HA6MHzAYme9
sZ8BPaTO6STS6EAaLY8JbX5MNj24sggGnAi/a70aBE2fPoBGA1sVa5ljMPWzcpIO55Noo5G5
LCEK0UBbhTjWG4ti8IHyb61Xi7D5ZKTaPZlr5boawMtt1TX2J8PAZRiQQzKfeBdjoEHDnUlF
PDgVxeAN5Qd09T42n0wTmxED8D6B/k96S7wwBlHkSzCFM9KZ5py+jQsY+MmNnwk1rWDw9DgA
MstjID6Fi1uMAxdOO4PLQxIHdMQhRmcEA6NopFm1jsGz48CKTwGDyVSIMADtTSeNgfjQR30R
Az2jccCTC2Ayk2BQdPBVcQB4IXUdV3MDBpwnLl8To+yvYgBJb86F0kzgtaTyqjiAyBMIuQCb
MSisiTSkgkG2HmzAQOMAihnTIkAbWYhoqg+s7veW9oWwiaX1QbwCZvUB/+FMFPUmbJiyL9h4
XyAMys4JBjcW6Oz/dukO/Q4tgoHlt1LgJZaU5DWay1o+CW88Mww+CwanysWdL+6MUufOlaJq
r+ieZWNqG2YucWebjkSB0t5Ih8rLn8Sda2N2idYuMaWJmkvc2aQjXYU7CxWz3bmzwIkzWlMZ
s1O4KLQBgzXuHA5XuLMUy4qBCxg0cWeICk3oAgE7b21032ONOwcMhEYvcWcuNwPp7sGdwfXF
QMyfxYGSiBJ3jjCAcNUr3DmPg2bubKm259DtggHnrARDjEGdOzMF0IY6d4YyBq6VO0OUvl0w
AMbA6CVc587JerDAnWU1KK2JbdxZR0JfDHRZ2MKdyxjka+IiBi3cmRgnsCVd9gWMxbw+WOTO
G+sDziaZ5uLObVou7vxpeOPFnU+Wiztf3Bnl6dzZEMObUdswcwt35mnknvU6d646VOfOHWsk
U6O2TdxZKBIoFCUF6fQVh4rcWSs44g2NGyaoJWmt3IM7U9UHMQbi9ow7c/FvLalbwCCplY3/
iVjtQTEmojIBA0hqZahhsMidyX1gjhd657UyYwBV4ljgzrNKvnnvZqcgxcC1c2c/3ilfgBp3
tovkucCdZxjES8xxDIQ19eTOSpUCBiHgZtzZY1B0psKdAwatuSBOQYaBcxEGR7gzPcc2BGWK
Qc6dJ5E4KEltPbDy8wwMPLZRtB3gzn5vLGGQcGe9g7NxPRDGaRE4IFZryqR7q8iGrxS6I3d2
6vw6d5bP8r6QcufXMJhETlDppcCdW6P/kMDrVXop8cYTbiOcGQYXd3YXd0a5uHORO0st0ktg
9zvbW/fG+XNneYm723PnbhgceGd7I3f2jy1CjeTmCnZhEHFn66i0pEIDWslCIEuw+Z3trdzZ
CTMW32IM2p87GzKSKyvohIHWwx25swYXSKVpfZ1YeM4E2587B7NCmHXBQLiuYuDaubPT6248
eahx533PnenZqGCglKwHBlaq+4BB5+fOkhdl7rz3ubOY5ZO4Dwaw/Z3tZD1Y4M7itV8TXZ07
rz1z/Q9Umf2cul0e4gAAAABJRU5ErkJggg==
*/

/*! \fn QString QCheckListItem::text( int n ) const

  \reimp
*/

/*! \base64 treeview.png

iVBORw0KGgoAAAANSUhEUgAAAOMAAAEFAgMAAABsirj0AAAADFBMVEUAAACDgYPFwsX////f
g3KtAAAFA0lEQVR4nO2azarjNhSAzc1muBT8Cl2GrC5kMRuDniIU+h7dtmVWlzzF4JWbpzB4
e0GrbvoUBlECRUWVLNuJpHOOjlOmJdiahRlb39WP9fnYOikMXX4Bzv1xsaUpaPBviPzRk39d
qPLbz8DJ5yTPDUn+WqDkF5q8HFDyvWle3+vmFSXP5+8vLzBZ118+1TVBvu4biDzvL/V5T5IH
hHxvXs77ry84eTjsC5isXZsgOJC1JYOTM/m1pntbH7BxNs2n856Y2+aCzC3czRt5aeJbyibr
h8nzw2RSRvL3E1l+As6NJA2CZSTN1f4zRiAPIj1ekckFS4qr0eJNqxImZSV3uty1+kPH5O5q
lCyrXmFkpZWSwh5isvh8UkZUCiW1VsaSOiVdb0my8KSKydLOkCRJM5DmGJPGjxMjXTeHcSbk
n6cf/NzCZGEqY+e2a3V4Z3QmrgQlIa9cUjxK6vB/S9pcK+lWArgIpgrCHUE/3erDyYzZ7YeC
xc6b3X0Hi503WyKqZM3OkITZGZI0G3mAZc3ubZvI3ObNxm/pXBCzGaR4lNzMXky6lUBWEO6I
mU2QlNnFZ6NUr5NrI0mY3doVb5e8QEnU7PZ6slc0CNJmq+vQW4LEzB5I6xJOUmZ3EiUzMfuI
9naL2SO5GH9e0r6qyvTderjhblqlcFXkNMX3pNa67GPSmz2Qxj+UQdKkbWp7WlW606UUO9fm
26hMMM4jQJpS676rKuf+4ExZyYg8nSwHkL1tUw5Phcp42wSzTeObI0lknOIYkXFvafI2TvvC
n5DY/ZQWGue237nnUTpOpCAPYgaZdINL4g+oJ7Xsvyb9K7lbCW3oKBWzB9IFF2+2XTH3taiY
7UkbuP26tWRQgYjZnrSB21sm1YeISCxmT+Rodlt2MiKxmO1JNbcpZUIiMXsm53EmJBKz7+YW
JPGYHd1PqeL7icds9vKBSQ5+35dFJBKz/6UrS/AnJv1ykHEF4Y6on/MSjD+VsmZ70n5xq1K+
hWTGbE/aL+5eyVJEJGn29Ehx7+UiJkmzR3L4FkhJymxPlghJmT3PbZ+QGbNv97OMyKzZnOVD
kBn8vi+LSMrsp7Ls/zI7FpRvdkKyzbZfhaINSK7Z9ks0WLiLzJbChCTb7JTkmp2SLLPTcXLN
TuZ2M3uF5LRLPn0bjxWEMSyzY5Ifsz9Ea5dpL9uJXGC2SynZwC1mkmm2VUUPqt1IptkTWdyR
PLPnNm/jZJot3Ti7u3GyY7aUrbZ6z3O7mb1C0q8EEVcYTnDexiOSb3b4Mr7E7PBlfJHZIiG5
ZgMk02yAZJodj5NvdjS3m9krJKH817KYHVTgm23X2n36dcF3dlfp+5TvIrOD33IsMrtISKbZ
Vdom0+yY5JstdkcVVPi2ZmeKeJT8Njto6yBt0N2l2Stm1mt4ywz/NDPr5cioU8ysl60Q7Iwb
dtbLttlFjTKzXuk4mVkveIY4WS+czGa9wPu5Zb3WRELb4lMF4Y5k1gv7BaShSZFui49kLusF
bIvPJJ31ArbFZ5LOegHb4jeSzHoB2+I3Mpf1irfFR5KT9Sohcst6rYv02+IyfUSyzHZqpyTH
bH2ULfAbF4bZw+ZZ2ibTbKC3TLNhkmM2TObMRsbJMNvNrRYxuZm9LhJKeE0VhDtmzU5Jntlh
UmYkeWanIYlrtoxBttkIyTAbIRlmQ+NkmQ3N7Wb2uki/efbo2zhMssy2+gIkZvY/t3Mngm89
4IgAAAAASUVORK5CYII=
*/


/*!  Returns a pointer to the QHeader object that manages this list
  view's columns.  Please don't modify the header behind the list
  view's back.
*/

QHeader * QListView::header() const
{
    return d->h;
}


/*!  Returns the current number of parentless QListViewItem objects in
  this QListView, like QListViewItem::childCount() returns the number
  of child items for a QListViewItem.

  \sa QListViewItem::childCount()
*/

int QListView::childCount() const
{
    return d->r->childCount();
}


/*!  Moves this item to just after \a olderSibling.  \a olderSibling
  and this object must have the same parent.
*/

void QListViewItem::moveToJustAfter( QListViewItem * olderSibling )
{
    if ( parentItem && olderSibling &&
	 olderSibling->parentItem == parentItem ) {
	if ( parentItem->childItem == this ) {
	    parentItem->childItem = siblingItem;
	} else {
	    QListViewItem * i = parentItem->childItem;
	    while( i && i->siblingItem != this )
		i = i->siblingItem;
	    if ( i )
		i->siblingItem = siblingItem;
	}
	siblingItem = olderSibling->siblingItem;
	olderSibling->siblingItem = this;
    }
}


/*!  \reimp */

void QListView::showEvent( QShowEvent * )
{
    if ( d->drawables )
	d->drawables->clear();
    delete d->dirtyItems;
    d->dirtyItems = 0;
    d->dirtyItemTimer->stop();

    updateGeometries();
}


/*!  Returns the y coordinate of \a item in the list view's
  coordinate system.  This functions is normally much slower than
  QListView::itemAt(), but it works for all items, while
  QListView::itemAt() normally works only for items on the screen.

  \sa QListView::itemAt() QListView::itemRect() QListView::itemPos()
*/

int QListViewItem::itemPos() const
{
    QStack<QListViewItem> s;
    QListViewItem * i = (QListViewItem *)this;
    while( i ) {
	s.push( i );
	i = i->parentItem;
    }

    int a = 0;
    QListViewItem * p = 0;
    while( s.count() ) {
	i = s.pop();
	if ( p ) {
	    if ( !p->configured ) {
		p->configured = TRUE;
		p->setup(); // ### virtual non-const function called in const
	    }
	    a += p->height();
	    QListViewItem * s = p->firstChild();
	    while( s && s != i ) {
		a += s->totalHeight();
		s = s->nextSibling();
	    }
	}
	p = i;
    }
    return a;
}


/*!\obsolete
  Removes \a i from the list view; \a i must be a top-level item.
  The warnings regarding QListViewItem::removeItem( i ) apply to this
  function too.

  \sa QListViewItem::removeItem() (important) insertItem()
*/

void QListView::removeItem( QListViewItem * i )
{
    d->r->removeItem( i );
}

/*!  Removes \a i from the list view; \a i must be a top-level item.
  The warnings regarding QListViewItem::takeItem( i ) apply to this
  function too.

  \sa QListViewItem::takeItem() (important) insertItem()
*/
void QListView::takeItem( QListViewItem * i )
{
    removeItem( i );
}


/**********************************************************************
 *
 * Class QListViewItemIterator
 *
 **********************************************************************/


/*!
  \class QListViewItemIterator qlistview.h
  \brief The QListViewItemIterator class provides an iterator for collections of
  QListViewItems (QListViews)

  Construct an instance of a QListViewItemIterator with either a QListView* or
  a QListViewItem* as argument, to operate on the tree of QListViewItems.

  Example:

  Often you want to get all items, which were selected by a user. Here is
  an example which does this and stores the pointers to all selected items
  in a QList.

  \code

  \/ Somewhere a listview is generated like this
  QListView *lv = new QListView(this);
  \/ Enable multiselection
  lv->setMultiSelection( TRUE );

  \/ Insert the items here

  * * * *

  \/ This function is called to get a list of the selected items of a listview
  QList<QListViewItem> * getSelectedItems( QListView *lv ) {
    if ( !lv )
      return 0;

    \/ Create the list
    QList<QListViewItem> *lst = new QList<QListViewItem>;
    lst->setAutoDelete( FALSE );

    \/ Create an iterator and give the listview as argument
    QListViewItemIterator it( lv );
    \/ iterate through all items of the listview
    for ( ; it->current(); ++it ) {
      if ( it->current()->isSelected() )
        lst->append( it->current() );
    }

    return lst;
  }

  \endcode

  Using a QListViewItemIterator is a convinient way to traverse the tree
  of QListViewItems of a QListView. It makes especially operating on a
  hirarchical QListView easy.
  Also multiple QListViewItemIterators can operate on the tree of
  QListViewItems. A QListView knows about all iterators which are operating
  on its QListViewItems. So when a QListViewItem gets removed, all iterators
  that point to this item get updated and point to the new current item after
  that.

  \sa QListView, QListViewItem
*/

/*!
  \fn QListViewItemIterator::QListViewItemIterator()
  Default constructor. Constructs and empty iterator.
*/

QListViewItemIterator::QListViewItemIterator()
    : curr( 0L ), listView( 0L )
{
}

/*!
  \fn QListViewItemIterator::QListViewItemIterator(QListViewItem *item)
  Constructs an iterator for the QListView of the \e item. The current iterator item is
  set to point on the \e item.
*/

QListViewItemIterator::QListViewItemIterator( QListViewItem *item )
    : curr( item ), listView( item ? item->listView() : 0L )
{
    addToListView();
}

/*!
  \fn QListViewItemIterator::QListViewItemIterator(const QListViewItemIterator& it)
  Constructs an iterator for the same QListView as \e it. The current iterator item is
  set to point on the current item of \e it.
*/

QListViewItemIterator::QListViewItemIterator( const QListViewItemIterator& it )
    : curr( it.curr ), listView( it.listView )
{
    addToListView();
}

/*!
  \fn QListViewItemIterator::QListViewItemIterator( QListView *lv )
  Constructs an iterator for the QListView \e lv. The current iterator item is
  set to point on the first child ( QListViewItem ) of \e lv.
*/

QListViewItemIterator::QListViewItemIterator( QListView *lv )
    : curr( lv->firstChild() ), listView( lv )
{
    addToListView();
}

/*!
  \fn QListViewItemIterator &QListViewItemIterator::operator=( const QListViewItemIterator &it )
  Assignment. Makes a copy \e it and returns a reference to its iterator.
*/

QListViewItemIterator &QListViewItemIterator::operator=( const QListViewItemIterator &it )
{
    if ( listView ) {
        if ( listView->d->iterators->removeRef( this ) ) {
            if ( listView->d->iterators->count() == 0 ) {
                delete listView->d->iterators;
                listView->d->iterators = 0L;
            }
        }
    }

    listView = it.listView;
    addToListView();
    curr = it.curr;

    return *this;
}

/*!
  \fn QListViewItemIterator::~QListViewItemIterator()
  Destroys the iterator.
*/

QListViewItemIterator::~QListViewItemIterator()
{
    if ( listView ) {
        if ( listView->d->iterators->removeRef( this ) ) {
            if ( listView->d->iterators->count() == 0 ) {
                delete listView->d->iterators;
                listView->d->iterators = 0L;
            }
        }
    }
}

/*!
  \fn QListViewItemIterator& QListViewItemIterator::operator++()
  Prefix ++ makes the next item in the QListViewItem tree of the QListView
  of the iterator the current item and returns it. If the current item was the last item
  in the QListView or null, null is returned.
*/

QListViewItemIterator &QListViewItemIterator::operator++()
{
    if ( !curr )
        return *this;

    QListViewItem *item = curr->firstChild();
    if ( item ) {
        curr = item;
        return *this;
    }

    item = curr->nextSibling();
    if ( item ) {
        curr = item;
        return *this;
    }

    curr = curr->parent();
    if ( curr )
        curr = curr->nextSibling();

    return *this;
}

/*!
  \fn const QListViewItemIterator QListViewItemIterator::operator++( int )
  Postfix ++ makes the next item in the QListViewItem tree of the QListView
  of the iterator the current item and returns the item, which was the current one before.
*/

const QListViewItemIterator QListViewItemIterator::operator++( int )
{
    QListViewItemIterator oldValue = *this;
    ++( *this );
    return oldValue;
}

/*!
  \fn QListViewItemIterator &QListViewItemIterator::operator+=( int j )
  Sets the current item to the item \e j positions after the current item
  in the QListViewItem hirarchie. If this item is beyond the last item, the
  current item is set to null.
  The new current item (or null, if the new current item is null) is returned.
*/

QListViewItemIterator &QListViewItemIterator::operator+=( int j )
{
    while ( curr && j-- )
        ++( *this );

    return *this;
}

/*!
  \fn QListViewItemIterator& QListViewItemIterator::operator--()
  Prefix -- makes the previous item in the QListViewItem tree of the QListView
  of the iterator the current item and returns it. If the current item was the last first
  in the QListView or null, null is returned.
*/

QListViewItemIterator &QListViewItemIterator::operator--()
{
    if ( !curr )
        return *this;

    if ( !curr->parent() ) {
        // we are in the first depth
       if ( curr->listView() ) {
            if ( curr->listView()->firstChild() != curr ) {
                // go the previous sibling
                QListViewItem *i = curr->listView()->firstChild();
                while ( i && i->siblingItem != curr )
                    i = i->siblingItem;

                curr = i;

                if ( i && i->firstChild() ) {
                    // go to the last child of this item
                    QListViewItemIterator it( curr->firstChild() );
                    for ( ; it.current() && it.current()->parent(); ++it )
                        curr = it.current();
                }

                return *this;
            } else {
                // we are already the first child of the listview, so it's over
                curr = 0L;
                return *this;
            }
        } else
            return *this;
    } else {
        QListViewItem *parent = curr->parent();

        if ( curr != parent->firstChild() ) {
            // go to the previous sibling
            QListViewItem *i = parent->firstChild();
            while ( i && i->siblingItem != curr )
                i = i->siblingItem;

            curr = i;

            if ( i && i->firstChild() ) {
                // go to the last child of this item
                QListViewItemIterator it( curr->firstChild() );
                for ( ; it.current() && it.current()->parent() != parent; ++it )
                    curr = it.current();
            }

            return *this;
        } else {
            // make our parent the current item
            curr = parent;
            return *this;
        }
    }
}

/*!
  \fn const QListViewItemIterator QListViewItemIterator::operator--( int )
  Postfix -- makes the previous item in the QListViewItem tree of the QListView
  of the iterator the current item and returns the item, which was the current one before.
*/

const QListViewItemIterator QListViewItemIterator::operator--( int )
{
    QListViewItemIterator oldValue = *this;
    --( *this );
    return oldValue;
}

/*!
  \fn QListViewItemIterator &QListViewItemIterator::operator-=( int j )
  Sets the current item to the item \e j positions before the current item
  in the QListViewItem hirarchie. If this item is above the first item, the
  current item is set to null.
  The new current item (or null, if the new current item is null) is returned.
*/

QListViewItemIterator &QListViewItemIterator::operator-=( int j )
{
    while ( curr && j-- )
        --( *this );

    return *this;
}

/*!
  \fn QListViewItem *QListViewItemIterator::current() const
  Returns a pointer to the current item of the iterator.
*/

QListViewItem *QListViewItemIterator::current() const
{
    return curr;
}

/*!
  \fn void QListViewItemIterator::addToListView()
  Adds the iterator to the list of iterators of the iterator's QListViewItem.
*/

void QListViewItemIterator::addToListView()
{
    if ( listView ) {
        if ( !listView->d->iterators ) {
            listView->d->iterators = new QList<QListViewItemIterator>;
            CHECK_PTR( listView->d->iterators );
        }
        listView->d->iterators->append( this );
    }
}

/*!
  \fn void QListViewItemIterator::currentRemoved()
  This methode is called to notify the iterator that the current item gets deleted,
  and lets the current item point to another (valid) item.
*/

void QListViewItemIterator::currentRemoved()
{
    if ( !curr ) return;

    if ( curr->parent() )
        curr = curr->parent();
    else if ( curr->nextSibling() )
        curr = curr->nextSibling();
    else if ( listView && listView->firstChild() && listView->firstChild() != curr )
        curr = listView->firstChild();
    else
        curr = 0L;
}

