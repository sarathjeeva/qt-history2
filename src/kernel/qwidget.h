/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.h#63 $
**
** Definition of QWidget class
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWIDGET_H
#define QWIDGET_H

#include "qwindefs.h"
#include "qobject.h"
#include "qpaintd.h"
#include "qpalette.h"
#include "qcursor.h"
#include "qfont.h"
#include "qfontmet.h"
#include "qfontinf.h"


class QWidget : public QObject, public QPaintDevice
{						// base class for UI objects
    Q_OBJECT
public:
    QWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QWidget();

    WId		 id() const;

  // GUI style setting

    GUIStyle	 style() const;
    void	 setStyle( GUIStyle );

  // Widget control functions

    void	 enable();
    void	 disable();
    void	 setEnabled( bool );
    bool	 isEnabled()	const;
    bool	 isDisabled()	const;

  // Widget coordinates

    const QRect &frameGeometry() const;
    const QRect &geometry()	const;
    int		 x()		const;
    int		 y()		const;
    QPoint	 pos()		const;
    QSize	 size()		const;
    int		 width()	const;
    int		 height()	const;
    QRect	 rect()		const;
    QRect	 childrenRect() const;

    void	 setMinimumSize( int w, int h );
    void	 setMaximumSize( int w, int h );
    void	 setSizeIncrement( int w, int h );

  // Widget coordinate mapping

    QPoint	 mapToGlobal( const QPoint & )	 const;
    QPoint	 mapFromGlobal( const QPoint & ) const;
    QPoint	 mapToParent( const QPoint & )	 const;
    QPoint	 mapFromParent( const QPoint & ) const;

    QWidget	*topLevelWidget()   const;

  // Widget attribute functions

    const QColor &backgroundColor() const;
    const QColor &foregroundColor() const;
    virtual void setBackgroundColor( const QColor & );
    virtual void setBackgroundPixmap( const QPixmap & );

    const QColorGroup &colorGroup() const;
    const QPalette    &palette()    const;
    virtual void       setPalette( const QPalette & );

    const QFont &font()		const;
    virtual void setFont( const QFont & );
    QFontMetrics fontMetrics()	const;
    QFontInfo	 fontInfo()	const;

    const QCursor &cursor() const;
    void	 setCursor( const QCursor & );

    bool	 setMouseTracking( bool enable );

  // Keyboard input focus functions

    void	 setActiveWindow();
    bool	 hasFocus() const;
    void	 setFocus();

  // Grab functions

    void	 grabMouse();
    void	 grabMouse( const QCursor & );
    void	 releaseMouse();
    void	 grabKeyboard();
    void	 releaseKeyboard();
    static QWidget *mouseGrabber();
    static QWidget *keyboardGrabber();

  // Update/refresh functions

    bool	 enableUpdates( bool enable );
    void	 update();
    void	 update( int x, int y, int w, int h);
    void	 repaint( bool erase=TRUE );
    void	 repaint( int x, int y, int w, int h, bool erase=TRUE );
    void	 repaint( const QRect &, bool erase=TRUE );

  // Widget management functions

    virtual void show();
    virtual void hide();
    virtual bool close( bool forceKill=FALSE );
    bool	 isVisible()	const;
    bool	 isActive()	const;
    void	 raise();
    void	 lower();
    virtual void move( int x, int y );
    void	 move( const QPoint & );
    virtual void resize( int w, int h );
    void	 resize( const QSize & );
    virtual void setGeometry( int x, int y, int w, int h );
    void	 setGeometry( const QRect & );
    virtual void adjustSize();

    void	 recreate( QWidget *parent, WFlags f, const QPoint &p,
			   bool showIt=FALSE );

    void	 erase();
    void	 erase( int x, int y, int w, int h );
    void	 erase( const QRect & );
    void	 scroll( int dx, int dy );

    void	 drawText( int x, int y, const char * );
    void	 drawText( const QPoint &, const char * );

  // Misc. functions

public:
    QWidget	*parentWidget() const;
    bool	 testWFlags( WFlags n ) const;
    static QWidget	 *find( WId );
    static QWidgetMapper *wmapper();

  // Signals

signals:
    void	 destroyed();

  // Event handlers

protected:
    bool	 event( QEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void moveEvent( QMoveEvent * );
    virtual void resizeEvent( QResizeEvent * );
    virtual void closeEvent( QCloseEvent * );

#if defined(_WS_MAC_)
    virtual bool macEvent( MSG * );		// Macintosh event
#elif defined(_WS_WIN_)
    virtual bool winEvent( MSG * );		// Windows event
#elif defined(_WS_PM_)
    virtual bool pmEvent( QMSG * );		// OS/2 PM event
#elif defined(_WS_X11_)
    virtual bool x11Event( XEvent * );		// X11 event
#endif

  // Misc. protected functions

protected:
    bool	 acceptFocus()	const;
    void	 setAcceptFocus( bool );
    long	 metric( int )	const;

    WFlags	 getWFlags()	const;
    void	 setWFlags( WFlags );
    void	 clearWFlags( WFlags n );
    void	 setFRect( const QRect & );
    void	 setCRect( const QRect & );

    virtual bool focusNextChild();
    virtual bool focusPrevChild();

    QWExtra	*extraData();

#if defined(_WS_PM_)
    int		 convertYPos( int );
    void	 reposChildren();
    WId		 frm_wnd;
#endif

private:
    void	 set_id( WId );
    bool	 create();
    bool	 destroy();
    void	 createExtra();
    WId		 ident;
    WFlags	 flags;
    QRect	 frect;
    QRect	 crect;
    QColor	 bg_col;
    QPalette	 pal;
    QFont	 fnt;
    QCursor	 curs;
    QWExtra	*extra;
    QWidget	*focusChild;
    static void	 createMapper();
    static void	 destroyMapper();
    static QWidgetMapper *mapper;
    friend class QApplication;
    friend class QPainter;
    friend class QFontMetrics;
    friend class QFontInfo;
};


inline bool QWidget::testWFlags( WFlags f ) const
{ return (flags & f) != 0; }

inline WId QWidget::id() const
{ return ident; }

inline bool QWidget::isEnabled() const
{ return !testWFlags(WState_Disabled); }

inline bool QWidget::isDisabled() const
{ return testWFlags(WState_Disabled); }

inline const QRect &QWidget::frameGeometry() const
{ return frect; }

inline const QRect &QWidget::geometry() const
{ return crect; }

inline int QWidget::x() const
{ return frect.x(); }

inline int QWidget::y() const
{ return frect.y(); }

inline QPoint QWidget::pos() const
{ return frect.topLeft(); }

inline QSize QWidget::size() const
{ return crect.size(); }

inline int QWidget::width() const
{ return crect.width(); }

inline int QWidget::height() const
{ return crect.height(); }

inline QRect QWidget::rect() const
{ return QRect(0,0,crect.width(),crect.height()); }

inline const QColor &QWidget::backgroundColor() const
{ return bg_col; }

inline const QPalette &QWidget::palette() const
{ return pal; }

inline const QFont &QWidget::font() const
{ return fnt; }

inline QFontMetrics QWidget::fontMetrics() const
{ return QFontMetrics(this); }

inline QFontInfo QWidget::fontInfo() const
{ return QFontInfo(this); }

inline void QWidget::repaint( bool erase )
{ repaint( rect(), erase ); }

inline void QWidget::repaint( int x, int y, int w, int h, bool erase )
{ repaint( QRect(x,y,w,h), erase ); }

inline void QWidget::erase()
{ erase( 0, 0, crect.width(), crect.height() ); }

inline void QWidget::erase( const QRect &r )
{ erase( r.x(), r.y(), r.width(), r.height() ); }

inline bool QWidget::isVisible() const
{ return testWFlags(WState_Visible); }

inline bool QWidget::isActive() const
{ return testWFlags(WState_Active); }

inline void QWidget::move( const QPoint &p )
{ move( p.x(), p.y() ); }

inline void QWidget::resize( const QSize &s )
{ resize( s.width(), s.height()); }

inline void QWidget::setGeometry( const QRect &r )
{ setGeometry( r.left(), r.top(), r.width(), r.height() ); }

inline void QWidget::drawText( const QPoint &p, const char *s )
{ drawText( p.x(), p.y(), s ); }

inline QWidget *QWidget::parentWidget() const
{ return (QWidget *)QObject::parent(); };

inline QWidgetMapper *QWidget::wmapper()
{ return mapper; }

inline bool QWidget::acceptFocus() const
{ return testWFlags(WState_AcceptFocus); }

inline WFlags QWidget::getWFlags() const
{ return flags; }

inline void QWidget::setWFlags( WFlags f )
{ flags |= f; }

inline void QWidget::clearWFlags( WFlags f )
{ flags &= ~f; }


#endif // QWIDGET_H
