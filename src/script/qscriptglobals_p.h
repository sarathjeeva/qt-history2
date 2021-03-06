/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTGLOBALS_P_H
#define QSCRIPTGLOBALS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#ifndef QT_NO_SCRIPT

QT_BEGIN_NAMESPACE

class QScriptValue;
class QScriptValueImpl;
class QScriptClassInfo;
class QScriptEngine;
class QScriptEnginePrivate;
class QScriptContext;
class QScriptContextPrivate;

typedef QScriptValueImpl (*QScriptInternalFunctionSignature)(QScriptContextPrivate *, QScriptEnginePrivate *, QScriptClassInfo *);
typedef QScriptValue (*QScriptFunctionSignature)(QScriptContext *, QScriptEngine *);

namespace QScript {

enum Type {
    ObjectBased         = 0x20000000,
    FunctionBased       = 0x40000000,

    UndefinedType       = 1,
    NullType            = 2,
    ReferenceType       = 3,

    // Integer based
    BooleanType         =  4,
    IntegerType         =  5,
    StringType          =  6,

    // Double based
    NumberType          =  7,

    // Pointer based
    PointerType         =  8,

    // Object data based
    ObjectType          =  9 | ObjectBased,
    FunctionType        = 10 | ObjectBased | FunctionBased,
    VariantType         = 11 | ObjectBased,
    QObjectType         = 12 | ObjectBased | FunctionBased,
    QMetaObjectType     = 13 | ObjectBased | FunctionBased,

    // Types used by the runtime
    ActivationType      = 100 | ObjectBased,

    CustomType          = 1000
};

} // namespace QScript

QT_END_NAMESPACE

#endif // QT_NO_SCRIPT

#endif // QSCRIPTGLOBALS_P_H
