/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGVECTOR_H
#define QGVECTOR_H

#include "qptrcollection.h"


class Q_COMPAT_EXPORT QGVector : public QPtrCollection        // generic vector
{
friend class QGList;                                // needed by QGList::toVector
public:
#ifndef QT_NO_DATASTREAM
    QDataStream &read(QDataStream &);                // read vector from stream
    QDataStream &write(QDataStream &) const;        // write vector to stream
#endif
    virtual int compareItems(Item, Item);

protected:
    QGVector();                                        // create empty vector
    QGVector(uint size);                        // create vector with nullptrs
    QGVector(const QGVector &v);                // make copy of other vector
   ~QGVector();

    QGVector &operator=(const QGVector &v);        // assign from other vector
    bool operator==(const QGVector &v) const;

    Item         *data()    const        { return vec; }
    uint  size()    const        { return len; }
    uint  count()   const        { return numItems; }

    bool  insert(uint index, Item);                // insert item at index
    bool  remove(uint index);                        // remove item
    Item          take(uint index);                        // take out item

    void  clear();                                // clear vector
    bool  resize(uint newsize);                // resize vector

    bool  fill(Item, int flen);                // resize and fill vector

    void  sort();                                // sort vector
    int          bsearch(Item) const;                        // binary search (when sorted)

    int          findRef(Item, uint index) const;        // find exact item in vector
    int          find(Item, uint index) const;        // find equal item in vector
    uint  containsRef(Item) const;                // get number of exact matches
    uint  contains(Item) const;                // get number of equal matches

    Item          at(uint index) const                // return indexed item
    {
        Q_ASSERT(index < size());
        return vec[index];
    }

    bool insertExpand(uint index, Item);        // insert, expand if necessary

    void toList(QGList *) const;                // put items in list

#ifndef QT_NO_DATASTREAM
    virtual QDataStream &read(QDataStream &, Item &);
    virtual QDataStream &write(QDataStream &, Item) const;
#endif
private:
    Item         *vec;
    uint  len;
    uint  numItems;

};


/*****************************************************************************
  QGVector stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_COMPAT_EXPORT QDataStream &operator>>(QDataStream &, QGVector &);
Q_COMPAT_EXPORT QDataStream &operator<<(QDataStream &, const QGVector &);
#endif

#endif // QGVECTOR_H
