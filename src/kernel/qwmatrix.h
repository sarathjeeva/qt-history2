/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwmatrix.h#12 $
**
** Definition of QWMatrix class
**
** Created : 941020
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

#ifndef QWMATRIX_H
#define QWMATRIX_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpointarray.h"
#include "qrect.h"
#endif // QT_H


class Q_EXPORT QWMatrix					// 2D transform matrix
{
public:
    QWMatrix();
    QWMatrix( double m11, double m12, double m21, double m22,
	      double dx, double dy );

    void	setMatrix( double m11, double m12, double m21, double m22,
			   double dx,  double dy );

    double	m11() const { return _m11; }
    double	m12() const { return _m12; }
    double	m21() const { return _m21; }
    double	m22() const { return _m22; }
    double	dx()  const { return _dx; }
    double	dy()  const { return _dy; }

    void	map( int x, int y, int *tx, int *ty )	      const;
    void	map( double x, double y, double *tx, double *ty ) const;
    QPoint	map( const QPoint & )	const;
    QRect	map( const QRect & )	const;
    QPointArray map( const QPointArray & ) const;

    void	reset();

    QWMatrix   &translate( double dx, double dy );
    QWMatrix   &scale( double sx, double sy );
    QWMatrix   &shear( double sh, double sv );
    QWMatrix   &rotate( double a );

    QWMatrix	invert( bool * = 0 ) const;

    bool	operator==( const QWMatrix & ) const;
    bool	operator!=( const QWMatrix & ) const;
    QWMatrix   &operator*=( const QWMatrix & );

private:
    QWMatrix   &bmul( const QWMatrix & );
    double	_m11, _m12;
    double	_m21, _m22;
    double	_dx,  _dy;
};


Q_EXPORT QWMatrix operator*( const QWMatrix &, const QWMatrix & );


/*****************************************************************************
  QWMatrix stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QWMatrix & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QWMatrix & );


#endif // QWMATRIX_H
