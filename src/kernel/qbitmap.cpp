/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.cpp#44 $
**
** Implementation of QBitmap class
**
** Created : 941020
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qbitmap.h"
#include "qimage.h"


// REVISED: paul
/*!
  \class QBitmap qbitmap.h
  \brief The QBitmap class provides monochrome (1 bit depth) pixmaps.

  \ingroup drawing
  \ingroup shared

  The QBitmap class is a monochrome off-screen paint device, used
  mainly for creating custom QCursor and QBrush objects, in
  QPixmap::setMask() and for QRegion.

  A QBitmap is a QPixmap with a QPixmap::depth() of 1.
  If a pixmap with a depth greater than 1 is assigned to a bitmap, the
  bitmap will be automatically dithered.  A QBitmap is guaranteed to always
  have the depth 1, unless it is QPixmap::isNull()  (has depth 0).

  When drawing in a QBitmap (or QPixmap with depth 1), we recommend using
  the  QColor objects \c Qt::color0 and \c Qt::color1.  Painting with \c
  color0 sets the bitmap bits to 0, and painting with \c color1 sets the
  bits to 1.  For a bitmap, 0-bits indicate background (or white) and
  1-bits indicate foreground (or black).  Using the \c black and \c white
  QColor objects make no sense, because the QColor::pixel()
  value is not necessarily 0 for black and 1 for white.

  Just like the QPixmap class, QBitmap is optimized by the use of \link
  shclass.html implicit sharing\endlink, so it is very efficient to pass
  QBitmap objects as arguments.

  \sa QPixmap, QPainter::drawPixmap(), bitBlt(), \link shclass.html Shared
  Classes\endlink
*/


/*!
  Constructs a null bitmap.
  \sa QPixmap::isNull()
*/

QBitmap::QBitmap()
{
    data->bitmap = TRUE;
}


/*!
  Constructs a bitmap with \a w width and \a h height.

  The contents of the bitmap is uninitialized if \a clear is FALSE, otherwise
  it is filled with pixel value 0 (the QColor \c Qt::color0).

  The optional \a optimization argument specifies the optimization
  setting for the bitmap.  The default optimization should be used
  in most cases.  Games and other pixmap-intensive applications may
  benefit from setting this argument.

  \sa QPixmap::setOptimization(), QPixmap::setDefaultOptimization()
*/

QBitmap::QBitmap( int w, int h, bool clear,
		  QPixmap::Optimization optimization )
    : QPixmap( w, h, 1, optimization )
{
    data->bitmap = TRUE;
    if ( clear )
	fill( Qt::color0 );
}


/*!
  \overload
*/

QBitmap::QBitmap( const QSize &size, bool clear,
		  QPixmap::Optimization optimization )
    : QPixmap( size, 1, optimization )
{
    data->bitmap = TRUE;
    if ( clear )
	fill( Qt::color0 );
}


/*!
  Constructs a bitmap with \a w width and \a h height and sets the contents
  to \a bits.

  The \a isXbitmap should be TRUE if \a bits was generated by the
  X11 bitmap program.  The X bitmap bit order is little endian.
  The QImage documentation discusses bit order of monochrome images.

  Example (creates an arrow bitmap):
  \code
    uchar arrow_bits[] = { 0x3f, 0x1f, 0x0f, 0x1f, 0x3b, 0x71, 0xe0, 0xc0 };
    QBitmap bm( 8, 8, arrow_bits, TRUE );
  \endcode
*/

QBitmap::QBitmap( int w, int h, const uchar *bits, bool isXbitmap )
    : QPixmap( w, h, bits, isXbitmap )
{
    data->bitmap = TRUE;
}


/*!
  \overload
*/

QBitmap::QBitmap( const QSize &size, const uchar *bits, bool isXbitmap )
    : QPixmap( size.width(), size.height(), bits, isXbitmap )
{
    data->bitmap = TRUE;
}


/*!
  Constructs a bitmap which is a copy of \a bitmap.
*/

QBitmap::QBitmap( const QBitmap &bitmap )
    : QPixmap( bitmap )
{
}


/*!
  Constructs a pixmap from the file \a fileName. If the file does not
  exist, or is of an unknown format, the pixmap becomes a null pixmap.

  The parameters are passed on to QPixmap::load(). Dithering will be
  performed if the file format uses more than 1 bit per pixel.

  \sa QPixmap::isNull(), QPixmap::load(), QPixmap::loadFromData(),
  QPixmap::save(), QPixmap::imageFormat()
*/

QBitmap::QBitmap( const QString& fileName, const char *format )
    : QPixmap() // Will set bitmap to null bitmap, explicit call for clarity
{
    data->bitmap = TRUE;
    load( fileName, format, Mono );
}


/*!
  Assigns the bitmap \a bitmap to this bitmap and returns a reference to this
  bitmap.
*/

QBitmap &QBitmap::operator=( const QBitmap &bitmap )
{
    QPixmap::operator=(bitmap);
#if defined(CHECK_STATE)
    ASSERT( data->bitmap );
#endif
    return *this;
}


/*!
  Assigns the pixmap \a pixmap to this bitmap and returns a reference to this
  bitmap.

  Dithering will be performed if the pixmap has a
  QPixmap::depth() greater than 1.
*/

QBitmap &QBitmap::operator=( const QPixmap &pixmap )
{
    if ( pixmap.isNull() ) {			// a null pixmap
	QBitmap bm( 0, 0, FALSE, pixmap.optimization() );
	QBitmap::operator=(bm);
    } else if ( pixmap.depth() == 1 ) {		// 1-bit pixmap
	if ( pixmap.isQBitmap() ) {		// another QBitmap
	    QPixmap::operator=(pixmap);		// shallow assignment
	} else {				// not a QBitmap, but 1-bit
	    QBitmap bm( pixmap.size(), FALSE, pixmap.optimization() );
	    bitBlt( &bm, 0,0, &pixmap, 0,0,pixmap.width(),pixmap.height() );
	    QBitmap::operator=(bm);
	}
    } else {					// n-bit depth pixmap
	QImage image;
	image = pixmap;				// convert pixmap to image
	*this = image;				// will dither image
    }
    return *this;
}


/*!
  Converts the image \a image to a bitmap and assigns the result to
  this bitmap.  Returns a reference to the bitmap.

  Dithering will be performed if the image has a
  QImage::depth()  greater than 1.
*/

QBitmap &QBitmap::operator=( const QImage &image )
{
    convertFromImage( image );
    return *this;
}


#if QT_FEATURE_TRANSFORMATIONS
/*!
  Returns a transformed copy of this bitmap, using \a matrix.

  This function does exactly the same as QPixmap::xForm(), except that
  it returns a QBitmap instead of a QPixmap.

  \sa QPixmap::xForm()
*/

QBitmap QBitmap::xForm( const QWMatrix &matrix ) const
{
    QPixmap pm = QPixmap::xForm( matrix );
    QBitmap bm;
    // Here we fake the pixmap to think it's a QBitmap. With this trick,
    // the QBitmap::operator=(const QPixmap&) will just refer the
    // pm.data and we do not need to perform a bitBlt.
    pm.data->bitmap = TRUE;
    bm = pm;
    return bm;
}
#endif // QT_FEATURE_TRANSFORMATIONS



