/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.h#123 $
**
** Definition of QApplication class
**
** Created : 931107
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

#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#ifndef QT_H
#include "qwidget.h"
#include "qlist.h"

#include "qtranslator.h"
#include "qasciidict.h"
#include "qpalette.h"
#endif // QT_H

class QSessionManager;
class QStyle;

class QApplication;
extern Q_EXPORT QApplication *qApp;		// global application object

#if defined(QT_DLL) || defined(QT_MAKEDLL)
#define QT_BASEAPP
typedef QApplication QNonBaseApplication;
#define QApplication QBaseApplication
#else
#define QNonBaseApplication QApplication
#endif


class Q_EXPORT QApplication : public QObject
{
    Q_OBJECT
public:
    QApplication( int &argc, char **argv );
#if defined(_WS_X11_)
    QApplication( Display* dpy );
#endif
    virtual ~QApplication();

    int		    argc()	const;
    char	  **argv()	const;

    static QStyle&	    style();
    static void	    setStyle( QStyle* );

#if 1	/* OBSOLETE */
    enum ColorMode { NormalColors, CustomColors };
    static ColorMode colorMode();
    static void      setColorMode( QApplication::ColorMode );
#endif

    enum ColorSpec { NormalColor=0, CustomColor=1, ManyColor=2 };
    static int	     colorSpec();
    static void      setColorSpec( int );

    static QCursor  *overrideCursor();
    static void	     setOverrideCursor( const QCursor &, bool replace=FALSE );
    static void	     restoreOverrideCursor();

    static bool	     hasGlobalMouseTracking();
    static void	     setGlobalMouseTracking( bool enable );

    static QPalette  palette( const QWidget* = 0 );
    static void	     setPalette( const QPalette &, bool updateAllWidgets=FALSE, const char* className = 0 );

    static QFont     font( const QWidget* = 0 );
    static void	     setFont( const QFont &, bool updateAllWidgets=FALSE, const char* className = 0 );
    static QFontMetrics fontMetrics();

    QWidget	    *mainWidget()  const;
    virtual void     setMainWidget( QWidget * );
    virtual void     polish( QWidget * );

    static QWidgetList *allWidgets();
    static QWidgetList *topLevelWidgets();
    static QWidget     *desktop();
    static QWidget     *activePopupWidget();
    static QWidget     *activeModalWidget();
    static QClipboard  *clipboard();
    QWidget	       *focusWidget() const;
    QWidget	       *activeWindow() const;

    static QWidget  *widgetAt( int x, int y, bool child=FALSE );
    static QWidget  *widgetAt( const QPoint &, bool child=FALSE );

    int		     exec();
    void	     processEvents();
    void	     processEvents( int maxtime );
    void	     processOneEvent();
    int		     enter_loop();
    void	     exit_loop();
    static void	     exit( int retcode=0 );

    static bool	     sendEvent( QObject *receiver, QEvent *event );
    static void	     postEvent( QObject *receiver, QEvent *event );
    static void	     sendPostedEvents( QObject *receiver, int event_type );
    static void	     sendPostedEvents();

    static void      removePostedEvents( QObject *receiver );

    virtual bool     notify( QObject *, QEvent * );

    static bool	     startingUp();
    static bool	     closingDown();

    static void	     flushX();
    static void	     syncX();

    static void	     beep();

    void	     setDefaultCodec( QTextCodec* );
    void	     installTranslator( QTranslator * );
    void	     removeTranslator( QTranslator * );
    QString	     translate( const char * scope, const char * key ) const;

    static void 	setWinStyleHighlightColor( const QColor & );
    static const QColor& winStyleHighlightColor();

    static void 	setDesktopSettingsAware( bool );
    static bool 	desktopSettingsAware();

    static void 	setCursorFlashTime( int );
    static int 	cursorFlashTime();

    static void 	setDoubleClickInterval( int );
    static int 	doubleClickInterval();

#if defined(_WS_WIN_)
    static WindowsVersion winVersion();
#endif

#if defined(_WS_MAC_)
    virtual bool     macEventFilter( MSG * );
#elif defined(_WS_WIN_)
    virtual bool     winEventFilter( MSG * );
#elif defined(_WS_X11_)
    virtual bool     x11EventFilter( XEvent * );
    virtual int	     x11ClientMessage( QWidget*, XEvent*, bool passive_only);
    int              x11ProcessEvent( XEvent* );
#endif

#if defined(_WS_WIN_)
    void	     winFocus( QWidget *, bool );
#endif


    // session management
    bool isSessionRestored() const;
    QString sessionId() const;
    virtual void commitData( QSessionManager& sm );
    virtual void saveState( QSessionManager& sm );


signals:
    void	     lastWindowClosed();
    void	     aboutToQuit();
public slots:
    void	     quit();
    void	     closeAllWindows();

private:
    bool	     processNextEvent( bool );
    void	     initialize( int, char ** );
    void	     init_precmdline();
    void	     process_cmdline( int* argcptr, char ** argv );

    int		     app_argc;
    char	   **app_argv;
    bool	     quit_now;
    int		     quit_code;
    static QStyle   *app_style;
    static int	     app_cspec;
    static QPalette *app_pal;
    static QFont    *app_font;
    static QCursor  *app_cursor;
    static int	     app_tracking;
    static bool	     is_app_running;
    static bool	     is_app_closing;
    static bool	     app_exit_loop;
    static int	     loop_level;
    static QWidget  *main_widget;
    static QWidget  *focus_widget;
    static QWidget  *active_window;
    static bool	     obey_desktop_settings;
    static int	     cursor_flash_time;
    static int	     mouse_double_click_time;
    QList<QTranslator> * translators;
    QSessionManager* session_manager;
    QString session_id;
    bool is_session_restored;

    static QAsciiDict<QPalette> *app_palettes;
    static QAsciiDict<QFont>    *app_fonts;

    static QWidgetList *popupWidgets;
    bool	    inPopupMode() const;
    void	    closePopup( QWidget *popup );
    void	    openPopup( QWidget *popup );
    void	    noteTopLevel( QWidget* tlw );

    static void	removePostedEvent( QEvent * );

    friend class QWidget;
    friend class QETWidget;
    friend class QEvent;
#if defined QT_BASEAPP
    friend QNonBaseApplication;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication( const QApplication & );
    QApplication &operator=( const QApplication & );
#endif
};


inline int QApplication::argc() const
{
    return app_argc;
}

inline char **QApplication::argv() const
{
    return app_argv;
}

inline QStyle& QApplication::style()
{
    return *app_style;
}

inline QCursor *QApplication::overrideCursor()
{
    return app_cursor;
}

inline bool QApplication::hasGlobalMouseTracking()
{
    return app_tracking > 0;
}

inline QWidget *QApplication::mainWidget() const
{
    return main_widget;
}

inline QWidget *QApplication::focusWidget() const
{
    return focus_widget;
}

inline QWidget *QApplication::activeWindow() const
{
    return active_window;
}

inline QWidget *QApplication::widgetAt( const QPoint &p, bool child )
{
    return widgetAt( p.x(), p.y(), child );
}

inline bool QApplication::inPopupMode() const
{
    return popupWidgets != 0;
}

inline bool QApplication::isSessionRestored() const
{
    return is_session_restored;
}

inline QString QApplication::sessionId() const
{
    return session_id;
}

#if defined(QT_BASEAPP)


#undef QApplication

class QApplication : public QBaseApplication
{
public:
    QApplication( int &, char ** );
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QApplication( const QApplication & );
    QApplication &operator=( const QApplication & );
#endif
    friend QBaseApplication;
};

inline bool QBaseApplication::sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }

#if defined(Q_MOC_OUTPUT_REVISION) && defined(Q_MOC_QApplication)
#if defined(QT_MAKEDLL)
#define QApplication QBaseApplication
#endif
#endif

#else
inline bool QApplication::sendEvent( QObject *receiver, QEvent *event )
	{ return qApp->notify( receiver, event ); }
#endif



#endif // QAPPLICATION_H
