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

#ifndef Patternist_DerivedString_H
#define Patternist_DerivedString_H

#include <QRegExp>

#include "BuiltinTypes.h"
#include "Debug.h"
#include "ValidationError.h"
#include "XPathHelper.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Represents instances of derived @c xs:string types, such as @c
     * xs:normalizedString.
     *
     * Whitespace is a significant part for creating values from the lexical
     * space. Of course the specification is tricky here. Here's some pointers:
     *
     * - From <a href="4.3.6.1 The whiteSpace Schema Component">XML Schema Part 2: Datatypes
     *   Second Edition, 4.3.6 whiteSpace</a>:
     *   "For all ·atomic· datatypes other than string (and types
     *   ·derived· by ·restriction· from it) the value of whiteSpace is
     *   collapse and cannot be changed by a schema author; for string the
     *   value of whiteSpace is preserve; for any type ·derived· by
     *   ·restriction· from string the value of whiteSpace can be any of the
     *   three legal values."
     * - From <a href="http://www.w3.org/TR/xmlschema-1/#d0e1654">XML Schema Part 1: Structures
     *   Second Edition, 3.1.4 White Space Normalization during Validation</a>:
     *   "[Definition:]  The normalized value of an element or attribute
     *   information item is an ·initial value· whose white space, if any,
     *   has been normalized according to the value of the whiteSpace facet of
     *   the simple type definition used in its ·validatio."
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     * @todo Documentation is missing
     */
    template<const TypeOfDerivedString DerivedType>
    class DerivedString : public AtomicValue
    {
    private:
        static inline ItemType::Ptr itemType()
        {
            switch(DerivedType)
            {
                case TypeNormalizedString:  return BuiltinTypes::xsNormalizedString;
                case TypeToken:             return BuiltinTypes::xsToken;
                case TypeLanguage:          return BuiltinTypes::xsLanguage;
                case TypeNMTOKEN:           return BuiltinTypes::xsNMTOKEN;
                case TypeName:              return BuiltinTypes::xsName;
                case TypeNCName:            return BuiltinTypes::xsNCName;
                case TypeID:                return BuiltinTypes::xsID;
                case TypeIDREF:             return BuiltinTypes::xsIDREF;
                case TypeENTITY:            return BuiltinTypes::xsENTITY;
                case TypeString:            return BuiltinTypes::xsString;
            }

            Q_ASSERT_X(false, "DerivedString::itemType()", "This line is not supposed to be reached.");
            return ItemType::Ptr();
        }

        const QString m_value;

        inline DerivedString(const QString &value) : m_value(value)
        {
            qDebug() << "DerivedString ctor: " << value;
        }

        /**
         * @short This is an incomplete test for whether @p ch conforms to
         * the XML 1.0 NameChar production.
         */
        static inline bool isNameChar(const QChar &ch)
        {
            return ch.isLetter()            ||
                   ch.isDigit()             ||
                   ch == QLatin1Char('.')   ||
                   ch == QLatin1Char('-')   ||
                   ch == QLatin1Char('_')   ||
                   ch == QLatin1Char(':');
        }

        /**
         * @returns @c true if @p input is a valid @c xs:Name.
         * @see <a href="http://www.w3.org/TR/REC-xml/#NT-Name">Extensible
         * Markup Language (XML) 1.0 (Fourth Edition), [5] Name</a>
         */
        static inline bool isValidName(const QString &input)
        {
            if(input.isEmpty())
                return false;

            const QChar first(input.at(0));

            if(first.isLetter()             ||
               first == QLatin1Char('_')    ||
               first == QLatin1Char(':'))
            {
                const int len = input.length();

                if(len == 1)
                    return true;

                /* Since we've checked the first character above, we start at
                 * position 1. */
                for(int i = 1; i < len; ++i)
                {
                    if(!isNameChar(input.at(i)))
                        return false;
                }

                return true;
            }
            else
                return false;
        }

        /**
         * @returns @c true if @p input conforms to the XML 1.0 @c Nmtoken product.
         *
         * @see <a
         * href="http://www.w3.org/TR/2000/WD-xml-2e-20000814#NT-Nmtoken">Extensible
         * Markup Language (XML) 1.0 (Second Edition), [7] Nmtoken</a>
         */
        static inline bool isValidNMTOKEN(const QString &input)
        {
            const int len = input.length();

            if(len == 0)
                return false;

            for(int i = 0; i < len; ++i)
            {
                if(!isNameChar(input.at(i)))
                    return false;
            }

            return true;
        }

        /**
         * @short Performs attribute value normalization as if @p input was not
         * from a @c CDATA section.
         *
         * Each whitespace character in @p input that's not a space, such as tab
         * or new line character, is replaced with a space. This algorithm
         * differs from QString::simplified() in that it doesn't collapse
         * subsequent whitespace characters to a single one, or remove trailing
         * and leading space.
         *
         * @see <a href="http://www.w3.org/TR/REC-xml/#AVNormalize">Extensible
         * Markup Language (XML) 1.0 (Second Edition), 3.3.3 [E70]Attribute-Value Normalization<a/>
         */
        static QString attributeNormalize(const QString &input)
        {
            QString retval(input);
            const int len = retval.length();
            const QLatin1Char space(' ');

            for(int i = 0; i < len; ++i)
            {
                const QChar ati(retval.at(i));

                if(ati.isSpace() && ati != space)
                    retval[i] = space;
            }

            return retval;
        }

        static AtomicValue::Ptr error(const NamePool::Ptr &np, const QString &invalidValue)
        {
            return ValidationError::createError(QString::fromLatin1("%1 is not a valid value for "
                                                                    "type %2.").arg(formatData(invalidValue))
                                                                               .arg(formatType(np, itemType())));
        }

    public:

        /**
         * @note This function doesn't perform any cleanup/normalizaiton of @p
         * value. @p value must be a canonical value space of the type.
         *
         * If you want cleanup to be performed and/or the lexical space
         * checked, use fromLexical().
         */
        static AtomicValue::Ptr fromValue(const QString &value)
        {
            return AtomicValue::Ptr(new DerivedString(value));
        }

        /**
         * Constructs an instance from the lexical
         * representation @p lexical.
         */
        static AtomicValue::Ptr fromLexical(const NamePool::Ptr &np, const QString &lexical)
        {
            switch(DerivedType)
            {
                case TypeString:
                    return AtomicValue::Ptr(new DerivedString(lexical));
                case TypeNormalizedString:
                    return AtomicValue::Ptr(new DerivedString(attributeNormalize(lexical)));
                case TypeToken:
                    return AtomicValue::Ptr(new DerivedString(lexical.simplified()));
                case TypeLanguage:
                {
                    const QString simplified(lexical.trimmed());

                    const QRegExp validate(QLatin1String("[a-zA-Z]{1,8}(-[a-zA-Z0-9]{1,8})*"));
                    Q_ASSERT(validate.isValid());

                    if(validate.exactMatch(simplified))
                        return AtomicValue::Ptr(new DerivedString(lexical.simplified()));
                    else
                        return error(np, simplified);
                }
                case TypeNMTOKEN:
                {
                    const QString trimmed(lexical.trimmed());

                    if(isValidNMTOKEN(trimmed))
                        return AtomicValue::Ptr(new DerivedString(trimmed));
                    else
                        return error(np, trimmed);
                }
                case TypeName:
                {
                    const QString simplified(lexical.simplified());

                    if(isValidName(simplified))
                        return AtomicValue::Ptr(new DerivedString(simplified));
                    else
                        return error(np, simplified);
                }
                case TypeID:
                /* Fallthrough. */
                case TypeIDREF:
                /* Fallthrough. */
                case TypeENTITY:
                /* Fallthrough. */
                case TypeNCName:
                {
                    /* We treat xs:ID, xs:ENTITY, xs:IDREF and xs:NCName in the exact same
                     * way, except for the type annotation.
                     *
                     * We use trimmed() instead of simplified() because it's
                     * faster and whitespace isn't allowed between
                     * non-whitespace characters anyway, for these types. */
                    const QString trimmed(lexical.trimmed());

                    if(XPathHelper::isNCName(trimmed))
                        return AtomicValue::Ptr(new DerivedString(trimmed));
                    else
                        return error(np, trimmed);
                }
                default:
                {
                    Q_ASSERT_X(false, "", "This line is not supposed to be reached.");
                    return AtomicValue::Ptr();
                }
            }
        }

        virtual QString stringValue() const
        {
            return m_value;
        }

        virtual bool evaluateEBV(const PlainSharedPtr<DynamicContext> &) const
        {
             return m_value.length() > 0;
        }

        virtual ItemType::Ptr type() const
        {
            return itemType();
        }
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
