/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qeventloop.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qpointer.h"
#include "qhash.h"
#include "qlibrary.h"
#include "qmetaobject.h"
#include "qmime.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qsessionmanager.h"
#include "qstyle.h"
#include "qwhatsthis.h" // ######## dependency
#include "qwidget.h"
#include "qcolormap.h"
#include "qt_windows.h"
#if defined(QT_NON_COMMERCIAL)
#include "qnc_win.h"
#endif

#include "qpaintengine_win.h"

#ifdef QT_THREAD_SUPPORT
#include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#include <winable.h>
#include <oleacc.h>
#ifndef WM_GETOBJECT
#define WM_GETOBJECT                    0x003D
#endif

extern IAccessible *qt_createWindowsAccessible(QAccessibleInterface *object);
#endif // QT_NO_ACCESSIBILITY

#include "private/qapplication_p.h"
#define d d_func()
#define q q_func()

#include "private/qinternal_p.h"
#include "private/qinputcontext_p.h"

#include <windowsx.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef Q_OS_TEMP
#include <sipapi.h>
#endif

#if defined(QT_TABLET_SUPPORT)
#define PACKETDATA  (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | \
                      PK_ORIENTATION | PK_CURSOR)
#define PACKETMODE  0

extern bool chokeMouse;

#include <wintab.h>
#ifndef CSR_TYPE
#define CSR_TYPE 20 // Some old Wacom wintab.h may not provide this constant.
#endif
#include <pktdef.h>
#include <math.h>

typedef HCTX        (API *PtrWTOpen)(HWND, LPLOGCONTEXT, BOOL);
typedef BOOL        (API *PtrWTClose)(HCTX);
typedef UINT        (API *PtrWTInfo)(UINT, UINT, LPVOID);
typedef BOOL        (API *PtrWTEnable)(HCTX, BOOL);
typedef BOOL        (API *PtrWTOverlap)(HCTX, BOOL);
typedef int        (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
typedef BOOL        (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
typedef int     (API *PtrWTQueueSizeGet)(HCTX);
typedef BOOL    (API *PtrWTQueueSizeSet)(HCTX, int);

static PtrWTInfo         ptrWTInfo = 0;
static PtrWTEnable         ptrWTEnable = 0;
static PtrWTOverlap         ptrWTOverlap = 0;
static PtrWTPacketsGet         ptrWTPacketsGet = 0;
static PtrWTGet                 ptrWTGet = 0;

static const double PI = 3.14159265358979323846;

static PACKET localPacketBuf[QT_TABLET_NPACKETQSIZE];  // our own tablet packet queue.
HCTX qt_tablet_context;  // the hardware context for the tablet (like a window handle)
bool qt_tablet_tilt_support;
static void prsInit(HCTX hTab);
static UINT prsAdjust(PACKET p, HCTX hTab);
static void initWinTabFunctions();        // resolve the WINTAB api functions
#endif

Q_CORE_EXPORT bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                            UINT wMsgFilterMax, UINT wRemoveMsg);
Q_CORE_EXPORT bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#if defined(__CYGWIN32__)
#define __INSIDE_CYGWIN32__
#include <mywinsock.h>
#endif

// support for on-the-fly changes of the XP theme engine
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif
#ifndef COLOR_MENUHILIGHT
#define COLOR_MENUHILIGHT                29
#define COLOR_MENUBAR                        30
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

#if(_WIN32_WINNT < 0x0400)
// This struct is defined in winuser.h if the _WIN32_WINNT >= 0x0400 -- in the
// other cases we have to define it on our own.
typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;
#endif
#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE                   0x02A3
#endif

#include "private/qwidget_p.h"

extern void qt_dispatchEnterLeave(QWidget*, QWidget*); // qapplication.cpp
static int translateButtonState(int s, int type, int button);

/*
  Internal functions.
*/

void qt_draw_tiled_pixmap(HDC, int, int, int, int,
                           const QPixmap *, int, int);

void qt_erase_background(HDC hdc, int x, int y, int w, int h,
                         const QBrush &brush, int off_x, int off_y,
                         QWidget *widget)
{
    if (brush.pixmap() && brush.pixmap()->isNull())        // empty background
        return;
    HPALETTE oldPal = 0;
    HPALETTE hpal = QColormap::hPal();
    if (hpal) {
        oldPal = SelectPalette(hdc, hpal, false);
        RealizePalette(hdc);
    }
    if (brush.style() == Qt::LinearGradientPattern) {
        QPainter p(widget);
        p.fillRect(x, y, w, h, brush);
        return;
    }
    else if (brush.pixmap()) {
        qt_draw_tiled_pixmap(hdc, x, y, w, h, brush.pixmap(), off_x, off_y);
    } else {
        HBRUSH hbrush = CreateSolidBrush(brush.color().pixel());
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hbrush);
        PatBlt(hdc, x, y, w, h, PATCOPY);
        SelectObject(hdc, oldBrush);
        DeleteObject(hbrush);
    }
    if (hpal) {
        SelectPalette(hdc, oldPal, true);
        RealizePalette(hdc);
    }
}

// ##### get rid of this!
QRgb qt_colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

extern Q_CORE_EXPORT char      appName[];
extern Q_CORE_EXPORT char      appFileName[];
extern Q_CORE_EXPORT HINSTANCE appInst;                        // handle to app instance
extern Q_CORE_EXPORT HINSTANCE appPrevInst;                        // handle to prev app instance
extern Q_CORE_EXPORT int appCmdShow;                                // main window show command
static HWND         curWin                = 0;                // current window
static HDC         displayDC        = 0;                // display device context
#ifdef Q_OS_TEMP
static UINT         appUniqueID        = 0;                // application id
#endif

// Session management
static bool        sm_blockUserInput    = false;
static bool        sm_smActive             = false;
extern QSessionManager* qt_session_manager_self;
static bool        sm_cancel;

static bool replayPopupMouseEvent = false; // replay handling when popups close

// ignore the next release event if return from a modal widget
Q_GUI_EXPORT bool qt_win_ignoreNextMouseReleaseEvent = false;

#if defined(QT_DEBUG)
static bool        appNoGrab        = false;        // mouse/keyboard grabbing
#endif

static bool        app_do_modal           = false;        // modal mode
extern QWidgetList *qt_modal_stack;
extern QDesktopWidget *qt_desktopWidget;
static QWidget *popupButtonFocus   = 0;
static bool        qt_try_modal(QWidget *, MSG *, int& ret);

QWidget               *qt_button_down = 0;                // widget got last button-down

static HWND        autoCaptureWnd = 0;
static void        setAutoCapture(HWND);                // automatic capture
static void        releaseAutoCapture();

static void     unregWinClasses();

static int        translateKeyCode(int);

extern QCursor *qt_grab_cursor();

#if defined(Q_WS_WIN)
#define __export
#endif

extern "C" LRESULT CALLBACK QtWndProc(HWND, UINT, WPARAM, LPARAM);

class QETWidget : public QWidget                // event translator widget
{
public:
    void        setWFlags(Qt::WFlags f)        { QWidget::setWFlags(f); }
    void        clearWFlags(Qt::WFlags f) { QWidget::clearWFlags(f); }
    void        setWState(Qt::WState f)        { QWidget::setWState(f); }
    void        clearWState(Qt::WState f) { QWidget::clearWState(f); }
    QWExtra    *xtra()                        { return d->extraData(); }
    bool        winEvent(MSG *m)        { return QWidget::winEvent(m); }
    void        markFrameStrutDirty()        { data->fstrut_dirty = 1; }
    bool        translateMouseEvent(const MSG &msg);
    bool        translateKeyEvent(const MSG &msg, bool grab);
    bool        translateWheelEvent(const MSG &msg);
    bool        sendKeyEvent(QEvent::Type type, int code,
                              int state, bool grab, const QString& text,
                              bool autor=false);
    bool        translatePaintEvent(const MSG &msg);
    bool        translateConfigEvent(const MSG &msg);
    bool        translateCloseEvent(const MSG &msg);
#if defined(QT_TABLET_SUPPORT)
        bool        translateTabletEvent(const MSG &msg, PACKET *localPacketBuf,
                                          int numPackets);
#endif
    void        repolishStyle(QStyle &style) { setStyle(&style); }
    void eraseWindowBackground(HDC);
    inline void showChildren(bool spontaneous) { QWidget::showChildren(spontaneous); }
    inline void hideChildren(bool spontaneous) { QWidget::hideChildren(spontaneous); }
};

static void qt_show_system_menu(QWidget* tlw)
{
    HMENU menu = GetSystemMenu(tlw->winId(), false);
    if (!menu)
        return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

#ifndef Q_OS_TEMP
    EnableMenuItem(menu, SC_MINIMIZE, enabled);
    bool maximized  = IsZoomed(tlw->winId());

    EnableMenuItem(menu, SC_MAXIMIZE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem(menu, SC_SIZE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_MOVE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_CLOSE, enabled);
#endif

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx(menu,
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
#ifndef Q_OS_TEMP
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;
    QFont titleFont;
    QFont smallTitleFont;

    QT_WA({
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont(ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont(ncm.lfSmCaptionFont,true);
    } , {
        // A version
        NONCLIENTMETRICSA ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfSmCaptionFont,true);
    });

    QApplication::setFont(menuFont, "QMenu");
    QApplication::setFont(menuFont, "QMenuBar");
    QApplication::setFont(messageFont, "QMessageBox");
    QApplication::setFont(statusFont, "QTipLabel");
    QApplication::setFont(statusFont, "QStatusBar");
    QApplication::setFont(titleFont, "QTitleBar");
    QApplication::setFont(smallTitleFont, "QDockWindowTitleBar");
#else
    LOGFONT lf;
    HGDIOBJ stockFont = GetStockObject(SYSTEM_FONT);
    GetObject(stockFont, sizeof(lf), &lf);
    QApplication::setFont(qt_LOGFONTtoQFont(lf, true));
#endif// Q_OS_TEMP

    if (qt_std_pal && *qt_std_pal != QApplication::palette())
        return;

    // Do the color settings
    QPalette pal;
    pal.setColor(QPalette::Foreground,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))));
    pal.setColor(QPalette::Button,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))));
    pal.setColor(QPalette::Light,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))));
    pal.setColor(QPalette::Dark,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNSHADOW))));
    pal.setColor(QPalette::Mid, pal.button().color().dark(150));
    pal.setColor(QPalette::Text,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOWTEXT))));
    pal.setColor(QPalette::BrightText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNHIGHLIGHT))));
    pal.setColor(QPalette::Base,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_WINDOW))));
    pal.setColor(QPalette::Background,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNFACE))));
    pal.setColor(QPalette::ButtonText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_BTNTEXT))));
    pal.setColor(QPalette::Midlight,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DLIGHT))));
    pal.setColor(QPalette::Shadow,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_3DDKSHADOW))));
    pal.setColor(QPalette::Highlight,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))));
    pal.setColor(QPalette::HighlightedText,
                 QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))));
    // ### hardcoded until I find out how to get it from the system settings.
    pal.setColor(QPalette::Link, Qt::blue);
    pal.setColor(QPalette::LinkVisited, Qt::magenta);

    if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95) {
        if (pal.midlight() == pal.button())
            pal.setColor(QPalette::Midlight, pal.button().color().light(110));
        if (pal.background() != pal.base()) {
            pal.setColor(QPalette::Inactive, QPalette::Highlight, pal.color(QPalette::Inactive, QPalette::Background));
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, pal.color(QPalette::Inactive, QPalette::Text));
        }
    }

    const QColor fg = pal.foreground().color(), btn = pal.button().color();
    QColor disabled((fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
                     (fg.blue()+btn.blue())/2);
    pal.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
    pal.setColor(QPalette::Disabled, QPalette::Text, disabled);
    pal.setColor(QPalette::Disabled, QPalette::Highlight,
                  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHT))));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                  QColor(qt_colorref2qrgb(GetSysColor(COLOR_HIGHLIGHTTEXT))));

    QApplication::setPalette(pal);
    *qt_std_pal = pal;

    QColor menuCol(qt_colorref2qrgb(GetSysColor(COLOR_MENU)));
    QColor menuText(qt_colorref2qrgb(GetSysColor(COLOR_MENUTEXT)));
    {
        BOOL isFlat = 0;
        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
            SystemParametersInfo(0x1022 /*SPI_GETFLATMENU*/, 0, &isFlat, 0);
        QPalette menu(pal);
        // we might need a special color group for the menu.
        menu.setColor(QPalette::Active, QPalette::Button, menuCol);
        menu.setColor(QPalette::Active, QPalette::Text, menuText);
        menu.setColor(QPalette::Active, QPalette::Foreground, menuText);
        menu.setColor(QPalette::Active, QPalette::ButtonText, menuText);
        const QColor fg = menu.foreground().color(), btn = menu.button().color();
        QColor disabled(qt_colorref2qrgb(GetSysColor(COLOR_GRAYTEXT)));
        menu.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
        menu.setColor(QPalette::Disabled, QPalette::Text, disabled);
        menu.setColor(QPalette::Disabled, QPalette::Highlight,
                       QColor(qt_colorref2qrgb(GetSysColor(
                                               QSysInfo::WindowsVersion == QSysInfo::WV_XP
                                               && isFlat ? COLOR_MENUHILIGHT
                                                         : COLOR_HIGHLIGHT))));
        menu.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);
        menu.setColor(QPalette::Inactive, QPalette::Button,
                      menu.color(QPalette::Active, QPalette::Button));
        menu.setColor(QPalette::Inactive, QPalette::Text,
                      menu.color(QPalette::Active, QPalette::Text));
        menu.setColor(QPalette::Inactive, QPalette::Foreground,
                      menu.color(QPalette::Active, QPalette::Foreground));
        menu.setColor(QPalette::Inactive, QPalette::ButtonText,
                      menu.color(QPalette::Active, QPalette::ButtonText));
        if (QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95)
            menu.setColor(QPalette::Inactive, QPalette::ButtonText,
                          pal.color(QPalette::Inactive, QPalette::Dark));
        QApplication::setPalette(menu, "QMenu");

        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP && isFlat) {
            QColor menubar(qt_colorref2qrgb(GetSysColor(COLOR_MENUBAR)));
            menu.setColor(QPalette::Active, QPalette::Button, menubar);
            menu.setColor(QPalette::Disabled, QPalette::Button, menubar);
            menu.setColor(QPalette::Inactive, QPalette::Button, menubar);
        }
        QApplication::setPalette(menu, "QMenuBar");
    }

    QColor ttip(qt_colorref2qrgb(GetSysColor(COLOR_INFOBK)));

    QColor ttipText(qt_colorref2qrgb(GetSysColor(COLOR_INFOTEXT)));
    {
        QPalette tiplabel(pal);
        tiplabel.setColor(QPalette::All, QPalette::Button, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Background, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Text, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Foreground, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::ButtonText, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Button, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Background, ttip);
        tiplabel.setColor(QPalette::All, QPalette::Text, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::Foreground, ttipText);
        tiplabel.setColor(QPalette::All, QPalette::ButtonText, ttipText);
        const QColor fg = tiplabel.foreground().color(), btn = tiplabel.button().color();
        QColor disabled((fg.red()+btn.red())/2,(fg.green()+btn.green())/2,
                         (fg.blue()+btn.blue())/2);
        tiplabel.setColor(QPalette::Disabled, QPalette::Foreground, disabled);
        tiplabel.setColor(QPalette::Disabled, QPalette::Text, disabled);
        tiplabel.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
        tiplabel.setColor(QPalette::Disabled, QPalette::BrightText, Qt::white);
        QApplication::setPalette(tiplabel, "QTipLabel");
    }
}

/*****************************************************************************
  qt_init() - initializes Qt for Windows
 *****************************************************************************/

// need to get default font?
extern bool qt_app_has_font;

void qt_init(QApplicationPrivate *priv, int)
{

#if defined(QT_DEBUG)
    int argc = priv->argc;
    char **argv = priv->argv;
    int i, j;

  // Get command line params

    j = argc ? 1 : 0;
    for (i=1; i<argc; i++) {
        if (argv[i] && *argv[i] != '-') {
            argv[j++] = argv[i];
            continue;
        }
        if (qstrcmp(argv[i], "-nograb") == 0)
            appNoGrab = !appNoGrab;
        else
            argv[j++] = argv[i];
    }
    priv->argc = j;
#else
    Q_UNUSED(priv);
#endif // QT_DEBUG

    // Get the application name/instance if qWinMain() was not invoked
#ifndef Q_OS_TEMP
    // No message boxes but important ones
    SetErrorMode(SetErrorMode(0) | SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#endif

    if (appInst == 0) {
        QT_WA({
            appInst = GetModuleHandle(0);
        }, {
            appInst = GetModuleHandleA(0);
        });
    }

#ifndef Q_OS_TEMP
    // Initialize OLE/COM
    //         S_OK means success and S_FALSE means that it has already
    //         been initialized
    HRESULT r;
    r = OleInitialize(0);
    if (r != S_OK && r != S_FALSE) {
        qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
    }
#endif

    // Misc. initialization
#if defined(QT_DEBUG)
    GdiSetBatchLimit(1);
#endif

    QWindowsMime::initialize();
    QColormap::initialize();
    QFont::initialize();
    QCursor::initialize();
    QWin32PaintEngine::initialize();
    qApp->setObjectName(appName);

    // default font
    if (!qt_app_has_font) {
        HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        QFont f("MS Sans Serif",8);
        QT_WA({
            LOGFONT lf;
            if (GetObject(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        } , {
            LOGFONTA lf;
            if (GetObjectA(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        });
        QApplication::setFont(f);
    }

    // QFont::locale_init();  ### Uncomment when it does something on Windows

    if (!qt_std_pal)
        qt_create_std_palette();
    if (QApplication::desktopSettingsAware())
        qt_set_windows_resources();

    QT_WA({
        WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    } , {
        WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
    });
#if defined(QT_TABLET_SUPPORT)
    initWinTabFunctions();
#endif
    QInputContext::init();
}

/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    unregWinClasses();
    QPixmapCache::clear();
    QWin32PaintEngine::cleanup();

    QCursor::cleanup();
    QFont::cleanup();
    QColormap::cleanup();
    if (displayDC) {
        ReleaseDC(0, displayDC);
        displayDC = 0;
    }

    QInputContext::shutdown();

#ifndef Q_OS_TEMP
  // Deinitialize OLE/COM
    OleUninitialize();
#endif
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

Q_GUI_EXPORT int qWinAppCmdShow()                        // get main window show command
{
    return appCmdShow;
}


Q_GUI_EXPORT HDC qt_display_dc()                        // get display DC
{
    if (!displayDC)
        displayDC = GetDC(0);
    return displayDC;
}

bool qt_nograb()                                // application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return false;
#endif
}

typedef QHash<QString, int> WinClassNameHash;
Q_GLOBAL_STATIC(WinClassNameHash, winclassNames)

const QString qt_reg_winclass(Qt::WFlags flags)        // register window class
{
    uint style;
    bool icon;
    QString cname;
    if (flags & Qt::WWinOwnDC) {
        cname = "QWidgetOwnDC";
#ifndef Q_OS_TEMP
        style = CS_OWNDC | CS_DBLCLKS;
#else
        style = CS_DBLCLKS;
#endif
        icon  = true;
    } else if ((flags & (Qt::WType_Popup|Qt::WStyle_Tool)) == 0) {
        cname = "QWidget";
        style = CS_DBLCLKS;
        icon  = true;
    } else {
        cname = "QPopup";
#ifndef Q_OS_TEMP
        style = CS_DBLCLKS | CS_SAVEBITS;
#else
        style = CS_DBLCLKS;
#endif
        if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
            style |= 0x00020000;                // CS_DROPSHADOW
        icon  = false;
    }

#ifdef Q_OS_TEMP
    // We need to register the classes with the
    // unique ID on WinCE to make sure we can
    // move the windows to the front when starting
    // a second instance.
    cname = QString::number(appUniqueID);
#endif

    // since multiple Qt versions can be used in one process
    // each one has to have window class names with a unique name
    // The first instance gets the unmodified name; if the class
    // has already been registered by another instance of Qt then
    // add an instance-specific ID, the address of the window proc.
    static int classExists = -1;

    if (classExists == -1) {
        QT_WA({
            WNDCLASS wcinfo;
            classExists = GetClassInfo((HINSTANCE)qWinAppInst(), (TCHAR*)cname.utf16(), &wcinfo);
            classExists &= classExists ? wcinfo.lpfnWndProc != QtWndProc : 0;
        }, {
            WNDCLASSA wcinfo;
            classExists = GetClassInfoA((HINSTANCE)qWinAppInst(), cname.latin1(), &wcinfo);
            classExists &= classExists ? wcinfo.lpfnWndProc != QtWndProc : 0;
        });
    }

    if (classExists)
        cname += QString::number((uint)QtWndProc);

    if (winclassNames()->contains(cname))        // already registered in our list
        return cname;

    ATOM atom;
#ifndef Q_OS_TEMP
    QT_WA({
        WNDCLASS wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIcon(appInst, L"IDI_ICON1");
            if (!wc.hIcon)
                wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
        wc.lpszClassName= (TCHAR*)cname.utf16();
        atom = RegisterClass(&wc);
    } , {
        WNDCLASSA wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIconA(appInst, (char*)"IDI_ICON1");
            if (!wc.hIcon)
                wc.hIcon = LoadIconA(0, (char*)IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
        wc.lpszClassName= cname.latin1();
        atom = RegisterClassA(&wc);
    });
#else
        WNDCLASS wc;
        wc.style        = style;
        wc.lpfnWndProc        = (WNDPROC)QtWndProc;
        wc.cbClsExtra        = 0;
        wc.cbWndExtra        = 0;
        wc.hInstance        = (HINSTANCE)qWinAppInst();
        if (icon) {
            wc.hIcon = LoadIcon(appInst, L"IDI_ICON1");
//            if (!wc.hIcon)
//                wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        } else {
            wc.hIcon = 0;
        }
        wc.hCursor        = 0;
        wc.hbrBackground= 0;
        wc.lpszMenuName        = 0;
        wc.lpszClassName= (TCHAR*)cname.utf16();
        atom = RegisterClass(&wc);
#endif

#ifndef QT_NO_DEBUG
    if (!atom)
        qSystemWarning("QApplication: Registering window class failed.");
#endif

    winclassNames()->insert(cname, 1);
    return cname;
}

static void unregWinClasses()
{
    WinClassNameHash *hash = winclassNames();
    QHash<QString, int>::ConstIterator it = hash->constBegin();
    while (it != hash->constEnd()) {
        QT_WA({
            UnregisterClass((TCHAR*)it.key().utf16(), (HINSTANCE)qWinAppInst());
        } , {
            UnregisterClassA(it.key().latin1(), (HINSTANCE)qWinAppInst());
        });
        ++it;
    }
    hash->clear();
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget(QWidget *mainWidget)
{
    main_widget = mainWidget;
    if (main_widget && windowIcon().isNull() && main_widget->testAttribute(Qt::WA_SetWindowIcon))
        setWindowIcon(main_widget->windowIcon());
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor*> QCursorList;

static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor(const QCursor &cursor, bool replace)
{
    if (replace && !qApp->d->cursor_list.isEmpty())
        qApp->d->cursor_list.replace(0, cursor);
    else
        qApp->d->cursor_list.prepend(cursor);

    SetCursor(qApp->d->cursor_list.first().handle());
}

void QApplication::restoreOverrideCursor()
{
    if (qApp->d->cursor_list.isEmpty())
        return;
    qApp->d->cursor_list.removeFirst();

    if (!qApp->d->cursor_list.isEmpty()) {
        SetCursor(qApp->d->cursor_list.first().handle());
    } else {
        QWidget *w = QWidget::find(curWin);
        if (w)
            SetCursor(w->cursor().handle());
        else
            SetCursor(QCursor(Qt::ArrowCursor).handle());
    }
}

#endif

/*
  Internal function called from QWidget::setCursor()
*/

void qt_set_cursor(QWidget *w, const QCursor& /* c */)
{
    if (!curWin)
        return;
    QWidget* cW = QWidget::find(curWin);
    if (!cW || cW->topLevelWidget() != w->topLevelWidget() ||
         !cW->isVisible() || !cW->underMouse()
         /* ##### || cursorStack */
         )
        return;

    SetCursor(cW->cursor().handle());
}



/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

static QWidget *findChildWidget(const QWidget *p, const QPoint &pos)
{
    QObjectList children = p->children();
    for(int i = children.size(); i > 0 ;) {
        --i;
        QObject *o = children.at(i);
        if (o->isWidgetType()) {
            QWidget *w = (QWidget*)o;
            if (w->isVisible() && w->geometry().contains(pos)) {
                QWidget *c = findChildWidget(w, w->mapFromParent(pos));
                return c ? c : w;
            }
        }
    }
    return 0;
}

QWidget *QApplication::topLevelAt(int x, int y)
{
    QWidget *c = widgetAt_sys(x, y);
    return c ? c->topLevelWidget() : 0;
}

QWidget *QApplication::widgetAt_sys(int x, int y)
{
    POINT p;
    HWND  win;
    QWidget *w;
    p.x = x;
    p.y = y;
    win = WindowFromPoint(p);
    if (!win)
        return 0;

    w = QWidget::find(win);
    while (!w && win) {
        win = GetParent(win);
        w = QWidget::find(win);
    }
    return w;
}

void QApplication::beep()
{
    MessageBeep(MB_OK);
}

/*****************************************************************************
  Windows-specific drawing used here
 *****************************************************************************/

static void drawTile(HDC hdc, int x, int y, int w, int h,
                      const QPixmap *pixmap, int xOffset, int yOffset)
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    HDC tmp_hdc = pixmap->getDC();
    while(yPos < y + h) {
        drawH = pixmap->height() - yOff;        // Cropping first row
        if (yPos + drawH > y + h)                // Cropping last row
            drawH = y + h - yPos;
        xPos = x;
        xOff = xOffset;
        while(xPos < x + w) {
            drawW = pixmap->width() - xOff;        // Cropping first column
            if (xPos + drawW > x + w)                // Cropping last column
                drawW = x + w - xPos;
            BitBlt(hdc, xPos, yPos, drawW, drawH, tmp_hdc, xOff, yOff, SRCCOPY);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    pixmap->releaseDC(tmp_hdc);
}

extern void qt_fill_tile(QPixmap *tile, const QPixmap &pixmap);

void qt_draw_tiled_pixmap(HDC hdc, int x, int y, int w, int h,
                           const QPixmap *bg_pixmap,
                           int off_x, int off_y)
{
    QPixmap *tile = 0;
    QPixmap *pm;
    int  sw = bg_pixmap->width(), sh = bg_pixmap->height();
    if (sw*sh < 8192 && sw*sh < 16*w*h) {
        int tw = sw, th = sh;
        while (tw*th < 32678 && tw < w/2)
            tw *= 2;
        while (tw*th < 32678 && th < h/2)
            th *= 2;
        tile = new QPixmap(tw, th, bg_pixmap->depth(),
                            QPixmap::NormalOptim);
        qt_fill_tile(tile, *bg_pixmap);
        pm = tile;
    } else {
        pm = (QPixmap*)bg_pixmap;
    }
    drawTile(hdc, x, y, w, h, pm, off_x, off_y);
    if (tile)
        delete tile;
}



/*****************************************************************************
  Main event loop
 *****************************************************************************/

extern uint qGlobalPostedEventsCount();

/*!
    If \a gotFocus is true, \a widget will become the active window.
    Otherwise the active window is reset to NULL.
*/
void QApplication::winFocus(QWidget *widget, bool gotFocus)
{
    if (inPopupMode()) // some delayed focus event to ignore
        return;
    if (gotFocus) {
        setActiveWindow(widget);
        if (active_window && active_window->testWFlags(Qt::WType_Dialog)) {
            // raise the entire application, not just the dialog
            QWidget* mw = active_window;
            while(mw->parentWidget() && mw->testWFlags(Qt::WType_Dialog))
                mw = mw->parentWidget()->topLevelWidget();
            if (mw != active_window)
                SetWindowPos(mw->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
    } else {
        setActiveWindow(0);
    }
}


//
// QtWndProc() receives all messages from the main event loop
//

static bool inLoop = false;
static int inputcharset = CP_ACP;

#define RETURN(x) { inLoop=false;return x; }

bool qt_sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
    return QCoreApplication::sendSpontaneousEvent(receiver, event);
}

extern "C"
LRESULT CALLBACK QtWndProc(HWND hwnd, UINT message, WPARAM wParam,
                            LPARAM lParam)
{
    bool result = true;
    QEvent::Type evt_type = QEvent::None;
    QETWidget *widget = 0;

#if defined(QT_TABLET_SUPPORT)
        // there is no need to process pakcets from tablet unless
        // it is actually on the tablet, a flag to let us know...
        int nPackets;        // the number of packets we get from the queue
#endif

    if (!qApp)                                // unstable app state
        goto do_default;

    // make sure we update widgets also when the user resizes
    if (inLoop && qApp->loopLevel())
        qApp->sendPostedEvents(0, QEvent::Paint);

    inLoop = true;

    MSG msg;
    msg.hwnd = hwnd;                                // create MSG structure
    msg.message = message;                        // time and pt fields ignored
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.pt.x = GET_X_LPARAM(lParam);
    msg.pt.y = GET_Y_LPARAM(lParam);
    ClientToScreen(msg.hwnd, &msg.pt);         // the coords we get are client coords

    /*
    // sometimes the autograb is not released, so the clickevent is sent
    // to the wrong window. We ignore this for now, because it doesn't
    // cause any problems.
    if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN) {
        HWND handle = WindowFromPoint(msg.pt);
        if (msg.hwnd != handle) {
            msg.hwnd = handle;
            hwnd = handle;
        }
    }
    */

#if defined(QT_NON_COMMERCIAL)
    QT_NC_WNDPROC
#endif

    if (QEventLoop::instance()) {
        LRESULT res;
        if (QEventLoop::instance()->winEventFilter(&msg, &res))                // send through app filter
            RETURN(res);
    }

    switch (message) {
#ifndef Q_OS_TEMP
    case WM_QUERYENDSESSION: {
        if (sm_smActive) // bogus message from windows
            RETURN(true);

        sm_smActive = true;
        sm_blockUserInput = true; // prevent user-interaction outside interaction windows
        sm_cancel = false;
        if (qt_session_manager_self)
            qApp->commitData(*qt_session_manager_self);
        if (lParam == (LPARAM)ENDSESSION_LOGOFF) {
            _flushall();
        }
        RETURN(!sm_cancel);
    }
    case WM_ENDSESSION: {
        sm_smActive = false;
        sm_blockUserInput = false;
        bool endsession = (bool) wParam;

        if (endsession) {
            // since the process will be killed immediately quit() has no real effect
            int index = QApplication::staticMetaObject.indexOfSignal("aboutToQuit()");
            qApp->qt_metacall(QMetaObject::InvokeMetaMember, index,0);
            qApp->quit();
        }

        RETURN(0);
    }
    case WM_DISPLAYCHANGE:
        if (qApp->type() == QApplication::Tty)
            break;
        if (qt_desktopWidget) {
            int x = GetSystemMetrics(76);
            int y = GetSystemMetrics(77);
            QMoveEvent mv(QPoint(x, y), qt_desktopWidget->pos());
            QApplication::sendEvent(qt_desktopWidget, &mv);
            x = GetSystemMetrics(78);
            y = GetSystemMetrics(79);
            qt_desktopWidget->resize(x, y);
        }
        break;
#endif

    case WM_SETTINGCHANGE:
#ifdef Q_OS_TEMP
        // CE SIP hide/show
        if (wParam == SPI_SETSIPINFO) {
            QResizeEvent re(QSize(0, 0), QSize(0, 0)); // Calculated by QDesktopWidget
            QApplication::sendEvent(qt_desktopWidget, &re);
            break;
        }
#endif
        // ignore spurious XP message when user logs in again after locking
        if (qApp->type() == QApplication::Tty)
            break;
        if (QApplication::desktopSettingsAware() && wParam != SPI_SETWORKAREA) {
            widget = (QETWidget*)QWidget::find(hwnd);
            if (widget) {
                widget->markFrameStrutDirty();
                if (!widget->parentWidget())
                    qt_set_windows_resources();
            }
        }
        break;
    case WM_SYSCOLORCHANGE:
        if (qApp->type() == QApplication::Tty)
            break;
        if (QApplication::desktopSettingsAware()) {
            widget = (QETWidget*)QWidget::find(hwnd);
            if (widget && !widget->parentWidget())
                qt_set_windows_resources();
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
        if (qt_win_ignoreNextMouseReleaseEvent)
            qt_win_ignoreNextMouseReleaseEvent = false;
        break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
	if (qt_win_ignoreNextMouseReleaseEvent) {
	    qt_win_ignoreNextMouseReleaseEvent = false;
	    if (qt_button_down && qt_button_down->winId() == autoCaptureWnd) {
		releaseAutoCapture();
		qt_button_down = 0;
	    }

            RETURN(0);
        }
        break;

    default:
        break;
    }

    if (!widget)
        widget = (QETWidget*)QWidget::find(hwnd);
    if (!widget)                                // don't know this widget
        goto do_default;

    if (app_do_modal)        {                        // modal event handling
        int ret = 0;
        if (!qt_try_modal(widget, &msg, ret))
            RETURN(ret);
    }

    if (widget->winEvent(&msg))                // send through widget filter
        RETURN(0);

    if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST ||
           message >= WM_XBUTTONDOWN && message <= WM_XBUTTONDBLCLK)
         && message != WM_MOUSEWHEEL) {
        if (qApp->activePopupWidget() != 0) { // in popup mode
            POINT curPos = msg.pt;
            QWidget* w = QApplication::widgetAt(curPos.x, curPos.y);
            if (w)
                widget = (QETWidget*)w;
        }

#if defined(QT_TABLET_SUPPORT)
        if (!chokeMouse) {
#endif
            widget->translateMouseEvent(msg);        // mouse event
#if defined(QT_TABLET_SUPPORT)
        } else {
            // Sometimes we only get a WM_MOUSEMOVE message
            // and sometimes we get both a WM_MOUSEMOVE and
            // a WM_LBUTTONDOWN/UP, this creates a spurious mouse
            // press/release event, using the winPeekMessage
            // will help us fix this.  This leaves us with a
            // question:
            //    This effectively kills using the mouse AND the
            //    tablet simultaneously, well creates wacky input.
            //    Is this going to be a problem? (probably not)
            bool next_is_button = false;
            bool is_mouse_move = (message == WM_MOUSEMOVE);
            if (is_mouse_move) {
                MSG msg1;
                if (winPeekMessage(&msg1, msg.hwnd, WM_MOUSEFIRST,
                                    WM_MOUSELAST, PM_NOREMOVE))
                    next_is_button = (msg1.message == WM_LBUTTONUP
                                       || msg1.message == WM_LBUTTONDOWN);
            }
            if (!is_mouse_move || (is_mouse_move && !next_is_button))
                chokeMouse = false;
        }
#endif
    } else if (message == WM95_MOUSEWHEEL) {
        result = widget->translateWheelEvent(msg);
    } else {
        switch (message) {
        case WM_KEYDOWN:                        // keyboard event
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_IME_CHAR:
        case WM_IME_KEYDOWN:
        case WM_CHAR: {
            MSG msg1;
            bool anyMsg = winPeekMessage(&msg1, msg.hwnd, 0, 0, PM_NOREMOVE);
            if (anyMsg && msg1.message == WM_DEADCHAR) {
                result = true; // consume event since there is a dead char next
                break;
            }
            QWidget *g = QWidget::keyboardGrabber();
            if (g)
                widget = (QETWidget*)g;
            else if (qApp->focusWidget())
                widget = (QETWidget*)qApp->focusWidget();
            else if (!widget || widget->winId() == GetFocus()) // We faked the message to go to exactly that widget.
                widget = (QETWidget*)widget->topLevelWidget();
            if (widget->isEnabled())
                result = widget->translateKeyEvent(msg, g != 0);
            break;
        }
        case WM_SYSCHAR:
            result = true;                        // consume event
            break;

        case WM_MOUSEWHEEL:
            result = widget->translateWheelEvent(msg);
            break;

        case WM_APPCOMMAND:
            {
                uint cmd = GET_APPCOMMAND_LPARAM(lParam);
                uint uDevice = GET_DEVICE_LPARAM(lParam);
                uint dwKeys = GET_KEYSTATE_LPARAM(lParam);

                int state = translateButtonState(dwKeys, QEvent::KeyPress, 0);

                switch (uDevice) {
                case FAPPCOMMAND_KEY:
                    {
                        int key = 0;

                        switch(cmd) {
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
                        if (key) {
                            bool res = false;
                            QWidget *g = QWidget::keyboardGrabber();
                            if (g)
                                widget = (QETWidget*)g;
                            else if (qApp->focusWidget())
                                widget = (QETWidget*)qApp->focusWidget();
                            else
                                widget = (QETWidget*)widget->topLevelWidget();
                            if (widget->isEnabled())
                                res = ((QETWidget*)widget)->sendKeyEvent(QEvent::KeyPress, key, state, false, QString::null, g != 0);
                            if (res)
                                return true;
                        }
                    }
                    break;

                default:
                    break;
                }

                result = false;
            }
            break;

#ifndef Q_OS_TEMP
        case WM_NCMOUSEMOVE:
            {
                // span the application wide cursor over the
                // non-client area.
                QCursor *c = qt_grab_cursor();
                if (!c)
                    c = QApplication::overrideCursor();
                if (c)        // application cursor defined
                    SetCursor(c->handle());
                else
                    result = false;
                // generate leave event also when the caret enters
                // the non-client area.
                qt_dispatchEnterLeave(0, QWidget::find(curWin));
                curWin = 0;
            }
            break;
#endif

        case WM_SYSCOMMAND: {
#ifndef Q_OS_TEMP
            bool window_state_change = false;
            switch(wParam) {
            case SC_CONTEXTHELP:
#ifndef QT_NO_WHATSTHIS
                QWhatsThis::enterWhatsThisMode();
#endif
                QT_WA({
                    DefWindowProc(hwnd, WM_NCPAINT, 1, 0);
                } , {
                    DefWindowProcA(hwnd, WM_NCPAINT, 1, 0);
                });
                break;
#if defined(QT_NON_COMMERCIAL)
                QT_NC_SYSCOMMAND
#endif
            case SC_MAXIMIZE:
                window_state_change = true;
                widget->clearWState(Qt::WState_Minimized);
                widget->setWState(Qt::WState_Maximized);
                result = false;
                break;
            case SC_MINIMIZE:
                window_state_change = true;
                widget->setWState(Qt::WState_Minimized);
                if (widget->isVisible()) {
                    QHideEvent e;
                    qt_sendSpontaneousEvent(widget, &e);
                    widget->hideChildren(true);
                }
                result = false;
                break;
            case SC_RESTORE:
                window_state_change = true;
                if (widget->isMinimized()) {
                    widget->clearWState(Qt::WState_Minimized);
                    widget->showChildren(true);
                    QShowEvent e;
                    qt_sendSpontaneousEvent(widget, &e);
                } else {
                    widget->clearWState(Qt::WState_Maximized);
                }
                result = false;
                break;
            default:
                result = false;
                break;
            }

            if (window_state_change) {
                QEvent e(QEvent::WindowStateChange);
                qt_sendSpontaneousEvent(widget, &e);
            }
#endif

            break;
        }

        case WM_SETTINGCHANGE:
            if ( qApp->type() == QApplication::Tty )
	        break;

            if (!msg.wParam) {
                QString area = QT_WA_INLINE(QString::fromUtf16((unsigned short *)msg.lParam),
                                             QString::fromLocal8Bit((char*)msg.lParam));
                if (area == "intl")
                    QApplication::postEvent(widget, new QEvent(QEvent::LocaleChange));
            }
            break;

#ifndef Q_OS_TEMP
        case WM_NCLBUTTONDBLCLK:
            if (wParam == HTCAPTION) {
                bool window_state_changed = false;
                if (widget->isMaximized()) {
                    window_state_changed = true;
                    widget->clearWState(Qt::WState_Maximized);
                } else if (widget->testWFlags(Qt::WStyle_Maximize)){
                    window_state_changed = true;
                    widget->setWState(Qt::WState_Maximized);
                }

                if (window_state_changed) {
                    QEvent e(QEvent::WindowStateChange);
                    qt_sendSpontaneousEvent(widget, &e);
                }
            }
            result = false;
            break;
#endif
        case WM_PAINT:                                // paint event
            result = widget->translatePaintEvent(msg);
            break;

        case WM_ERASEBKGND:                        // erase window background
            if (!widget->testAttribute(Qt::WA_PendingUpdate))
                widget->eraseWindowBackground((HDC)wParam);
            RETURN(true);
            break;

        case WM_MOVE:                                // move window
        case WM_SIZE:                                // resize window
            result = widget->translateConfigEvent(msg);
            break;

        case WM_ACTIVATE:
	    if ( qApp->type() == QApplication::Tty )
	        break;

#if defined(QT_TABLET_SUPPORT)
            if (ptrWTOverlap && ptrWTEnable) {
                // cooperate with other tablet applications, but when
                // we get focus, I want to use the tablet...
                if (qt_tablet_context && GET_WM_ACTIVATE_STATE(wParam, lParam)) {
                    if (ptrWTEnable(qt_tablet_context, true))
                        ptrWTOverlap(qt_tablet_context, true);
                }
            }
#endif
            if (QApplication::activePopupWidget() && LOWORD(wParam) == WA_INACTIVE &&
                QWidget::find((HWND)lParam) == 0) {
                // Another application was activated while our popups are open,
                // then close all popups.  In case some popup refuses to close,
                // we give up after 1024 attempts (to avoid an infinite loop).
                int maxiter = 1024;
                QWidget *popup;
                while ((popup=QApplication::activePopupWidget()) && maxiter--)
                    popup->close();
            }

            // Windows tries to activate a modally blocked window.
            // This happens when restoring an application after "Show Desktop"
            if (app_do_modal && LOWORD(wParam) == WA_ACTIVE) {
                QWidget *top = 0;
                if (!qt_tryModalHelper(widget, &top) && top && widget != top) {
                    top->setActiveWindow();
                    break;
                }
            }
	    qApp->winFocus(widget, LOWORD(wParam) != WA_INACTIVE);
	    break;

#ifndef Q_OS_TEMP
            case WM_MOUSEACTIVATE:
                {
                    const QWidget *tlw = widget->topLevelWidget();
                    // Do not change activation if the clicked widget is inside a floating dock window
                    if (tlw->inherits("QDockWindow") && qApp->activeWindow()
                         && !qApp->activeWindow()->inherits("QDockWindow"))
                        RETURN(MA_NOACTIVATE);
                }
                result = false;
                break;
#endif
            case WM_SHOWWINDOW:
#ifndef Q_OS_TEMP
                if (lParam == SW_PARENTOPENING) {
                    if (widget->testWState(Qt::WState_ForceHide))
                        RETURN(0);
                }
#endif
                if  (!wParam && autoCaptureWnd == widget->winId())
                    releaseAutoCapture();
                result = false;
                break;

        case WM_PALETTECHANGED:                        // our window changed palette
            if (QColormap::hPal() && (WId)wParam == widget->winId())
                RETURN(0);                        // otherwise: FALL THROUGH!
            // FALL THROUGH
        case WM_QUERYNEWPALETTE:                // realize own palette
            if (QColormap::hPal()) {
                HDC hdc = GetDC(widget->winId());
                HPALETTE hpalOld = SelectPalette(hdc, QColormap::hPal(), false);
                uint n = RealizePalette(hdc);
                if (n)
                    InvalidateRect(widget->winId(), 0, true);
                SelectPalette(hdc, hpalOld, true);
                RealizePalette(hdc);
                ReleaseDC(widget->winId(), hdc);
                RETURN(n);
            }
            break;
        case WM_CLOSE:                                // close window
            widget->translateCloseEvent(msg);
            RETURN(0);                                // always handled

        case WM_DESTROY:                        // destroy window
            if (hwnd == curWin) {
                QEvent leave(QEvent::Leave);
                QApplication::sendEvent(widget, &leave);
                curWin = 0;
            }
            if (widget == popupButtonFocus)
                popupButtonFocus = 0;
            result = false;
            break;

#ifndef Q_OS_TEMP
        case WM_GETMINMAXINFO:
            if (widget->xtra()) {
                MINMAXINFO *mmi = (MINMAXINFO *)lParam;
                QWExtra           *x = widget->xtra();
                QRect           f  = widget->frameGeometry();
                QSize           s  = widget->size();
                if (x->minw > 0)
                    mmi->ptMinTrackSize.x = x->minw + f.width()         - s.width();
                if (x->minh > 0)
                    mmi->ptMinTrackSize.y = x->minh + f.height() - s.height();
                if (x->maxw < QWIDGETSIZE_MAX)
                    mmi->ptMaxTrackSize.x = x->maxw + f.width()         - s.width();
                if (x->maxh < QWIDGETSIZE_MAX)
                    mmi->ptMaxTrackSize.y = x->maxh + f.height() - s.height();
                RETURN(0);
            }
            break;

            case WM_CONTEXTMENU:
            {
                // it's not VK_APPS or Shift+F10, but a click in the NC area
                if (lParam != (int)0xffffffff) {
                    result = false;
                    break;
                }
                QWidget *fw = qApp->focusWidget();
                if (fw) {
                    QContextMenuEvent e(QContextMenuEvent::Keyboard, QPoint(5, 5), fw->mapToGlobal(QPoint(5, 5)), 0);
                    result = qt_sendSpontaneousEvent(fw, &e);
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
            result = QInputContext::composition(lParam);
            break;

#ifndef Q_OS_TEMP
        case WM_CHANGECBCHAIN:
        case WM_DRAWCLIPBOARD:
        case WM_RENDERFORMAT:
        case WM_RENDERALLFORMATS:
        case WM_DESTROYCLIPBOARD:
            if (qt_clipboard) {
                QCustomEvent e(QEvent::Clipboard, &msg);
                qt_sendSpontaneousEvent(qt_clipboard, &e);
                RETURN(0);
            }
            result = false;
            break;
#endif
#ifndef QT_NO_ACCESSIBILITY
        case WM_GETOBJECT:
            {
                // Ignoring all requests while starting up
                if (qApp->startingUp() || !qApp->loopLevel() || (DWORD)lParam != OBJID_CLIENT) {
                    result = false;
                    break;
                }

                typedef LRESULT (WINAPI *PtrLresultFromObject)(REFIID, WPARAM, LPUNKNOWN);
                static PtrLresultFromObject ptrLresultFromObject = 0;
                static bool oleaccChecked = false;

                if (!oleaccChecked) {
                    oleaccChecked = true;
                    ptrLresultFromObject = (PtrLresultFromObject)QLibrary::resolve("oleacc.dll", "LresultFromObject");
                }
                if (ptrLresultFromObject) {
                    QAccessibleInterface *acc = QAccessible::queryAccessibleInterface(widget);
                    if (!acc) {
                        result = false;
                        break;
                    }

                    // and get an instance of the IAccessibile implementation
                    IAccessible *iface = qt_createWindowsAccessible(acc);
                    LRESULT res = ptrLresultFromObject(IID_IAccessible, wParam, iface);  // ref == 2
                    iface->Release(); // the client will release the object again, and then it will destroy itself

                    if (res > 0)
                        RETURN(res);
                }
            }
            result = false;
            break;
#endif
#if defined(QT_TABLET_SUPPORT)
        case WT_PACKET:
            // Get the packets and also don't link against the actual library...
            if (ptrWTPacketsGet) {
                if ((nPackets = ptrWTPacketsGet(qt_tablet_context, QT_TABLET_NPACKETQSIZE, &localPacketBuf))) {
                    result = widget->translateTabletEvent(msg, localPacketBuf, nPackets);
                }
            }
            break;
        case WT_PROXIMITY:
            // flush the QUEUE
            if (ptrWTPacketsGet)
                ptrWTPacketsGet(qt_tablet_context, QT_TABLET_NPACKETQSIZE + 1, NULL);
            if (chokeMouse)
                chokeMouse = false;
            break;
#endif
        case WM_KILLFOCUS:
            if (!QWidget::find((HWND)wParam)) { // we don't get focus, so unset it now
                if (!widget->hasFocus()) // work around Windows bug after minimizing/restoring
                    widget = (QETWidget*)qApp->focusWidget();
                HWND focus = ::GetFocus();
                if (!widget || (focus && ::IsChild(widget->winId(), focus))) {
                    result = false;
                } else {
                    widget->clearFocus();
                    result = true;
                }
            } else {
                result = false;
            }
            break;

        case WM_THEMECHANGED:
            if (widget->testWFlags(Qt::WType_Desktop) || !qApp || qApp->closingDown()
                                                         || qApp->type() == QApplication::Tty)
                break;

            if (widget->testWState(Qt::WState_Polished))
                qApp->style().unPolish(widget);

            if (widget->testWState(Qt::WState_Polished))
                qApp->style().polish(widget);
            widget->repolishStyle(qApp->style());
            if (widget->isVisible())
                widget->update();
            break;

#ifndef Q_OS_TEMP
        case WM_INPUTLANGCHANGE: {
            char info[7];
            if (!GetLocaleInfoA(MAKELCID(lParam, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, info, 6)) {
                inputcharset = CP_ACP;
            } else {
                inputcharset = QString(info).toInt();
            }
            break;
        }
#else
        case WM_COMMAND:
            result = (wParam == 0x1);
            if (result)
                QApplication::postEvent(widget, new QEvent(QEvent::OkRequest));
            break;
        case WM_HELP:
            QApplication::postEvent(widget, new QEvent(QEvent::HelpRequest));
            result = true;
            break;
#endif

        case WM_MOUSELEAVE:
            // We receive a mouse leave for curWin, meaning
            // the mouse was moved outside our widgets
            if (widget->winId() == curWin) {
                bool dispatch = !widget->underMouse();
                // hasMouse is updated when dispatching enter/leave,
                // so test if it is actually up-to-date
                if (!dispatch) {
                    QRect geom = widget->geometry();
                    if (widget->parentWidget() && !widget->isTopLevel()) {
                        QPoint gp = widget->parentWidget()->mapToGlobal(widget->pos());
                        geom.setX(gp.x());
                        geom.setY(gp.y());
                    }
                    QPoint cpos = QCursor::pos();
                    dispatch = !geom.contains(cpos);
                    if (!dispatch) {
                        HRGN hrgn = CreateRectRgn(0,0,0,0);
                        if (GetWindowRgn(curWin, hrgn) != ERROR) {
                            QPoint lcpos = widget->mapFromGlobal(cpos);
                            dispatch = !PtInRegion(hrgn, lcpos.x(), lcpos.y());
                        }
                        DeleteObject(hrgn);
                    }
                }
                if (dispatch) {
                    qt_dispatchEnterLeave(0, QWidget::find((WId)curWin));
                    curWin = 0;
                }
            }
            break;

        case WM_CANCELMODE:
            if (qApp->focusWidget()) {
                QFocusEvent::setReason(QFocusEvent::ActiveWindow);
                QFocusEvent e(QEvent::FocusOut);
                QApplication::sendEvent(qApp->focusWidget(), &e);
                QFocusEvent::resetReason();
            }
            break;

        default:
            result = false;                        // event was not processed
            break;
        }
    }

    if (evt_type != QEvent::None) {                // simple event
        QEvent e(evt_type);
        result = qt_sendSpontaneousEvent(widget, &e);
    }
    if (result)
        RETURN(false);

do_default:
    RETURN(QInputContext::DefWindowProc(hwnd,message,wParam,lParam))
}


/*****************************************************************************
  Modal widgets; We have implemented our own modal widget mechanism
  to get total control.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
        Enters modal state
        Arguments:
            QWidget *widget        A modal widget

  qt_leave_modal()
        Leaves modal state for a widget
        Arguments:
            QWidget *widget        A modal widget
 *****************************************************************************/

bool qt_modal_state()
{
    return app_do_modal;
}


void Q_GUI_EXPORT qt_enter_modal(QWidget *widget)
{
    if (!qt_modal_stack) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }
    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowBlocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }

    releaseAutoCapture();
    qt_dispatchEnterLeave(0, QWidget::find((WId)curWin));
    qt_modal_stack->insert(0, widget);
    app_do_modal = true;
    curWin = 0;
    qt_button_down = 0;
    qt_win_ignoreNextMouseReleaseEvent = false;
}


void Q_GUI_EXPORT qt_leave_modal(QWidget *widget)
{
    if (qt_modal_stack && qt_modal_stack->removeAll(widget)) {
        if (qt_modal_stack->isEmpty()) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p(QCursor::pos());
            app_do_modal = false; // necessary, we may get recursively into qt_try_modal below
            QWidget* w = QApplication::widgetAt(p.x(), p.y());
            qt_dispatchEnterLeave(w, QWidget::find(curWin)); // send synthetic enter event
            curWin = w? w->winId() : 0;
        }
        qt_win_ignoreNextMouseReleaseEvent = true;
    }
    app_do_modal = qt_modal_stack != 0;

    if (widget->parentWidget()) {
        QEvent e(QEvent::WindowUnblocked);
        QApplication::sendEvent(widget->parentWidget(), &e);
    }
}

static bool qt_blocked_modal(QWidget *widget)
{
    if (!app_do_modal)
        return false;
    if (qApp->activePopupWidget())
        return false;
    if (widget->testWFlags(Qt::WStyle_Tool))        // allow tool windows
        return false;

    QWidget *modal=0, *top=qt_modal_stack->first();

    widget = widget->topLevelWidget();
    if (widget->testWFlags(Qt::WShowModal))        // widget is modal
        modal = widget;
    if (!top || modal == top)                                // don't block event
        return false;
    return true;
}

static bool qt_try_modal(QWidget *widget, MSG *msg, int& ret)
{
    QWidget * top = 0;

    if (qt_tryModalHelper(widget, &top))
        return true;

    int         type  = msg->message;

    bool block_event = false;
#ifndef Q_OS_TEMP
    if (type == WM_NCHITTEST) {
      //block_event = true;
        // QApplication::beep();
    } else
#endif
        if ((type >= WM_MOUSEFIRST && type <= WM_MOUSELAST) ||
             type == WM_MOUSEWHEEL || type == (int)WM95_MOUSEWHEEL ||
             type == WM_MOUSELEAVE ||
             (type >= WM_KEYFIRST        && type <= WM_KEYLAST)
#ifndef Q_OS_TEMP
                        || type == WM_NCMOUSEMOVE
#endif
               ) {
      if (type == WM_MOUSEMOVE
#ifndef Q_OS_TEMP
                        || type == WM_NCMOUSEMOVE
#endif
                       ) {
        QCursor *c = qt_grab_cursor();
        if (!c)
            c = QApplication::overrideCursor();
        if (c)                                // application cursor defined
            SetCursor(c->handle());
        else
            SetCursor(QCursor(Qt::ArrowCursor).handle());
      }
      block_event = true;
    } else if (type == WM_CLOSE) {
        block_event = true;
    }
#ifndef Q_OS_TEMP
    else if (type == WM_MOUSEACTIVATE || type == WM_NCLBUTTONDOWN){
        if (!top->isActiveWindow()) {
            top->setActiveWindow();
        } else {
            QApplication::beep();
        }
        block_event = true;
        ret = MA_NOACTIVATEANDEAT;
    } else if (type == WM_SYSCOMMAND) {
        if (!(msg->wParam == SC_RESTORE && widget->isMinimized()))
            block_event = true;
    }
#endif

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  openPopup()
        Adds a widget to the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be added

  closePopup()
        Removes a widget from the list of popup widgets
        Arguments:
            QWidget *widget        The popup widget to be removed
 *****************************************************************************/

void QApplication::openPopup(QWidget *popup)
{
    if (!popupWidgets) {                        // create list
        popupWidgets = new QWidgetList;
    }
    popupWidgets->append(popup);                // add to end of list
    if (!popup->isEnabled())
        return;

    if (popupWidgets->count() == 1 && !qt_nograb())
        setAutoCapture(popup->winId());        // grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason(QFocusEvent::Popup);
    if (popup->focusWidget())
        popup->focusWidget()->setFocus();
    else
        popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup(QWidget *popup)
{
    if (!popupWidgets)
        return;
    popupWidgets->removeAll(popup);
    POINT curPos;
    GetCursorPos(&curPos);
    replayPopupMouseEvent = (!popup->geometry().contains(QPoint(curPos.x, curPos.y))
                             && !popup->testAttribute(Qt::WA_NoMouseReplay));

    if (popupWidgets->count() == 0) {                // this was the last popup
        delete popupWidgets;
        popupWidgets = 0;
        if (!popup->isEnabled())
            return;
        if (!qt_nograb())                        // grabbing not disabled
            releaseAutoCapture();
        if (active_window) {
            QFocusEvent::setReason(QFocusEvent::Popup);
            if (active_window->focusWidget())
                active_window->focusWidget()->setFocus();
            else
                active_window->setFocus();
            QFocusEvent::resetReason();
        }
    } else {
        // Popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QFocusEvent::setReason(QFocusEvent::Popup);
        QWidget* aw = popupWidgets->last();
        if (popupWidgets->count() == 1)
            setAutoCapture(aw->winId());
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

static void setAutoCapture(HWND h)
{
    if (autoCaptureWnd)
        releaseAutoCapture();
    autoCaptureWnd = h;
    SetCapture(h);
}

static void releaseAutoCapture()
{
    if (autoCaptureWnd) {
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
    WM_MOUSEMOVE,        QEvent::MouseMove,                0,
    WM_LBUTTONDOWN,        QEvent::MouseButtonPress,        Qt::LeftButton,
    WM_LBUTTONUP,        QEvent::MouseButtonRelease,        Qt::LeftButton,
    WM_LBUTTONDBLCLK,        QEvent::MouseButtonDblClick,        Qt::LeftButton,
    WM_RBUTTONDOWN,        QEvent::MouseButtonPress,        Qt::RightButton,
    WM_RBUTTONUP,        QEvent::MouseButtonRelease,        Qt::RightButton,
    WM_RBUTTONDBLCLK,        QEvent::MouseButtonDblClick,        Qt::RightButton,
    WM_MBUTTONDOWN,        QEvent::MouseButtonPress,        Qt::MidButton,
    WM_MBUTTONUP,        QEvent::MouseButtonRelease,        Qt::MidButton,
    WM_MBUTTONDBLCLK,        QEvent::MouseButtonDblClick,        Qt::MidButton,
    WM_XBUTTONDOWN,        QEvent::MouseButtonPress,        Qt::MidButton*2, //### Qt::XButton1/2
    WM_XBUTTONUP,        QEvent::MouseButtonRelease,        Qt::MidButton*2,
    WM_XBUTTONDBLCLK,        QEvent::MouseButtonDblClick,        Qt::MidButton*2,
    0,                        0,                                0
};

static int translateButtonState(int s, int type, int button)
{
    int bst = 0;
    if (s & MK_LBUTTON)
        bst |= Qt::LeftButton;
    if (s & MK_MBUTTON)
        bst |= Qt::MidButton;
    if (s & MK_RBUTTON)
        bst |= Qt::RightButton;
    if (s & MK_SHIFT)
        bst |= Qt::ShiftButton;
    if (s & MK_CONTROL)
        bst |= Qt::ControlButton;

    if (s & MK_XBUTTON1)
        bst |= Qt::MidButton*2;//### Qt::XButton1;
    if (s & MK_XBUTTON2)
        bst |= Qt::MidButton*4;//### Qt::XButton2;

    if (GetKeyState(VK_MENU) < 0)
        bst |= Qt::AltButton;

    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        bst |= Qt::MetaButton;

    // Translate from Windows-style "state after event"
    // to X-style "state before event"
    if (type == QEvent::MouseButtonPress ||
         type == QEvent::MouseButtonDblClick)
        bst &= ~button;
    else if (type == QEvent::MouseButtonRelease)
        bst |= button;

    return bst;
}

void qt_win_eatMouseMove()
{
    // after closing a windows dialog with a double click (i.e. open a file)
    // the message queue still contains a dubious WM_MOUSEMOVE message where
    // the left button is reported to be down (wParam != 0).
    // remove all those messages (usually 1) and post the last one with a
    // reset button state

    MSG msg = {0, 0, 0, 0, 0, 0, 0};
    QT_WA( {
        while (PeekMessage(&msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
            ;
        if (msg.message == WM_MOUSEMOVE)
            PostMessage(msg.hwnd, msg.message, 0, msg.lParam);
    }, {
        MSG msg;
        msg.message = 0;
        while (PeekMessageA(&msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
            ;
        if (msg.message == WM_MOUSEMOVE)
            PostMessageA(msg.hwnd, msg.message, 0, msg.lParam);
    } );
}

// In DnD, the mouse release event never appears, so the
// mouse button state machine must be manually reset
/*! \internal */
void QApplication::winMouseButtonUp()
{
    qt_button_down = 0;
    releaseAutoCapture();
}

bool QETWidget::translateMouseEvent(const MSG &msg)
{
    static QPoint pos;
    static POINT gpos={-1,-1};
    QEvent::Type type;                                // event parameters
    int           button;
    int           state;
    int           i;

    if (sm_blockUserInput) //block user interaction during session management
        return true;

    // Compress mouse move events
    if (msg.message == WM_MOUSEMOVE) {
        MSG mouseMsg;
        while (winPeekMessage(&mouseMsg, msg.hwnd, WM_MOUSEFIRST,
                WM_MOUSELAST, PM_NOREMOVE)) {
            if (mouseMsg.message == WM_MOUSEMOVE) {
#define PEEKMESSAGE_IS_BROKEN 1
#ifdef PEEKMESSAGE_IS_BROKEN
                // Since the Windows PeekMessage() function doesn't
                // correctly return the wParam for WM_MOUSEMOVE events
                // if there is a key release event in the queue
                // _before_ the mouse event, we have to also consider
                // key release events (kls 2003-05-13):
                MSG keyMsg;
                bool done = false;
                while (winPeekMessage(&keyMsg, 0, WM_KEYFIRST, WM_KEYLAST,
                        PM_NOREMOVE)) {
                    if (keyMsg.time < mouseMsg.time) {
                        if ((keyMsg.lParam & 0xC0000000) == 0x40000000) {
                            winPeekMessage(&keyMsg, 0, keyMsg.message,
                                            keyMsg.message, PM_REMOVE);
                        } else {
                            done = true;
                            break;
                        }
                    } else {
                        break; // no key event before the WM_MOUSEMOVE event
                    }
                }
                if (done)
                    break;
#else
                // Actually the following 'if' should work instead of
                // the above key event checking, but apparently
                // PeekMessage() is broken :-(
                if (mouseMsg.wParam != msg.wParam)
                    break; // leave the message in the queue because
                           // the key state has changed
#endif
                MSG *msgPtr = (MSG *)(&msg);
                // Update the passed in MSG structure with the
                // most recent one.
                msgPtr->lParam = mouseMsg.lParam;
                msgPtr->wParam = mouseMsg.wParam;
                msgPtr->pt = mouseMsg.pt;
                // Remove the mouse move message
                winPeekMessage(&mouseMsg, msg.hwnd, WM_MOUSEMOVE,
                                WM_MOUSEMOVE, PM_REMOVE);
            } else {
                break; // there was no more WM_MOUSEMOVE event
            }
        }
    }


    for (i=0; (UINT)mouseTbl[i] != msg.message || !mouseTbl[i]; i += 3)
        ;
    if (!mouseTbl[i])
        return false;
    type   = (QEvent::Type)mouseTbl[++i];        // event type
    button = mouseTbl[++i];                        // which button
    if (button > Qt::MidButton) {
        switch(GET_XBUTTON_WPARAM(msg.wParam)) {
        case XBUTTON1:
            button = Qt::MidButton*2; //### XButton1;
            break;
        case XBUTTON2:
            button = Qt::MidButton*4; //### XButton2;
            break;
        }
    }
    state  = translateButtonState(msg.wParam, type, button); // button state
    if (type == QEvent::MouseMove) {
        if (!(state & Qt::MouseButtonMask))
            qt_button_down = 0;
        QCursor *c = qt_grab_cursor();
        if (!c)
            c = QApplication::overrideCursor();
        if (c)                                // application cursor defined
            SetCursor(c->handle());
        else {
            QWidget *w = this; // use  widget cursor if widget is enabled
            while (!w->isTopLevel() && !w->isEnabled())
                w = w->parentWidget();
            SetCursor(w->cursor().handle());
        }
        if (curWin != winId()) {                // new current window
            qt_dispatchEnterLeave(this, QWidget::find(curWin));
            curWin = winId();
#ifndef Q_OS_TEMP
            static bool trackMouseEventLookup = false;
            typedef BOOL (WINAPI *PtrTrackMouseEvent)(LPTRACKMOUSEEVENT);
            static PtrTrackMouseEvent ptrTrackMouseEvent = 0;
            if (!trackMouseEventLookup) {
                trackMouseEventLookup = true;
                ptrTrackMouseEvent = (PtrTrackMouseEvent)QLibrary::resolve("comctl32", "_TrackMouseEvent");
            }
            if (ptrTrackMouseEvent && !qApp->inPopupMode()) {
                // We always have to set the tracking, since
                // Windows detects more leaves than we do..
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = 0x00000002;    // TME_LEAVE
                tme.hwndTrack = curWin;      // Track on window receiving msgs
                tme.dwHoverTime = (DWORD)-1; // HOVER_DEFAULT
                ptrTrackMouseEvent(&tme);
            }
#endif // Q_OS_TEMP
        }

        POINT curPos = msg.pt;
        if (curPos.x == gpos.x && curPos.y == gpos.y)
            return true;                        // same global position
        gpos = curPos;

        ScreenToClient(winId(), &curPos);

        pos.rx() = curPos.x;
        pos.ry() = curPos.y;
        pos = d->mapFromWS(pos);
    } else {
        gpos = msg.pt;
        pos = mapFromGlobal(QPoint(gpos.x, gpos.y));

        if (type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick) {        // mouse button pressed
            // Magic for masked widgets
            qt_button_down = findChildWidget(this, pos);
            if (!qt_button_down || !qt_button_down->testWFlags(Qt::WMouseNoMask))
                qt_button_down = this;
        }
    }

    if (qApp->inPopupMode()) {                        // in popup mode
        replayPopupMouseEvent = false;
        QWidget* activePopupWidget = qApp->activePopupWidget();
        QWidget *popup = activePopupWidget;

        if (popup != this) {
            if (testWFlags(Qt::WType_Popup) && rect().contains(pos))
                popup = this;
            else                                // send to last popup
                pos = popup->mapFromGlobal(QPoint(gpos.x, gpos.y));
        }
        QWidget *popupChild = findChildWidget(popup, pos);
        bool releaseAfter = false;
        switch (type) {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonDblClick:
                popupButtonFocus = popupChild;
                break;
            case QEvent::MouseButtonRelease:
                releaseAfter = true;
                break;
            default:
                break;                                // nothing for mouse move
        }

        if (popupButtonFocus)
            popup = popupButtonFocus;
        else if (popupChild)
            popup = popupChild;

        QPoint globalPos(gpos.x, gpos.y);
        pos = popup->mapFromGlobal(globalPos);
	QMouseEvent e(type, pos, globalPos, button, state);
	QApplication::sendSpontaneousEvent(popup, &e);

        if (releaseAfter) {
            popupButtonFocus = 0;
            qt_button_down = 0;
        }

        if (type == QEvent::MouseButtonPress
             && qApp->activePopupWidget() != activePopupWidget
             && replayPopupMouseEvent) {
            // the popup dissappeared. Replay the event
            QWidget* w = QApplication::widgetAt(gpos.x, gpos.y);
            if (w && !qt_blocked_modal(w)) {
                if (QWidget::mouseGrabber() == 0)
                    setAutoCapture(w->winId());
                POINT widgetpt = gpos;
                ScreenToClient(w->winId(), &widgetpt);
                LPARAM lParam = MAKELPARAM(widgetpt.x, widgetpt.y);
                winPostMessage(w->winId(), msg.message, msg.wParam, lParam);
            }
         } else if (type == QEvent::MouseButtonRelease && button == Qt::RightButton
                   && qApp->activePopupWidget() == activePopupWidget) {
            // popup still alive and received right-button-release
	    QContextMenuEvent e2( QContextMenuEvent::Mouse, pos, globalPos, state );
	    QApplication::sendSpontaneousEvent( popup, &e2 );
        }
    } else {                                        // not popup mode
        int bs = state & Qt::MouseButtonMask;
        if ((type == QEvent::MouseButtonPress ||
              type == QEvent::MouseButtonDblClick) && bs == 0) {
            if (QWidget::mouseGrabber() == 0)
                setAutoCapture(winId());
        } else if (type == QEvent::MouseButtonRelease && bs == button) {
            if (QWidget::mouseGrabber() == 0)
                releaseAutoCapture();
        }

        QWidget *widget = this;
        QWidget *w = QWidget::mouseGrabber();
        if (!w)
            w = qt_button_down;
        if (w && w != this) {
            widget = w;
            pos = w->mapFromGlobal(QPoint(gpos.x, gpos.y));
        }

        if (type == QEvent::MouseButtonRelease &&
             (state & (~button) & (Qt::MouseButtonMask)) == 0) {
            qt_button_down = 0;
        }

        QMouseEvent e(type, pos, QPoint(gpos.x,gpos.y), button, state);
        QApplication::sendSpontaneousEvent(widget, &e);
        if (type == QEvent::MouseButtonRelease && button == Qt::RightButton) {
            QContextMenuEvent e2(QContextMenuEvent::Mouse, pos, QPoint(gpos.x,gpos.y), state);
            QApplication::sendSpontaneousEvent(widget, &e2);
        }

        if (type != QEvent::MouseMove)
            pos.rx() = pos.ry() = -9999;        // init for move compression
    }
    return true;
}


//
// Keyboard event translation
//

static const uint KeyTbl[] = {                // keyboard mapping table
    VK_ESCAPE,                Qt::Key_Escape,                // misc keys
    VK_TAB,                Qt::Key_Tab,
    VK_BACK,                Qt::Key_Backspace,
    VK_RETURN,                Qt::Key_Return,
    VK_INSERT,                Qt::Key_Insert,
    VK_DELETE,                Qt::Key_Delete,
    VK_CLEAR,                Qt::Key_Clear,
    VK_PAUSE,                Qt::Key_Pause,
    VK_SNAPSHOT,        Qt::Key_Print,
    VK_HOME,                Qt::Key_Home,                // cursor movement
    VK_END,                Qt::Key_End,
    VK_LEFT,                Qt::Key_Left,
    VK_UP,                Qt::Key_Up,
    VK_RIGHT,                Qt::Key_Right,
    VK_DOWN,                Qt::Key_Down,
    VK_PRIOR,                Qt::Key_Prior,
    VK_NEXT,                Qt::Key_Next,
    VK_SHIFT,                Qt::Key_Shift,                // modifiers
    VK_CONTROL,                Qt::Key_Control,
    VK_LWIN,                Qt::Key_Meta,
    VK_RWIN,                Qt::Key_Meta,
    VK_MENU,                Qt::Key_Alt,
    VK_CAPITAL,                Qt::Key_CapsLock,
    VK_NUMLOCK,                Qt::Key_NumLock,
    VK_SCROLL,                Qt::Key_ScrollLock,
    VK_NUMPAD0,                Qt::Key_0,                        // numeric keypad
    VK_NUMPAD1,                Qt::Key_1,
    VK_NUMPAD2,                Qt::Key_2,
    VK_NUMPAD3,                Qt::Key_3,
    VK_NUMPAD4,                Qt::Key_4,
    VK_NUMPAD5,                Qt::Key_5,
    VK_NUMPAD6,                Qt::Key_6,
    VK_NUMPAD7,                Qt::Key_7,
    VK_NUMPAD8,                Qt::Key_8,
    VK_NUMPAD9,                Qt::Key_9,
    VK_MULTIPLY,        Qt::Key_Asterisk,
    VK_ADD,                Qt::Key_Plus,
    VK_SEPARATOR,        Qt::Key_Comma,
    VK_SUBTRACT,        Qt::Key_Minus,
    VK_DECIMAL,                Qt::Key_Period,
    VK_DIVIDE,                Qt::Key_Slash,
    VK_APPS,                Qt::Key_Menu,
    0,                        0
};

static int translateKeyCode(int key)                // get Qt::Key_... code
{
    int code;
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        code = 0;
    } else if (key >= VK_F1 && key <= VK_F24) {
        code = Qt::Key_F1 + (key - VK_F1);                // function keys
    } else {
        int i = 0;                                // any other keys
        code = 0;
        while (KeyTbl[i]) {
            if (key == (int)KeyTbl[i]) {
                code = KeyTbl[i+1];
                break;
            }
            i += 2;
        }
    }
    return code;
}

Q_GUI_EXPORT int qt_translateKeyCode(int key)
{
    return translateKeyCode(key);
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

static KeyRec* find_key_rec(int code, bool remove)
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

static void store_key_rec(int code, int ascii, const QString& text)
{
    if (nrecs == maxrecs) {
        qWarning("Qt: Internal keyboard buffer overflow");
        return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,text);
}

static int asciiToKeycode(char a, int state)
{
    if (a >= 'a' && a <= 'z')
        a = toupper(a);
    if ((state & Qt::ControlButton) != 0) {
        if ( a >= 0 && a <= 31 )      // Ctrl+@..Ctrl+A..CTRL+Z..Ctrl+_
        a += '@';                     // to @..A..Z.._
    }

    return a & 0xff;
}

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[2];
        mb[0] = c&0xff;
        mb[1] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(inputcharset, MB_PRECOMPOSED, mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[3];
        mb[0] = (c>>8)&0xff;
        mb[1] = c&0xff;
        mb[2] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
            mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

bool QETWidget::translateKeyEvent(const MSG &msg, bool grab)
{
    bool k0=false, k1=false;
    int  state = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (GetKeyState(VK_SHIFT) < 0)
        state |= Qt::ShiftButton;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= Qt::ControlButton;
    if (GetKeyState(VK_MENU) < 0)
        state |= Qt::AltButton;
    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        state |= Qt::MetaButton;

    if (msg.message == WM_CHAR) {
        // a multi-character key not found by our look-ahead
        QString s;
        QChar ch = wmchar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        k0 = sendKeyEvent(QEvent::KeyPress, 0, state, grab, s);
        k1 = sendKeyEvent(QEvent::KeyRelease, 0, state, grab, s);
    }
    else if (msg.message == WM_IME_CHAR) {
        // input method characters not found by our look-ahead
        QString s;
        QChar ch = imechar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        k0 = sendKeyEvent(QEvent::KeyPress, 0, state, grab, s);
        k1 = sendKeyEvent(QEvent::KeyRelease, 0, state, grab, s);
    } else {
        extern bool qt_use_rtl_extensions;
        if (qt_use_rtl_extensions) {
            // for Directionality changes (BiDi)
            static int dirStatus = 0;
            if (!dirStatus && state == Qt::ControlButton && msg.wParam == VK_CONTROL && msg.message == WM_KEYDOWN) {
                if (GetKeyState(VK_LCONTROL) < 0) {
                    dirStatus = VK_LCONTROL;
                } else if (GetKeyState(VK_RCONTROL) < 0) {
                    dirStatus = VK_RCONTROL;
                }
            } else if (dirStatus) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.wParam == VK_SHIFT) {
                        if (dirStatus == VK_LCONTROL && GetKeyState(VK_LSHIFT) < 0) {
                                dirStatus = VK_LSHIFT;
                        } else if (dirStatus == VK_RCONTROL && GetKeyState(VK_RSHIFT) < 0) {
                            dirStatus = VK_RSHIFT;
                        }
                    } else {
                        dirStatus = 0;
                    }
                } else if (msg.message == WM_KEYUP) {
                    if (dirStatus == VK_LSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_LCONTROL)  ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_LSHIFT))) {
                        k0 = sendKeyEvent(QEvent::KeyPress, Qt::Key_Direction_L, 0, grab, QString::null);
                        k1 = sendKeyEvent(QEvent::KeyRelease, Qt::Key_Direction_L, 0, grab, QString::null);
                        dirStatus = 0;
                    } else if (dirStatus == VK_RSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_RCONTROL) ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_RSHIFT))) {
                        k0 = sendKeyEvent(QEvent::KeyPress, Qt::Key_Direction_R, 0, grab, QString::null);
                        k1 = sendKeyEvent(QEvent::KeyRelease, Qt::Key_Direction_R, 0, grab, QString::null);
                        dirStatus = 0;
                    } else {
                        dirStatus = 0;
                    }
                } else {
                    dirStatus = 0;
                }
            }
        }

        int code = translateKeyCode(msg.wParam);
        // Invert state logic
        if (code == Qt::Key_Alt)
            state = state^Qt::AltButton;
        else if (code == Qt::Key_Control)
            state = state^Qt::ControlButton;
        else if (code == Qt::Key_Shift)
            state = state^Qt::ShiftButton;

        // If the bit 24 of lParm is set you received a enter,
        // otherwise a Return. (This is the extended key bit)
        if ((code == Qt::Key_Return) && (msg.lParam & 0x1000000)) {
            code = Qt::Key_Enter;
        }

        if (!(msg.lParam & 0x1000000)) {        // All cursor keys without extended bit
            switch (code) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_Insert:
            case Qt::Key_Delete:
            case Qt::Key_Asterisk:
            case Qt::Key_Plus:
            case Qt::Key_Minus:
            case Qt::Key_Period:
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                state |= Qt::Keypad;
            default:
                if ((uint)msg.lParam == 0x004c0001 ||
                     (uint)msg.lParam == 0xc04c0001)
                    state |= Qt::Keypad;
                break;
            }
        } else {                                // And some with extended bit
            switch (code) {
            case Qt::Key_Enter:
            case Qt::Key_Slash:
            case Qt::Key_NumLock:
                state |= Qt::Keypad;
            default:
                break;
            }
        }

        int t = msg.message;
        if (t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN) {
            // KEYDOWN
            KeyRec* rec = find_key_rec(msg.wParam, false);
            // Find uch
            QChar uch;
            MSG wm_char;
            UINT charType = (t == WM_KEYDOWN ? WM_CHAR :
                              t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR);
            if (winPeekMessage(&wm_char, 0, charType, charType, PM_REMOVE)) {
                // Found a XXX_CHAR
                uch = charType == WM_IME_CHAR
                        ? imechar_to_unicode(wm_char.wParam)
                        : wmchar_to_unicode(wm_char.wParam);
                if (t == WM_SYSKEYDOWN &&
                     uch.isLetter() && (msg.lParam & KF_ALTDOWN)) {
                    // (See doc of WM_SYSCHAR)
                    uch = uch.toLower(); //Alt-letter
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }
            if (uch.isNull()) {
                // No XXX_CHAR; deduce uch from XXX_KEYDOWN params
                if (msg.wParam == VK_DELETE)
                    uch = QChar((char)0x7f); // Windows doesn't know this one.
                else {
                    if (t != WM_SYSKEYDOWN || !code) {
                        UINT map;
                        QT_WA({
                            map = MapVirtualKey(msg.wParam, 2);
                        } , {
                            map = MapVirtualKeyA(msg.wParam, 2);
                            // High-order bit is 0x8000 on '95
                            if (map & 0x8000)
                                map = (map^0x8000)|0x80000000;
                        });
                        // If the high bit of the return value of
                        // MapVirtualKey is set, the key is a deadkey.
                        if (!(map & 0x80000000)) {
                            uch = wmchar_to_unicode((DWORD)map);
                        }
                    }
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }

            if (state == Qt::AltButton) {
                // Special handling of global Windows hotkeys
                switch (code) {
                case Qt::Key_Escape:
                case Qt::Key_Tab:
                case Qt::Key_Enter:
                case Qt::Key_F4:
                    return false;                // Send the event on to Windows
                case Qt::Key_Space:
                    // do not pass this key to windows, we will process it ourselves
                    qt_show_system_menu(topLevelWidget());
                    return true;
                default:
                    break;
                }
            }

            // map shift+tab to shift+backtab, QShortcutMap knows about it
            // and will handle it
            if (code == Qt::Key_Tab && (state & Qt::ShiftButton) == Qt::ShiftButton)
                code = Qt::Key_BackTab;

            if (rec) {
                // it is already down (so it is auto-repeating)
                if (code < Qt::Key_Shift || code > Qt::Key_ScrollLock) {
                    k0 = sendKeyEvent(QEvent::KeyRelease, code, state, grab, rec->text, true);
                    k1 = sendKeyEvent(QEvent::KeyPress, code, state, grab, rec->text, true);
                }
            } else {
                QString text;
                if (!uch.isNull())
                    text += uch;
                char a = uch.row() ? 0 : uch.cell();
                store_key_rec(msg.wParam, a, text);
                k0 = sendKeyEvent(QEvent::KeyPress, code, state, grab, text);
            }

        } else {
            // Must be KEYUP
            KeyRec* rec = find_key_rec(msg.wParam, true);
            if (!rec) {
                // Someone ate the key down event
            } else {
                if (!code)
                    code = asciiToKeycode(rec->ascii ? rec->ascii : msg.wParam,
                                state);

                // see comment above
                if (code == Qt::Key_Tab && (state & Qt::ShiftButton) == Qt::ShiftButton)
                    code = Qt::Key_BackTab;

                k0 = sendKeyEvent(QEvent::KeyRelease, code, state, grab, rec->text);
                if (code == Qt::Key_Alt)
                    k0 = true; // don't let window see the meta key
            }
        }
    }

    return k0 || k1;
}


bool QETWidget::translateWheelEvent(const MSG &msg)
{
    int  state = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (GetKeyState(VK_SHIFT) < 0)
        state |= Qt::ShiftButton;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= Qt::ControlButton;
    if (GetKeyState(VK_MENU) < 0)
        state |= Qt::AltButton;
    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        state |= Qt::MetaButton;

    int delta;
    if (msg.message == WM_MOUSEWHEEL)
        delta = (short) HIWORD (msg.wParam);
    else
        delta = (int) msg.wParam;

    Qt::Orientation orient = (state&Qt::AltButton
#if 0 // disabled for now - Trenton's one-wheel mouse makes trouble...
    // "delta" for usual wheels is +-120. +-240 seems to indicate the second wheel
    // see more recent MSDN for WM_MOUSEWHEEL

         || delta == 240 || delta == -240)?Qt::Horizontal:Vertical;
    if (delta == 240 || delta == -240)
        delta /= 2;
#endif
       ) ? Qt::Horizontal : Qt::Vertical;

    QPoint globalPos;

    globalPos.rx() = (short)LOWORD (msg.lParam);
    globalPos.ry() = (short)HIWORD (msg.lParam);


    // if there is a widget under the mouse and it is not shadowed
    // by modality, we send the event to it first
    int ret = 0;
    QWidget* w = QApplication::widgetAt(globalPos);
    if (!w || !qt_try_modal(w, (MSG*)&msg, ret))
        w = this;

    // send the event to the widget or its ancestors
    {
        QWidget* popup = qApp->activePopupWidget();
        if (popup && w->topLevelWidget() != popup)
            popup->close();
        QWheelEvent e(w->mapFromGlobal(globalPos), globalPos, delta, state, orient);
        if (QApplication::sendSpontaneousEvent(w, &e))
            return true;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if (w != qApp->focusWidget() && (w = qApp->focusWidget())) {
        QWidget* popup = qApp->activePopupWidget();
        if (popup && w->topLevelWidget() != popup)
            popup->close();
        QWheelEvent e(w->mapFromGlobal(globalPos), globalPos, delta, state, orient);
        if (QApplication::sendSpontaneousEvent(w, &e))
            return true;
    }
    return false;
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
static void prsInit(HCTX hTab)
{
    /* browse WinTab's many info items to discover pressure handling. */
    if (ptrWTInfo && ptrWTGet) {
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
        prsInit(hTab);
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


bool QETWidget::translateTabletEvent(const MSG &msg, PACKET *localPacketBuf,
                                      int numPackets)
{
    POINT ptNew;
    static DWORD btnNew, btnOld, btnChange;
    UINT prsNew;
    ORIENTATION ort;
    static bool button_pressed = false;
    int i,
        dev,
        tiltX,
        tiltY;
    bool sendEvent;
    QEvent::Type t;


    // the most typical event that we get...
    t = QEvent::TabletMove;
    for (i = 0; i < numPackets; i++) {
        if (localPacketBuf[i].pkCursor == 2) {
            dev = QTabletEvent::Eraser;
        } else if (localPacketBuf[i].pkCursor == 1){
            dev = QTabletEvent::Stylus;
        } else {
            dev = QTabletEvent::NoDevice;
        }
        btnOld = btnNew;
        btnNew = localPacketBuf[i].pkButtons;
        btnChange = btnOld ^ btnNew;

        if (btnNew & btnChange) {
            button_pressed = true;
            t = QEvent::TabletPress;
        }
        ptNew.x = (UINT)localPacketBuf[i].pkX;
        ptNew.y = (UINT)localPacketBuf[i].pkY;
        prsNew = 0;
        if (btnNew) {
            prsNew = prsAdjust(localPacketBuf[i], qt_tablet_context);
        } else if (button_pressed) {
            // One button press, should only give one button release
            t = QEvent::TabletRelease;
            button_pressed = false;
        }
        QPoint globalPos(ptNew.x, ptNew.y);

        // make sure the tablet event get's sent to the proper widget...
        QWidget *w = QApplication::widgetAt(globalPos, true);
        if (w == NULL)
            w = this;
        QPoint localPos = w->mapFromGlobal(globalPos);
        if (!qt_tablet_tilt_support)
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
            double radAzim = (ort.orAzimuth / 10) * (PI / 180);
            //double radAlt = abs(ort.orAltitude / 10) * (PI / 180);
            double tanAlt = tan((abs(ort.orAltitude / 10)) * (PI / 180));

            double degX = atan(sin(radAzim) / tanAlt);
            double degY = atan(cos(radAzim) / tanAlt);
            tiltX = degX * (180 / PI);
            tiltY = -degY * (180 / PI);
        }
        // get the unique ID of the device...
        int csr_type,
            csr_physid;
        ptrWTInfo(WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_TYPE, &csr_type);
        ptrWTInfo(WTI_CURSORS + localPacketBuf[i].pkCursor, CSR_PHYSID, &csr_physid);
        QPair<int,int> llId(csr_type, csr_physid);
        QTabletEvent e(t, localPos, globalPos, dev, prsNew, tiltX, tiltY, llId);
        sendEvent = QApplication::sendSpontaneousEvent(w, &e);
    }
    return sendEvent;
}

extern bool qt_is_gui_used;
static void initWinTabFunctions()
{
    if (!qt_is_gui_used)
        return;
    QLibrary library("wintab32");
    library.setAutoUnload(false);
    QT_WA({
        ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoW");
        ptrWTGet = (PtrWTGet)library.resolve("WTGetW");
    } , {
        ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoA");
        ptrWTGet = (PtrWTGet)library.resolve("WTGetA");
    });

    ptrWTEnable = (PtrWTEnable)library.resolve("WTEnable");
    ptrWTOverlap = (PtrWTEnable)library.resolve("WTOverlap");
    ptrWTPacketsGet = (PtrWTPacketsGet)library.resolve("WTPacketsGet");
}
#endif

static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent(QEvent::Type type, int code,
                              int state, bool grab, const QString& text,
                              bool autor)
{
#if !defined QT_NO_COMPAT && !defined(QT_NO_ACCEL)
    if (type == QEvent::KeyPress && !grab
        && static_cast<QApplicationPrivate*>(qApp->d_ptr)->use_compat()) {
        // send accel events if the keyboard is not grabbed
        QKeyEvent a(type, code, state, text, autor, qMax(1, int(text.length())));
        if (static_cast<QApplicationPrivate*>(qApp->d_ptr)->qt_tryAccelEvent(this, &a))
            return true;
    }
#endif
    if (!isEnabled())
        return false;
    QKeyEvent e(type, code, state, text, autor, qMax(1, int(text.length())));
    QApplication::sendSpontaneousEvent(this, &e);
    if (!isModifierKey(code) && state == Qt::AltButton
         && ((code>=Qt::Key_A && code<=Qt::Key_Z) || (code>=Qt::Key_0 && code<=Qt::Key_9))
         && type == QEvent::KeyPress && !e.isAccepted())
        QApplication::beep();  // emulate windows behavioar
    return e.isAccepted();
}

//
// Paint event translation
//
bool QETWidget::translatePaintEvent(const MSG &)
{
    QRegion rgn(0, 0, 1, 1);
    int res = GetUpdateRgn(winId(), rgn.handle(), false);
    if (!GetUpdateRect(winId(), 0, false)  // The update bounding rect is invalid
         || (res == ERROR)
         || (res == NULLREGION)) {
        d->hd = 0;
        return false;
    }

    PAINTSTRUCT ps;
    d->hd = BeginPaint(winId(), &ps);

    // Mapping region from system to qt (32 bit) coordinate system.
    rgn.translate(data->wrect.topLeft());
    repaint(rgn);

    d->hd = 0;
    EndPaint(winId(), &ps);
    return true;
}

//
// Window move and resize (configure) events
//

bool QETWidget::translateConfigEvent(const MSG &msg)
{
    if (!testWState(Qt::WState_Created))                // in QWidget::create()
        return true;
    if (testWState(Qt::WState_ConfigPending))
        return true;
    if (!isTopLevel())
        return true;
    setWState(Qt::WState_ConfigPending);                // set config flag
    QRect cr = geometry();
    if (msg.message == WM_SIZE) {                // resize event
        WORD a = LOWORD(msg.lParam);
        WORD b = HIWORD(msg.lParam);
        QSize oldSize = size();
        QSize newSize(a, b);
        cr.setSize(newSize);
        if (msg.wParam != SIZE_MINIMIZED)
            data->crect = cr;
        if (isTopLevel()) {                        // update caption/icon text
            d->createTLExtra();
            QString txt;
#ifndef Q_OS_TEMP
            if (IsIconic(winId()) && windowIconText().size())
                txt = windowIconText();
            else
#endif
                if (!windowTitle().isNull())
                    txt = windowTitle();
            if(isWindowModified())
                txt += " *";
            if (txt.size()) {
                QT_WA({
                    SetWindowText(winId(), (TCHAR*)txt.utf16());
                } , {
                    SetWindowTextA(winId(), txt.local8Bit());
                });
            }
        }
        if (msg.wParam != SIZE_MINIMIZED && oldSize != newSize) {
            if (isVisible()) {
                QResizeEvent e(newSize, oldSize);
                QApplication::sendSpontaneousEvent(this, &e);
                if (!testAttribute(Qt::WA_StaticContents))
                    testWState(Qt::WState_InPaintEvent)?update():repaint();
            } else {
                QResizeEvent *e = new QResizeEvent(newSize, oldSize);
                QApplication::postEvent(this, e);
            }
        }
    } else if (msg.message == WM_MOVE) {        // move event
        int a = (int) (short) LOWORD(msg.lParam);
        int b = (int) (short) HIWORD(msg.lParam);
        QPoint oldPos = geometry().topLeft();
        QPoint newCPos(a, b);
        // Ignore silly Windows move event to wild pos after iconify.
        if (!IsIconic(winId()) && newCPos != oldPos) {
            cr.moveTopLeft(newCPos);
            data->crect = cr;
            if (isVisible()) {
                QMoveEvent e(newCPos, oldPos);  // cpos (client position)
                QApplication::sendSpontaneousEvent(this, &e);
            } else {
                QMoveEvent * e = new QMoveEvent(newCPos, oldPos);
                QApplication::postEvent(this, e);
            }
        }
    }
    clearWState(Qt::WState_ConfigPending);                // clear config flag
    return true;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent(const MSG &)
{
    return d->close_helper(QWidgetPrivate::CloseWithSpontaneousEvent);
}


void QETWidget::eraseWindowBackground(HDC hdc)
{
    if (testAttribute(Qt::WA_NoSystemBackground))
        return;

    const QWidget *w = this;
    QPoint offset;
    while (w->d->isBackgroundInherited()) {
        offset += w->pos();
        w = w->parentWidget();
    }

    RECT r;
    GetClientRect(data->winid, &r);

    QWidget *that = const_cast<QWidget*>(w);
    that->setWState(Qt::WState_InPaintEvent);
    qt_erase_background
        (hdc, r.left, r.top,
          r.right-r.left, r.bottom-r.top,
          data->pal.brush(w->d->bg_role),
          offset.x(), offset.y(), const_cast<QWidget*>(w));
    that->clearWState(Qt::WState_InPaintEvent);
}


void  QApplication::setCursorFlashTime(int msecs)
{
    SetCaretBlinkTime(msecs / 2);
    cursor_flash_time = msecs;
}


int QApplication::cursorFlashTime()
{
    int blink = (int)GetCaretBlinkTime();
    if (!blink)
        return cursor_flash_time;
    if (blink > 0)
        return 2*blink;
    return 0;
}

void QApplication::setDoubleClickInterval(int ms)
{
#ifndef Q_OS_TEMP
    SetDoubleClickTime(ms);
#endif
    mouse_double_click_time = ms;
}


int QApplication::doubleClickInterval()
{
    int ms = GetDoubleClickTime();
    if (ms != 0)
        return ms;
    return mouse_double_click_time;
}

void QApplication::setWheelScrollLines(int n)
{
#ifdef SPI_SETWHEELSCROLLLINES
    if (n < 0)
        n = 0;
    QT_WA({
        SystemParametersInfo(SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0);
    } , {
        SystemParametersInfoA(SPI_SETWHEELSCROLLLINES, (uint)n, 0, 0);
    });
#else
    wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    QT_WA({
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, sizeof(uint), &i, 0);
    } , {
        SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, sizeof(uint), &i, 0);
    });
    if (i > INT_MAX)
        i = INT_MAX;
    return i;
#else
    return wheel_scroll_lines;
#endif
}

static bool effect_override = false;

void QApplication::setEffectEnabled(Qt::UIEffect effect, bool enable)
{
    effect_override = true;
    switch (effect) {
    case Qt::UI_AnimateMenu:
        animate_menu = enable;
        break;
    case Qt::UI_FadeMenu:
        fade_menu = enable;
        break;
    case Qt::UI_AnimateCombo:
        animate_combo = enable;
        break;
    case Qt::UI_AnimateTooltip:
        animate_tooltip = enable;
        break;
    case Qt::UI_FadeTooltip:
        fade_tooltip = enable;
        break;
    case Qt::UI_AnimateToolBox:
        animate_toolbox = enable;
        break;
    default:
        animate_ui = enable;
        break;
    }
    if (desktopSettingsAware() && !(QSysInfo::WindowsVersion == QSysInfo::WV_95 || QSysInfo::WindowsVersion == QSysInfo::WV_NT)) {
        // we know that they can be used when we are here
        UINT api;
        switch (effect) {
        case Qt::UI_AnimateMenu:
            api = SPI_SETMENUANIMATION;
            break;
        case Qt::UI_FadeMenu:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return;
            api = SPI_SETMENUFADE;
            break;
        case Qt::UI_AnimateCombo:
            api = SPI_SETCOMBOBOXANIMATION;
            break;
        case Qt::UI_AnimateTooltip:
            api = SPI_SETTOOLTIPANIMATION;
            break;
        case Qt::UI_FadeTooltip:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return;
            api = SPI_SETTOOLTIPFADE;
            break;
        default:
           api = SPI_SETUIEFFECTS;
        break;
        }
        BOOL onoff = enable;
        QT_WA({
            SystemParametersInfo(api, 0, (void*)onoff, 0);
        }, {
            SystemParametersInfoA(api, 0, (void*)onoff, 0);
        });
    }
}

bool QApplication::isEffectEnabled(Qt::UIEffect effect)
{
    if (QColormap::instance().depth() < 16)
        return false;

    if (!effect_override && desktopSettingsAware() && !(QSysInfo::WindowsVersion == QSysInfo::WV_95 || QSysInfo::WindowsVersion == QSysInfo::WV_NT)) {
        // we know that they can be used when we are here
        BOOL enabled = false;
        UINT api;
        switch (effect) {
        case Qt::UI_AnimateMenu:
            api = SPI_GETMENUANIMATION;
            break;
        case Qt::UI_FadeMenu:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return false;
            api = SPI_GETMENUFADE;
            break;
        case Qt::UI_AnimateCombo:
            api = SPI_GETCOMBOBOXANIMATION;
            break;
        case Qt::UI_AnimateTooltip:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                api = SPI_GETMENUANIMATION;
            else
                api = SPI_GETTOOLTIPANIMATION;
            break;
        case Qt::UI_FadeTooltip:
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                return false;
            api = SPI_GETTOOLTIPFADE;
            break;
        default:
            api = SPI_GETUIEFFECTS;
            break;
        }
        QT_WA({
            SystemParametersInfo(api, 0, &enabled, 0);
        } , {
            SystemParametersInfoA(api, 0, &enabled, 0);
        });
        return enabled;
    } else {
        switch(effect) {
        case Qt::UI_AnimateMenu:
            return animate_menu;
        case Qt::UI_FadeMenu:
            return fade_menu;
        case Qt::UI_AnimateCombo:
            return animate_combo;
        case Qt::UI_AnimateTooltip:
            return animate_tooltip;
        case Qt::UI_FadeTooltip:
            return fade_tooltip;
        case Qt::UI_AnimateToolBox:
            return animate_toolbox;
        default:
            return animate_ui;
        }
    }
}

bool QSessionManager::allowsInteraction()
{
    sm_blockUserInput = false;
    return true;
}

bool QSessionManager::allowsErrorInteraction()
{
    sm_blockUserInput = false;
    return true;
}

void QSessionManager::release()
{
    if (sm_smActive)
        sm_blockUserInput = true;
}

void QSessionManager::cancel()
{
    sm_cancel = true;
}

#if defined(Q_OS_WIN) && defined(QT_COMPAT)
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
    \value WV_2003
    \value WV_NT_based

    \value WV_CE
    \value WV_CENET
    \value WV_CE_based
*/
#endif
