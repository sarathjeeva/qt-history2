/****************************************************************************
** $Id$
**
** Implementation of Win32 startup routines and event handling
**
** Created : 931203
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
#if defined(QT_NON_COMMERCIAL)
#include "qnc_win.h"
#endif
#include <private/qapplication_p.h>
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qsessionmanager.h"
#include "qmime.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qthread.h"
#include "qwhatsthis.h" // ######## dependency
#include "qthread.h"
#include "qlibrary.h"
#include "qt_windows.h"
#include <private/qinternal_p.h>
#include <private/qinputcontext_p.h>
#include "qstyle.h"

#include <windowsx.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#if defined(QT_TABLET_SUPPORT)
#define PACKETDATA  ( PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | \
		      PK_ORIENTATION | PK_CURSOR )
#define PACKETMODE  0

extern bool chokeMouse;
// Wacom isn't being nice by not providing updated headers for this. so
// define it here so we can do things correctly according to their
// software implementation guide, and make it somewhat standard.
#if !defined(CSR_TYPE)
#define CSR_TYPE ( (UINT) 20 )
#endif

#include <wintab.h>
#include <pktdef.h>
#include <math.h>

typedef HCTX	( API *PtrWTOpen )(HWND, LPLOGCONTEXT, BOOL);
typedef BOOL	( API *PtrWTClose )(HCTX);
typedef UINT	( API *PtrWTInfo )(UINT, UINT, LPVOID);
typedef BOOL	( API *PtrWTEnable )(HCTX, BOOL);
typedef BOOL	( API *PtrWTOverlap )(HCTX, BOOL);
typedef int	( API *PtrWTPacketsGet )(HCTX, int, LPVOID);
typedef BOOL	( API *PtrWTGet )(HCTX, LPLOGCONTEXT);

static PtrWTOpen	ptrWTOpen = 0;
static PtrWTClose	ptrWTClose = 0;
static PtrWTInfo	ptrWTInfo = 0;
static PtrWTEnable	ptrWTEnable = 0;
static PtrWTOverlap	ptrWTOverlap = 0;
static PtrWTPacketsGet	ptrWTPacketsGet = 0;
static PtrWTGet		ptrWTGet = 0;



static const int NPACKETQSIZE = 32;	// minimum size of queue.
static const double PI = 3.14159265359;

static PACKET localPacketBuf[NPACKETQSIZE];  // our own tablet packet queue.
static LOGCONTEXT lcMine;   // the logical context for the tablet ( describes capapilities )
static HCTX hTab;  // the hardware context for the tablet ( like a window handle )
static bool tilt_support;
static void prsInit(HCTX hTab);
static UINT prsAdjust(PACKET p, HCTX hTab);
static void initWinTabFunctions();	// resolve the WINTAB api functions
#endif

extern bool winPeekMessage( MSG* msg, HWND hWnd, UINT wMsgFilterMin,
			    UINT wMsgFilterMax, UINT wRemoveMsg );
extern bool winPostMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

#if defined(QT_ACCESSIBILITY_SUPPORT)
#include <qaccessible.h>
#include <winable.h>
#include <oleacc.h>
#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif

extern IAccessible *qt_createWindowsAccessible( QAccessibleInterface *object );
#endif // QT_ACCESSIBILITY_SUPPORT

// support for on-the-fly changes of the XP theme engine
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT		29
#define COLOR_MENUBAR			30
#endif

// support for xbuttons
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C
#define WM_XBUTTONDBLCLK                0x020D
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))
#define GET_XBUTTON_WPARAM(wParam)      (HIWORD(wParam))
#define XBUTTON1      0x0001
#define XBUTTON2      0x0002
#define MK_XBUTTON1         0x0020
#define MK_XBUTTON2         0x0040
#endif

// support for multi-media-keys on ME/2000/XP
#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND                   0x0319

#define FAPPCOMMAND_MOUSE 0x8000
#define FAPPCOMMAND_KEY   0
#define FAPPCOMMAND_OEM   0x1000
#define FAPPCOMMAND_MASK  0xF000
#define GET_APPCOMMAND_LPARAM(lParam) ((short)(HIWORD(lParam) & ~FAPPCOMMAND_MASK))
#define GET_DEVICE_LPARAM(lParam)     ((WORD)(HIWORD(lParam) & FAPPCOMMAND_MASK))
#define GET_MOUSEORKEY_LPARAM         GET_DEVICE_LPARAM
#define GET_FLAGS_LPARAM(lParam)      (LOWORD(lParam))
#define GET_KEYSTATE_LPARAM(lParam)   GET_FLAGS_LPARAM(lParam)

#define APPCOMMAND_BROWSER_BACKWARD       1
#define APPCOMMAND_BROWSER_FORWARD        2
#define APPCOMMAND_BROWSER_REFRESH        3
#define APPCOMMAND_BROWSER_STOP           4
#define APPCOMMAND_BROWSER_SEARCH         5
#define APPCOMMAND_BROWSER_FAVORITES      6
#define APPCOMMAND_BROWSER_HOME           7
#define APPCOMMAND_VOLUME_MUTE            8
#define APPCOMMAND_VOLUME_DOWN            9
#define APPCOMMAND_VOLUME_UP              10
#define APPCOMMAND_MEDIA_NEXTTRACK        11
#define APPCOMMAND_MEDIA_PREVIOUSTRACK    12
#define APPCOMMAND_MEDIA_STOP             13
#define APPCOMMAND_MEDIA_PLAY_PAUSE       14
#define APPCOMMAND_LAUNCH_MAIL            15
#define APPCOMMAND_LAUNCH_MEDIA_SELECT    16
#define APPCOMMAND_LAUNCH_APP1            17
#define APPCOMMAND_LAUNCH_APP2            18
#define APPCOMMAND_BASS_DOWN              19
#define APPCOMMAND_BASS_BOOST             20
#define APPCOMMAND_BASS_UP                21
#define APPCOMMAND_TREBLE_DOWN            22
#define APPCOMMAND_TREBLE_UP              23
#endif

static UINT WM95_MOUSEWHEEL = 0;

extern void qt_dispatchEnterLeave( QWidget*, QWidget* ); // qapplication.cpp
static int translateButtonState( int s, int type, int button );

/*
  Internal functions.
*/

Q_EXPORT
void qt_draw_tiled_pixmap( HDC, int, int, int, int,
			   const QPixmap *, int, int );

void qt_erase_background( HDC hdc, int x, int y, int w, int h,
			  const QColor &bg_color,
			  const QPixmap *bg_pixmap, int off_x, int off_y )
{
    if ( bg_pixmap && bg_pixmap->isNull() )	// empty background
	return;
    HPALETTE oldPal = 0;
    if ( QColor::hPal() ) {
	oldPal = SelectPalette( hdc, QColor::hPal(), FALSE );
	RealizePalette( hdc );
    }
    if ( bg_pixmap ) {
	qt_draw_tiled_pixmap( hdc, x, y, w, h, bg_pixmap, off_x, off_y );
    } else {
	HBRUSH brush = CreateSolidBrush( bg_color.pixel() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
	PatBlt( hdc, x, y, w, h, PATCOPY );
	SelectObject( hdc, oldBrush );
	DeleteObject( brush );
    }
    if ( QColor::hPal() ) {
	SelectPalette( hdc, oldPal, TRUE );
	RealizePalette( hdc );
    }
}




#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x020A
#endif

QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char	 appName[256];			// application name
static char	 appFileName[256];		// application file name
static HINSTANCE appInst	= 0;		// handle to app instance
static HINSTANCE appPrevInst	= 0;		// handle to prev app instance
static int	 appCmdShow	= 0;		// main window show command
static HWND	 curWin		= 0;		// current window
static HDC	 displayDC	= 0;		// display device context

// Session management
static bool	sm_blockUserInput    = FALSE;
static bool	sm_smActive	     = FALSE;
extern QSessionManager* qt_session_manager_self;
static bool	sm_cancel;

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static bool replayPopupMouseEvent = FALSE; // replay handling when popups close

static bool ignoreNextMouseReleaseEvent = FALSE; // ignore the next release event if
						 // return from a modal widget
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// mouse/keyboard grabbing
#endif

static bool	app_do_modal	   = FALSE;	// modal mode
extern QWidgetList *qt_modal_stack;
extern QDesktopWidget *qt_desktopWidget;
static QWidget *popupButtonFocus   = 0;
static bool	popupCloseDownMode = FALSE;
static bool	qt_try_modal( QWidget *, MSG *, int& ret );

QWidget	       *qt_button_down = 0;		// widget got last button-down

extern bool qt_dispatchAccelEvent( QWidget*, QKeyEvent* ); // def in qaccel.cpp

static HWND	autoCaptureWnd = 0;
static void	setAutoCapture( HWND );		// automatic capture
static void	releaseAutoCapture();

// ### Remove 4.0 [start] -------------------
typedef void (*VFPTR)();
// VFPTR qt_set_preselect_handler( VFPTR );
static VFPTR qt_preselect_handler = 0;
// VFPTR qt_set_postselect_handler( VFPTR );
static VFPTR qt_postselect_handler = 0;
Q_EXPORT VFPTR qt_set_preselect_handler( VFPTR handler )
{
    VFPTR old_handler = qt_preselect_handler;
    qt_preselect_handler = handler;
    return old_handler;
}
Q_EXPORT VFPTR qt_set_postselect_handler( VFPTR handler )
{
    VFPTR old_handler = qt_postselect_handler;
    qt_postselect_handler = handler;
    return old_handler;
}

typedef int (*QWinEventFilter) (MSG*);
static QWinEventFilter qt_win_event_filter = 0;

Q_EXPORT QWinEventFilter qt_set_win_event_filter (QWinEventFilter filter)
{
    QWinEventFilter old_filter = qt_win_event_filter;
    qt_win_event_filter = filter;
    return old_filter;
}
// ### Remove 4.0 [end] --------------------

bool qt_winEventFilter( MSG* msg )
{
    if ( qt_win_event_filter && qt_win_event_filter( msg )  )
	return TRUE;
    return qApp->winEventFilter( msg );
}

static void	msgHandler( QtMsgType, const char* );
static void     unregWinClasses();

static int	translateKeyCode( int );

extern QCursor *qt_grab_cursor();

#if defined(Q_WS_WIN)
#define __export
#endif

extern "C" LRESULT CALLBACK QtWndProc( HWND, UINT, WPARAM, LPARAM );

class QETWidget : public QWidget		// event translator widget
{
public:
    void	setWFlags( WFlags f )	{ QWidget::setWFlags(f); }
    void	clearWFlags( WFlags f ) { QWidget::clearWFlags(f); }
    QWExtra    *xtra()			{ return QWidget::extraData(); }
    bool	winEvent( MSG *m )	{ return QWidget::winEvent(m); }
    bool	translateMouseEvent( const MSG &msg );
    bool	translateKeyEvent( const MSG &msg, bool grab );
    bool	translateWheelEvent( const MSG &msg );
    bool	sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor=FALSE );
    bool	translatePaintEvent( const MSG &msg );
    bool	translateConfigEvent( const MSG &msg );
    bool	translateCloseEvent( const MSG &msg );
#if defined (QT_TABLET_SUPPORT)
	bool	translateTabletEvent( const MSG &msg, PACKET *localPacketBuf,
		                          int numPackets );
#endif
    void	repolishStyle( QStyle &style ) { styleChange( style ); }

};


static void set_winapp_name()
{
    static bool already_set = FALSE;
    if ( !already_set ) {
	already_set = TRUE;
#ifndef Q_OS_TEMP
	GetModuleFileNameA( 0, appFileName, sizeof(appFileName) );
#else
	GetModuleFileName( 0, appFilename.ucs2(), sizeof(appFileName) );
#endif
	const char *p = strrchr( appFileName, '\\' );	// skip path
	if ( p )
	    memcpy( appName, p+1, qstrlen(p) );
	int l = qstrlen( appName );
	if ( (l > 4) && !qstricmp( appName + l - 4, ".exe" ) )
	    appName[l-4] = '\0';		// drop .exe extension
    }
}


/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/

#if defined( Q_OS_TEMP )
Q_EXPORT void __cdecl qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QMemArray<pchar> &argv )
#else
Q_EXPORT
void qWinMain( HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdParam,
	       int cmdShow, int &argc, QMemArray<pchar> &argv )
#endif
{
    static bool already_called = FALSE;

    if ( already_called ) {
#if defined(QT_CHECK_STATE)
	qWarning( "Qt internal error: qWinMain should be called only once" );
#endif
	return;
    }
    already_called = TRUE;

  // Install default debug handler

#if defined(Q_CC_MSVC)
    qInstallMsgHandler( msgHandler );
#endif

  // Create command line

    set_winapp_name();

    char *p = cmdParam;
    char *p_end = p + strlen(p);

    argc = 1;
    argv[0] = appName;

    while ( *p && p < p_end ) {				// parse cmd line arguments
	while ( isspace((uchar) *p) )			// skip white space
	    p++;
	if ( *p && p < p_end ) {				// arg starts
	    int quote;
	    char *start, *r;
	    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
		quote = *p;
		start = ++p;
	    } else {
		quote = 0;
		start = p;
	    }
	    r = start;
	    while ( *p && p < p_end ) {
		if ( *p == '\\' ) {		// escape char?
		    p++;
		    if ( *p == '\"' || *p == '\'' )
			;			// yes
		    else
			p--;			// treat \ literally
		} else if ( quote ) {
		    if ( *p == quote ) {
			p++;
			if ( isspace((uchar) *p) )
			    break;
			quote = 0;
		    }
		} else {
		    if ( *p == '\"' || *p == '\'' ) {	// " or ' quote
			quote = *p++;
			continue;
		    } else if ( isspace((uchar) *p) )
			break;
		}
		if ( p )
		    *r++ = *p++;
	    }
	    if ( *p && p < p_end )
		p++;
	    *r = '\0';

	    if ( argc >= (int)argv.size()-1 )	// expand array
		argv.resize( argv.size()*2 );
	    argv[argc++] = start;
	}
    }
    argv[argc] = 0;
  // Get Windows parameters

    appInst = instance;
    appPrevInst = prevInstance;
    appCmdShow = cmdShow;
}

static void qt_show_system_menu( QWidget* tlw)
{
    HMENU menu = GetSystemMenu( tlw->winId(), FALSE );
    if ( !menu )
	return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

#ifndef Q_OS_TEMP
    EnableMenuItem( menu, SC_MINIMIZE, enabled);
    bool maximized  = IsZoomed( tlw->winId() );

    EnableMenuItem( menu, SC_MAXIMIZE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem( menu, SC_SIZE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_MOVE, maximized?disabled:enabled);
    EnableMenuItem( menu, SC_CLOSE, enabled);
#endif

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx( menu,
				TPM_LEFTALIGN  | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
				tlw->geometry().x(), tlw->geometry().y(),
				tlw->winId(),
				0);
    if (ret)
#ifdef Q_OS_TEMP
	DefWindowProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#else
	QtWndProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#endif
}

extern QFont qt_LOGFONTtoQFont(LOGFONT& lf,bool scale);

// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

static void qt_set_windows_resources()
{
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;
    QFont titleFont;
    QFont smallTitleFont;

    QT_WA( {
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfo( SPI_GETNONCLIENTMETRICS,
			      sizeof( ncm ), &ncm, NULL);
	menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,TRUE);
	titleFont = qt_LOGFONTtoQFont(ncm.lfCaptionFont,TRUE);
	smallTitleFont = qt_LOGFONTtoQFont(ncm.lfSmCaptionFont,TRUE);
    } , {
	// A version
	NONCLIENTMETRICSA ncm;
	ncm.cbSize = sizeof( ncm );
	SystemParametersInfoA( SPI_GETNONCLIENTMETRICS,
			      sizeof( ncm ), &ncm, NULL);
	menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,TRUE);
	messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,TRUE);
	statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,TRUE);
	titleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfCaptionFont,TRUE);
	smallTitleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfSmCaptionFont,TRUE);
    } );

    QApplication::setFont( menuFont, TRUE, "QPopupMenu" );
    QApplication::setFont( menuFont, TRUE, "QMenuBar" );
    QApplication::setFont( messageFont, TRUE, "QMessageBox" );
    QApplication::setFont( statusFont, TRUE, "QTipLabel" );
    QApplication::setFont( statusFont, TRUE, "QStatusBar" );
    QApplication::setFont( titleFont, TRUE, "QTitleBar" );
    QApplication::setFont( smallTitleFont, TRUE, "QDockWindowTitleBar" );

    if ( qt_std_pal && *qt_std_pal != QApplication::palette() )
	return;

    // Do the color settings

    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    cg.setColor( QColorGroup::Button,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    cg.setColor( QColorGroup::Light,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    cg.setColor( QColorGroup::Midlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DLIGHT))) );
    cg.setColor( QColorGroup::Dark,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNSHADOW))) );
    cg.setColor( QColorGroup::Mid, cg.button().dark( 150 ) );
    cg.setColor( QColorGroup::Text,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))) );
    cg.setColor( QColorGroup::BrightText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))) );
    cg.setColor( QColorGroup::ButtonText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNTEXT))) );
    cg.setColor( QColorGroup::Base,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOW))) );
    cg.setColor( QColorGroup::Background,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))) );
    cg.setColor( QColorGroup::Shadow,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DDKSHADOW))) );
    cg.setColor( QColorGroup::Highlight,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    cg.setColor( QColorGroup::HighlightedText,
		 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

    // ### hardcoded until I find out how to get it from the system settings.
    cg.setColor( QColorGroup::Link, Qt::blue );
    cg.setColor( QColorGroup::LinkVisited, Qt::magenta );

    if ( qt_winver != Qt::WV_NT && qt_winver != Qt::WV_95 ) {
	if ( cg.midlight() == cg.button() )
	    cg.setColor( QColorGroup::Midlight, cg.button().light(110) );
    }

    QColor disabled( (cg.foreground().red()+cg.button().red())/2,
		     (cg.foreground().green()+cg.button().green())/2,
		     (cg.foreground().blue()+cg.button().blue())/2);
    QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
		     disabled, cg.brightText(), cg.base(), cg.background() );
    dcg.setColor( QColorGroup::Highlight,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
    dcg.setColor( QColorGroup::HighlightedText,
		  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

    QColorGroup icg = cg;
    if ( qt_winver != Qt::WV_NT && qt_winver != Qt::WV_95 ) {
	if ( icg.background() != icg.base() ) {
	    icg.setColor( QColorGroup::Highlight, icg.background() );
	    icg.setColor( QColorGroup::HighlightedText, icg.text() );
	}
    }

    QPalette pal( cg, dcg, icg );
    QApplication::setPalette( pal, TRUE );
    *qt_std_pal = pal;

    QColor menu(qt_colorref2qrgb(GetSysColor(COLOR_MENU)));
    QColor menuText(qt_colorref2qrgb(GetSysColor(COLOR_MENUTEXT)));
    {
	// we might need a special color group for the menu.
	cg.setColor( QColorGroup::Button, menu );
	cg.setColor( QColorGroup::Text, menuText );
	cg.setColor( QColorGroup::Foreground, menuText );
	cg.setColor( QColorGroup::ButtonText, menuText );
	QColor disabled( (cg.foreground().red()+cg.button().red())/2,
			 (cg.foreground().green()+cg.button().green())/2,
			 (cg.foreground().blue()+cg.button().blue())/2);
	QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
			 disabled, cg.brightText(), cg.base(), cg.background() );
	if ( qt_winver == Qt::WV_XP ) {
	    dcg.setColor( QColorGroup::Highlight,
			  QColor(qt_colorref2qrgb(GetSysColor(COLOR_MENUHILIGHT))) );
	} else {
	    dcg.setColor( QColorGroup::Highlight,
			  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))) );
	}
	dcg.setColor( QColorGroup::HighlightedText,
		      QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))) );

	icg = cg;
	if ( qt_winver != Qt::WV_NT && qt_winver != Qt::WV_95 )
	    icg.setColor( QColorGroup::ButtonText, icg.dark() );

	QPalette menu(cg, dcg, icg);
	QApplication::setPalette( menu, TRUE, "QPopupMenu");

	if ( qt_winver == Qt::WV_XP ) {
	    QColor menubar(qt_colorref2qrgb(GetSysColor(COLOR_MENUBAR)));
	    cg.setColor( QColorGroup::Button, menubar );
	    dcg.setColor( QColorGroup::Button, menubar );
	    icg = cg;
	    menu = QPalette( cg, dcg, icg );
	}
	QApplication::setPalette( menu, TRUE, "QMenuBar");
    }

    QColor ttip(qt_colorref2qrgb(GetSysColor(COLOR_INFOBK)));
    QColor ttipText(qt_colorref2qrgb(GetSysColor(COLOR_INFOTEXT)));
    {
	cg.setColor( QColorGroup::Button, ttip );
	cg.setColor( QColorGroup::Background, ttip );
	cg.setColor( QColorGroup::Text, ttipText );
	cg.setColor( QColorGroup::Foreground, ttipText );
	cg.setColor( QColorGroup::ButtonText, ttipText );
	QColor disabled( (cg.foreground().red()+cg.button().red())/2,
			 (cg.foreground().green()+cg.button().green())/2,
			 (cg.foreground().blue()+cg.button().blue())/2);
	QColorGroup dcg( disabled, cg.button(), cg.light(), cg.dark(), cg.mid(),
			 disabled, Qt::white, Qt::white, cg.background() );

	QPalette pal(cg, dcg, cg);
	QApplication::setPalette( pal, TRUE, "QTipLabel");
    }
}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

void qt_init( int *argcptr, char **argv, QApplication::Type )
{

#if defined(QT_DEBUG)
    int argc = *argcptr;
    int i, j;

  // Get command line params

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-nograb" )
	    appNoGrab = !appNoGrab;
	else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
#else
    Q_UNUSED( argcptr );
    Q_UNUSED( argv );
#endif // QT_DEBUG


    // Get the application name/instance if qWinMain() was not invoked
    set_winapp_name();
    if ( appInst == 0 ) {
	QT_WA( {
	    appInst = GetModuleHandle( 0 );
	}, {
	    appInst = GetModuleHandleA( 0 );
	} );
    }

#ifndef Q_OS_TEMP
    // Initialize OLE/COM
    //	 S_OK means success and S_FALSE means that it has already
    //	 been initialized
    HRESULT r;
    r = OleInitialize(0);
    if ( r != S_OK && r != S_FALSE ) {
#if defined(QT_CHECK_STATE)
	qWarning( "Qt: Could not initialize OLE (error %x)", r );
#endif
    }
#endif

    // Misc. initialization

    QWindowsMime::initialize();
    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
#endif
    qApp->setName( appName );

    // default font
    {
	HFONT hfont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
	QFont f("MS Sans Serif",8);
	QT_WA( {
	    LOGFONT lf;
	    if ( GetObject( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	} , {
	    LOGFONTA lf;
	    if ( GetObjectA( hfont, sizeof(lf), &lf ) )
		f = qt_LOGFONTtoQFont((LOGFONT&)lf,TRUE);
	} );
	QApplication::setFont( f );
    }

    // QFont::locale_init();  ### Uncomment when it does something on Windows

    if ( !qt_std_pal )
	qt_create_std_palette();
    if ( QApplication::desktopSettingsAware() )
	qt_set_windows_resources();

    QT_WA( {
	WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    } , {
	WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
    } );

    // the WinTab API
#if defined(QT_TABLET_SUPPORT)

#define FIX_DOUBLE(x) ( double(INT(x)) + double(FRAC(x) / 0x10000 ) )
    // get the WinTab Functions
    initWinTabFunctions();
    int max_pressure;
    struct tagAXIS tpOri[3];
    struct tagAXIS pressureAxis;
    double tpvar,
 	   aziFactor = 1,
 	   altFactor = 1,
 	   altAdjust = 1;

    if ( ptrWTInfo ) {
	// make sure we have WinTab
	if ( !ptrWTInfo( 0, 0, NULL ) ) {
#if defined(QT_CHECK_STATE)
	    qWarning( "Wintab services not available" );
#endif
	    return;
	}

	// some tablets don't support tilt, check if its possible,
	tilt_support = ptrWTInfo( WTI_DEVICES, DVC_ORIENTATION, &tpOri );

	if ( tilt_support ) {
	    // check for azimuth and altitude
	    if ( tpOri[0].axResolution && tpOri[1].axResolution ) {
		tpvar = FIX_DOUBLE( tpOri[0].axResolution );
		aziFactor = tpvar/(2 * PI);
		tpvar = FIX_DOUBLE( tpOri[1].axResolution);
		altFactor = tpvar / 1000;
		altAdjust = double(tpOri[1].axMax) / altFactor;
 	    } else {
		tilt_support = FALSE;
 	    }
	}
	max_pressure = ptrWTInfo( WTI_DEVICES, DVC_NPRESSURE, &pressureAxis );
	if ( max_pressure ) {
	    //get the maximum pressure then
	    max_pressure = pressureAxis.axMax;
	} else {
	    max_pressure = 0;
	}
	// build our context from the default context
	ptrWTInfo( WTI_DEFSYSCTX, 0, &lcMine );
	lcMine.lcOptions |= CXO_MESSAGES | CXO_CSRMESSAGES;
	lcMine.lcPktData = PACKETDATA;
	lcMine.lcPktMode = PACKETMODE;
	lcMine.lcMoveMask = PACKETDATA;

	lcMine.lcOutOrgX = 0;
	lcMine.lcOutExtX = GetSystemMetrics( SM_CXSCREEN );
	lcMine.lcOutOrgY = 0;
	// the tablet deals with coordinates as most humans do, make it follow
	// the standard screen idea of coordinates...
	lcMine.lcOutExtY = -GetSystemMetrics( SM_CYSCREEN );
    }
#undef FIX_DOUBLE
#endif

    QInputContext::init();
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
#if defined(USE_HEARTBEAT)
    KillTimer( 0, heartBeat );
#endif
    unregWinClasses();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();
    QSharedDoubleBuffer::cleanup();
#if defined(QT_THREAD_SUPPORT)
    QThread::cleanup();
#endif
    if ( displayDC ) {
	ReleaseDC( 0, displayDC );
	displayDC = 0;
    }

    QInputContext::shutdown();

#ifndef Q_OS_TEMP
  // Deinitialize OLE/COM
    OleUninitialize();
#endif

#if defined(QT_TABLET_SUPPORT)
    if ( ptrWTClose )
	ptrWTClose( hTab );
#endif
    delete activeBeforePopup;
    activeBeforePopup = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

static void msgHandler( QtMsgType t, const char* str )
{
    if ( !str )
	str = "(null)";
    QCString s = str;
    s += "\n";
#ifndef Q_OS_TEMP
    OutputDebugStringA( s.data() );
#else
    OutputDebugString( QString( s ).ucs2() );
#endif
    if ( t == QtFatalMsg )
#ifndef Q_OS_TEMP
	ExitProcess( 1 );
#else
	exit(1);
#endif
}

Q_EXPORT const char *qAppFileName()		// get application file name
{
    return appFileName;
}

Q_EXPORT const char *qAppName()			// get application name
{
    return appName;
}

Q_EXPORT HINSTANCE qWinAppInst()		// get Windows app handle
{
    return appInst;
}

Q_EXPORT HINSTANCE qWinAppPrevInst()		// get Windows prev app handle
{
    return appPrevInst;
}

Q_EXPORT int qWinAppCmdShow()			// get main window show command
{
    return appCmdShow;
}


Q_EXPORT HDC qt_display_dc()			// get display DC
{
    if ( !displayDC )
	displayDC = GetDC( 0 );
    return displayDC;
}

bool qt_nograb()				// application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}


static QAsciiDict<int> *winclassNames = 0;

const QString qt_reg_winclass( int flags )	// register window class
{
    if ( !winclassNames ) {
	winclassNames = new QAsciiDict<int>;
	Q_CHECK_PTR( winclassNames );
    }
    uint style;
    bool icon;
    QString cname;
    if ( flags & Qt::WWinOwnDC ) {
	cname = "QWidgetOwnDC";
#ifndef Q_OS_TEMP
	style = CS_OWNDC | CS_DBLCLKS;
#else
	style = CS_DBLCLKS;
#endif
	icon  = TRUE;
    } else if ( (flags & (Qt::WType_Popup|Qt::WStyle_Tool)) == 0 ) {
	cname = "QWidget";
	style = CS_DBLCLKS;
	icon  = TRUE;
    } else {
	cname = "QPopup";
#ifndef Q_OS_TEMP
	style = CS_DBLCLKS | CS_SAVEBITS;
#else
	style = CS_DBLCLKS;
#endif
	if ( qt_winver == Qt::WV_XP )
	    style |= 0x00020000;		// CS_DROPSHADOW
	icon  = FALSE;
    }

    if ( winclassNames->find(cname.latin1()) )		// already registered
	return cname;

    QT_WA( {
	WNDCLASS wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    wc.hIcon = LoadIcon( appInst, L"IDI_ICON1" );
//#ifndef Q_OS_TEMP
	    if ( !wc.hIcon )
		wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
//#endif
	}
	else
	{
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= (TCHAR*)cname.ucs2();
	RegisterClass( &wc );
    } , {
	WNDCLASSA wc;
	wc.style	= style;
	wc.lpfnWndProc	= (WNDPROC)QtWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= (HINSTANCE)qWinAppInst();
	if ( icon ) {
	    wc.hIcon = LoadIconA( appInst, (char*)"IDI_ICON1" );
	    if ( !wc.hIcon )
		wc.hIcon = LoadIconA( 0, (char*)IDI_APPLICATION );
	}
	else {
	    wc.hIcon = 0;
	}
	wc.hCursor	= 0;
	wc.hbrBackground= 0;
	wc.lpszMenuName	= 0;
	wc.lpszClassName= cname.latin1();
	RegisterClassA( &wc );
    } );

    winclassNames->insert( cname.latin1(), (int*)1 );
    return cname;
}

static void unregWinClasses()
{
    if ( !winclassNames )
	return;
    QAsciiDictIterator<int> it(*winclassNames);
    const char *k;
    while ( (k = it.currentKey()) ) {
	QT_WA( {
	    UnregisterClass( (TCHAR*)QString::fromLatin1(k).ucs2(), (HINSTANCE)qWinAppInst() );
	} , {
	    UnregisterClassA( k, (HINSTANCE)qWinAppInst() );
	} );
	++it;
    }
    delete winclassNames;
    winclassNames = 0;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    main_widget = mainWidget;			// set main widget
#if defined (QT_TABLET_SUPPORT )
    if ( ptrWTOpen && main_widget ) {
	hTab = ptrWTOpen( main_widget->winId(), &lcMine, TRUE );
#if defined (QT_CHECK_STATE)
	if ( hTab == NULL )
	    qWarning( "Failed to open the tablet" );
#endif	
    }
#endif
}

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QPtrList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	Q_CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    Q_CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    SetCursor( app_cursor->handle() );
}

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( app_cursor ) {
	SetCursor( app_cursor->handle() );
    } else {
	delete cursorStack;
	cursorStack = 0;
	QWidget *w = QWidget::find( curWin );
	if ( w )
	    SetCursor( w->cursor().handle() );
	else
	    SetCursor( arrowCursor.handle() );
    }
}

#endif

/*
  Internal function called from QWidget::setCursor()
*/

void qt_set_cursor( QWidget *w, const QCursor& /* c */)
{
    if ( !curWin )
	return;
    QWidget* cW = QWidget::find( curWin );
    if ( !cW || cW->topLevelWidget() != w->topLevelWidget() ||
	 !cW->isVisible() || !cW->hasMouse() || cursorStack )
	return;

    SetCursor( cW->cursor().handle() );
}


void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		}
	    } else {				// switch off
		if ( w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->isVisible() && w->geometry().contains(pos) ) {
		    QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		    return c ? c : w;
		}
	    }
	    --it;
	}
    }
    return 0;
}

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    POINT p;
    HWND  win;
    QWidget *w;
    p.x = x;
    p.y = y;
    win = WindowFromPoint( p );
    if ( !win )
	return 0;

    w = QWidget::find( win );
    while ( !w && win ) {
	win = GetParent( win );
	w = QWidget::find( win );
    }
    if ( !w )
	return 0;
    if ( !child && !w->isTopLevel() )
	w = w->topLevelWidget();
    return w;
}

void QApplication::beep()
{
    MessageBeep( MB_OK );
}

/*****************************************************************************
  Windows-specific drawing used here
 *****************************************************************************/

static void drawTile( HDC hdc, int x, int y, int w, int h,
		      const QPixmap *pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
	drawH = pixmap->height() - yOff;	// Cropping first row
	if ( yPos + drawH > y + h )		// Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while( xPos < x + w ) {
	    drawW = pixmap->width() - xOff;	// Cropping first column
	    if ( xPos + drawW > x + w )		// Cropping last column
		drawW = x + w - xPos;
	    if ( pixmap->isMultiCellPixmap() ) {
		BitBlt( hdc, xPos, yPos,
			drawW, drawH, pixmap->multiCellHandle(),
			xOff, yOff+pixmap->multiCellOffset(), SRCCOPY );
	    } else {
		BitBlt( hdc, xPos, yPos,
			drawW, drawH, pixmap->handle(),
			xOff, yOff, SRCCOPY );
	    }
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}

Q_EXPORT
void qt_fill_tile( QPixmap *tile, const QPixmap &pixmap )
{
    bitBlt( tile, 0, 0, &pixmap, 0, 0, -1, -1, Qt::CopyROP, TRUE );
    int x = pixmap.width();
    while ( x < tile->width() ) {
	bitBlt( tile, x, 0, tile, 0, 0, x, pixmap.height(), Qt::CopyROP, TRUE);
	x *= 2;
    }
    int y = pixmap.height();
    while ( y < tile->height() ) {
	bitBlt( tile, 0, y, tile, 0, 0, tile->width(), y, Qt::CopyROP, TRUE );
	y *= 2;
    }
}


#ifndef Q_OS_TEMP
static inline void qt_draw_tiled_pixmapA( HDC hdc, int x, int y, int w, int h,
			   const QPixmap *bg_pixmap,
			   int off_x, int off_y ) 
{
    QPixmap *tile = 0;
    QPixmap *pm;
    int  sw = bg_pixmap->width(), sh = bg_pixmap->height();
    if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
	int tw = sw, th = sh;
	while ( tw*th < 32678 && tw < w/2 )
	    tw *= 2;
	while ( tw*th < 32678 && th < h/2 )
	    th *= 2;
	tile = new QPixmap( tw, th, bg_pixmap->depth(),
			    QPixmap::NormalOptim );
	qt_fill_tile( tile, *bg_pixmap );
	pm = tile;
    } else {
	pm = (QPixmap*)bg_pixmap;
    }
    drawTile( hdc, x, y, w, h, pm, off_x, off_y );
    if ( tile )
	delete tile;
}
#endif

Q_EXPORT
void qt_draw_tiled_pixmap( HDC hdc, int x, int y, int w, int h,
			   const QPixmap *bg_pixmap,
			   int off_x, int off_y )
{
    QT_WA( {
	// NT has no brush size limitation, so this is straight-forward
	// Note: Since multi cell pixmaps are not used under NT, we can
	// safely access the hbm() parameter of the pixmap.
	HBRUSH brush = CreatePatternBrush( bg_pixmap->hbm() );
	HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
	POINT p;
	SetBrushOrgEx( hdc, -off_x, -off_y, &p );
	PatBlt( hdc, x, y, w, h, PATCOPY );
	SetBrushOrgEx( hdc, p.x, p.y, 0 );
	SelectObject( hdc, oldBrush );
	DeleteObject( brush );
    } , {
	// For Windows 9x, we must do everything ourselves.
	qt_draw_tiled_pixmapA( hdc, x, y, w, h, bg_pixmap, off_x, off_y );
    } );
}



/*****************************************************************************
  Main event loop
 *****************************************************************************/


void QApplication::processEvents( int maxtime )
{
    uint ticks = (uint)GetTickCount();
    while ( !quit_now && processNextEvent(FALSE) ) {
	if ( (uint)GetTickCount() - ticks > (uint)maxtime )
	    break;
    }
}

extern uint qGlobalPostedEventsCount();

/*!
    The message procedure calls this function for every message
    received. Reimplement this function if you want to process window
    messages \e msg that are not processed by Qt.
*/
bool QApplication::winEventFilter( MSG * /*msg*/ )	// Windows event filter
{
    return FALSE;
}

/*!
    If \a gotFocus is TRUE, \a widget will become the active window.
    Otherwise the active window is reset to NULL.
*/
void QApplication::winFocus( QWidget *widget, bool gotFocus )
{
    if ( inPopupMode() ) // some delayed focus event to ignore
	return;
    if ( gotFocus ) {
	setActiveWindow( widget );
	if ( active_window && active_window->testWFlags( WType_Dialog ) ) {
	    // raise the entire application, not just the dialog
	    QWidget* mw = active_window;
	    while( mw->parentWidget() && mw->testWFlags( WType_Dialog) )
		mw = mw->parentWidget()->topLevelWidget();
	    if ( mw != active_window )
		SetWindowPos( mw->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
	}
    } else {
	setActiveWindow( 0 );
    }
}


//
// QtWndProc() receives all messages from the main event loop
//

static bool inLoop = FALSE;
static int inputcharset = CP_ACP;

#define RETURN(x) { inLoop=FALSE;return x; }

bool qt_sendSpontaneousEvent( QObject *receiver, QEvent *event )
{
    return QApplication::sendSpontaneousEvent( receiver, event );
}

extern "C"
LRESULT CALLBACK QtWndProc( HWND hwnd, UINT message, WPARAM wParam,
			    LPARAM lParam )
{
    bool result = TRUE;
    QEvent::Type evt_type = QEvent::None;
    QETWidget *widget;

#if defined (QT_TABLET_SUPPORT)
	// there is no need to process pakcets from tablet unless
	// it is actually on the tablet, a flag to let us know...
//	bool processPackets = FALSE;
	int nPackets;	// the number of packets we get from the queue
#endif

    if ( !qApp )				// unstable app state
	goto do_default;

    if ( inLoop && qApp->loopLevel() )
	qApp->sendPostedEvents( 0, QEvent::ShowWindowRequest );

    inLoop = TRUE;

    MSG msg;
    msg.hwnd = hwnd;				// create MSG structure
    msg.message = message;			// time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WNDPROC
#endif

    if ( qt_winEventFilter(&msg) )		// send through app filter
	RETURN(0);

    switch ( message ) {
#ifndef Q_OS_TEMP
    case WM_QUERYENDSESSION: {
	if ( sm_smActive ) // bogus message from windows
	    RETURN(TRUE);

	sm_smActive = TRUE;
	sm_blockUserInput = TRUE; // prevent user-interaction outside interaction windows
	sm_cancel = FALSE;
	if ( qt_session_manager_self )
	    qApp->commitData( *qt_session_manager_self );
	if ( lParam == (LPARAM)ENDSESSION_LOGOFF ) {
	    _flushall();
	}
	RETURN(!sm_cancel);
    }
    case WM_ENDSESSION: {
	sm_smActive = FALSE;
	sm_blockUserInput = FALSE;
	bool endsession = (bool) wParam;

	if ( endsession ) {
	    qApp->quit();
	}

	RETURN(0);
    }
    case WM_DISPLAYCHANGE:
	if ( qt_desktopWidget ) {
	    QMoveEvent mv( QPoint(GetSystemMetrics( 76 ), GetSystemMetrics( 77 )), qt_desktopWidget->pos() );
	    QApplication::sendEvent( qt_desktopWidget, &mv );
	    QResizeEvent re( QSize(GetSystemMetrics( 78 ), GetSystemMetrics( 79 )), qt_desktopWidget->size() );
	    QApplication::sendEvent( qt_desktopWidget, &re );
	}
	break;
#endif

    case WM_SETTINGCHANGE:
    case WM_SYSCOLORCHANGE:
	if ( QApplication::desktopSettingsAware() )
	    qt_set_windows_resources();
	break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
	if ( ignoreNextMouseReleaseEvent )
	    ignoreNextMouseReleaseEvent = FALSE;
	break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
	if ( ignoreNextMouseReleaseEvent ) {
	    ignoreNextMouseReleaseEvent = FALSE;
	    if ( qt_button_down && qt_button_down->winId() == autoCaptureWnd ) {
		releaseAutoCapture();
		qt_button_down = 0;
	    }


	    RETURN(0);
	}
	break;

    default:
	break;
    }

    widget = (QETWidget*)QWidget::find( hwnd );
    if ( !widget )				// don't know this widget
	goto do_default;

    if ( app_do_modal )	{			// modal event handling
	int ret = 0;
	if ( !qt_try_modal(widget, &msg, ret ) ) {
	    if ( message == WM_MOUSEMOVE && widget->winId() != curWin ) {
		qt_dispatchEnterLeave( widget, QWidget::find( curWin ) );
		curWin = widget->winId();
	    }
	    return ret;
	}
    }

    if ( widget->winEvent(&msg) )		// send through widget filter
	RETURN(0);

    if ( ( message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ||
	    message >= WM_XBUTTONDOWN && message <= WM_XBUTTONDBLCLK )
	    && message != WM_MOUSEWHEEL ) {
	if ( qApp->activePopupWidget() != 0) { // in popup mode
	    POINT curPos;
	    DWORD ol_pos = GetMessagePos();
#ifndef Q_OS_TEMP
	    curPos.x = GET_X_LPARAM(ol_pos);
	    curPos.y = GET_Y_LPARAM(ol_pos);
#else
	    curPos.x = LOWORD(ol_pos);
	    curPos.y = HIWORD(ol_pos);
#endif
	    QWidget* w = QApplication::widgetAt(curPos.x, curPos.y, TRUE );
	    if ( w )
		widget = (QETWidget*)w;
	}
	if ( widget->isEnabled() &&
		(message == WM_LBUTTONDOWN ||
		message == WM_MBUTTONDOWN ||
		message == WM_RBUTTONDOWN ||
		message == WM_XBUTTONDOWN ) ) {
	    QWidget* w = widget;
	    while ( w->focusProxy() )
		w = w->focusProxy();
	    if ( w->focusPolicy() & QWidget::ClickFocus ) {
		QFocusEvent::setReason( QFocusEvent::Mouse);
		w->setFocus();
		QFocusEvent::resetReason();
	    }
	}
#if defined(QT_TABLET_SUPPORT)
	if ( !chokeMouse )
#endif
	    widget->translateMouseEvent( msg );	// mouse event
#if defined(QT_TABLET_SUPPORT)
	else
	    chokeMouse = FALSE;
#endif
    } else if ( message == WM95_MOUSEWHEEL ) {
	result = widget->translateWheelEvent( msg );
    } else {
	switch ( message ) {
	case WM_KEYDOWN:			// keyboard event
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_IME_CHAR:
	case WM_IME_KEYDOWN:
	case WM_CHAR: {
	    QWidget *g = QWidget::keyboardGrabber();
	    if ( g )
		widget = (QETWidget*)g;
	    else if ( qApp->focusWidget() )
		widget = (QETWidget*)qApp->focusWidget();
	    else if ( !widget || widget->winId() == GetFocus() ) // We faked the message to go to exactly that widget.
		widget = (QETWidget*)widget->topLevelWidget();
	    if ( widget->isEnabled() )
		result = widget->translateKeyEvent( msg, g != 0 );
	    break;
	}
	case WM_SYSCHAR:
	    result = TRUE;			// consume event
	    break;

	case WM_MOUSEWHEEL:
	    result = widget->translateWheelEvent( msg );
	    break;

	case WM_APPCOMMAND:
	    {
		uint cmd = GET_APPCOMMAND_LPARAM(lParam);
		uint uDevice = GET_DEVICE_LPARAM(lParam);
		uint dwKeys = GET_KEYSTATE_LPARAM(lParam);

		int state = translateButtonState( dwKeys, QEvent::KeyPress, 0 );

		switch ( uDevice ) {
		case FAPPCOMMAND_KEY:
		    {
			int key = 0;

			switch( cmd ) {
			case APPCOMMAND_BASS_BOOST:
			    key = Qt::Key_BassBoost;
			    break;
			case APPCOMMAND_BASS_DOWN:
			    key = Qt::Key_BassDown;
			    break;
			case APPCOMMAND_BASS_UP:
			    key = Qt::Key_BassUp;
			    break;
			case APPCOMMAND_BROWSER_BACKWARD:
			    key = Qt::Key_Back;
			    break;
			case APPCOMMAND_BROWSER_FAVORITES:
			    key = Qt::Key_Favorites;
			    break;
			case APPCOMMAND_BROWSER_FORWARD:
			    key = Qt::Key_Forward;
			    break;
			case APPCOMMAND_BROWSER_HOME:
			    key = Qt::Key_HomePage;
			    break;
			case APPCOMMAND_BROWSER_REFRESH:
			    key = Qt::Key_Refresh;
			    break;
			case APPCOMMAND_BROWSER_SEARCH:
			    key = Qt::Key_Search;
			    break;
			case APPCOMMAND_BROWSER_STOP:
			    key = Qt::Key_Stop;
			    break;
			case APPCOMMAND_LAUNCH_APP1:
			    key = Qt::Key_Launch0;
			    break;
			case APPCOMMAND_LAUNCH_APP2:
			    key = Qt::Key_Launch1;
			    break;
			case APPCOMMAND_LAUNCH_MAIL:
			    key = Qt::Key_LaunchMail;
			    break;
			case APPCOMMAND_LAUNCH_MEDIA_SELECT:
			    key = Qt::Key_LaunchMedia;
			    break;
			case APPCOMMAND_MEDIA_NEXTTRACK:
			    key = Qt::Key_MediaNext;
			    break;
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			    key = Qt::Key_MediaPlay;
			    break;
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			    key = Qt::Key_MediaPrev;
			    break;
			case APPCOMMAND_MEDIA_STOP:
			    key = Qt::Key_MediaStop;
			    break;
			case APPCOMMAND_TREBLE_DOWN:
			    key = Qt::Key_TrebleDown;
			    break;
			case APPCOMMAND_TREBLE_UP:
			    key = Qt::Key_TrebleUp;
			    break;
			case APPCOMMAND_VOLUME_DOWN:
			    key = Qt::Key_VolumeDown;
			    break;
			case APPCOMMAND_VOLUME_MUTE:
			    key = Qt::Key_VolumeMute;
			    break;
			case APPCOMMAND_VOLUME_UP:
			    key = Qt::Key_VolumeUp;
			    break;
			default:
			    break;
			}
			if ( key ) {
			    bool res = FALSE;
			    QWidget *g = QWidget::keyboardGrabber();
			    if ( g )
				widget = (QETWidget*)g;
			    else if ( qApp->focusWidget() )
				widget = (QETWidget*)qApp->focusWidget();
			    else
				widget = (QETWidget*)widget->topLevelWidget();
			    if ( widget->isEnabled() )
				res = ((QETWidget*)widget)->sendKeyEvent( QEvent::KeyPress, key, 0, state, FALSE, QString::null, g != 0 );
			    if ( res )
				return TRUE;
			}
		    }
		    break;

		default:
		    break;
		}

		result = FALSE;
	    }
	    break;

#ifndef Q_OS_TEMP
	case WM_NCMOUSEMOVE:
	    {
		// span the application wide cursor over the
		// non-client area.
		QCursor *c = qt_grab_cursor();
		if ( !c )
		    c = QApplication::overrideCursor();
		if ( c )	// application cursor defined
		    SetCursor( c->handle() );
		else
		    result = FALSE;
		// generate leave event also when the caret enters
		// the non-client area.
		qt_dispatchEnterLeave( 0, QWidget::find(curWin) );
		curWin = 0;
	    }
	    break;
#endif

	case WM_SYSCOMMAND:
#ifndef Q_OS_TEMP
	    switch( wParam ) {
	    case SC_CONTEXTHELP:
		QWhatsThis::enterWhatsThisMode();
		QT_WA( {
		    DefWindowProc( hwnd, WM_NCPAINT, 1, 0 );
		} , {
		    DefWindowProcA( hwnd, WM_NCPAINT, 1, 0 );
		} );
		break;
#if defined(QT_NON_COMMERCIAL)
		QT_NC_SYSCOMMAND
#endif
	    case SC_MAXIMIZE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
		result = FALSE;
		break;
	    case SC_MINIMIZE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMinimized ) );
		result = FALSE;
		break;
	    case SC_RESTORE:
		QApplication::postEvent( widget, new QEvent( QEvent::ShowNormal ) );
		result = FALSE;
		break;
	    default:
		result = FALSE;
		break;
	    }
#endif
	    break;

	case WM_SETTINGCHANGE:
	    if ( !msg.wParam ) {
		QString area = QT_WA_INLINE( QString::fromUcs2( (unsigned short *)msg.lParam ),  
					     QString::fromLocal8Bit( (char*)msg.lParam ) );
		if ( area == "intl" )
		    QApplication::postEvent( widget, new QEvent( QEvent::LocaleChange ) );
	    }
	    break;

#ifndef Q_OS_TEMP
	case WM_NCLBUTTONDBLCLK:
	    if ( wParam == HTCAPTION ) {
		if ( widget->isMaximized() )
		    QApplication::postEvent( widget, new QEvent( QEvent::ShowNormal ) );
		else
		    QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
	    }
	    result = FALSE;
	    break;
#endif
	    case WM_PAINT:				// paint event
	    result = widget->translatePaintEvent( msg );
	    break;

	case WM_ERASEBKGND:			// erase window background
	    {
		RECT r;
		QPoint offset = widget->backgroundOffset();
		int ox = offset.x();
		int oy = offset.y();
		GetClientRect( hwnd, &r );
		qt_erase_background
		    ( (HDC)wParam, r.left, r.top,
			r.right-r.left, r.bottom-r.top,
			widget->backgroundColor(),
			widget->backgroundPixmap(), ox, oy );
		RETURN(TRUE);
	    }
	    break;

	case WM_MOVE:				// move window
	case WM_SIZE:				// resize window
	    result = widget->translateConfigEvent( msg );
	    if ( widget->isMaximized() )
		QApplication::postEvent( widget, new QEvent( QEvent::ShowMaximized ) );
	    break;

	case WM_ACTIVATE:
#if defined (QT_TABLET_SUPPORT)
	    if ( ptrWTOverlap && ptrWTEnable ) {
		// cooperate with other tablet applications, but when
		// we get focus, I want to use the tablet...
		if (hTab && GET_WM_ACTIVATE_STATE(wParam, lParam)) {
		    if ( ptrWTEnable(hTab, TRUE) )
			ptrWTOverlap(hTab, TRUE);		    
		}
	    }
#endif
	    if ( QApplication::activePopupWidget() && LOWORD(wParam) == WA_INACTIVE &&
		QWidget::find((HWND)lParam) == 0 ) {
		// Another application was activated while our popups are open,
		// then close all popups.  In case some popup refuses to close,
		// we give up after 1024 attempts (to avoid an infinite loop).
		int maxiter = 1024;
		QWidget *popup;
		while ( (popup=QApplication::activePopupWidget()) && maxiter-- )
		    popup->close();
	    }
	    qApp->winFocus( widget, LOWORD(wParam) == WA_INACTIVE ? 0 : 1 );
	    break;

#ifndef Q_OS_TEMP
	case WM_MOUSEACTIVATE:
	    {
		const QWidget *tlw = widget->topLevelWidget();
		// Do not change activation if the clicked widget is inside a floating dock window
		if ( tlw->inherits( "QDockWindow" ) ) {
		    if ( tlw->focusWidget() )
			RETURN(MA_ACTIVATE);
		    SetForegroundWindow( tlw->winId() );
		    if ( tlw->parentWidget() && !tlw->parentWidget()->isActiveWindow() )
			tlw->parentWidget()->setActiveWindow();
		    RETURN(MA_NOACTIVATE);
		}
	    }
	    result = FALSE;
	    break;
#endif

	case WM_PALETTECHANGED:			// our window changed palette
	    if ( QColor::hPal() && (WId)wParam == widget->winId() )
		RETURN(0);			// otherwise: FALL THROUGH!
	    // FALL THROUGH
	case WM_QUERYNEWPALETTE:		// realize own palette
	    if ( QColor::hPal() ) {
		HDC hdc = GetDC( widget->winId() );
		HPALETTE hpalOld = SelectPalette( hdc, QColor::hPal(), FALSE );
		uint n = RealizePalette( hdc );
		if ( n )
		    InvalidateRect( widget->winId(), 0, TRUE );
		SelectPalette( hdc, hpalOld, TRUE );
		RealizePalette( hdc );
		ReleaseDC( widget->winId(), hdc );
		RETURN(n);
	    }
	    break;

	case WM_CLOSE:				// close window
	    widget->translateCloseEvent( msg );
	    RETURN(0);				// always handled

	case WM_DESTROY:			// destroy window
	    if ( hwnd == curWin ) {
		QEvent leave( QEvent::Leave );
		QApplication::sendEvent( widget, &leave );
		curWin = 0;
	    }
	    if ( widget == popupButtonFocus )
		popupButtonFocus = 0;
	    result = FALSE;
	    break;

#ifndef Q_OS_TEMP
	case WM_GETMINMAXINFO:
	    if ( widget->xtra() ) {
		MINMAXINFO *mmi = (MINMAXINFO *)lParam;
		QWExtra	   *x = widget->xtra();
		QRect	   f  = widget->frameGeometry();
		QSize	   s  = widget->size();
		if ( x->minw > 0 )
		    mmi->ptMinTrackSize.x = x->minw + f.width()	 - s.width();
		if ( x->minh > 0 )
		    mmi->ptMinTrackSize.y = x->minh + f.height() - s.height();
		if ( x->maxw < QWIDGETSIZE_MAX )
		    mmi->ptMaxTrackSize.x = x->maxw + f.width()	 - s.width();
		if ( x->maxh < QWIDGETSIZE_MAX )
		    mmi->ptMaxTrackSize.y = x->maxh + f.height() - s.height();
		RETURN(0);
	    }
	    break;

	    case WM_CONTEXTMENU:
	    {
		// it's not VK_APPS or Shift+F10, but a click in the NC area
		if ( lParam != (int)0xffffffff ) {
		    result = FALSE;
		    break;
		}
		QWidget *fw = qApp->focusWidget();
		if ( fw ) {
		    QContextMenuEvent e( QContextMenuEvent::Keyboard, QPoint( 5, 5 ), fw->mapToGlobal( QPoint( 5, 5 ) ), 0 );
		    result = qt_sendSpontaneousEvent( fw, &e );
		}
	    }
	    break;
#endif

	case WM_IME_STARTCOMPOSITION:
    	    result = QInputContext::startComposition();
	    break;
	case WM_IME_ENDCOMPOSITION:
	    result = QInputContext::endComposition();
	    break;
	case WM_IME_COMPOSITION:
	    result = QInputContext::composition( lParam );
	    break;

#ifndef Q_OS_TEMP
	case WM_CHANGECBCHAIN:
	case WM_DRAWCLIPBOARD:
	case WM_RENDERFORMAT:
	case WM_RENDERALLFORMATS:
	    if ( qt_clipboard ) {
		QCustomEvent e( QEvent::Clipboard, &msg );
		qt_sendSpontaneousEvent( qt_clipboard, &e );
		RETURN(0);
	    }
	    result = FALSE;
	    break;
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
	case WM_GETOBJECT:
	    {
		// Ignoring all requests while starting up
		if ( qApp->startingUp() || !qApp->loopLevel() || lParam != OBJID_CLIENT ) {
		    result = FALSE;
		    break;
		}

		typedef LRESULT (WINAPI *PtrLresultFromObject)(REFIID, WPARAM, LPUNKNOWN );
		static PtrLresultFromObject ptrLresultFromObject = 0;
		static bool oleaccChecked = FALSE;

		if ( !oleaccChecked ) {
		    oleaccChecked = TRUE;
		    ptrLresultFromObject = (PtrLresultFromObject)QLibrary::resolve( "oleacc.dll", "LresultFromObject" );
		}
		if ( ptrLresultFromObject ) {
		    QAccessibleInterface *acc = 0;
		    QAccessible::queryAccessibleInterface( widget, &acc );
		    if ( !acc ) {
			result = FALSE;
			break;
		    }

		    QCustomEvent e( QEvent::Accessibility, acc );
		    QApplication::sendEvent( widget, &e );

		    // and get an instance of the IAccessibile implementation
		    IAccessible *iface = qt_createWindowsAccessible( acc );
		    acc->release();
		    LRESULT res = ptrLresultFromObject( IID_IAccessible, wParam, iface );  // ref == 2
		    iface->Release(); // the client will release the object again, and then it will destroy itself

		    if ( res > 0 )
			RETURN(res);
		}
	    }
	    result = FALSE;
	    break;
#endif
#if defined (QT_TABLET_SUPPORT)
	case WT_PACKET:
	    // Get the packets and also don't link against the actual library...
	    if ( ptrWTPacketsGet ) {
		if ( (nPackets = ptrWTPacketsGet( hTab, NPACKETQSIZE, &localPacketBuf)) ) {
		    result = widget->translateTabletEvent( msg, localPacketBuf, nPackets );
		}
	    }
	    break;
	case WT_PROXIMITY:		
	    // flush the QUEUE
	    if ( ptrWTPacketsGet )
		ptrWTPacketsGet( hTab, NPACKETQSIZE + 1, NULL);
	    break;
#endif
	case WM_KILLFOCUS:
	    if ( !QWidget::find( (HWND)wParam ) ) { // we don't get focus, so unset it now
		if ( ::IsChild( widget->winId(), ::GetFocus() ) )
		    result = FALSE;
		else {
		    widget->clearFocus();
		    result = TRUE;
		}
	    } else {
		result = FALSE;
	    }
	    break;

	case WM_THEMECHANGED:
	    if ( widget->testWFlags( Qt::WType_Desktop ) || !qApp || qApp->closingDown() )
		break;

	    qApp->style().unPolish( qApp );

	    if ( widget->testWState(Qt::WState_Polished) )
		qApp->style().unPolish(widget);

	    qApp->style().polish( qApp );

	    if ( widget->testWState(Qt::WState_Polished) )
		qApp->style().polish(widget);
	    widget->repolishStyle( qApp->style() );
	    if ( widget->isVisible() )
		widget->update();
	    break;

	case WM_INPUTLANGCHANGE: {
	    char info[7];
	    if ( !GetLocaleInfoA( MAKELCID(lParam, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, info, 6 ) ) {
		inputcharset = CP_ACP;
	    } else {
		inputcharset = QString( info ).toInt();
	    }
	    break;
	}
	default:
	    result = FALSE;			// event was not processed
	    break;
	}
    }

    if ( evt_type != QEvent::None ) {		// simple event
	QEvent e( evt_type );
	result = qt_sendSpontaneousEvent(widget, &e);
    }
    if ( result )
	RETURN(FALSE);

do_default:
    RETURN( QInputContext::DefWindowProc(hwnd,message,wParam,lParam) )
}


/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

Q_EXPORT bool qt_modal_state()
{
    return app_do_modal;
}


Q_EXPORT void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    releaseAutoCapture();
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    qt_dispatchEnterLeave( 0, QWidget::find( (WId)curWin  ) ); // send synthetic leave event
    curWin = 0;
    qt_button_down = 0;
    ignoreNextMouseReleaseEvent = FALSE;
}


Q_EXPORT void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	    QPoint p( QCursor::pos() );
	    app_do_modal = FALSE; // necessary, we may get recursively into qt_try_modal below
	    QWidget* w = QApplication::widgetAt( p.x(), p.y(), TRUE );
	    qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
	    curWin = w? w->winId() : 0;
	}
	ignoreNextMouseReleaseEvent = TRUE;
    }
    app_do_modal = qt_modal_stack != 0;

}

static bool qt_blocked_modal( QWidget *widget )
{
    if ( !app_do_modal )
	return FALSE;
    if ( qApp->activePopupWidget() )
	return FALSE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return FALSE;

    QWidget *modal=0, *top=qt_modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )				// don't block event
	return FALSE;
    return TRUE;
}

static bool qt_try_modal( QWidget *widget, MSG *msg, int& ret )
{
    if ( qApp->activePopupWidget() )
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();
    int	 type  = msg->message;

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;

    if ( !top || modal == top )			// don't block event
	return TRUE;

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while ( modal && unrelated ) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }

    bool block_event = FALSE;
#ifndef Q_OS_TEMP
    if ( type == WM_NCHITTEST ) {
      //block_event = TRUE;
	// QApplication::beep();
    } else
#endif
	if ( (type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
	 (type == WM_MOUSEWHEEL || type == (int)WM95_MOUSEWHEEL) ||
	 (type >= WM_KEYFIRST	&& type <= WM_KEYLAST)
#ifndef Q_OS_TEMP
			|| type == WM_NCMOUSEMOVE
#endif
		) {
      if ( type == WM_MOUSEMOVE
#ifndef Q_OS_TEMP
			|| type == WM_NCMOUSEMOVE
#endif
			) {
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else
	    SetCursor( Qt::arrowCursor.handle() );
      }
      block_event = TRUE;
    }
#ifndef Q_OS_TEMP
    else if ( type == WM_MOUSEACTIVATE || type == WM_NCLBUTTONDOWN ){
	if ( !top->isActiveWindow() ) {
	    top->setActiveWindow();
	} else {
	    QApplication::beep();
	}
	block_event = TRUE;
	ret = MA_NOACTIVATEANDEAT;
    }
#endif

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  openPopup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  closePopup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/

void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	Q_CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = focus_widget ? focus_widget : active_window;
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() )
	setAutoCapture( popup->winId() );	// grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    if ( popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    POINT curPos;
    GetCursorPos( &curPos );
    replayPopupMouseEvent = !popup->geometry().contains( QPoint(curPos.x, curPos.y) );
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() )			// grabbing not disabled
	    releaseAutoCapture();
	// windows does not have
	// A reasonable focus handling for our popups => we have
	// to restore the focus manually.
	if ( *activeBeforePopup ) {
	    active_window = (*activeBeforePopup)->topLevelWidget();
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    (*activeBeforePopup)->setFocus();
	    QFocusEvent::resetReason();
	} else {
	    active_window = 0;
	}
    } else {
	// Popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	QFocusEvent::setReason( QFocusEvent::Popup );
	QWidget* aw = popupWidgets->getLast();
	if ( popupWidgets->count() == 1 )
	    setAutoCapture( aw->winId() );
	if (aw->focusWidget())
	    aw->focusWidget()->setFocus();
	else
	    aw->setFocus();
	QFocusEvent::resetReason();
    }
}




/*****************************************************************************
  Event translation; translates Windows events to Qt events
 *****************************************************************************/

//
// Auto-capturing for mouse press and mouse release
//

static void setAutoCapture( HWND h )
{
    if ( autoCaptureWnd )
	releaseAutoCapture();
    autoCaptureWnd = h;
    SetCapture( h );
}

static void releaseAutoCapture()
{
    if ( autoCaptureWnd ) {
	ReleaseCapture();
	autoCaptureWnd = 0;
    }
}


//
// Mouse event translation
//
// Non-client mouse messages are not translated
//

static ushort mouseTbl[] = {
    WM_MOUSEMOVE,	QEvent::MouseMove,		0,
    WM_LBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::LeftButton,
    WM_LBUTTONUP,	QEvent::MouseButtonRelease,	Qt::LeftButton,
    WM_LBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::LeftButton,
    WM_RBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::RightButton,
    WM_RBUTTONUP,	QEvent::MouseButtonRelease,	Qt::RightButton,
    WM_RBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::RightButton,
    WM_MBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton,
    WM_MBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton,
    WM_MBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton,
    WM_XBUTTONDOWN,	QEvent::MouseButtonPress,	Qt::MidButton*2, //### Qt::XButton1/2
    WM_XBUTTONUP,	QEvent::MouseButtonRelease,	Qt::MidButton*2,
    WM_XBUTTONDBLCLK,	QEvent::MouseButtonDblClick,	Qt::MidButton*2,
    0,			0,				0
};

static int translateButtonState( int s, int type, int button )
{
    int bst = 0;
    if ( s & MK_LBUTTON )
	bst |= Qt::LeftButton;
    if ( s & MK_MBUTTON )
	bst |= Qt::MidButton;
    if ( s & MK_RBUTTON )
	bst |= Qt::RightButton;
    if ( s & MK_SHIFT )
	bst |= Qt::ShiftButton;
    if ( s & MK_CONTROL )
	bst |= Qt::ControlButton;

    if ( s & MK_XBUTTON1 )
	bst |= Qt::MidButton*2;//### Qt::XButton1;
    if ( s & MK_XBUTTON2 )
	bst |= Qt::MidButton*4;//### Qt::XButton2;

    if ( GetKeyState(VK_MENU) < 0 )
	bst |= Qt::AltButton;

    // Translate from Windows-style "state after event"
    // to X-style "state before event"
    if ( type == QEvent::MouseButtonPress ||
	 type == QEvent::MouseButtonDblClick )
	bst &= ~button;
    else if ( type == QEvent::MouseButtonRelease )
	bst |= button;

    return bst;
}

// In DnD, the mouse release event never appears, so the
// mouse button state machine must be manually reset
/*! \internal */
void QApplication::winMouseButtonUp()
{
    qt_button_down = 0;
    releaseAutoCapture();
}

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;				// event parameters
    int	   button;
    int	   state;
    int	   i;

    if ( sm_blockUserInput ) //block user interaction during session management
	return TRUE;

    if ( msg.message == WM_MOUSEMOVE ) {	
	// Process only the most current mouse move event. Otherwise you
	// end up queueing events in certain situations. Very bad for graphics
	// applications. Just grabbing the million poly model and moving the
	// mouse an inch queues redraws for 30+ seconds.
	//
	// (Patch submitted by: Steve Williams)

	bool keepLooking = true;
	bool messageFound = false;
	MSG mouseMsg1;
	MSG mouseMsg2;

	while ( winPeekMessage( &mouseMsg1, msg.hwnd, WM_MOUSEFIRST, WM_MOUSELAST,
			       PM_NOREMOVE) && keepLooking )
	{
	    // Check to make sure that the message is a mouse move message
	    if ( mouseMsg1.message == WM_MOUSEMOVE ) {
		 mouseMsg2 = mouseMsg1;
		 messageFound = true;

		// Remove the mouse move message
		winPeekMessage( &mouseMsg1, msg.hwnd, WM_MOUSEMOVE, WM_MOUSEMOVE,
			       PM_REMOVE );
	    } else
		keepLooking = false;
	}

	if ( messageFound ) {
	    MSG *msgPtr = (MSG *)(&msg);

	    // Update the passed in MSG structure with the
	    // most current one.
	    msgPtr->lParam = mouseMsg2.lParam;
	    msgPtr->wParam = mouseMsg2.wParam;
	}
    }

    for ( i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3 )
	;
    if ( !mouseTbl[i] )
	return FALSE;
    type   = (QEvent::Type)mouseTbl[++i];	// event type
    button = mouseTbl[++i];			// which button
    if ( button > Qt::MidButton ) {
	switch( GET_XBUTTON_WPARAM( msg.wParam ) ) {
	case XBUTTON1:
	    button = Qt::MidButton*2; //### XButton1;
	    break;
	case XBUTTON2:
	    button = Qt::MidButton*4; //### XButton2;
	    break;
	}
    }
    state  = translateButtonState( msg.wParam, type, button ); // button state
    if ( type == QEvent::MouseMove ) {
	if ( !(state & MouseButtonMask) )
	    qt_button_down = 0;
	QCursor *c = qt_grab_cursor();
	if ( !c )
	    c = QApplication::overrideCursor();
	if ( c )				// application cursor defined
	    SetCursor( c->handle() );
	else if ( isEnabled() )		// use widget cursor if widget is enabled
	    SetCursor( cursor().handle() );
	else {
	    QWidget *parent = parentWidget();
	    while ( parent && !parent->isEnabled() )
		parent = parent->parentWidget();
	    if ( parent )
		SetCursor( parent->cursor().handle() );
	}
	if ( curWin != winId() ) {		// new current window
	    qt_dispatchEnterLeave( this, QWidget::find(curWin) );
	    curWin = winId();
	}

	POINT curPos;
	DWORD ol_pos = GetMessagePos();
#ifndef Q_OS_TEMP
	curPos.x = GET_X_LPARAM(ol_pos);
	curPos.y = GET_Y_LPARAM(ol_pos);
#else
	curPos.x = LOWORD(ol_pos);
	curPos.y = HIWORD(ol_pos);
#endif

	if ( curPos.x == gpos.x && curPos.y == gpos.y )
	    return TRUE;			// same global position
	gpos = curPos;

	if ( state == 0 && autoCaptureWnd == 0 && !hasMouseTracking() &&
	     !QApplication::hasGlobalMouseTracking() )
	    return TRUE;			// no button

	ScreenToClient( winId(), &curPos );

	pos.rx() = (short)curPos.x;
	pos.ry() = (short)curPos.y;
    } else {
	DWORD ol_pos = GetMessagePos();
#ifndef Q_OS_TEMP
	gpos.x = GET_X_LPARAM(ol_pos);
	gpos.y = GET_Y_LPARAM(ol_pos);
#else
	gpos.x = LOWORD(ol_pos);
	gpos.y = HIWORD(ol_pos);
#endif

	pos = mapFromGlobal( QPoint(gpos.x, gpos.y) );

	if ( type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick ) {	// mouse button pressed
	    // Magic for masked widgets
	    qt_button_down = findChildWidget( this, pos );
	    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
		qt_button_down = this;
	}
    }

    if ( qApp->inPopupMode() ) {			// in popup mode
	QWidget* activePopupWidget = qApp->activePopupWidget();
	QWidget *popup = activePopupWidget;

	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( QPoint(gpos.x, gpos.y) );
	}
	QWidget *popupChild = findChildWidget( popup, pos );
	bool releaseAfter = FALSE;
	switch ( type ) {
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonDblClick:
		popupButtonFocus = popupChild;
		break;
	    case QEvent::MouseButtonRelease:
		releaseAfter = TRUE;
		break;
	    default:
		break;				// nothing for mouse move
	}

	if ( popupButtonFocus ) {
	    QMouseEvent e( type,
		popupButtonFocus->mapFromGlobal(QPoint(gpos.x,gpos.y)),
		QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendSpontaneousEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
	    }
	} else if ( popupChild ){
	    QMouseEvent e( type,
		popupChild->mapFromGlobal(QPoint(gpos.x,gpos.y)),
		QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendSpontaneousEvent( popupChild, &e );
	} else {
	    QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	    QApplication::sendSpontaneousEvent( popupChild ? popupChild : popup, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( type == QEvent::MouseButtonPress
	     && qApp->activePopupWidget() != activePopupWidget
	     && replayPopupMouseEvent ) {
	    // the popup dissappeared. Replay the event
	    QWidget* w = QApplication::widgetAt( gpos.x, gpos.y, TRUE );
	    if (w && !qt_blocked_modal( w ) ) {
		if ( w->isEnabled() ) {
		    QWidget* tw = w;
		    while ( tw->focusProxy() )
			tw = tw->focusProxy();
		    if ( tw->focusPolicy() & QWidget::ClickFocus ) {
			QFocusEvent::setReason( QFocusEvent::Mouse);
			w->setFocus();
			QFocusEvent::resetReason();
		    }
		}
		if ( QWidget::mouseGrabber() == 0 )
		    setAutoCapture( w->winId() );
		winPostMessage( w->winId(), msg.message, msg.wParam, msg.lParam );
	    }
	}
    } else {					// not popup mode
	int bs = state & MouseButtonMask;
	if ( (type == QEvent::MouseButtonPress ||
	      type == QEvent::MouseButtonDblClick) && bs == 0 ) {
	    if ( QWidget::mouseGrabber() == 0 )
		setAutoCapture( winId() );
	} else if ( type == QEvent::MouseButtonRelease && bs == button ) {
	    if ( QWidget::mouseGrabber() == 0 )
		releaseAutoCapture();
	}

	QWidget *widget = this;
	QWidget *w = QWidget::mouseGrabber();
	if ( !w )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = w->mapFromGlobal(QPoint(gpos.x, gpos.y));
	}

	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( MouseButtonMask )) == 0 ) {
	    qt_button_down = 0;
	}

	QMouseEvent e( type, pos, QPoint(gpos.x,gpos.y), button, state );
	QApplication::sendSpontaneousEvent( widget, &e );
	if ( type == QEvent::MouseButtonRelease && button == RightButton ) {
	    QContextMenuEvent e2( QContextMenuEvent::Mouse, pos, QPoint(gpos.x,gpos.y), state );
	    QApplication::sendSpontaneousEvent( widget, &e2 );
	}

	if ( type != QEvent::MouseMove )
	    pos.rx() = pos.ry() = -9999;	// init for move compression
    }
    return TRUE;
}


//
// Keyboard event translation
//

static const ushort KeyTbl[] = {		// keyboard mapping table
    VK_ESCAPE,		Qt::Key_Escape,		// misc keys
    VK_TAB,		Qt::Key_Tab,
    VK_BACK,		Qt::Key_Backspace,
    VK_RETURN,		Qt::Key_Return,
    VK_INSERT,		Qt::Key_Insert,
    VK_DELETE,		Qt::Key_Delete,
    VK_CLEAR,		Qt::Key_Clear,
    VK_PAUSE,		Qt::Key_Pause,
    VK_SNAPSHOT,	Qt::Key_Print,
    VK_HOME,		Qt::Key_Home,		// cursor movement
    VK_END,		Qt::Key_End,
    VK_LEFT,		Qt::Key_Left,
    VK_UP,		Qt::Key_Up,
    VK_RIGHT,		Qt::Key_Right,
    VK_DOWN,		Qt::Key_Down,
    VK_PRIOR,		Qt::Key_Prior,
    VK_NEXT,		Qt::Key_Next,
    VK_SHIFT,		Qt::Key_Shift,		// modifiers
    VK_CONTROL,		Qt::Key_Control,
//  VK_????,		Qt::Key_Meta,
//  VK_????,		Qt::Key_Meta,
    VK_MENU,		Qt::Key_Alt,
    VK_CAPITAL,		Qt::Key_CapsLock,
    VK_NUMLOCK,		Qt::Key_NumLock,
    VK_SCROLL,		Qt::Key_ScrollLock,
    VK_NUMPAD0,		Qt::Key_0,			// numeric keypad
    VK_NUMPAD1,		Qt::Key_1,
    VK_NUMPAD2,		Qt::Key_2,
    VK_NUMPAD3,		Qt::Key_3,
    VK_NUMPAD4,		Qt::Key_4,
    VK_NUMPAD5,		Qt::Key_5,
    VK_NUMPAD6,		Qt::Key_6,
    VK_NUMPAD7,		Qt::Key_7,
    VK_NUMPAD8,		Qt::Key_8,
    VK_NUMPAD9,		Qt::Key_9,
    VK_MULTIPLY,	Qt::Key_Asterisk,
    VK_ADD,		Qt::Key_Plus,
    VK_SEPARATOR,	Qt::Key_Comma,
    VK_SUBTRACT,	Qt::Key_Minus,
    VK_DECIMAL,		Qt::Key_Period,
    VK_DIVIDE,		Qt::Key_Slash,
    VK_APPS,		Qt::Key_Menu,
    0,			0
};

static int translateKeyCode( int key )		// get Qt::Key_... code
{
    int code;
    if ( (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') ) {
	code = 0;
    } else if ( key >= VK_F1 && key <= VK_F24 ) {
	code = Qt::Key_F1 + (key - VK_F1);		// function keys
    } else {
	int i = 0;				// any other keys
	code = 0;
	while ( KeyTbl[i] ) {
	    if ( key == (int)KeyTbl[i] ) {
		code = KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
    }
    return code;
}

struct KeyRec {
    KeyRec(int c, int a, const QString& t) : code(c), ascii(a), text(t) { }
    KeyRec() { }
    int code, ascii;
    QString text;
};

static const int maxrecs=64; // User has LOTS of fingers...
static KeyRec key_rec[maxrecs];
static int nrecs=0;

static KeyRec* find_key_rec( int code, bool remove )
{
    KeyRec *result = 0;
    for (int i=0; i<nrecs; i++) {
	if (key_rec[i].code == code) {
	    if (remove) {
		static KeyRec tmp;
		tmp = key_rec[i];
		while (i+1 < nrecs) {
		    key_rec[i] = key_rec[i+1];
		    i++;
		}
		nrecs--;
		result = &tmp;
	    } else {
		result = &key_rec[i];
	    }
	    break;
	}
    }
    return result;
}

static void store_key_rec( int code, int ascii, const QString& text )
{
    if ( nrecs == maxrecs ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "Qt: Internal keyboard buffer overflow" );
#endif
	return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,text);
}

static int asciiToKeycode(char a, int state)
{
    if ( a >= 'a' && a <= 'z' )
	a = toupper( a );
    if ( (state & Qt::ControlButton) != 0 ) {
	if ( a >= 1 && a <= 26 )	// Ctrl+'A'..'Z'
	    a += 'A' - 1;
    }
    return a;
}

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA( {
	return QChar( (ushort)c );
    } , {
	char mb[2];
	mb[0] = c&0xff;
	mb[1] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( inputcharset, MB_PRECOMPOSED, mb, -1, wc, 1);
	return QChar(wc[0]);
    } );
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA( {
	return QChar( (ushort)c );
    } , {
	char mb[3];
	mb[0] = (c>>8)&0xff;
	mb[1] = c&0xff;
	mb[2] = 0;
	WCHAR wc[1];
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
	    mb, -1, wc, 1);
	return QChar(wc[0]);
    } );
}

bool QETWidget::translateKeyEvent( const MSG &msg, bool grab )
{
    bool k0=FALSE, k1=FALSE;
    int  state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= Qt::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= Qt::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= Qt::AltButton;
    //TODO: if it is a pure shift/ctrl/alt keydown, invert state logic, like X

    if ( msg.message == WM_CHAR ) {
	// a multi-character key not found by our look-ahead
	QString s;
	QChar ch = wmchar_to_unicode(msg.wParam);
	if (!ch.isNull())
	    s += ch;
	k0 = sendKeyEvent( QEvent::KeyPress, 0, msg.wParam, state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, msg.wParam, state, grab, s );
    }
    else if ( msg.message == WM_IME_CHAR ) {
	// input method characters not found by our look-ahead
	QString s;
	QChar ch = imechar_to_unicode(msg.wParam);
	if (!ch.isNull())
	    s += ch;
	k0 = sendKeyEvent( QEvent::KeyPress, 0, ch.row() ? 0 : ch.cell(), state, grab, s );
	k1 = sendKeyEvent( QEvent::KeyRelease, 0, ch.row() ? 0 : ch.cell(), state, grab, s );
    }
    else {

	// for Directionality changes (BiDi)
        static int dirStatus = 0;
        if ( !dirStatus && state == Qt::ControlButton && msg.wParam == VK_CONTROL && msg.message == WM_KEYDOWN ) {
	    if ( GetKeyState( VK_LCONTROL ) < 0 ) {
		dirStatus = VK_LCONTROL;
	    } else if ( GetKeyState( VK_RCONTROL ) < 0 ) {
		dirStatus = VK_RCONTROL;
	    }
	} else if ( dirStatus ) {
	    if ( msg.message == WM_KEYDOWN ) {
		if ( msg.wParam == VK_SHIFT ) {
		    if ( dirStatus == VK_LCONTROL && GetKeyState( VK_LSHIFT ) < 0 ) {
			    dirStatus = VK_LSHIFT;
		    } else if ( dirStatus == VK_RCONTROL && GetKeyState( VK_RSHIFT ) < 0 ) {
    			dirStatus = VK_RSHIFT;
		    }
		} else {
		    dirStatus = 0;
		}
	    } else if ( msg.message == WM_KEYUP ) {
		if ( dirStatus == VK_LSHIFT &&
		    ( msg.wParam == VK_SHIFT && GetKeyState( VK_LCONTROL )  ||
		      msg.wParam == VK_CONTROL && GetKeyState( VK_LSHIFT ) ) ) {
		    k0 = sendKeyEvent( QEvent::KeyPress, Qt::Key_Direction_L, 0, 0, grab, QString::null );
		    k1 = sendKeyEvent( QEvent::KeyRelease, Qt::Key_Direction_L, 0, 0, grab, QString::null );
		    dirStatus = 0;
		} else if ( dirStatus == VK_RSHIFT &&
		    ( msg.wParam == VK_SHIFT && GetKeyState( VK_RCONTROL ) ||
		      msg.wParam == VK_CONTROL && GetKeyState( VK_RSHIFT ) ) ) {
		    k0 = sendKeyEvent( QEvent::KeyPress, Qt::Key_Direction_R, 0, 0, grab, QString::null );
		    k1 = sendKeyEvent( QEvent::KeyRelease, Qt::Key_Direction_R, 0, 0, grab, QString::null );
		    dirStatus = 0;
		} else {
		    dirStatus = 0;
		}
	    } else {
		dirStatus = 0;
	    }
	}

	int code = translateKeyCode( msg.wParam );
	// If the bit 24 of lParm is set you received a enter,
	// otherwise a Return. (This is the extended key bit)
	if ((code == Qt::Key_Return) && (msg.lParam & 0x1000000)) {
	    code = Qt::Key_Enter;
	}

	if ( !(msg.lParam & 0x1000000) ) {	// All cursor keys without extended bit
	    switch ( code ) {
	    case Key_Left:
	    case Key_Right:
	    case Key_Up:
	    case Key_Down:
	    case Key_PageUp:
	    case Key_PageDown:
	    case Key_Home:
	    case Key_End:
	    case Key_Insert:
	    case Key_Delete:
	    case Key_Asterisk:
	    case Key_Plus:
	    case Key_Minus:
	    case Key_Period:
	    case Key_0:
	    case Key_1:
	    case Key_2:
	    case Key_3:
	    case Key_4:
	    case Key_5:
	    case Key_6:
	    case Key_7:
	    case Key_8:
	    case Key_9:
		state |= Keypad;
	    default:
		if ( (uint)msg.lParam == 0x004c0001 ||
		     (uint)msg.lParam == 0xc04c0001 )
		    state |= Keypad;
		break;
	    }
	} else {				// And some with extended bit
	    switch ( code ) {
	    case Key_Enter:
	    case Key_Slash:
	    case Key_NumLock:
		state |= Keypad;
	    default:
		break;
	    }
	}

	int t = msg.message;
	if ( t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN ) {
	    // KEYDOWN
	    KeyRec* rec = find_key_rec( msg.wParam, FALSE );
	    // Find uch
	    QChar uch;
	    MSG wm_char;
	    UINT charType = ( t == WM_KEYDOWN ? WM_CHAR :
			      t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR );
	    if ( winPeekMessage(&wm_char, 0, charType, charType, PM_REMOVE) ) {
		// Found a XXX_CHAR
		uch = charType == WM_IME_CHAR
			? imechar_to_unicode(wm_char.wParam)
			: wmchar_to_unicode(wm_char.wParam);
		if ( t == WM_SYSKEYDOWN &&
		     uch.isLetter() && (msg.lParam & KF_ALTDOWN) ) {
		    // (See doc of WM_SYSCHAR)
		    uch = uch.lower(); //Alt-letter
		}
		if ( !code && !uch.row() )
		    code = asciiToKeycode(uch.cell(), state);
	    } else {
		// No XXX_CHAR; deduce uch from XXX_KEYDOWN params
		if ( msg.wParam == VK_DELETE )
		    uch = QChar((char)0x7f); // Windows doesn't know this one.
		else {
		    if (t != WM_SYSKEYDOWN) {
			UINT map;
			QT_WA( {
			    map = MapVirtualKey( msg.wParam, 2 );
			} , {
			    map = MapVirtualKeyA( msg.wParam, 2 );
			    // High-order bit is 0x8000 on '95
			    if ( map & 0x8000 )
				map = (map^0x8000)|0x80000000;
			} );
			// If the high bit of the return value of
			// MapVirtualKey is set, the key is a deadkey.
			if ( !(map & 0x80000000) ) {
			    uch = wmchar_to_unicode((DWORD)map);
			}
		    }
		}
		if ( !code && !uch.row() )
		    code = asciiToKeycode( uch.cell(), state);
	    }

	    if ( state == Qt::AltButton ) {
		// Special handling of global Windows hotkeys
		switch ( code ) {
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Enter:
		case Qt::Key_F4:
		    return FALSE;		// Send the event on to Windows
		case Qt::Key_Space:
		    // do not pass this key to windows, we will process it ourselves
		    qt_show_system_menu( topLevelWidget() );
		    return TRUE;
		default:
		    break;
		}
	    }

	    if ( rec ) {
		// it is already down (so it is auto-repeating)
		if ( code < Key_Shift || code > Key_ScrollLock ) {
		    // map shift+tab to shift+backtab, QAccel knows about it
		    // and will handle it
		    if ( code == Key_Tab && ( state & ShiftButton ) == ShiftButton )
			code = Key_BackTab;
		    k0 = sendKeyEvent( QEvent::KeyRelease, code, rec->ascii,
				       state, grab, rec->text, TRUE);
		    k1 = sendKeyEvent( QEvent::KeyPress, code, rec->ascii,
				       state, grab, rec->text, TRUE);
		}
	    } else {
		QString text;
		if ( !uch.isNull() )
		    text += uch;
		char a = uch.row() ? 0 : uch.cell();
		store_key_rec( msg.wParam, a, text );
		k0 = sendKeyEvent( QEvent::KeyPress, code, a,
				   state, grab, text );
	    }


	} else {
	    // Must be KEYUP
	    KeyRec* rec = find_key_rec( msg.wParam, TRUE );
	    if ( !rec ) {
		// Someone ate the key down event
	    } else {
		if ( !code )
		    code = asciiToKeycode(rec->ascii ? rec->ascii : msg.wParam,
				state);
		k0 = sendKeyEvent( QEvent::KeyRelease, code, rec->ascii,
				    state, grab, rec->text);
		if ( code == Qt::Key_Alt )
		    k0 = TRUE; // don't let window see the meta key
	    }
	}
    }

    return k0 || k1;
}


bool QETWidget::translateWheelEvent( const MSG &msg )
{
    int  state = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( GetKeyState(VK_SHIFT) < 0 )
	state |= Qt::ShiftButton;
    if ( GetKeyState(VK_CONTROL) < 0 )
	state |= Qt::ControlButton;
    if ( GetKeyState(VK_MENU) < 0 )
	state |= Qt::AltButton;

    int delta;
    if ( msg.message == WM_MOUSEWHEEL )
	delta = (short) HIWORD ( msg.wParam );
    else
	delta = (int) msg.wParam;

    QPoint globalPos;

    globalPos.rx() = (short)LOWORD ( msg.lParam );
    globalPos.ry() = (short)HIWORD ( msg.lParam );


    // if there is a widget under the mouse and it is not shadowed
    // by modality, we send the event to it first
    int ret = 0;
    QWidget* w = QApplication::widgetAt( globalPos, TRUE );
    if ( !w || !qt_try_modal( w, (MSG*)&msg, ret ) )
	w = this;

    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( w->focusPolicy() == QWidget::WheelFocus ) {
	QFocusEvent::setReason( QFocusEvent::Mouse);
	w->setFocus();
	QFocusEvent::resetReason();
    }

    // send the event to the widget or its ancestors
    {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w->topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, (state&AltButton)?Horizontal:Vertical  );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if ( w != qApp->focusWidget() && ( w = qApp->focusWidget() ) ) {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w->topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, (state&AltButton)?Horizontal:Vertical  );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }
    return FALSE;
}

#if defined(QT_TABLET_SUPPORT)

//
// Windows Wintab to QTabletEvent translation
//

// the following is copied from the wintab syspress example (public domain)
/*------------------------------------------------------------------------------
The functions PrsInit and PrsAdjust make sure that our pressure out can always
reach the full 0-255 range we desire, regardless of the button pressed or the
"pressure button marks" settings.
------------------------------------------------------------------------------*/
/* pressure adjuster local state. */
/* need wOldCsr = -1, so PrsAdjust will call PrsInit first time */
static UINT wActiveCsr = 0,  wOldCsr = (UINT)-1;
static BYTE wPrsBtn;
static UINT prsYesBtnOrg, prsYesBtnExt, prsNoBtnOrg, prsNoBtnExt;
/* -------------------------------------------------------------------------- */
static void prsInit( HCTX hTab)
{
    /* browse WinTab's many info items to discover pressure handling. */
    if ( ptrWTInfo && ptrWTGet ) {
	AXIS np;
	LOGCONTEXT lc;
	BYTE logBtns[32];
	UINT btnMarks[2];
	UINT size;

	/* discover the LOGICAL button generated by the pressure channel. */
	/* get the PHYSICAL button from the cursor category and run it */
	/* through that cursor's button map (usually the identity map). */
	wPrsBtn = (BYTE)-1;
	ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_NPBUTTON, &wPrsBtn);
	size = ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_BUTTONMAP, &logBtns);
	if ((UINT)wPrsBtn < size)
	    wPrsBtn = logBtns[wPrsBtn];

	/* get the current context for its device variable. */
	ptrWTGet(hTab, &lc);

	/* get the size of the pressure axis. */
	ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &np);
	prsNoBtnOrg = (UINT)np.axMin;
	prsNoBtnExt = (UINT)np.axMax - (UINT)np.axMin;

	/* get the button marks (up & down generation thresholds) */
	/* and calculate what pressure range we get when pressure-button is down. */
	btnMarks[1] = 0; /* default if info item not present. */
	ptrWTInfo(WTI_CURSORS + wActiveCsr, CSR_NPBTNMARKS, btnMarks);
	prsYesBtnOrg = btnMarks[1];
	prsYesBtnExt = (UINT)np.axMax - btnMarks[1];
    }
}
/* -------------------------------------------------------------------------- */
static UINT prsAdjust(PACKET p, HCTX hTab)
{
    UINT wResult;

    wActiveCsr = p.pkCursor;
    if (wActiveCsr != wOldCsr) {

	/* re-init on cursor change. */
	prsInit( hTab );
	wOldCsr = wActiveCsr;
    }

    /* scaling output range is 0-255 */

    if (p.pkButtons & (1 << wPrsBtn)) {
	/* if pressure-activated button is down, */
	/* scale pressure output to compensate btn marks */
	wResult = p.pkNormalPressure - prsYesBtnOrg;
	wResult = MulDiv(wResult, 255, prsYesBtnExt);
    } else {
	/* if pressure-activated button is not down, */
	/* scale pressure output directly */
	wResult = p.pkNormalPressure - prsNoBtnOrg;
	wResult = MulDiv(wResult, 255, prsNoBtnExt);
    }

    return wResult;
}


bool QETWidget::translateTabletEvent( const MSG &msg, PACKET *localPacketBuf,
				      int numPackets )
{
    POINT ptNew;
    DWORD btnNew;
    UINT prsNew;
    ORIENTATION ort;
    static bool button_pressed = FALSE;
    int i,
	dev,
	tiltX,
	tiltY;
    bool sendEvent;
    QEvent::Type t;

    MSG msg1;
    // the most typical event that we get...
    t = QEvent::TabletMove;
    if ( winPeekMessage( &msg1, msg.hwnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE) ) {
	switch (msg1.message) {
	case WM_MOUSEMOVE:    	
	    t = QEvent::TabletMove;
	    break;
	case WM_LBUTTONDOWN:
	    button_pressed = TRUE;
	    t = QEvent::TabletPress;
	    break;
	default:
	    break;
	}
    }
    for ( i = 0; i < numPackets; i++ ) {
	if ( localPacketBuf[i].pkCursor == 2 ) {
	    dev = QTabletEvent::Eraser;
	} else if ( localPacketBuf[i].pkCursor == 1 ){
	    dev = QTabletEvent::Stylus;
	} else {
	    dev = QTabletEvent::NoDevice;
	}

	btnNew = localPacketBuf[i].pkButtons;
	ptNew.x = (UINT)localPacketBuf[i].pkX;
	ptNew.y = (UINT)localPacketBuf[i].pkY;
	prsNew = 0;
	if ( btnNew ) {	
	    prsNew = prsAdjust( localPacketBuf[i], hTab );
	} else if ( button_pressed ) {
	    // One button press, should only give one button release
	    t = QEvent::TabletRelease;
	    button_pressed = FALSE;
	}
	QPoint globalPos( ptNew.x, ptNew.y );

	// make sure the tablet event get's sent to the proper widget...
	QWidget *w = QApplication::widgetAt( globalPos, TRUE );
	if ( w == NULL )
	    w = this;
	QPoint localPos = w->mapFromGlobal( globalPos );
	if ( !tilt_support )
	    tiltX = tiltY = 0;
	else {
	    ort = localPacketBuf[i].pkOrientation;
	    // convert from azimuth and altitude to x tilt and y tilt
	    // what follows is the optimized version.  Here are the equations
	    // I used to get to this point (in case things change :)
	    // X = sin(azimuth) * cos(altitude)
	    // Y = cos(azimuth) * cos(altitude)
	    // Z = sin(altitude)
	    // X Tilt = arctan(X / Z)
	    // Y Tilt = arctan(Y / Z)
	    double radAzim = ( ort.orAzimuth / 10 ) * ( PI / 180 );
	    //double radAlt = abs( ort.orAltitude / 10 ) * ( PI / 180 );
	    double tanAlt = tan( (abs(ort.orAltitude / 10)) * ( PI / 180 ) );

	    double degX = atan( sin(radAzim) / tanAlt );
	    double degY = atan( cos(radAzim) / tanAlt );
	    tiltX = degX * ( 180 / PI );
	    tiltY = -degY * ( 180 / PI );
	}
        // get the unique ID of the device...
        int csr_type,
	    csr_physid;
	ptrWTInfo( WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_TYPE, &csr_type );
	ptrWTInfo( WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_PHYSID, &csr_physid );
	QPair<int,int> llId( csr_type, csr_physid );	
	QTabletEvent e( t, localPos, globalPos, dev, prsNew, tiltX, tiltY, llId );
	sendEvent = QApplication::sendSpontaneousEvent( w, &e );
    }
    return sendEvent;
}
#endif

static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent( QEvent::Type type, int code, int ascii,
			      int state, bool grab, const QString& text,
			      bool autor )
{
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel events if the keyboard is not grabbed
	QKeyEvent a( type, code, ascii, state, text, autor, int(text.length()) );
	if ( qt_dispatchAccelEvent( this, &a ) )
	    return TRUE;
    }
    if ( !isEnabled() )
	return FALSE;
    QKeyEvent e( type, code, ascii, state, text, autor );
    QApplication::sendSpontaneousEvent( this, &e );
    if ( !isModifierKey(code) && state == Qt::AltButton
	 && ((code>=Key_A && code<=Key_Z) || (code>=Key_0 && code<=Key_9))
	 && type == QEvent::KeyPress && !e.isAccepted() )
	QApplication::beep();  // emulate windows behavioar
    return e.isAccepted();
}


//
// Paint event translation
//
bool QETWidget::translatePaintEvent( const MSG & )
{
    PAINTSTRUCT ps;
    QRegion rgn(0,0,1,1); // trigger handle
    int res = GetUpdateRgn(winId(), (HRGN) rgn.handle(), FALSE);
    setWState( WState_InPaintEvent );
    hdc = BeginPaint( winId(), &ps );
    if ( res != COMPLEXREGION )
	rgn = QRect(QPoint(ps.rcPaint.left,ps.rcPaint.top),
		    QPoint(ps.rcPaint.right-1,ps.rcPaint.bottom-1));
    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent( this, (QEvent*) &e );
    hdc = 0;
    EndPaint( winId(), &ps );
    clearWState( WState_InPaintEvent );
    return TRUE;
}

//
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent( const MSG &msg )
{
    if ( !testWState(WState_Created) )		// in QWidget::create()
	return TRUE;
    setWState( WState_ConfigPending );		// set config flag
    QRect cr = geometry();
    if ( msg.message == WM_SIZE ) {		// resize event
	WORD a = LOWORD(msg.lParam);
	WORD b = HIWORD(msg.lParam);
	QSize oldSize = size();
	QSize newSize( a, b );
	cr.setSize( newSize );
	if ( msg.wParam != SIZE_MINIMIZED )
	    crect = cr;
	if ( isTopLevel() ) {			// update caption/icon text
	    createTLExtra();
	    if ( msg.wParam == SIZE_MINIMIZED ) {
		// being "hidden"
		extra->topextra->iconic = 1;
		if ( isVisible() ) {
		    clearWState( WState_Visible );
		    QHideEvent e;
		    QApplication::sendSpontaneousEvent( this, &e );
		    sendHideEventsToChildren( TRUE );
		}
	    } else if ( extra->topextra->iconic ) {
		// being shown
		extra->topextra->iconic = 0;
		if ( !isVisible() ) {
		    setWState( WState_Visible );
		    clearWState( WState_ForceHide );
		    sendShowEventsToChildren( TRUE );
		    QShowEvent e;
		    QApplication::sendSpontaneousEvent( this, &e );
		}
	    }
	    QString txt;
#ifndef Q_OS_TEMP
	    if ( IsIconic(winId()) && !!iconText() )
		txt = iconText();
	    else
#endif
		if ( !caption().isNull() )
		txt = caption();

	    if ( !!txt ) {
		QT_WA( {
		    SetWindowText( winId(), (TCHAR*)txt.ucs2() );
		} , {
		    SetWindowTextA( winId(), txt.local8Bit() );
		} );
	    }
	}
	if ( msg.wParam != SIZE_MINIMIZED && oldSize != newSize) {
	    if ( isVisible() ) {
		QResizeEvent e( newSize, oldSize );
		QApplication::sendSpontaneousEvent( this, &e );
		if ( !testWFlags( WStaticContents ) )
		    repaint( visibleRect(), !testWFlags(WResizeNoErase) );
	    } else {
		QResizeEvent *e = new QResizeEvent( newSize, oldSize );
		QApplication::postEvent( this, e );
	    }
	}
    } else if ( msg.message == WM_MOVE ) {	// move event
	int a = (int) (short) LOWORD(msg.lParam);
	int b = (int) (short) HIWORD(msg.lParam);
	QPoint oldPos = geometry().topLeft();
	QPoint newCPos( a, b );
	// Ignore silly Windows move event to wild pos after iconify.
	if ( !isMinimized() && newCPos != oldPos ) {
	    cr.moveTopLeft( newCPos );
	    crect = cr;
	    if ( isVisible() ) {
		QMoveEvent e( newCPos, oldPos );  // cpos (client position)
		QApplication::sendSpontaneousEvent( this, &e );
	    } else {
		QMoveEvent * e = new QMoveEvent( newCPos, oldPos );
		QApplication::postEvent( this, e );
	    }
	}
    }
    clearWState( WState_ConfigPending );		// clear config flag
    return TRUE;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent( const MSG & )
{
    return close(FALSE);
}

void  QApplication::setCursorFlashTime( int msecs )
{
    SetCaretBlinkTime( msecs / 2 );
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    int blink = GetCaretBlinkTime();
    if ( blink != 0 )
	return 2*blink;
    return cursor_flash_time;
}

void QApplication::setDoubleClickInterval( int ms )
{
#ifndef Q_OS_TEMP
    SetDoubleClickTime( ms );
#endif
    mouse_double_click_time = ms;
}


int QApplication::doubleClickInterval()
{
    int ms = GetDoubleClickTime();
    if ( ms != 0 )
	return ms;
    return mouse_double_click_time;
}

void QApplication::setWheelScrollLines( int n )
{
#ifdef SPI_SETWHEELSCROLLLINES
    if ( n < 0 )
	n = 0;
    QT_WA( {
	SystemParametersInfo( SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0 );
    } , {
	SystemParametersInfoA( SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0 );
    } );
#else
    wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    QT_WA( {
	SystemParametersInfo( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
    } , {
	SystemParametersInfoA( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
    } );
    if ( i > INT_MAX )
	i = INT_MAX;
    return i;
#else
    return wheel_scroll_lines;
#endif
}

static bool effect_override = FALSE;

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    effect_override = TRUE;
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }

    if ( desktopSettingsAware() && !( qt_winver == WV_95 || qt_winver == WV_NT ) ) {
	// we know that they can be used when we are here
	UINT api;
	switch (effect) {
	case UI_AnimateMenu:
	    api = SPI_SETMENUANIMATION;
	    break;
	case UI_FadeMenu:
	    if ( qt_winver == WV_98 )
		return;
	    api = SPI_SETMENUFADE;
	    break;
	case UI_AnimateCombo:
	    api = SPI_SETCOMBOBOXANIMATION;
	    break;
	case UI_AnimateTooltip:
	    api = SPI_SETTOOLTIPANIMATION;
	    break;
	case UI_FadeTooltip:
	    if ( qt_winver == WV_98 )
		return;
	    api = SPI_SETTOOLTIPFADE;
	    break;
	default:
	   api = SPI_SETUIEFFECTS;
	break;
	}
	BOOL onoff = enable;
	QT_WA( {
	    SystemParametersInfo( api, 0, &onoff, 0 );
	}, {
	    SystemParametersInfoA( api, 0, &onoff, 0 );
	} );
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( !effect_override && desktopSettingsAware() && !( qt_winver == WV_95 || qt_winver == WV_NT ) ) {
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	// we know that they can be used when we are here
	BOOL enabled = FALSE;
	UINT api;
	switch (effect) {
	case UI_AnimateMenu:
	    api = SPI_GETMENUANIMATION;
	    break;
	case UI_FadeMenu:
	    if ( qt_winver == WV_98 )
		return FALSE;
	    api = SPI_GETMENUFADE;
	    break;
	case UI_AnimateCombo:
	    api = SPI_GETCOMBOBOXANIMATION;
	    break;
	case UI_AnimateTooltip:
	    if ( qt_winver == WV_98 )
		api = SPI_GETMENUANIMATION;
	    else
		api = SPI_GETTOOLTIPANIMATION;
	    break;
	case UI_FadeTooltip:
	    if ( qt_winver == WV_98 )
		return FALSE;
	    api = SPI_GETTOOLTIPFADE;
	    break;
	default:
	    api = SPI_GETUIEFFECTS;
	    break;
	}
	QT_WA( {
	    SystemParametersInfo( api, 0, &enabled, 0 );
	} , {
	    SystemParametersInfoA( api, 0, &enabled, 0 );
	} );
	return enabled;
    } else {
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	switch( effect ) {
	case UI_AnimateMenu:
	    return animate_menu;
	case UI_FadeMenu:
	    return fade_menu;
	case UI_AnimateCombo:
	    return animate_combo;
	case UI_AnimateTooltip:
	    return animate_tooltip;
	case UI_FadeTooltip:
	    return fade_tooltip;
	default:
	    return animate_ui;
	}
    }
}

void QApplication::flush()
{
}


#if defined (QT_TABLET_SUPPORT)
extern bool qt_is_gui_used;
static void initWinTabFunctions()
{
    if ( !qt_is_gui_used )
	return;
    QLibrary library( "wintab32" );
    library.setAutoUnload( FALSE );
    QT_WA( {
	ptrWTOpen = (PtrWTOpen)library.resolve( "WTOpenW" );
	ptrWTInfo = (PtrWTInfo)library.resolve( "WTInfoW" );
	ptrWTGet = (PtrWTGet)library.resolve( "WTGetW" );
    } , {
	ptrWTOpen = (PtrWTOpen)library.resolve( "WTOpenA" );
	ptrWTInfo = (PtrWTInfo)library.resolve( "WTOInfoA" );
	ptrWTGet = (PtrWTGet)library.resolve( "WTGetA" );
    } );

    ptrWTClose = (PtrWTClose)library.resolve( "WTClose" );
    ptrWTEnable = (PtrWTEnable)library.resolve( "WTEnable" );
    ptrWTOverlap = (PtrWTEnable)library.resolve( "WTOverlap" );
    ptrWTPacketsGet = (PtrWTPacketsGet)library.resolve( "WTPacketsGet" );
}
#endif
/*!
    \enum Qt::WindowsVersion

    \value WV_32s
    \value WV_95
    \value WV_98
    \value WV_Me
    \value WV_DOS_based

    \value WV_NT
    \value WV_2000
    \value WV_XP
    \value WV_NT_based

*/


