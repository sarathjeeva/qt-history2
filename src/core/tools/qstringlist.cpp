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

#include <qstringlist.h>

/*! \typedef QStringListIterator
    \relates QStringList

    The QStringListIterator typedef provides a Java-style const
    iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style const iterator is simply
    a typedef for QListIterator<QString>.

    \sa QStringListMutableIterator, QStringList::const_iterator
*/

/*! \typedef QStringListMutableIterator
    \relates QStringList

    The QStringListIterator typedef provides a Java-style non-const
    iterator for QStringList.

    QStringList provides both \l{Java-style iterators} and
    \l{STL-style iterators}. The Java-style non-const iterator is
    simply a typedef for QListMutableIterator<QString>.

    \sa QStringListIterator, QStringList::iterator
*/

/*!
    \class QStringList
    \brief The QStringList class provides a list of strings.

    \ingroup tools
    \ingroup shared
    \ingroup text
    \mainclass
    \reentrant

    QStringList inherits from QList\<QString\>. All of QList's
    functionality also applies to QStringList. For example, you can
    use isEmpty() to test whether the list is empty, and you can call
    functions like append(), prepend(), insert(), replace(), and
    remove() to modify a QStringList. In addition, QStringList
    provides a few convenience functions that make handling lists of
    strings easier.

    Like QList, QStringList is \l{implicitly shared}. QStringList
    provides fast index-based access as well as fast insertions and
    removals. Passing string lists as value parameters is both fast
    and safe.

    Strings can be added to a list using append(), operator+=(), or
    operator<<(). For example:
    \code
        QStringList fonts;
        fonts << "Arial" << "Helvetica" << "Times" << "Courier";
    \endcode

    To iterate over a string, you can either use index positions or
    QList's Java-style and STL-style iterator types. Here are
    examples of each approach.

    Indexing:
    \code
        for (int i = 0; i < fonts.size(); ++i)
            cout << fonts.at(i).ascii() << endl;
    \endcode

    Java-style iterator:
    \code
        QStringListIterator i(fonts);
        while (i.hasNext())
            cout << i.next().ascii() << endl;
    \endcode

    STL-style iterator:
    \code
        QStringList::const_iterator i;
        for (i = fonts.constBegin(); i != fonts.constEnd(); ++i)
            cout << (*i).ascii() << endl;
    \endcode

    QStringListIterator and QStringListMutableIterator are simply
    typedefs for QListIterator<QString> and
    QListMutableIterator<QString>.

    You can concatenate all the strings in a string list into a single
    string (with an optional separator) using join(). For example:

    \code
        QString str = fonts.join(",");
        // str == "Arial,Helvetica,Times,Courier"
    \endcode

    To break up a string into a string list, use QString::split():

    \code
        QString str = "Arial,Helvetica,Times,Courier";
        QStringList list = str.split(",");
        // list: ["Arial", "Helvetica", "Times", "Courier"]
    \endcode

    The argument to split can be a single character, a string, or a
    QRegExp.

    You can sort a string list with sort(), and extract a new list
    which contains only those strings which contain a particular
    substring (or match a particular regular expression) using the
    find() functions. For example:

    \code
        QStringList monospacedFonts = fonts.find(QRegExp("Courier|Fixed"));
    \endcode

    Similarly, the replace() function calls QString::replace() on
    each string in the string list in turn. Here's an example that
    uses it to replace all occurrences of "$QTDIR" with "/usr/lib/qt"
    in a string list:

    \code
        QStringList files;
        files << "$QTDIR/src/moc/moc.y"
              << "$QTDIR/src/moc/moc.l"
              << "$QTDIR/include/qconfig.h";

        files.replace("$QTDIR", "/usr/lib/qt");
    \endcode

    \sa QString, QStringListIterator, QStringListMutableIterator
*/

/*!
    \fn QStringList::QStringList()

    Constructs an empty string list.
*/

/*!
    \fn QStringList::QStringList(const QString &str)

    Constructs a string list that contains one string, \a str. Longer
    lists are easily created like this:

    \code
        list = (QStringList() << str1 << str2 << str3);
    \endcode
*/

/*!
    \fn QStringList::QStringList(const QStringList &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QStringList is
    \l{implicitly shared}. This makes returning a QStringList from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*!
    \fn QStringList::QStringList(const QList<QString> &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QStringList is
    \l{implicitly shared}. This makes returning a QStringList from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*!
    Sorts the list of strings in ascending order (case sensitively).

    Sorting is performed using Qt's qHeapSort() algorithm,
    which operates in \l{linear-logarithmic time}, i.e. O(\e{n} log \e{n}).

    If you want to sort your strings in an arbitrary order, consider
    using a QMap. For example, you could use a QMap\<QString,
    QString\> to create a case-insensitive ordering (e.g. with the
    keys being lower-case versions of the strings, and the values
    being the strings), or a QMap\<int, QString\> to sort the strings
    by some integer index.

    \sa qHeapSort()
*/
void QStringList::sort()
{
    qHeapSort(*this);
}


#ifdef QT_COMPAT
/*!
    \overload

    This version of the function uses a QChar as separator.

    \sa join() QString::section()
*/
QStringList QStringList::split(const QChar &sep, const QString &str, bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}

/*!
    \overload

    This version of the function uses a QString as separator.

    If \a sep is an empty string, the return value is a list of
    one-character strings: split(QString(""), "four") returns the
    four-item list, "f", "o", "u", "r".

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    \sa join() QString::section()
*/
QStringList QStringList::split(const QString &sep, const QString &str,
                                      bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}

#ifndef QT_NO_REGEXP
/*!
    Splits the string \a str into strings wherever the regular
    expression \a sep occurs, and returns the list of those strings.

    If \a allowEmptyEntries is true, an empty string is inserted in
    the list wherever the separator matches twice without intervening
    text.

    For example, if you split the string "a,,b,c" on commas, split()
    returns the three-item list "a", "b", "c" if \a allowEmptyEntries
    is false (the default), and the four-item list "a", "", "b", "c"
    if \a allowEmptyEntries is true.

    Use \c{split(QRegExp("\\s+"), str)} to split on arbitrary amounts
    of whitespace.

    If \a sep does not match anywhere in \a str, split() returns a
    single element list with the element containing the original
    string, \a str.

    \sa join() QString::section()
*/
QStringList QStringList::split(const QRegExp &sep, const QString &str,
                                      bool allowEmptyEntries)
{
    if (str.isEmpty())
        return QStringList();
    return str.split(sep, allowEmptyEntries ? QString::KeepEmptyParts
                                            : QString::SkipEmptyParts);
}
#endif
#endif // QT_COMPAT

/*!
    Returns a list of all the strings containing the substring \a str.

    If \a cs is \l Qt::CaseSensitive (the default), the string
    comparison is case sensitive; otherwise the comparison is case
    insensitive.

    \code
        QStringList list;
        list << "Bill Murray" << "John Doe" << "Bill Clinton";
        list = list.find("Bill");
        // list: ["Bill Murray", "Bill Clinton"]
    \endcode

    \sa QString::contains()
*/
QStringList QStringList::find(const QString &str, Qt::CaseSensitivity cs) const
{
    QStringMatcher matcher(str, cs);
    QStringList res;
    for (int i = 0; i < size(); ++i)
        if (matcher.indexIn(at(i)) != -1)
            res << at(i);
    return res;
}


/*!
    Returns true if the list contains the string \a str.
    Does a case insensitive search if \a cs is Qt::CaseSensitive,
    otherwise the search will be case insensitive.
 */
QBool QStringList::contains(const QString &str, Qt::CaseSensitivity cs) const
{
    QStringMatcher matcher(str, cs);
    for (int i = 0; i < size(); ++i) {
        QString string(at(i));
        if (string.length() == str.length() && matcher.indexIn(string) == 0)
            return QBool(true);
    }
    return QBool(false);
}

#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns a list of all the strings that match the regular
    expression \a rx.
*/
QStringList QStringList::find(const QRegExp &rx) const
{
    QStringList res;
    for (int i = 0; i < size(); ++i)
        if (at(i).contains(rx))
            res << at(i);
    return res;
}
#endif

/*! \fn void QStringList::replace(int i, const QString &s)

    \internal
*/

/*!
    Returns a string list where every string has had the \a before
    text replaced with the \a after text wherever the \a before text
    is found. The \a before text is matched case-sensitively or not
    depending on the \a cs flag.

    Example:
    \code
        QStringList list;
        list << "alpha" << "beta" << "gamma" << "epsilon";
        list.replace("a", "o");
        // list == ["olpho", "beto", "gommo", "epsilon"]
    \endcode

    \sa QString::replace()
*/
QStringList &QStringList::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    for (int i = 0; i < size(); ++i)
        (*this)[i].replace(before, after, cs);
    return *this;
}


#ifndef QT_NO_REGEXP
/*!
    \fn QStringList &QStringList::replace(const QRegExp &rx, const QString &after)

    \overload

    Replaces every occurrence of the regexp \a rx, in each of the
    string lists's strings, with \a after. Returns a reference to the
    string list.

    Example:
    \code
        QStringList list;
        list << "alpha" << "beta" << "gamma" << "epsilon";
        list.replace(QRegExp("^a"), "o");
        // list == ["olpha", "beta", "gamma", "epsilon"]
    \endcode

    For regular expressions that contain \link
    qregexp.html#capturing-text capturing parentheses \endlink,
    occurrences of \bold{\\1}, \bold{\\2}, ..., in \a after are
    replaced with \a{rx}.cap(1), \a{rx}.cap(2), ...

    Example:
    \code
        QStringList list;
        list << "Bill Clinton" << "Murray, Bill";
        list.replace(QRegExp("^(.*), (.*)$"), "\\2 \\1");
        // list == ["Bill Clinton", "Bill Murray"]
    \endcode

    \sa replace()
*/
QStringList& QStringList::replace(const QRegExp &rx, const QString &after)
{
    for (int i = 0; i < size(); ++i)
        (*this)[i].replace(rx, after);
    return *this;
}
#endif

/*!
    Joins the all the string list's strings into a single string with
    each element separated by the string \a sep (which can be an empty
    string).

    \sa QString::split()
*/
QString QStringList::join(const QString &sep) const
{
    QString res;
    for (int i = 0; i < size(); ++i) {
        if (i)
            res += sep;
        res += at(i);
    }
    return res;
}

/*!
    \fn QStringList QStringList::operator+(const QStringList &other) const

    Returns a string list that is the concatenation of this string
    list with the \a other string list.
*/

/*!
    \fn QStringList &QStringList::operator<<(const QString &str)

    Appends string \a str to the string list and returns a reference
    to the string list.
*/

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator>>(QDataStream &in, QStringList &list)

    \relates QStringList

    Reads a string list from stream \a in into \a list.

    \sa \link datastreamformat.html Format of the QDataStrea operators \endlink
*/

/*!
    \fn QDataStream &operator<<(QDataStream &out, const QStringList &list)

    Writes the string list \a list to stream \a out.

    \sa \link datastreamformat.html Format of the QDataStrea operators \endlink
*/
#endif // QT_NO_DATASTREAM

/*!
    \fn QStringList QStringList::grep(const QString &str, bool cs = true) const

    Use find() instead.
*/

/*!
    \fn QStringList QStringList::grep(const QRegExp &rx) const

    Use find() instead.
*/

/*!
    \fn QStringList &QStringList::gres(const QString &before, const QString &after, bool cs = true)

    Use replace() instead.
*/

/*!
    \fn QStringList &QStringList::gres(const QRegExp &rx, const QString &after)

    Use replace() instead.
*/

/*!
    \fn Iterator QStringList::fromLast()

    Use end() instead.

    \oldcode
    QStringList::Iterator i = list.fromLast();
    \newcode
    QStringList::Iterator i = list.isEmpty() ? list.end() : --list.end();
    \endcode
*/

/*!
    \fn ConstIterator QStringList::fromLast() const

    Use end() instead.

    \oldcode
    QStringList::ConstIterator i = list.fromLast();
    \newcode
    QStringList::ConstIterator i = list.isEmpty() ? list.end() : --list.end();
    \endcode
*/


#ifndef QT_NO_REGEXP
/*!
    \overload

    Returns the index position of the first exact match of \a rx in
    the list, searching forward from index position \a from. Returns
    -1 if no item matched.

    \sa lastIndexOf, QRegExp::exactMatch
*/
int QStringList::indexOf(const QRegExp &rx, int from) const
{
   if (from < 0)
       from = qMax(from + size(), 0);
   for (int i = from; i < size(); ++i) {
        if (rx.exactMatch(at(i)))
            return i;
    }
    return -1;
}

/*!
    \overload

    Returns the index position of the last exact match of \a rx in
    the list, searching backward from index position \a from. If \a
    from is -1 (the default), the search starts at the last item.
    Returns -1 if no item matched.

    \sa indexOf, QRegExp::exactMatch
*/
int QStringList::lastIndexOf(const QRegExp &rx, int from) const
{
    if (from < 0)
        from += size();
    else if (from >= size())
        from = size() - 1;
    for (int i = from; i >= 0; --i) {
        if (rx.exactMatch(at(i)))
            return i;
        }
    return -1;
}
#endif
