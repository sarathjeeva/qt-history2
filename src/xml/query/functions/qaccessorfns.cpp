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

#include "qanyuri_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"
#include "qitem_p.h"
#include "qqnamevalue_p.h"
#include "qatomicstring_p.h"

#include "qaccessorfns_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

Item NodeNameFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operands.first()->evaluateSingleton(context));

    if(item)
    {
        const QName name(item.asNode().name());

        if(name.isNull())
            return Item();
        else
            return toItem(QNameValue::fromValue(context->namePool(), name));
    }
    else
        return Item();
}

Item NilledFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operands.first()->evaluateSingleton(context));

    if(item && item.asNode().kind() == Node::Element)
    {
        /* We have no access to the PSVI -- always return false. */
        return CommonValues::BooleanFalse;
    }
    else
        return Item();
}

Item StringFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item item(m_operands.first()->evaluateSingleton(context));

    if(item)
        return AtomicString::fromValue(item.stringValue());
    else
        return CommonValues::EmptyString;
}

Expression::Ptr StringFN::typeCheck(const StaticContext::Ptr &context,
                                    const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    if(me.get() != this)
        return me;

    if(BuiltinTypes::xsString->xdtTypeMatches(m_operands.first()->staticType()->itemType()))
        return m_operands.first(); /* No need for string(), it's already a string. */
    else
        return me;
}

Item BaseURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item node(m_operands.first()->evaluateSingleton(context));

    if(node)
    {
        const QUrl base(node.asNode().baseURI());

        if(base.isEmpty())
            return Item();
        else if(base.isValid())
        {
            Q_ASSERT_X(!base.isRelative(), Q_FUNC_INFO,
                       "The base URI must be absolute.");
            return toItem(AnyURI::fromValue(base));
        }
        else
            return Item();
    }
    else
        return Item();
}

Item DocumentURIFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item node(m_operands.first()->evaluateSingleton(context));

    if(node)
    {
        const QUrl documentURI(node.asNode().documentURI());

        if(documentURI.isValid())
        {
            if(documentURI.isEmpty())
                return Item();
            else
            {
                Q_ASSERT_X(!documentURI.isRelative(), Q_FUNC_INFO,
                           "The document URI must be absolute.");
                return toItem(AnyURI::fromValue(documentURI));
            }
        }
        else
            return Item();
    }
    else
        return Item();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
