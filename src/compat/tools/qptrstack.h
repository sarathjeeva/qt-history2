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

#ifndef QPTRSTACK_H
#define QPTRSTACK_H

#include "qglist.h"

template<class type>
class QPtrStack : protected QGList
{
public:
    QPtrStack()                                { }
    QPtrStack(const QPtrStack<type> &s) : QGList(s) { }
    ~QPtrStack()                        { clear(); }
    QPtrStack<type> &operator=(const QPtrStack<type> &s)
                        { return static_cast<QPtrStack<type> &>(QGList::operator=(s)); }
    bool  autoDelete() const                { return QPtrCollection::autoDelete(); }
    void  setAutoDelete(bool del)        { QPtrCollection::setAutoDelete(del); }
    uint  count()   const                { return QGList::count(); }
    bool  isEmpty() const                { return QGList::count() == 0; }
    void  push(const type *d)                { QGList::insertAt(0,Item(d)); }
    type *pop()                                { return static_cast<type *>(QGList::takeFirst()); }
    bool  remove()                        { return QGList::removeFirst(); }
    void  clear()                        { QGList::clear(); }
    type *top()            const                { return static_cast<type *>(QGList::cfirst()); }
          operator type *() const        { return static_cast<type *>(QGList::cfirst()); }
    type *current() const                { return static_cast<type *>(QGList::cfirst()); }

#ifdef qdoc
protected:
    virtual QDataStream& read(QDataStream&, QPtrCollection::Item&);
    virtual QDataStream& write(QDataStream&, QPtrCollection::Item) const;
#endif

private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrStack<void>::deleteItem(QPtrCollection::Item)
{
}
#endif

template<class type> inline void QPtrStack<type>::deleteItem(QPtrCollection::Item d)
{
    if (del_item) delete static_cast<type *>(d);
}

#endif // QPTRSTACK_H
