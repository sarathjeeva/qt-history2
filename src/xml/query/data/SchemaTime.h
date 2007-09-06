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

#ifndef Patternist_Time_H
#define Patternist_Time_H

#include "AbstractDateTime.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:time type.
     *
     * The header file for this class was orignally called Time.h, but this
     * clashed with a system header on MinGW.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class SchemaTime : public AbstractDateTime
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static SchemaTime::Ptr fromLexical(const QString &string);
        static SchemaTime::Ptr fromDateTime(const QDateTime &dt);

        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;
        virtual Item fromValue(const QDateTime &dt) const;

    protected:
        friend class CommonValues;

        SchemaTime(const QDateTime &dateTime);
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
