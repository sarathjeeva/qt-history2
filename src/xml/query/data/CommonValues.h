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

#ifndef Patternist_CommonValues_H
#define Patternist_CommonValues_H

#include "DayTimeDuration.h"
#include "YearMonthDuration.h"
#include "EmptyIterator.h"

QT_BEGIN_HEADER 

namespace Patternist
{

    /**
     * @short A collection of common values.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing/incomplete
     */
    class CommonValues
    {
    public:
        /**
         * An empty, zero-length string.
         *
         * @note It is not @c null, but empty.
         */
        static const AtomicValue::Ptr EmptyString;

        /**
         * The string "true", the lexical representation of
         * @c xs:boolean's value @c true.
         */
        static const AtomicValue::Ptr TrueString;

        /**
         * The string "false", the lexical representation of
         * @c xs:boolean's value @c false.
         */
        static const AtomicValue::Ptr UntypedAtomicFalse;

        /**
         * The string "true", the lexical representation of
         * @c xs:boolean's value @c true.
         */
        static const AtomicValue::Ptr UntypedAtomicTrue;

        /**
         * The string "false", the lexical representation of
         * @c xs:boolean's value @c false.
         */
        static const AtomicValue::Ptr FalseString;

        /**
         * @returns a Boolean instance carrying the boolean value @c true.
         * Use this value instead of Boolean::fromValue() if you
         * know what boolean value you need.
         */
        static const AtomicValue::Ptr BooleanTrue;

        /**
         * @returns a Boolean instance carrying the boolean value @c true.
         * Use this value instead of Boolean::fromValue() if you
         * know what boolean value you need.
         */
        static const AtomicValue::Ptr BooleanFalse;

        /**
         * Not-a-Numeric typed as @c xs:double.
         */
        static const AtomicValue::Ptr DoubleNaN;

        /**
         * Not-a-Number typed as @c xs:float, <tt>xs:float("NaN")</tt>.
         */
        static const AtomicValue::Ptr FloatNaN;

        /**
         * Zero(0) typed as @c xs:integer, <tt>xs:integer("0")</tt>.
         */
        static const Item IntegerZero;

        /**
         * An empty, "", @c xs:anyURI.
         */
        static const AtomicValue::Ptr EmptyAnyURI;

        /**
         * @see emptySequence
         */
        static const EmptyIterator<Item>::Ptr emptyIterator;

        /**
         * <tt>xs:float("-INF")</tt>
         */
        static const AtomicValue::Ptr NegativeInfFloat;

        /**
         * <tt>xs:float("INF")</tt>
         */
        static const AtomicValue::Ptr InfFloat;

        /**
         * <tt>xs:double("-INF")</tt>
         */
        static const AtomicValue::Ptr NegativeInfDouble;

        /**
         * <tt>xs:double("INF")</tt>
         */
        static const AtomicValue::Ptr InfDouble;

        /**
         * <tt>xs:float("1")</tt>
         */
        static const AtomicValue::Ptr FloatOne;
        /**
         * <tt>xs:double("1")</tt>
         */
        static const AtomicValue::Ptr DoubleOne;
        /**
         * <tt>xs:decimal("1")</tt>
         */
        static const AtomicValue::Ptr DecimalOne;

        /**
         * <tt>xs:integer("1")</tt>
         */
        static const Item IntegerOne;

        /**
         * <tt>xs:integer("-1")</tt>
         */
        static const Item IntegerOneNegative;

        /**
         * <tt>xs:double("0")</tt>
         */
        static const AtomicValue::Ptr DoubleZero;

        /**
         * <tt>xs:float("0")</tt>
         */
        static const AtomicValue::Ptr FloatZero;
        /**
         * <tt>xs:integer("0")</tt>
         */
        static const AtomicValue::Ptr DecimalZero;

        /**
         * The @c xs:dayTimeDuration value PT0S
         */
        static const DayTimeDuration::Ptr DayTimeDurationZero;

        /**
         * The @c xs:yearMonthDuration value P0M
         */
        static const DayTimeDuration::Ptr YearMonthDurationZero;

    private:
        /**
         * The constructor is private because this class is not meant to be instantiated,
         * but should only be used via its static const members.
         */
        inline CommonValues();

        Q_DISABLE_COPY(CommonValues)
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
