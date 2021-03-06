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

#include "qdebug_p.h"
#include "qlistiterator_p.h"

#include "qexpressionvariablereference_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

ExpressionVariableReference::ExpressionVariableReference(const VariableSlotID slotP,
                                                         const VariableDeclaration::Ptr varDecl) : VariableReference(slotP)
                                                                                                 , m_varDecl(varDecl)
{
    qDebug() << Q_FUNC_INFO << "slot: " << slotP;
}

bool ExpressionVariableReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return context->expressionVariable(slot())->evaluateEBV(context);
}

Item ExpressionVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return context->expressionVariable(slot())->evaluateSingleton(context);
}

Item::Iterator::Ptr ExpressionVariableReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return context->expressionVariable(slot())->evaluateSequence(context);
}
Expression::Ptr ExpressionVariableReference::typeCheck(const StaticContext::Ptr &context,
                                                       const SequenceType::Ptr &reqType)
{
    if(m_varDecl->canSourceRewrite)
        return m_varDecl->expression()->typeCheck(context, reqType);
    else
        return VariableReference::typeCheck(context, reqType);
}

Expression::ID ExpressionVariableReference::id() const
{
    return IDExpressionVariableReference;
}

ExpressionVisitorResult::Ptr ExpressionVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

SequenceType::Ptr ExpressionVariableReference::staticType() const
{
    return m_varDecl->expression()->staticType();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
