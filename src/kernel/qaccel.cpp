/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qaccel.cpp#86 $
**
** Implementation of QAccel class
**
** Created : 950419
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

#include "qaccel.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsignal.h"
#include "qwhatsthis.h"
#include "qguardedptr.h"

// BEING REVISED: paul
/*!
  \class QAccel qaccel.h
  \brief The QAccel class handles keyboard accelerator and shortcut keys.

  \ingroup kernel

  A QAccel contains a list of accelerator items. Each accelerator item
  consists of an identifier and a keyboard code combined with modifiers
  (\c SHIFT, \c CTRL, \c ALT or \c UNICODE_ACCEL).

  For example, <code>CTRL + Key_P</code> could be a shortcut for printing
  a document. The key codes are listed in qnamespace.h.

  When pressed, an accelerator key sends out the signal activated() with a
  number that identifies this particular accelerator item.  Accelerator
  items can also be individually connected, so that two different keys
  will activate two different slots (see connectItem()).

  A QAccel object handles key events to the
  \link QWidget::topLevelWidget() top level window\endlink
  containing \a parent, and hence to any child widgets of that window.
  Note that the accelerator will be deleted only when the \a parent
  is deleted, and will consume relevant key events until then.

  Example:
  \code
     QAccel *a = new QAccel( myWindow );	// create accels for myWindow
     a->connectItem( a->insertItem(Key_P+CTRL), // adds Ctrl+P accelerator
		     myWindow,			// connected to myWindow's
		     SLOT(printDoc()) );	// printDoc() slot
  \endcode

  \sa QKeyEvent QWidget::keyPressEvent() QMenuData::setAccel()
  QButton::setAccel()
  <a href="guibooks.html#fowler">GUI Design Handbook: Keyboard Shortcuts,</a>
*/


struct QAccelItem {				// internal accelerator item
    QAccelItem( int k, int i )
	{ key=k; id=i; enabled=TRUE; signal=0; }
   ~QAccelItem()	       { delete signal; }
    int		id;
    int		key;
    bool	enabled;
    QSignal    *signal;
    QString whatsthis;
};


typedef QList<QAccelItem> QAccelList; // internal accelerator list


class QAccelPrivate {
public:
    QAccelPrivate() { aitems.setAutoDelete( TRUE ); ignorewhatsthis = FALSE;}
    ~QAccelPrivate() {}
    QAccelList aitems;
    bool enabled;
    QGuardedPtr<QWidget> tlw;
    QGuardedPtr<QWidget> watch;
    bool ignorewhatsthis;
};


static QAccelItem *find_id( QAccelList &list, int id )
{
    register QAccelItem *item = list.first();
    while ( item && item->id != id )
	item = list.next();
    return item;
}

static QAccelItem *find_key( QAccelList &list, int key, QChar ch )
{
    register QAccelItem *item = list.first();
    while ( item ) {
	int k = item->key;
	int km = k & Qt::MODIFIER_MASK;
	QChar kc = QChar(k & 0xffff);
	if ( k & Qt::UNICODE_ACCEL )
	{
	    if ( km ) {
		// Modifiers must match...
		QChar c;
		if ( (key & Qt::CTRL) && (ch < ' ') )
		    c = ch.unicode()+'@'+' '; // Ctrl-A is ASCII 001, etc.
		else
		    c = ch;
		if ( kc.lower() == c.lower()
		  && (key & Qt::MODIFIER_MASK) == km )
		    break;
	    } else {
		// No modifiers requested, ignore Shift but require others...
		if ( kc == ch
		  && (key & (Qt::MODIFIER_MASK^Qt::SHIFT)) == km )
		    break;
	    }
	} else if ( k == key ) {
	    break;
	}
	item = list.next();
    }
    return item;
}


/*!
  Constructs a QAccel object with a parent widget and a name.
*/

QAccel::QAccel( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    d = new QAccelPrivate;
    d->enabled = TRUE;
    d->watch = parent;
    if ( d->watch ) {				// install event filter
	d->tlw = d->watch->topLevelWidget();
	d->tlw->installEventFilter( this );
    } else {
#if defined(CHECK_NULL)
	qWarning( "QAccel: An accelerator must have a parent or a watch widget" );
#endif
    }
}

/*!
  Constructs a QAccel object with a watch widget, a parent object and a
  name.

  The accelerator operates on the the watch widget.
*/
QAccel::QAccel( QWidget* watch, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QAccelPrivate;
    d->enabled = TRUE;
    d->watch = watch;
    if ( watch ) {				// install event filter
	d->tlw = d->watch->topLevelWidget();
	d->tlw->installEventFilter( this );
    } else {
#if defined(CHECK_NULL)
	qWarning( "QAccel: An accelerator must have a parent or a watch widget" );
#endif
    }
}

/*!
  Destroys the accelerator object.
*/

QAccel::~QAccel()
{
    delete d;
}


/*!
  \fn void QAccel::activated( int id )
  This signal is emitted when an accelerator key is pressed. \e id is
  a number that identifies this particular accelerator item.
*/

/*!
  Returns TRUE if the accelerator is enabled, or FALSE if it is disabled.
  \sa setEnabled(), isItemEnabled()
*/

bool QAccel::isEnabled() const
{
    return d->enabled;
}


/*!
  Enables the accelerator if \e enable is TRUE, or disables it if
  \e enable is FALSE.

  Individual keys can also be enabled or disabled.

  \sa isEnabled(), setItemEnabled()
*/

void QAccel::setEnabled( bool enable )
{
    d->enabled = enable;
}


/*!
  Returns the number of accelerator items.
*/

uint QAccel::count() const
{
    return d->aitems.count();
}


static int get_seq_id()
{
    static int seq_no = -2;  // -1 is used as return value in findKey()
    return seq_no--;
}

/*!
  Inserts an accelerator item and returns the item's identifier.

  \arg \e key is a key code plus a combination of SHIFT, CTRL and ALT.
  \arg \e id is the accelerator item id.

  If \e id is negative, then the item will be assigned a unique
  negative identifier.

  \code
    QAccel *a = new QAccel( myWindow );		// create accels for myWindow
    a->insertItem( Key_P + CTRL, 200 );		// Ctrl+P to print document
    a->insertItem( Key_X + ALT , 201 );		// Alt+X  to quit
    a->insertItem( UNICODE_ACCEL + 'q', 202 );	// Unicode 'q' to quit
    a->insertItem( Key_D );			// gets a unique negative id
    a->insertItem( Key_P + CTRL + SHIFT );	// gets a unique negative id
  \endcode
*/

int QAccel::insertItem( int key, int id )
{
    if ( id == -1 )
	id = get_seq_id();
    d->aitems.insert( 0, new QAccelItem(key,id) );
    return id;
}

/*!
  Removes the accelerator item with the identifier \e id.
*/

void QAccel::removeItem( int id )
{
    if ( find_id( d->aitems, id) )
	d->aitems.remove();
}


/*!
  Removes all accelerator items.
*/

void QAccel::clear()
{
    d->aitems.clear();
}


/*!
  Returns the key code of the accelerator item with the identifier \e id,
  or zero if the id cannot be found.
*/

int QAccel::key( int id )
{
    QAccelItem *item = find_id( d->aitems, id);
    return item ? item->key : 0;
}


/*!
  Returns the identifier of the accelerator item with the key code \e key, or
  -1 if the item cannot be found.
*/

int QAccel::findKey( int key ) const
{
    QAccelItem *item = find_key( d->aitems, key, QChar(key & 0xffff) );
    return item ? item->id : -1;
}


/*!
  Returns TRUE if the accelerator item with the identifier \e id is enabled.
  Returns FALSE if the item is disabled or cannot be found.
  \sa setItemEnabled(), isEnabled()
*/

bool QAccel::isItemEnabled( int id ) const
{
    QAccelItem *item = find_id( d->aitems, id);
    return item ? item->enabled : FALSE;
}


/*!
  Enables or disables an accelerator item.
  \arg \e id is the item identifier.
  \arg \e enable specifies whether the item should be enabled or disabled.

  \sa isItemEnabled(), isEnabled()
*/

void QAccel::setItemEnabled( int id, bool enable )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item )
	item->enabled = enable;
}


/*!
  Connects an accelerator item to a slot/signal in another object.

  \arg \e id is the accelerator item id.
  \arg \e receiver is the object to receive a signal.
  \arg \e member is a slot or signal function in the receiver.

  \code
    a->connectItem( 201, mainView, SLOT(quit()) );
  \endcode

  \sa disconnectItem()
*/

bool QAccel::connectItem( int id, const QObject *receiver, const char *member )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item ) {
	if ( !item->signal ) {
	    item->signal = new QSignal;
	    CHECK_PTR( item->signal );
	}
	return item->signal->connect( receiver, member );
    }
    return FALSE;
}

/*!
  Disconnects an accelerator item from a function in another
  object.
  \sa connectItem()
*/

bool QAccel::disconnectItem( int id, const QObject *receiver,
			     const char *member )
{
    QAccelItem *item = find_id( d->aitems, id);
    if ( item && item->signal )
	return item->signal->disconnect( receiver, member );
    return FALSE;
}


/*!
  Make sure that the accelerator is watching the correct event
  filter.
*/

void QAccel::repairEventFilter()
{
    QWidget *ntlw;
    if ( d->watch )
	ntlw = d->watch->topLevelWidget();
    else
	ntlw = 0;

    if ( (QWidget*) d->tlw != ntlw ) {
	if ( d->tlw )
	    d->tlw->removeEventFilter( this );
	d->tlw = ntlw;
	if ( d->tlw )
	    d->tlw->installEventFilter( this );
    }
}


/*!
  Processes accelerator events intended for the top level widget.
*/

bool QAccel::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::Reparent && d->watch == o ) {
	repairEventFilter();
    } else  if ( d->enabled &&
	 ( e->type() == QEvent::Accel || e->type() == QEvent::AccelAvailable) &&
	 d->watch && d->watch->isVisible() ) {
	QKeyEvent *k = (QKeyEvent *)e;
	int key = k->key();
	if ( k->state() & ShiftButton )
	    key |= SHIFT;
	if ( k->state() & ControlButton )
	    key |= CTRL;
	if ( k->state() & AltButton )
	    key |= ALT;
	QAccelItem *item = find_key( d->aitems, key, k->text()[0] );
	bool b = QWhatsThis::inWhatsThisMode();
	if ( item && ( item->enabled || b )) {
	    if (e->type() == QEvent::Accel) {
		if ( b && !d->ignorewhatsthis ) {
		    QWhatsThis::leaveWhatsThisMode( item->whatsthis );
		}
		else if ( item->enabled ){
		    if ( item->signal )
			item->signal->activate();
		    else
			emit activated( item->id );
		}
	    }
	    k->accept();
	    return TRUE;
	}
    }
    return QObject::eventFilter( o, e );
}



/*!
  Returns the shortcut key for \a string, or 0 if \a string has no
  shortcut sequence.

  For example, acceleratorKey("E&xit") returns ALT+Key_X,
  shortcutChar("&Exit") returns ALT+Key_E and shortcutChar("Exit")
  returns 0.  (In code that does not inherit the Qt namespace class,
  you need to write e.g. Qt::ALT+Qt::Key_X.)

  We provide a \link accelerators.html list of common accelerators
  \endlink in English.  (At the time of writing the Microsoft and The
  Open Group appear to not have issued such recommendations for other
  languages.)
*/

int QAccel::shortcutKey( const QString &str )
{
    int p = 0;
    while ( p >= 0 ) {
	p = str.find( '&', p ) + 1;
	if ( p <= 0 || p == (int)str.length() )
	    return 0;
	if ( str[p] != '&' ) {
	    QChar c = str[p];
	    if ( c < QChar(' ') || ( c > QChar('\176') && c < QChar('\240') ) )
		return 0;
	    c = c.upper();
	    return c.unicode() + ALT + UNICODE_ACCEL;
	}
    }
    return 0;
}

/*!
   Creates an accelerator string for the key \a k.
   For instance CTRL+Key_O gives "Ctrl+O".  The "Ctrl" etc.
   are \link QObject::tr() translated\endlink in the "QAccel" scope.

   \sa stringToKey()
*/
QString QAccel::keyToString( int k )
{
    QString s;
    if ( (k & CTRL) == CTRL ) {
	s += tr( "Ctrl" );
    }
    if ( (k & ALT) == ALT ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Alt" );
    }
    if ( (k & SHIFT) == SHIFT ) {
	if ( !s.isEmpty() )
	    s += tr( "+" );
	s += tr( "Shift" );
    }
    k &= ~(SHIFT | CTRL | ALT);
    QString p;
    if ( (k & UNICODE_ACCEL) == UNICODE_ACCEL ) {
	p = QChar(k & 0xffff);
    } else if ( k >= Key_F1 && k <= Key_F24 ) {
	p = tr( "F%1" ).arg(k - Key_F1 + 1);
    } else if ( k > Key_Space && k <= Key_AsciiTilde ) {
	p.sprintf( "%c", k );
    } else {
	switch ( k ) {
	    case Key_Space:
		p = tr( "Space" );
		break;
	    case Key_Escape:
		p = tr( "Esc" );
		break;
	    case Key_Tab:
		p = tr( "Tab" );
		break;
	    case Key_Backtab:
		p = tr( "Backtab" );
		break;
	    case Key_Backspace:
		p = tr( "Backspace" );
		break;
	    case Key_Return:
		p = tr( "Return" );
		break;
	    case Key_Enter:
		p = tr( "Enter" );
		break;
	    case Key_Insert:
		p = tr( "Ins" );
		break;
	    case Key_Delete:
		p = tr( "Del" );
		break;
	    case Key_Pause:
		p = tr( "Pause" );
		break;
	    case Key_Print:
		p = tr( "Print" );
		break;
	    case Key_SysReq:
		p = tr( "SysReq" );
		break;
	    case Key_Home:
		p = tr( "Home" );
		break;
	    case Key_End:
		p = tr( "End" );
		break;
	    case Key_Left:
		p = tr( "Left" );
		break;
	    case Key_Up:
		p = tr( "Up" );
		break;
	    case Key_Right:
		p = tr( "Right" );
		break;
	    case Key_Down:
		p = tr( "Down" );
		break;
	    case Key_Prior:
		p = tr( "PgUp" );
		break;
	    case Key_Next:
		p = tr( "PgDown" );
		break;
	    case Key_CapsLock:
		p = tr( "CapsLock" );
		break;
	    case Key_NumLock:
		p = tr( "NumLock" );
		break;
	    case Key_ScrollLock:
		p = tr( "ScrollLock" );
		break;
	    default:
		p.sprintf( "<%d?>", k );
		break;
	}
    }
    if ( s.isEmpty() )
	s = p;
    else {
	s += tr( "+" );
	s += p;
    }
    return s;
}

/*!
   Creates an accelerator code for the string \a s.
   For example "Ctrl+O" gives CTRL+UNICODE_ACCEL+'O'.
   The strings "Ctrl", "Shift", "Alt" and their
   \link QObject::tr() translated\endlink equivalents
   in the "QAccel" scope are recognized.

   A common usage of this function is to provide
   translatable accelerator values for menus:

   \code
	QPopupMenu* file = new QPopupMenu(this);
	file->insertItem( p1, tr("&Open..."), this, SLOT(open()),
	    QAccel::stringToKey(tr("Ctrl+O")) );
   \endcode

   Note that this function currently only supports character
   accelerators (unlike keyToString() which can produce
   Ctrl+Backspace, etc. from the appropriate key codes).
*/
int QAccel::stringToKey( const QString & s )
{
    int k = 0;
    int p = s.findRev('+');
    if ( p > 0 ) {
	k = s[p+1].unicode() | UNICODE_ACCEL;
	if ( s.contains("Ctrl+") || s.contains(tr("Ctrl")+"+") )
	    k |= CTRL;
	if ( s.contains("Shift+") || s.contains(tr("Shift")+"+") )
	    k |= SHIFT;
	if ( s.contains("Alt+") || s.contains(tr("Alt")+"+") )
	    k |= ALT;
    }
    else if ( s.length() == 1 ) {
	k = s[0].unicode() | UNICODE_ACCEL;
    }
    return k;
}


/*!
  Sets a Whats This help for a certain accelerator.

  \arg \e id is the accelerator item id.
  \arg \e text is the Whats This help text.

  The text will be shown when the application is in What's This mode
  and the user either hits the respective accelerator key or selects a
  menu item that has been attached to this accelerator.

  \sa whatsThis(), QWhatsThis::inWhatsThisMode()
 */
void QAccel::setWhatsThis( int id, const QString& text )
{

    QAccelItem *item = find_id( d->aitems, id);
    if ( item )
	item->whatsthis = text;
}

/*!
  Returns the Whats This help text for the specified item \e id or
  QString::null if no text has been defined yet.

  \sa setWhatsThis()
 */
QString QAccel::whatsThis( int id ) const
{

    QAccelItem *item = find_id( d->aitems, id);
    return item? item->whatsthis : QString::null;
}

/*!\internal
 */
void QAccel::setIgnoreWhatsThis( bool b)
{
    d->ignorewhatsthis = b;
}

/*!\internal
 */
bool QAccel::ignoreWhatsThis() const
{
    return d->ignorewhatsthis;
}



/*! \page accelerators.html

<title>Standard Accelerators</title>
</head><body bgcolor="#ffffff">
\postheader

<h1 align="center">Standard Accelerator Keys</h1>

Microsoft defines a large number of standard accelerators; the Open
Group defines a somewhat smaller number.  Here is a list of the ones
that involve letter keys, sorted alphabetically.  The boldfaced letter
(A in About) together with Alt is Microsoft's accelerator; where the
Open Group has a different standard we explain the difference in
parentheses.

<ul>
<li><b><u>A</u></b>bout
<li>Always on <b><u>T</u></b>op
<li><b><u>A</u></b>pply
<li><b><u>B</u></b>ack
<li><b><u>B</u></b>rowse
<li><b><u>C</u></b>lose (CDE: Alt-F4.  Alt-F4 is "close window" in Windows.)
<li><b><u>C</u></b>opy (CDE: Ctrl-C, Ctrl-Insert)
<li><b><u>C</u></b>opy Here
<li>Create <b><u>S</u></b>hortcut
<li>Create <b><u>S</u></b>hortcut Here
<li>Cu<b><u>t</u></b>
<li><b><u>D</u></b>elete
<li><b><u>E</u></b>dit
<li><b><u>E</u></b>xit
<li><b><u>E</u></b>xplore
<li><b><u>F</u></b>ile
<li><b><u>F</u></b>ind
<li><b><u>H</u></b>elp
<li>Help <b><u>T</u></b>opics
<li><b><u>H</u></b>ide
<li><b><u>I</u></b>nsert
<li>Insert <b><u>O</u></b>bject
<li><b><u>L</u></b>ink Here
<li>Ma<b><u>x</u></b>imize
<li>Mi<b><u>n</u></b>imize
<li><b><u>M</u></b>ove
<li><b><u>M</u></b>ove Here
<li><b><u>N</u></b>ew
<li><b><u>N</u></b>ext
<li><b><u>N</u></b>o
<li><b><u>O</u></b>pen
<li>Open <b><u>W</u></b>ith
<li>Page Set<b><u>u</u></b>p
<li><b><u>P</u></b>aste
<li>Paste <b><u>L</u></b>ink
<li>Paste <b><u>S</u></b>hortcut
<li>Paste <b><u>S</u></b>pecial
<li><b><u>P</u></b>ause
<li><b><u>P</u></b>lay
<li><b><u>P</u></b>rint
<li><b><u>P</u></b>rint Here
<li>P<b><u>r</u></b>operties
<li><b><u>Q</u></b>uick View
<li><b><u>R</u></b>edo (CDE: Ctrl-Y, Alt-Backspace)
<li><b><u>R</u></b>epeat
<li><b><u>R</u></b>estore
<li><b><u>R</u></b>esume
<li><b><u>R</u></b>etry
<li><b><u>R</u></b>un
<li><b><u>S</u></b>ave
<li>Save <b><u>A</u></b>s
<li>Select <b><u>A</u></b>ll
<li>Se<b><u>n</u></b>d To
<li><b><u>S</u></b>how
<li><b><u>S</u></b>ize
<li>S<b><u>p</u></b>lit
<li><b><u>S</u></b>top
<li><b><u>U</u></b>ndo (CDE says Ctrl-Z or Alt-Backspace)
<li><b><u>V</u></b>iew
<li><b><u>W</u></b>hat's This?
<li><b><u>W</u></b>indow
<li><b><u>Y</u></b>es
</ul>

The
<a href="http://www.amazon.com/exec/obidos/ASIN/1556156790/trolltech/t">
Microsoft book</a> has ISBN 1556156790.  The corresponding
<a href="http://www.amazon.com/exec/obidos/ASIN/1859121047/trolltech/t">
Open Group book</a> has ISBN 1859121047.  (The link does not work at
the time of writing, since Amazon, like most book stores, does not
supply it.  The Open Group books are \e very hard to find, and
also rather expensive.  If you really want it, OGPubs@opengroup.org
may be able to help.)

*/
