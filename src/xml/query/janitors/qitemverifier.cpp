/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcommonsequencetypes_p.h"
#include "qdebug_p.h"
#include "qgenericsequencetype_p.h"
#include "qitemmappingiterator_p.h"
#include "qlistiterator_p.h"
#include "qpatternistlocale_p.h"

#include "qitemverifier_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

ItemVerifier::ItemVerifier(const Expression::Ptr &operand,
                           const ItemType::Ptr &reqType,
                           const ReportContext::ErrorCode errorCode) : SingleContainer(operand),
                                                                       m_reqType(reqType),
                                                                       m_errorCode(errorCode)
{
    Q_ASSERT(reqType);
    qDebug() << Q_FUNC_INFO;
}

void ItemVerifier::verifyItem(const Item &item, const DynamicContext::Ptr &context) const
{
    if(m_reqType->itemMatches(item))
        return;

    context->error(tr("The item %1 did not match the required type %2.")
                                    .arg(formatData(item.stringValue()),
                                         formatType(context->namePool(), m_reqType)),
                      m_errorCode,
                      this);
}

const SourceLocationReflection *ItemVerifier::actualReflection() const
{
    return m_operand->actualReflection();
}

Item ItemVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operand->evaluateSingleton(context));

    if(item)
    {
        verifyItem(item, context);
        return item;
    }
    else
        return Item();
}

Item ItemVerifier::mapToItem(const Item &item, const DynamicContext::Ptr &context) const
{
    verifyItem(item, context);
    return item;
}

Item::Iterator::Ptr ItemVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return makeItemMappingIterator<Item>(ItemVerifier::Ptr(const_cast<ItemVerifier *>(this)),
                                              m_operand->evaluateSequence(context),
                                              context);
}

SequenceType::Ptr ItemVerifier::staticType() const
{
    return makeGenericSequenceType(m_reqType, m_operand->staticType()->cardinality());
}

SequenceType::List ItemVerifier::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr ItemVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
