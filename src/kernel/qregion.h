/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion.h#43 $
**
** Definition of QRegion class
**
** Created : 940514
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QREGION_H
#define QREGION_H

#ifndef QT_H
#include "qshared.h"
#include "qrect.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion( int x, int y, int w, int h, RegionType = Rectangle );
    QRegion( const QRect &, RegionType = Rectangle );
    QRegion( const QPointArray &, bool winding=FALSE );
    QRegion( const QRegion & );
   ~QRegion();
    QRegion &operator=( const QRegion & );

    bool    isNull()   const;
    bool    isEmpty()  const;

    bool    contains( const QPoint &p ) const;
    bool    contains( const QRect &r )	const;

    void    translate( int dx, int dy );

    QRegion unite( const QRegion & )	const;
    QRegion intersect( const QRegion &) const;
    QRegion subtract( const QRegion & ) const;

// Work around clash with the ANSI C++ keyword "xor".
//
// Use of QRegion::xor() is deprecated - you should use QRegion::eor().
// Calls to QRegion::xor() will work for now, but give a warning.
//
// If possible, compile the Qt library without this ANSI C++ feature enabled,
// thus including both the old xor() and new eor() in the library, so old
// binaries will continue to work (with the warning).
//
// We also hide the xor() function if there is a #define for xor, in
// case someone is using #define xor ^ to work around deficiencies in
// their compiler that cause problems with some other header files.
//
#if !(defined(__STRICT_ANSI__) && defined(_CC_GNU_)) && !defined(_CC_EDG_) && !defined(_CC_HP_) && !defined(_CC_HP_ACC_) && !defined(_CC_USLC_) && !defined(xor)
    QRegion xor( const QRegion & )	const;
#endif
    QRegion eor( const QRegion & )	const;

    QRect   boundingRect() const;
    QArray<QRect> rects() const;

    bool    operator==( const QRegion & )  const;
    bool    operator!=( const QRegion &r ) const
			{ return !(operator==(r)); }

#if defined(_WS_WIN_)
    QRegion winRegion( HANDLE );
    HANDLE  handle() const { return data->rgn; }
#elif defined(_WS_X11_)
    Region  handle() const { return data->rgn; }
#endif

    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );

private:
    QRegion( bool );
    QRegion copy() const;
    void    detach();
#if defined(_WS_WIN_)
    QRegion winCombine( const QRegion &, int ) const;
#endif
    void    cmd( int id, void *, const QRegion * = 0, const QRegion * = 0 );
    void    exec( const QByteArray & );
    struct QRegionData : public QShared {
#if defined(_WS_WIN_)
	HANDLE rgn;
#elif defined(_WS_X11_)
	Region rgn;
#endif
	bool   is_null;
    } *data;
};


#define QRGN_SETRECT		1		// region stream commands
#define QRGN_SETELLIPSE		2		//  (these are internal)
#define QRGN_SETPTARRAY_ALT	3
#define QRGN_SETPTARRAY_WIND	4
#define QRGN_TRANSLATE		5
#define QRGN_OR			6
#define QRGN_AND		7
#define QRGN_SUB		8
#define QRGN_XOR		9
#define QRGN_RECTS	       10


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );


#endif // QREGION_H
