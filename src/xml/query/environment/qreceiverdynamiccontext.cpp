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

#include "qreceiverdynamiccontext_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

ReceiverDynamicContext::
ReceiverDynamicContext(const DynamicContext::Ptr &prevContext,
                       const SequenceReceiver::Ptr &receiver) : DelegatingDynamicContext(prevContext),
                                                                m_receiver(receiver)
{
    Q_ASSERT(receiver);
}

SequenceReceiver::Ptr ReceiverDynamicContext::outputReceiver() const
{
    return m_receiver;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
