/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.h#23 $
**
** Definition of QGDict and QGDictIterator classes
**
** Created : 920529
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

#ifndef QGDICT_H
#define QGDICT_H

#ifndef QT_H
#include "qcollection.h"
#endif // QT_H


class QBucket;					// internal classes
class QListM_QGDictIterator;
#define QGDItList QListM_QGDictIterator


class QGDict : public QCollection		// generic dictionary class
{
friend class QGDictIterator;
public:
    uint	count() const	{ return numItems; }
    uint	size()	const	{ return vlen; }
    GCI		look( const char *key, GCI, int );

    QDataStream &read( QDataStream & );
    QDataStream &write( QDataStream & ) const;

protected:
    QGDict( uint len, bool cs, bool ck, bool th );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool	remove( const char *key );
    GCI		take( const char *key );
    void	clear();
    void	resize( uint );

    virtual int hashKey( const char * );

    void	statistics() const;

    virtual QDataStream &read( QDataStream &, GCI & );
    virtual QDataStream &write( QDataStream &, GCI ) const;

private:
    QBucket   **vec;
    uint	vlen;
    uint	numItems;
    uint	cases	: 1;
    uint	copyk	: 1;
    uint	triv	: 1;
    QGDItList  *iterators;
    QBucket    *unlink( const char * );
    void        init( uint );
};


class QGDictIterator				// generic dictionary iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
    QGDictIterator( const QGDictIterator & );
    QGDictIterator &operator=( const QGDictIterator & );
   ~QGDictIterator();

    GCI		toFirst();

    GCI		get()	 const;
    const char *getKey() const;

    GCI		operator()();
    GCI		operator++();
    GCI		operator+=(uint);

protected:
    QGDict     *dict;

private:
    QBucket    *curNode;
    uint	curIndex;
};


#endif // QGDICT_H
