/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qiconview.cpp#19 $
**
** Definition of QIconView widget class
**
** Created : 990707
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

#include "qiconview.h"

#include <qpixmap.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qevent.h>
#include <qpalette.h>
#include <qmime.h>
#include <qimage.h>
#include <qpen.h>
#include <qbrush.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <qapplication.h>

static const char *unknown[] = {
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

/*****************************************************************************
 *
 * Class QIconViewItemDrag
 *
 *****************************************************************************/

class QIconViewItemDrag : public QDragObject
{
public:
    QIconViewItemDrag( QWidget *source = 0, const char *name = 0 )
	: QDragObject( source, name )  {}

    virtual const char *format( int ) const
    { return 0; }

    virtual QByteArray encodedData( const char * ) const
    { return QByteArray(); }

    virtual bool provides( const char *mime ) const {
	if ( QString( mime ) == "application/iconview-items" )
	    return TRUE;
	return FALSE;
    }

};

/*****************************************************************************
 *
 * Struct QIconViewPrivate
 *
 *****************************************************************************/

struct QIconViewPrivate
{
    QIconViewItem *firstItem, *lastItem;
    unsigned int count;
    QIconSet::Size mode;
    bool mousePressed, startDrag;
    QIconView::SelectionMode selectionMode;
    QIconViewItem *currentItem, *tmpCurrentItem;
    QRect *rubber;
    QTimer *scrollTimer;
    int rastX, rastY, spacing;
    bool cleared, dropped;
    int dragItems;
    int numSelectedItems;
    QPoint oldDragPos;
    QIconView::AlignMode alignMode;
    QIconView::ResizeMode resizeMode;
    int mostOuter;
};

/*****************************************************************************
 *
 * Class QIconViewItemLineEdit
 *
 *****************************************************************************/

class QIconViewItemLineEdit : public QMultiLineEdit
{
    Q_OBJECT

public:
    QIconViewItemLineEdit( const QString &text, QWidget *parent, QIconViewItem *theItem, const char *name = 0 );

signals:
    void escapePressed();

protected:
    void keyPressEvent( QKeyEvent *e );

protected:
    QIconViewItem *item;
    QString startText;

};

QIconViewItemLineEdit::QIconViewItemLineEdit( const QString &text, QWidget *parent, QIconViewItem *theItem, const char *name )
    : QMultiLineEdit( parent, name ), item( theItem ), startText( text )
{
    setText( text );
    clearTableFlags();
}

void QIconViewItemLineEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key()  == Key_Escape ) {
	item->QIconViewItem::setText( startText );
	emit escapePressed();
    }
    else if ( e->key() == Key_Enter ||
	      e->key() == Key_Return )
	emit returnPressed();
    else
	QMultiLineEdit::keyPressEvent( e ) ;
}

/*****************************************************************************
 *
 * Class QIconViewItem
 *
 *****************************************************************************/

/*!
  \class QIconViewItem qiconview.h
  \brief The QIconViewItem class implements a iconview item.

  An iconview item is a object containing an icon and a text, which can display
  itself in an iconview.

  The most simple way to create an iconview item and insert it into an iconview is,
  to construct it with a pointer to the iconview, a string and an icon:

  \code
    \/ parent is a pointer to our iconview, pixmap a QPixmap, which we want to use as icon
    (void) new QIconViewItem( parent, "This is the text of the item", QIocnSet( pixmap ) );
  \endcode

  When the iconview is deleted, all items of it are deleted automatically, so the programmer
  must not delete the iconview items itself.

  To iterate over all items of an iconview do something like

  \code
    for ( QIconViewItem *item = iconview->firstItem(); item; item = item->nextItem() )
      do_something_with( item );
  \endcode

  As the iconview is designed to use DnD, the iconview has methodes for DnD too.

  Subclassing QIconViewItem will provide a big flexibility.
*/

/*!
 Constructs an iconview item with no text and a default icon, and inserts it into
 the iconview \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent )
    : view( parent ), itemText(), itemIcon( QPixmap( unknown ) ),
      prev( 0 ), next( 0 ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
 Constructs an iconview item with no text and a default icon, and inserts it into
 the iconview \a parent after the iconview item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after )
    : view( parent ), itemText(), itemIcon( QPixmap( unknown ) ),
      prev( 0 ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
 Constructs an iconview item using \a text as text and a default icon, and inserts it into
 the iconview \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent, const QString &text )
    : view( parent ), itemText( text ), itemIcon( QPixmap( unknown ) ),
      prev( 0 ), next( 0 ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
 Constructs an iconview item using \a text as text and a default icon, and inserts it into
 the iconview \a parent after the iconview item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text )
    : view( parent ), itemText( text ), itemIcon( QPixmap( unknown ) ),
      prev( 0 ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
 Constructs an iconview item using \a text as text and a \icon as icon, and inserts it into
 the iconview \a parent.
*/

QIconViewItem::QIconViewItem( QIconView *parent, const QString &text, const QIconSet &icon )
    : view( parent ), itemText( text ), itemIcon( icon ),
      prev( 0 ), next( 0 ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
 Constructs an iconview item using \a text as text and a \icon as icon, and inserts it into
 the iconview \a parent after the iconview item \a after.
*/

QIconViewItem::QIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text, const QIconSet &icon )
    : view( parent ), itemText( text ), itemIcon( icon ),
      prev( 0 ), next( after ), allow_rename( TRUE ), allow_drag( TRUE ), allow_drop( TRUE ),
      selected( FALSE ), selectable( TRUE ), fm( 0 ), renameBox( 0 )
{
    init();
}

/*!
  Initializes the iconview item and inserts it into the iconview.
*/

void QIconViewItem::init()
{
    if ( view ) {
	fm = new QFontMetrics( view->font() );
	viewMode = view->viewMode();

	calcRect();

	view->insertItem( this );
	if ( view->isVisible() )
	    repaint();
    }
}

/*!
  Destructs the iconview item and tells the iconview about it.
*/

QIconViewItem::~QIconViewItem()
{
    view->removeItem( this );
    removeRenameBox();
}

/*!
  Sets \a text as text of the iconview item.
*/

void QIconViewItem::setText( const QString &text )
{
    if ( text == itemText )
	return;

    itemText = text;
    calcRect();
    repaint();
}

/*!
  Sets \a icon as item icon of the iconview item.
*/

void QIconViewItem::setIcon( const QIconSet &icon )
{
    itemIcon = icon;
    calcRect();
    repaint();
}

/*!
  If \allow is TRUE, the user can rename the iconview item, when clicking
  on the text after the item has been selected (in-place renaming). If \a allow
  is FALSE, no inplace-renaming is done.
*/

void QIconViewItem::setAllowRename( bool allow )
{
    allow_rename = allow;
}

/*!
  If \a allow is TRUE, the iconview lets the user to drag the iconview item (inside
  the iconview and outside of it). if \a allow is FALSE, no dragging is possible with this item.
*/

void QIconViewItem::setAllowDrag( bool allow )
{
    allow_drag = allow;
}

/*!
  If \a allow is TRUE, the iconview lets the use drop something  on this iconview item.
*/

void QIconViewItem::setAllowDrop( bool allow )
{
    allow_drop = allow;
}

/*!
 Returns the text of the iconview item.
*/

QString QIconViewItem::text()
{
    return itemText;
}

/*!
  Returns the icon of the iconview item.
*/

QIconSet QIconViewItem::icon()
{
    return itemIcon;
}

/*!
  Returns TRUE, if the item can be renamed ny the user with in-place renaming,
  else FALSE.
*/

bool QIconViewItem::allowRename()
{
    return allow_rename;
}

/*!
  Returns TRUE, if the user is allowed to drag the iconview item, else FALSE.
*/

bool QIconViewItem::allowDrag()
{
    return allow_drag;
}

/*!
  Returns TRUE, if the user is allowed to drop something onto the item, else FALSE.
*/

bool QIconViewItem::allowDrop()
{
    return allow_drop;
}

/*!
  Returns a pointer to the iconview of this item.
*/

QIconView *QIconViewItem::iconView() const
{
    return view;
}

/*!
  Returns a pointer to the previous item, or NULL if this is the first item
  of the iconview.
*/

QIconViewItem *QIconViewItem::prevItem() const
{
    return prev;
}

/*!
  Returns a pointer to the next item, or NULL if this is the last item
  of the iconview.
*/

QIconViewItem *QIconViewItem::nextItem() const
{
    return next;
}

/*!
  Returns the index of this item in the iconview, of -1 if something went wrong.
*/

int QIconViewItem::index()
{
    if ( view )
	return view->index( this );

    return -1;
}

/*!
  Selects / Unselects the item depending on the selectionMode of the iconview. If
  \a s is FALSE, the item gets unselected. If \a s is TRUE
  <li> and QIconView::selectionMode is Single, the item gets selected and the
  item which was selected, gets unselected
  <li> and QIconView::selectionMode is StrictMulti and \a cb is TRUE, the
  item gets selected
  <li> and QIconView::selectionMode is Multi the item gets selected.

  The item redraws itself if the selection changed.
*/

void QIconViewItem::setSelected( bool s, bool cb )
{
    if ( selectable && s != selected ) {
	if ( !s )
	    selected = FALSE;
	else {
	    if ( view->d->selectionMode == QIconView::Single && view->d->currentItem )
		view->d->currentItem->setSelected( FALSE );
	    else if ( view->d->selectionMode == QIconView::StrictMulti && !cb )
		view->selectAll( FALSE );
	}
	selected = s;

	repaint();
	view->emitSelectionChanged();
    }
}

/*!
  If \a s is TRUE, this item can be selected, else it can't be selected.
*/

void QIconViewItem::setSelectable( bool s )
{
    selectable = s;
    if ( selected )
	selected = FALSE;
}

/*!
  Returns TRUE, if the item is selected, else FALSE.
*/

bool QIconViewItem::isSelected()
{
    return selected;
}

/*!
  Returns TRUE, of the item is selectable, else FALSE.
*/

bool QIconViewItem::isSelectable()
{
    return selectable;
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
  Moves the item to \a x and \a y in the iconview (these are contents coordinates)
*/

void QIconViewItem::move( int x, int y )
{
    itemRect.setRect( x, y, itemRect.width(), itemRect.height() );
}

/*!
  Moves the item by the distance \a dx and \a dy.
*/

void QIconViewItem::moveBy( int dx, int dy )
{
    itemRect.moveBy( dx, dy );
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

QRect QIconViewItem::rect()
{
    return itemRect;
}

/*!
  Returns the X-Coordinate of the item (in contents coordinates).
*/

int QIconViewItem::x()
{
    return itemRect.x();
}

/*!
  Returns the Y-Coordinate of the item (in contents coordinates).
*/

int QIconViewItem::y()
{
    return itemRect.y();
}

/*!
  Returns the  width of the item.
*/

int QIconViewItem::width()
{
    return itemRect.width();
}

/*!
  Returns the height of the item.
*/

int QIconViewItem::height()
{
    return itemRect.height();
}

/*!
  Returns the size of the item.
*/

QSize QIconViewItem::size()
{
    return QSize( itemRect.width(), itemRect.height() );
}

/*!
  Returns the position of the item (in contents coordinates).
*/

QPoint QIconViewItem::pos()
{
    return QPoint( itemRect.x(), itemRect.y() );
}

/*!
  Returns the rectangle of the text of the item.
  If relative is FALSE, the returned rectangle is relative to the origin of the contents'
  coordinate system, else the rectangle is relative to the origin of the item's rectangle
 */

QRect QIconViewItem::textRect( bool relative )
{
    if ( relative )
	return itemTextRect;
    else
	return QRect( x() + itemTextRect.x(), y() + itemTextRect.y(), itemTextRect.width(), itemTextRect.height() );
}

/*!
  Returns the rectangle of the icon of the item.
  If relative is FALSE, the returned rectangle is relative to the origin of the contents'
  coordinate system, else the rectangle is relative to the origin of the item's rectangle
 */

QRect QIconViewItem::iconRect( bool relative )
{
    if ( relative )
	return itemIconRect;
    else
	return QRect( x() + itemIconRect.x(), y() + itemIconRect.y(), itemIconRect.width(), itemIconRect.height() );
}

/*!
  Returns TRUE, if the item contains the point \a pnt (in contents coordinates).
*/

bool QIconViewItem::contains( QPoint pnt )
{
    return ( textRect( FALSE ).contains( pnt ) ||
	     iconRect( FALSE ).contains( pnt ) );
}

/*!
  Returns TRUE, if the item intersects the rectangle \a r (in contents coordinates).
*/

bool QIconViewItem::intersects( QRect r )
{
    return ( textRect( FALSE ).intersects( r ) ||
	     iconRect( FALSE ).intersects( r ) );
}

/*!
  Changes the \font as the font of this item. This shouldn't be manually, as the font of the iconview
  is and should be used.
*/

void QIconViewItem::setFont( const QFont &font )
{
    f = font;

    if ( fm )
	delete fm;

    fm = new QFontMetrics( f );
    calcRect();
    repaint();
}

/*!
  Returns the font of this item.
*/

QFont QIconViewItem::font()
{
    return f;
}

/*!
  Sets the viewmode of the item. This can be QIconSet::Automatic, QIconSet::Small, QIconSet::Large.
  QIconSet::Automatic is default. Use QIconView::setViewMode() instead to set the viewmode of all items.
  
  \sa QIconView::setViewMode()
*/

void QIconViewItem::setViewMode( QIconSet::Size mode )
{
    viewMode = mode;
    calcRect();
}

/*!
  Returns TRUE, of the item accepts the QMimeSource \a mime (so it could be dropped
  on the item), else FALSE is returned.

  The default implementation does nothing and returns always TRUE. A subclass should
  reimplement this to allow dropping something on an item.
  
  \sa QFileIconViewItem::acceptDrop()
*/

bool QIconViewItem::acceptDrop( QMimeSource *mime )
{
    if ( mime )
	;
    return FALSE;
}

/*!
  Returns the QDragObject of this item, which should be use for dragging. 
  
  The default implementation returns NULL. Subclasses should implement this.

  \sa QFileIconViewItem::dragObject()
*/

QDragObject *QIconViewItem::dragObject()
{
    return 0;
}

/*!
  Starts in-place renaming.
*/

void QIconViewItem::rename()
{
    renameBox = new QIconViewItemLineEdit( itemText, view->viewport(), this );
    renameBox->resize( textRect().width() - 2, textRect().height() - 2 );
    view->addChild( renameBox, textRect( FALSE ).x() + 1, textRect( FALSE ).y() + 1 );
    renameBox->setFrameStyle( QFrame::NoFrame );
    renameBox->selectAll();
    renameBox->setFocus();
    renameBox->show();
    view->viewport()->setFocusProxy( renameBox );
    connect( renameBox, SIGNAL( returnPressed() ), this, SLOT( renameItem() ) );
    connect( renameBox, SIGNAL( escapePressed() ), this, SLOT( cancelRenameItem() ) );
}

/*!
  This function is called when the user pressed RETURN in the in-place renaming.
*/

void QIconViewItem::renameItem()
{
    if ( !renameBox )
	return;

    QRect r = itemRect;
    setText( renameBox->text() );
    view->repaintContents( r.x() - 1, r.y() - 1, r.width() + 2, r.height() + 2 );
    removeRenameBox();
}

/*!
  Cancels in-place renaming.
*/

void QIconViewItem::cancelRenameItem()
{
    if ( !renameBox )
	return;

    removeRenameBox();
}

/*!
  Removed the editbox which was used for in-place renaming.
*/

void QIconViewItem::removeRenameBox()
{
    if ( !renameBox )
	return;

    delete renameBox;
    renameBox = 0;
    view->viewport()->setFocusProxy( view );
    view->setFocus();
}

/*!
  recalculates the bounding rect, text- and iconrect of the item.
*/

void QIconViewItem::calcRect()
{
    if ( !fm )
	return;

    int w = 0, h = 0;

    if ( view->rastX() != -1 && fm->width( itemText ) > view->rastX() - 4 ) {
	QStringList lst;
	breakLines( itemText, lst, view->rastX() - 4 );

	QValueListIterator<QString> it = lst.begin();
	for ( ; it != lst.end(); ++it ) {
	    w = QMAX( w, fm->width( *it ) );
	    h += fm->height();
	}
	w += 6;
	h += 2;
    } else {
	w = fm->width( itemText ) + 6;
	h = fm->height() + 2;
    }

    itemTextRect.setWidth( w );
    itemTextRect.setHeight( h );

    w = itemIcon.pixmap( viewMode, QIconSet::Normal ).width() + 2;
    h = itemIcon.pixmap( viewMode, QIconSet::Normal ).height() + 2;

    itemIconRect.setWidth( w );
    itemIconRect.setHeight( h );

    w = QMAX( itemTextRect.width(), itemIconRect.width() );
    h = itemTextRect.height() + itemIconRect.height() + 1;

    itemRect.setWidth( w );
    itemRect.setHeight( h );

    itemTextRect = QRect( ( width() - itemTextRect.width() ) / 2, height() - itemTextRect.height(),
			  itemTextRect.width(), itemTextRect.height() );
    itemIconRect = QRect( ( width() - itemIconRect.width() ) / 2, 0,
			  itemIconRect.width(), itemIconRect.height() );
}

/*!
  Paints the item using the painter \a p.
*/

void QIconViewItem::paintItem( QPainter *p )
{
    QIconSet::Mode m = QIconSet::Normal;
    if ( isSelected() )
	m = QIconSet::Active;
    else if ( !isSelectable() )
	m = QIconSet::Disabled;

    int w = itemIcon.pixmap( viewMode, QIconSet::Normal ).width();

    if ( isSelected() )
	p->fillRect( iconRect( FALSE ), view->colorGroup().highlight() );
    p->drawPixmap( x() + ( width() - w ) / 2, y(), itemIcon.pixmap( viewMode, m ) );


    p->save();
    if ( isSelected() ) {
	p->fillRect( textRect( FALSE ), view->colorGroup().highlight() );
	p->setPen( QPen( view->colorGroup().highlightedText() ) );
    }
    else
	p->setPen( view->colorGroup().text() );

    if ( view->rastX() != -1 && fm->width( itemText ) > view->rastX() - 4 ) {
	QStringList lst;
	breakLines( itemText, lst, view->rastX() - 4 );

	QValueListIterator<QString> it = lst.begin();
	int h = 0;
	for ( ; it != lst.end(); ++it ) {
	    p->drawText( QRect( textRect( FALSE ).x(), h + textRect( FALSE ).y(),
				textRect( FALSE ).width(), fm->height() ),
			 Qt::AlignCenter, *it);
	    h += fm->height();
	}
    } else {
	p->drawText( textRect( FALSE ), Qt::AlignCenter, itemText );
    }

    p->restore();
}

/*!
  Paints the focus rect of the item using the painter \a.
*/

void QIconViewItem::paintFocus( QPainter *p )
{
    view->style().drawFocusRect( p, QRect( textRect( FALSE ).x(), textRect( FALSE ).y(),
					   textRect( FALSE ).width(), textRect( FALSE ).height() ),
				 view->colorGroup(), isSelected() ? &view->colorGroup().highlight() :
				 &view->colorGroup().base(), isSelected() );
    view->style().drawFocusRect( p, QRect( iconRect( FALSE ).x(), iconRect( FALSE ).y(), iconRect( FALSE ).width(),
					   iconRect( FALSE ).height() ),
				 view->colorGroup(), isSelected() ? &view->colorGroup().highlight() :
				 &view->colorGroup().base(), isSelected() );
}

/*!
  This methode is called, when something was dropped on the item. \a e
  gives you all information about the drop.

  The default implementation does nothing, subclasses should reimplement this
  methode.

  \sa QFileIconViewItem::dropped()
*/

void QIconViewItem::dropped( QDropEvent *e )
{
    if ( e )
	;
}

/*!
  Does linebreaking for the item's text
*/

void QIconViewItem::breakLines( const QString text, QStringList &lst, int width )
{
    lst.clear();

    if ( !fm )
	return;

    if ( text.isEmpty() ) {
	lst.append( text );
	return;
    }

    QStringList words;
    QString line = text;

    if ( line.simplifyWhiteSpace().find( QChar( ' ' ) ) == -1 ) {
	// we have only one word - do some ugly line breaking

	QString txt;
	for ( unsigned int i = 0; i < line.length(); i++ ) {
	    if ( fm->width( txt ) + fm->width( line.at( i ) ) <= width ) {
		txt += line.at( i );
	    } else {
		lst.append( txt );
		txt = line.at( i );
	    }
	}
	if ( !txt.isEmpty() )
	    lst.append( txt );

	return;
    }


    int i = text.find( QChar( ' ' ), 0 );

    while ( i != -1 ) {
	words.append( line.left( i ) );
	line.remove( 0, i + 1 );
	i = line.find( QChar( ' ' ), 0 );
    }

    if ( !line.simplifyWhiteSpace().isEmpty() )
	words.append( line.simplifyWhiteSpace() );

    QValueListIterator<QString> it = words.begin();

    int w = 0;
    line = QString::null;

    for ( ; it != words.end(); ++it ) {
	if ( w == 0 ) {
	    line = *it;
	    line += QChar( ' ' );
	    w = fm->width( line );
	} else {
	    if ( fm->width( line + *it ) <= width ) {
		line += *it;
		line += QChar( ' ' );
		w = fm->width( line );
	    } else {
		lst.append( line );
		w = 0;
		--it;
	    }
	}
    }

    if ( !line.isEmpty() )
	lst.append( line );
}

/*!
  This methode is called, when a drag enterd the item's bounding rect.

  The default implementation does nothing, subcasses should reimplement
  this methode.
*/

void QIconViewItem::dragEntered()
{
}

/*!
  This methode is called, when a drag left the item's bounding rect.

  The default implementation does nothing, subcasses should reimplement
  this methode.
*/

void QIconViewItem::dragLeft()
{
}

/*****************************************************************************
 *
 * Class QIconView
 *
 *****************************************************************************/

/*!
  \class QIconView qiconview.h
  \brief The QIconView class 
  
  The QIconView provides a widget which can contain lots of iconview items which can
  be selected, dragged and so on.
  
  Items can be inserted in a grid and can flow from top to bottom (South) or from
  left to right (East).
  
  The QIconView is designed for Drag'n'Drop, as the icons are also moved inside
  the iconview itself using DnD. So the QIconView provides some methodes for DnD
  too. 
  
  There can be specified different selection modes, which describe if more that one item
  can be selected and under which conditions.
  
  Items can of course be selected. When multiple items may be selected, the iconview
  provides a rubberband too. 
  
*/

QIconView::QIconView( QWidget *parent, const char *name )
    : QScrollView( parent, name, WNorthWestGravity )
{
    d = new QIconViewPrivate;
    d->firstItem = 0;
    d->lastItem = 0;
    d->count = 0;
    d->mode = QIconSet::Automatic;
    d->mousePressed = FALSE;
    d->selectionMode = Single;
    d->currentItem = 0;
    d->rubber = 0;
    d->scrollTimer = 0;
    d->startDrag = FALSE;
    d->tmpCurrentItem = 0;
    d->rastX = d->rastY = -1;
    d->spacing = 5;
    d->cleared = FALSE;
    d->numSelectedItems = 0;
    d->alignMode = East;
    d->resizeMode = Fixed;
    d->mostOuter = 0;
    d->dropped = FALSE;

    setAcceptDrops( TRUE );
    viewport()->setAcceptDrops( TRUE );

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    viewport()->setBackgroundMode( NoBackground );
    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( QWidget::WheelFocus );
}

/*!
  Destructs the iconview and deletes all items.
*/

QIconView::~QIconView()
{
    QIconViewItem *tmp, *item = d->firstItem;
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }
    
    delete d;
}

/*!
  Inserts the iconview item \a item after \a after. If \a after is NULL,
  \a item is appended. 
  
  You should never need to call this methode yourself, you should rather do
  
  \code
    (void) new QIconViewItem( iconview, "This is the text of the item", QIocnSet( pixmap ) );
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
	int w = 0, h = 0;

	insertInGrid( item );

	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
	resizeContents( QMAX( contentsWidth(), w ),
			QMAX( contentsHeight(), h ) );
    }

    d->count++;
}

/*!
  Removes the iconview item \a item from the iconview. You should never
  need to call this methode yourself, just delete an item to get rid of it. The 
  destructor of QIconViewItem does everything, which is required for removing
  and item.
*/

void QIconView::removeItem( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( item == d->firstItem )
	d->firstItem = d->firstItem->next;
    else if ( item == d->lastItem )
	d->lastItem = d->lastItem->prev;
    else {
	QIconViewItem *i = d->firstItem;
	while ( i != item )
	    i = i->next;

	if ( i ) {
	    i->prev->next = i->next;
	    i->next->prev = i->prev;
	}
    }

    d->count--;
}

/*!
  Returns the index of \a item or -1 if there is something wrong.
*/

int QIconView::index( QIconViewItem *item )
{
    if ( !item )
	return -1;

    if ( item == d->firstItem )
	return 0;
    else if ( item == d->lastItem )
	return d->count - 1;
    else {
	QIconViewItem *i = d->firstItem;
	unsigned int j = 0;
	while ( i != item ) {
	    i = i->next;
	    ++j;
	}

	return j;
    }
}

/*!
  Returns a pointer to the first item fo the iconview, or NULL, if there
  are no items in the iconview.
*/

QIconViewItem *QIconView::firstItem() const
{
    return d->firstItem;
}

/*!
  Returns a pointer to the last item fo the iconview, or NULL, if there
  are no items in the iconview.
*/

QIconViewItem *QIconView::lastItem() const
{
    return d->lastItem;
}

/*!
  Returns a pointer to the current item fo the iconview, or NULL, if no
  item is current.
*/

QIconViewItem *QIconView::currentItem() const
{
    return d->currentItem;
}

/*!
  Makes \a item the new current item of the iconview.
*/
  
void QIconView::setCurrentItem( QIconViewItem *item )
{
    d->currentItem = item;
    emit currentChanged();
    emit currentChanged( d->currentItem );
    emitNewSelectionNumber();
}

/*!
  Returns the number of inserted items.
*/

unsigned int QIconView::count()
{
    return d->count;
}

/*!
  Sets all icons of the iconview to the viewmode \a mode. This can be QIconSet::Automatic, QIconSet::Small, QIconSet::Large.
  QIconSet::Automatic is default.
*/

void QIconView::setViewMode( QIconSet::Size mode )
{
    d->mode = mode;

    if ( !d->firstItem )
	return;

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	item->setViewMode( mode );

    repaintContents( contentsX(), contentsY(), contentsWidth(), contentsHeight() );
}

/*!
  Returns the viewmode of the iconview.
*/

QIconSet::Size QIconView::viewMode()
{
    return d->mode;
}

/*!
  Does autoscrolling when selecting multiple icons with the rubber band.
*/

void QIconView::doAutoScroll()
{
    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( Qt::black, 1 ) );
    p.setBrush( Qt::NoBrush );
    drawRubber( &p );
    p.end();

    ensureVisible( pos.x(), pos.y() );

    pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    QRect oldRubber = QRect( *d->rubber );

    d->rubber->setRight( pos.x() );
    d->rubber->setBottom( pos.y() );

    selectByRubber( oldRubber );

    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( Qt::black, 1 ) );
    p.setBrush( Qt::NoBrush );
    drawRubber( &p );

    p.end();

    pos = QCursor::pos();
    pos = mapFromGlobal( pos );

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
    p->save();
    p->resetXForm();
    QRect r( contentsToViewport( QPoint( cx, cy ) ), QSize( cw, ch ) );
    p->setClipRect( r );
    drawBackground( p, r );
    p->restore();

    if ( !d->firstItem )
	return;

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	if ( item->rect().intersects( QRect( cx, cy, cw, ch ) ) )
	    item->paintItem( p );

    if ( ( hasFocus() || viewport()->hasFocus() ) && d->currentItem &&
	 d->currentItem->rect().intersects( QRect( cx, cy, cw, ch ) ) )
	d->currentItem->paintFocus( p );
}

/*!
  Aligns all items in the grid.
*/

void QIconView::orderItemsInGrid()
{
    int w = 0, h = 0;

    resizeContents( viewport()->width(), viewport()->height() );
    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	insertInGrid( item );
	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
    }

    resizeContents( w, h );
}

/*!
  \reimp
*/

void QIconView::show()
{
    QScrollView::show();
    resizeContents( viewport()->width(), viewport()->height() );
    orderItemsInGrid();
}

/*!
  Sets the selection mode of the iconview to \a m. This can be 
  <li>Single (only one item can be selected)
  <li>Multi (multiple items can be selected)
  <li>StrictMulti (multiple items can be selected, but only if the user pressed CTRL while selecting them)
*/

void QIconView::setSelectionMode( SelectionMode m )
{
    d->selectionMode = m;
}

/*!
  Returns the selection mode of the iconview.
*/

QIconView::SelectionMode QIconView::selectionMode()
{
    return d->selectionMode;
}

/*!
  Returns a pointer to the item which contains \a pos, which is given
  on contents coordinates.
*/

QIconViewItem *QIconView::findItem( const QPoint &pos )
{
    if ( !d->firstItem )
	return 0;

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	if ( item->contains( pos ) )
	    return item;

    return 0;
}

/*!
  If \a select is TRUE, all items are selected, else all are unselected.
*/

void QIconView::selectAll( bool select )
{
    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	item->setSelected( select );
}

/*!
  Repaints the \a item.
*/

void QIconView::repaintItem( QIconViewItem *item )
{
    if ( !item )
	return;

    if ( QRect( contentsX(), contentsY(), contentsWidth(), contentsHeight() ).
	 intersects( QRect( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 ) ) )
	repaintContents( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 );
}

/*!
  Makes sure, that \a item is visible, and scrolls the view if required.
*/

void QIconView::ensureItemVisible( QIconViewItem *item )
{
    if ( !item )
	return;

    ensureVisible( item->x(), item->y(),
		   item->width(), item->height() );
}

/*!
  Cleares the iconview.
*/

void QIconView::clear()
{
    resizeContents( visibleWidth() - verticalScrollBar()->width(),
		    visibleHeight() - horizontalScrollBar()->width() );
    setContentsPos( 0, 0 );

    if ( !d->firstItem )
	return;

    QIconViewItem *item = d->firstItem, *tmp;
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }

    d->count = 0;
    d->firstItem = 0;
    d->lastItem = 0;
    setCurrentItem( 0 );
    d->tmpCurrentItem = 0;

    viewport()->repaint( TRUE );

    d->cleared = TRUE;
}

/*!
  Sets the horizontal raster to \a rx.  If \a rx is -1, there is no 
  horizontal raster used for laigning items.
*/

void QIconView::setRastX( int rx )
{
    d->rastX = rx;
}

/*!
  Sets the vertical raster to \a ry.  If \a ry is -1, there is no 
  vertical raster used for laigning items.
*/

void QIconView::setRastY( int ry )
{
    d->rastY = ry;
}

/*!
  Returns the horizonal raster.
*/

int QIconView::rastX()
{
    return d->rastX;
}

/*!
  Returns the vertical raster.
*/

int QIconView::rastY()
{
    return d->rastY;
}

/*!
  Sets the space between iconview items to \a sp.
*/

void QIconView::setSpacing( int sp )
{
    d->spacing = sp;
}

/*! 
  Returns the spacing between iconview items.
*/

int QIconView::spacing()
{
    return d->spacing;
}

/*!
  Sets the alignment mode of the iconview to \a am. This can be 
  <li> East (Items, which don't fit onto the view, go further down (you get a vertical scrollbar)
  <li> South (Items, which don't fit onto the view, go further right (you get a horizontal scrollbar)
*/

void QIconView::setAlignMode( AlignMode am )
{
    if ( d->alignMode == am )
	return;

    d->alignMode = am;
    resizeContents( viewport()->width(), viewport()->height() );
    orderItemsInGrid();
}

/*!
  Returns the alignment mode of the iconview.
*/

QIconView::AlignMode QIconView::alignMode() const
{
    return d->alignMode;
}

/*!
  Sets the resize mode of the iconview. This can be
  <li> Fixed (when resizing the view, the items stay as they are.
  <li> Adjust (when resizing the view, the items are reordered to
  fit as good as possible into the view.
*/

void QIconView::setResizeMode( ResizeMode rm )
{
    if ( d->resizeMode == rm )
	return;

    d->resizeMode = rm;
}

/*!
  Returns the resizemode of the iconview.
*/

QIconView::ResizeMode QIconView::resizeMode() const
{
    return d->resizeMode;
}

/*!
  \reimp
*/

void QIconView::contentsMousePressEvent( QMouseEvent *e )
{
    if ( d->currentItem )
	d->currentItem->renameItem();

    if ( e->button() == LeftButton ) {
	d->startDrag = FALSE;

	QIconViewItem *item = findItem( e->pos() );
	QIconViewItem *oldCurrent = d->currentItem;

	if ( item  && item->isSelected() &&
	     item->textRect( FALSE ).contains( e->pos() ) ) {

	    if ( !item->allowRename() )
		return;

	    ensureItemVisible( item );
	    setCurrentItem( item );
	    repaintItem( item );
	    item->rename();
	    return;
	}

	if ( item && item->isSelectable() )
	    if ( d->selectionMode == Single )
		item->setSelected( TRUE, e->state() & ControlButton );
	    else
		item->setSelected( !item->isSelected(), e->state() & ControlButton );
	else
	    selectAll( FALSE );

	setCurrentItem( item );

	repaintItem( oldCurrent );
	repaintItem( d->currentItem );

	if ( !d->currentItem && d->selectionMode != Single ) {
	    if ( d->rubber )
		delete d->rubber;
	    d->rubber = 0;
	    d->rubber = new QRect( e->x(), e->y(), 0, 0 );

	    if ( d->selectionMode == StrictMulti && !( e->state() & ControlButton ) )
		selectAll( FALSE );
	}

	d->mousePressed = TRUE;
    } else if ( e->button() == RightButton ) {
	QIconViewItem *item = findItem( e->pos() );
	if ( item )
	    emit itemRightClicked( item );
	else
	    emit viewportRightClicked();
    }
}

/*!
  \reimp
*/

void QIconView::contentsMouseReleaseEvent( QMouseEvent * )
{
    d->mousePressed = FALSE;
    d->startDrag = FALSE;

    if ( d->rubber ) {
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( Qt::black, 1 ) );
	p.setBrush( Qt::NoBrush );

	drawRubber( &p );

	p.end();

	delete d->rubber;
	d->rubber = 0;
    }

    if ( d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }
}

/*!
  \reimp
*/

void QIconView::contentsMouseMoveEvent( QMouseEvent * )
{
    if ( d->mousePressed && d->currentItem && d->currentItem->allowDrag() ) {
	if ( !d->startDrag ) {
	    d->currentItem->setSelected( TRUE, TRUE );
	    d->startDrag = TRUE;
	}
	else {
	    d->mousePressed = FALSE;
	    d->cleared = FALSE;
	    startDrag();
	    if ( d->tmpCurrentItem )
		repaintItem( d->tmpCurrentItem );
	}
    } else if ( d->mousePressed && !d->currentItem && d->rubber )
	doAutoScroll();
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

void QIconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    d->tmpCurrentItem = 0;
    d->dragItems = dragItems( e );
    d->oldDragPos = contentsToViewport( e->pos() );
    drawDragShape( e->pos() );
    d->dropped = FALSE;
}

/*!
  \reimp
*/

void QIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    drawDragShape( d->oldDragPos );

    if ( d->tmpCurrentItem )
	repaintItem( d->tmpCurrentItem );

    QIconViewItem *old = d->tmpCurrentItem;
    d->tmpCurrentItem = 0;

    QIconViewItem *item = findItem( e->pos() );

    if ( item ) {
	if ( item != old ) {
	    if ( old )
		old->dragLeft();
	    item->dragEntered();
	}

	if ( item->acceptDrop( e ) )
	    e->accept();
	else
	    e->ignore();

	d->tmpCurrentItem = item;
	QPainter p;
	p.begin( viewport() );
	p.translate( -contentsX(), -contentsY() );
	item->paintFocus( &p );
	p.end();
    } else {
	e->accept();
	if ( old )
	    old->dragLeft();
    }

    d->oldDragPos = contentsToViewport( e->pos() );
    drawDragShape( e->pos() );
}

/*!
  \reimp
*/

void QIconView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    if ( !d->dropped )
	drawDragShape( d->oldDragPos );

    if ( d->tmpCurrentItem ) {
	repaintItem( d->tmpCurrentItem );
	d->tmpCurrentItem->dragLeft();
    }

    d->tmpCurrentItem = 0;
}

/*!
  \reimp
*/

void QIconView::contentsDropEvent( QDropEvent *e )
{
    d->dropped = TRUE;
    drawDragShape( d->oldDragPos );

    if ( d->tmpCurrentItem )
	repaintItem( d->tmpCurrentItem );

    QIconViewItem *i = findItem( e->pos() );

    if ( !i && e->source() == viewport() && d->currentItem && !d->cleared ) {
	QRect r = d->currentItem->rect();

	d->currentItem->move( QPoint( e->pos().x() - d->currentItem->iconRect().width() / 2,
				      e->pos().y() - d->currentItem->iconRect().height() / 2 ) );

	int w = d->currentItem->x() + d->currentItem->width() + 1;
	int h = d->currentItem->y() + d->currentItem->height() + 1;

	repaintItem( d->currentItem );
	repaintContents( r.x(), r.y(), r.width(), r.height() );

	if ( d->selectionMode != Single ) {
	    int dx = d->currentItem->x() - r.x();
	    int dy = d->currentItem->y() - r.y();

	    QIconViewItem *item = d->firstItem;
	    for ( ; item; item = item->next )
		if ( item->isSelected() && item != d->currentItem ) {
		    QRect pr = item->rect();
		    item->moveBy( dx, dy );
		    if ( d->alignMode == East )
			d->mostOuter = QMAX( d->mostOuter, dx + item->width() );
		    else
			d->mostOuter = QMAX( d->mostOuter, dy + item->height() );
		    repaintItem( item );
		    repaintContents( pr.x(), pr.y(), pr.width(), pr.height() );
		    w = QMAX( w, item->x() + item->width() + 1 );
		    h = QMAX( h, item->y() + item->height() + 1 );
		}
	}
	bool fullRepaint = FALSE;
	if ( w > contentsWidth() ||
	     h > contentsHeight() )
	    fullRepaint = TRUE;

	int oldw = contentsWidth();
	int oldh = contentsHeight();

	resizeContents( QMAX( contentsWidth(), w ), QMAX( contentsHeight(), h ) );

	if ( fullRepaint ) {
	    repaintContents( oldw, 0, contentsWidth() - oldw, contentsHeight() );
	    repaintContents( 0, oldh, contentsWidth(), contentsHeight() - oldh );
	}
	e->accept();
    } else if ( !i && e->source() != viewport() || d->cleared )
	emit dropped( e );
    else if ( i )
	i->dropped( e );
}

/*!
  \reimp
*/

void QIconView::resizeEvent( QResizeEvent* e )
{
    QScrollView::resizeEvent( e );
    if ( d->resizeMode == Adjust ) {
	if ( d->alignMode == East && d->mostOuter > e->size().width() ||
	     d->alignMode == South && d->mostOuter > e->size().height() ) {
	    d->mostOuter = 0;
	    orderItemsInGrid();
	    viewport()->repaint( FALSE );
	} else if ( e->size().width() > e->oldSize().width() && d->mostOuter + 50 < e->size().width() ) {
	    d->mostOuter = 0;
	    orderItemsInGrid();
	    viewport()->repaint( FALSE );
	}
    }
}

/*!
  \reimp
*/

void QIconView::keyPressEvent( QKeyEvent *e )
{
    if ( !d->firstItem )
	return;

    if ( !d->currentItem ) {
	if ( e->key() == Key_Control )
	    return;

	setCurrentItem( d->firstItem );

	if ( d->selectionMode == Single )
	    d->currentItem->setSelected( TRUE, TRUE );

	repaintItem( d->currentItem );
	return;
    }

    switch ( e->key() )
    {
    case Key_Home:
    {
	if ( !d->firstItem )
	    return;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->firstItem );

	if ( d->selectionMode == Single ) {
	    item->setSelected( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	} else {
	    if ( e->state() & ShiftButton )
		d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
	}

	repaintItem( item );
	repaintItem( d->currentItem );
    } break;
    case Key_End:
    {
	if ( !d->lastItem )
	    return;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->lastItem );

	if ( d->selectionMode == Single ) {
	    item->setSelected( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	} else {
	    if ( e->state() & ShiftButton )
		d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
	}

	repaintItem( item );
	repaintItem( d->currentItem );
    } break;
    case Key_Right:
    {
	if ( !d->currentItem->next )
	    return;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->currentItem->next );

	if ( d->selectionMode == Single ) {
	    item->setSelected( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	} else {
	    if ( e->state() & ShiftButton )
		d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
	}

	repaintItem( item );
	repaintItem( d->currentItem );
    } break;
    case Key_Left:
    {
	if ( !d->currentItem->prev )
	    return;

	QIconViewItem *item = d->currentItem;
	setCurrentItem( d->currentItem->prev );

	if ( d->selectionMode == Single ) {
	    item->setSelected( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	} else {
	    if ( e->state() & ShiftButton )
		d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
	}

	repaintItem( item );
	repaintItem( d->currentItem );
    } break;
    case Key_Space:
    {
	if ( d->selectionMode == Single )
	    return;

	d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    } break;
    case Key_Enter: case Key_Return:
	emit doubleClicked( d->currentItem );
	break;
    case Key_Down:
    {
	QIconViewItem *item = d->firstItem;
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
		d->currentItem = item;
		if ( d->selectionMode == Single ) {
		    i->setSelected( FALSE );
		    d->currentItem->setSelected( TRUE, TRUE );
		} else {
		    if ( e->state() & ShiftButton )
			d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
		}
		repaintItem( i );
		repaintItem( d->currentItem );
		break;
	    }
	}
	if ( !item ) {
	    item = d->firstItem;
	    QRect r( d->currentItem->x(), 0, d->currentItem->width(), contentsHeight() );
	    for ( ; item; item = item->next ) {
		if ( r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->next && r.intersects( item->next->rect() ) ) {
			QRect irn = r.intersect( item->next->rect() );
			if ( irn.width() > ir.width() )
			    item = item->next;
		    }
		    item = item->next;
		    QIconViewItem *i = d->currentItem;
		    d->currentItem = item;
		    if ( d->selectionMode == Single ) {
			i->setSelected( FALSE );
			d->currentItem->setSelected( TRUE, TRUE );
		    } else {
			if ( e->state() & ShiftButton )
			    d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
		    }
		    repaintItem( i );
		    repaintItem( d->currentItem );
		    break;
		}
	    }
	}

    } break;
    case Key_Up:
    {
	QIconViewItem *item = d->lastItem;
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
		d->currentItem = item;
		if ( d->selectionMode == Single ) {
		    i->setSelected( FALSE );
		    d->currentItem->setSelected( TRUE, TRUE );
		} else {
		    if ( e->state() & ShiftButton )
			d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
		}
		repaintItem( i );
		repaintItem( d->currentItem );
		break;
	    }
	}
    } break;
    }

    ensureItemVisible( d->currentItem );
}

/*!
  \reimp
*/

void QIconView::focusInEvent( QFocusEvent * )
{
    if ( d->currentItem )
	repaintItem( d->currentItem );
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
  Selects items by the rubber band.
*/

void QIconView::selectByRubber( QRect oldRubber )
{
    if ( !d->rubber )
	return;

    oldRubber = oldRubber.normalize();
    int minx = contentsWidth(), miny = contentsHeight();
    int maxx = 0, maxy = 0;
    int selected = 0;

    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	if ( item->intersects( oldRubber ) &&
	     !item->intersects( d->rubber->normalize() ) )
	    item->setSelected( FALSE );
	else if ( item->intersects( d->rubber->normalize() ) ) {
	    item->setSelected( TRUE, TRUE );
	    ++selected;
	    minx = QMIN( minx, item->x() - 1 );
	    miny = QMIN( miny, item->y() - 1 );
	    maxx = QMAX( maxx, item->x() + item->width() + 1 );
	    maxy = QMAX( maxy, item->y() + item->height() + 1 );
	}
    }
    emit selectionChanged();
    emit selectionChanged( selected );
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
    p->drawRect( QRect( pnt.x(), pnt.y(), d->rubber->width(), d->rubber->height() ) );
}

/*!
  Returns the QDragObject which should be used for DnD. In fact, this calls
  QIconViewItem::dragObject() of the current item, or returns NULL, of the 
  is no current item.
  
  Subclasses should reimplement this.

  \sa QIconViewItem::dragObject()
*/

QDragObject *QIconView::dragObject()
{
    if ( !d->currentItem )
	return 0;

    QIconViewItemDrag *drag = new QIconViewItemDrag( viewport() );
    drag->setPixmap( QPixmap( d->currentItem->icon().pixmap( d->mode, QIconSet::Normal ) ),
		     QPoint( d->currentItem->iconRect().width() / 2, d->currentItem->iconRect().height() / 2 ) );

    return drag;
}

/*!
  Starts a drag.
*/

void QIconView::startDrag()
{
    QDragObject *drag = dragObject();
    if ( !drag )
	return;

    if ( drag->drag() )
	if ( drag->target() != viewport() )
	    emit moved();
}

/*!
  Inserts an item in the grid of the iconview. You should never need
  to call this manually.
*/

void QIconView::insertInGrid( QIconViewItem *item )
{
    if ( !item )
	return;

    int xpos = 0;
    int ypos = 0;

    if ( d->alignMode == East ) {
	int px = d->spacing;
	int py = d->spacing;
	int pw = d->rastX == -1 ? 1 : d->rastX / 2;
	int ph = d->rastY == -1 ? 1 : d->rastY / 2;
	bool isFirst = item == d->firstItem;

	if ( item->prev ) {
	    px = item->prev->x();
	    py = item->prev->y();
	    pw = item->prev->width();
	    ph = item->prev->height();
	    if ( d->rastX != - 1 && pw > d->rastX - 1 ) {
		px = px + pw - d->rastX;
		pw = d->rastX - 1;
	    }
	    if ( d->rastY != - 1 && ph > d->rastY - 1 ) {
		py = py + ph - d->rastY;
		ph = d->rastY - 1;
	    }
	}

	bool nextRow = FALSE;

	if ( d->rastX == -1 ) {
	    xpos = px + pw + d->spacing;
	    if ( xpos + item->width() >= viewport()->width() ) {
		xpos = d->spacing;
		nextRow = TRUE;
	    }
	} else {
	    int fact = px / d->rastX;

	    if ( !isFirst )
		xpos = ( fact + 1 ) * d->rastX;
	    else
		xpos = fact * d->rastX;

	    if ( xpos + d->rastX >= viewport()->width() ) {
		xpos = d->spacing;
		nextRow = TRUE;
	    }
	    xpos += ( d->rastX - item->width() ) / 2 + d->spacing;
	}

	if ( d->rastY == -1 ) {
	    if ( !nextRow )
		ypos = py;
	    else
		ypos = py + ph + d->spacing;
	} else {
	    int fact = py / d->rastY;

	    if ( !nextRow )
		ypos = fact * d->rastY;
	    else
		ypos = ( fact + 1 ) * d->rastY;
	    ypos += ( d->rastY - item->height() ) / 2;
	}
	d->mostOuter = QMAX( d->mostOuter, xpos + item->width() );
    } else {
	int px = d->spacing;
	int py = d->spacing;
	int pw = d->rastX == -1 ? 1 : d->rastX / 2;
	int ph = d->rastY == -1 ? 1 : d->rastY / 2;
	bool isFirst = item == d->firstItem;

	if ( item->prev ) {
	    px = item->prev->x();
	    py = item->prev->y();
	    pw = item->prev->width();
	    ph = item->prev->height();
	    if ( d->rastX != - 1 && pw > d->rastX - 1 ) {
		px = px + pw - d->rastX;
		pw = d->rastX - 1;
	    }
	    if ( d->rastY != - 1 && ph > d->rastY - 1 ) {
		py = py + ph - d->rastY;
		ph = d->rastY - 1;
	    }
	}

	bool nextCol = FALSE;

	if ( d->rastY == -1 ) {
	    ypos = py + ph + d->spacing;
	    if ( ypos + item->height() >= viewport()->height() ) {
		ypos = d->spacing;
		nextCol = TRUE;
	    }
	} else {
	    int fact = py / d->rastY;

	    if ( !isFirst )
		ypos = ( fact + 1 ) * d->rastY;
	    else
		ypos = fact * d->rastY;

	    if ( ypos + d->rastY >= viewport()->height() ) {
		ypos = d->spacing;
		nextCol = TRUE;
	    }
	    ypos += ( d->rastY - item->height() ) / 2 + d->spacing;
	}

	if ( d->rastX == -1 ) {
	    if ( !nextCol )
		xpos = px;
	    else
		xpos = px + pw + d->spacing;
	} else {
	    int fact = px / d->rastX;

	    if ( !nextCol )
		xpos = fact * d->rastX;
	    else
		xpos = ( fact + 1 ) * d->rastX;
	    xpos += ( d->rastX - item->width() ) / 2;
	}
	d->mostOuter = QMAX( d->mostOuter, ypos + item->height() );
    }

    item->move( xpos, ypos );
}

/*!
  Emits signals, that indciate selection changes.
*/

void QIconView::emitSelectionChanged()
{
    emit selectionChanged();
    emitNewSelectionNumber();
}

/*!
  Emits signals, that indciate the number of selected items
*/

void QIconView::emitNewSelectionNumber()
{
    int num = 0;
    QIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	if ( item->isSelected() )
	    ++num;

    emit selectionChanged( num );
    d->numSelectedItems = num;
}

/*!
  If a drag enters the iconview, shades of the objects, which the drag
  contains are drawn, usnig \a pos as origin.
*/

void QIconView::drawDragShape( const QPoint &pos )
{
    if ( d->dragItems > 0 ) {
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	QRect r = QIconSet( QPixmap( unknown ), viewMode() ).pixmap().rect();
	r.setWidth( r.width() + 10 );
	int num = viewport()->width() / ( r.width() + 10 );

	QPoint coord( 0, 0 );

	for ( int i = 0; i < d->dragItems; ++i ) {
	    style().drawFocusRect( &p, QRect( pos.x() + coord.x() * ( r.width() + 10 ) + 5 ,
					      pos.y() + coord.y() * ( r.height() + 30 ),
					      r.width(), r.height() ), colorGroup() );
	    style().drawFocusRect( &p, QRect( pos.x() + coord.x() * ( r.width() + 10 ),
					      pos.y() + coord.y() * ( r.height() + 30 ) + r.height() + 5,
					      r.width() + 10, 20 ), colorGroup() );
	    if ( coord.x() == num ) {
		coord.setX( 0 );
		coord.setY( coord.y() + 1 );
	    } else {
		coord.setX( coord.x() + 1 );
	    }

	}

	p.end();
    }
}

/*!
  Returns the number of the items, which the drag of \a e contains. This should
  be reimplemented in subclasses, so that it works for all accepted drags. 

  \sa QFileIconView::dragItems()
*/

int QIconView::dragItems( QDropEvent *e )
{
    if ( e->source() == viewport() )
	return d->numSelectedItems;
    return -1;
}

/*!
  This methode is called to draw the rectangle \a r of the background using
  the painter \a p. xOffset and yOffset are known using the methodes
  contentsX() and contentsY().
  
  The default implementation only fills \a r with colorGroup().base(). Subclasses
  may reimplement this to draw fency backgrounds.
  
  \sa QFileIconView::drawBackground()
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

    return QScrollView::eventFilter( o, e );
}

#include "qiconview.moc"
