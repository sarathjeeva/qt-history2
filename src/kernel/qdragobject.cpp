/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdragobject.cpp#71 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtextcodec.h"
#include "qdragobject.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qimage.h"
#include "qbuffer.h"
#include <ctype.h>


// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

struct QDragData {
    QDragData(): autoDelete( TRUE ) {}
    bool autoDelete;
    QPixmap pixmap;
    QPoint hot;
    QWidget* target;
};

/*!
  After the drag completes, this function will return the QWidget
  which received the drop, or 0 if the data was dropped on some other
  program.

  This can be useful for detecting the case where drag-and-drop is to
  and from the same widget.
*/
QWidget * QDragObject::target()
{
    return d->target;
}

/*!
  \internal
  Sets the target.
*/
void QDragObject::setTarget(QWidget* t)
{
    d->target = t;
}

struct QStoredDragData {
    QStoredDragData() {}
    char* fmt;
    QByteArray enc;
};


// These pixmaps approximate the images in the Windows User Interface Guidelines.

/* XPM */
static const char * move_xpm[] = {
"11 20 3 1",
".	c None",
#if defined(_WS_WIN_)
" 	c #000000", // Windows cursor is traditionally white
"X	c #FFFFFF",
#else
" 	c #FFFFFF", // X11 cursor is traditionally white
"X	c #000000",
#endif
"  .........",
" X ........",
" XX .......",
" XXX ......",
" XXXX .....",
" XXXXX ....",
" XXXXXX ...",
" XXXXXXX ..",
" XXXXXXXX .",
" XXXXXXXXX ",
" XXXXXX    ",
" XXX XX ...",
" XX  XX ...",
" X .. XX ..",
"  ... XX ..",
" ..... XX .",
"...... XX .",
"....... XX ",
"....... XX ",
"........  ."};

/* XPM */
static const char * copy_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
#if defined(_WS_WIN_) // Windows cursor is traditionally white
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX    .............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
#else
"XX......................",
"X X.....................",
"X  X....................",
"X   X...................",
"X    X..................",
"X     X.................",
"X      X................",
"X       X...............",
"X        X..............",
"X         X.............",
"X      XXXX.............",
"X   X  X................",
"X  XX  X................",
"X X..X  X...............",
"XX...X  X...............",
"X.....X  X..............",
"......X  X..............",
".......X  X.............",
".......X  X.............",
"........XX...           ",
#endif
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XX     XX ",
"............. XXXX XXXX ",
"............. XXXX XXXX ",
"............. XXXXXXXXX ",
"............. XXXXXXXXX ",
".............           "};

/* XPM */
static const char * link_xpm[] = {
"24 30 3 1",
".	c None",
" 	c #000000",
"X	c #FFFFFF",
#if defined(_WS_WIN_) // Windows cursor is traditionally white
"  ......................",
" X .....................",
" XX ....................",
" XXX ...................",
" XXXX ..................",
" XXXXX .................",
" XXXXXX ................",
" XXXXXXX ...............",
" XXXXXXXX ..............",
" XXXXXXXXX .............",
" XXXXXX    .............",
" XXX XX ................",
" XX  XX ................",
" X .. XX ...............",
"  ... XX ...............",
" ..... XX ..............",
"...... XX ..............",
"....... XX .............",
"....... XX .............",
"........  ...           ",
#else
"XX......................",
"X X.....................",
"X  X....................",
"X   X...................",
"X    X..................",
"X     X.................",
"X      X................",
"X       X...............",
"X        X..............",
"X         X.............",
"X      XXXX.............",
"X   X  X................",
"X  XX  X................",
"X X..X  X...............",
"XX...X  X...............",
"X.....X  X..............",
"......X  X..............",
".......X  X.............",
".......X  X.............",
"........XX...           ",
#endif
"............. XXXXXXXXX ",
"............. XXX    XX ",
"............. XXXX   XX ",
"............. XXX    XX ",
"............. XX   X XX ",
"............. XX  XXXXX ",
"............. XX XXXXXX ",
"............. XXX XXXXX ",
"............. XXXXXXXXX ",
".............           "};


// the universe's only drag manager
static QDragManager * manager = 0;


QDragManager::QDragManager()
    : QObject( qApp, "global drag manager" )
{
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap(move_xpm);
    pm_cursor[1] = QPixmap(copy_xpm);
    pm_cursor[2] = QPixmap(link_xpm);
    object = 0;
    dragSource = 0;
    dropWidget = 0;
    if ( !manager )
	manager = this;
    beingCancelled = FALSE;
    restoreCursor = FALSE;
    willDrop = FALSE;
}


QDragManager::~QDragManager()
{
    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
    manager = 0;
    delete [] pm_cursor;
}




/*!  Creates a drag object which is a child of \a dragSource and
  named \a name.

  Note that the drag object will be deleted when \a dragSource is.
*/

QDragObject::QDragObject( QWidget * dragSource, const char * name )
    : QObject( dragSource, name )
{
    d = new QDragData();
    if ( !manager && qApp )
	(void)new QDragManager();
}


/*!  Deletes the drag object and frees up the storage used. */

QDragObject::~QDragObject()
{
    d->autoDelete = FALSE; // so cancel() won't delete this object
    if ( manager && manager->object == this )
	manager->cancel();
    delete d;
}

/*!
  Set the pixmap \a pm to display while dragging the object.
  The platform-specific
  implementation will use this in a loose fashion - so provide a small masked
  pixmap, but do not require that the user ever sees it in all its splendor.
  In particular, cursors on Windows 95 are of limited size.

  The \a hotspot is the point on (or off) the pixmap that should be under the
  cursor as it is dragged. It is relative to the top-left pixel of the pixmap.
*/
void QDragObject::setPixmap(QPixmap pm, QPoint hotspot)
{
    d->pixmap = pm;
    d->hot = hotspot;
    if ( manager && manager->object == this )
	manager->updatePixmap();
}

/*!
  \overload
  Uses a hotspot that positions the pixmap below and to the
  right of the mouse pointer.  This allows the user to clearly
  see the point on the window which they are dragging the data onto.
*/
void QDragObject::setPixmap(QPixmap pm)
{
    setPixmap(pm,QPoint(-10,-10));
}

/*!
  Returns the currently set pixmap
  (which \link QPixmap::isNull() isNull()\endlink if none is set).
*/
QPixmap QDragObject::pixmap() const
{
    return d->pixmap;
}

/*!
  Returns the currently set pixmap hitspot.
*/
QPoint QDragObject::pixmapHotSpot() const
{
    return d->hot;
}

/*!
  Starts a drag operation using the contents of this object,
  using DragDefault mode.

  The function returns TRUE if the caller should delete the
  original copy of the dragged data (but also note target()).
*/
bool QDragObject::drag()
{
    return drag( DragDefault );
}


/*!
  Starts a drag operation using the contents of this object,
  using DragMove mode.
*/
bool QDragObject::dragMove()
{
    return drag( DragMove );
}


/*!
  Starts a drag operation using the contents of this object,
  using DragCopy mode.

  See drag(DragMove) for important further information.
*/
void QDragObject::dragCopy()
{
    (void)drag( DragCopy );
}


/*!
  Starts a drag operation using the contents of this object.

  At this point, the object becomes owned by Qt, not the
  application.  You should not delete the drag object nor
  anything it references.  The actual transfer of data to
  the target application will be done during future event
  processing - after that time the drag object will be deleted.

  Returns TRUE if the dragged data was dragged as a \e move,
  indicating that the caller should remove the original source
  of the data (the drag object must continue to have a copy).

  \define DragMode

  The \a mode is one of:

  <ul>
   <li>\c DragDefault - the mode is determined heuristically.
   <li>\c DragCopy - the data is copied, never moved.
   <li>\c DragMove - the data is moved, if dragged at all.
   <li>\c DragCopyOrMove - the user chooses the mode
	    by using control key to switch from the default.
  </ul>

  Normally one of simpler drag(), dragMove(), or dragCopy() functions
  would be used instead.

  \warning in Qt 1.x, drag operations all return FALSE.  This will change
	    in later versions - the functions are provided in this way to
	    assist preemptive development - code both move and copy with
	    Qt 1.x to be prepared.
*/
bool QDragObject::drag(DragMode mode)
{
    if ( manager )
	return manager->drag( this, mode );
    else
	return FALSE;
}



/*!  Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}


/*! \class QDragObject qdragobject.h

  \brief The QDragObject encapsulates MIME-based drag-and-drop
  interaction.

  \ingroup kernel

  Drag-and-drop in Qt uses the MIME open standard, to allow
  independently developers applications to exchange information.

  To start a drag, for example in a \link QWidget::mouseMoveEvent
  mouse motion event\endlink, create an object of the
  QDragObject subclass appropriate for your media, such as
  QTextDrag for text and QImageDrag for images. Then call
  the drag() method. This is all you need for simple dragging
  of existing types.

  To be able to receive media dropped on a widget, multiply-inherit
  the QDropSite class and override the
  \link QWidget::dragEnterEvent() dragEnterEvent()\endlink,
  \link QWidget::dragMoveEvent() dragMoveEvent()\endlink,
  \link QWidget::dragLeaveEvent() dragLeaveEvent()\endlink, and
  \link QWidget::dropEvent() dropEvent()\endlink event handler methods.

  Support for specific media types is provided by subclasses of
  QDragObject.
  For example,
  QTextDrag provides support for
   the "<tt>text/plain</tt>" MIME type (ordinary unformatted text), and
   the Unicode formats "<tt>text/utf8</tt>" and "<tt>text/utf16</tt>";
  QImageDrag provides for "<tt>image/</tt><tt>*</tt>",
  where <tt>*</tt>
  is all the \link QImageIO image formats that Qt supports\endlink,
  and the QUrlDrag subclass provides "<tt>url/url</tt>",
  a standard format for transferring a list of filenames.

  To implement drag-and-drop of some type of media for which there
  is no available QDragObject subclass, the
  first and most important step is to look for existing formats
  that are appropriate - the Internet Assigned Numbers Authority
  (<a href=http://www.iana.org>IANA</a>) provides a
  <a href=http://www.isi.edu/in-notes/iana/assignments/media-types/>
  hierarchical list of MIME media types</a>
  at the Information Sciences Institute
  (<a href=http://www.isi.edu>ISI</a>).
  This maximizes inter-operability of your software.

  To support an additional media type, subclass either QDragObject
  or QStoredDrag. Subclass QDragObject when you need to provide
  support for multiple media types. Subclass the simpler QStoredDrag
  when one type is sufficient.

  Subclasses of QDragObject will override the format() and
  encodedData() members, and provide a set-method to encode
  the media data and static members canDecode()
  and decode() to decode incoming data.  QImageDrag
  is an example of such a class in Qt. Of course, you can
  provide drag-only or drop-only support for a media type
  by omitting some of these methods.

  Subclasses of QStoredDrag provide a set-method to encode
  the media data and static members canDecode()
  and decode() to decode incoming data.

  <h3>Inter-operating with existing applications</h3>
  On X11, the public
  <a href=http://www.cco.caltech.edu/~jafl/xdnd/>XDND protocol</a>
  is used, while on Windows Qt uses the OLE standard.  On X11,
  XDND uses MIME, so no translation is necessary.  On Windows,
  MIME-aware applications can communicate by using clipboard
  format names that are MIME types. Internally, Qt has facilities
  for translating all proprietary clipboard formats to and from
  MIME.  This interface will be made public at some time, but
  if you need to do such translations now, contact your Qt
  Technical Support service.
*/

/*! \class QTextDrag qdragobject.h

  \brief The QTextDrag provides a drag-and-drop object for
	      transferring plain and Unicode text.

  \ingroup kernel

  Plain text is single- or multi-line 8-bit text in the local encoding.

  Qt provides no built-in mechanism for delivering only single-line.

  Drag&Drop text does \e not have a NUL terminator when it
  is dropped onto the target.

  For detailed information about drag-and-drop, see the QDragObject class.
*/


/*!  Creates a text drag object and sets it to \a text.  \a dragSource
  must be the drag source, \a name is the object name. */

QTextDrag::QTextDrag( const QString &text,
		      QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    setText( text );
}


/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QTextDrag::QTextDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
}


/*!  Destroys the text drag object and frees all allocated resources.
*/

QTextDrag::~QTextDrag()
{
    // nothing
}


/*!
  Sets the text to be dragged.  You will need to call this if you did
  not pass the text during construction.
*/
void QTextDrag::setText( const QString &text )
{
    txt = text;
}

static const char* text_formats[] = { // All must start with "text/"
	"text/utf16",
	"text/utf8",
	"text/plain", // LAST
	0
    };

const char * QTextDrag::format(int i) const
{
    for (int j=0; i>=0 && text_formats[j]; j++) {
	const char* fmt = text_formats[j]+5;
	QTextCodec *codec = QTextCodec::codecForName(fmt);
	if ( codec || !text_formats[j+1] ) {
	    if ( !i )
		return text_formats[j];
	    i--;
	}
    }

    return 0;
}

QByteArray QTextDrag::encodedData(const char* mime) const
{
    QCString r;
    if ( 0==strnicmp(mime,"text/",5) ) {
	QTextCodec *codec = QTextCodec::codecForName(mime+5);
	if (codec) {
	    int l;
	    r = codec->fromUnicode(txt,l);
	} else if ( 0==stricmp(mime,"text/plain") ) {
	    r = txt.ascii();
	}
    }
    return r;
}

/*!
  Returns TRUE if the information in \a e can be decoded into a QString.
  \sa decode()
*/
bool QTextDrag::canDecode( QMimeSource* e )
{
    for ( int i=0; text_formats[i]; i++ )
	if ( e->provides( text_formats[i] ) )
	    return TRUE;
    return FALSE;
}

/*!
  Attempts to decode the dropped information in \a e
  into \a str, returning TRUE if successful.

  \sa decode()
*/
bool QTextDrag::decode( QMimeSource* e, QString& str )
{
    QTextCodec* codec = 0;
    QByteArray payload;
    for ( int i=0; !codec && text_formats[i]; i++ ) {
	payload = e->encodedData(text_formats[i]);
	if ( payload.size() ) {
	    codec = QTextCodec::codecForName(text_formats[i]+5); // 5="text/"
	    if ( !codec && !text_formats[i+1] ) {
		// text/plain
		str = payload;
		return TRUE;
	    }
	}
    }
    if ( !codec )
	return FALSE;
    str = codec->toUnicode(payload);
    return TRUE;
}


/*! \class QImageDrag qdragobject.h

  \brief The QImageDrag provides a drag-and-drop object for
  transferring images.

  \ingroup kernel

  Images are offered to the receiving application in multiple formats,
  determined by the \link QImage::outputFormats() output formats\endlink
  in Qt.

  For detailed information about drag-and-drop, see the QDragObject class.
*/


/*!  Creates an image drag object and sets it to \a image.  \a dragSource
  must be the drag source, \a name is the object name. */

QImageDrag::QImageDrag( QImage image,
			QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    setImage( image );
}

/*!  Creates a default text drag object.  \a dragSource must be the drag
  source, \a name is the object name.
*/

QImageDrag::QImageDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
}


/*!  Destroys the image drag object and frees all allocated resources.
*/

QImageDrag::~QImageDrag()
{
    // nothing
}


/*!
  Sets the image to be dragged.  You will need to call this if you did
  not pass the image during construction.
*/
void QImageDrag::setImage( QImage image )
{
    img = image;
    // ### should detach?
    ofmts = QImage::outputFormats();
    ofmts.remove("PBM");
    if ( image.depth()!=32 ) {
	// BMP better than PPM for paletted images
	if ( ofmts.remove("BMP") ) // move to front
	    ofmts.insert(0,"BMP");
    }
    // Could do more magic to order mime types
}

const char * QImageDrag::format(int i) const
{
    if ( i < (int)ofmts.count() ) {
	static QCString str;
	str.sprintf("image/%s",(((QImageDrag*)this)->ofmts).at(i));
	str = str.lower();
	if ( str == "image/pbmraw" )
	    str = "image/ppm";
	return str;
    } else {
	return 0;
    }
}

QByteArray QImageDrag::encodedData(const char* fmt) const
{
    if ( qstrnicmp( fmt, "image/", 6 )==0 ) {
	QCString f = fmt+6;
	QByteArray data;
	QBuffer w( data );
	w.open( IO_WriteOnly );
	QImageIO io( &w, f.upper() );
	io.setImage( img );
	if  ( !io.write() )
	    return QByteArray();
	w.close();
	return data;
    } else {
	return QByteArray();
    }
}

/*!
  Returns TRUE if the information in \a e can be decoded into an image.
  \sa decode()
*/
bool QImageDrag::canDecode( QMimeSource* e )
{
    return e->provides( "image/ppm" )
        || e->provides( "image/bmp" )
        || e->provides( "image/gif" );
    // ### more Qt images types
}

/*!
  Attempts to decode the dropped information in \a e
  into \a img, returning TRUE if successful.

  \sa canDecode()
*/
bool QImageDrag::decode( QMimeSource* e, QImage& img )
{
    QByteArray payload = e->encodedData( "image/ppm" );
    if ( payload.isEmpty() )
	payload = e->encodedData( "image/bmp" );
    if ( payload.isEmpty() )
	payload = e->encodedData( "image/gif" );
    // ### more Qt images types
    if ( payload.isEmpty() )
	return FALSE;

    img.loadFromData(payload);
    return !img.isNull();
}

/*!
  Attempts to decode the dropped information in \a e
  into \a pm, returning TRUE if successful.

  This is a convenience function that converts
  to \a pm via a QImage.

  \sa canDecode()
*/
bool QImageDrag::decode( QMimeSource* e, QPixmap& pm )
{
    QImage img;
    if ( decode( e, img ) )
	return pm.convertFromImage( img );
    return FALSE;
}




/*!
  \class QStoredDrag qdragobject.h
  \brief Simple stored-value drag object for arbitrary MIME data.

  When a block of data only has one representation, you can use
  a QStoredDrag to hold it.

  For detailed information about drag-and-drop, see the QDragObject class.
*/

/*!
  Constructs a QStoredDrag.  The parameters are passed
  to the QDragObject constructor, and the format is set to \a mimeType.

  The data will be unset.  Use setEncodedData() to set it.
*/
QStoredDrag::QStoredDrag( const char* mimeType, QWidget * dragSource, const char * name ) :
    QDragObject(dragSource,name)
{
    d = new QStoredDragData();
    d->fmt = qstrdup(mimeType);
}

/*!
  Destroys the drag object and frees all allocated resources.
*/
QStoredDrag::~QStoredDrag()
{
    delete d->fmt;
    delete d;
}

const char * QStoredDrag::format(int i) const
{
    if ( i==0 )
	return d->fmt;
    else
	return 0;
}


/*!
  Sets the encoded data of this drag object to \a encodedData.  The
  encoded data is what's delivered to the drop sites, and must be in a
  strictly defined and portable format.

  The drag object can't be dropped (by the user) until this function
  has been called.
*/

void QStoredDrag::setEncodedData( const QByteArray & encodedData )
{
    d->enc = encodedData.copy();
}

/*!
  Returns the stored data.

  \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if ( !qstricmp(m,d->fmt) )
	return d->enc;
    else
	return QByteArray();
}


/*!
  \class QUrlDrag qdragobject.h
  \brief Provides for drag-and-drop of a list of file references.

  URLs are a useful way to refer to files that may be distributed
  across multiple machines.  Much of the time a URL will refer to
  a file on a machine local to both the drag source and the
  drop target, and so the URL will be equivalent to passing a
  filename, but more extensible.
*/

/*!
  Creates an object to drag the list of urls in \a urls.
  The \a dragSource and \a name arguments are passed on to
  QStoredDrag.
*/
QUrlDrag::QUrlDrag( QStrList urls,
	    QWidget * dragSource, const char * name ) :
    QStoredDrag( "url/url", dragSource, name )
{
    setUrls(urls);
}

/*!
  Creates a object to drag.  You will need to call
  setUrls() before you start the drag().
*/
QUrlDrag::QUrlDrag( QWidget * dragSource, const char * name ) :
    QStoredDrag( "url/url", dragSource, name )
{
}

/*!
  Destroys the object.
*/
QUrlDrag::~QUrlDrag()
{
}

/*!
  Changes the list of \a urls to be dragged.
*/
void QUrlDrag::setUrls( QStrList urls )
{
    QByteArray a;
    int c=0;
    for (const char* s = urls.first(); s; s = urls.next() ) {
	int l = strlen(s)+1;
	a.resize(c+l);
	memcpy(a.data()+c,s,l);
	c+=l;
    }
    a.resize(c-1); // chop off last nul
    setEncodedData(a);
}


/*!
  Returns TRUE if decode() would be able to decode \a e.
*/
bool QUrlDrag::canDecode( QMimeSource* e )
{
    return e->provides( "url/url" );
}

/*!
  Decodes URLs from \a e, placing the result in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URLs.
*/
bool QUrlDrag::decode( QMimeSource* e, QStrList& l )
{
    QByteArray payload = e->encodedData( "url/url" );
    if ( payload.size() ) {
	l.clear();
	l.setAutoDelete(TRUE);
	uint c=0;
	char* d = payload.data();
	while (c < payload.size()) {
	    uint f = c;
	    while (c < payload.size() && d[c])
		c++;
	    if ( c < payload.size() ) {
		l.append( d+f );
		c++;
	    } else {
		QCString s(d+f,c-f+1);
		l.append( s );
	    }
	}
	return TRUE;
    }
    return FALSE;
}

static
int htod(int h)
{
    if (isdigit(h)) return h-'0';
    return tolower(h)-'a';
}

/*!
  Returns the name of a local file equivalent to \a url,
  or a null string if \a url is not a local file.
*/
QString QUrlDrag::urlToLocalFile(const char* url)
{
    QString result;

    if ( url && 0==qstrnicmp(url,"file:/",6) ) {
	url += 6;
	if ( url[0] != '/' || url[1] == '/' ) {
	    // It is local.
	    while (*url) {
		switch (*url) {
		  case '|': result += ':';
		    break;
		  case '+': result += ' ';
		    break;
		  case '%': {
			int ch = url[1];
			if ( ch && url[2] ) {
			    ch = htod(ch)*16 + htod(url[2]);
			    result += ch;
			}
		    }
		    break;
		  default:
		    result += *url;
		}
		++url;
	    }
	}
    }

    return result;
}

/*!
  Decodes URLs from \a e, converts them to local files if they refer to
  local files, and places them in \a l (which is first cleared).

  Returns TRUE if the event contained a valid list of URLs.
  The list will be empty if no URLs were local files.
*/
bool QUrlDrag::decodeLocalFiles( QMimeSource* e, QStrList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    l.setAutoDelete(TRUE);
    for (const char* s=u.first(); s; s=u.next()) {
	QString lf = urlToLocalFile(s);
	if ( !lf.isNull() )
	    l.append( lf );
    }
    return TRUE;
}


/*!
  If the source of the drag operation is a widget in this application,
  this function returns that source, otherwise 0.  The source of the
  operation is the first parameter to to drag object subclass.

  This is useful if your widget needs special behavior when dragging
  to itelf, etc.

  See QDragObject::QDragObject() and subclasses.
*/
QWidget* QDragMoveEvent::source() const
{
    return manager ? manager->dragSource : 0;
}

/*!
  If the source of the drag operation is a widget in this application,
  this function returns that source, otherwise 0.  The source of the
  operation is the first parameter to to drag object subclass.

  This is useful if your widget needs special behavior when dragging
  to itelf, etc.

  See QDragObject::QDragObject() and subclasses.
*/
QWidget* QDropEvent::source() const
{
    return manager ? manager->dragSource : 0;
}
