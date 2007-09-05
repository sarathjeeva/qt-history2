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

#include "AtomicType.h"
#include "BuiltinTypes.h"

#include "AnyItemType.h"

using namespace Patternist;

AnyItemType::AnyItemType()
{
}

AnyItemType::~AnyItemType()
{
}

bool AnyItemType::itemMatches(const Item &) const
{
    return true;
}

bool AnyItemType::xdtTypeMatches(const ItemType::Ptr &) const
{
    return true;
}

QString AnyItemType::displayName(const NamePool::Ptr &) const
{
    return QLatin1String("item()");
}

ItemType::Ptr AnyItemType::xdtSuperType() const
{
    return ItemType::Ptr();
}

bool AnyItemType::isNodeType() const
{
    return false;
}

bool AnyItemType::isAtomicType() const
{
    return false;
}

ItemType::Ptr AnyItemType::atomizedType() const
{
    return BuiltinTypes::xsAnyAtomicType;
}

// vim: et:ts=4:sw=4:sts=4
