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

#include "qmetaobject.h"
#include "qmetatype.h"
#include "qobject.h"
#include <qcoreapplication.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qcorevariant.h>
#include <qhash.h>
#include <ctype.h>

/*!
    \class QMetaObject qmetaobject.h

    \brief The QMetaObject class contains meta information about Qt
    objects.

    \ingroup objectmodel

    The Qt Meta Object System in Qt is responsible for the signals and
    slots inter-object communication mechanism, runtime type
    information, and the Qt property system. A single QMetaObject
    instance is created for each QObject subclass that is used in an
    application, and this instance stores all the meta information for
    the QObject subclass.

    This class is not normally required for application programming,
    but it is useful if you write meta applications, such as scripting
    engines or GUI builders.

    The functions you are most likely to find useful are these:
    \list
    \i className() returns the name of a class.
    \i superClass() returns the super-class's meta object.
    \i memberCount(), member() provide information
       about a class's meta members (signals, slots and methods).
    \i enumeratorCount() and enumerator() provide information about
       a class's enumerators.
    \i propertyCount() and property() provide information about a
       class's properties.
    \endlist

    The index functions indexOfMember(), indexOfEnumerator(), and
    indexOfProperty() map names of member functions, enumerators, or
    properties to indexes in the meta object. For example, Qt uses
    indexOfMember() internally when you connect a signal to a slot.

    Classes can also have a list of name--value pairs of additional
    \link QMetaClassInfo class information\endlink. The number of
    pairs is returned by classInfoCount(), single pairs are returned
    by classInfo(), and you can search for pairs with
    indexOfClassInfo().

    \sa \link moc.html moc (Meta QObject Compiler)\endlink

*/

/*!
    \enum QMetaObject::Call

    \internal

    \value InvokeSlot
    \value EmitSignal
    \value ReadProperty
    \value WriteProperty
    \value ResetProperty
    \value QueryPropertyDesignable
    \value QueryPropertyScriptable
    \value QueryPropertyStored
    \value QueryPropertyEditable
*/

/*!
    \fn void QMetaObject::activate(QObject *obj, int signal_index, void **argv)

    \internal
*/

/*!
    \enum QMetaMember::Access

    \internal
*/

// do not touch without touching the moc as well
enum ProperyFlags  {
    Invalid = 0x00000000,
    Readable = 0x00000001,
    Writable = 0x00000002,
    Resetable = 0x00000004,
    EnumOrFlag = 0x00000008,
    StdCppSet = 0x00000100,
    Override = 0x00000200,
    Designable = 0x00001000,
    ResolveDesignable = 0x00002000,
    Scriptable = 0x00004000,
    ResolveScriptable = 0x00008000,
    Stored = 0x00010000,
    ResolveStored = 0x00020000,
    Editable = 0x00040000,
    ResolveEditable = 0x00080000
};

enum MethodFlags  {
    AccessPrivate = 0x00,
    AccessProtected = 0x01,
    AccessPublic = 0x02,
    AccessMask = 0x03, //mask

    MemberMethod = 0x00,
    MemberSignal = 0x04,
    MemberSlot = 0x08,
    MemberTypeMask = 0x0c,

    MemberCompatibility = 0x10,
    MemberCloned = 0x20,
    MemberScriptable = 0x40
};

struct QMetaObjectPrivate
{
    int revision;
    int className;
    int classInfoCount, classInfoData;
    int memberCount, memberData;
    int propertyCount, propertyData;
    int enumeratorCount, enumeratorData;
};

static inline const QMetaObjectPrivate *priv(const uint* data)
{ return reinterpret_cast<const QMetaObjectPrivate*>(data); }


/*!
    \fn const char *QMetaObject::className() const

    Returns the class name.

    \sa superClass()
*/

/*!
    \fn QMetaObject *QMetaObject::superClass() const

    Returns the meta object of the super-class, or 0 if there is no
    such object.
*/

/*!
    \internal

    Returns \a obj if object \a obj inherits from this meta
    object; otherwise returns 0.
*/
QObject *QMetaObject::cast(QObject *obj) const
{
    if (obj) {
        const QMetaObject *m = obj->metaObject();
        do {
            if (m == this)
                return const_cast<QObject*>(obj);
        } while ((m = m->d.superdata));
    }
    return 0;
}

#ifndef QT_NO_TRANSLATION
/*!
    \internal

    Forwards a tr() call from the \c Q_OBJECT macro to the QApplication.
*/
QString QMetaObject::tr(const char *s, const char *c) const
{
    if (QCoreApplication::instance())
        return QCoreApplication::instance()->translate(d.stringdata, s, c, QCoreApplication::DefaultCodec);
    else
        return QString::fromLatin1(s);
}
/*!
    \internal

    Forwards a trUtf8() call from the \c Q_OBJECT macro to the
    QApplication.
*/
QString QMetaObject::trUtf8(const char *s, const char *c) const
{
    if (QCoreApplication::instance())
        return QCoreApplication::instance()->translate(d.stringdata, s, c, QCoreApplication::UnicodeUTF8);
    else
        return QString::fromUtf8(s);
}
#endif // QT_NO_TRANSLATION


/*!
    Returns the member offset for this class; i.e. the index position of
    this class's first member. The offset is the sum of all the members in
    the class's super-classes (which is always positive since QObject
    has the deleteLater() slot and a destroyed() signal).
 */
int QMetaObject::memberOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->memberCount;
        m = m->d.superdata;
    }
    return offset;
}


/*!
    Returns the enumerator offset for this class; i.e. the index
    position of this class's first enumerator. If the class has no
    super-classes with enumerators, the offset is 0; otherwise the
    offset is the sum of all the enumerators in the class's
    super-classes.
 */
int QMetaObject::enumeratorOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->enumeratorCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the property offset for this class; i.e. the index
    position of this class's first property. The offset is the sum of
    all the properties in the class's super-classes (which is always
    positive since QObject has the name() property).
 */
int QMetaObject::propertyOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->propertyCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the class information offset for this class; i.e. the
    index position of this class's first class information item. If
    the class has no super-classes with class information, the offset
    is 0; otherwise the offset is the sum of all the class information
    items in the class's super-classes.
 */
int QMetaObject::classInfoOffset() const
{
    int offset = 0;
    const QMetaObject *m = d.superdata;
    while (m) {
        offset += priv(m->d.data)->classInfoCount;
        m = m->d.superdata;
    }
    return offset;
}

/*!
    Returns the number of slots in this class.

    \sa slot()
*/
int QMetaObject::memberCount() const
{
    int n = priv(d.data)->memberCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->memberCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of enumerators in this class.

    \sa enumerator()
*/
int QMetaObject::enumeratorCount() const
{
    int n = priv(d.data)->enumeratorCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->enumeratorCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of properties in this class.

    \sa property()
*/
int QMetaObject::propertyCount() const
{
    int n = priv(d.data)->propertyCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->propertyCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Returns the number of items of class information in this class.
*/
int QMetaObject::classInfoCount() const
{
    int n = priv(d.data)->classInfoCount;
    const QMetaObject *m = d.superdata;
    while (m) {
        n += priv(m->d.data)->classInfoCount;
        m = m->d.superdata;
    }
    return n;
}

/*!
    Finds \a member and returns its index; otherwise returns -1.

    \sa member(), memberCount()
*/
int QMetaObject::indexOfMember(const char *member) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->memberCount-1; i >= 0; --i)
            if (strcmp(member, m->d.stringdata
                       + m->d.data[priv(m->d.data)->memberData + 5*i]) == 0) {
                i += m->memberOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Finds \a signal and returns its index; otherwise returns -1.

    \sa indexOfMember(), member(), memberCount()
*/
int QMetaObject::indexOfSignal(const char *signal) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->memberCount-1; i >= 0; --i)
            if ((m->d.data[priv(m->d.data)->memberData + 5*i + 4] & MemberTypeMask) == MemberSignal
                && strcmp(signal, m->d.stringdata
                          + m->d.data[priv(m->d.data)->memberData + 5*i]) == 0) {
                i += m->memberOffset();
                break;
            }
        m = m->d.superdata;
    }
#ifndef QT_NO_DEBUG
    if (i >= 0 && m->d.superdata) {
        int conflict = m->d.superdata->indexOfMember(signal);
        if (conflict >= 0)
            qWarning("QMetaObject::indexOfSignal:%s: Conflict with %s::%s",
                      m->d.stringdata, m->d.superdata->d.stringdata, signal);
    }
#endif
    return i;
}

/*!
    Finds \a slot and returns its index; otherwise returns -1.

    \sa indexOfMember(), member(), memberCount()
*/
int QMetaObject::indexOfSlot(const char *slot) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->memberCount-1; i >= 0; --i)
            if ((m->d.data[priv(m->d.data)->memberData + 5*i + 4] & MemberTypeMask) == MemberSlot
                && strcmp(slot, m->d.stringdata
                       + m->d.data[priv(m->d.data)->memberData + 5*i]) == 0) {
                i += m->memberOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}



/*!
    Finds enumerator \a name and returns its index; otherwise returns
    -1.

    \sa enumerator(), enumeratorCount()
*/
int QMetaObject::indexOfEnumerator(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    int scope = 0;
    const char *qualified_name = name;
    const char *s = name;
    while (*s  && *s != ':')
        ++s;
    if (*s && *(s+1)==':') {
        scope = s - name;
        name += scope + 2;
    }
    while (m && i < 0) {
        for (i = priv(m->d.data)->enumeratorCount-1; i >= 0; --i) {
            if ((!scope || strncmp(qualified_name, m->d.stringdata, scope) == 0)
                && strcmp(name, m->d.stringdata
                          + m->d.data[priv(m->d.data)->enumeratorData + 4*i]) == 0) {
                i += m->enumeratorOffset();
                break;
            }
        }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Finds property \a name and returns its index; otherwise returns
    -1.

    \sa property(), propertyCount()
*/
int QMetaObject::indexOfProperty(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->propertyCount-1; i >= 0; --i)
            if (strcmp(name, m->d.stringdata
                       + m->d.data[priv(m->d.data)->propertyData + 3*i]) == 0) {
                i += m->propertyOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Finds class information item \a name and returns its index;
    otherwise returns -1.

    \sa classInfo(), classInfoCount()
*/
int QMetaObject::indexOfClassInfo(const char *name) const
{
    int i = -1;
    const QMetaObject *m = this;
    while (m && i < 0) {
        for (i = priv(m->d.data)->classInfoCount-1; i >= 0; --i)
            if (strcmp(name, m->d.stringdata
                       + m->d.data[priv(d.data)->classInfoData + 2*i]) == 0) {
                i += m->classInfoOffset();
                break;
            }
        m = m->d.superdata;
    }
    return i;
}

/*!
    Returns the meta data for the member with the given \a index.
*/
QMetaMember QMetaObject::member(int index) const
{
    int i = index;
    i -= memberOffset();
    if (i < 0 && d.superdata)
        return d.superdata->member(index);

    QMetaMember result;
    if (i >= 0 && i <= priv(d.data)->memberCount) {
        result.mobj = this;
        result.handle = priv(d.data)->memberData + 5*i;
    }
    return result;
}

/*!
    Returns the meta data for the enumerator with the given \a index.

    \sa indexOfEnumerator()
*/
QMetaEnum QMetaObject::enumerator(int index) const
{
    int i = index;
    i -= enumeratorOffset();
    if (i < 0 && d.superdata)
        return d.superdata->enumerator(index);

    QMetaEnum result;
    if (i >= 0 && i <= priv(d.data)->enumeratorCount) {
        result.mobj = this;
        result.handle = priv(d.data)->enumeratorData + 4*i;
    }
    return result;
}

/*!
    Returns the meta data for the property with the given \a index.

    \sa indexOfProperty()
*/
QMetaProperty QMetaObject::property(int index) const
{
    int i = index;
    i -= propertyOffset();
    if (i < 0 && d.superdata)
        return d.superdata->property(index);

    QMetaProperty result;
    if (i >= 0 && i <= priv(d.data)->propertyCount) {
        int handle = priv(d.data)->propertyData + 3*i;
        int flags = d.data[handle + 2];
        const char *name = d.stringdata + d.data[handle];
        const char *type = d.stringdata + d.data[handle + 1];
        if ((flags & Override) && d.superdata){
            result = property(d.superdata->indexOfProperty(name));
            if (qstrcmp(result.typeName(), type)) // type missmatch, no override
                ::memset(&result, 0, sizeof(QMetaProperty));
        }
        if (flags & EnumOrFlag) {
            result.menum = enumerator(indexOfEnumerator(type));
        }
        if (flags & Readable) {
            result.mobj[ReadProperty] = this;
            result.idx[ReadProperty] = i;
        }
        if (flags & Writable) {
            result.mobj[WriteProperty] = this;
            result.idx[WriteProperty] = i;
        }
        if (flags & Resetable) {
            result.mobj[ResetProperty] = this;
            result.idx[ResetProperty] = i;
        }
        if ((flags & ResolveDesignable) == 0) {
            result.mobj[QueryPropertyDesignable] = this;
            result.idx[QueryPropertyDesignable] = i;
        }
        if ((flags & ResolveScriptable) == 0) {
            result.mobj[QueryPropertyScriptable] = this;
            result.idx[QueryPropertyScriptable] = i;
        }
        if ((flags & ResolveStored) == 0) {
            result.mobj[QueryPropertyStored] = this;
            result.idx[QueryPropertyStored] = i;
        }
        if ((flags & ResolveEditable) == 0) {
            result.mobj[QueryPropertyEditable] = this;
            result.idx[QueryPropertyEditable] = i;
        }
    }
    return result;
}

/*!
    Returns the meta data for the item of class information with the
    given \a index.

    \sa indexOfClassInfo()
 */
QMetaClassInfo QMetaObject::classInfo(int index) const
{
    int i = index;
    i -= classInfoOffset();
    if (i < 0 && d.superdata)
        return d.superdata->classInfo(index);

    QMetaClassInfo result;
    if (i >= 0 && i <= priv(d.data)->classInfoCount) {
        result.mobj = this;
        result.handle = priv(d.data)->classInfoData + 2*i;
    }
    return result;
}

/*!
    Returns true if the \a signal and \a member arguments are
    compatible; otherwise returns false.

    Both \a signal and \a member are expected to be normalized.

    \sa normalizedSignature()
*/
bool QMetaObject::checkConnectArgs(const char *signal, const char *member)
{
    const char *s1 = signal;
    const char *s2 = member;
    while (*s1++ != '(') { }                        // scan to first '('
    while (*s2++ != '(') { }
    if (*s2 == ')' || qstrcmp(s1,s2) == 0)        // member has no args or
        return true;                                //   exact match
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    if (s2len < s1len && strncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',')
        return true;                                // member has less args
    return false;
}

static inline bool is_ident_char(char s)
{
    return ((s >= 'a' && s <= 'z')
            || (s >= 'A' && s <= 'Z')
            || (s >= '0' && s <= '9')
            || s == '_'
       );
}

static inline bool is_space(char s)
{
    return (s == ' ' || s == '\t');
}

// WARNING: a copy of this function is in moc.cpp
static QByteArray normalizeTypeInternal(const char *t, const char *e, bool fixScope = true, bool adjustConst = true)
{
    int len = e - t;
    if (strncmp("void", t, len) == 0)
        return QByteArray();
    /*
      Convert 'char const *' into 'const char *'. Start at index 1,
      not 0, because 'const char *' is already OK.
    */
    QByteArray constbuf;
    for (int i = 1; i < len; i++) {
        if ( t[i] == 'c'
             && strncmp(t + i + 1, "onst", 4) == 0
             && (i + 5 >= len || !is_ident_char(t[i + 5]))
             && !is_ident_char(t[i-1])
            ) {
            constbuf = QByteArray(t, len);
            if (is_space(t[i-1]))
                constbuf.remove(i-1, 6);
            else
                constbuf.remove(i, 5);
            constbuf.prepend("const ");
            t = constbuf.data();
            e = constbuf.data() + constbuf.length();
            break;
        }
        /*
          We musn't convert 'char * const *' into 'const char **'
          and we must beware of 'Bar<const Bla>'.
        */
        if (t[i] == '&' || t[i] == '*' ||t[i] == '<')
            break;
    }
    if (adjustConst && e > t + 6 && strncmp("const ", t, 6) == 0) {
        if (*(e-1) == '&') { // treat const reference as value
            t += 6;
            --e;
        } else if (is_ident_char(*(e-1))) { // treat const value as value
            t += 6;
        }
    }
    QByteArray result;
    result.reserve(len);

    // some type substitutions for 'unsigned x'
    if (strncmp("unsigned ", t, 9) == 0) {
        if (strncmp("int", t+9, 3) == 0) {
            t += 9+3;
            result += "uint";
        } else if (strncmp("long", t+9, 4) == 0) {
            t += 9+4;
            result += "ulong";
        }
    }

    while (t != e) {
        char c = *t++;
        if (fixScope && c == ':' && *t == ':' ) {
            ++t;
            c = *t++;
            int i = result.size() - 1;
            while (i >= 0 && is_ident_char(result.at(i)))
                   --i;
            result.resize(i + 1);
        }
        result += c;
        if (c == '<') {
            //template recursion
            const char* tt = t;
            int templdepth = 1;
            while (t != e) {
                c = *t++;
                if (c == '<')
                    ++templdepth;
                if (c == '>')
                    --templdepth;
                if (templdepth == 0) {
                    result += normalizeTypeInternal(tt, t-1, fixScope, false);
                    result += c;
                    if (*t == '>')
                        result += ' '; // avoid >>
                    break;
                }
            }
        }
    }

    return result;
}

/*!
    Normalizes the signature of the given \a member.

    Qt uses normalized signatures to decide whether two given signals
    and slots are compatible. Normalization reduces whitespace to a
    minimum, moves 'const' to the front where appropriate, removes
    'const' from value types and replaces const references with
    values.

    \sa checkConnectArgs()
 */
QByteArray QMetaObject::normalizedSignature(const char *member)
{
    const char *s = member;
    if (!s || !*s)
        return "";
    int len = strlen(s);
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len+1] : stackbuf);
    char *d = buf;
    char last = 0;
    while(*s && is_space(*s))
        s++;
    while (*s) {
        while (*s && !is_space(*s))
            last = *d++ = *s++;
        while (*s && is_space(*s))
            s++;
        if (*s && is_ident_char(*s) && is_ident_char(last))
            last = *d++ = ' ';
    }
    *d = '\0';
    d = buf;

    QByteArray result;
    result.reserve(len);

    int argdepth = 0;
    int templdepth = 0;
    while (*d) {
        if (argdepth == 1) {
            const char *t = d;
            while (*d&& (templdepth
                           || (*d != ',' && *d != ')'))) {
                if (*d == '<')
                    ++templdepth;
                if (*d == '>')
                    --templdepth;
                d++;
            }
            result += normalizeTypeInternal(t, d);
        }
        if (*d == '(')
            ++argdepth;
        if (*d == ')')
            --argdepth;
        result += *d++;
    }

    if (buf != stackbuf)
        delete [] buf;
    return result;
}


/*!
    \class QMetaMember qmetaobject.h

    \brief The QMetaMember class provides meta data about a member
    function.

    \ingroup objectmodel

    A QMetaMember has a memberType(), a signature(), a list of
    parameterTypes() and parameterNames(), a return typeName(), a
    tag(), and an access() specifier.
*/

/*!
    \enum QMetaMember::Attributes

    \value Compatibility
    \value Cloned
    \value Scriptable

*/

/*!
    \enum QMetaMember::MemberType

    \value Method
    \value Signal
    \value Slot
*/

/*!
    \fn QMetaMember::QMetaMember()
    \internal
*/

/*!
    Returns the signature of this member.
*/
const char *QMetaMember::signature() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns a list of parameter types.
*/
QList<QByteArray> QMetaMember::parameterTypes() const
{
    QList<QByteArray> list;
    if (!mobj)
        return list;
    const char *signature = mobj->d.stringdata + mobj->d.data[handle];
    while (*signature && *signature != '(')
        ++signature;
    while (*signature && *signature != ')' && *++signature != ')') {
        const char *begin = signature;
        int level = 0;
        while (*signature && (level > 0 || *signature != ',') && *signature != ')') {
            if (*signature == '<')
                ++level;
            else if (*signature == '>')
                --level;
            ++signature;
        }
        list += QByteArray(begin, signature - begin);
    }
    return list;
}

/*!
    Returns a list of parameter names.
*/
QList<QByteArray> QMetaMember::parameterNames() const
{
    QList<QByteArray> list;
    if (!mobj)
        return list;
    const char *names =  mobj->d.stringdata + mobj->d.data[handle + 1];
    if (*names == 0) {
        // do we have one or zero arguments?
        const char *signature = mobj->d.stringdata + mobj->d.data[handle];
        while (*signature && *signature != '(')
            ++signature;
        if (*++signature != ')')
            list += QByteArray();
    } else {
        --names;
        do {
            const char *begin = ++names;
            while (*names && *names != ',')
                ++names;
            list += QByteArray(begin, names - begin);
        } while (*names);
    }
    return list;
}


/*!
    Returns the return type of this member, or an empty string if the
    return type is \e void.
*/
const char *QMetaMember::typeName() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 2];
}

/*!
    Returns the tag associated with this member.
*/

const char *QMetaMember::tag() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 3];
}


/*! \internal */
int QMetaMember::attributes() const
{
    if (!mobj)
        return false;
    return ((mobj->d.data[handle + 4])>>4);
}

/*!
  Returns the access specification of this member: private,
    protected, or public. Signals are always protected.
*/

QMetaMember::Access QMetaMember::access() const
{
    if (!mobj)
        return Private;
    return (QMetaMember::Access)(mobj->d.data[handle + 4] & AccessMask);
}

/*!
    Returns the type of this member: Signal, Slot, or Method.
*/

QMetaMember::MemberType QMetaMember::memberType() const
{
    if (!mobj)
        return QMetaMember::Method;
    return (QMetaMember::MemberType)((mobj->d.data[handle + 4] & MemberTypeMask)>>2);
}

/*!
    \class QMetaEnum qmetaobject.h

    \brief The QMetaEnum class provides meta data about an enumerator.

    \ingroup objectmodel

    Use name() for the enumerator's name. The enumerator's keys (names
    of each enumerated item) are returned by key(); use keyCount() to find
    the number of keys. isFlag() returns whether the enumerator is
    meant to be used as a flag, meaning that its values can be combined
    using the OR operator.

    The conversion functions keyToValue(), valueToKey(), keysToValue(),
    and valueToKeys() allow conversion between the integer
    representation of an enumeration or set value and its literal
    representation. The scope() function returns the class scope this
    enumerator was declared in.
*/

/*!
    \fn bool QMetaEnum::isValid() const

    Returns true if this enum is valid (has a name); otherwise returns
    false.
*/

/*!
    \fn QMetaEnum::QMetaEnum()
    \internal
*/

/*!
    Returns the name of the enumerator.

    \sa isValid()
*/
const char* QMetaEnum::name() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns the number of keys.

    \sa key()
*/
int QMetaEnum::keyCount() const
{
    if (!mobj)
        return 0;
    return mobj->d.data[handle + 2];
}


/*!
    Returns the key with the given \a index, or 0 if no such key exists.

    \sa keyCount() value()
*/
const char *QMetaEnum::key(int index) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    if (index >= 0  && index < count)
        return mobj->d.stringdata + mobj->d.data[data + 2*index];
    return 0;
}

/*!
    Returns the value with the given \a index; or returns -1 if there
    is no such value.

    \sa keyCount() key()
*/
int QMetaEnum::value(int index) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    if (index >= 0  && index < count)
        return mobj->d.data[data + 2*index + 1];
    return -1;
}


/*!
    Returns true if this enumerator is used as a flag; otherwise returns
    false.

    When used as flags, enumerators can be combined using the OR
    operator.

    \sa keysToValue(), valueToKeys()
*/
bool QMetaEnum::isFlag() const
{
    return mobj && mobj->d.data[handle + 1];
}


/*!
    Returns the scope this enumerator was declared in.
 */
const char *QMetaEnum::scope() const
{
    return mobj?mobj->d.stringdata : 0;
}

/*!
    Returns the integer value of the given enumeration \a key, or -1
    if \a key is not defined.

    For set types, use keysToValue().

    \sa valueToKey(), isFlag(), keysToValue()
*/
int QMetaEnum::keyToValue(const char *key) const
{
    if (!mobj || !key)
        return -1;
    int scope = 0;
    const char *qualified_key = key;
    const char *s = key;
    while (*s  && *s != ':')
        ++s;
    if (*s && *(s+1)==':') {
        scope = s - key;
        key += scope + 2;
    }
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int i = 0; i < count; ++i)
        if ((!scope || strncmp(qualified_key, mobj->d.stringdata, scope) == 0)
             && strcmp(key, mobj->d.stringdata + mobj->d.data[data + 2*i]) == 0)
            return mobj->d.data[data + 2*i + 1];
    return -1;
}

/*!
    Returns the string that is used as the name of the given
    enumeration \a value, or 0 if \a value is not defined.

    For set types, use valueToKeys().

    \sa valueToKey() isFlag() valueToKeys()
*/
const char* QMetaEnum::valueToKey(int value) const
{
    if (!mobj)
        return 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int i = 0; i < count; ++i)
        if (value == (int)mobj->d.data[data + 2*i + 1])
            return mobj->d.stringdata + mobj->d.data[data + 2*i];
    return 0;
}

/*!
    Returns the value derived from combining together the values of the
    \a keys using the OR operator. Note that the strings in \a keys
    must be '|'-separated.

    \sa isFlag(), valueToKey(), keysToValue()
*/
int QMetaEnum::keysToValue(const char *keys) const
{
    if (!mobj)
        return -1;
    QStringList l = QString::fromLatin1(keys).split(QLatin1Char('|'));
    //#### TODO write proper code, do not use QStringList
    int value = 0;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    for (int li = 0; li < l.size(); ++li) {
        QString trimmed = l.at(li).trimmed();
        const char *key = trimmed.latin1();
        int scope = 0;
        const char *qualified_key = key;
        const char *s = key;
        while (*s  && *s != ':')
            ++s;
        if (*s && *(s+1)==':') {
            scope = s - key;
            key += scope + 2;
        }
        int i;
        for (i = count-1; i >= 0; --i)
            if ((!scope || strncmp(qualified_key, mobj->d.stringdata, scope) == 0)
                 && strcmp(key, mobj->d.stringdata + mobj->d.data[data + 2*i]) == 0) {
                value |= mobj->d.data[data + 2*i + 1];
                break;
            }
        if (i < 0)
            value |= -1;
    }
    return value;
}

/*!
    Returns a byte array of '|'-separated keys that represents the
    given \a value.

    \sa isFlag(), valueToKey(), valueToKeys()
*/
QByteArray QMetaEnum::valueToKeys(int value) const
{
    QByteArray keys;
    if (!mobj)
        return keys;
    int count = mobj->d.data[handle + 2];
    int data = mobj->d.data[handle + 3];
    int v = value;
    for(int i = count - 1; i >= 0; --i) {
        int k = mobj->d.data[data + 2*i + 1];
        if ((k != 0 && (v & k) == k ) ||  (k == value))  {
            v = v & ~k;
            if (!keys.isEmpty())
                keys += '|';
            keys += mobj->d.stringdata + mobj->d.data[data + 2*i];
        }
    }
    return keys;
}


/*!
    \class QMetaProperty qmetaobject.h

    \brief The QMetaProperty class provides meta data about a property.

    \ingroup objectmodel

    A property has a name() and a type(), as well as various
    attributes that specify its behavior: isReadable(), isWritable(),
    isDesignable(), isScriptable(), isStored(), and isEditable().

    If the property is an enumeration, isEnumType() returns true; if the
    property is an enumeration that is also a flag (i.e. its values
    can be combined using the OR operator), isEnumType() and
    isFlagType() both return true. The enumerator for these types is
    available from enumerator().

    The property's values are set and retrieved with read(), write(),
    and reset(); they can also be changed through QObject's set and get
    functions. See QObject::setProperty() and QObject::property() for
    details.

    You get meta property data through an object's meta object. See
    QMetaObject::property() and QMetaObject::propertyCount() for
    details.
*/

/*!
    \fn bool QMetaProperty::isValid() const

    Returns true if this property is valid (readable); otherwise
    returns false.
*/

/*!
    Constructs an invalid property
    \internal
*/
QMetaProperty::QMetaProperty()
{
    ::memset(this, 0, sizeof(QMetaProperty));
}


/*!
    Returns this property's name.
 */
const char *QMetaProperty::name() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    return mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle];
}

/*!
    Returns the name of this property's type.
 */
const char *QMetaProperty::typeName() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    return mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle + 1];
}

/*!
    Returns this property's type. The return value is one
    of the values of the QCoreVariant::Type enumeration.
*/
QCoreVariant::Type QMetaProperty::type() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return QCoreVariant::Invalid;

    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];

    QCoreVariant::Type type = QCoreVariant::Type(flags >> 24);
    if (type)
        return type;
    if (isEnumType())
        return QCoreVariant::Int;

    return QCoreVariant::UserType;
}


/*!
    Returns true if the property's type is an enumeration value that
    is used as a flag; otherwise returns false.

    Flags can be combined using the OR operator. A set type is
    implicitly also an enum type.

    \sa isEnumType(), enumerator()
*/

bool QMetaProperty::isFlagType() const
{
    return isEnumType() && menum.isFlag();
}

/*!
    Returns true if the property's type is an enumeration value;
    otherwise returns false.

    \sa enumerator(), isFlagType()
*/
bool QMetaProperty::isEnumType() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return (flags & EnumOrFlag) && menum.name();
}

/*!
    \internal

    Returns true if the property has a C++ setter function that
    follows Qt's standard "name" / "setName" pattern. Designer and uic
    query hasStdCppSet() in order to avoid expensive
    QObject::setProperty() calls. All properties in Qt [should] follow
    this pattern.
*/
bool QMetaProperty::hasStdCppSet() const
{
    if (!mobj[QMetaObject::ReadProperty])
        return 0;
    int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
    int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
    return (flags & StdCppSet);
}

/*!
    Returns the enumerator if this property's type is an enumerator
    type; otherwise the returned value is undefined.

    \sa isEnumType()
 */
QMetaEnum QMetaProperty::enumerator() const
{
    return menum;
}


/*!
    \fn QCoreVariant QMetaProperty::read(const QObject *object) const

    Reads the property's value from the given \a object. Returns the value
    if it was able to read it; otherwise returns an invalid variant.

    \sa write() isReadable()
*/
QCoreVariant QMetaProperty::read(const QObject *obj) const
{
    if (!obj || !mobj[QMetaObject::ReadProperty])
        return QCoreVariant();

    int  t = QCoreVariant::Int;
    if (!isEnumType()) {
        int handle = priv(mobj[QMetaObject::ReadProperty]->d.data)->propertyData + 3*idx[QMetaObject::ReadProperty];
        int flags = mobj[QMetaObject::ReadProperty]->d.data[handle + 2];
        const char *typeName = mobj[QMetaObject::ReadProperty]->d.stringdata + mobj[QMetaObject::ReadProperty]->d.data[handle + 1];
        t = (flags >> 24);
        if (t == QCoreVariant::Invalid)
            t = QMetaType::type(typeName);
        if (t == QCoreVariant::Invalid)
            t = QCoreVariant::nameToType(typeName);
        if (t == QCoreVariant::Invalid || t == QCoreVariant::UserType)
            return QCoreVariant();
    }
    QCoreVariant value;
    void *argv[1];
    if (t == int(QCoreVariant::LastType)) {
        argv[0] = &value;
    } else {
        value = QCoreVariant(t, (void*)0);
        argv[0] = value.data();
    }
    const_cast<QObject*>(obj)->qt_metacall(QMetaObject::ReadProperty,
                     idx[QMetaObject::ReadProperty] + mobj[QMetaObject::ReadProperty]->propertyOffset(),
                     argv);
    if (t != int(QCoreVariant::LastType) && argv[0] != value.data())
        return QCoreVariant((QCoreVariant::Type)t, argv[0]);
    return value;
}

/*!
    \fn bool QMetaProperty::write(QObject *object, const QCoreVariant &value) const

    Writes \a value as the property's value to the given \a object. Returns
    true if the write succeeded; otherwise returns false.

    \sa read() isWritable()
*/
bool QMetaProperty::write(QObject *obj, const QCoreVariant &value) const
{
    if (!obj || !isWritable())
        return false;

    QCoreVariant v = value;
    uint t = QCoreVariant::Invalid;
    if (isEnumType()) {
        if (v.type() == QCoreVariant::String || v.type() == QCoreVariant::CString) {
            if (isFlagType())
                v = QCoreVariant(menum.keysToValue(value.toByteArray()));
            else
                v = QCoreVariant(menum.keyToValue(value.toByteArray()));
        } else if (v.type() != QCoreVariant::Int && v.type() != QCoreVariant::UInt) {
            return false;
        }
        v.cast(QCoreVariant::Int);
    } else {
        int handle = priv(mobj[QMetaObject::WriteProperty]->d.data)->propertyData + 3*idx[QMetaObject::WriteProperty];
        int flags = mobj[QMetaObject::WriteProperty]->d.data[handle + 2];
        const char *typeName = mobj[QMetaObject::WriteProperty]->d.stringdata + mobj[QMetaObject::WriteProperty]->d.data[handle + 1];
        t = flags >> 24;
        if (t == QCoreVariant::Invalid) {
            const char *vtypeName = value.typeName();
            if (vtypeName && strcmp(typeName, vtypeName) == 0)
                t = value.userType();
            else
                t = QCoreVariant::nameToType(typeName);
        }
        if (t == QCoreVariant::Invalid)
            return false;
        if (t != QCoreVariant::LastType && (t != (uint)value.userType() || (t < QCoreVariant::UserType && !v.cast((QCoreVariant::Type)t))))
            return false;
    }

    void *argv[1];
    if (t == QCoreVariant::LastType)
        argv[0] = &v;
    else
        argv[0] = v.data();
    obj->qt_metacall(QMetaObject::WriteProperty,
                     idx[QMetaObject::WriteProperty] + mobj[QMetaObject::WriteProperty]->propertyOffset(),
                     argv);
    return true;
}

/*!
    \fn bool QMetaProperty::reset(QObject *object) const

    Resets the property for the given \a object with a reset method.
    Returns true if the reset worked; otherwise returns false.

    Reset methods are optional; only a few properties support them.
*/
bool QMetaProperty::reset(QObject *obj) const
{
    if (!obj || !mobj[QMetaObject::ResetProperty])
        return false;
    void *argv[] = { 0 };
    obj->qt_metacall(QMetaObject::ResetProperty,
                     idx[QMetaObject::ResetProperty] + mobj[QMetaObject::ResetProperty]->propertyOffset(),
                     argv);
    return true;
}


/*!
    Returns true if this property is readable; otherwise returns false.

    \sa read() isWritable()
 */
bool QMetaProperty::isReadable() const
{
    return mobj[QMetaObject::ReadProperty] != 0;
}

/*!
    Returns true if this property is writable; otherwise returns
    false.

    \sa write() isReadable()
 */
bool QMetaProperty::isWritable() const
{
    return mobj[QMetaObject::WriteProperty] != 0;
}


static bool qt_query_property(const QMetaObject*const*mobj,const int *idx, uint flag,
                              QMetaObject::Call call, const QObject* obj)
{
    if (!mobj[call])
        return false;
    int handle = priv(mobj[call]->d.data)->propertyData + 3*idx[call];
    int flags = mobj[call]->d.data[handle + 2];
    bool b = (flags & flag);
    if (obj) {
        void *argv[] = { &b };
        const_cast<QObject*>(obj)->qt_metacall(call,
                                               idx[call]
                                               + mobj[call]->propertyOffset(),
                                               argv);
    }
    return b;
}

/*!
    \fn bool QMetaProperty::isDesignable(const QObject *object) const

    Returns true if this property is designable for the given \a object;
    otherwise returns false.

    If no \a object is given, the function returns false if the
    \c{Q_PROPERTY}'s \c DESIGNABLE attribute is false; otherwise
    returns true (if the attribute is true or is a function or expression).
 */
bool QMetaProperty::isDesignable(const QObject *obj) const
{
    if (!mobj[QMetaObject::WriteProperty])
        return false;
    return qt_query_property(mobj, idx, Designable,
                              QMetaObject::QueryPropertyDesignable,
                              obj);
}

/*!
    \fn bool QMetaProperty::isScriptable(const QObject *object) const

    Returns true if the property is scriptable for the given \a object;
    otherwise returns false.

    If no \a object is given, the function returns false if the
    \c{Q_PROPERTY}'s \c DESIGNABLE attribute is false; otherwise returns
    true (if the attribute is true or is a function or expression).
 */
bool QMetaProperty::isScriptable(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Scriptable,
                              QMetaObject::QueryPropertyScriptable,
                              obj);
}

/*!
    \fn bool QMetaProperty::isStored(const QObject *object) const

    Returns true if the property is stored for \a object; otherwise returns
    false.

    If no \a object is given, the function returns false if the
    \c{Q_PROPERTY}'s \c DESIGNABLE attribute is false; otherwise returns
    true (if the attribute is true or is a function or expression).
 */
bool QMetaProperty::isStored(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Stored,
                              QMetaObject::QueryPropertyStored,
                              obj);
}

/*!
    \fn bool QMetaProperty::isEditable(const QObject *object) const

    Returns true if the property is editable for the given \a object;
    otherwise returns false.

    If no \a object is given, the function returns false if the
    \c{Q_PROPERTY}'s \c DESIGNABLE attribute is false; otherwise returns
    true (if the attribute is true or is a function or expression).
 */
bool QMetaProperty::isEditable(const QObject *obj) const
{
    return qt_query_property(mobj, idx, Editable,
                              QMetaObject::QueryPropertyEditable,
                              obj);
}


/*!
    \class QMetaClassInfo qmetaobject.h

    \brief The QMetaClassInfo class provides additional information
    about a class.

    \ingroup objectmodel

    Class information items are simple \e{name}--\e{value} pairs that
    are specified using \c Q_CLASSINFO in the source code. The
    information can be retrieved using name() and value().
*/


/*!
    \fn QMetaClassInfo::QMetaClassInfo()
    \internal
*/

/*!
    Returns the name of this item.

    \sa value()
*/
const char* QMetaClassInfo::name() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle];
}

/*!
    Returns the value of this item.

    \sa name()
*/
const char* QMetaClassInfo::value() const
{
    if (!mobj)
        return 0;
    return mobj->d.stringdata + mobj->d.data[handle + 1];
}


