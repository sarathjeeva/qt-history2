/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor.h#57 $
**
** Definition of QColor class
**
** Created : 940112
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

#ifndef QCOLOR_H
#define QCOLOR_H

#ifndef QT_H
#include "qwindowdefs.h"
#endif // QT_H


const QRgb  RGB_DIRTY	= 0x80000000;		// flags unset color
const QRgb  RGB_INVALID = 0x40000000;		// flags invalid color
const QRgb  RGB_DIRECT	= 0x20000000;		// flags directly set pixel
const QRgb  RGB_MASK	= 0x00ffffff;		// masks RGB values


Q_EXPORT inline int qRed( QRgb rgb )		// get red part of RGB
{ return (int)((rgb >> 16) & 0xff); }

Q_EXPORT inline int qGreen( QRgb rgb )		// get green part of RGB
{ return (int)((rgb >> 8) & 0xff); }

Q_EXPORT inline int qBlue( QRgb rgb )		// get blue part of RGB
{ return (int)(rgb & 0xff); }

Q_EXPORT inline int qAlpha( QRgb rgb )		// get alpha part of RGBA
{ return (int)((rgb >> 24) & 0xff); }

Q_EXPORT inline QRgb qRgb( int r, int g, int b )// set RGB value
{ return (uint)((uint)(r & 0xff) << 16) |((uint)(g & 0xff) << 8) |(b & 0xff); }

Q_EXPORT inline QRgb qRgba( int r, int g, int b, int a )// set RGBA value
{ return qRgb(r,g,b) | ((uint)(a & 0xff) << 24); }

Q_EXPORT inline int qGray( int r, int g, int b )// convert R,G,B to gray 0..255
{ return (r*11+g*16+b*5)/32; }

Q_EXPORT inline int qGray( QRgb rgb )		// convert RGB to gray 0..255
{ return qGray( qRed(rgb), qGreen(rgb), qBlue(rgb) ); }


class Q_EXPORT QColor
{
public:
    enum Spec { Rgb, Hsv };

    QColor();
    QColor( int r, int g, int b );
    QColor( int x, int y, int z, Spec );
    QColor( QRgb rgb, uint pixel=0xffffffff);
    QColor( const QString& name );
    QColor( const QColor & );
    QColor &operator=( const QColor & );

    bool   isValid() const;
    bool   isDirty() const;

    void   setNamedColor( const QString& name );

    void   rgb( int *r, int *g, int *b ) const;
    QRgb   rgb()    const;
    void   setRgb( int r, int g, int b );
    void   setRgb( QRgb rgb );

    int	   red()    const;
    int	   green()  const;
    int	   blue()   const;

    void   hsv( int *h, int *s, int *v ) const;
    void   getHsv( int &h, int &s, int &v ) const;
    void   setHsv( int h, int s, int v );

    QColor light( int f = 150 ) const;
    QColor dark( int f = 200 )	const;

    bool   operator==( const QColor &c ) const;
    bool   operator!=( const QColor &c ) const;

    static bool lazyAlloc();
    static void setLazyAlloc( bool );
    uint   alloc();
    uint   pixel()  const;

    static int  maxColors();
    static int  numBitPlanes();

    static int  enterAllocContext();
    static void leaveAllocContext();
    static int  currentAllocContext();
    static void destroyAllocContext( int );

#if defined(_WS_WIN_)
    static HPALETTE hPal()  { return hpal; }
    static uint	realizePal( QWidget * );
#endif

    static void initialize();
    static void cleanup();

private:
    void   setSystemNamedColor( const QString& name );
    static void initGlobalColors();
    static bool color_init;
    static bool globals_init;
    static bool lazy_alloc;
#if defined(_WS_WIN_)
    static HPALETTE hpal;
#endif
    uint   pix;
    QRgb   rgbVal;
};


inline QColor::QColor()
{ rgbVal = RGB_INVALID; pix = 0; }

inline QColor::QColor( int r, int g, int b )
{ setRgb( r, g, b ); }

inline bool QColor::isValid() const
{ return (rgbVal & RGB_INVALID) == 0; }

inline bool QColor::isDirty() const
{ return (rgbVal & RGB_DIRTY) != 0; }

inline QRgb QColor::rgb() const
{ return rgbVal & RGB_MASK; }

inline int QColor::red() const
{ return qRed(rgbVal); }

inline int QColor::green() const
{ return qGreen(rgbVal); }

inline int QColor::blue() const
{ return qBlue(rgbVal); }

inline uint QColor::pixel() const
{ return (rgbVal & RGB_DIRTY) == 0 ? pix : ((QColor*)this)->alloc(); }

inline bool QColor::lazyAlloc()
{ return lazy_alloc; }


inline bool QColor::operator==( const QColor &c ) const
{
    return (((rgbVal | c.rgbVal) & RGB_DIRECT) == 0 &&
	    (rgbVal & 0x00ffffff) == (c.rgbVal & 0x00ffffff)) ||
	   ((rgbVal & c.rgbVal & RGB_DIRECT) != 0 &&
	    (rgbVal & 0x00ffffff) == (c.rgbVal & 0x00ffffff) && pix == c.pix);
}

inline bool QColor::operator!=( const QColor &c ) const
{
    return !operator==(c);
}


/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QColor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QColor & );


#endif // QCOLOR_H
