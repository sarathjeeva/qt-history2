/****************************************************************************
** $Id: qt/src/widgets/qiconview.cpp   2.1.1   edited 2000-04-24 $
**
** Implementation of QIconView widget class
**
** Created : 990707
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the iconview module of the Qt GUI Toolkit.
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

#include "qiconview.h"

#ifndef QT_NO_ICONVIEW

#include "qfontmetrics.h"
#include "qpainter.h"
#include "qevent.h"
#include "qpalette.h"
#include "qmime.h"
#include "qimage.h"
#include "qpen.h"
#include "qbrush.h"
#include "qtimer.h"
#include "qcursor.h"
#include "qkeycode.h"
#include "qapplication.h"
#include "qtextedit.h"
#include "qarray.h"
#include "qlist.h"
#include "qvbox.h"
#include "qtooltip.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qptrdict.h"
#include "qstringlist.h"
#include "qcleanuphandler.h"

#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define RECT_EXTENSION 300

static const char * const unknown_xpm[] = {
    "32 32 11 1",
    "c c #ffffff",
    "g c #c0c0c0",
    "a c #c0ffc0",
    "h c #a0a0a4",
    "d c #585858",
    "f c #303030",
    "i c #400000",
    "b c #00c000",
    "e c #000000",
    "# c #000000",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebaaaaaaa#.........",
    ".#cccccgcgghhebbbbaaaaa#........",
    ".#ccccccgcgggdebbbbbbaa#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "....##ddhhhghhhhhhhhhhd#b#......",
    "......##ddhhhhhhhhhhhhd#b#......",
    "........##ddhhhhhhhhhhd#b#......",
    "..........##ddhhhhhhhhd#b#......",
    "............##ddhhhhhhd#b###....",
    "..............##ddhhhhd#b#####..",
    "................##ddhhd#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};

static QPixmap *unknown_icon = 0;
static QPixmap *qiv_buffer_pixmap = 0;
static QPixmap *qiv_selection = 0;

static QCleanupHandler<QPixmap> qiv_cleanup_pixmap;

#if !defined(Q_WS_X11)
static void createSelectionPixmap( const QColorGroup &cg )
{
    qiv_selection = new QPixmap( 2, 2 );
    qiv_cleanup_pixmap.add( qiv_selection );
    qiv_selection->fill( Qt::color0 );
    QBitmap m( 2, 2 );
    m.fill( Qt::color1 );
    QPainter p( &m );
    p.setPen( Qt::color0 );
    for ( int j = 0; j < 2; ++j ) {
	p.drawPoint( j % 2, j );
    }
    p.end();
    qiv_selection->setMask( m );
    qiv_selection->fill( cg.highlight() );
}
#endif

static QPixmap *get_qiv_buffer_pixmap( const QSize &s )
{
    if ( !qiv_buffer_pixmap ) {
	qiv_buffer_pixmap = new QPixmap( s );
	qiv_cleanup_pixmap.add( qiv_buffer_pixmap );
	return qiv_buffer_pixmap;
    }

    qiv_buffer_pixmap->resize( s );
    return qiv_buffer_pixmap;
}



/*****************************************************************************
 *
 * Struct QIconViewPrivate
 *
 *****************************************************************************/

#ifndef QT_NO_DRAGANDDROP

class QIconDragData
{
public:
    QIconDragData();
    QIconDragData( const QRect &ir, const QRect &tr );

    QRect pixmapRect() const;
    QRect textRect() const;

    void setPixmapRect( const QRect &r );
    void setTextRect( const QRect &r );

    QRect iconRect_, textRect_;
    QString key_;

    bool operator==( const QIconDragData &i ) const;
};

class QIconDragDataItem
{
public:
    QIconDragDataItem() {}
    QIconDragDataItem( const QIconDragItem &i1, const QIconDragData &i2 ) : data( i1 ), item( i2 ) {}
    QIconDragItem data;
    QIconDragData item;
    bool operator== ( const QIconDragDataItem& ) const;
};

struct QIconDragPrivate
{
    QValueList<QIconDragDataItem> items;
    static bool decode( QMimeSource* e, QValueList<QIconDragDataItem> &lst );
};

#endif

class QIconViewToolTip;

class QIconViewPrivate
{
public:
    QIconViewItem *firstItem, *lastItem;
    uint count;
    bool mousePressed;
    QIconView::SelectionMode selectionMode;
    QIconViewItem *currentItem, *tmpCurrentItem, *highlightedItem, *startDragItem, *pressedItem, *selectAnchor;
    QRect *rubber;
    QTimer *scrollTimer, *adjustTimer, *updateTimer, *inputTimer,
	*fullRedrawTimer;
    int rastX, rastY, spacing;
    bool cleared, dropped, clearing;
    int dragItems;
    QPoint oldDragPos;
    QIconView::Arrangement arrangement;
    QIconView::ResizeMode resizeMode;
    QSize oldSize;
#ifndef QT_NO_DRAGANDDROP
    QValueList<QIconDragDataItem> iconDragData;
#endif
    bool isIconDrag;
    int numDragItems, cachedW, cachedH;
    int maxItemWidth, maxItemTextLength;
    QPoint dragStart;
    bool drawDragShapes;
    QString currInputString;
    bool dirty, rearrangeEnabled;
    QIconView::ItemTextPos itemTextPos;
    bool reorderItemsWhenInsert;
#ifndef QT_NO_CURSOR
    QCursor oldCursor;
#endif
    bool resortItemsWhenInsert, sortDirection;
    bool wordWrapIconText;
    int cachedContentsX, cachedContentsY;
    QBrush itemTextBrush;
    bool drawAllBack;
    QRegion clipRegion;
    QPoint dragStartPos;
    QFontMetrics *fm;
    int minLeftBearing, minRightBearing;
    bool containerUpdateLocked;
    bool firstSizeHint;
    QIconViewToolTip *toolTip;
    bool showTips;
    QPixmapCache maskCache;
    bool pressedSelected;
    bool dragging;
    QPtrDict<QIconViewItem> selectedItems;

    struct ItemContainer {
	ItemContainer( ItemContainer *pr, ItemContainer *nx, const QRect &r )
	    : p( pr ), n( nx ), rect( r ) {
		items.setAutoDelete( FALSE );
		if ( p )
		    p->n = this;
		if ( n )
		    n->p = this;
	}
	ItemContainer *p, *n;
	QRect rect;
	QList<QIconViewItem> items;
    } *firstContainer, *lastContainer;

    struct SortableItem {
	QIconViewItem *item;
    };
};

/*****************************************************************************
 *
 * Class QIconViewToolTip
 *
 *****************************************************************************/

class QIconViewToolTip : public QToolTip
{
public:
    QIconViewToolTip( QWidget *parent, QIconView *iv );

    void maybeTip( const QPoint &pos );

private:
    QIconView *view;

};

QIconViewToolTip::QIconViewToolTip( QWidget *parent, QIconView *iv )
    : QToolTip( parent ), view( iv )
{
}

void QIconViewToolTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget() || !view || view->wordWrapIconText() || !view->showToolTips() )
	return;

    QIconViewItem *item = view->findItem( view->viewportToContents( pos ) );
    if ( !item || item->tmpText == item->itemText )
	return;

    QRect r( item->textRect( FALSE ) );
    r = QRect( view->contentsToViewport( QPoint( r.x(), r.y() ) ), QSize( r.width(), r.height() ) );
    QRect r2( item->pixmapRect( FALSE ) );
    r2 = QRect( view->contentsToViewport( QPoint( r2.x(), r2.y() ) ), QSize( r2.width(), r2.height() ) );
    tip( r, r2, item->itemText );
}

/*****************************************************************************
 *
 * Struct QIconViewItemPrivate
 *
 *****************************************************************************/

struct QIconViewItemPrivate
{
    QIconViewPrivate::ItemContainer *container1, *container2;
};

/*****************************************************************************
 *
 * Class QIconViewItemLineEdit
 *
 *****************************************************************************/

class QIconViewItemLineEdit : public QTextEdit
{
    friend class QIconViewItem;

    Q_OBJECT

public:
    QIconViewItemLineEdit( const QString &text, QWidget *parent, QIconViewItem *theItem, const char *name = 0 );

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusOutEvent( QFocusEvent *e );

protected:
    QIconViewItem *item;
    QString startText;

};

QIconViewItemLineEdit::QIconViewItemLineEdit( const QString &text, QWidget *parent,
					      QIconViewItem *theItem, const char *name )
    : QTextEdit( parent, name ), item( theItem ), startText( text )
{
    setFrameStyle( QFrame::Plain | QFrame::Box );
    setLineWidth( 1 );

    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( AlwaysOff );

    setWordWrap( FixedPixelWidth );
    setWrapColumnOrWidth( item->iconView()->maxItemWidth() -
			  ( item->iconView()->itemTextPos() == QIconView::Bottom ?
			    0 : item->pixmapRect().width() ) );
    resize( 200, 200 ); // ### some size, there should be a forceReformat()
    setText( text );
    setAlignment( Qt::AlignCenter );

    resize( wrapColumnOrWidth() + 6, heightForWidth( wrapColumnOrWidth() ) + 6 );
}

void QIconViewItemLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key()  == Key_Escape ) {
	item->QIconViewItem::setText( startText );
	item->cancelRenameItem();
    } else if ( e->key() == Key_Enter ||
		e->key() == Key_Return ) {
	item->renameItem();
    } else {
	QTextEdit::keyPressEvent( e );
	resize( wrapColumnOrWidth() + 6, heightForWidth( wrapColumnOrWidth() ) + 6 );
    }
}

void QIconViewItemLineEdit::focusOutEvent( QFocusEvent * )
{
    item->cancelRenameItem();
}

#ifndef QT_NO_DRAGANDDROP

/*****************************************************************************
 *
 * Class QIconDragItem
 *
 *****************************************************************************/

/*!
  \class QIconDragItem qiconview.h

  \brief The QIconDragItem class is an internal class used by the
  QIconDrag class to encapsulate a drag item.

  \module iconview

    The QIconDragItem class is an internal class that is used to
    encapsulate drag items. The QIconDrag class uses a list of
    QIconDragItems to support drag and drop operations.

    In practice a QIconDragItem object (or an object of a class derived
    from QIconDragItem) is created for each icon view item which is
    dragged. Each of these QIconDragItems is stored in a QIconDrag
    object.

    See QIconView::dragObject() for more information.

    For examples see \c qt/examples/fileiconview (especially
    \c qfileiconview.h and \c qfileiconview.cpp) and
    \c qt/examples/iconview/simple_dd.
*/

/*!
  Constructs a QIconDragItem with no data.
*/

QIconDragItem::QIconDragItem()
    : ba( strlen( "no data" ) )
{
    memcpy( ba.data(), "no data", strlen( "no data" ) );
}

/*!
  Destructor.
*/

QIconDragItem::~QIconDragItem()
{
}

/*!
  Returns the data contained in the QIconDragItem.
*/

QByteArray QIconDragItem::data() const
{
    return ba;
}

/*!
  Sets the data for the QIconDragItem.
*/

void QIconDragItem::setData( const QByteArray &d )
{
    ba = d;
}

/*! \reimp */

bool QIconDragItem::operator==( const QIconDragItem &i ) const
{
    return ba == i.ba;
}

/*!
  \reimp
*/

bool QIconDragDataItem::operator==( const QIconDragDataItem &i ) const
{
    return ( i.item == item &&
	     i.data == data );
}

/*!
  \reimp
*/

bool QIconDragData::operator==( const QIconDragData &i ) const
{
    return key_ == i.key_;
}

/*****************************************************************************
 *
 * Class QIconDrag
 *
 *****************************************************************************/

/*!
  \class QIconDrag qiconview.h

  \brief The QIconDrag class is used to support drag and drop operations
  within a QIconView.

  \ingroup draganddrop

  \module iconview

    A QIconDrag object is used to maintain information about the
    positions of dragged items and data associated with the dragged
    items. QIconViews are able to use this information to paint the
    dragged items in the correct positions. Internally QIconDrag stores
    the data associated with drag items in QIconDragItem objects.

    If you want to use the extended drag-and-drop functionality of
    QIconView create a QIconDrag object in a reimplementation of
    QIconView::dragObject(). Then create a QIconDragItem for each item
    which should be dragged, set the data it represents with
    QIconDragItem::setData(), and add each QIconDragItem to the drag
    object using append().

    The data in QIconDragItems is stored in a QByteArray and is
    mime-typed (see QMimeSource and the
    <a href="http://doc.trolltech.com/dnd.html">Drag and Drop</a>
    overview). If you want to use your own mime-types derive a class
    from QIconDrag and reimplement format(), encodedData() and
    canDecode().

    The fileiconview example program demonstrates the use of the
    QIconDrag class including subclassing and reimplementing
    dragObject(), format(), encodedData() and canDecode(). See the files
    \c qt/examples/fileiconview/qfileiconview.h and
    \c qt/examples/fileiconview/qfileiconview.cpp.

    \sa QMimeSource::format()
*/
// ### consider using \dontinclude and friends there
// ### Not here in the module overview instead...

/*!
  \reimp
*/

QIconDrag::QIconDrag( QWidget * dragSource, const char* name )
    : QDragObject( dragSource, name )
{
    d = new QIconDragPrivate;
}

/*!
  Destructor.
*/

QIconDrag::~QIconDrag()
{
    delete d;
}

/*!
  Append the QIconDragItem, \a i, to the QIconDrag object's list of items.
  You must also supply the geometry of the pixmap, \a pr, and of the
  textual caption, \a tr.

  \sa QIconDragItem
*/

void QIconDrag::append( const QIconDragItem &i, const QRect &pr, const QRect &tr )
{
    d->items.append( QIconDragDataItem( i, QIconDragData( pr, tr ) ) );
}

/*!
  \reimp
*/

const char* QIconDrag::format( int i ) const
{
    if ( i == 0 )
	return "application/x-qiconlist";
    return 0;
}

/*!
  Returns the encoded data of the drag object if
  \a mime is application/x-qiconlist.
*/

QByteArray QIconDrag::encodedData( const char* mime ) const
{
    QByteArray a;
    if ( QString( mime ) == "application/x-qiconlist" ) {
	QValueList<QIconDragDataItem>::ConstIterator it = d->items.begin();
	QString s;
	for ( ; it != d->items.end(); ++it ) {
	    QString k( "%1$@@$%2$@@$%3$@@$%4$@@$%5$@@$%6$@@$%7$@@$%8$@@$" );
	    k = k.arg( (*it).item.pixmapRect().x() ).arg( (*it).item.pixmapRect().y() ).arg( (*it).item.pixmapRect().width() ).
		arg( (*it).item.pixmapRect().height() ).arg( (*it).item.textRect().x() ).arg( (*it).item.textRect().y() ).
		arg( (*it).item.textRect().width() ).arg( (*it).item.textRect().height() );
	    k += QString( (*it).data.data() ) + "$@@$";
	    s += k;
	}
	a.resize( s.length() + 1 );
	memcpy( a.data(), s.latin1(), a.size() );
    }

    return a;
}

/*!
  Returns TRUE if \a e can be decoded by the QIconDrag,
  otherwise return FALSE.
*/

bool QIconDrag::canDecode( QMimeSource* e )
{
    if ( e->provides( "application/x-qiconlist" ) )
	return TRUE;
    return FALSE;
}

/*!
  Decodes the data which is stored (encoded) in \a e and, if successful,
  fills the \a list of icon drag items with the decoded data.
*/

bool QIconDragPrivate::decode( QMimeSource* e, QValueList<QIconDragDataItem> &lst )
{
    QByteArray ba = e->encodedData( "application/x-qiconlist" );
    if ( ba.size() ) {
	lst.clear();
	QString s = ba.data();
	QIconDragDataItem item;
	QRect ir, tr;
	QByteArray d;
	QStringList l = QStringList::split( "$@@$", s );

	int i = 0;
	QStringList::Iterator it = l.begin();
	for ( ; it != l.end(); ++it ) {
	    if ( i == 0 ) {
		ir.setX( ( *it ).toInt() );
	    } else if ( i == 1 ) {
		ir.setY( ( *it ).toInt() );
	    } else if ( i == 2 ) {
		ir.setWidth( ( *it ).toInt() );
	    } else if ( i == 3 ) {
		ir.setHeight( ( *it ).toInt() );
	    } else if ( i == 4 ) {
		tr.setX( ( *it ).toInt() );
	    } else if ( i == 5 ) {
		tr.setY( ( *it ).toInt() );
	    } else if ( i == 6 ) {
		tr.setWidth( ( *it ).toInt() );
	    } else if ( i == 7 ) {
		tr.setHeight( ( *it ).toInt() );
	    } else if ( i == 8 ) {
		d.resize( ( *it ).length() );
		memcpy( d.data(), ( *it ).latin1(), ( *it ).length() );
		item.item.setPixmapRect( ir );
		item.item.setTextRect( tr );
		item.data.setData( d );
		lst.append( item );
	    }
	    ++i;
	    if ( i > 8 )
		i = 0;
	}
	return TRUE;
    }

    return FALSE;
}

QIconDragData::QIconDragData()
    : iconRect_(), textRect_()
{
}

QIconDragData::QIconDragData( const QRect &ir, const QRect &tr )
    : iconRect_( ir ), textRect_( tr )
{
}

QRect QIconDragData::textRect() const
{
    return textRect_;
}

QRect QIconDragData::pixmapRect() const
{
    return iconRect_;
}

void QIconDragData::setPixmapRect( const QRect &r )
{
    iconRect_ = r;
}

void QIconDragData::setTextRect( const QRect &r )
{
    textRect_ = r;
}

#endif

/*****************************************************************************
 *
 * Class QIconViewItem
 *
 *****************************************************************************/

/*!
  \class QIconViewItem qiconview.h
  \brief A QIconViewItem encapsulates a single item in a QIconView.
  \module iconview

  A QIconViewItem encapsulates a single item in a QIconView; it contains
  an icon and a string, and can display itself in a QIconView.

  The simplest way to create an QIconViewItem and insert it into a
  QIconView is to construct the item with a pointer to the icon view, a
  string and an icon:

  \code
    (void) new QIconViewItem(
		    parent,	// A pointer to a QIconView
		    "This is the text of the item",
		    pixmap );
  \endcode

  By default the text of an icon view item may not be edited by the user
  but calling setRenameEnabled(TRUE) will allow the user to perform
  in-place editing of the item's text.

  When the icon view is deleted all items in it are deleted automatically.

  The QIconView::firstItem() and QIconViewItem::nextItem() functions
  provide a means of iterating over all the items in a QIconView:

  \code
    QIconViewItem *item;
    for ( item = iconView->firstItem(); item; item = item->nextItem() )
      do_something_with( item );
  \endcode

  To remove an item from an icon view, just delete the item. The
  QIconViewItem destructor removes it cleanly from its icon view.

  Because the icon view is designed to use drag-and-drop, the icon
  view item also has functions for drag-and-drop which may be
  reimplemented.

  The class is designed to be very similar to QListView and QListBox
  in use, both via instantiation and subclassing.
*/

/*!
  Constructs a QIconViewItem with no text and a default icon, and
  inserts it into the icon view \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent )
    : view( parent ), itemText(), itemIcon( unknown_icon )
{
    init();
}

/*!
  Constructs a QIconViewItem with no text and a default icon, and inserts
  it into the icon view \a parent after the icon view item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after )
    : view( parent ), itemText(), itemIcon( unknown_icon ),
      prev( 0 ), next( 0 )
{
    init( after );
}

/*!
  Constructs an icon view item using \a text as the text and a default
  icon, and inserts it into the icon view \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent, const QString &text )
    : view( parent ), itemText( text ), itemIcon( unknown_icon )
{
    init( 0 );
}

/*!
  Constructs an icon view item using \a text as the text and a default
  icon, and inserts it into the icon view \a parent after the icon view
  item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after,
			      const QString &text )
    : view( parent ), itemText( text ), itemIcon( unknown_icon )
{
    init( after );
}

/*!
  Constructs an icon view item using \a text as the text and a \a icon as
  the icon, and inserts it into the icon view \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent, const QString &text,
			      const QPixmap &icon )
    : view( parent ),
      itemText( text ), itemIcon( new QPixmap( icon ) )
{
    init( 0 );
}


/*!
  Constructs an icon view item using \a text as the text and \a icon as
  the icon, and inserts it into the icon view \a parent after the icon
  view item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after,
			      const QString &text, const QPixmap &icon )
    : view( parent ), itemText( text ), itemIcon( new QPixmap( icon ) )
{
    init( after );
}

/*!
  Constructs an icon view item using \a text as the text and a \a icon as
  the icon, and inserts it into the icon view \a parent.
*/

#ifndef QT_NO_PICTURE
QIconViewItem::QIconViewItem( QIconView *parent, const QString &text,
			      const QPicture &picture )
    : view( parent ), itemText( text ), itemIcon( 0 )
{
    init( 0, new QPicture( picture ) );
}

/*!
  Constructs an icon view item using \a text as the text and \a icon as
  the icon, and inserts it into the icon view \a parent after the icon
  view item \a after.
 */

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after,
			      const QString &text, const QPicture &picture )
    : view( parent ), itemText( text ), itemIcon( 0 )
{
    init( after, new QPicture( picture ) );
}
#endif

/*!
  This private function initializes the icon view item and inserts it
  into the icon view.
*/

void QIconViewItem::init( QIconViewItem *after
#ifndef QT_NO_PICTURE
			  , QPicture *pic
#endif
			  )
{
    d = new QIconViewItemPrivate;
    d->container1 = 0;
    d->container2 = 0;
    prev = next = 0;
    allow_rename = FALSE;
    allow_drag = TRUE;
    allow_drop = TRUE;
    selected = FALSE;
    selectable = TRUE;
    renameBox = 0;
#ifndef QT_NO_PICTURE
    itemPic = pic;
#endif
    if ( view ) {
	itemKey = itemText;
	dirty = TRUE;
	wordWrapDirty = TRUE;
	calcRect();
	view->insertItem( this, after );
    }
}

/*!
  Destroys the icon view item and tells the parent icon view that the
  item has been destroyed.
*/

QIconViewItem::~QIconViewItem()
{
    if ( view && !view->d->clearing )
	view->takeItem( this );
    view = 0;
    if ( itemIcon && itemIcon->serialNumber() != unknown_icon->serialNumber() )
	delete itemIcon;
#ifndef QT_NO_PICTURE
    delete itemPic;
#endif
    delete d;
}

/*!
  Sets \a text as the text of the icon view item.  This function might
  be a no-op if you reimplement text().

  \sa text()
*/

void QIconViewItem::setText( const QString &text )
{
    if ( text == itemText )
	return;

    wordWrapDirty = TRUE;
    itemText = text;
    if ( itemKey.isEmpty() )
	itemKey = itemText;

    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
    }
}

/*!
  Sets \a k as the sort key of the icon view item.

  \sa compareItems()
*/

void QIconViewItem::setKey( const QString &k )
{
    if ( k == itemKey )
	return;

    itemKey = k;
}

/*!
  Sets \a icon as the item's icon in the icon view. This function might be
  a no-op if you reimplement pixmap().

  \sa pixmap()
*/

void QIconViewItem::setPixmap( const QPixmap &icon )
{
    if ( itemIcon && itemIcon == unknown_icon )
	itemIcon = 0;

    if ( itemIcon )
	*itemIcon = icon;
    else
	itemIcon = new QPixmap( icon );
    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
    }
}

/*!
  Sets \a icon as the item's icon in the icon view. This function might be
  a no-op if you reimplement picture().

  \sa picture()
*/

#ifndef QT_NO_PICTURE
void QIconViewItem::setPicture( const QPicture &icon )
{
    // clear assigned pixmap if any
    if ( itemIcon ) {
	if ( itemIcon == unknown_icon ) {
	    itemIcon = 0;
	} else {
	    delete itemIcon;
	    itemIcon = 0;
	}
    }
    if ( itemPic )
	delete itemPic;
    itemPic = new QPicture( icon );

    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
    }
}
#endif

/*!
  Sets \a text as the text of the icon view item. If \a recalc is TRUE,
  the icon view's layout is recalculated. If \a redraw is TRUE (the
  default), the icon view is repainted.

  \sa text()
*/

void QIconViewItem::setText( const QString &text, bool recalc, bool redraw )
{
    if ( text == itemText )
	return;

    wordWrapDirty = TRUE;
    itemText = text;

    if ( recalc )
	calcRect();
    if ( redraw )
	repaint();
}

/*!
  Sets \a icon as the item's icon in the icon view. If \a recalc is TRUE, the
  icon view's layout is recalculated. If \a redraw is TRUE (the
  default), the icon view is repainted.

  \sa pixmap()
*/

void QIconViewItem::setPixmap( const QPixmap &icon, bool recalc, bool redraw )
{
    if ( itemIcon && itemIcon == unknown_icon )
	itemIcon = 0;

    if ( itemIcon )
	*itemIcon = icon;
    else
	itemIcon = new QPixmap( icon );

    if ( recalc )
	calcRect();
    if ( redraw ) {
	if ( recalc ) {
	    QRect oR = rect();
	    calcRect();
	    oR = oR.unite( rect() );

	    if ( view ) {
		if ( QRect( view->contentsX(), view->contentsY(),
			    view->visibleWidth(), view->visibleHeight() ).
		     intersects( oR ) )
		    view->repaintContents( oR.x() - 1, oR.y() - 1,
				       oR.width() + 2, oR.height() + 2, FALSE );
	    }
	} else {
	    repaint();
	}
    }
}

/*!
  If \a allow is TRUE, the user can rename the icon view item by
  clicking on the text (or pressing F2) while the item is selected
  (in-place renaming). If \a allow is FALSE, in-place renaming is not
  possible.
*/

void QIconViewItem::setRenameEnabled( bool allow )
{
    allow_rename = (uint)allow;
}

/*!
  If \a allow is TRUE, the icon view permits the user to drag the icon
  view item either to another position within the icon view or to
  somewhere outside of it. If \a allow is FALSE, the item cannot be
  dragged.
*/

void QIconViewItem::setDragEnabled( bool allow )
{
    allow_drag = (uint)allow;
}

/*!
  If \a allow is TRUE, the icon view lets the user drop something on
  this icon view item.
*/

void QIconViewItem::setDropEnabled( bool allow )
{
    allow_drop = (uint)allow;
}

/*!
  Returns the text of the icon view item. Normally you set the
  text of the item with setText(), but sometimes it's inconvenient to
  call setText() for each item; so you can subclass QIconViewItem,
  reimplement this function, and return the text of the item. If you do
  this, you have to call calcRect() manually each time the text (and so
  the size of it) changes.

  \sa setText()
*/

QString QIconViewItem::text() const
{
    return itemText;
}

/*!
  Returns the key of the icon view item.

  \sa setKey(), compareItems()
*/

QString QIconViewItem::key() const
{
    return itemKey;
}

/*!
  Returns the icon of the icon view item if it is a pixmap, or 0 if it is a
  picture. In the latter case use picture() instead. Normally you will set the
  pixmap of the item with setPixmap(), but sometimes it's inconvenient
  to call setText() for each item. So you can subclass QIconViewItem,
  reimplement this function and return a pointer to the item's pixmap.
  If you do this, you have to call calcRect() manually each time the
  size of this pixmap changes!

  \sa setPixmap()
*/

QPixmap *QIconViewItem::pixmap() const
{
    return itemIcon;
}

/*!
  Returns the icon of the icon view item if it is a picture, or 0 if it is a
  pixmap. In the latter case use pixmap() instead. Normally you will set the
  picture of the item with setPicture(), but sometimes it's inconvenient
  to call setText() for each item. So you can subclass QIconViewItem,
  reimplement this function and return a pointer to the item's picture.
  If you do this, you have to call calcRect() manually each time the
  size of this picture changes!

  \sa setPicture()
*/

#ifndef QT_NO_PICTURE
QPicture *QIconViewItem::picture() const
{
    return itemPic;
}
#endif

/*!
  Returns TRUE if the item can be renamed by the user with in-place
  renaming, or else FALSE.

  \sa setRenameEnabled()
*/

bool QIconViewItem::renameEnabled() const
{
    return (bool)allow_rename;
}

/*!
  Returns TRUE if the user is allowed to drag the icon view item, or else
  FALSE.

  \sa setDragEnabled()
*/

bool QIconViewItem::dragEnabled() const
{
    return (bool)allow_drag;
}

/*!
  Returns TRUE if the user is allowed to drop something onto the item,
  otherwise FALSE.

  \sa setDropEnabled()
*/

bool QIconViewItem::dropEnabled() const
{
    return (bool)allow_drop;
}

/*!
  Returns a pointer to this item's icon view parent.
*/

QIconView *QIconViewItem::iconView() const
{
    return view;
}

/*!
  Returns a pointer to the previous item, or 0 if this is the first item
  of the icon view.
*/

QIconViewItem *QIconViewItem::prevItem() const
{
    return prev;
}

/*!
  Returns a pointer to the next item, or 0 if this is the last item
  of the icon view.
*/

QIconViewItem *QIconViewItem::nextItem() const
{
    return next;
}

/*!
  Returns the index of this item in the icon view, or -1 if an error occurred.
*/

int QIconViewItem::index() const
{
    if ( view )
	return view->index( this );

    return -1;
}



/*! \overload

  This variant is equivalent to calling the other variant with \a cb
  set to FALSE.
*/

void QIconViewItem::setSelected( bool s )
{
    setSelected( s, FALSE );
}

/*!
  Selects or unselects the item, depending on \a s; it may also
  unselect other items, depending on QIconView::selectionMode() and \a
  cb.

  If \a s is FALSE, the item is unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Single, the item
  is selected and the item previously selected is unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Extended, the item
  is selected. If \a cb is TRUE, the other items of the icon view are
  not touched. If \a cb is FALSE (the default), all other items are unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Multi, the item
  is selected.

  Note that \a cb is used only if QIconView::selectionMode() is \c
  Extended; cb defaults to FALSE.

  All items whose selection status changes repaint themselves.
*/

void QIconViewItem::setSelected( bool s, bool cb )
{
    if ( !view )
	return;
    if ( view->selectionMode() != QIconView::NoSelection &&
	 selectable && s != (bool)selected ) {

	if ( view->d->selectionMode == QIconView::Single && this != view->d->currentItem ) {
	    QIconViewItem *o = view->d->currentItem;
	    if ( o && o->selected )
		o->selected = FALSE;
	    view->d->currentItem = this;
	    if ( o )
		o->repaint();
	    emit view->currentChanged( this );
	}

	if ( !s ) {
	    selected = FALSE;
	} else {
	    if ( view->d->selectionMode == QIconView::Single && view->d->currentItem ) {
		view->d->currentItem->selected = FALSE;
	    }
	    if ( ( view->d->selectionMode == QIconView::Extended && !cb ) ||
		 view->d->selectionMode == QIconView::Single ) {
		bool b = view->signalsBlocked();
		view->blockSignals( TRUE );
		view->selectAll( FALSE );
		view->blockSignals( b );
	    }
	    selected = s;
	}

	repaint();
	if ( !view->signalsBlocked() ) {
	    bool emitIt = view->d->selectionMode == QIconView::Single && s;
	    QIconView *v = view;
	    emit v->selectionChanged();
	    if ( emitIt )
		emit v->selectionChanged( this );
	}
    }
}

/*!   Sets this item to be selectable if \a enable is TRUE (the
  default) or not to be selectable if \a enable is FALSE.

  The user is not able to select a non-selectable item using either
  the keyboard or mouse.  The application programmer still can, of
  course.  \sa isSelectable() */

void QIconViewItem::setSelectable( bool enable )
{
    selectable = (uint)enable;
}

/*!
  Returns TRUE if the item is selected, or else FALSE.

  \sa setSelected()
*/

bool QIconViewItem::isSelected() const
{
    return (bool)selected;
}

/*!
  Returns TRUE if the item is selectable, or else FALSE.

  \sa setSelectable()
*/

bool QIconViewItem::isSelectable() const
{
    return (bool)selectable;
}

/*!
  Repaints the item.
*/

void QIconViewItem::repaint()
{
    if ( view )
	view->repaintItem( this );
}

/*!
  Moves the item to \a x and \a y in the icon view (these are contents
  coordinates).
*/

void QIconViewItem::move( int x, int y )
{
    itemRect.setRect( x, y, itemRect.width(), itemRect.height() );
    checkRect();
    if ( view )
	view->updateItemContainer( this );
}

/*!
  Moves the item by the distance \a dx and \a dy.
*/

void QIconViewItem::moveBy( int dx, int dy )
{
    itemRect.moveBy( dx, dy );
    checkRect();
    if ( view )
	view->updateItemContainer( this );
}

/*!
  \overload void QIconViewItem::move( const QPoint &pnt  )
*/

void QIconViewItem::move( const QPoint &pnt )
{
    move( pnt.x(), pnt.y() );
}

/*!
  \overload void QIconViewItem::moveBy( const QPoint &pnt )
*/

void QIconViewItem::moveBy( const QPoint &pnt )
{
    moveBy( pnt.x(), pnt.y() );
}

/*!
  Returns the bounding rect of the item (in contents coordinates).
*/

QRect QIconViewItem::rect() const
{
    return itemRect;
}

/*!
  Returns the X-Coordinate of the item (in contents coordinates).
*/

int QIconViewItem::x() const
{
    return itemRect.x();
}

/*!
  Returns the Y-Coordinate of the item (in contents coordinates).
*/

int QIconViewItem::y() const
{
    return itemRect.y();
}

/*!
  Returns the width of the item.
*/

int QIconViewItem::width() const
{
    return QMAX( itemRect.width(), QApplication::globalStrut().width() );
}

/*!
  Returns the height of the item.
*/

int QIconViewItem::height() const
{
    return QMAX( itemRect.height(), QApplication::globalStrut().height() );
}

/*!
  Returns the size of the item.
*/

QSize QIconViewItem::size() const
{
    return QSize( itemRect.width(), itemRect.height() );
}

/*!
  Returns the position of the item (in contents coordinates).
*/

QPoint QIconViewItem::pos() const
{
    return QPoint( itemRect.x(), itemRect.y() );
}

/*!
  Returns the bounding rectangle of the item's text.

  If \a relative is FALSE, the returned rectangle is relative to the
  origin of the icon view's contents coordinate system. If \a relative
  is TRUE, the rectangle is relative to the origin of the item's
  rectangle.
*/

QRect QIconViewItem::textRect( bool relative ) const
{
    if ( relative )
	return itemTextRect;
    else
	return QRect( x() + itemTextRect.x(), y() + itemTextRect.y(), itemTextRect.width(), itemTextRect.height() );
}

/*!
  Returns the bounding rectangle of the item's icon.

  If \a relative is FALSE, the returned rectangle is relative to the
  origin of the icon view's contents coordinate system. If \a relative
  is TRUE, the rectangle is relative to the origin of the item's
  rectangle.
*/

QRect QIconViewItem::pixmapRect( bool relative ) const
{
    if ( relative )
	return itemIconRect;
    else
	return QRect( x() + itemIconRect.x(), y() + itemIconRect.y(), itemIconRect.width(), itemIconRect.height() );
}

/*!
  Returns TRUE if the item contains the point \a pnt (in contents
  coordinates), and FALSE if it does not.
*/

bool QIconViewItem::contains( const QPoint& pnt ) const
{
    return ( textRect( FALSE ).contains( pnt ) ||
	     pixmapRect( FALSE ).contains( pnt ) );
}

/*!
  Returns TRUE if the item intersects the rectangle \a r (in contents
  coordinates), and FALSE if it does not.
*/

bool QIconViewItem::intersects( const QRect& r ) const
{
    return ( textRect( FALSE ).intersects( r ) ||
	     pixmapRect( FALSE ).intersects( r ) );
}

/*!
  \fn bool QIconViewItem::acceptDrop( const QMimeSource *mime ) const

  Returns TRUE if the item accepts the QMimeSource\a mime (so it
  could be dropped on the item), and FALSE if it does not.

  The default implementation does nothing and always returns FALSE. A
  subclass must reimplement this to accept drops.
*/

bool QIconViewItem::acceptDrop( const QMimeSource * ) const
{
    return FALSE;
}

/*!
  Starts in-place renaming of an icon, if allowed.

  This function sets up the icon view so the user can edit the
  item text, and then returns. When the user is done, setText() will
  be called and QIconView::itemRenamed() will be emitted.
*/

void QIconViewItem::rename()
{
    if ( !view )
	return;
    oldRect = rect();
    renameBox = new QIconViewItemLineEdit( itemText, view->viewport(), this );
    QRect tr( textRect( FALSE ) );
    view->addChild( renameBox, tr.x() + ( tr.width() / 2 - renameBox->width() / 2 ), tr.y() - 3 );
    renameBox->selectAll();
    view->viewport()->setFocusProxy( renameBox );
    renameBox->setFocus();
    renameBox->show();
}

/*!
  Compares this icon view item to \a i. Returns -1 if this item
  is less than \a i, 0 if they are equal, and 1 if this icon view item
  is greater than \a i.

  The default implementation uses QIconViewItem::key() to compare the
  items. A reimplementation may use different values.

  \sa key()
*/

int QIconViewItem::compare( QIconViewItem *i ) const
{
    return key().compare( i->key() );
}

/*!
  This private function is called when the user pressed Return in the
  in-place renaming.
*/

void QIconViewItem::renameItem()
{
    if ( !renameBox || !view )
	return;

    if ( !view->d->wordWrapIconText ) {
	wordWrapDirty = TRUE;
	calcRect();
    }
    QRect r = itemRect;
    setText( renameBox->text() );
    view->repaintContents( oldRect.x() - 1, oldRect.y() - 1, oldRect.width() + 2, oldRect.height() + 2, FALSE );
    view->repaintContents( r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2, FALSE );
    removeRenameBox();

    view->emitRenamed( this );
}

/*!
  Cancels in-place renaming.
*/

void QIconViewItem::cancelRenameItem()
{
    if ( !view )
	return;

    QRect r = itemRect;
    calcRect();
    view->repaintContents( oldRect.x() - 1, oldRect.y() - 1, oldRect.width() + 2, oldRect.height() + 2, FALSE );
    view->repaintContents( r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2, FALSE );

    if ( !renameBox )
	return;

    removeRenameBox();
}

/*!
  Removes the editbox that is used for in-place renaming.
*/

void QIconViewItem::removeRenameBox()
{
    if ( !renameBox || !view )
	return;

    bool resetFocus = view->viewport()->focusProxy() == renameBox;
    delete renameBox;
    renameBox = 0;
    if ( resetFocus ) {
	view->viewport()->setFocusProxy( view );
	view->setFocus();
    }
}

/*! This virtual function is responsible for calculating the
  rectangles returned by rect(), textRect() and pixmapRect().
  setRect(), setTextRect() and setPixmapRect() are provided mainly for
  reimplementations of this function.
*/

void QIconViewItem::calcRect( const QString &text_ )
{
    if ( !view ) // #####
	return;

    wordWrapDirty = TRUE;
    int pw = 0;
    int ph = 0;

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QRect br = picture()->boundingRect();
	pw = br.width() + 2;
	ph = br.height() + 2;
    } else
#endif
    {
	pw = ( pixmap() ? pixmap() : unknown_icon )->width() + 2;
	ph = ( pixmap() ? pixmap() : unknown_icon )->height() + 2;
    }

    itemIconRect.setWidth( pw );
    itemIconRect.setHeight( ph );

    calcTmpText();

    QString t = text_;
    if ( t.isEmpty() ) {
	if ( view->d->wordWrapIconText )
	    t = itemText;
	else
	    t = tmpText;
    }

    int tw = 0;
    int th = 0;
    // ##### TODO: fix font bearings!
    QRect r;
    if ( view->d->wordWrapIconText ) {
	r = QRect( view->d->fm->boundingRect( 0, 0, iconView()->maxItemWidth() -
					      ( iconView()->itemTextPos() == QIconView::Bottom ? 0 :
						pixmapRect().width() ) - 4,
					      0xFFFFFFFF, AlignHCenter | WordBreak, t ) );
	r.setWidth( r.width() + 4 );
    } else {
	r = QRect( 0, 0, view->d->fm->width( t ), view->d->fm->height() );
	r.setWidth( r.width() + 4 );
    }

    if ( r.width() > iconView()->maxItemWidth() -
	 ( iconView()->itemTextPos() == QIconView::Bottom ? 0 :
	   pixmapRect().width() ) )
	r.setWidth( iconView()->maxItemWidth() - ( iconView()->itemTextPos() == QIconView::Bottom ? 0 :
						   pixmapRect().width() ) );

    tw = r.width();
    th = r.height();
    if ( tw < view->d->fm->width( "X" ) )
	tw = view->d->fm->width( "X" );

    itemTextRect.setWidth( tw );
    itemTextRect.setHeight( th );

    int w = 0;
    int h = 0;
    if ( view->itemTextPos() == QIconView::Bottom ) {
	w = QMAX( itemTextRect.width(), itemIconRect.width() );
	h = itemTextRect.height() + itemIconRect.height() + 1;

	itemRect.setWidth( w );
	itemRect.setHeight( h );

	itemTextRect = QRect( ( width() - itemTextRect.width() ) / 2, height() - itemTextRect.height(),
			      itemTextRect.width(), itemTextRect.height() );
	itemIconRect = QRect( ( width() - itemIconRect.width() ) / 2, 0,
			      itemIconRect.width(), itemIconRect.height() );
    } else {
	h = QMAX( itemTextRect.height(), itemIconRect.height() );
	w = itemTextRect.width() + itemIconRect.width() + 1;

	itemRect.setWidth( w );
	itemRect.setHeight( h );

	itemTextRect = QRect( width() - itemTextRect.width(), ( height() - itemTextRect.height() ) / 2,
			      itemTextRect.width(), itemTextRect.height() );
	itemIconRect = QRect( 0, ( height() - itemIconRect.height() ) / 2,
			      itemIconRect.width(), itemIconRect.height() );
    }
    if ( view )
	view->updateItemContainer( this );
}

/*!
  Paints the item using the painter \a p and the color group \a cg. If
  you want the item to be drawn with a different font or color,
  reimplement this function, change the values of the color group or
  the painter's font, and then call the QIconViewItem::paintItem()
  with the changed values.
*/

void QIconViewItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    if ( !view )
	return;

    p->save();

    if ( isSelected() ) {
	p->setPen( cg.highlightedText() );
    } else {
	p->setPen( cg.text() );
    }

    calcTmpText();

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
	    p->fillRect( pixmapRect( FALSE ), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = view->itemTextPos() == QIconView::Bottom ? AlignHCenter : AlignAuto;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak;
	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );
	p->restore();
	return;
    }
#endif
    // ### get rid of code duplication
    if ( view->itemTextPos() == QIconView::Bottom ) {
	int w = ( pixmap() ? pixmap() : unknown_icon )->width();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap() ? pixmap() : unknown_icon;
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_qiv_buffer_pixmap( pix->size() );
		QBitmap mask = view->mask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
#if defined(Q_WS_X11)
 		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
#else // in WIN32 Dense4Pattern doesn't work correctly (transparence problem), so work around it
		if ( !qiv_selection )
		    createSelectionPixmap( cg );
		p2.drawTiledPixmap( 0, 0, pix->width(), pix->height(), *qiv_selection );
#endif
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() + ( width() - w ) / 2, y(), *buffer, 0, 0, cr.width(), cr.height() );

	    }
	} else {
	    p->drawPixmap( x() + ( width() - w ) / 2, y(), *( pixmap() ? pixmap() : unknown_icon ) );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = AlignHCenter;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak;
	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );

	p->restore();
    } else {
	int h = ( pixmap() ? pixmap() : unknown_icon )->height();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap() ? pixmap() : unknown_icon;
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_qiv_buffer_pixmap( pix->size() );
		QBitmap mask = view->mask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
#if defined(Q_WS_X11)
		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
#else // in WIN32 Dense4Pattern doesn't work correctly (transparence problem), so work around it
		if ( !qiv_selection )
		    createSelectionPixmap( cg );
		p2.drawTiledPixmap( 0, 0, pix->width(), pix->height(), *qiv_selection );
#endif
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() , y() + ( height() - h ) / 2, *buffer, 0, 0, cr.width(), cr.height() );
	    }
	} else {
	    p->drawPixmap( x() , y() + ( height() - h ) / 2, *( pixmap() ? pixmap() : unknown_icon ) );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = AlignAuto;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak;
	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );

	p->restore();
    }

    p->restore();
}

/*!
  Paints the focus rect of the item using the painter \a p and the color group \a cg.
*/

void QIconViewItem::paintFocus( QPainter *p, const QColorGroup &cg )
{
    if ( !view )
	return;

    view->style().drawFocusRect( p, QRect( textRect( FALSE ).x(), textRect( FALSE ).y(),
					   textRect( FALSE ).width(), textRect( FALSE ).height() ),
				 cg, isSelected() ? &cg.highlight() : &cg.base(), isSelected() );
    if ( this != view->d->currentItem ) {
	view->style().drawFocusRect( p, QRect( pixmapRect( FALSE ).x(), pixmapRect( FALSE ).y(),
					       pixmapRect( FALSE ).width(),
					       pixmapRect( FALSE ).height() ),
				     cg, &cg.base(), FALSE );
    }
}

/*!
  \fn void QIconViewItem::dropped( QDropEvent *e, const QValueList<QIconDragItem> &lst )

  This function is called when something is dropped on the item.  \a e
  contains all the information about the drop.  If the drag object of the drop
  was a QIconDrag, \a lst contains the list of the dropped items.  You can then get
  the data using QIconDragItem::data() of each item.

  If \a lst is not empty, you can use \a lst for further processing. Otherwise
  the drag was not a QIconDrag, so you have to decode \a e
  yourself and work with that.

  The default implementation does nothing; subclasses may reimplement this
  function.
*/

#ifndef QT_NO_DRAGANDDROP
void QIconViewItem::dropped( QDropEvent *, const QValueList<QIconDragItem> & )
{
}
#endif

/*!
  This function is called when a drag enters the item's bounding rect.

  The default implementation does nothing; subclasses may reimplement
  this function.
*/

void QIconViewItem::dragEntered()
{
}

/*!
  This function is called when a drag leaves the item's bounding rect.

  The default implementation does nothing; subclasses may reimplement
  this function.
*/

void QIconViewItem::dragLeft()
{
}

/*!  Sets the bounding rectangle of the whole item.  This function is
  provided fort subclasses which reimplement calcRect(), so that they
  can set the calculated rectangle. Other use is discouraged.

  \sa calcRect() itemRect() setTextRect() setPixmapRect()
*/

void QIconViewItem::setItemRect( const QRect &r )
{
    itemRect = r;
    checkRect();
    if ( view )
	view->updateItemContainer( this );
}

/*!  Sets the bounding rectangle of the item's text.  This function is
  provided fort subclasses which reimplement calcRect(), so that they
  can set the calculated rectangle. Other use is discouraged.

  \sa calcRect() textRect() setItemRect() setPixmapRect()
*/

void QIconViewItem::setTextRect( const QRect &r )
{
    itemTextRect = r;
    if ( view )
	view->updateItemContainer( this );
}

/*!  Sets the bounding rectangle of the item's icon.  This function is
  provided fort subclasses which reimplement calcRect(), so that they
  can set the calculated rectangle. Other use is discouraged.

  \sa calcRect() pixmapRect() setItemRect() setTextRect()
*/

void QIconViewItem::setPixmapRect( const QRect &r )
{
    itemIconRect = r;
    if ( view )
	view->updateItemContainer( this );
}

/*!
  \internal
*/

void QIconViewItem::calcTmpText()
{
    if ( !view || view->d->wordWrapIconText || !wordWrapDirty )
	return;
    wordWrapDirty = FALSE;

    int w = iconView()->maxItemWidth() - ( iconView()->itemTextPos() == QIconView::Bottom ? 0 :
					   pixmapRect().width() ) - 4;
    if ( view->d->fm->width( itemText ) < w ) {
	tmpText = itemText;
	return;
    }

    tmpText = "...";
    int i = 0;
    while ( view->d->fm->width( tmpText + itemText[ i ] ) < w )
	tmpText += itemText[ i++ ];
    tmpText.remove( 0, 3 );
    tmpText += "...";
}

void QIconViewItem::checkRect()
{
    int x = itemRect.x();
    int y = itemRect.y();
    int w = itemRect.width();
    int h = itemRect.height();

    bool changed = FALSE;
    if ( x < 0 ) {
	x = 0;
	changed = TRUE;
    }
    if ( y < 0 ) {
	y = 0;
	changed = TRUE;
    }

    if ( changed )
	itemRect.setRect( x, y, w, h );
}

/*****************************************************************************
 *
 * Class QIconView
 *
 *****************************************************************************/

/*!
  \class QIconView qiconview.h
  \brief The QIconView class provides an area with movable labelled icons.
  \module iconview

  \ingroup advanced

    The QIconView class provides a rectangular area which contains
    moveable labelled icons. It can display and manage a grid or other
    2D layout of labelled icons. Each labelled icon is a QIconViewItem.
    Items (QIconViewItems) can be added or deleted at any time; items
    can be moved within the QIconView. Single or multiple items can be
    selected. Items can be renamed in-place. QIconView also supports
    drag and drop.

    Each item contains a label string, a pixmap (the icon itself) and
    optionally an index key. The index key is used for sorting the items
    and defaults to the label string. The label string can be displayed
    below or to the right of the icon (see \l ItemTextPos).

    The simplest way to create a QIconView is to create a QIconView
    object and create some QIconViewItems with the QIconView as their
    parent, set the icon view's geometry and show it. Below is an
    example of how such code might look:

    \code
    QIconView *iv = new QIconView( this );
    QDir dir( path, "*.xpm" );
    for ( uint i = 0; i < dir.count(); i++ ) {
	(void) new QIconViewItem( iv, dir[i], QPixmap( path + dir[i] ) );
    }
    iv->resize( 600, 400 );
    iv->show();
    \endcode

    When an item is inserted the QIconView allocates a position for it.
    The default arrangement is \c LeftToRight -- QIconView fills up the
    \e left-most column from top to bottom, then moves one column \e right
    and fills that from top to bottom and so on. The arrangement can be
    modified with any of the following approaches:
    <ul>
    <li>Call setArrangement(), e.g. with \c TopToBottom which will fill
    the \e top-most row from left to right, then moves one row \e down and
    fills that row from left to right and so on.
    <li>Construct each QIconViewItem using a constructor which allows
    you to specify which item the new one is to follow.
    <li>Call setSorting() or sort() to sort the items.
    </ul>

    Items which are <em>\link QIconViewItem::isSelectable()
    selectable\endlink</em> may be selected depending on the
    SelectionMode (default is \c Single). Because QIconView offers
    multiple selection it has to display keyboard focus and selection
    state separately. Therefore there are functions to set the selection
    state of an item (setSelected()) and to select which item displays
    keyboard focus (setCurrentItem()). When multiple items may be
    selected the icon view provides a rubberband, too.

    When in-place renaming is enabled (it is disabled by default), the
    user may change the item's label. They do this by selecting the item
    (single clicking it or navigating to it with the arrow keys), then
    single clicking it (or pressing F2), and entering their text. If no
    key has been set with QIconViewItem::setKey() the new text will also
    serve as the key. (See QIconViewItem::setRenameEnabled().)

    QIconView offers functions similar to QListView and QListBox, such as
    takeItem(), clearSelection(), setSelected(), setCurrentItem(),
    currentItem() and many more.

    Because the internal structure used to store the icon view items is
    linear (a double-linked list), no iterator class is needed to
    iterate over all the items. Instead we iterate by getting the first
    item from the icon view and then each subsequent (\l nextItem) from
    each item in turn:

  \code
  for ( QIconViewItem *item = iv->firstItem(); item; item = item->nextItem() )
    do_something( item );
  \endcode

    QIconView supports the drag and drop of items within the QIconView
    itself. It also supports the drag and drop of items out of or into
    the QIconView and drag and drop onto items themselves. The drag and
    drop of items outside the QIconView can be achieved in a simple way
    with basic functionality, or in a more sophisticated way which
    provides more power and control.

    The simple approach to dragging items out of the icon view is to
    subclass QIconView and reimplement QIconView::dragObject().

    \code
    QDragObject *MyIconView::dragObject()
    {
      return new QTextDrag( currentItem()->text(), this );
    }
    \endcode

    In this example we create a QTextDrag object, (derived from
    QDragObject), containing the item's label and return it as the drag
    object. We could just as easily have created a QImageDrag from the
    item's pixmap and returned that instead.

    QIconViews and their QIconViewItems can also be the targets of drag
    and drops. To make the QIconView itself able to accept drops connect
    to the dropped() signal. When a drop occurs this signal will be
    emitted with a QDragEvent and a QValueList of QIconDragItems. To
    make a QIconViewItem into a drop target subclass QIconViewItem and
    reimplement QIconViewItem::acceptDrop() and
    QIconViewItem::dropped().

    \code
    bool MyIconViewItem::acceptDrop( const QMimeSource *mime ) const
    {
	if ( mime->provides( "text/plain" ) )
	    return TRUE;
	return FALSE;
    }

    void MyIconViewItem::dropped( QDropEvent *evt, const QValueList<QIconDragItem>& )
    {
	QString label;
	if ( QTextDrag::decode( evt, label ) )
	    setText( label );
    }
    \endcode

    See qt/examples/iconview/simple_dd for a simple drag and drop
    example which demonstrates drag and drop between a QIconView and a
    QListBox.

  If you want to have drag shapes drawn you have to do more complex things.

  The first part is starting drags; if you want to use extended drag-and-drop in
  the QIconView, you should use QIconDrag (or a derived class from
  that) as dragobject, and in dragObject() create such an object and
  return it. But before returning it, fill it there with QIconDragItems.
  Normally such a drag should offer data of each selected item. So in
  dragObject() you should iterate over all items, create for each
  selected item a QIconDragItem, and append this with
  QIconDrag::append() to the QIconDrag object. With
  QIconDragItem::setData() you can set the data of each item that
  should be dragged. If you want to offer the data in additional
  mime-types, it's best to use a class derived from QIconDrag,
  which implements additional encoding and decoding functions.

  Now when a drag enters the icon view, there is not much to do. Just
  connect to the dropped() signal and reimplement
  QIconViewItem::dropped() and QIconViewItem::acceptDrop(). The only
  thing special in this case is the second argument in the dropped()
  signal and in QIconViewItem::dropped(). For further details, look at
  the documentation of this signal/function.

  For an example implementation of the complex drag- and-drop stuff look at the
  qfileiconview example (qt/examples/qfileiconview).

  Finally, see also QIconViewItem::setDragEnabled(),
  QIconViewItem::setDropEnabled(), QIconViewItem::acceptDrop() and
  QIconViewItem::dropped().

  <img src=qiconview-m.png> <img src=qiconview-w.png>
*/

/*! \enum QIconView::ResizeMode

  This enum type decides how QIconView should treat the positions of
  its icons when the widget is resized.  The currently defined modes
  are:

  \value Fixed  The icons' positions are not changed.
  \value Adjust  The icons' positions are adjusted to be within the
  new geometry, if possible.
*/

/*! \enum QIconView::SelectionMode

  This enumerated type is used by QIconView to indicate how it reacts
  to selection by the user. It has four values:

  \value Single  When the user selects an item, any already-selected
  item becomes unselected and the user cannot unselect the selected
  item. This means that the user can never clear the selection. (The
  application programmer can, using QIconView::clearSelection().)

  \value Multi  When the user selects an item in the most ordinary
  way, the selection status of that item is toggled and the other
  items are left alone.

  \value Extended  When the user selects an item in the most
  ordinary way, the selection is cleared and the new item selected.
  However, if the user presses the CTRL key when clicking on an item,
  the clicked item gets toggled and all other items are left untouched. And
  if the user presses the SHIFT key while clicking on an item, all items
  between the current item and the clicked item get selected or unselected,
  depending on the state of the clicked item.
  Also, multiple items can be selected by dragging the mouse while the
  left mouse button stays pressed.

  \value NoSelection  Items cannot be selected.

  In other words, \c Single is a real single-selection icon view; \c
  Multi a real multi-selection icon view; \c Extended is an icon view
  in which users can select multiple items but usually want to select
  either just one or a range of contiguous items; and \c NoSelection
  is for an icon view where the user can look but not touch.
*/

/*! \enum QIconView::Arrangement

   This enum type decides in which direction the items flow when the view
   overflows.

   \value LeftToRight  Items which don't fit onto the view go
   further down (you get a vertical scrollbar)

   \value TopToBottom  Items which don't fit onto the view go
   further right (you get a horizontal scrollbar)
*/

/*! \enum QIconView::ItemTextPos

   This enum type specifies the position of the item text in relation to the icon.

   \value Bottom  The text is drawn below the icon.
   \value Right  The text is drawn to the right of the icon.
*/

/*! \fn void  QIconView::dropped ( QDropEvent * e, const QValueList<QIconDragItem> &lst )

  This signal is emitted when a drop event occurs in the viewport
  (but not any icon) which the icon view itself can't handle.

  \a e gives you all information about the drop. If the drag object of
  the drop was a QIconDrag, \a lst contains the list of the dropped
  items. You can get the data using QIconDragItem::data() of each item
  then.

  If \a lst is not empty, you can use \a lst for further processing. Otherwise
  the drag was not a QIconDrag, so you have to decode \a e
  yourself and work with that.
*/

/*! \fn void QIconView::moved ()

  This signal is emitted after successfully dropping one (or more)
  item(s) of the icon view somewhere. If they should be removed, it's
  best to do it in a slot connected to this signal.
*/

/*! \fn void  QIconView::doubleClicked (QIconViewItem * item)
  This signal is emitted when the user double-clicks on \a item.
*/

/*! \fn void  QIconView::returnPressed (QIconViewItem * item)
  This signal is emitted if the user presses the Return or Enter button.
  \a item is the currentItem() at the time of the press.
*/

/*! \fn void  QIconView::selectionChanged ()
  This signal is emitted when the selection has been changed. It's emitted
  in each selection mode.
*/

/*! \fn void  QIconView::selectionChanged( QIconViewItem *item )

  This signal is emitted when the selection changes. \a item is the
  new selected item. This signal is emitted only in single selection
  mode.
*/

/*! \fn void QIconView::currentChanged ( QIconViewItem *item )

  This signal is emitted when a new item becomes current.
  \a item is the new current item (or 0 if no item is current now).

  \sa currentItem()
*/

/*! \fn void  QIconView::onItem( QIconViewItem *i )

  This signal is emitted when the user moves the mouse cursor onto an
  item, similar to the QWidget::enterEvent() function.
*/

// ### bug here - enter/leave event aren't considered. move the mouse
// out of the window and back in, to the same item.

/*! \fn void QIconView::onViewport()

  This signal is emitted when the user moves the mouse cursor from an
  item to an empty part of the icon view.

  \sa onItem()
*/

/*!
  \fn void QIconView::itemRenamed (QIconViewItem * item)

  This signal is emitted when \a item has bee renamed, usually by
  in in-place renaming.

  \sa setRenameEnabled() rename()
*/

/*!
  \fn void QIconView::itemRenamed (QIconViewItem * item, const QString &name)

  This signal is emitted when \a item has bee renamed to \a name,
  usually by in in-place renaming.

  \sa setRenameEnabled() rename()
*/

/*!
  \fn void QIconView::rightButtonPressed (QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user presses the right mouse
  button. If \a item is non-null, the cursor is on \a item. If \a item
  is null, the mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global
  coordinate system (QMouseEvent::globalPos()).

  \sa rightButtonClicked() mouseButtonPressed() pressed()
*/

/*!
  \fn void QIconView::rightButtonClicked (QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user clicks the right mouse
  button. If \a item is non-null, the cursor is on \a item. If \a item
  is null, the mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global coordinate
  system (QMouseEvent::globalPos()). (If the click's press and release
  differ by a pixel or two, \a pos is the  position at release time.)

  \sa rightButtonPressed() mouseButtonClicked() clicked()
*/

/*!
  \fn void QIconView::mouseButtonPressed (int button, QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user presses mouse button \a
  button. If \a item is non-null, the cursor is on \a item. If \a item
  is null, the mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global
  coordinate system (QMouseEvent::globalPos()).

  \sa rightButtonClicked() mouseButtonPressed() pressed()
*/

/*!
  \fn void QIconView::mouseButtonClicked (int button, QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user clicks mouse button \a
  button. If \a item is non-null, the cursor is on \a item. If \a item
  is null, the mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global coordinate
  system (QMouseEvent::globalPos()). (If the click's press and release
  differ by a pixel or two, \a pos is the  position at release time.)

  \sa mouseButtonPressed() rightButtonClicked() clicked()
*/

/*!
  \fn void QIconView::clicked ( QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user clicks any mouse button. If \a
  item is non-null, the cursor is on \a item. If \a item is null, the
  mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global coordinate
  system (QMouseEvent::globalPos()). (If the click's press and release
  differ by a pixel or two, \a pos is the  position at release time.)

  \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
  \fn void QIconView::pressed ( QIconViewItem * item, const QPoint & pos)

  This signal is emitted when the user presses any mouse button. If \a
  item is non-null, the cursor is on \a item. If \a item is null, the
  mouse cursor isn't on any item.

  \a pos is the position of the mouse cursor in the global coordinate
  system (QMouseEvent::globalPos()). (If the click's press and release
  differ by a pixel or two, \a pos is the  position at release time.)

  \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
  \fn void QIconView::clicked ( QIconViewItem * item )

  This signal is emitted when the user clicks any mouse button. If \a
  item is non-null, the cursor is on \a item. If \a item is null, the
  mouse cursor isn't on any item.

  \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
  \fn void QIconView::pressed ( QIconViewItem * item )

  This signal is emitted when the user presses any mouse button. If \a
  item is non-null, the cursor is on \a item. If \a item is null, the
  mouse cursor isn't on any item.

  \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
  Constructs an empty icon view.
*/

QIconView::QIconView( QWidget *parent, const char *name, WFlags f )
    : QScrollView( parent, name, WNorthWestGravity | WRepaintNoErase  | f )
{
    if ( !unknown_icon ) {
	unknown_icon = new QPixmap( (const char **)unknown_xpm );
	qiv_cleanup_pixmap.add( unknown_icon );
    }

    d = new QIconViewPrivate;
    d->dragging = FALSE;
    d->firstItem = 0;
    d->lastItem = 0;
    d->count = 0;
    d->mousePressed = FALSE;
    d->selectionMode = Single;
    d->currentItem = 0;
    d->highlightedItem = 0;
    d->rubber = 0;
    d->scrollTimer = 0;
    d->startDragItem = 0;
    d->tmpCurrentItem = 0;
    d->rastX = d->rastY = -1;
    d->spacing = 5;
    d->cleared = FALSE;
    d->arrangement = LeftToRight;
    d->resizeMode = Fixed;
    d->dropped = FALSE;
    d->adjustTimer = new QTimer( this );
    d->isIconDrag = FALSE;
#ifndef QT_NO_DRAGANDDROP
    d->iconDragData.clear();
#endif
    d->numDragItems = 0;
    d->updateTimer = new QTimer( this );
    d->cachedW = d->cachedH = 0;
    d->maxItemWidth = 100;
    d->maxItemTextLength = 255;
    d->inputTimer = new QTimer( this );
    d->currInputString = QString::null;
    d->dirty = FALSE;
    d->rearrangeEnabled = TRUE;
    d->itemTextPos = Bottom;
    d->reorderItemsWhenInsert = TRUE;
#ifndef QT_NO_CURSOR
    d->oldCursor = arrowCursor;
#endif
    d->resortItemsWhenInsert = FALSE;
    d->sortDirection = TRUE;
    d->wordWrapIconText = TRUE;
    d->cachedContentsX = d->cachedContentsY = -1;
    d->clearing = FALSE;
    d->fullRedrawTimer = new QTimer( this );
    d->itemTextBrush = NoBrush;
    d->drawAllBack = TRUE;
    d->fm = new QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();
    d->firstContainer = d->lastContainer = 0;
    d->containerUpdateLocked = FALSE;
    d->firstSizeHint = TRUE;
    d->selectAnchor = 0;

    connect( d->adjustTimer, SIGNAL( timeout() ),
	     this, SLOT( adjustItems() ) );
    connect( d->updateTimer, SIGNAL( timeout() ),
	     this, SLOT( slotUpdate() ) );
    connect( d->inputTimer, SIGNAL( timeout() ),
	     this, SLOT( clearInputString() ) );
    connect( d->fullRedrawTimer, SIGNAL( timeout() ),
	     this, SLOT( updateContents() ) );
    connect( this, SIGNAL( contentsMoving( int, int ) ),
	     this, SLOT( movedContents( int, int ) ) );

    setAcceptDrops( TRUE );
    viewport()->setAcceptDrops( TRUE );

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    viewport()->setBackgroundMode( PaletteBase );
    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( QWidget::WheelFocus );

    d->toolTip = new QIconViewToolTip( viewport(), this );
    d->showTips = TRUE;
}

/*!
  \reimp
*/

void QIconView::styleChange( QStyle& old )
{
    QScrollView::styleChange( old );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }

    delete qiv_selection;
    qiv_selection = 0;
}

/*!
  \reimp
*/

void QIconView::setFont( const QFont & f )
{
    QScrollView::setFont( f );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
}

/*!
  \reimp
*/

void QIconView::setPalette( const QPalette & p )
{
    QScrollView::setPalette( p );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
}

/*!
  Destructs the icon view and deletes all items.
*/

QIconView::~QIconView()
{
    QIconViewItem *tmp, *item = d->firstItem;
    d->clearing = TRUE;
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }
    delete d->fm;
    d->fm = 0;
    delete d->toolTip;
    d->toolTip = 0;
    delete d;
}

/*!
  Inserts the icon view item \a item after \a after. If \a after is 0,
  \a item is appended.

  You should never need to call this function yourself; you should do

  \code
    (void) new QIconViewItem( iconview, "This is the text of the item", pixmap );
  \endcode

  This does everything required for inserting an item.
*/

void QIconView::insertItem( QIconViewItem *item, QIconViewItem *after )
{
    if ( !item )
	return;

    if ( !d->firstItem ) {
	d->firstItem = d->lastItem = item;
	item->prev = 0;
	item->next = 0;
    } else {
	if ( !after || after == d->lastItem ) {
	    d->lastItem->next = item;
	    item->prev = d->lastItem;
	    item->next = 0;
	    d->lastItem = item;
	} else {
	    QIconViewItem *i = d->firstItem;
	    while ( i != after )
		i = i->next;

	    if ( i ) {
		QIconViewItem *next = i->next;
		item->next = next;
		item->prev = i;
		i->next = item;
		next->prev = item;
	    }
	}
    }

    if ( isVisible() ) {
	if ( d->reorderItemsWhenInsert ) {
	    if ( d->updateTimer->isActive() )
		d->updateTimer->stop();
	    d->fullRedrawTimer->stop();
	    // #### uncomment this ASA insertInGrid uses cached values and is efficient
	    //insertInGrid( item );

	    d->cachedW = QMAX( d->cachedW, item->x() + item->width() );
	    d->cachedH= QMAX( d->cachedH, item->y() + item->height() );

	    d->updateTimer->start( 100, TRUE );
	} else {
	    insertInGrid( item );
	}
    }

    d->count++;
    d->dirty = TRUE;
}

/*!  Because of efficiency, the icon view is not redrawn immediately
  after inserting a new item but with a very small delay using a
  QTimer. Hence, when many items are inserted in a loop, the icon view
  is probably redrawn only once, at the end of the loop. This makes
  the loop flicker-free and much faster.

  This slot is used for the slightly-delayed update.
*/

void QIconView::slotUpdate()
{
    d->updateTimer->stop();
    d->fullRedrawTimer->stop();

    if ( !d->firstItem || !d->lastItem )
	return;

    // #### remove that ASA insertInGrid uses cached values and is efficient
    if ( d->resortItemsWhenInsert )
	sort( d->sortDirection );
    else {
	int y = d->spacing;
	QIconViewItem *item = d->firstItem;
	int w = 0, h = 0;
	while ( item ) {
	    QIconViewItem *next = makeRowLayout( item, y );

	    if ( !next || !next->next )
		break;

	    if( !QApplication::reverseLayout() )
		item = next;
	    w = QMAX( w, item->x() + item->width() );
	    h = QMAX( h, item->y() + item->height() );
	    item = next;
	    if ( d->arrangement == LeftToRight )
		h = QMAX( h, y );

	    item = item->next;
	}

	if ( d->lastItem && d->arrangement == TopToBottom ) {
	    item = d->lastItem;
	    int x = item->x();
	    while ( item && item->x() >= x ) {
		w = QMAX( w, item->x() + item->width() );
		h = QMAX( h, item->y() + item->height() );
		item = item->prev;
	    }
	}

	w = QMAX( QMAX( d->cachedW, w ), d->lastItem->x() + d->lastItem->width() );
	h = QMAX( QMAX( d->cachedH, h ), d->lastItem->y() + d->lastItem->height() );

	if ( d->arrangement == TopToBottom )
	    w += d->spacing;
	else
	    h += d->spacing;
	viewport()->setUpdatesEnabled( FALSE );
	resizeContents( w, h );
	viewport()->setUpdatesEnabled( TRUE );
	viewport()->repaint( FALSE );
    }

    int cx = d->cachedContentsX == -1 ? contentsX() : d->cachedContentsX;
    int cy = d->cachedContentsY == -1 ? contentsY() : d->cachedContentsY;

    if ( cx != contentsX() || cy != contentsY() )
	setContentsPos( cx, cy );

    d->cachedContentsX = d->cachedContentsY = -1;
    d->cachedW = d->cachedH = 0;
}

/*!
  Takes the icon view item \a item out of the icon view and causes an update
  of the screen display.  The item is not deleted.  You should normally not
  need to call this function because QIconViewItem::~QIconViewItem() calls it.
  The normal way to delete an item is to delete it.
*/

void QIconView::takeItem( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( item->d->container1 )
	item->d->container1->items.removeRef( item );
    if ( item->d->container2 )
	item->d->container2->items.removeRef( item );
    item->d->container2 = 0;
    item->d->container1 = 0;

    bool block = signalsBlocked();
    blockSignals( d->clearing );

    QRect r = item->rect();

    if ( d->currentItem == item ) {
	if ( item->prev ) {
	    d->currentItem = item->prev;
	    emit currentChanged( d->currentItem );
	    repaintItem( d->currentItem );
	} else if ( item->next ) {
	    d->currentItem = item->next;
	    emit currentChanged( d->currentItem );
	    repaintItem( d->currentItem );
	} else {
	    d->currentItem = 0;
	    emit currentChanged( d->currentItem );
	}
    }
    if ( item->isSelected() ) {
	item->selected = FALSE;
	emit selectionChanged();
    }

    if ( item == d->firstItem ) {
	d->firstItem = d->firstItem->next;
	if ( d->firstItem )
	    d->firstItem->prev = 0;
    } else if ( item == d->lastItem ) {
	d->lastItem = d->lastItem->prev;
	if ( d->lastItem )
	    d->lastItem->next = 0;
    } else {
	QIconViewItem *i = item;
	if ( i ) {
	    if ( i->prev )
		i->prev->next = i->next;
	    if ( i->next )
		i->next->prev = i->prev;
	}
    }

    if ( d->selectAnchor == item )
	d->selectAnchor = d->currentItem;

    if ( !d->clearing )
	repaintContents( r.x(), r.y(), r.width(), r.height(), TRUE );

    d->count--;

    blockSignals( block );
}

/*!
  Returns the index of \a item, or -1 if \a item doesn't exist
  in this icon view.
*/

int QIconView::index( const QIconViewItem *item ) const
{
    if ( !item )
	return -1;

    if ( item == d->firstItem )
	return 0;
    else if ( item == d->lastItem )
	return d->count - 1;
    else {
	QIconViewItem *i = d->firstItem;
	int j = 0;
	while ( i && i != item ) {
	    i = i->next;
	    ++j;
	}

	return i ? j : -1;
    }
}

/*!
  Returns a pointer to the first item of the icon view, or 0 if there
  are no items in the icon view.
*/

QIconViewItem *QIconView::firstItem() const
{
    return d->firstItem;
}

/*!
  Returns a pointer to the last item of the icon view, or 0 if there
  are no items in the icon view.
*/

QIconViewItem *QIconView::lastItem() const
{
    return d->lastItem;
}

/*!
  Returns a pointer to the current item of the icon view, or 0 if no
  item is current.
*/

QIconViewItem *QIconView::currentItem() const
{
    return d->currentItem;
}

/*!
  Makes \a item the new current item of the icon view.
*/

void QIconView::setCurrentItem( QIconViewItem *item )
{
    if ( !item || item == d->currentItem )
	return;
    QIconViewItem *old = d->currentItem;
    d->currentItem = item;
    emit currentChanged( d->currentItem );
    if ( d->selectionMode == Single ) {
	bool changed = FALSE;
	if ( old && old->selected ) {
	    old->selected = FALSE;
	    changed = TRUE;
	}
	if ( item && !item->selected && item->isSelectable() && d->selectionMode != NoSelection ) {
	    item->selected = TRUE;
	    changed = TRUE;
	    emit selectionChanged( item );
	}
	if ( changed )
	    emit selectionChanged();
    }

    if ( old )
	repaintItem( old );
    repaintItem( d->currentItem );
}

/*!
  Selects or unselects \a item depending on \a s, and may also
  unselect other items, depending on QIconView::selectionMode() and \a
  cb.

  If \a s is FALSE, \a item is unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Single, \a item
  is selected, and the item which was selected is unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Extended, \a
  item is selected. If \a cb is TRUE, the other items of the icon view
  are not touched. If \a cb is FALSE (the default) all other items are
  unselected.

  If \a s is TRUE and QIconView::selectionMode() is \c Multi \a item
  is selected.

  Note that \a cb is used only if QIconView::selectionMode() is \c
  Extended. \a cb defaults to FALSE.

  All items whose selection status change repaint themselves.
*/

void QIconView::setSelected( QIconViewItem *item, bool s, bool cb )
{
    if ( !item )
	return;
    item->setSelected( s, cb );
}

/*!
  Returns the number of items in the icon view.
*/

uint QIconView::count() const
{
    return d->count;
}

/*!
  Does autoscrolling when selecting multiple icons with the rubber band.
*/

void QIconView::doAutoScroll()
{
    QRect oldRubber = QRect( *d->rubber );

    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    d->rubber->setRight( pos.x() );
    d->rubber->setBottom( pos.y() );

    int minx = contentsWidth(), miny = contentsHeight();
    int maxx = 0, maxy = 0;
    bool changed = FALSE;
    bool block = signalsBlocked();

    QRect rr;
    QRegion region( 0, 0, visibleWidth(), visibleHeight() );

    blockSignals( TRUE );
    viewport()->setUpdatesEnabled( FALSE );
    bool alreadyIntersected = FALSE;
    QRect nr = d->rubber->normalize();
    QRect rubberUnion = nr.unite( oldRubber.normalize() );
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( rubberUnion ) ) {
	    alreadyIntersected = TRUE;
	    QIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( d->selectedItems.find( item ) )
		    continue;
		if ( !item->intersects( nr ) ) {
		    if ( item->isSelected() ) {
			item->setSelected( FALSE );
			changed = TRUE;
			rr = rr.unite( item->rect() );
		    }
		} else if ( item->intersects( nr ) ) {
		    if ( !item->isSelected() ) {
			item->setSelected( TRUE, TRUE );
			changed = TRUE;
			rr = rr.unite( item->rect() );
		    } else {
			region = region.subtract( QRect( contentsToViewport( item->pos() ),
							 item->size() ) );
		    }

		    minx = QMIN( minx, item->x() - 1 );
		    miny = QMIN( miny, item->y() - 1 );
		    maxx = QMAX( maxx, item->x() + item->width() + 1 );
		    maxy = QMAX( maxy, item->y() + item->height() + 1 );
		}
	    }
	} else {
 	    if ( alreadyIntersected )
 		break;
	}
    }
    viewport()->setUpdatesEnabled( TRUE );
    blockSignals( block );

    QRect r = *d->rubber;
    *d->rubber = oldRubber;

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    d->dragging = FALSE;
    p.end();

    *d->rubber = r;

    if ( changed ) {
	d->drawAllBack = FALSE;
	d->clipRegion = region;
	repaintContents( rr, FALSE );
	d->drawAllBack = TRUE;
    }

    ensureVisible( pos.x(), pos.y() );

    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    d->dragging = TRUE;

    p.end();

    if ( changed ) {
	emit selectionChanged();
	if ( d->selectionMode == Single )
	    emit selectionChanged( d->currentItem );
    }

    if ( !QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
	 !d->scrollTimer ) {
	d->scrollTimer = new QTimer( this );

	connect( d->scrollTimer, SIGNAL( timeout() ),
		 this, SLOT( doAutoScroll() ) );
	d->scrollTimer->start( 100, FALSE );
    } else if ( QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
		d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL( timeout() ),
		    this, SLOT( doAutoScroll() ) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

}

/*!
  \reimp
*/

void QIconView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( d->dragging &&& d->rubber )
	drawRubber( p );

    QRect r = QRect( cx, cy, cw, ch );

    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QRegion remaining( QRect( cx, cy, cw, ch ) );
    bool alreadyIntersected = FALSE;
    while ( c ) {
	if ( c->rect.intersects( r ) ) {
	    p->save();
	    p->resetXForm();
	    QRect r2 = c->rect;
	    r2 = r2.intersect( r );
	    QRect r3( contentsToViewport( QPoint( r2.x(), r2.y() ) ), QSize( r2.width(), r2.height() ) );
	    if ( d->drawAllBack ) {
		p->setClipRect( r3 );
	    } else {
		QRegion reg = d->clipRegion.intersect( r3 );
		p->setClipRegion( reg );
	    }
	    drawBackground( p, r3 );
	    remaining = remaining.subtract( r3 );
	    p->restore();

	    QIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( item->rect().intersects( r ) && !item->dirty ) {
		    p->save();
		    p->setFont( font() );
		    item->paintItem( p, colorGroup() );
		    p->restore();
		}
	    }
	    alreadyIntersected = TRUE;
	} else {
	    if ( alreadyIntersected )
		break;
	}
	c = c->n;
    }

    if ( !remaining.isNull() && !remaining.isEmpty() ) {
	p->save();
	p->resetXForm();
	if ( d->drawAllBack ) {
	    p->setClipRegion( remaining );
	} else {
	    remaining = d->clipRegion.intersect( remaining );
	    p->setClipRegion( remaining );
	}
	drawBackground( p, remaining.boundingRect() );
	p->restore();
    }

    if ( ( hasFocus() || viewport()->hasFocus() ) && d->currentItem &&
	 d->currentItem->rect().intersects( r ) ) {
	d->currentItem->paintFocus( p, colorGroup() );
    }

    if ( d->dragging &&& d->rubber )
	drawRubber( p );
}

/*!
  Arranges all items in the grid given by gridX() and gridY().

  Even if sorting() is enabled, the items are not resorted by this
  function. If you want to sort or rearrange all items, use
  iconview->sort(iconview->sortDirection()).

  If \a update is TRUE (the default), the viewport is repainted as
  well.

  \sa QIconView::setGridX(), QIconView::setGridY(), QIconView::sort()
*/

void QIconView::arrangeItemsInGrid( bool update )
{
    if ( !d->firstItem || !d->lastItem )
	return;

    d->containerUpdateLocked = TRUE;

    int w = 0, h = 0, y = d->spacing;

    QIconViewItem *item = d->firstItem;
    while ( item ) {
	QIconViewItem *next = makeRowLayout( item, y );
	if( !QApplication::reverseLayout() )
	    item = next;
	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
	item = next;
	if ( d->arrangement == LeftToRight )
	    h = QMAX( h, y );

	if ( !item || !item->next )
	    break;

	item = item->next;
    }

    if ( d->lastItem && d->arrangement == TopToBottom ) {
	item = d->lastItem;
	int x = item->x();
	while ( item && item->x() >= x ) {
	    w = QMAX( w, item->x() + item->width() );
	    h = QMAX( h, item->y() + item->height() );
	    item = item->prev;
	}
    }
    d->containerUpdateLocked = FALSE;

    w = QMAX( QMAX( d->cachedW, w ), d->lastItem->x() + d->lastItem->width() );
    h = QMAX( QMAX( d->cachedH, h ), d->lastItem->y() + d->lastItem->height() );

    if ( d->arrangement == TopToBottom )
	w += d->spacing;
    else
	h += d->spacing;

    viewport()->setUpdatesEnabled( FALSE );
    int vw = visibleWidth();
    int vh = visibleHeight();
    resizeContents( w, h );
    bool doAgain = FALSE;
    if ( d->arrangement == LeftToRight )
	doAgain = visibleWidth() != vw;
    if ( d->arrangement == TopToBottom )
	doAgain = visibleHeight() != vh;
    if ( doAgain ) // in the case that the visibleExtend changed because of the resizeContents (scrollbar show/hide), redo layout again
	arrangeItemsInGrid( FALSE );
    viewport()->setUpdatesEnabled( TRUE );
    d->dirty = FALSE;
    rebuildContainers();
    if ( update )
	repaintContents( contentsX(), contentsY(), viewport()->width(), viewport()->height(), FALSE );
}

// ### ### why two seeming overloads? neither seem to call each other...

/*! \overload

  This variant uses \a grid instead of (gridX(),gridY()).  If \a grid
  is invalid (see QSize::isValid()), arrangeItemsInGrid() calculates a
  good grid itself and uses that.

  If \a update is TRUE (the default) the viewport is repainted.

*/

void QIconView::arrangeItemsInGrid( const QSize &grid, bool update )
{
    d->containerUpdateLocked = TRUE;
    QSize grid_( grid );
    if ( !grid_.isValid() ) {
	int w = 0, h = 0;
	QIconViewItem *item = d->firstItem;
	for ( ; item; item = item->next ) {
	    w = QMAX( w, item->width() );
	    h = QMAX( h, item->height() );
	}

	grid_ = QSize( QMAX( d->rastX + d->spacing, w ),
		       QMAX( d->rastY + d->spacing, h ) );
    }

    int w = 0, h = 0;
    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	int nx = item->x() / grid_.width();
	int ny = item->y() / grid_.height();
	item->move( nx * grid_.width(),
		    ny * grid_.height() );
	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
	item->dirty = FALSE;
    }
    d->containerUpdateLocked = FALSE;

    resizeContents( w, h );
    rebuildContainers();
    if ( update )
	repaintContents( contentsX(), contentsY(), viewport()->width(), viewport()->height(), FALSE );
}

/*!
  \reimp
*/

void QIconView::setContentsPos( int x, int y )
{
    if ( d->updateTimer->isActive() ) {
	d->cachedContentsX = x;
	d->cachedContentsY = y;
    } else {
	d->cachedContentsY = d->cachedContentsX = -1;
	QScrollView::setContentsPos( x, y );
    }
}

/*!
  \reimp
*/

void QIconView::showEvent( QShowEvent * )
{
    if ( d->dirty ) {
	resizeContents( viewport()->width(), viewport()->height() );
	arrangeItemsInGrid( FALSE );
    }
    QScrollView::show();
}

/*!
  Sets the selection mode of the icon view to \a m, which may be one of
\c Single (the default), \c Extended, \c Multi or \c NoSelection.

  \sa selectionMode()
*/

void QIconView::setSelectionMode( SelectionMode m )
{
    d->selectionMode = m;
}

/*!
  Returns the selection mode of the icon view.  The initial mode is
  \c Single.

  \sa setSelectionMode()
*/

QIconView::SelectionMode QIconView::selectionMode() const
{
    return d->selectionMode;
}

/*!
  Returns a pointer to the item that contains \a pos, which is given
  on contents coordinates.
*/

QIconViewItem *QIconView::findItem( const QPoint &pos ) const
{
    if ( !d->firstItem )
	return 0;

    QIconViewPrivate::ItemContainer *c = d->lastContainer;
    for ( ; c; c = c->p ) {
	if ( c->rect.contains( pos ) ) {
	    QIconViewItem *item = c->items.last();
	    for ( ; item; item = c->items.prev() )
		if ( item->contains( pos ) )
		    return item;
	}
    }

    return 0;
}

/*!
  Returns a pointer to the first item which could be found that contains
  \a text, or 0 if no such item could be found.
*/

QIconViewItem *QIconView::findItem( const QString &text ) const
{
    if ( !d->firstItem )
	return 0;

    QIconViewItem *item = d->currentItem;
    for ( ; item; item = item->next ) {
	if ( item->text().lower().left( text.length() ) == text )
	    return item;
    }

    item = d->firstItem;
    for ( ; item && item != d->currentItem; item = item->next ) {
	if ( item->text().lower().left( text.length() ) == text )
	    return item;
    }

    return 0;
}

/*!
  Unselects all items.
*/

void QIconView::clearSelection()
{
    selectAll( FALSE );
}

/*!  In Multi and Extended modes, this function sets all items to be
  selected if \a select is TRUE, and to be unselected if \a select is
  FALSE.

  In Single and NoSelection modes, this function only changes the
  selection status of currentItem().
*/

void QIconView::selectAll( bool select )
{
    if ( d->selectionMode == NoSelection )
	return;

    if ( d->selectionMode == Single ) {
	if ( d->currentItem )
	    d->currentItem->setSelected( select );
	return;
    }

    bool b = signalsBlocked();
    blockSignals( TRUE );
    QIconViewItem *item = d->firstItem;
    QIconViewItem *i = d->currentItem;
    bool changed = FALSE;
    bool ue = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled( FALSE );
    QRect rr;
    for ( ; item; item = item->next ) {
	if ( select != item->isSelected() ) {
	    item->setSelected( select, TRUE );
	    rr = rr.unite( item->rect() );
	    changed = TRUE;
	}
    }
    viewport()->setUpdatesEnabled( ue );
    repaintContents( rr, FALSE );
    if ( i )
	setCurrentItem( i );
    blockSignals( b );
    if ( changed ) {
	emit selectionChanged();
    }
}

/*!
  Inverts the selection. Works only in Multi and Extended selection mode.
*/

void QIconView::invertSelection()
{
    if ( d->selectionMode == Single ||
	 d->selectionMode == NoSelection )
	return;

    bool b = signalsBlocked();
    blockSignals( TRUE );
    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	item->setSelected( !item->isSelected(), TRUE );
    blockSignals( b );
    emit selectionChanged();
}

/*!
  Repaints the \a item.
*/

void QIconView::repaintItem( QIconViewItem *item )
{
    if ( !item || item->dirty )
	return;

    if ( QRect( contentsX(), contentsY(), visibleWidth(), visibleHeight() ).
	 intersects( QRect( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 ) ) )
	repaintContents( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2, FALSE );
}

/*!  Makes sure that \a item is entirely visible. If necessary,
  ensureItemVisible() scrolls the icon view.

  \sa ensureVisible()
*/

void QIconView::ensureItemVisible( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( d->updateTimer && d->updateTimer->isActive() ||
	 d->fullRedrawTimer && d->fullRedrawTimer->isActive() )
	slotUpdate();

    int w = item->width();
    int h = item->height();
    ensureVisible( item->x() + w / 2, item->y() + h / 2,
		   w / 2 + 1, h / 2 + 1 );
}

/*! Finds the first item whose bounding rectangle overlaps \a r and
  returns a pointer to that item.  \a r is given in content
  coordinates.

  If you want to find all items that touch \a r, you must use this
  function, nextItem() in a loop ending at findLastVisibleItem() and
  test on QItem::rect() for each of those items.

  \sa findLastItem() QIconViewItem::rect()
*/

QIconViewItem* QIconView::findFirstVisibleItem( const QRect &r ) const
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QIconViewItem *i = 0;
    bool alreadyIntersected = FALSE;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( r ) ) {
	    alreadyIntersected = TRUE;
	    QIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( r.intersects( item->rect() ) ) {
		    if ( !i ) {
			i = item;
		    } else {
			QRect r2 = item->rect();
			QRect r3 = i->rect();
			if ( r2.y() < r3.y() )
			    i = item;
			else if ( r2.y() == r3.y() &&
				  r2.x() < r3.x() )
			    i = item;
		    }
		}
	    }
	} else {
	    if ( alreadyIntersected )
		break;
	}
    }

    return i;
}

/*! Finds the last item whose bounding rectangle overlaps \a r and
  returns a pointer to that item.  \a r is given in content
  coordinates.

  \sa findFirstVisibleItem()
*/

QIconViewItem* QIconView::findLastVisibleItem( const QRect &r ) const
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    QIconViewItem *i = 0;
    bool alreadyIntersected = FALSE;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( r ) ) {
	    alreadyIntersected = TRUE;
	    QIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( r.intersects( item->rect() ) ) {
		    if ( !i ) {
			i = item;
		    } else {
			QRect r2 = item->rect();
			QRect r3 = i->rect();
			if ( r2.y() > r3.y() )
			    i = item;
			else if ( r2.y() == r3.y() &&
				  r2.x() > r3.x() )
			    i = item;
		    }
		}
	    }
	} else {
	    if ( alreadyIntersected )
		break;
	}
    }

    return i;
}

/*!
  Clears the icon view. All items are deleted.
*/

void QIconView::clear()
{
    d->clearing = TRUE;
    blockSignals( TRUE );
    clearSelection();
    blockSignals( FALSE );
    setContentsPos( 0, 0 );
    d->currentItem = 0;

    if ( !d->firstItem ) {
	d->clearing = FALSE;
	return;
    }

    QIconViewItem *item = d->firstItem, *tmp;
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    d->count = 0;
    d->firstItem = 0;
    d->lastItem = 0;
    setCurrentItem( 0 );
    d->highlightedItem = 0;
    d->tmpCurrentItem = 0;
    d->drawDragShapes = FALSE;

    resizeContents( 0, 0 );
    // maybe we don�t need this update, so delay it
    d->fullRedrawTimer->start( 0, TRUE );

    d->cleared = TRUE;
    d->clearing = FALSE;
}

/*!  Sets the horizontal grid width to \a rx.  If \a rx is -1, as it
  is initially, QIconView computes suitable column widths based on
  the icon view's contents.

  Note that setting a grid width overrides setMaxItemWidth().

  \sa setGridY() gridX() setMaxItemWidth()
*/

void QIconView::setGridX( int rx )
{ // ### bug 0
    d->rastX = rx >= 0 ? rx : -1;
}

/*!  Sets the vertical grid width to \a ry.  If \a ry is -1, as it is
  initially, QIconView computes a suitable row heights based on the
  icon view's contents.

  \sa setGridX() gridY()
*/

void QIconView::setGridY( int ry )
{ // ### bug 0
    d->rastY = ry >= 0 ? ry : -1;
}

/*!
  Returns the horizontal grid.

  \sa QIconView::setGridX()
*/

int QIconView::gridX() const
{
    return d->rastX;
}

/*!
  Returns the vertical grid size (the row height).

  \sa QIconView::setGridY()
*/

int QIconView::gridY() const
{
    return d->rastY;
}

/*!
  Sets the space between icon view items to \a sp.
*/

void QIconView::setSpacing( int sp )
{
    d->spacing = sp;
}

/*!
  Returns the spacing between icon view items.
*/

int QIconView::spacing() const
{
    return d->spacing;
}

/*!
  Sets the position where the text of each item is drawn. This can be Bottom
  or Right. The default is Bottom.

  \sa ItemTextPos itemTextPos()
*/

void QIconView::setItemTextPos( ItemTextPos pos )
{
    if ( pos == d->itemTextPos || ( pos != Bottom && pos != Right ) )
	return;

    d->itemTextPos = pos;

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }

    arrangeItemsInGrid( TRUE );
}

/*! Returns Bottom if each item's text is drawn below its icon, and
  Right if it is drawn to the right of its icon.

  \sa QIconView::setItemTextPos() ItemTextPos
*/

QIconView::ItemTextPos QIconView::itemTextPos() const
{
    return d->itemTextPos;
}

/*!
  Sets the \a brush that should be used when drawing the background
  of an item text. By default this brush is set to NoBrush, meaning that
  only the normal icon view background is used.
*/

void QIconView::setItemTextBackground( const QBrush &brush )
{
    d->itemTextBrush = brush;
}

/*!
  Returns the brush that is used to draw the background of an item text.

  \sa setItemTextBackground()
*/

QBrush QIconView::itemTextBackground() const
{
    return d->itemTextBrush;
}

/*!
  Sets the arrangement mode of the icon view to \a am, which must be
  one of \c LeftToRight and TopToBottom.

  \sa Arrangement
*/

void QIconView::setArrangement( Arrangement am )
{
    if ( d->arrangement == am )
	return;

    d->arrangement = am;

    viewport()->setUpdatesEnabled( FALSE );
    resizeContents( viewport()->width(), viewport()->height() );
    viewport()->setUpdatesEnabled( TRUE );
    arrangeItemsInGrid( TRUE );
}

/*!
  Returns the arrangement mode of the icon view.

  \sa QIconView::setArrangement()
*/

QIconView::Arrangement QIconView::arrangement() const
{
    return d->arrangement;
}

/*!
  Sets the resize mode of the icon view to \a rm, which must be one of
  \c Fixed and Adjust.

  \sa ResizeMode
*/

void QIconView::setResizeMode( ResizeMode rm )
{
    if ( d->resizeMode == rm )
	return;

    d->resizeMode = rm;
}

/*!
  Returns the resize mode of the icon view.

  \sa QIconView::setResizeMode()
*/

QIconView::ResizeMode QIconView::resizeMode() const
{
    return d->resizeMode;
}

/*!
  Sets the maximum width that an item may have.

  Note that if gridX() value is set, QIconView mostly ignores
  maxItemWidth().
*/

void QIconView::setMaxItemWidth( int w )
{
    d->maxItemWidth = w;
}

/*!  Sets the maximum length (in characters) that an item text may
  have. The default is 255 characters.
*/

void QIconView::setMaxItemTextLength( int w )
{
    d->maxItemTextLength = w;
}

/*!  Returns the maximum width (in pixels) that an item may have. This
  may be the value set using setMaxItemWidth(), but may also be
  derived from gridX() if gridX() overrides setMaxItemWidth().

  \sa QIconView::setMaxItemWidth() setGridX()
*/

int QIconView::maxItemWidth() const
{
    if ( d->rastX != -1 )
	return d->rastX - 2;
    else
	return d->maxItemWidth;
}

/*!
  Returns the maximum length (in characters) that the
  text of an icon may have.

  \sa QIconView::setMaxItemTextLength()
*/

int QIconView::maxItemTextLength() const
{
    return d->maxItemTextLength;
}

/*!
  If \a b is TRUE, the user is allowed to move items around in
  the icon view.
  if \a b is FALSE, the user is not allowed to do that.
*/

void QIconView::setItemsMovable( bool b )
{
    d->rearrangeEnabled = b;
}

/*!
  Returns TRUE if the user is allowed to move items around
  in the icon view, otherwise FALSE. The initial setting is TRUE.

  \sa QIconView::setItemsMovable()
*/

bool QIconView::itemsMovable() const
{
    return d->rearrangeEnabled;
}

/*! Sets the icon view to rearrange its items when a new item is
  inserted if \a b is TRUE, and to simply find a spot for the new item
  if \a b is FALSE. The initial setting is TRUE.

  Note that if the icon view is not visible at the time of insertion,
  QIconView defers all position-related work until it's shown and then
  calls arrangeItemsInGrid().
*/

void QIconView::setAutoArrange( bool b )
{
    d->reorderItemsWhenInsert = b;
}

/*! Returns TRUE if inserting a new item triggers rearrangement of the
other items, and FALSE if it does not.

  \sa QIconView::setAutoArrange()
*/

bool QIconView::autoArrange() const
{
    return d->reorderItemsWhenInsert;
}

/*! If \a sort is TRUE, this function sets the icon view to resort
  items when a new item is inserted. If \a sort is FALSE, the icon
  view will not sort.

  Note that autoArrange() has to be set for this to work.

  If \a ascending is TRUE, items are sorted in ascending order. If \a
  ascending is FALSE, items are sorted in descending order.

  \sa QIconView::setAutoArrange(), QIconView::autoArrange() sortDirection() sort()
*/

void QIconView::setSorting( bool sort, bool ascending )
{
    d->resortItemsWhenInsert = sort;
    d->sortDirection = ascending;
}

/*! Returns TRUE if the icon view resorts on insertion and FALSE otherwise.

  \sa QIconView::setSorting()
*/

bool QIconView::sorting() const
{
    return d->resortItemsWhenInsert;
}

/*!
  Returns TRUE if the sort direction for inserting new items is ascending;
  FALSE means descending. This sort direction only has meaning if sorting()
  and autoArrange() are both TRUE.

  \sa QIconView::setSorting(), QIconView::setAutoArrange()
*/

bool QIconView::sortDirection() const
{
    return d->sortDirection;
}

/*! If \a b is TRUE, this function sets the icon view to display text
  wrapped when necessary, and if \a b is FALSE, it sets the icon view
  to show the text truncated.

  If an item's text fits on a single line, this setting is irrelevant.

  The initial value is TRUE.

  \sa setShowToolTips()
*/

void QIconView::setWordWrapIconText( bool b )
{
    if ( d->wordWrapIconText == b )
	return;

    d->wordWrapIconText = b;
    for ( QIconViewItem *item = d->firstItem; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
    arrangeItemsInGrid( TRUE );
}

/*! Returns TRUE if a too-long item text is word-wrapped, and FALSE if
  it is shown truncated.

  \sa setWordWrapIconText(), setShowToolTips()
*/

bool QIconView::wordWrapIconText() const
{
    return d->wordWrapIconText;
}

/*! Sets the icon view to display a tool tip with complete text for
  any truncated item text if \a b is TRUE, and to not show such tool
  tips if \a b is FALSE.

  The initial setting is TRUE. Note that this has no effect if
  setWordWrapIconText() is TRUE, as it is by default.

  \sa setWordWrapIconText()
*/

void QIconView::setShowToolTips( bool b )
{
    d->showTips = b;
}

/*!  Returns TRUE if a tool tip is shown for truncated item texts, and
  FALSE if it is not.

  \sa setShowToolTips(), setWordWrapIconText()
*/

bool QIconView::showToolTips() const
{
    return d->showTips;
}

/*!
  \reimp
*/

void QIconView::contentsMousePressEvent( QMouseEvent *e )
{
    d->dragStartPos = e->pos();
    QIconViewItem *item = findItem( e->pos() );
    d->pressedItem = item;

    if ( item )
	d->selectAnchor = item;

    if ( d->currentItem )
	d->currentItem->renameItem();

    if ( !d->currentItem && !item && d->firstItem ) {
	d->currentItem = d->firstItem;
	repaintItem( d->firstItem );
    }

    d->startDragItem = 0;

    if ( e->button() == LeftButton && !( e->state() & ShiftButton ) &&
	 !( e->state() & ControlButton ) && item && item->isSelected() &&
	 item->textRect( FALSE ).contains( e->pos() ) ) {

	if ( !item->renameEnabled() ) {
	    d->mousePressed = TRUE;
	} else {
	    ensureItemVisible( item );
	    setCurrentItem( item );
	    item->rename();
	    goto emit_signals;
	}
    }

    d->pressedSelected = item && item->isSelected();

    if ( item && item->isSelectable() ) {
	if ( d->selectionMode == Single )
	    item->setSelected( TRUE, e->state() & ControlButton );
	else if ( d->selectionMode == Multi )
	    item->setSelected( !item->isSelected(), e->state() & ControlButton );
	else if ( d->selectionMode == Extended ) {
	    if ( e->state() & ShiftButton ) {
		d->pressedSelected = FALSE;
		bool block = signalsBlocked();
		blockSignals( TRUE );
		viewport()->setUpdatesEnabled( FALSE );
		QRect r;
		bool select = TRUE;
		if ( d->currentItem )
		    r = QRect( QMIN( d->currentItem->x(), item->x() ),
			       QMIN( d->currentItem->y(), item->y() ),
			       0, 0 );
		else
		    r = QRect( 0, 0, 0, 0 );
		if ( d->currentItem ) {
		    if ( d->currentItem->x() < item->x() )
			r.setWidth( item->x() - d->currentItem->x() + item->width() );
		    else
			r.setWidth( d->currentItem->x() - item->x() + d->currentItem->width() );
		    if ( d->currentItem->y() < item->y() )
			r.setHeight( item->y() - d->currentItem->y() + item->height() );
		    else
			r.setHeight( d->currentItem->y() - item->y() + d->currentItem->height() );
		    r = r.normalize();
		    QIconViewPrivate::ItemContainer *c = d->firstContainer;
		    bool alreadyIntersected = FALSE;
		    QRect redraw;
		    for ( ; c; c = c->n ) {
			if ( c->rect.intersects( r ) ) {
			    alreadyIntersected = TRUE;
			    QIconViewItem *i = c->items.first();
			    for ( ; i; i = c->items.next() ) {
				if ( r.intersects( i->rect() ) ) {
				    redraw = redraw.unite( i->rect() );
				    i->setSelected( select, TRUE );
				}
			    }
			} else {
			    if ( alreadyIntersected )
				break;
			}
		    }
		    redraw = redraw.unite( item->rect() );
		    viewport()->setUpdatesEnabled( TRUE );
		    repaintContents( redraw, FALSE );
		}
		blockSignals( block );
		viewport()->setUpdatesEnabled( TRUE );
		item->setSelected( select, TRUE );
		emit selectionChanged();
	    } else if ( e->state() & ControlButton ) {
		d->pressedSelected = FALSE;
		item->setSelected( !item->isSelected(), e->state() & ControlButton );
	    } else {
		item->setSelected( TRUE, e->state() & ControlButton );
	    }
	}
    } else if ( ( d->selectionMode != Single || e->button() == RightButton )
		&& !( e->state() & ControlButton ) )
	selectAll( FALSE );

    setCurrentItem( item );

    if ( e->button() == LeftButton ) {
	if ( !item && ( d->selectionMode == Multi ||
				  d->selectionMode == Extended ) ) {
	    d->tmpCurrentItem = d->currentItem;
	    d->currentItem = 0;
	    repaintItem( d->tmpCurrentItem );
	    if ( d->rubber )
		delete d->rubber;
	    d->rubber = 0;
	    d->rubber = new QRect( e->x(), e->y(), 0, 0 );
	    d->selectedItems.clear();
	    if ( ( e->state() & ControlButton ) == ControlButton ) {
		for ( QIconViewItem *i = firstItem(); i; i = i->nextItem() )
		    if ( i->isSelected() )
			d->selectedItems.insert( i, i );
	    }
	}

	d->mousePressed = TRUE;
    }

 emit_signals:
    if ( !d->rubber ) {
	emit mouseButtonPressed( e->button(), item, e->globalPos() );
	emit pressed( item );
	emit pressed( item, e->globalPos() );

	if ( e->button() == RightButton )
	    emit rightButtonPressed( item, e->globalPos() );
    }
}

/*!
  \reimp
*/

void QIconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    QIconViewItem *item = findItem( e->pos() );
    d->selectedItems.clear();

    bool emitClicked = TRUE;
    d->mousePressed = FALSE;
    d->startDragItem = 0;

    if ( d->rubber ) {
	emitClicked = FALSE;
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( color0, 1 ) );
	p.setBrush( NoBrush );

	drawRubber( &p );
	d->dragging = FALSE;
	p.end();

	if ( ( d->rubber->topLeft() - d->rubber->bottomRight() ).manhattanLength() >
	     QApplication::startDragDistance() )
	    emitClicked = FALSE;
	delete d->rubber;
	d->rubber = 0;
	d->currentItem = d->tmpCurrentItem;
	d->tmpCurrentItem = 0;
	if ( d->currentItem )
	    repaintItem( d->currentItem );
    }

    if ( d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

    if ( d->selectionMode == Extended &&
	 d->currentItem == d->pressedItem &&
	 d->pressedSelected && d->currentItem ) {
	bool block = signalsBlocked();
	blockSignals( TRUE );
	clearSelection();
	blockSignals( block );
	if ( d->currentItem->isSelectable() ) {
	    d->currentItem->selected = TRUE;
	    repaintItem( d->currentItem );
	}
	emit selectionChanged();
    }
    d->pressedItem = 0;

    if ( emitClicked ) {
	emit mouseButtonClicked( e->button(), item, e->globalPos() );
	emit clicked( item );
	emit clicked( item, e->globalPos() );
	if ( e->button() == RightButton ) {
	    emit rightButtonClicked( item, e->globalPos() );
	}
    }
}

/*!
  \reimp
*/

void QIconView::contentsMouseMoveEvent( QMouseEvent *e )
{
    QIconViewItem *item = findItem( e->pos() );
    if ( d->highlightedItem != item ) {
	if ( item )
	    emit onItem( item );
	else
	    emit onViewport();
	d->highlightedItem = item;
    }

    if ( d->mousePressed && e->state() == NoButton )
	d->mousePressed = FALSE;

    if ( d->startDragItem )
	item = d->startDragItem;

    if ( d->mousePressed && item && item == d->currentItem &&
	 ( item->isSelected() || d->selectionMode == NoSelection ) && item->dragEnabled() ) {
	if ( !d->startDragItem ) {
	    d->currentItem->setSelected( TRUE, TRUE );
	    d->startDragItem = item;
	}
	if ( ( d->dragStartPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
	    d->mousePressed = FALSE;
	    d->cleared = FALSE;
#ifndef QT_NO_DRAGANDDROP
	    startDrag();
#endif
	    if ( d->tmpCurrentItem )
		repaintItem( d->tmpCurrentItem );
	}
    } else if ( d->mousePressed && !d->currentItem && d->rubber ) {
	doAutoScroll();
    }
}

/*!
  \reimp
*/

void QIconView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    QIconViewItem *item = findItem( e->pos() );
    if ( item ) {
	selectAll( FALSE );
	item->setSelected( TRUE, TRUE );
	emit doubleClicked( item );
    }
}

/*!
  \reimp
*/

#ifndef QT_NO_DRAGANDDROP
void QIconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    d->dragging = TRUE;
    d->drawDragShapes = TRUE;
    d->tmpCurrentItem = 0;
    initDragEnter( e );
    d->oldDragPos = e->pos();
    drawDragShapes( e->pos() );
    d->dropped = FALSE;
}

/*!
  \reimp
*/

void QIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( e->pos() == d->oldDragPos )
	return;
    drawDragShapes( d->oldDragPos );
    d->dragging = FALSE;

    QIconViewItem *old = d->tmpCurrentItem;
    d->tmpCurrentItem = 0;

    QIconViewItem *item = findItem( e->pos() );

    if ( item ) {
	if ( old ) {
	    old->dragLeft();
	    repaintItem( old );
	}
	item->dragEntered();
	if ( item->acceptDrop( e ) )
	    e->acceptAction();
	else
	    e->ignore();

	d->tmpCurrentItem = item;
	QPainter p;
	p.begin( viewport() );
	p.translate( -contentsX(), -contentsY() );
	item->paintFocus( &p, colorGroup() );
	p.end();
    } else {
	e->acceptAction();
	if ( old ) {
	    old->dragLeft();
	    repaintItem( old );
	}
    }

    d->oldDragPos = e->pos();
    drawDragShapes( e->pos() );
    d->dragging = TRUE;
}

/*!
  \reimp
*/

void QIconView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    if ( !d->dropped )
	drawDragShapes( d->oldDragPos );
    d->dragging = FALSE;

    if ( d->tmpCurrentItem ) {
	repaintItem( d->tmpCurrentItem );
	d->tmpCurrentItem->dragLeft();
    }

    d->tmpCurrentItem = 0;
    d->isIconDrag = FALSE;
    d->iconDragData.clear();
}

/*!
  \reimp
*/

void QIconView::contentsDropEvent( QDropEvent *e )
{
    d->dropped = TRUE;
    d->dragging = FALSE;
    drawDragShapes( d->oldDragPos );

    if ( d->tmpCurrentItem )
	repaintItem( d->tmpCurrentItem );

    QIconViewItem *i = findItem( e->pos() );

    if ( !i && e->source() == viewport() && d->currentItem && !d->cleared ) {
	if ( !d->rearrangeEnabled )
	    return;
	QRect r = d->currentItem->rect();

	d->currentItem->move( e->pos() - d->dragStart );

	int w = d->currentItem->x() + d->currentItem->width() + 1;
	int h = d->currentItem->y() + d->currentItem->height() + 1;

	repaintItem( d->currentItem );
	repaintContents( r.x(), r.y(), r.width(), r.height(), FALSE );

	if ( d->selectionMode != Single ) {
	    int dx = d->currentItem->x() - r.x();
	    int dy = d->currentItem->y() - r.y();

	    QIconViewItem *item = d->firstItem;
	    QRect rr;
	    for ( ; item; item = item->next ) {
		if ( item->isSelected() && item != d->currentItem ) {
		    rr = rr.unite( item->rect() );
		    item->moveBy( dx, dy );
		    rr = rr.unite( item->rect() );
		    w = QMAX( w, item->x() + item->width() + 1 );
		    h = QMAX( h, item->y() + item->height() + 1 );
		}
	    }
	    repaintContents( rr, FALSE );
	}
	bool fullRepaint = FALSE;
	if ( w > contentsWidth() ||
	     h > contentsHeight() )
	    fullRepaint = TRUE;

	int oldw = contentsWidth();
	int oldh = contentsHeight();

	resizeContents( QMAX( contentsWidth(), w ), QMAX( contentsHeight(), h ) );

	if ( fullRepaint ) {
	    repaintContents( oldw, 0, contentsWidth() - oldw, contentsHeight(), FALSE );
	    repaintContents( 0, oldh, contentsWidth(), contentsHeight() - oldh, FALSE );
	}
	e->acceptAction();
    } else if ( !i && ( e->source() != viewport() || d->cleared ) ) {
	QValueList<QIconDragItem> lst;
	if ( QIconDrag::canDecode( e ) ) {
	    QValueList<QIconDragDataItem> l;
	    QIconDragPrivate::decode( e, l );
	    QValueList<QIconDragDataItem>::Iterator it = l.begin();
	    for ( ; it != l.end(); ++it )
		lst << ( *it ).data;
	}
	emit dropped( e, lst );
    } else if ( i ) {
	QValueList<QIconDragItem> lst;
	if ( QIconDrag::canDecode( e ) ) {
	    QValueList<QIconDragDataItem> l;
	    QIconDragPrivate::decode( e, l );
	    QValueList<QIconDragDataItem>::Iterator it = l.begin();
	    for ( ; it != l.end(); ++it )
		lst << ( *it ).data;
	}
	i->dropped( e, lst );
    }
    d->isIconDrag = FALSE;
}
#endif

/*!
  \reimp
*/

void QIconView::resizeEvent( QResizeEvent* e )
{
    QScrollView::resizeEvent( e );
    if ( d->resizeMode == Adjust ) {
	d->oldSize = e->oldSize();
	if ( d->adjustTimer->isActive() )
	    d->adjustTimer->stop();
	d->adjustTimer->start( 100, TRUE );
    }
}

/*!
  Adjusts the positions of the items to the geometry of the icon view.
*/

void QIconView::adjustItems()
{
    d->adjustTimer->stop();
    if ( d->resizeMode == Adjust )
	    arrangeItemsInGrid( TRUE );
}

/*!
  \reimp
*/

void QIconView::keyPressEvent( QKeyEvent *e )
{
    if ( !d->firstItem )
	return;

    if ( !d->currentItem ) {
	setCurrentItem( d->firstItem );
	if ( d->selectionMode == Single )
	    d->currentItem->setSelected( TRUE, TRUE );
	return;
    }

    bool selectCurrent = TRUE;

    switch ( e->key() ) {
    case Key_Escape:
	e->ignore();
	break;
    case Key_F2: {
	if ( d->currentItem->renameEnabled() ) {
	    d->currentItem->renameItem();
	    d->currentItem->rename();
	    return;
	}
    } break;
    case Key_Home: {
	d->currInputString = QString::null;
	if ( !d->firstItem )
	    break;

	selectCurrent = FALSE;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->firstItem );

	handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
    } break;
    case Key_End: {
	d->currInputString = QString::null;
	if ( !d->lastItem )
	    break;

	selectCurrent = FALSE;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->lastItem );

	handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
    } break;
    case Key_Right: {
	d->currInputString = QString::null;
	QIconViewItem *item;
	selectCurrent = FALSE;
	if ( d->arrangement == LeftToRight ) {
	    if ( !d->currentItem->next )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->next );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	} else {
	    item = d->firstItem;
	    QRect r( 0, d->currentItem->y(), contentsWidth(), d->currentItem->height() );
	    for ( ; item; item = item->next ) {
		if ( item->x() > d->currentItem->x() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->next && r.intersects( item->next->rect() ) ) {
			QRect irn = r.intersect( item->next->rect() );
			if ( irn.height() > ir.height() )
			    item = item->next;
		    }
		    QIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	}
    } break;
    case Key_Left: {
	d->currInputString = QString::null;
	QIconViewItem *item;
	selectCurrent = FALSE;
	if ( d->arrangement == LeftToRight ) {
	    if ( !d->currentItem->prev )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->prev );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	} else {
	    item = d->lastItem;
	    QRect r( 0, d->currentItem->y(), contentsWidth(), d->currentItem->height() );
	    for ( ; item; item = item->prev ) {
		if ( item->x() < d->currentItem->x() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->prev && r.intersects( item->prev->rect() ) ) {
			QRect irn = r.intersect( item->prev->rect() );
			if ( irn.height() > ir.height() )
			    item = item->prev;
		    }
		    QIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	}
    } break;
    case Key_Space: {
	d->currInputString = QString::null;
	if ( d->selectionMode == Single)
	    break;

	d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    } break;
    case Key_Enter: case Key_Return:
	d->currInputString = QString::null;
	emit returnPressed( d->currentItem );
	break;
    case Key_Down: {
	d->currInputString = QString::null;
	QIconViewItem *item;
	selectCurrent = FALSE;

	if ( d->arrangement == LeftToRight ) {
	    item = d->firstItem;
	    QRect r( d->currentItem->x(), 0, d->currentItem->width(), contentsHeight() );
	    for ( ; item; item = item->next ) {
		if ( item->y() > d->currentItem->y() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->next && r.intersects( item->next->rect() ) ) {
			QRect irn = r.intersect( item->next->rect() );
			if ( irn.width() > ir.width() )
			    item = item->next;
		    }
		    QIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	} else {
	    if ( !d->currentItem->next )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->next );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Up: {
	d->currInputString = QString::null;
	QIconViewItem *item;
	selectCurrent = FALSE;

	if ( d->arrangement == LeftToRight ) {
	    item = d->lastItem;
	    QRect r( d->currentItem->x(), 0, d->currentItem->width(), contentsHeight() );
	    for ( ; item; item = item->prev ) {
		if ( item->y() < d->currentItem->y() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->prev && r.intersects( item->prev->rect() ) ) {
			QRect irn = r.intersect( item->prev->rect() );
			if ( irn.width() > ir.width() )
			    item = item->prev;
		    }
		    QIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	} else {
	    if ( !d->currentItem->prev )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->prev );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Next: {
	d->currInputString = QString::null;
	selectCurrent = FALSE;
	QRect r;
	if ( d->arrangement == LeftToRight )
	    r = QRect( 0, d->currentItem->y() + visibleHeight(), contentsWidth(), visibleHeight() );
	else
	    r = QRect( d->currentItem->x() + visibleWidth(), 0, visibleWidth(), contentsHeight() );
	QIconViewItem *item = d->currentItem;
	QIconViewItem *ni = findFirstVisibleItem( r  );
	if ( !ni ) {
	    if ( d->arrangement == LeftToRight )
		r = QRect( 0, d->currentItem->y() + d->currentItem->height(), contentsWidth(), contentsHeight() );
	    else
		r = QRect( d->currentItem->x() + d->currentItem->width(), 0, contentsWidth(), contentsHeight() );
	    ni = findLastVisibleItem( r  );
	}
	if ( ni ) {
	    setCurrentItem( ni );
	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Prior: {
	d->currInputString = QString::null;
	selectCurrent = FALSE;
	QRect r;
	if ( d->arrangement == LeftToRight )
	    r = QRect( 0, d->currentItem->y() - visibleHeight(), contentsWidth(), visibleHeight() );
	else
	    r = QRect( d->currentItem->x() - visibleWidth(), 0, visibleWidth(), contentsHeight() );
	QIconViewItem *item = d->currentItem;
	QIconViewItem *ni = findFirstVisibleItem( r  );
	if ( !ni ) {
	    if ( d->arrangement == LeftToRight )
		r = QRect( 0, 0, contentsWidth(), d->currentItem->y() );
	    else
		r = QRect( 0, 0, d->currentItem->x(), contentsHeight() );
	    ni = findFirstVisibleItem( r  );
	}
	if ( ni ) {
	    setCurrentItem( ni );
	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    default:
	if ( !e->text().isEmpty() && e->text()[ 0 ].isPrint() ) {
	    selectCurrent = FALSE;
	    findItemByName( e->text() );
	} else {
	    selectCurrent = FALSE;
	    d->currInputString = QString::null;
	    if ( e->state() & ControlButton ) {
		switch ( e->key() ) {
		case Key_A:
		    selectAll( TRUE );
		    break;
		}
	    }
	}
    }

    if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
	d->selectAnchor = d->currentItem;

    if ( d->currentItem && !d->currentItem->isSelected() &&
	 d->selectionMode == Single && selectCurrent ) {
	d->currentItem->setSelected( TRUE );
    }

    if ( e->key() != Key_Shift &&
	 e->key() != Key_Control &&
	 e->key() != Key_Alt )
	ensureItemVisible( d->currentItem );
}

/*!
  \reimp
*/

void QIconView::focusInEvent( QFocusEvent *e )
{
    d->mousePressed = FALSE;
    if ( d->currentItem )
	repaintItem( d->currentItem );
    else if ( d->firstItem && e->reason() != QFocusEvent::Mouse ) {
	d->currentItem = d->firstItem;
	emit currentChanged( d->currentItem );
	repaintItem( d->currentItem );
    }
}

/*!
  \reimp
*/

void QIconView::focusOutEvent( QFocusEvent * )
{
    if ( d->currentItem )
	repaintItem( d->currentItem );
}

/*!
  Draws the rubber band using the painter \a p.
*/

void QIconView::drawRubber( QPainter *p )
{
    if ( !p || !d->rubber )
	return;

    QPoint pnt( d->rubber->x(), d->rubber->y() );
    pnt = contentsToViewport( pnt );
    style().drawFocusRect( p, QRect( pnt.x(), pnt.y(), d->rubber->width(), d->rubber->height() ),
			   colorGroup(), &colorGroup().base() );
}

/*!  Returns the QDragObject that should be used for
  drag-and-drop. This function is called by the icon view when
  starting a drag to get the dragobject which should be used for the
  drag.  Subclasses may reimplement this.

  \sa QIconDrag
*/

#ifndef QT_NO_DRAGANDDROP
QDragObject *QIconView::dragObject()
{
    if ( !d->currentItem )
	return 0;

    QPoint orig = d->dragStartPos;

    QIconDrag *drag = new QIconDrag( viewport() );
    drag->setPixmap( *d->currentItem->pixmap(),
 		     QPoint( d->currentItem->pixmapRect().width() / 2,
			     d->currentItem->pixmapRect().height() / 2 ) );

    if ( d->selectionMode == NoSelection ) {
	QIconViewItem *item = d->currentItem;
	drag->append( QIconDragItem(),
		      QRect( item->pixmapRect( FALSE ).x() - orig.x(),
			     item->pixmapRect( FALSE ).y() - orig.y(),
			     item->pixmapRect().width(), item->pixmapRect().height() ),
		      QRect( item->textRect( FALSE ).x() - orig.x(),
			     item->textRect( FALSE ).y() - orig.y(),
			     item->textRect().width(), item->textRect().height() ) );
    } else {
	for ( QIconViewItem *item = d->firstItem; item; item = item->next ) {
	    if ( item->isSelected() ) {
		drag->append( QIconDragItem(),
			      QRect( item->pixmapRect( FALSE ).x() - orig.x(),
				     item->pixmapRect( FALSE ).y() - orig.y(),
				     item->pixmapRect().width(), item->pixmapRect().height() ),
			      QRect( item->textRect( FALSE ).x() - orig.x(),
				     item->textRect( FALSE ).y() - orig.y(),
				     item->textRect().width(), item->textRect().height() ) );
	    }
	}
    }

    return drag;
}

/*!
  Starts a drag.
*/

void QIconView::startDrag()
{
    if ( !d->startDragItem )
	return;

    QPoint orig = d->dragStartPos;
    d->dragStart = QPoint( orig.x() - d->startDragItem->x(),
			   orig.y() - d->startDragItem->y() );
    d->startDragItem = 0;
    d->mousePressed = FALSE;
    d->pressedItem = 0;
    d->pressedSelected = 0;

    QDragObject *drag = dragObject();
    if ( !drag )
	return;

    if ( drag->drag() )
	if ( drag->target() != viewport() )
	    emit moved();
}

#endif

/*!
  Inserts an item in the grid of the icon view. You should never need
  to call this manually.
*/

void QIconView::insertInGrid( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( d->reorderItemsWhenInsert ) {
	// #### make this efficient - but it's not too dramatic
	int y = d->spacing;

	item->dirty = FALSE;
	if ( item == d->firstItem ) {
	    makeRowLayout( item, y );
	    return;
	}

	QIconViewItem *begin = rowBegin( item );
	y = begin->y();
	while ( begin ) {
	    begin = makeRowLayout( begin, y );

	    if ( !begin || !begin->next )
		break;

	    begin = begin->next;
	}
	item->dirty = FALSE;
    } else {
	QRegion r( QRect( 0, 0, QMAX( contentsWidth(), visibleWidth() ),
			  QMAX( contentsHeight(), visibleHeight() ) ) );

	QIconViewItem *i = d->firstItem;
	int y = -1;
	for ( ; i; i = i->next ) {
	    r = r.subtract( i->rect() );
	    y = QMAX( y, i->y() + i->height() );
	}

	QArray<QRect> rects = r.rects();
	QArray<QRect>::Iterator it = rects.begin();
	bool foundPlace = FALSE;
	for ( ; it != rects.end(); ++it ) {
	    QRect rect = *it;
	    if ( rect.width() >= item->width() &&
		 rect.height() >= item->height() ) {
		int sx = 0, sy = 0;
		if ( rect.width() >= item->width() + d->spacing )
		    sx = d->spacing;
		if ( rect.height() >= item->height() + d->spacing )
		    sy = d->spacing;
		item->move( rect.x() + sx, rect.y() + sy );
		foundPlace = TRUE;
		break;
	    }
	}

	if ( !foundPlace )
	    item->move( d->spacing, y + d->spacing );

	resizeContents( QMAX( contentsWidth(), item->x() + item->width() ),
			QMAX( contentsHeight(), item->y() + item->height() ) );
	item->dirty = FALSE;
    }
}

/*!  Emits signals that indicate selection changes. You should never
  need to call this.
*/

void QIconView::emitSelectionChanged( QIconViewItem *i )
{
    emit selectionChanged();
    if ( d->selectionMode == Single )
	emit selectionChanged( i ? i : d->currentItem );
}

/*!
  \internal
*/

void QIconView::emitRenamed( QIconViewItem *item )
{
    if ( !item )
	return;

    emit itemRenamed( item, item->text() );
    emit itemRenamed( item );
}

/*!
  If a drag enters the icon view, shapes of the objects, which the drag
  contains are drawn, usnig \a pos as origin.
*/

void QIconView::drawDragShapes( const QPoint &pos )
{
#ifndef QT_NO_DRAGANDDROP
    if ( pos == QPoint( -1, -1 ) )
	return;

    if ( !d->drawDragShapes ) {
	d->drawDragShapes = TRUE;
	return;
    }

    if ( d->isIconDrag ) {
	QPainter p;
	p.begin( viewport() );
	p.translate( -contentsX(), -contentsY() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( color0 ) );

	QValueList<QIconDragDataItem>::Iterator it = d->iconDragData.begin();
	for ( ; it != d->iconDragData.end(); ++it ) {
	    QRect ir = (*it).item.pixmapRect();
	    QRect tr = (*it).item.textRect();
	    tr.moveBy( pos.x(), pos.y() );
	    ir.moveBy( pos.x(), pos.y() );
	    if ( !ir.intersects( QRect( contentsX(), contentsY(), visibleWidth(), visibleHeight() ) ) )
		continue;
	    style().drawFocusRect( &p, ir, colorGroup(), &colorGroup().base() );
	    style().drawFocusRect( &p, tr, colorGroup(), &colorGroup().base() );
	}

	p.end();
    } else if ( d->numDragItems > 0 ) {
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( color0 ) );

	for ( int i = 0; i < d->numDragItems; ++i ) {
	    QRect r( pos.x() + i * 40, pos.y(), 35, 35 );
	    style().drawFocusRect( &p, r, colorGroup(), &colorGroup().base() );
	}

	p.end();
    }
#endif
}

/*!
  When a drag enters the icon view, this function is called to
  initialize it. Initializing means here to get information about
  the drag, this means if the icon view knows enough about
  the drag to be able to draw drag shapes of the drag data
  (e.g. shapes of icons which are dragged), etc.
*/

#ifndef QT_NO_DRAGANDDROP
void QIconView::initDragEnter( QDropEvent *e )
{
    if ( QIconDrag::canDecode( e ) ) {
	QIconDragPrivate::decode( e, d->iconDragData );
	d->isIconDrag = TRUE;
    } else if ( QUriDrag::canDecode( e ) ) {
	QStrList lst;
	QUriDrag::decode( e, lst );
	d->numDragItems = lst.count();
    } else {
	d->numDragItems = 0;
    }

}
#endif

/*!
  This function is called to draw the rectangle \a r of the background using
  the painter \a p.

  The default implementation fills \a r with
  colorGroup().base(). Subclasses may reimplement this to draw fancy
  backgrounds.

  \sa contentsX() contentsY() drawContents()
*/

void QIconView::drawBackground( QPainter *p, const QRect &r )
{
    p->fillRect( r, QBrush( colorGroup().base() ) );
}

/*!
  \reimp
*/

bool QIconView::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return FALSE;

    switch( e->type() ) {
    case QEvent::FocusIn:
	focusInEvent( (QFocusEvent*)e );
	return TRUE;
    case QEvent::FocusOut:
	focusOutEvent( (QFocusEvent*)e );
	return TRUE;
    case QEvent::Enter:
	enterEvent( e );
	return TRUE;
    case QEvent::Paint:
	if ( o == viewport() ) {
	    if ( d->dragging ) {
		if ( !d->rubber )
		    drawDragShapes( d->oldDragPos );
	    }
	    viewportPaintEvent( (QPaintEvent*)e );
	    if ( d->dragging ) {
		if ( !d->rubber )
		    drawDragShapes( d->oldDragPos );
	    }
	}
	return TRUE;
    default:
	// nothing
	break;
    }

    return QScrollView::eventFilter( o, e );
}


/*!
  \reimp
*/

QSize QIconView::minimumSizeHint() const
{
    return QSize( 100, 100 );
}

/*!
  \internal
  Clears string which is used for setting the current item
  when the user types something in
*/

void QIconView::clearInputString()
{
    d->currInputString = QString::null;
}

/*!
  \internal
  Finds the first item beginning with \a text and makes
  it the current one
*/

void QIconView::findItemByName( const QString &text )
{
    if ( d->inputTimer->isActive() )
	d->inputTimer->stop();
    d->inputTimer->start( 500, TRUE );
    d->currInputString += text.lower();
    QIconViewItem *item = findItem( d->currInputString );
    if ( item ) {
	setCurrentItem( item );
	if ( d->selectionMode == Extended ) {
	    bool changed = FALSE;
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( FALSE );
	    if ( !item->selected && item->isSelectable() ) {
		changed = TRUE;
		item->selected = TRUE;
		repaintItem( item );
	    }
	    if ( changed )
		emit selectionChanged();
	}
    }
}

/*!
  Lays out a row of icons (in Arrangement == TopToBottom this is a column). Starts
  laying out with the item \a begin. \a y is the starting coordinate.
  Returns the last item of the row and sets the new starting coordinate to \a y.

  This function may be made private in a future version of Qt. We
  recommend not calling it.
*/

QIconViewItem *QIconView::makeRowLayout( QIconViewItem *begin, int &y )
{
    QIconViewItem *end = 0;

    bool reverse = QApplication::reverseLayout();

    if ( d->arrangement == LeftToRight ) {

	if ( d->rastX == -1 ) {
	    // first calculate the row height
	    int h = 0;
	    int x = 0;
	    int ih = 0;
	    QIconViewItem *item = begin;
	    while ( TRUE ) {
		x += d->spacing + item->width();
		if ( x > visibleWidth() && item != begin ) {
		    h = QMAX( h, item->height() );
		    ih = QMAX( ih, item->pixmapRect().height() );
		    item = item->prev;
		    break;
		}
		h = QMAX( h, item->height() );
		ih = QMAX( ih, item->pixmapRect().height() );
		QIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastY != -1 )
		h = QMAX( h, d->rastY );

	    // now move the items
	    item = begin;
	    while ( TRUE ) {
		item->dirty = FALSE;
		int x;
		if ( item == begin ) {
		    if ( reverse )
			x = visibleWidth() - d->spacing - item->width();
		    else
			x = d->spacing;
		} else {
		    if ( reverse )
			x = item->prev->x() - item->width() - d->spacing;
		    else
			x = item->prev->x() + item->prev->width() + d->spacing;
		}
		item->move( x, y + ih - item->pixmapRect().height() );
		if ( y + h < item->y() + item->height() )
		    h = QMAX( h, ih + item->textRect().height() );
		if ( item == end )
		    break;
		item = item->next;
	    }
	    y += h + d->spacing;
	} else {
	    // first calculate the row height
	    int h = begin->height();
	    int x = d->spacing;
	    int ih = begin->pixmapRect().height();
	    QIconViewItem *item = begin;
	    int i = 0;
	    int sp = 0;
	    while ( TRUE ) {
		int r = calcGridNum( item->width(), d->rastX );
		if ( item == begin ) {
		    i += r;
		    sp += r;
		    x = d->spacing + d->rastX * r;
		} else {
		    sp += r;
		    i += r;
		    x = i * d->rastX + sp * d->spacing;
		}
		if ( x > visibleWidth() && item != begin ) {
		    h = QMAX( h, item->height() );
		    ih = QMAX( ih, item->pixmapRect().height() );
		    item = item->prev;
		    break;
		}
		h = QMAX( h, item->height() );
		ih = QMAX( ih, item->pixmapRect().height() );
		QIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastY != -1 )
		h = QMAX( h, d->rastY );

	    // now move the items
	    item = begin;
	    i = 0;
	    sp = 0;
	    while ( TRUE ) {
		item->dirty = FALSE;
		int r = calcGridNum( item->width(), d->rastX );
		if ( item == begin ) {
		    if ( d->itemTextPos == Bottom )
			item->move( d->spacing + ( r * d->rastX - item->width() ) / 2,
				    y + ih - item->pixmapRect().height() );
		    else
			item->move( d->spacing, y + ih - item->pixmapRect().height() );
		    i += r;
		    sp += r;
		} else {
		    sp += r;
		    int x = i * d->rastX + sp * d->spacing;
		    if ( d->itemTextPos == Bottom )
			item->move( x + ( r * d->rastX - item->width() ) / 2,
				    y + ih - item->pixmapRect().height() );
		    else
			item->move( x, y + ih - item->pixmapRect().height() );
		    i += r;
		}
		if ( y + h < item->y() + item->height() )
		    h = QMAX( h, ih + item->textRect().height() );
		if ( item == end )
		    break;
		item = item->next;
	    }
	    y += h + d->spacing;
	}


    } else { // -------------------------------- SOUTH ------------------------------

	int x = y;

	{
	    int w = 0;
	    int y = 0;
	    QIconViewItem *item = begin;
	    while ( TRUE ) {
		y += d->spacing + item->height();
		if ( y > visibleHeight() && item != begin ) {
		    item = item->prev;
		    break;
		}
		w = QMAX( w, item->width() );
		QIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastX != -1 )
		w = QMAX( w, d->rastX );

	    // now move the items
	    item = begin;
	    while ( TRUE ) {
		item->dirty = FALSE;
		if ( d->itemTextPos == Bottom ) {
		    if ( item == begin )
			item->move( x + ( w - item->width() ) / 2, d->spacing );
		    else
			item->move( x + ( w - item->width() ) / 2,
				    item->prev->y() + item->prev->height() + d->spacing );
		} else {
		    if ( item == begin )
			item->move( x, d->spacing );
		    else
			item->move( x, item->prev->y() + item->prev->height() + d->spacing );
		}
		if ( item == end )
		    break;
		item = item->next;
	    }
	    x += w + d->spacing;
	}

	y = x;
    }

    return end;
}

/*!
  \internal
  Calculates how many cells and item of the width \a w needs in a grid with of
  \a x and returns the result.
*/

int QIconView::calcGridNum( int w, int x ) const
{
    float r = (float)w / (float)x;
    if ( ( w / x ) * x != w )
	r += 1.0;
    return (int)r;
}

/*!
  \internal
  Returns the first item of the row which contains \a item.
*/

QIconViewItem *QIconView::rowBegin( QIconViewItem * ) const
{
    // #### todo
    return d->firstItem;
}

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static int cmpIconViewItems( const void *n1, const void *n2 )
{
    if ( !n1 || !n2 )
	return 0;

    QIconViewPrivate::SortableItem *i1 = (QIconViewPrivate::SortableItem *)n1;
    QIconViewPrivate::SortableItem *i2 = (QIconViewPrivate::SortableItem *)n2;

    return i1->item->compare( i2->item );
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*! Sorts and rearranges all items in the icon view. If \a ascending
  is TRUE, the items are sorted in increasing order, otherwise they
  are sorted in decreasing order.  QIconViewItem::compare() is used to
  compare pairs of items.

  Note that this function sets the sort direction to \a ascending.

  \sa QIconViewItem::key(), QIconViewItem::setKey(), QIconViewItem::compare(),
  QIconView::setSorting(), QIconView::sortDirection()
*/

void QIconView::sort( bool ascending )
{
    if ( count() == 0 )
	return;

    d->sortDirection = ascending;
    QIconViewPrivate::SortableItem *items = new QIconViewPrivate::SortableItem[ count() ];

    QIconViewItem *item = d->firstItem;
    int i = 0;
    for ( ; item; item = item->next )
	items[ i++ ].item = item;

    qsort( items, count(), sizeof( QIconViewPrivate::SortableItem ), cmpIconViewItems );

    QIconViewItem *prev = 0;
    item = 0;
    if ( ascending ) {
	for ( i = 0; i < (int)count(); ++i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->prev = prev;
		if ( item->prev )
		    item->prev->next = item;
		item->next = 0;
	    }
	    if ( i == 0 )
		d->firstItem = item;
	    if ( i == (int)count() - 1 )
		d->lastItem = item;
	    prev = item;
	}
    } else {
	for ( i = (int)count() - 1; i >= 0 ; --i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->prev = prev;
		if ( item->prev )
		    item->prev->next = item;
		item->next = 0;
	    }
	    if ( i == (int)count() - 1 )
		d->firstItem = item;
	    if ( i == 0 )
		d->lastItem = item;
	    prev = item;
	}
    }

    delete [] items;

    arrangeItemsInGrid( TRUE );
}

/*!
  \reimp
*/

QSize QIconView::sizeHint() const
{
    constPolish();

    if ( !d->firstItem )
	return QSize( 50, 50 );

    if ( d->dirty && d->firstSizeHint ) {
	( (QIconView*)this )->resizeContents( QMAX( 400, contentsWidth() ),
					      QMAX( 400, contentsHeight() ) );
	( (QIconView*)this )->arrangeItemsInGrid( FALSE );
	d->firstSizeHint = FALSE;
    }

    d->dirty = TRUE;

    return QSize( QMIN( 400, contentsWidth() + style().scrollBarExtent().width()),
		  QMIN( 400, contentsHeight() + style().scrollBarExtent().height() ) );
}

/*!
  \internal
*/

void QIconView::updateContents()
{
    viewport()->update();
}

/*!
  \reimp
*/

void QIconView::enterEvent( QEvent *e )
{
    QScrollView::enterEvent( e );
    emit onViewport();
}

/*!
  \internal
  This function is always called when the geometry of an item changes. This function
  moves the item into the correct area in the internal data structure then.
*/

void QIconView::updateItemContainer( QIconViewItem *item )
{
    if ( !item || d->containerUpdateLocked || !isVisible() )
	return;

    if ( item->d->container1 && d->firstContainer ) {
	item->d->container1->items.removeRef( item );
    }
    item->d->container1 = 0;
    if ( item->d->container2 && d->firstContainer ) {
	item->d->container2->items.removeRef( item );
    }
    item->d->container2 = 0;

    QIconViewPrivate::ItemContainer *c = d->firstContainer;
    if ( !c ) {
	appendItemContainer();
	c = d->firstContainer;
    }

    bool contains = FALSE;
    while ( TRUE ) {
	if ( c->rect.intersects( item->rect() ) ) {
	    contains = c->rect.contains( item->rect() );
	    break;
	}

	c = c->n;
	if ( !c ) {
	    appendItemContainer();
	    c = d->lastContainer;
	}
    }

    if ( !c ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QIconViewItem::updateItemContainer(): No fitting container found!" );
#endif
	return;
    }

    c->items.append( item );
    item->d->container1 = c;

    if ( !contains ) {
	c = c->n;
	if ( !c ) {
	    appendItemContainer();
	    c = d->lastContainer;
	}
	c->items.append( item );
	item->d->container2 = c;
    }
}

/*!
  \internal
  Appends a new rect area to the internal data structure of the items
*/

void QIconView::appendItemContainer()
{
    QSize s;
    // #### We have to find out which value is best here
    if ( d->arrangement == LeftToRight )
	s = QSize( INT_MAX - 1, RECT_EXTENSION );
    else
	s = QSize( RECT_EXTENSION, INT_MAX - 1 );

    if ( !d->firstContainer ) {
	d->firstContainer = new QIconViewPrivate::ItemContainer( 0, 0, QRect( QPoint( 0, 0 ), s ) );
	d->lastContainer = d->firstContainer;
    } else {
	if ( d->arrangement == LeftToRight )
	    d->lastContainer = new QIconViewPrivate::ItemContainer(
		d->lastContainer, 0, QRect( d->lastContainer->rect.bottomLeft(), s ) );
	else
	    d->lastContainer = new QIconViewPrivate::ItemContainer(
		d->lastContainer, 0, QRect( d->lastContainer->rect.topRight(), s ) );
    }
}

/*!  \internal

  Rebuilds the whole internal data structure. This is done when it's
  likelt that most/all items change their geometry (e.g. in
  arrangeItemsInGrid()), because calling this is then more efficient
  than calling updateItemContainer() for each item.
*/

void QIconView::rebuildContainers()
{
    QIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    QIconViewItem *item = d->firstItem;
    appendItemContainer();
    c = d->lastContainer;
    while ( item ) {
	if ( c->rect.contains( item->rect() ) ) {
	    item->d->container1 = c;
	    item->d->container2 = 0;
	    c->items.append( item );
	    item = item->next;
	} else if ( c->rect.intersects( item->rect() ) ) {
	    item->d->container1 = c;
	    c->items.append( item );
	    c = c->n;
	    if ( !c ) {
		appendItemContainer();
		c = d->lastContainer;
	    }
	    c->items.append( item );
	    item->d->container2 = c;
	    item = item->next;
	    c = c->p;
	} else {
	    if ( d->arrangement == LeftToRight ) {
		if ( item->y() < c->rect.y() && c->p ) {
		    c = c->p;
		    continue;
		}
	    } else {
		if ( item->x() < c->rect.x() && c->p ) {
		    c = c->p;
		    continue;
		}
	    }

	    c = c->n;
	    if ( !c ) {
		appendItemContainer();
		c = d->lastContainer;
	    }
	}
    }
}

/*!
  \internal
*/

void QIconView::movedContents( int, int )
{
    if ( d->drawDragShapes ) {
	drawDragShapes( d->oldDragPos );
	d->oldDragPos = QPoint( -1, -1 );
    }
}

void QIconView::handleItemChange( QIconViewItem *old, bool shift, bool control )
{
    if ( d->selectionMode == Single ) {
	blockSignals( TRUE );
	if ( old )
	    old->setSelected( FALSE );
	blockSignals( FALSE );
	d->currentItem->setSelected( TRUE, TRUE );
    } else if ( d->selectionMode == Extended ) {
	if ( control ) {
	    // nothing
	} else if ( shift ) {
	    if ( !d->selectAnchor ) {
		if ( old && !old->selected && old->isSelectable() ) {
		    old->selected = TRUE;
		    repaintItem( old );
		}
		d->currentItem->setSelected( TRUE, TRUE );
	    } else {
		QIconViewItem *from = d->selectAnchor, *to = d->currentItem;
		if ( !from || !to )
		    return;
		QIconViewItem *i = 0;
		int index =0;
		int f_idx = -1, t_idx = -1;
		for ( i = d->firstItem; i; i = i->next, index++ ) {
		    if ( i == from )
			f_idx = index;
		    if ( i == to )
			t_idx = index;
		    if ( f_idx != -1 && t_idx != -1 )
			break;
		}
		if ( f_idx > t_idx ) {
		    i = from;
		    from = to;
		    to = i;
		}

		bool changed = FALSE;
		for ( i = d->firstItem; i && i != from; i = i->next ) {
			if ( i->selected ) {
			    i->selected = FALSE;
			    changed = TRUE;
			    repaintItem( i );
			}
		    }
		for ( i = to->next; i; i = i->next ) {
		    if ( i->selected ) {
			i->selected = FALSE;
			changed = TRUE;
			repaintItem( i );
		    }
		}

		for ( i = from; i; i = i->next ) {
		    if ( !i->selected && i->isSelectable() ) {
			i->selected = TRUE;
			changed = TRUE;
			repaintItem( i );
		    }
		    if ( i == to )
			break;
		}
		if ( changed )
		    emit selectionChanged();
	    }
	} else {
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	}
    } else {
	if ( shift )
	    d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    }
}

QBitmap QIconView::mask( QPixmap *pix ) const
{
    QBitmap m;
    if ( d->maskCache.find( QString::number( pix->serialNumber() ), m ) )
	return m;
    m = pix->createHeuristicMask();
    d->maskCache.insert( QString::number( pix->serialNumber() ), m );
    return m;
}

/*!
  \reimp

  (Implemented to get rid of a compiler warning.)
*/
void QIconView::drawContents( QPainter * )
{
}

#include "qiconview.moc"

#endif // QT_NO_ICONVIEW
