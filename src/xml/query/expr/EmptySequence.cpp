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

#include "CommonValues.h"
#include "CommonSequenceTypes.h"
#include "EmptySequenceType.h"
#include "ListIterator.h"

#include "EmptySequence.h"

using namespace Patternist;

Expression::Ptr EmptySequence::create(const Expression *const replacementFor,
                                      const StaticContext::Ptr &context)
{
    Q_ASSERT(replacementFor);
    Q_ASSERT(context);

    EmptySequence *const retval = new EmptySequence();
    context->addLocation(retval, context->locationFor(replacementFor));
    return Expression::Ptr(retval);
}

Item::Iterator::Ptr EmptySequence::evaluateSequence(const DynamicContext::Ptr &) const
{
    return CommonValues::emptyIterator;
}

Item EmptySequence::evaluateSingleton(const DynamicContext::Ptr &) const
{
    return Item();
}

void EmptySequence::evaluateToSequenceReceiver(const DynamicContext::Ptr &) const
{
}

ItemType::Ptr EmptySequence::type() const
{
    return CommonSequenceTypes::Empty;
}

SequenceType::Ptr EmptySequence::staticType() const
{
    return CommonSequenceTypes::Empty;
}

bool EmptySequence::evaluateEBV(const DynamicContext::Ptr &) const
{
    return false;
}

QString EmptySequence::stringValue() const
{
    return QString();
}

ExpressionVisitorResult::Ptr EmptySequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID EmptySequence::id() const
{
    return IDEmptySequence;
}

// vim: et:ts=4:sw=4:sts=4
