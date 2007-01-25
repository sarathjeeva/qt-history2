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

#include "qscriptengine.h"
#include "qscriptcontext.h"

#include "qscriptengine_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

Object::Object(QScriptEngine *eng, QScriptClassInfo *classInfo):
    Core(eng), m_classInfo(classInfo)
{
    newObject(&publicPrototype, eng->nullValue());
}

Object::~Object()
{
}

void Object::initialize()
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(engine());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng_p->createFunction(method_toLocaleString, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("hasOwnProperty"),
                                eng_p->createFunction(method_hasOwnProperty, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("isPrototypeOf"),
                                eng_p->createFunction(method_isPrototypeOf, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("propertyIsEnumerable"),
                                eng_p->createFunction(method_propertyIsEnumerable, 1, m_classInfo), flags);

    publicPrototype.setProperty(QLatin1String("__defineGetter__"),
                                eng_p->createFunction(method_defineGetter, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("__defineSetter__"),
                                eng_p->createFunction(method_defineSetter, 2, m_classInfo), flags);
}

void Object::execute(QScriptContext *context)
{
    QScriptValue value;

    if (context->argumentCount() > 0)
        value = context->argument(0).toObject();
    else
        value.invalidate();

    if (! value.isValid())
        newObject(&value);

    context->setReturnValue(value);
}

void Object::newObject(QScriptValue *result, const QScriptValue &proto)
{
    QScriptEnginePrivate::get(engine())->newObject(result, proto, classInfo());
}

QScriptValue Object::method_toString(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue glo = eng->globalObject();
    QString s = QLatin1String("[object ");
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->objectValue() == QScriptValueImpl::get(glo)->objectValue())
        s += QLatin1String("global");
    else
        s += QScriptValueImpl::get(self)->classInfo()->name();
    s += QLatin1String("]");
    return (QScriptValue(eng, s));
}

QScriptValue Object::method_toLocaleString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_toString(eng, classInfo);
}

QScriptValue Object::method_valueOf(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return (context->thisObject());
}

QScriptValue Object::method_hasOwnProperty(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();

    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValue arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = QScriptValueImpl::get(arg)->stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValue base;
        QScriptValue self = context->thisObject();
        if (QScriptValueImpl::get(self)->resolve(id, &member, &base, QScriptValue::ResolveLocal))
            result = true;
    }

    return (QScriptValue(eng, result));
}

QScriptValue Object::method_isPrototypeOf(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValue arg = context->argument(0);

        if (arg.isObject()) {
            QScriptValue proto = arg.prototype();

            if (proto.isObject()) {
                QScriptValue self = context->thisObject();
                result = QScriptValueImpl::get(self)->objectValue() == QScriptValueImpl::get(proto)->objectValue();
            }
        }
    }

    return (QScriptValue(eng, result));
}

QScriptValue Object::method_propertyIsEnumerable(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    bool result = false;

    if (context->thisObject().isObject() && (context->argumentCount() > 0)) {
        QScriptValue arg = context->argument(0);

        QScriptNameIdImpl *id = 0;
        if (arg.isString())
            id = QScriptValueImpl::get(arg)->stringValue();

        if (! id || ! id->unique) {
            QString str = arg.toString();
            id = eng->nameId(str);
        }

        QScript::Member member;
        QScriptValue base;
        QScriptValue self = context->thisObject();
        if (QScriptValueImpl::get(self)->resolve(id, &member, &base, QScriptValue::ResolveLocal)) {
            result = ! member.dontEnum();
            if (result) {
                QScriptValue tmp;
                QScriptValueImpl::get(base)->get(member, &tmp);
                result = tmp.isValid();
            }
        }
    }

    return (QScriptValue(eng, result));
}

QScriptValue Object::method_defineGetter(QScriptEngine *eng,
                                         QScriptClassInfo *)
{
    QScriptContext *ctx = eng->currentContext();
    QString propertyName = ctx->argument(0).toString();
    QScriptValue getter = ctx->argument(1);
    if (!getter.isFunction())
        return ctx->throwError(QLatin1String("getter must be a function"));
    ctx->thisObject().setProperty(propertyName, getter, QScriptValue::PropertyGetter);
    return eng->undefinedValue();
}

QScriptValue Object::method_defineSetter(QScriptEngine *eng,
                                         QScriptClassInfo *)
{
    QScriptContext *ctx = eng->currentContext();
    QString propertyName = ctx->argument(0).toString();
    QScriptValue setter = ctx->argument(1);
    if (!setter.isFunction())
        return ctx->throwError(QLatin1String("setter must be a function"));
    ctx->thisObject().setProperty(propertyName, setter, QScriptValue::PropertySetter);
    return eng->undefinedValue();
}

} } // namespace QScript::Ecma
