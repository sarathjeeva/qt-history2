/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarray.h#26 $
**
** Definition of QBitArray class
**
** Created : 940118
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

#ifndef QBITARRAY_H
#define QBITARRAY_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


/*****************************************************************************
  QBitVal class; a context class for QBitArray::operator[]
 *****************************************************************************/

class QBitArray;

class Q_EXPORT QBitVal
{
private:
    QBitArray *array;
    uint    index;
public:
    QBitVal( QBitArray *a, uint i ) : array(a), index(i) {}
    operator int();
    QBitVal &operator=( const QBitVal &v );
    QBitVal &operator=( int v );
};


/*****************************************************************************
  QBitArray class
 *****************************************************************************/

class Q_EXPORT QBitArray : public QByteArray
{
public:
    QBitArray();
    QBitArray( uint size );
    QBitArray( const QBitArray &a ) : QByteArray( a ) {}

    QBitArray &operator=( const QBitArray & );

    uint    size() const;
    bool    resize( uint size );

    bool    fill( bool v, int size = -1 );

    void    detach();
    QBitArray copy() const;

    bool    testBit( uint index ) const;
    void    setBit( uint index );
    void    setBit( uint index, bool value );
    void    clearBit( uint index );
    bool    toggleBit( uint index );

    bool    at( uint index ) const;
    QBitVal operator[]( int index );

    QBitArray &operator&=( const QBitArray & );
    QBitArray &operator|=( const QBitArray & );
    QBitArray &operator^=( const QBitArray & );
    QBitArray  operator~() const;

protected:
    struct bitarr_data : public QGArray::array_data {
	uint   nbits;
    };
    array_data *newData()		    { return new bitarr_data; }
    void	deleteData( array_data *d ) { delete (bitarr_data*)d; }
private:
    void    pad0();
};


inline QBitArray &QBitArray::operator=( const QBitArray &a )
{ return (QBitArray&)assign( a ); }

inline uint QBitArray::size() const
{ return ((bitarr_data*)sharedBlock())->nbits; }

inline void QBitArray::setBit( uint index, bool value )
{ if ( value ) setBit(index); else clearBit(index); }

inline bool QBitArray::at( uint index ) const
{ return testBit(index); }

inline QBitVal QBitArray::operator[]( int index )
{ return QBitVal( (QBitArray*)this, index ); }


/*****************************************************************************
  Misc. QBitArray operator functions
 *****************************************************************************/

QBitArray operator&( const QBitArray &, const QBitArray & );
QBitArray operator|( const QBitArray &, const QBitArray & );
QBitArray operator^( const QBitArray &, const QBitArray & );


inline QBitVal::operator int()
{
    return array->testBit( index );
}

inline QBitVal &QBitVal::operator=( const QBitVal &v )
{
    array->setBit( index, v.array->testBit(v.index) );
    return *this;
}

inline QBitVal &QBitVal::operator=( int v )	// ### Qt 2.0: change to bool
{
    array->setBit( index, v );
    return *this;
}


/*****************************************************************************
  QBitArray stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QBitArray & );
QDataStream &operator>>( QDataStream &, QBitArray & );


#endif // QBITARRAY_H
