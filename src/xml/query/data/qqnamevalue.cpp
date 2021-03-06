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
#include "qbuiltintypes_p.h"
#include "qdebug_p.h"
#include "qxpathhelper_p.h"

#include "qqnamevalue_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

QNameValue::QNameValue(const NamePool::Ptr &np, const QName name) : m_qName(name),
                                                                     m_namePool(np)
{
    Q_ASSERT(!name.isNull());
    Q_ASSERT(m_namePool);
}

QNameValue::Ptr QNameValue::fromValue(const NamePool::Ptr &np, const QName name)
{
    Q_ASSERT(!name.isNull());
    return QNameValue::Ptr(new QNameValue(np, name));
}

QString QNameValue::stringValue() const
{
    return m_namePool->toLexical(m_qName);
}

ItemType::Ptr QNameValue::type() const
{
    return BuiltinTypes::xsQName;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
