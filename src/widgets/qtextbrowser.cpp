/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextbrowser.cpp#9 $
**
** Implementation of the QTextView class
**
** Created : 990101
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

#include "qtextbrowser.h"
#include "../kernel/qrichtextintern.cpp"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

#include "qstack.h"
#include "stdio.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qtimer.h"
#include "qimage.h"
#include "qsimplerichtext.h"
#include "qdragobject.h"


/*!
  \class QTextBrowser qtextbrowser.h
  \brief A rich text  browser with simple navigation.
  \ingroup realwidgets

  This class is the same as the QTextView it inherits, with the
  addition that it provides basic navigation features to follow links
  in hypertext documents that link to other rich text documents. While
  QTextView only allows to set its contents with setText(),
  QTextBrowser has an additional function setSource(), that makes it
  possible to set documents by name. These names are looked up in the
  text view's mime source factory. If a document name ends with an
  anchor, for example "\c #anchor", the text browser will
  automatically scroll accordingly ( using scrollToAnchor() ). When
  the user clicks on a hyperlink, the browser will call setSource()
  itself, with the link's \c href value as argument.

  QTextBrowser doesn't provide actual Back and Forward buttons, but it
  has backward() and forward() slots that implement the
  functionality. The home() slots brings it back to its very first
  document displayed.

  By using QTextView::setMimeSourceFactory(), you can provide your own
  subclass of QMimeSourceFactory. This makes it possible to access
  data from anywhere you need to, may it be the network or a
  database. See QMimeSourceFactory::data() for details.
  
  If you intend to use the mime factory to read the data directly from
  the file system, you may have to specify the encoding for the file
  extension you are using. For example
  \code
  mimeSourceFactory()->setExtensionType("qml", "text/utf8");
  \endcode
  Otherwise, the factory will not be able to resolve the document names.

  For simpler richt text use, see QLabel, QTextView or QSimpleRichText.
*/

class QTextBrowserData
{
public:
    QString searchPath;
    const QTextContainer* buttonDown;
    const QTextContainer* highlight;
    QPoint lastClick;
    QStack<QString> stack;
    QStack<QString> forwardStack;
    QString home;
    QString curmain;
};


/*!
  Constructs an empty QTextBrowser.
*/
QTextBrowser::QTextBrowser(QWidget *parent, const char *name)
    : QTextView( parent, name )
{
    d = new QTextBrowserData;

    viewport()->setMouseTracking( TRUE );
    d->buttonDown = 0;
    d->highlight = 0;
}

/*!
  Destructs the browser.
*/
QTextBrowser::~QTextBrowser()
{
    delete d;
}


/*!
  Sets the text document with the given \a name to be displayed.  The
  name is looked up in the mimeSourceFactory() of the browser.

  In addition to the factory lookup, this functions also checks for
  optional anchors and scrolls the document accordingly.

  If the first tag in the document is \c &lt;qt \c type=detail&gt;, it is
  displayed as a popup rather than as new document in the browser
  window itself. Otherwise, the document is set normally via
  setText(), with \a name as new context.

  If you are using the filesystem access capabilities of the mime
  source factory, you have to ensure that the factory knows about the
  encoding of specified text files, otherwise no data will be
  available. The default factory handles a couple of common file
  extensions such as \c *.html and \c *.txt with reasonable defaults. See
  QMimeSourceFactory::data() for details.

*/
void QTextBrowser::setSource(const QString& name)
{
    QString source = name;
    QString mark;
    int hash = name.find('#');
    if ( hash != -1) {
	source  = name.left( hash );
	mark = name.mid( hash+1 );
    }

    if ( source.left(5) == "file:" )
	source = source.mid(6);

    QString url = mimeSourceFactory()->makeAbsolute( source, context() );

    if ( !source.isEmpty() && url != d->curmain ) {
	const QMimeSource* m = 0;
	m = mimeSourceFactory()->data( source, context() );
	QString txt;
	if ( !m ){
	    qWarning("QTextBrowser: no mimesource for %s", source.latin1() );
	}
	else {
	    if ( !QTextDrag::decode( m, txt ) ) {
		qWarning("QTextBrowser: cannot decode %s", source.latin1() );
	    }
	}
	d->curmain = url;
	setText( txt, url );
    }

    if ( !mark.isEmpty() ) {
	url += "#";
	url += mark;
    }

    if ( !d->home )
	d->home = url;

    if ( d->stack.isEmpty() || *d->stack.top() != url) {
	emit backwardAvailable( !d->stack.isEmpty() );
	d->stack.push(new QString( url ) );
    }

    if ( !mark.isEmpty() )
	scrollToAnchor( mark );
    else
	setContentsPos( contentsX(), 0 );
}

/*!
  Sets the contents of the browser to \a text, and emits the
  textChanged() signal.
*/
void QTextBrowser::setText( const QString& text, const QString& context )
{
    QTextView::setText( text, context );
    emit textChanged();
}

/*!
  \fn void QTextBrowser::backwardAvailable(bool available)
  This signal is emitted when the availability of the backward()
  changes.  It becomes available when the user navigates forward,
  and unavailable when the user is at the home().
*/

/*!
  \fn void QTextBrowser::forwardAvailable(bool available)
  This signal is emitted when the availability of the forward()
  changes.  It becomes available after backward() is activated,
  and unavailable when the user navigates or goes forward() to
  the last navigated document.
*/

/*!
  \fn void QTextBrowser::highlighted (const QString &href)
  This signal is emitted when the user has selected but not activated
  a link in the document.  \a href is the value of the href tag
  in the link.
*/

/*!
  \fn void QTextBrowser::textChanged()
  This signal is emitted whenever the setText() changes the
  contents (eg. because the user clicked on a link).
*/

/*!
  Changes the document displayed to be the previous document
  in the list of documents build by navigating links.

  \sa forward(), backwardAvailable()
*/
void QTextBrowser::backward()
{
    if ( d->stack.count() <= 1)
	return;
    d->forwardStack.push( d->stack.pop() );
    QString* ps = d->stack.pop();
    setSource( *ps );
    delete ps;
    emit forwardAvailable( TRUE );
}

/*!
  Changes the document displayed to be the next document
  in the list of documents build by navigating links.

  \sa backward(), forwardAvailable()
*/
void QTextBrowser::forward()
{
    if ( d->forwardStack.isEmpty() )
	return;
    QString* ps = d->forwardStack.pop();
    setSource( *ps );
    delete ps;
    emit forwardAvailable( !d->forwardStack.isEmpty() );
}

/*!
  Changes the document displayed to be the first document the
  browser displayed.
*/
void QTextBrowser::home()
{
    if (!d->home.isNull() )
	setSource( d->home );
}

/*!
  Add Backward and Forward on ALT-Left and ALT-Right respectively.
*/
void QTextBrowser::keyPressEvent( QKeyEvent * e )
{
    if ( e->state() & AltButton ) {
	switch (e->key()) {
	case Key_Right:
	    forward();
	    return;
	case Key_Left:
	    backward();
	    return;
	case Key_Up:
	    home();
	    return;
	}
    }
    QTextView::keyPressEvent(e);
}

/*!
  \e override to press anchors.
*/
void QTextBrowser::viewportMousePressEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	d->buttonDown = anchor( e->pos() );
	d->lastClick = e->globalPos();
    }
}

/*!
  \e override to activate anchors.
*/
void QTextBrowser::viewportMouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton ) {
	if (d->buttonDown && d->buttonDown == anchor( e->pos() )){
	  if ( d->buttonDown->attributes() && d->buttonDown->attributes()->contains("href")) {
	      QString href;
	      href = d->buttonDown->attributes()->operator[]("href");
	      setSource( href );
	  }
	}
    }
    d->buttonDown = 0;
}

/*!
  Activate to emit highlighted().
*/
void QTextBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
    const QTextContainer* act = anchor( e->pos() );
    if (d->highlight != act) {
	if (act && act->attributes() && act->attributes()->contains("href")) {
	    QString href;
	    href = act->attributes()->operator[]("href");
	    emit highlighted( href );
	    d->highlight = act;
	}
	else if ( d->highlight ) {
	    emit highlighted( QString::null );
	    d->highlight = 0;
	}
	viewport()->setCursor( d->highlight?pointingHandCursor:arrowCursor );
    }
}


const QTextContainer* QTextBrowser::anchor( const QPoint& pos)
{
    QPainter p( viewport() );
    QTextNode* n = richText().hitTest(&p, 0, 0,
					   contentsX() + pos.x(),
					   contentsY() + pos.y());
    if (n)
	return n->parent()->anchor();
    return 0;
}


class QTextDetailPopup : public QWidget
{
public:
    QTextDetailPopup()
	: QWidget ( 0, "automatic QText detail widget", WType_Popup | WDestructiveClose )
	{
	}

protected:

    void mousePressEvent( QMouseEvent*)
	{
	close();
	}
};


void QTextBrowser::popupDetail( const QString& contents, const QPoint& pos )
{

    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int normalMargin = 12; // *2
    const int leftMargin = 18;   // *3

    QWidget* popup = new QTextDetailPopup;
    popup->setBackgroundMode( QWidget::NoBackground );

    QWidget * desktop = QApplication::desktop();

    int w = desktop->width() / 3;
    if ( w < 200 )
	w = 200;
    else if ( w > 300 )
	w = 300;


    QPainter p( popup );

    QSimpleRichText* qmlDoc = new QSimpleRichText( contents, popup->font() );
    qmlDoc->setWidth( &p, w );
    QRect r( 0, 0, qmlDoc->width(), qmlDoc->height() );

    int h = r.height() + normalMargin + normalMargin;
    w = w + leftMargin + normalMargin;

    popup->resize( w + shadowWidth, h + shadowWidth );

    // okay, now to find a suitable location
    //###### we need a global fancy popup positioning somewhere
    popup->move(pos - popup->rect().center());
    if (popup->geometry().right() > QApplication::desktop()->width())
	popup->move( QApplication::desktop()->width() - popup->width(),
		     popup->y() );
    if (popup->geometry().bottom() > QApplication::desktop()->height())
	popup->move( popup->x(),
		     QApplication::desktop()->height() - popup->height() );
    if ( popup->x() < 0 )
	popup->move( 0, popup->y() );
    if ( popup->y() < 0 )
	popup->move( popup->x(), 0 );


    popup->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.

    p.setPen( QApplication::palette().normal().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( QApplication::palette().normal().mid() );
    p.setBrush( QColor( 255, 255, 240 ) );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( black );

    qmlDoc->draw( &p, leftMargin, normalMargin, r, popup->colorGroup(), 0 );
    delete qmlDoc;

    p.drawPoint( w + 5, 6 );
    p.drawLine( w + 3, 6,
		w + 5, 8 );
    p.drawLine( w + 1, 6,
		w + 5, 10 );
    int i;
    for( i=7; i < h; i += 2 )
	p.drawLine( w, i,
		    w + 5, i + 5 );
    for( i = w - i + h; i > 6; i -= 2 )
	p.drawLine( i, h,
		    i + 5, h + 5 );
    for( ; i > 0 ; i -= 2 )
	p.drawLine( 6, h + 6 - i,
		    i + 5, h + 5 );
    p.end();

}



/*!
  Scrolls the browser so that the part of the document named
  \a name is at the top of the view (or as close to the top
  as the size of the document allows).
*/
void QTextBrowser::scrollToAnchor(const QString& name)
{
    int x1,y1,h,ry,rh;

    QTextContainer* anchor = richText().findAnchor( name );
    if ( !anchor )
	return;

    QTextContainer* parent = anchor->parent;
    QTextNode* node = richText().nextLayout( anchor, parent);
    if (!node)
	return;
    y1 = contentsY();
    {
	QPainter p(viewport());
	(void) node->parent()->box()->locate( &p, node, x1, y1, h, ry, rh );
    }

    setContentsPos( contentsX(), y1 );
}

