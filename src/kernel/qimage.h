/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qimage.h#24 $
**
** Definition of QImage and QImageIO classes
**
** Author  : Haavard Nord
** Created : 950207
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QIMAGE_H
#define QIMAGE_H

#include "qpixmap.h"
#include "qstring.h"


class QImage
{
public:
    enum Endian { IgnoreEndian, BigEndian, LittleEndian };

    QImage();
    QImage( int width, int height, int depth, int numColors=0,
	    Endian bitOrder=IgnoreEndian );
    QImage( const QImage & );
   ~QImage();

    QImage     &operator=( const QImage & );
    QImage     &operator=( const QPixmap & );
    void	detach();
    QImage	copy()		const;

    bool	isNull()	const	{ return data->bits == 0; }

    int		width()		const	{ return data->w; }
    int		height()	const	{ return data->h; }
    QSize	size()		const	{ return QSize(data->w,data->h); }
    QRect	rect()		const	{ return QRect(0,0,data->w,data->h); }
    int		depth()		const	{ return data->d; }
    int		numColors()	const	{ return data->ncols; }
    QImage::Endian bitOrder()	const	{ return (Endian) data->bitordr; }

    QRgb	color( int i )	const;
    void	setColor( int i, QRgb c );
    void	setNumColors( int );

    uchar      *bits()		const;
    uchar      *scanLine( int ) const;
    uchar     **jumpTable()	const;
    QRgb       *colorTable()	const;
    int		numBytes()	const;
    int		bytesPerLine()	const;

    bool	create( int width, int height, int depth, int numColors=0,
			QImage::Endian bitOrder=IgnoreEndian );
    void	reset();

    QImage	convertDepth( int )	const;
    QImage	convertBitOrder( QImage::Endian )	const;

    static QImage::Endian systemBitOrder();
    static QImage::Endian systemByteOrder();

private:
    void	init();
    void	freeBits();

    struct QImageData : QShared {		// internal image data
	int	w;				// image width
	int	h;				// image height
	int	d;				// image depth
	int	ncols;				// number of colors
	int	nbytes;				// number of bytes data
	int	bitordr;			// bit order (1 bit depth)
	QRgb   *ctbl;				// color table
	uchar **bits;				// image data
    } *data;
};


class QIODevice;
typedef void (*image_io_handler)( QImageIO * ); // image IO handler


class QImageIO
{
public:
    QImageIO();
    QImageIO( QIODevice	 *ioDevice, const char *format );
    QImageIO( const char *fileName, const char *format );
   ~QImageIO();


    const QImage &image()	const	{ return im; }
    int		status()	const	{ return iostat; }
    const char *format()	const	{ return frmt; }
    QIODevice  *ioDevice()	const	{ return iodev; }
    const char *fileName()	const	{ return fname; }
    const char *parameters()	const	{ return params; }
    const char *description()	const	{ return descr; }

    void	setImage( const QImage & );
    void	setStatus( int );
    void	setFormat( const char * );
    void	setIODevice( QIODevice * );
    void	setFileName( const char * );
    void	setParameters( const char * );
    void	setDescription( const char * );

    bool	read();
    bool	write();

    static const char *imageFormat( const char *fileName );
    static const char *imageFormat( QIODevice * );

    static void defineIOHandler( const char *format,
				 const char *header,
				 const char *flags,
				 image_io_handler read_image,
				 image_io_handler write_image );

private:
    QImage	im;				// image
    int		iostat;				// IO status
    QString	frmt;				// image format
    QIODevice  *iodev;				// IO device
    QString	fname;				// file name
    char       *params;				// image parameters
    char       *descr;				// image description
};


// --------------------------------------------------------------------------
// QImage member functions
//

inline uchar **QImage::jumpTable() const
{
    return data->bits;
}

inline QRgb *QImage::colorTable() const
{
    return data->ctbl;
}

inline int QImage::numBytes() const
{
    return data->nbytes;
}

inline int QImage::bytesPerLine() const
{
    return data->h ? data->nbytes/data->h : 0;
}

inline uchar *QImage::bits() const
{
    return data->bits ? data->bits[0] : 0;
}

#if !(defined(QIMAGE_C) || defined(DEBUG))

inline QRgb QImage::color( int i ) const
{
    return data->ctbl ? data->ctbl[i] : (QRgb)-1;
}

inline void QImage::setColor( int i, QRgb c )
{
    if ( data->ctbl )
	data->ctbl[i] = c;
}

inline uchar *QImage::scanLine( int i ) const
{
    return data->bits ? data->bits[i] : 0;
}

#endif


#endif // QIMAGE_H
