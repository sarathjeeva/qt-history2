/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmetatype.h"
#include "qobjectdefs.h"
#include "qbytearray.h"
#include "qvector.h"

/*!
    \class QMetaType qmetaobject.h
    \brief The QMetaType class manages named types in the meta object system.

    \internal

    \ingroup objectmodel

    The class is used to queue signals and slots connections. Register
    new types with qRegisterMetaType<T>()
*/


static const struct { const char * typeName; int type; } types[] = {
    {"void*", QMetaType::VoidStar},
    {"long", QMetaType::Long},
    {"int", QMetaType::Int},
    {"short", QMetaType::Short},
    {"char", QMetaType::Char},
    {"ulong", QMetaType::ULong},
    {"unsigned long", QMetaType::ULong},
    {"uint", QMetaType::UInt},
    {"unsigned int", QMetaType::UInt},
    {"ushort", QMetaType::UShort},
    {"unsigned short", QMetaType::UShort},
    {"uchar", QMetaType::UChar},
    {"unsigned char", QMetaType::UChar},
    {"bool", QMetaType::Bool},
    {"float", QMetaType::Float},
    {"double", QMetaType::Double},
    {"QChar", QMetaType::QChar},
    {"QByteArray", QMetaType::QByteArray},
    {"QString", QMetaType::QString},
    {"void", QMetaType::Void},
    {"", QMetaType::Void},
    {0, QMetaType::Void}
};

class QCustomTypeInfo
{
public:
    QCustomTypeInfo() : typeName(0, '\0'), constr(0), destr(0), saveOp(0), loadOp(0) {}
    inline void setData(const char *tname, QMetaType::Constructor cp, QMetaType::Destructor de)
    { typeName = tname; constr = cp; destr = de; }
    inline void setData(QMetaType::Constructor cp, QMetaType::Destructor de)
    { constr = cp; destr = de; }
    inline void setOperators(QMetaType::SaveOperator sOp, QMetaType::LoadOperator lOp)
    { saveOp = sOp; loadOp = lOp; }

    QByteArray typeName;
    QMetaType::Constructor constr;
    QMetaType::Destructor destr;
    QMetaType::SaveOperator saveOp;
    QMetaType::LoadOperator loadOp;
};

Q_GLOBAL_STATIC(QVector<QCustomTypeInfo>, customTypes)

void QMetaType::registerStreamOperators(const char *typeName, SaveOperator saveOp,
                                        LoadOperator loadOp)
{
    int idx = type(typeName);
    if (!idx)
        return;
    QVector<QCustomTypeInfo> *ct = customTypes();
    if (!ct)
        return;
    (*ct)[idx - User].setOperators(saveOp, loadOp);
}

/*!
   Returns the type name associated with the given \a type, or 0 if no
   matching type was found. The returned pointer must not be deleted.
 */
const char *QMetaType::typeName(int type)
{
    if (type >= User) {
        if (!isRegistered(type))
            return 0;
        const QVector<QCustomTypeInfo> * const ct = customTypes();
        if (!ct)
            return 0;
        return ct->at(type - User).typeName.constData();
    }
    int i = 0;
    while (types[i].typeName) {
        if (types[i].type == type)
            return types[i].typeName;
        ++i;
    }
    return 0;
}

/*! \internal
    Registers a user type for marshalling, with \a typeName, a \a
    destructor, and a \a constructor. Returns the type's handle,
    or -1 if the type could not be registered.
 */
int QMetaType::registerType(const char *typeName, Destructor destructor,
                            Constructor constructor)
{
    QVector<QCustomTypeInfo> *ct = customTypes();
    static int currentIdx = User;
    if (!ct || !typeName || !destructor || !constructor)
        return -1;
    int idx = type(typeName);
    if (idx) {
        if (idx < User) {
            qWarning("cannot re-register basic type '%s'", typeName);
            return -1;
        }
        (*ct)[idx - User].setData(constructor, destructor);
    } else {
        idx = currentIdx++;
        ct->resize(ct->count() + 1);
        (*ct)[idx - User].setData(typeName, constructor, destructor);
    }
    return idx;
}

/*!
    Returns true if the custom datatype with ID \a type is registered;
    otherwise returns false.
 */
bool QMetaType::isRegistered(int type)
{
    const QVector<QCustomTypeInfo> * const ct = customTypes();
    return (type >= User) && (ct && ct->count() > type - User);
}

/*!
    Returns a handle to the type called \a typeName, or 0 if there is
    no such type.
 */
int QMetaType::type(const char *typeName)
{
    if (!typeName)
        return 0;
    int i = 0;
    while (types[i].typeName && strcmp(typeName, types[i].typeName))
        ++i;
    if (!types[i].type) {
        const QVector<QCustomTypeInfo> * const ct = customTypes();
        for (int v = 0; ct && v < ct->count(); ++v) {
            if (strcmp(ct->at(v).typeName, typeName) == 0)
                return v + User;
        }
    }
    return types[i].type;
}

bool QMetaType::save(QDataStream &stream, int type, const void *data)
{
    // FIXME - also stream simple types?
    if (!data || !isRegistered(type))
        return false;
    const QVector<QCustomTypeInfo> * const ct = customTypes();
    if (!ct)
        return false;
    QMetaType::SaveOperator saveOp = ct->at(type - User).saveOp;
    if (!saveOp)
        return false;
    saveOp(stream, data);
    return true;
}

bool QMetaType::load(QDataStream &stream, int type, void *data)
{
    // FIXME - also stream simple types?
    if (!data || !isRegistered(type))
        return false;
    const QVector<QCustomTypeInfo> * const ct = customTypes();
    if (!ct)
        return false;
    QMetaType::LoadOperator loadOp = ct->at(type - User).loadOp;
    if (!loadOp)
        return false;
    loadOp(stream, data);
    return true;
}

/*
  Returns a copy of \a copy, assuming it is of type \a type. If \a
  copy is zero, creates a default type.
 */
void *QMetaType::construct(int type, const void *copy)
{
    if (copy) {
        switch(type) {
        case QMetaType::VoidStar:
            return new void *(*static_cast<void* const *>(copy));
        case QMetaType::Long:
            return new long(*static_cast<const long*>(copy));
        case QMetaType::Int:
            return new int(*static_cast<const int*>(copy));
        case QMetaType::Short:
            return new short(*static_cast<const short*>(copy));
        case QMetaType::Char:
            return new char(*static_cast<const char*>(copy));
        case QMetaType::ULong:
            return new ulong(*static_cast<const ulong*>(copy));
        case QMetaType::UInt:
            return new uint(*static_cast<const uint*>(copy));
        case QMetaType::UShort:
            return new ushort(*static_cast<const ushort*>(copy));
        case QMetaType::UChar:
            return new uchar(*static_cast<const uchar*>(copy));
        case QMetaType::Bool:
            return new bool(*static_cast<const bool*>(copy));
        case QMetaType::Float:
            return new float(*static_cast<const float*>(copy));
        case QMetaType::Double:
            return new double(*static_cast<const double*>(copy));
        case QMetaType::QChar:
            return new ::QChar(*static_cast<const ::QChar*>(copy));
        case QMetaType::QByteArray:
            return new ::QByteArray(*static_cast<const ::QByteArray*>(copy));
        case QMetaType::QString:
            return new ::QString(*static_cast<const ::QString*>(copy));
        case QMetaType::Void:
            return 0;
        default:
            ;
        }
    } else {
        switch(type) {
        case QMetaType::VoidStar:
            return new void *;
        case QMetaType::Long:
            return new long;
        case QMetaType::Int:
            return new int;
        case QMetaType::Short:
            return new short;
        case QMetaType::Char:
            return new char;
        case QMetaType::ULong:
            return new ulong;
        case QMetaType::UInt:
            return new uint;
        case QMetaType::UShort:
            return new ushort;
        case QMetaType::UChar:
            return new uchar;
        case QMetaType::Bool:
            return new bool;
        case QMetaType::Float:
            return new float;
        case QMetaType::Double:
            return new double;
        case QMetaType::QChar:
            return new ::QChar;
        case QMetaType::QByteArray:
            return new ::QByteArray;
        case QMetaType::QString:
            return new ::QString;
        case QMetaType::Void:
            return 0;
        default:
            ;
        }
    }

    const QVector<QCustomTypeInfo> * const ct = customTypes();
    if (type >= User && (ct && ct->count() > type - User))
        return ct->at(type - User).constr(copy);
    return 0;
}

/*!
    Destroys the \a data, assuming it is of the \a type given.
 */
void QMetaType::destroy(int type, void *data)
{
    if (!data)
        return;
    switch(type) {
    case QMetaType::VoidStar:
        delete static_cast<void**>(data);
        break;
    case QMetaType::Long:
        delete static_cast<long*>(data);
        break;
    case QMetaType::Int:
        delete static_cast<int*>(data);
        break;
    case QMetaType::Short:
        delete static_cast<short*>(data);
        break;
    case QMetaType::Char:
        delete static_cast<char*>(data);
        break;
    case QMetaType::ULong:
        delete static_cast<ulong*>(data);
        break;
    case QMetaType::UInt:
        delete static_cast<uint*>(data);
        break;
    case QMetaType::UShort:
        delete static_cast<ushort*>(data);
        break;
    case QMetaType::UChar:
        delete static_cast<uchar*>(data);
        break;
    case QMetaType::Bool:
        delete static_cast<bool*>(data);
        break;
    case QMetaType::Float:
        delete static_cast<float*>(data);
        break;
    case QMetaType::Double:
        delete static_cast<double*>(data);
        break;
    case QMetaType::QChar:
        delete static_cast< ::QChar*>(data);
        break;
    case QMetaType::QByteArray:
        delete static_cast< ::QByteArray*>(data);
        break;
    case QMetaType::QString:
        delete static_cast< ::QString*>(data);
        break;
    case QMetaType::Void:
        break;
    default:
        {
            const QVector<QCustomTypeInfo> * const ct = customTypes();
            if (type >= User && (ct && ct->count() > type - User))
                ct->at(type - User).destr(data);
            break;
        }
    }
}
