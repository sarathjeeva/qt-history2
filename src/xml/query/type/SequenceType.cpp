/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "SequenceType.h"

using namespace Patternist;

SequenceType::SequenceType()
{
}

SequenceType::~SequenceType()
{
}

bool SequenceType::matches(const SequenceType::Ptr other) const
{
    Q_ASSERT(other);

    return itemType()->xdtTypeMatches(other->itemType()) &&
           cardinality().isMatch(other->cardinality());
}

// vim: et:ts=4:sw=4:sts=4
