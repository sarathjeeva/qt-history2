/****************************************************************************
 * **
 * ** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
 * **
 * ** This file is part of the Patternist project on Trolltech Labs.
 * **
 * ** $TROLLTECH_GPL_LICENSE$
 * **
 * ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * **
 * ****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_PullBridge_p_h
#define Patternist_PullBridge_p_h

#include <QPair>
#include <QStack>

#include "Item.h"
#include "Iterator.h"
#include "qabstractxmlpullprovider.h"

QT_BEGIN_HEADER

class PullBridge : public QAbstractXmlPullProvider
{
public:
    inline PullBridge(const Patternist::Item::Iterator::Ptr &it) : m_current(StartOfInput)
    {
        Q_ASSERT(it);
        m_iterators.push(qMakePair(StartOfInput, it));
    }

    virtual Event next();
    virtual Event current() const;
    virtual QXmlName name() const;
    virtual QVariant atomicValue() const;
    virtual QString stringValue() const;

private:
    typedef QStack<QPair<Event, Patternist::Item::Iterator::Ptr> > IteratorStack;
    IteratorStack       m_iterators;
    Patternist::Item    m_item;
    Event               m_current;
};

QT_END_HEADER

#endif
// vim: et:ts=4:sw=4:sts=4
