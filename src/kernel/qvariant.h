/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qvariant.h#4 $
**
** Definition of QVariant class
**
** Created : 990414
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

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

class QString;
class QCString;
class QFont;
class QPixmap;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
class QColorGroup;
class QIconSet;
class QDataStream;
class QPointArray;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
// Relevant header files rejected after QVariant declaration
// for GCC 2.7.* compatibility
class QVariant;
class QVariantPrivate;
template <class T> class QValueList;
template <class T> class QValueListConstIterator;
template <class T> class QValueListNode;
template <class Key, class T> class QMap;
template <class Key, class T> class QMapConstIterator;

/**
 * This class acts like a union. It can hold one value at the
 * time but it can hold the most common types.
 * For CORBA people: It is a poor mans CORBA::Any.
 */
class Q_EXPORT QVariant
{
public:
    enum Type {
	Invalid,
	Map,
	List,
	String,
	StringList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
	ColorGroup,
	IconSet,
	Point,
	Image,
	Int,
	UInt,
	Bool,
	Double,
	CString,
	PointArray,
	Region,
	Bitmap,
	Cursor
    };

    QVariant();
    ~QVariant();
    QVariant( const QVariant& );
    QVariant( QDataStream& s );

    QVariant( const QString& );
    QVariant( const QCString& );
    QVariant( const char* );
    QVariant( const QStringList& );
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QColorGroup& );
    QVariant( const QIconSet& );
    QVariant( const QPointArray& );
    QVariant( const QRegion& );
    QVariant( const QBitmap& );
    QVariant( const QCursor& );
    QVariant( const QValueList<QVariant>& );
    QVariant( const QMap<QString,QVariant>& );
    QVariant( int );
    QVariant( uint );
    // ### Problems on some compilers ?
    QVariant( bool, int );
    QVariant( double );

    QVariant& operator= ( const QVariant& );
    bool operator==( const QVariant& ) const;
    bool operator!=( const QVariant& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;

    bool isValid() const;

    void clear();

    const QString toString() const;
    const QCString toCString() const;
    const QStringList toStringList() const;
    const QFont toFont() const;
    const QPixmap toPixmap() const;
    const QImage toImage() const;
    const QBrush toBrush() const;
    const QPoint toPoint() const;
    const QRect toRect() const;
    const QSize toSize() const;
    const QColor toColor() const;
    const QPalette toPalette() const;
    const QColorGroup toColorGroup() const;
    const QIconSet toIconSet() const;
    const QPointArray toPointArray() const;
    const QBitmap toBitmap() const;
    const QRegion toRegion() const;
    const QCursor toCursor() const;
    int toInt() const;
    uint toUInt() const;
    bool toBool() const;
    double toDouble() const;
    const QValueList<QVariant> toList() const;
    const QMap<QString,QVariant> toMap() const;

    QValueListConstIterator<QVariant> listBegin() const;
    QValueListConstIterator<QVariant> listEnd() const;
    QValueListConstIterator<QString> stringListBegin() const;
    QValueListConstIterator<QString> stringListEnd() const;
    QMapConstIterator<QString,QVariant> mapBegin() const;
    QMapConstIterator<QString,QVariant> mapEnd() const;
    QMapConstIterator<QString,QVariant> mapFind( const QString& ) const;

    QString& asString();
    QCString& asCString();
    QStringList& asStringList();
    QFont& asFont();
    QPixmap& asPixmap();
    QImage& asImage();
    QBrush& asBrush();
    QPoint& asPoint();
    QRect& asRect();
    QSize& asSize();
    QColor& asColor();
    QPalette& asPalette();
    QColorGroup& asColorGroup();
    QIconSet& asIconSet();
    QPointArray& asPointArray();
    QBitmap& asBitmap();
    QRegion& asRegion();
    QCursor& asCursor();
    int& asInt();
    uint& asUInt();
    bool& asBool();
    double& asDouble();
    QValueList<QVariant>& asList();
    QMap<QString,QVariant>& asMap();

    void load( QDataStream& );
    void save( QDataStream& ) const;

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

private:
    void detach();

    QVariantPrivate* d;
};

class QVariantPrivate : public QShared
{
public:
    QVariantPrivate();
    QVariantPrivate( QVariantPrivate* );
    ~QVariantPrivate();

    void clear();

    QVariant::Type typ;
    union
    {
	uint u;
	int i;
	bool b;
	double d;
	void *ptr;
    } value;
};

// These header files are down here for GCC 2.7.* compatibility
#ifndef QT_H
#include "qvaluelist.h"
#include "qstringlist.h"
#include "qmap.h"
#endif // QT_H

inline QVariant::Type QVariant::type() const
{
    return d->typ;
}

inline bool QVariant::isValid() const
{
    return (d->typ != Invalid);
}

inline QValueListConstIterator<QString> QVariant::stringListBegin() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QString> QVariant::stringListEnd() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->end();
}

inline QValueListConstIterator<QVariant> QVariant::listBegin() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QVariant> QVariant::listEnd() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapBegin() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->begin();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapEnd() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapFind( const QString& key ) const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->find( key );
}

Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );


#endif
