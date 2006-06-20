/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// test the container forwards
#include <QtContainerFwd>

static QCache<int, int> *cacheX;
static QHash<int, int> *hashX;
static QLinkedList<int> *linkedListX;
static QList<int> *listX;
static QMap<int, int> *mapX;
static QMultiHash<int, int> *multiHashX;
static QMultiMap<int, int> *multiMapX;
static QPair<int, int> *pairX;
static QQueue<int> *queueX;
static QSet<int> *setX;
static QStack<int> *stackX;
static QVarLengthArray<int> *varLengthArrayX;
static QVarLengthArray<int, 512> *varLengthArrayY;
static QVector<int> *vectorX;

void foo()
{
    cacheX = 0;
    hashX = 0;
    linkedListX = 0;
    listX = 0;
    mapX = 0;
    multiHashX = 0;
    multiMapX = 0;
    pairX = 0;
    queueX = 0;
    setX = 0;
    stackX = 0;
    varLengthArrayX = 0;
    varLengthArrayY = 0;
    vectorX = 0;
}

#include <QtTest/QtTest>

#include <algorithm>

#include "qalgorithms.h"
#include "qbitarray.h"
#include "qbytearray.h"
#include "qcache.h"
#include "qhash.h"
#include "qlinkedlist.h"
#include "qlist.h"
#include "qmap.h"
#include "qpair.h"
#include "qregexp.h"
#include "qset.h"
#include "qstack.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qvarlengtharray.h"
#include "qvector.h"
#include "qqueue.h"

#ifdef QT3_SUPPORT
#include "q3cleanuphandler.h"
#endif

// Do not test initialization of pods on msvc6 and msvc 2002
// This is a known issue
#if defined Q_CC_MSVC && _MSC_VER < 1310
#   define NOPODINITIALIZATION
#endif


template class QList<int>;

//TESTED_FILES=

class tst_Collections : public QObject
{
    Q_OBJECT

public:
    tst_Collections();
    ~tst_Collections();

public slots:
    void init();
    void cleanup();
private slots:
    void typeinfo();
    void qstring();
    void list();
    void linkedList();
    void vector();
    void byteArray();
    void stack();
    void hash();
    void map();
    void bitArray();
    void cache();
    void regexp();
    void varLengthArray();
    void pair();
    void cleanupHandler();
    void sharableQList();
    void sharableQLinkedList();
    void sharableQVector();
    void sharableQMap();
    void sharableQHash();
    void q_foreach();
    void conversions();
    void javaStyleIterators();
    void constAndNonConstStlIterators();
#ifndef QT_NO_STL
    void vector_stl_data();
    void vector_stl();
    void list_stl_data();
    void list_stl();
    void linkedlist_stl_data();
    void linkedlist_stl();
#endif
    void q_init();
    void pointersize();
    void containerInstantiation();
#if QT_VERSION >= 0x040101
    void qtimerList();
#endif
    void containerTypedefs();
    void forwardDeclared();
};

struct LargeStatic {
    static int count;
    LargeStatic():c(count) { ++count; }
    LargeStatic(const LargeStatic& o):c(o.c) { ++count; }
    ~LargeStatic() { --count; }
    int c;
    int data[8];
};

int LargeStatic::count = 0;

struct Movable {
    static int count;
    Movable():c(count) { ++count; }
    Movable(const Movable& o):c(o.c) { ++count; }
    ~Movable() { --count; }
    int c;
};

int Movable::count = 0;
Q_DECLARE_TYPEINFO(Movable, Q_MOVABLE_TYPE);


struct Pod {
    int i1, i2;

#if defined NOPODINITIALIZATION
    Pod() : i1(0), i2(0) { }
#endif
};

tst_Collections::tst_Collections()
{
}

tst_Collections::~tst_Collections()
{

}

void tst_Collections::init()
{
}

void tst_Collections::cleanup()
{
}

void tst_Collections::typeinfo()
{
    QVERIFY(QTypeInfo<int*>::isPointer);
    QVERIFY(!QTypeInfo<int>::isPointer);
    QVERIFY(QTypeInfo<QString>::isComplex);
    QVERIFY(!QTypeInfo<int>::isComplex);
}

void tst_Collections::list()
{
    {
	QList<int> list;
	QVERIFY(list.isEmpty());
	list.append(1);
	QVERIFY(list.size() == 1);

	QVERIFY(*list.begin() == 1);

	list.push_back(2);
	list += (3);
	list << 4 << 5 << 6;
	QVERIFY(!list.isEmpty());
	QVERIFY(list.size() == 6);
	QVERIFY(list.end() - list.begin() == list.size());

#ifndef Q_CC_MSVC
#ifndef Q_CC_SUN
	QVERIFY(std::binary_search(list.begin(), list.end(), 2) == true);
	QVERIFY(std::binary_search(list.begin(), list.end(), 9) == false);
#endif
#endif
	QVERIFY(qBinaryFind(list.begin(), list.end(), 2) == list.begin() + 1);
	QVERIFY(qLowerBound(list.begin(), list.end(), 2) == list.begin() + 1);
        QVERIFY(qUpperBound(list.begin(), list.end(), 2) == list.begin() + 2);
	QVERIFY(qBinaryFind(list.begin(), list.end(), 9) == list.end());
	QVERIFY(qLowerBound(list.begin(), list.end(), 9) == list.end());
        QVERIFY(qUpperBound(list.begin(), list.end(), 9) == list.end());
	{
	    int sum = 0;
	    QListIterator<int> i(list);
	    while (i.hasNext())
		sum += i.next();
	    QVERIFY(sum == 21);
	}

	{
	    QList<int> list1;
            list1 << 1 << 2 << 3 << 5 << 7 << 8 << 9;
	    QList<int> list2 = list1;

	    QMutableListIterator<int> i1(list1);
            while (i1.hasNext()) {
		if (i1.next() % 2 != 0)
		    i1.remove();
	    }

	    QMutableListIterator<int> i2(list2);
            i2.toBack();
            while (i2.hasPrevious()) {
		if (i2.previous() % 2 != 0)
		    i2.remove();
            }
            QVERIFY(list1.size() == 2);
            QVERIFY(list2.size() == 2);
            QVERIFY(list1 == list2);
        }

	{
	    int sum = 0;
	    for (int i = 0; i < list.size(); ++i)
		sum += list[i];
	    QVERIFY(sum == 21);
	}
	{
	    int sum = 0;
	    QList<int>::const_iterator i = list.begin();
	    while (i != list.end())
		sum += *i++;
	    QVERIFY(sum == 21);
	}
	{
	    int sum = 0;
	    QList<int>::ConstIterator i = list.begin();
	    while (i != list.end())
		sum += *i++;
	    QVERIFY(sum == 21);
	}
	{
	    int sum = 0;
	    int i;
	    for (i = 0; i < list.size(); ++i)
		list[i] = list[i] +1;
	    for (i = 0; i < list.size(); ++i)
		sum += list[i];
	    QVERIFY(sum == 21 + list.size());
	}
	{
	    int sum = 0;
	    int i;
	    for (i = 0; i < list.size(); ++i)
		--list[i];
	    for (i = 0; i < list.size(); ++i)
		sum += list[i];
	    QVERIFY(sum == 21);
	}
	{
	    QMutableListIterator<int> i(list);
	    while (i.hasNext())
		i.setValue(2*i.next());
	}
	{
	    int sum = 0;
	    QListIterator<int> i(list);
	    i.toBack();
	    while (i.hasPrevious())
		sum += i.previous();
	    QVERIFY(sum == 2*21);
	}
	{
	    QMutableListIterator<int> i(list);
	    i.toBack();
	    while (i.hasPrevious())
		i.setValue(2*i.previous());
	}
	{
	    int sum = 0;
	    QListIterator<int> i(list);
	    i.toBack();
	    while (i.hasPrevious())
		sum += i.previous();
	    QVERIFY(sum == 2*2*21);
	}
	{
	    QMutableListIterator<int> i(list);
	    while (i.hasNext()) {
		int a = i.next();
		i.insert(a);
	    }
	}
	{
	    int sum = 0;
	    QList<int>::iterator i = list.begin();
	    while (i != list.end())
		sum += *i++;
	    QVERIFY(sum == 2*2*2*21);
	}
	{
	    int duplicates = 0;
	    QListIterator<int> i(list);
	    while (i.hasNext()) {
		int a = i.next();
		if (i.hasNext() && a == i.peekNext())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 6);
	}
	{
	    int duplicates = 0;
	    QListIterator<int> i(list);
	    i.toBack();
	    while (i.hasPrevious()) {
		int a = i.previous();
		if (i.hasPrevious() && a == i.peekPrevious())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 6);
	}
	{
	    QMutableListIterator<int> i(list);
	    while (i.hasNext()) {
		int a = i.next();
		if (i.hasNext() &&
		     i.peekNext() == a)
		    i.remove();
	    }
	}
	{
	    int duplicates = 0;
	    QMutableListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious()) {
		int a = i.previous();
		if (i.hasPrevious() && a == i.peekPrevious())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 0);
	}
	{
	    QVERIFY(list.size() == 6);
	    QMutableListIterator<int> i = list;
	    while (i.hasNext()) {
		int a = i.peekNext();
		i.insert(42);
		QVERIFY(i.peekPrevious() == 42 && i.peekNext() == a);
		i.next();
	    }
	    QVERIFY(list.size() == 12);
	    i.toFront();
	    while (i.findNext(42))
		i.remove();
	}
	{
	    QList<int> l;
	    l << 4 << 8 << 12 << 16 << 20 << 24;
	    QVERIFY(l == list);
	    QList<int> copy = list;
	    list += list;
	    QVERIFY(l != list && l.size() == list.size()/2 && l == copy);
	    l += copy;
	    QVERIFY(l == list);
	    list = copy;
	}
	{
	    QList<int> copy = list;
	    list << 8;
	    QVERIFY(list.indexOf(8) == 1);
	    QVERIFY(list.indexOf(8, list.indexOf(8)+1) == 6);
	    int a = list.indexOf(8);
	    QVERIFY(list.count(8) == 2);
	    int r = list.removeAll(8);
	    QVERIFY(r == 2);
	    list.insert(a, 8);
	    QVERIFY(list == copy);
	}
        {
            QList<QString> list;
            list << "one" << "two" << "three" << "four" << "five" << "six";
            while (!list.isEmpty())
                list.removeAll(list.first());
        }
	{
	    QList<int> copy = list;
	    list << 8;
	    QVERIFY(list.lastIndexOf(8) == 6);
	    QVERIFY(list.lastIndexOf(8, list.lastIndexOf(8)-1) == 1);
	    list = copy;
	}
	{
	    QList<int> copy = list;
	    list.insert(3, 999);
	    QVERIFY(list[3] == 999);
	    list.replace(3, 222);
	    QVERIFY(list[3] == 222);
	    QVERIFY(list.contains(222) && ! list.contains(999));
	    list.removeAt(3);
	    list = copy;
	    QVERIFY(list == copy);
	}
	{
	    list.clear();
	    QVERIFY(list.isEmpty());
	    QVERIFY(list.begin() == list.end());
	    QListIterator<int> i(list);
	    QVERIFY(!i.hasNext() && !i.hasPrevious());
	}
	{
	    QList<int> l1;
	    QList<int> l2;
	    l1 << 1 << 2 << 3;
	    l2 << 4 << 5 << 6;
	    QList<int> l3 = l1 + l2;
	    l1 += l2;
	    QVERIFY(l3 == l1);
	}
	{
	    QList<int> list;
	    QVERIFY(list.isEmpty());
	    list.append(1);
	    QList<int> list2;
	    list2 = list;
	    list2.clear();
	    QVERIFY(list2.size() == 0);
	    QVERIFY(list.size() == 1);
	}
	{
	    QList<int> list;
	    list.append(1);
	    list = list;
	    QVERIFY(list.size() == 1);
	}
    }
    {
	QList<void*> list;
	list.append(0);
	list.append((void*)42);
	QCOMPARE(list.size(), 2);
	QCOMPARE(list.at(0), (void*)0);
	QCOMPARE(list.at(1), (void*)42);
    }

    {
	QVector<QString> vector(5);
        vector[0] = "99";
        vector[4] ="100";
        QList<QString> list = vector.toList();

	QVERIFY(list.size() == 5);
        QVERIFY(list.at(0) == "99");
        QVERIFY(list.at(4) == "100");
        list[0] = "10";
        QVERIFY(list.at(0) == "10");
        QVERIFY(vector.at(0) == "99");

    }

    {
        QList<QString> list;
        list.append("Hello");

        QList<QString>::iterator it = list.begin();
        QVERIFY((*it)[0] == QChar('H'));
        QVERIFY(it->constData()[0] == QChar('H'));
        it->replace(QChar('H'), QChar('X'));
        QVERIFY(list.first() == "Xello");

        QList<QString>::const_iterator cit = list.constBegin();
        QVERIFY((*cit).toLower() == "xello");
        QVERIFY(cit->toUpper() == "XELLO");
    }

    {
        QList<int *> list;
        QVERIFY(list.value(0) == 0);
        int i;
        list.append(&i);
        QVERIFY(list.value(0) == &i);
    }
    {
        QList<const int *> list;
        QVERIFY(list.value(0) == 0);
        int i;
        list.append(&i);
        QVERIFY(list.value(0) == &i);
    }
    {
        QList<int> list;
        QVERIFY(list.value(0) == 0);
        list.append(10);
        QVERIFY(list.value(0) == 10);
    }
    {
        QList<Pod> list;
        QCOMPARE(list.value(0).i1, 0);
        QCOMPARE(list.value(0).i2, 0);
    }

    {
        QList<QString> list;
        list << "alpha" << "beta";
        list += list;
        QVERIFY(list.size() == 4);
        QVERIFY(list.at(0) == "alpha");
        QVERIFY(list.at(1) == "beta");
        QVERIFY(list.at(2) == "alpha");
        QVERIFY(list.at(3) == "beta");
    }

    // test endcases for inserting into a qlist
    {
        QList<QString> list;
        list << "foo" << "bar";
        QVERIFY(!list.isEmpty());

        list.insert(-1, "lessthanzero");
        QCOMPARE(list.at(0), QString("lessthanzero"));

        list.insert(0, "atzero");
        QCOMPARE(list.at(0), QString("atzero"));

        int listCount = list.count();
        list.insert(listCount, "atcount");
        QCOMPARE(list.at(listCount), QString("atcount"));

        listCount = list.count();
        list.insert(listCount + 1, "beyondcount");
        QCOMPARE(list.at(listCount), QString("beyondcount"));
    }

    {
        QList<int> list1;
        list1 << 0 << 1 << 2 << 3;
        list1.removeFirst();

        list1.swap(0, 0);
        QVERIFY(list1 == QList<int>() << 1 << 2 << 3);

        list1.swap(1, 1);
        QVERIFY(list1 == QList<int>() << 1 << 2 << 3);

        list1.swap(2, 2);
        QVERIFY(list1 == QList<int>() << 1 << 2 << 3);

        list1.swap(0, 1);
        QVERIFY(list1 == QList<int>() << 2 << 1 << 3);

        list1.swap(0, 2);
        QVERIFY(list1 == QList<int>() << 3 << 1 << 2);

        list1.swap(1, 2);
        QVERIFY(list1 == QList<int>() << 3 << 2 << 1);

        list1.swap(1, 2);
        QVERIFY(list1 == QList<int>() << 3 << 1 << 2);

        QList<QString> list2;
        list2 << "1" << "2" << "3";

        list2.swap(0, 0);
        QVERIFY(list2 == QList<QString>() << "1" << "2" << "3");

        list2.swap(1, 1);
        QVERIFY(list2 == QList<QString>() << "1" << "2" << "3");

        list2.swap(2, 2);
        QVERIFY(list2 == QList<QString>() << "1" << "2" << "3");

        list2.swap(0, 1);
        QVERIFY(list2 == QList<QString>() << "2" << "1" << "3");

        list2.swap(0, 2);
        QVERIFY(list2 == QList<QString>() << "3" << "1" << "2");

        list2.swap(1, 2);
        QVERIFY(list2 == QList<QString>() << "3" << "2" << "1");

        list2.swap(1, 2);
        QVERIFY(list2 == QList<QString>() << "3" << "1" << "2");

        QList<double> list3;
        list3 << 1.0 << 2.0 << 3.0;

        list3.swap(0, 0);
        QVERIFY(list3 == QList<double>() << 1.0 << 2.0 << 3.0);

        list3.swap(1, 1);
        QVERIFY(list3 == QList<double>() << 1.0 << 2.0 << 3.0);

        list3.swap(2, 2);
        QVERIFY(list3 == QList<double>() << 1.0 << 2.0 << 3.0);

        list3.swap(0, 1);
        QVERIFY(list3 == QList<double>() << 2.0 << 1.0 << 3.0);

        list3.swap(0, 2);
        QVERIFY(list3 == QList<double>() << 3.0 << 1.0 << 2.0);

        list3.swap(1, 2);
        QVERIFY(list3 == QList<double>() << 3.0 << 2.0 << 1.0);

        list3.swap(1, 2);
        QVERIFY(list3 == QList<double>() << 3.0 << 1.0 << 2.0);
    }

    // Check what happens when using references to own items.
    // Ideally we should run valgrind on this.
    {
        int i;

        QList<void *> list1;
        list1.append(reinterpret_cast<void *>(50));

        for (i = 1; i < 100; ++i) {
            list1.append(list1.at(i - 1));
            list1.prepend(list1.at(i));
            list1.insert(i, list1.at(i - 1));
            list1.insert(i, list1.at(i));
            list1.insert(i, list1.at(i + 1));
            list1.replace(i, list1.at(i - 1));
            list1.replace(i, list1.at(i));
            list1.replace(i, list1.at(i + 1));
        }
        QCOMPARE(list1.size(), 496);
        for (i = 0; i < list1.size(); ++i) {
            QCOMPARE(list1.at(i), reinterpret_cast<void *>(50));
        }

        QList<QString> list2;
        list2.append("50");

        for (i = 1; i < 100; ++i) {
            list2.append(list2.at(i - 1));
            list2.prepend(list2.at(i));
            list2.insert(i, list2.at(i - 1));
            list2.insert(i, list2.at(i));
            list2.insert(i, list2.at(i + 1));
            list2.replace(i, list2.at(i - 1));
            list2.replace(i, list2.at(i));
            list2.replace(i, list2.at(i + 1));
        }
        QCOMPARE(list2.size(), 496);
        for (i = 0; i < list2.size(); ++i) {
            QCOMPARE(list2.at(i), QString::fromLatin1("50"));
        }

        QList<double> list3;
        list3.append(50.0);

        for (i = 1; i < 100; ++i) {
            list3.append(list3.at(i - 1));
            list3.prepend(list3.at(i));
            list3.insert(i, list3.at(i - 1));
            list3.insert(i, list3.at(i));
            list3.insert(i, list3.at(i + 1));
            list3.replace(i, list3.at(i - 1));
            list3.replace(i, list3.at(i));
            list3.replace(i, list3.at(i + 1));
        }
        QCOMPARE(list3.size(), 496);
        for (i = 0; i < list3.size(); ++i) {
            QCOMPARE(list3.at(i), 50.0);
        }

        QList<QTime> list4;
        list4.append(QTime(12, 34, 56));

        for (i = 1; i < 100; ++i) {
            list4.append(list4.at(i - 1));
            list4.prepend(list4.at(i));
            list4.insert(i, list4.at(i - 1));
            list4.insert(i, list4.at(i));
            list4.insert(i, list4.at(i + 1));
            list4.replace(i, list4.at(i - 1));
            list4.replace(i, list4.at(i));
            list4.replace(i, list4.at(i + 1));
        }
        QCOMPARE(list4.size(), 496);
        for (i = 0; i < list4.size(); ++i) {
            QVERIFY(list4.at(i) == QTime(12, 34, 56));
        }

    }
}

void tst_Collections::linkedList()
{
    {
	QLinkedList<int> list;
	QVERIFY(list.isEmpty());
	list.append(1);
	list.push_back(2);
	list += (3);
	list << 4 << 5 << 6;
	QVERIFY(!list.isEmpty());
	QVERIFY(list.size() == 6);
	{
	    int sum = 0;
	    QLinkedListIterator<int> i = list;
	    while (i.hasNext()) {
		sum += i.next();
	    }
	    QVERIFY(sum == 21);
	}
	{
	    int sum = 0;
	    QLinkedList<int>::const_iterator i = list.begin();
	    while (i != list.end())
		sum += *i++;
	    QVERIFY(sum == 21);
	}
	{
	    QMutableLinkedListIterator<int> i = list;
	    while (i.hasNext())
		i.setValue(2*i.next());
	}
	{
	    int sum = 0;
	    QLinkedListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious())
		sum += i.previous();
	    QVERIFY(sum == 2*21);
	}
	{
	    QMutableLinkedListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious())
		i.setValue(2*i.previous());
	}
	{
	    int sum = 0;
	    QLinkedListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious())
		sum += i.previous();
	    QVERIFY(sum == 2*2*21);
	}
	{
	    QMutableLinkedListIterator<int> i = list;
	    while (i.hasNext()) {
		int a = i.next();
		i.insert(a);
	    }
	}
	{
	    int sum = 0;
	    QLinkedList<int>::iterator i = list.begin();
	    while (i != list.end())
		sum += *i++;
	    QVERIFY(sum == 2*2*2*21);
	}
	{
	    int duplicates = 0;
	    QLinkedListIterator<int> i = list;
	    while (i.hasNext()) {
		int a = i.next();
		if (i.hasNext() && a == i.peekNext())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 6);
	}
	{
	    int duplicates = 0;
	    QLinkedListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious()) {
		int a = i.previous();
		if (i.hasPrevious() && a == i.peekPrevious())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 6);
	}
	{
	    QMutableLinkedListIterator<int> i = list;
	    while (i.hasNext()) {
		int a = i.next();
		if (i.hasNext() &&
		     i.peekNext() == a)
		    i.remove();
	    }
	}
	{
	    int duplicates = 0;
	    QMutableLinkedListIterator<int> i = list;
	    i.toBack();
	    while (i.hasPrevious()) {
		int a = i.previous();
		if (i.hasPrevious() && a == i.peekPrevious())
		    duplicates++;
	    }
	    QVERIFY(duplicates == 0);
	}
	{
	    QVERIFY(list.size() == 6);
	    QMutableLinkedListIterator<int> i = list;
	    while (i.hasNext()) {
		int a = i.peekNext();
		i.insert(42);
		QVERIFY(i.peekPrevious() == 42 && i.peekNext() == a);
		i.next();
	    }
	    QVERIFY(list.size() == 12);
	    i.toFront();
	    while (i.findNext(42))
		i.remove();
	}
	{
	    QLinkedList<int> l;
	    l << 4 << 8 << 12 << 16 << 20 << 24;
	    QVERIFY(l == list);
	    QLinkedList<int> copy = list;
	    list += list;
	    QVERIFY(l != list && l.size() == list.size()/2 && l == copy);
	    l += copy;
	    QVERIFY(l == list);
	    list = copy;
	}
	{
	    QLinkedList<int> copy = list;
	    list.prepend(999);
	    list.append(999);
	    QVERIFY(list.contains(999));
	    QVERIFY(list.count(999) == 2);
	    list.removeAll(999);
	    QVERIFY(list == copy);
	}
        {
            QLinkedList<QString> list;
            list << "one" << "two" << "three" << "four" << "five" << "six";
            while (!list.isEmpty())
                list.removeAll(list.first());
        }
        {
	    list.clear();
	    QVERIFY(list.isEmpty());
	    QVERIFY(list.begin() == list.end());
	    QLinkedListIterator<int> i(list);
	    QVERIFY(!i.hasNext() && !i.hasPrevious());
	}
    }

    {
        QLinkedList<QString> list;
        list.append("Hello");

        QLinkedList<QString>::iterator it = list.begin();
        QVERIFY((*it)[0] == QChar('H'));
        QVERIFY(it->constData()[0] == QChar('H'));
        it->replace(QChar('H'), QChar('X'));
        QVERIFY(list.first() == "Xello");

        QLinkedList<QString>::const_iterator cit = list.constBegin();
        QVERIFY((*cit).toLower() == "xello");
        QVERIFY(cit->toUpper() == "XELLO");
    }

    {
        QLinkedList<QString> list;
        list << "alpha" << "beta";
        list += list;
        QVERIFY(list.size() == 4);
        QVERIFY(*list.begin() == "alpha");
        QVERIFY(*(list.begin() + 1) == "beta");
        QVERIFY(*(list.begin() + 2) == "alpha");
        QVERIFY(*(list.begin() + 3) == "beta");
    }
};


void tst_Collections::vector()
{
    QVector<int> v1;
    v1 << 1 << 2 << 3;
    QVector<int> v2;
    v2 << 4 << 5;
    QVector<int> v3;
    v3 << 1 << 2 << 3 << 4 << 5;
    QVERIFY(v1 + v2 == v3);

    QVector<int> emptyVector;
    QCOMPARE(emptyVector.size(), 0);
#if 0
    // this should trigger an assert
    emptyVector.remove(3, -3);
    QCOMPARE(emptyVector.size(), 0);
#endif
    emptyVector.remove(0, 0);
    QCOMPARE(emptyVector.size(), 0);

    QVector<int> v4;
    v4 << 1 << 2 << 3;
    QCOMPARE(v4.size(), 3);
    v4.remove(1, 0);
    QCOMPARE(v4.size(), 3);

    QVector<int> v;
    v.append(2);
    QVERIFY(*v.begin() == 2);
    v.prepend(1);

    v << 3 << 4 << 5 << 6;
    QVERIFY(std::binary_search(v.begin(), v.end(), 2) == true);
    QVERIFY(std::binary_search(v.begin(), v.end(), 9) == false);
    QVERIFY(qBinaryFind(v.begin(), v.end(), 2) == v.begin() + 1);
    QVERIFY(qLowerBound(v.begin(), v.end(), 2) == v.begin() + 1);
    QVERIFY(qUpperBound(v.begin(), v.end(), 2) == v.begin() + 2);
    QVERIFY(qBinaryFind(v.begin(), v.end(), 9) == v.end());
    QVERIFY(qLowerBound(v.begin(), v.end(), 9) == v.end());
    QVERIFY(qUpperBound(v.begin(), v.end(), 9) == v.end());

    v.clear();
    v << 1 << 2 << 3;
    v.insert(v.begin(), 0);
    v.insert(v.end(), 4);
    v.insert(v.begin()+2, 9);

    QVector<int> result;
    result << 0 << 1 << 9 << 2 << 3 << 4;

    QVERIFY( v == result );

    v.clear();
    v << 1 << 2 << 3;
    v.insert(0, 0);
    v.insert(4, 4);
    v.insert(2, 9);

    QVERIFY( v == result );

    QVector<QString> vec;
    vec << "foo" << "bar";
    vec.reserve( 512 );
    QVERIFY(vec[0] == "foo");
    QVERIFY(vec[1] == "bar");

    int initialLargeStaticCount = LargeStatic::count;
    {
        QVector<LargeStatic> vector;
        vector.append(LargeStatic());
        vector.resize(0);
    }
    QCOMPARE(LargeStatic::count, initialLargeStaticCount);

    {
        QVector<QString> vector;
        vector << "alpha" << "beta";
        vector += vector;
        QVERIFY(vector.size() == 4);
        QVERIFY(vector.at(0) == "alpha");
        QVERIFY(vector.at(1) == "beta");
        QVERIFY(vector.at(2) == "alpha");
        QVERIFY(vector.at(3) == "beta");
    }

    int originalLargeStaticCount = LargeStatic::count;
    {
        QVector<LargeStatic> vector(5);
    }
    QVERIFY(LargeStatic::count == originalLargeStaticCount);
    {
        QVector<LargeStatic> vector(5);
        QList<LargeStatic> list = vector.toList();
    }
    QVERIFY(LargeStatic::count == originalLargeStaticCount);
    {
        QVector<LargeStatic> vector;
        LargeStatic *dummy = 0;
        for (int i = 0; i < 10000; ++i) {
            delete dummy;
            dummy = new LargeStatic;
            vector.append(LargeStatic());
        }
	delete dummy;
    }
    QVERIFY(LargeStatic::count == originalLargeStaticCount);

    int originalMovableCount = Movable::count;
    {
        QVector<Movable> vector(5);
    }
    QVERIFY(Movable::count == originalMovableCount);
    {
        QVector<Movable> vector(5);
        QList<Movable> list = vector.toList();
    }
    QVERIFY(Movable::count == originalMovableCount);
    {
        QVector<Movable> vector;
        Movable *dummy = 0;
        for (int i = 0; i < 10000; ++i) {
            delete dummy;
            dummy = new Movable;
            vector.append(Movable());
        }
        delete dummy;
    }
    QVERIFY(Movable::count == originalMovableCount);

    // Check what happens when using references to own items.
    // Ideally we should run valgrind on this.
    {
        int i;

        QVector<void *> vect1;
        vect1.append(reinterpret_cast<void *>(50));

        for (i = 1; i < 100; ++i) {
            vect1.append(vect1.at(i - 1));
            vect1.prepend(vect1.at(i));
            vect1.insert(i, vect1.at(i - 1));
            vect1.insert(i, vect1.at(i));
            vect1.insert(i, vect1.at(i + 1));
            vect1.replace(i, vect1.at(i - 1));
            vect1.replace(i, vect1.at(i));
            vect1.replace(i, vect1.at(i + 1));
        }
        QCOMPARE(vect1.size(), 496);
        for (i = 0; i < vect1.size(); ++i) {
            QCOMPARE(vect1.at(i), reinterpret_cast<void *>(50));
        }

        QVector<QString> vect2;
        vect2.append("50");

        for (i = 1; i < 100; ++i) {
            vect2.append(vect2.at(i - 1));
            vect2.prepend(vect2.at(i));
            vect2.insert(i, vect2.at(i - 1));
            vect2.insert(i, vect2.at(i));
            vect2.insert(i, vect2.at(i + 1));
            vect2.replace(i, vect2.at(i - 1));
            vect2.replace(i, vect2.at(i));
            vect2.replace(i, vect2.at(i + 1));
        }
        QCOMPARE(vect2.size(), 496);
        for (i = 0; i < vect2.size(); ++i) {
            QCOMPARE(vect2.at(i), QString::fromLatin1("50"));
        }

        QVector<double> vect3;
        vect3.append(50.0);

        for (i = 1; i < 100; ++i) {
            vect3.append(vect3.at(i - 1));
            vect3.prepend(vect3.at(i));
            vect3.insert(i, vect3.at(i - 1));
            vect3.insert(i, vect3.at(i));
            vect3.insert(i, vect3.at(i + 1));
            vect3.replace(i, vect3.at(i - 1));
            vect3.replace(i, vect3.at(i));
            vect3.replace(i, vect3.at(i + 1));
        }
        QCOMPARE(vect3.size(), 496);
        for (i = 0; i < vect3.size(); ++i) {
            QCOMPARE(vect3.at(i), 50.0);
        }

        QVector<QTime> vect4;
        vect4.append(QTime(12, 34, 56));

        for (i = 1; i < 100; ++i) {
            vect4.append(vect4.at(i - 1));
            vect4.prepend(vect4.at(i));
            vect4.insert(i, vect4.at(i - 1));
            vect4.insert(i, vect4.at(i));
            vect4.insert(i, vect4.at(i + 1));
            vect4.replace(i, vect4.at(i - 1));
            vect4.replace(i, vect4.at(i));
            vect4.replace(i, vect4.at(i + 1));
        }
        QCOMPARE(vect4.size(), 496);
        for (i = 0; i < vect4.size(); ++i) {
            QVERIFY(vect4.at(i) == QTime(12, 34, 56));
        }
    }
}

void tst_Collections::byteArray()
{
    QByteArray hello = "hello";
    QByteArray ello = "ello";
    QByteArray World = "World";
    QByteArray Wor = "Wor";
    QByteArray helloWorld = "helloWorld";
    QVERIFY(hello + World == helloWorld);
    QVERIFY(hello + "World" == helloWorld);
    QVERIFY("hello" + World == helloWorld);


    QByteArray l;
    QVERIFY('h' + ello == hello);
    QVERIFY(Wor + 'l' + 'd' == "World");
    QVERIFY(hello + World == "helloWorld");
    QVERIFY(hello + "World" == "helloWorld");
    QVERIFY("hello" + World == "helloWorld");
    QVERIFY('h' + ello == "hello");
    QVERIFY(Wor + 'l' + 'd' == "World");
    QVERIFY("helloWorld" == hello + World);
    QVERIFY("helloWorld" == hello + "World");
    QVERIFY("helloWorld" == "hello" + World);
    QVERIFY("hello" == 'h' + ello);
    QVERIFY("World" == Wor + 'l' + 'd');

    QVERIFY(hello.contains('e'));
    QVERIFY (true == hello.contains('e'));
    QVERIFY (hello.contains('e') != false);

    QVERIFY(hello.indexOf('e') == 1);
    QVERIFY(hello.indexOf('e', -10) == 1);
    QVERIFY(hello.indexOf('l') == 2);
    QVERIFY(hello.indexOf('l',2) == 2);
    QVERIFY(hello.indexOf('l',3) == 3);

    QByteArray large = "000 100 200 300 400 500 600 700 800 900";

    QVERIFY(large.indexOf("700") == 28);
    QVERIFY(large.indexOf("700", 28) == 28);
    QVERIFY(large.indexOf("700", 29) == -1);
    QVERIFY(large.lastIndexOf("700") == 28);
    QVERIFY(large.lastIndexOf("700", 28) == 28);
    QVERIFY(large.lastIndexOf("700", 27) == -1);

    QVERIFY(large.contains("200"));
    QVERIFY(!large.contains("201"));
    QVERIFY(large.contains('3'));
    QVERIFY(!large.contains('a'));

    QVERIFY(large.count("00") == 11);
    QVERIFY(large.count('3') == 1);
    QVERIFY(large.count('0') == 21);
    QVERIFY(large.count("0") == 21);
    QVERIFY(large.count("200") == 1);
    QVERIFY(large.count("201") == 0);

    QVERIFY(hello.left(0) == "");
    QVERIFY(!hello.left(0).isNull());
    QVERIFY(hello.left(1) == "h");
    QVERIFY(hello.left(2) == "he");
    QVERIFY(hello.left(200) == "hello");
    QVERIFY(hello.left(hello.size()) == hello);
    QVERIFY(hello.left(hello.size()+1) == hello);

    QVERIFY(hello.right(0) == "");
    QVERIFY(!hello.right(0).isNull());
    QVERIFY(hello.right(1) == "o");
    QVERIFY(hello.right(2) == "lo");
    QVERIFY(hello.right(200) == "hello");
    QVERIFY(hello.right(hello.size()) == hello);
    QVERIFY(hello.right(hello.size()+1) == hello);

    QVERIFY(!hello.mid(0, 0).isNull());
    QVERIFY(hello.mid(0, 1) == "h");
    QVERIFY(hello.mid(0, 2) == "he");
    QVERIFY(hello.mid(0, 200) == "hello");
    QVERIFY(hello.mid(0) == "hello");
    QVERIFY(hello.mid(0, hello.size()) == hello);
    QVERIFY(hello.mid(0, hello.size()+1) == hello);

    QVERIFY(hello.mid(hello.size()-0) == "");

    QVERIFY(hello.mid(hello.size()-0).isNull()); // weird but valid 3.x semantics
    QVERIFY(hello.mid(hello.size()-1) == "o");
    QVERIFY(hello.mid(hello.size()-2) == "lo");
    QVERIFY(hello.mid(hello.size()-200) == "hello");

    QByteArray null;
    QByteArray nonNull = "";
    QVERIFY(null.left(10).isNull());
    QVERIFY(null.mid(0).isNull());

#if 0
    QVERIFY(null == QByteArray::null);
    QVERIFY(QByteArray::null  == null);
    QVERIFY(nonNull != QByteArray::null);
    QVERIFY(QByteArray::null != nonNull);
    QVERIFY(null == nonNull);
    QVERIFY(QByteArray::null == QByteArray::null);
#endif

    QByteArray str = "Hello";
    QByteArray cstr(str.data(), str.size());
    QVERIFY(str == "Hello");
    QVERIFY(cstr == "Hello");
    cstr.clear();
    QVERIFY(str == "Hello");
    QVERIFY(cstr.isEmpty());

    {
        QByteArray ba1("Foo");
        ba1.prepend(ba1);
        QCOMPARE(ba1, QByteArray("FooFoo"));
        ba1.append(ba1);
        QCOMPARE(ba1, QByteArray("FooFooFooFoo"));
        ba1.insert(2, ba1);
        QCOMPARE(ba1, QByteArray("FoFooFooFooFoooFooFooFoo"));
        ba1.replace(3, 3, ba1);
        QCOMPARE(ba1, QByteArray("FoFFoFooFooFooFoooFooFooFooooFooFoooFooFooFoo"));
        ba1 = "FooFoo";
        ba1.replace(char('F'), ba1);
        QCOMPARE(ba1, QByteArray("FooFooooFooFoooo"));

        ba1.replace(ba1, "xxx");
        QCOMPARE(ba1, QByteArray("xxx"));
        ba1.replace(ba1, QByteArray("yyy"));
        QCOMPARE(ba1, QByteArray("yyy"));
        ba1 += ba1;
        QCOMPARE(ba1, QByteArray("yyyyyy"));

	ba1.remove(1, -1); // do nothing
	QCOMPARE(ba1, QByteArray("yyyyyy"));

	ba1.replace(0, -1, "ZZZ");
	QCOMPARE(ba1, QByteArray("ZZZyyyyyy"));
    }
};

void tst_Collections::stack()
{
    QStack<int> stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    QVectorIterator<int> i = stack;
    i.toBack();
    int sum = 0;
    while (i.hasPrevious())
	sum += i.previous();
    QVERIFY(sum == 6);

    sum = 0;
    for (QStack<int>::iterator i = stack.begin(); i != stack.end(); ++i)
	sum += *i;
    QVERIFY(sum == 6);

    while (!stack.isEmpty())
	sum -= stack.pop();
    QVERIFY(sum == 0);
}

void tst_Collections::hash()
{
    const char *hello = "hello";
    const char *world = "world";
    const char *allo = "allo";
    const char *monde = "monde";

    {
	typedef QHash<QString, QString> Hash;
	Hash hash;
	QString key;
	for (int i = 0; i < 10; ++i) {
	    key[0] = i + '0';
	    for (int j = 0; j < 10; ++j) {
		key[1] = j + '0';
		hash.insert(key, "V" + key);
	    }
	}

	for (int i = 0; i < 10; ++i) {
	    key[0] = i + '0';
	    for (int j = 0; j < 10; ++j) {
		key[1] = j + '0';
		hash.remove(key);
	    }
	}
    }

    {
	typedef QHash<int, const char *> Hash;
	Hash hash;
	hash.insert(1, hello);
	hash.insert(2, world);

	QVERIFY(hash.size() == 2);
	QVERIFY(!hash.isEmpty());

	{
	    Hash hash2 = hash;
	    hash2 = hash;
	    hash = hash2;
	    hash2 = hash2;
	    hash = hash;
	    hash2.clear();
	    hash2 = hash2;
	    QVERIFY(hash2.size() == 0);
	    QVERIFY(hash2.isEmpty());
	}
	QVERIFY(hash.size() == 2);

	{
	    Hash hash2 = hash;
	    hash2[1] = allo;
	    hash2[2] = monde;

	    QVERIFY(hash2[1] == allo);
	    QVERIFY(hash2[2] == monde);
	    QVERIFY(hash[1] == hello);
	    QVERIFY(hash[2] == world);

	    hash2[1] = hash[1];
	    hash2[2] = hash[2];

	    QVERIFY(hash2[1] == hello);
	    QVERIFY(hash2[2] == world);

	    hash[1] = hash[1];
	    QVERIFY(hash[1] == hello);
	}

	{
	    Hash hash2 = hash;
	    hash2.detach();
	    hash2.remove(1);
	    QVERIFY(hash2.size() == 1);
	    hash2.remove(1);
	    QVERIFY(hash2.size() == 1);
	    hash2.remove(0);
	    QVERIFY(hash2.size() == 1);
	    hash2.remove(2);
	    QVERIFY(hash2.size() == 0);
	    QVERIFY(hash.size() == 2);
	}

	hash.detach();

	{
	    Hash::iterator it1 = hash.find(1);
	    QVERIFY(it1 != hash.end());

	    Hash::iterator it2 = hash.find(0);
	    QVERIFY(it2 != hash.begin());
	    QVERIFY(it2 == hash.end());

	    *it1 = monde;
	    QVERIFY(*it1 == monde);
	    QVERIFY(hash[1] == monde);

	    *it1 = hello;
	    QVERIFY(*it1 == hello);
	    QVERIFY(hash[1] == hello);

	    hash[1] = monde;
	    QVERIFY(it1.key() == 1);
	    QVERIFY(it1.value() == monde);
	    QVERIFY(*it1 == monde);
	    QVERIFY(hash[1] == monde);

	    hash[1] = hello;
	    QVERIFY(*it1 == hello);
	    QVERIFY(hash[1] == hello);
	}

	{
	    const Hash hash2 = hash;

	    Hash::const_iterator it1 = hash2.find(1);
	    QVERIFY(it1 != hash2.end());
	    QVERIFY(it1.key() == 1);
	    QVERIFY(it1.value() == hello);
	    QVERIFY(*it1 == hello);

	    Hash::const_iterator it2 = hash2.find(2);
	    QVERIFY(it1 != it2);
	    QVERIFY(it1 != hash2.end());
	    QVERIFY(it2 != hash2.end());

	    int count = 0;
	    it1 = hash2.begin();
	    while (it1 != hash2.end()) {
		count++;
		++it1;
	    }
	    QVERIFY(count == 2);

	    count = 0;
	    it1 = hash.begin();
	    while (it1 != hash.end()) {
		count++;
		++it1;
	    }
	    QVERIFY(count == 2);
	}

	{
	    QVERIFY(hash.contains(1));
	    QVERIFY(hash.contains(2));
	    QVERIFY(!hash.contains(0));
	    QVERIFY(!hash.contains(3));
	}

	{
	    QVERIFY(hash.value(1) == hello);
	    QVERIFY(hash.value(2) == world);
	    QVERIFY(hash.value(3) == 0);
	    QVERIFY(hash.value(1, allo) == hello);
	    QVERIFY(hash.value(2, allo) == world);
	    QVERIFY(hash.value(3, allo) == allo);
	    QVERIFY(hash.value(0, monde) == monde);
	}

	{
	    QHash<int,LargeStatic> hash;
	    for (int i = 0; i < 10; i++)
		hash.insert(i, LargeStatic());
	    QVERIFY(LargeStatic::count == 10);
	    hash.remove(7);
	    QVERIFY(LargeStatic::count == 9);

	}
	QVERIFY(LargeStatic::count == 0);
	{
	    QHash<int, int*> hash;
	    QVERIFY(((const QHash<int,int*>*) &hash)->operator[](7) == 0);
	}

	{
	    /*
                This test relies on a certain implementation of
                QHash. If you change the way QHash works internally,
                change this test as well.
            */
	    QHash<int, int> hash;
            for (int i = 0; i < 1000; ++i)
		hash.insert(i, i);
	    QVERIFY(hash.capacity() == 1031);
            hash.squeeze();
            QVERIFY(hash.capacity() == 521);

	    hash.insert(12345, 12345);
            QVERIFY(hash.capacity() == 1031);

	    for (int j = 0; j < 900; ++j)
		hash.remove(j);
            QVERIFY(hash.capacity() == 257);
	    hash.squeeze();
            QVERIFY(hash.capacity() == 67);
            hash.reserve(0);
        }
    }

    {
        QHash<int, QString> hash;
        hash.insert(0, "Hello");

        QHash<int, QString>::iterator it = hash.begin();
        QVERIFY((*it)[0] == QChar('H'));
        QVERIFY(it->constData()[0] == QChar('H'));
        it->replace(QChar('H'), QChar('X'));
        QVERIFY(*hash.begin() == "Xello");

        QHash<int, QString>::const_iterator cit = hash.constBegin();
        QVERIFY((*cit).toLower() == "xello");
        QVERIFY(cit->toUpper() == "XELLO");
    }

    {
        QHash<int, QString> hash1, hash2;
        hash1.insertMulti(1, "Alpha");
        hash1.insertMulti(1, "Gamma");
        hash2.insertMulti(1, "Beta");
        hash2.insertMulti(1, "Gamma");
        hash2.insertMulti(1, "Gamma");

        hash1.unite(hash2);
        QCOMPARE(hash1.size(), 5);
        QCOMPARE(hash1.values(),
                (QList<QString>() << "Gamma" << "Gamma" << "Beta" << "Gamma" << "Alpha"));

        hash2 = hash1;
        hash2.unite(hash2);
        QCOMPARE(hash2.size(), 10);
        QCOMPARE(hash2.values(), hash1.values() + hash1.values());
    }

#if 0
    {
        QMultiHash<QString, int> hash1;
        hash1.insert("1", 2);
        hash1.insert("1", 1);
        hash1.insert("a", 3);
        hash1.insert("a", 2);
        hash1.insert("a", 1);
        hash1.insert("b", 2);
        hash1.insert("b", 1);

        // hash1: [ "1" -> 1, "1" -> 2, "a" -> 1, "a" -> 2, "a" -> 3, "b" -> 1, "b" -> 2 ]

        QHashIterator<QString, int> i1(hash1);
        i1.toBack();

        bool found;

        found = i1.findPreviousKey("1");
        QVERIFY(found && i1.value() == 2);

        found = i1.findPreviousKey("1");
        QVERIFY(found && i1.value() == 1);

        found = i1.findPreviousKey("1");
        QVERIFY(!found);

        i1.toFront();
        found = i1.findNextKey("1");
        QVERIFY(found && i1.value() == 1);

        found = i1.findNextKey("1");
        QVERIFY(found && i1.value() == 2);

        found = i1.findNextKey("1");
        QVERIFY(!found);

        i1.toBack();
        found = i1.findPreviousKey("b");
        QVERIFY(found && i1.value() == 2);

        found = i1.findPreviousKey("b");
        QVERIFY(found && i1.value() == 1);

        found = i1.findPreviousKey("b");
        QVERIFY(!found);

        i1.toFront();
        found = i1.findNextKey("b");
        QVERIFY(found && i1.value() == 1);

        found = i1.findNextKey("b");
        QVERIFY(found && i1.value() == 2);

        found = i1.findNextKey("b");
        QVERIFY(!found);
    }
#endif
}

void tst_Collections::map()
{
    const char *hello = "hello";
    const char *world = "world";
    const char *allo = "allo";
    const char *monde = "monde";

    {
	typedef QMap<int, const char *> Map;
	Map map;
	map.insert(1, hello);
	map.insert(2, world);

	QVERIFY(*map.begin() == hello);

	QVERIFY(map.size() == 2);
	QVERIFY(!map.isEmpty());

	{
	    Map map2 = map;
	    map2 = map;
	    map = map2;
	    map2 = map2;
	    map = map;
	    map2.clear();
	    map2 = map2;
	    QVERIFY(map2.size() == 0);
	    QVERIFY(map2.isEmpty());
	}
	QVERIFY(map.size() == 2);

	{
	    Map map2 = map;
	    map2[1] = allo;
	    map2[2] = monde;

	    QVERIFY(map2[1] == allo);
	    QVERIFY(map2[2] == monde);
	    QVERIFY(map[1] == hello);
	    QVERIFY(map[2] == world);

	    map2[1] = map[1];
	    map2[2] = map[2];

	    QVERIFY(map2[1] == hello);
	    QVERIFY(map2[2] == world);

	    map[1] = map[1];
	    QVERIFY(map[1] == hello);
	}

	{
	    Map map2 = map;
	    map2.detach();
	    map2.remove(1);
	    QVERIFY(map2.size() == 1);
	    map2.remove(1);
	    QVERIFY(map2.size() == 1);
	    map2.remove(0);
	    QVERIFY(map2.size() == 1);
	    map2.remove(2);
	    QVERIFY(map2.size() == 0);
	    QVERIFY(map.size() == 2);
	}

	map.detach();

	{
	    Map::iterator it1 = map.find(1);
	    QVERIFY(it1 == map.begin());
	    QVERIFY(it1 != map.end());

	    Map::iterator it2 = map.find(0);
	    QVERIFY(it2 != map.begin());
	    QVERIFY(it2 == map.end());

	    *it1 = monde;
	    QVERIFY(*it1 == monde);
	    QVERIFY(map[1] == monde);

	    *it1 = hello;
	    QVERIFY(*it1 == hello);
	    QVERIFY(map[1] == hello);

	    map[1] = monde;
	    QVERIFY(it1.key() == 1);
	    QVERIFY(it1.value() == monde);
	    QVERIFY(*it1 == monde);
	    QVERIFY(map[1] == monde);

	    map[1] = hello;
	    QVERIFY(*it1 == hello);
	    QVERIFY(map[1] == hello);

	    *++it1 = allo;
	    QVERIFY(*it1 == allo);
	    QVERIFY(map[2] == allo);
	    *it1 = world;

	    ++it1;
	    QVERIFY(it1 == map.end());

	    int count = 0;
	    it1 = map.begin();
	    while (it1 != map.end()) {
		count++;
		++it1;
	    }
	    QVERIFY(count == 2);
	}

	{
	    const Map map2 = map;

	    Map::const_iterator it1 = map2.find(1);
	    QVERIFY(it1 != map2.end());
	    QVERIFY(it1.key() == 1);
	    QVERIFY(it1.value() == hello);
	    QVERIFY(*it1 == hello);
	    ++it1;

	    Map::const_iterator it2 = map2.find(2);
	    QVERIFY(it1 == it2);
	    ++it1;
	    QVERIFY(it1 == map2.end());
	    QVERIFY(it2 != map2.end());
	    QVERIFY(it1 != it2);

	    int count = 0;
	    it1 = map2.begin();
	    while (it1 != map2.end()) {
		count++;
		++it1;
	    }
	    QVERIFY(count == 2);

	    count = 0;
	    it1 = map.begin();
	    while (it1 != map.end()) {
		count++;
		++it1;
	    }
	    QVERIFY(count == 2);
	}

	{
	    QVERIFY(map.contains(1));
	    QVERIFY(map.contains(2));
	    QVERIFY(!map.contains(0));
	    QVERIFY(!map.contains(3));
	}

	{
	    QVERIFY(map.value(1) == hello);
	    QVERIFY(map.value(2) == world);
	    QVERIFY(map.value(3) == 0);
	    QVERIFY(map.value(1, allo) == hello);
	    QVERIFY(map.value(2, allo) == world);
	    QVERIFY(map.value(3, allo) == allo);
	    QVERIFY(map.value(0, monde) == monde);
	}
	int originalLargeStaticCount = LargeStatic::count;
	{
	    QMap<int,LargeStatic> map;
	    for (int i = 0; i < 10; i++)
		map.insert(i, LargeStatic());
            QVERIFY(LargeStatic::count == (originalLargeStaticCount + 10));
	    map.remove(7);
	    QVERIFY(LargeStatic::count == (originalLargeStaticCount + 9));

	}
	QVERIFY(LargeStatic::count == originalLargeStaticCount);
	{
	    QMap<int, int*> map;
	    QVERIFY(((const QMap<int,int*>*) &map)->operator[](7) == 0);
	}

	{
	    QMap<int, int> map;
	    map[0] = 1;
	    map[1] = 2;
	    map[2] = 4;
	    map[3] = 8;
	    int sum = 0;
	    int sumkey = 0;
	    QMapIterator<int,int> i = map;
	    while (i.hasNext()) {
		sum += i.next().value();
		sumkey += i.key();
	    }
	    QVERIFY(sum == 15);
	    QVERIFY(sumkey == 6);
	}
	{
	    QMap<int, int> map;
	    map[0] = 1;
	    map[1] = 2;
	    map[2] = 4;
	    map[3] = 8;
	    int sum = 0;
	    QMutableMapIterator<int,int> i = map;
	    while(i.hasNext())
		if (i.next().key()  == 2)
		    i.remove();
	    i.toFront();
	    while(i.hasNext()) {
		sum += i.next().value();
#if QT_VERSION >= 0x040100
                i.setValue(10);
                i.value() += 22;
                QVERIFY(i.value() == 32);
#endif
            }
	    QVERIFY(sum == 11);
	}
        {
            QMap<int, int> map;
            map[0] = 1;
            QMutableMapIterator<int,int> i(map);
            i.toBack();
            while (i.hasPrevious()) {
                i.previous();
                QCOMPARE(i.key(), 0);
                QCOMPARE(i.value(), 1);
            }
        }
    }

    {
        QMultiMap<QString, int> map1;
        map1.insert("1", 2);
        map1.insert("1", 1);
        map1.insert("a", 3);
        map1.insert("a", 2);
        map1.insert("a", 1);
        map1.insert("b", 2);
        map1.insert("b", 1);

	// map1: [ "1" -> 1, "1" -> 2, "a" -> 1, "a" -> 2, "a" -> 3, "b" -> 1, "b" -> 2 ]

#if 0
        QMapIterator<QString, int> i1(map1);
        i1.toBack();

        bool found;
        found = i1.findPreviousKey("1");
	QVERIFY(found && i1.value() == 2);

        found = i1.findPreviousKey("1");
	QVERIFY(found && i1.value() == 1);

        found = i1.findPreviousKey("1");
        QVERIFY(!found);

	i1.toFront();
        found = i1.findNextKey("1");
        QVERIFY(found && i1.value() == 1);

        found = i1.findNextKey("1");
        QVERIFY(found && i1.value() == 2);

        found = i1.findNextKey("1");
        QVERIFY(!found);

	i1.toBack();
        found = i1.findPreviousKey("b");
        QVERIFY(found && i1.value() == 2);

        found = i1.findPreviousKey("b");
        QVERIFY(found && i1.value() == 1);

        found = i1.findPreviousKey("b");
        QVERIFY(!found);

	i1.toFront();
        found = i1.findNextKey("b");
        QVERIFY(found && i1.value() == 1);

        found = i1.findNextKey("b");
        QVERIFY(found && i1.value() == 2);

        found = i1.findNextKey("b");
        QVERIFY(!found);
#endif

        QMultiMap<QString, int>::iterator j1, k1;

        j1 = map1.lowerBound("0"); k1 = map1.upperBound("0");
        QVERIFY(j1 == map1.begin() && k1 == j1);
        j1 = map1.lowerBound("00"); k1 = map1.upperBound("00");
        QVERIFY(j1 == map1.find("1") && k1 == j1);
        j1 = map1.lowerBound("1"); k1 = map1.upperBound("1");
        QVERIFY(j1 == map1.find("1") && --(--k1) == j1);
        j1 = map1.lowerBound("11"); k1 = map1.upperBound("11");
        QVERIFY(j1 == map1.find("a") && k1 == j1);
        j1 = map1.lowerBound("a"); k1 = map1.upperBound("a");
        QVERIFY(j1 == map1.find("a") && k1 == map1.find("b"));
        QVERIFY(j1.value() == 1 && j1.value() == 1);
        j1 = map1.lowerBound("aa"); k1 = map1.upperBound("aa");
        QVERIFY(j1 == map1.find("b") && k1 == j1);
        QVERIFY(j1.value() == 1);
        j1 = map1.lowerBound("b"); k1 = map1.upperBound("b");
        QVERIFY(j1 == map1.find("b") && k1 == map1.end());
        QVERIFY(j1.value() == 1);
        j1 = map1.lowerBound("bb"); k1 = map1.upperBound("bb");
        QVERIFY(j1 == map1.end() && k1 == j1);

        const QMultiMap<QString, int> map2 = map1;
        QMultiMap<QString, int>::const_iterator j2, k2;

        j2 = map2.lowerBound("0"); k2 = map2.upperBound("0");
        QVERIFY(j2 == map2.begin() && k2 == j2);
        j2 = map2.lowerBound("00"); k2 = map2.upperBound("00");
        QVERIFY(j2 == map2.find("1") && k2 == j2);
        j2 = map2.lowerBound("1"); k2 = map2.upperBound("1");
        QVERIFY(j2 == map2.find("1") && --(--k2) == j2);
        j2 = map2.lowerBound("11"); k2 = map2.upperBound("11");
        QVERIFY(j2 == map2.find("a") && k2 == j2);
        j2 = map2.lowerBound("a"); k2 = map2.upperBound("a");
        QVERIFY(j2 == map2.find("a") && k2 == map2.find("b"));
        QVERIFY(j2.value() == 1 && j2.value() == 1);
        j2 = map2.lowerBound("aa"); k2 = map2.upperBound("aa");
        QVERIFY(j2 == map2.find("b") && k2 == j2);
        QVERIFY(j2.value() == 1);
        j2 = map2.lowerBound("b"); k2 = map2.upperBound("b");
        QVERIFY(j2 == map2.find("b") && k2 == map2.end());
        QVERIFY(j2.value() == 1);
        j2 = map2.lowerBound("bb"); k2 = map2.upperBound("bb");
        QVERIFY(j2 == map2.end() && k2 == j2);
    }

    {
        QMap<int, QString> map;
        map.insert(0, "Hello");

        QMap<int, QString>::iterator it = map.begin();
        QVERIFY((*it)[0] == QChar('H'));
        QVERIFY(it->constData()[0] == QChar('H'));
        it->replace(QChar('H'), QChar('X'));
        QVERIFY(*map.begin() == "Xello");

        QMap<int, QString>::const_iterator cit = map.constBegin();
        QVERIFY((*cit).toLower() == "xello");
        QVERIFY(cit->toUpper() == "XELLO");
    }

    {
        QMap<int, QString> map1, map2;
        map1.insertMulti(1, "Alpha");
        map1.insertMulti(1, "Gamma");
        map2.insertMulti(1, "Beta");
        map2.insertMulti(1, "Gamma");
        map2.insertMulti(1, "Gamma");

        map1.unite(map2);
        QCOMPARE(map1.size(), 5);
        QCOMPARE(static_cast<QStringList>(map1.values()),
                (QStringList() << "Gamma" << "Gamma" << "Beta" << "Gamma" << "Alpha"));

        map2 = map1;
        map2.unite(map2);
        QCOMPARE(map2.size(), 10);
        QCOMPARE(map2.values(), map1.values() + map1.values());
    }
}

void tst_Collections::qstring()
{
    QString hello = "hello";
    QString ello = "ello";
    QString World = "World";
    QString Wor = "Wor";
    QString helloWorld = "helloWorld";

    QString s = hello + "World";
    QVERIFY(hello + World == helloWorld);
    QVERIFY(hello + "World" == helloWorld);
    QVERIFY("hello" + World == helloWorld);


    QString l;
    QVERIFY('h' + ello == hello);
    QVERIFY(Wor + 'l' + 'd' == "World");
    QVERIFY(hello + World == "helloWorld");
    QVERIFY(hello + "World" == "helloWorld");
    QVERIFY("hello" + World == "helloWorld");
    QVERIFY('h' + ello == "hello");
    QVERIFY(Wor + 'l' + 'd' == "World");
    QVERIFY("helloWorld" == hello + World);
    QVERIFY("helloWorld" == hello + "World");
    QVERIFY("helloWorld" == "hello" + World);
    QVERIFY("hello" == 'h' + ello);
    QVERIFY("World" == Wor + 'l' + 'd');

    QVERIFY(hello.contains('e'));
    QVERIFY (true == hello.contains('e'));
    QVERIFY (hello.contains('e') != false);

    QVERIFY(hello.indexOf('e') == 1);
    QVERIFY(hello.indexOf('e', -10) == 1);
    QVERIFY(hello.indexOf('l') == 2);
    QVERIFY(hello.indexOf('l',2) == 2);
    QVERIFY(hello.indexOf('l',3) == 3);

    QString large = "000 100 200 300 400 500 600 700 800 900";

    QVERIFY(large.indexOf("700") == 28);
    QVERIFY(large.indexOf("700", 28) == 28);
    QVERIFY(large.indexOf("700", 29) == -1);
    QVERIFY(large.lastIndexOf("700") == 28);
    QVERIFY(large.lastIndexOf("700", 28) == 28);
    QVERIFY(large.lastIndexOf("700", 27) == -1);

    QVERIFY(large.contains("200"));
    QVERIFY(!large.contains("201"));
    QVERIFY(large.contains('3'));
    QVERIFY(!large.contains('a'));

    QVERIFY(large.count("00") == 11);
    QVERIFY(large.count('3') == 1);
    QVERIFY(large.count('0') == 21);
    QVERIFY(large.count("0") == 21);
    QVERIFY(large.count("200") == 1);
    QVERIFY(large.count("201") == 0);

    QVERIFY(hello.left(0) == "");
    QVERIFY(!hello.left(0).isNull());
    QVERIFY(hello.left(1) == "h");
    QVERIFY(hello.left(2) == "he");
    QVERIFY(hello.left(200) == "hello");
    QVERIFY(hello.left(hello.size()) == hello);
    QVERIFY(hello.left(hello.size()+1) == hello);

    QVERIFY(hello.right(0) == "");
    QVERIFY(!hello.right(0).isNull());
    QVERIFY(hello.right(1) == "o");
    QVERIFY(hello.right(2) == "lo");
    QVERIFY(hello.right(200) == "hello");
    QVERIFY(hello.right(hello.size()) == hello);
    QVERIFY(hello.right(hello.size()+1) == hello);

    QVERIFY(!hello.mid(0, 0).isNull());
    QVERIFY(hello.mid(0, 1) == "h");
    QVERIFY(hello.mid(0, 2) == "he");
    QVERIFY(hello.mid(0, 200) == "hello");
    QVERIFY(hello.mid(0) == "hello");
    QVERIFY(hello.mid(0, hello.size()) == hello);
    QVERIFY(hello.mid(0, hello.size()+1) == hello);

    QVERIFY(hello.mid(hello.size()-0) == "");

    QVERIFY(hello.mid(hello.size()-0).isNull());
    QVERIFY(hello.mid(hello.size()-1) == "o");
    QVERIFY(hello.mid(hello.size()-2) == "lo");
    QVERIFY(hello.mid(hello.size()-200) == "hello");

    QString null;
    QString nonNull = "";
    QVERIFY(null.left(10).isNull());
    QVERIFY(null.mid(0).isNull());

    QVERIFY(null == QString::null);
    QVERIFY(QString::null  == null);
    QVERIFY(nonNull != QString::null);
    QVERIFY(QString::null != nonNull);
    QVERIFY(null == nonNull);
    QVERIFY(QString::null == QString::null);

    QString fill = "123";
    fill.fill('a');
    QVERIFY(fill == "aaa");

    s.clear();
    s = hello;
    s.append(World);
    QVERIFY(s == helloWorld);
    s.clear();
    s = World;
    s.insert(0,hello);
    QVERIFY(s == helloWorld);
    s = "012345";
    s.insert(3, 'E');
    QVERIFY(s == "012E345");
    s.insert(3, "INSID");
    QVERIFY(s == "012INSIDE345");
    s = "short";
    s.insert(7, 'E');
    QVERIFY(s == "short  E");
    s = "short";
    s.insert(7, "END");
    QVERIFY(s == "short  END");

    QVERIFY(QString::fromLatin1("hello") == "hello");

    s = "first";
    QVERIFY(s.toLatin1() == "first");
    s = "second";
    QVERIFY(s.toLatin1() == "second");
#ifdef QT3_SUPPORT
    const char* cache = s.latin1();
    QVERIFY(cache == s.latin1());
#endif
    s.clear();
#ifdef QT3_SUPPORT
    QVERIFY(*s.latin1() == '\0');
#endif
    QVERIFY(s.isNull());
    QVERIFY(s.toLatin1().size() == 0);
    QVERIFY(s.toLatin1().isEmpty());
    QVERIFY(s.toLatin1().isNull());

    s = "first-utf8";
    QVERIFY(s.toUtf8() == "first-utf8");
    s = "second-utf8";
    QVERIFY(s.toUtf8() == "second-utf8");
#ifdef QT3_SUPPORT
    cache = s.utf8();
    QVERIFY(cache == s.utf8());
#endif
    s.clear();
#ifdef QT3_SUPPORT
    QVERIFY(*s.utf8() == '\0');
#endif
    QVERIFY(s.isNull());
    QVERIFY(s.toUtf8().size() == 0);
    QVERIFY(s.toUtf8().isEmpty());
    QVERIFY(s.toUtf8().isNull());

    s = "first-utf8";
    QVERIFY(s.toUtf8() == "first-utf8");
    s = "second-utf8";
    QVERIFY(s.toUtf8() == "second-utf8");
#ifdef QT3_SUPPORT
    cache = s.utf8();
    QVERIFY(cache == s.utf8());
#endif
    s.clear();
#ifdef QT3_SUPPORT
    QVERIFY(*s.utf8() == '\0');
#endif
    QVERIFY(s.isNull());
    QVERIFY(s.toUtf8().size() == 0);
    QVERIFY(s.toUtf8().isEmpty());
    QVERIFY(s.toUtf8().isNull());

    s = "first-local8Bit";
    QVERIFY(s.toLocal8Bit() == "first-local8Bit");
    s = "second-local8Bit";
    QVERIFY(s.toLocal8Bit() == "second-local8Bit");
#ifdef QT3_SUPPORT
    cache = s.local8Bit();
    QVERIFY(cache == s.local8Bit());
#endif
    s.clear();
#ifdef QT3_SUPPORT
    QVERIFY(*s.local8Bit() == '\0');
#endif
    QVERIFY(s.isNull());
    QVERIFY(s.toLocal8Bit().size() == 0);
    QVERIFY(s.toLocal8Bit().isEmpty());
    QVERIFY(!s.toLocal8Bit().isNull());

    s = "first-ascii";
    QVERIFY(s.toAscii() == "first-ascii");
    s = "second-ascii";
    QVERIFY(s.toAscii() == "second-ascii");
#ifdef QT3_SUPPORT
    cache = s.ascii();
    QVERIFY(cache == s.ascii());
#endif
    s.clear();
#ifdef QT3_SUPPORT
    QVERIFY(*s.ascii() == '\0');
#endif
    QVERIFY(s.isNull());
    QVERIFY(s.toAscii().size() == 0);
    QVERIFY(s.toAscii().isEmpty());
    QVERIFY(s.toAscii().isNull());

    s = "ascii";
    s += (uchar) 0xb0;
    QVERIFY(s.toUtf8() != s.toLatin1());
    QString sa = s.toLatin1().constData();
    QVERIFY(sa[sa.length()-1] == (ushort) 0xb0);
    QVERIFY(sa.left(sa.length()-1) == "ascii");

    QVERIFY(s == QString::fromUtf8(s.toUtf8().constData()));

    s = "12";
    s.append('3');
    s += '4';
    QVERIFY(s == "1234");

    s = "repend";
    s.prepend('p');
    QVERIFY(s == "prepend");
    s.prepend("abc ");
    QVERIFY(s == "abc prepend");

    s = "   whitespace        ";
    QVERIFY(s.trimmed() == "whitespace");
    s = "    lots      of  stuff       ";
    QVERIFY(s.simplified() == "lots of stuff");

    s = "a hat, a stick, a ski";
    QVERIFY(s[2] == 'h');
    QVERIFY(s[1] < 'b');


    s = "12223";
    s.remove(1, 2);
    QVERIFY(s == "123");

    s = "(%1)(%2)";
    s = s.arg("foo").arg(7);
    QVERIFY(s == "(foo)(7)");


#if 0
    s = "stl rocks";
    std::string stl_string = s;
    s = stl_string;
    QVERIFY(s == "stl rocks");
#endif

    {
	QString str("Bananas");
	QVERIFY(str.startsWith("Ban"));
	QVERIFY(false == str.startsWith("Car"));
    }
    {
	QString str("Bananas");
	QVERIFY(str.endsWith("anas"));
	QVERIFY(false == str.endsWith("pple"));
    }


    QString str = "Hello";
    QString cstr = QString::fromRawData(str.unicode(), str.length());
    QVERIFY(str == "Hello");
    QVERIFY(cstr == "Hello");
    cstr.clear();
    QVERIFY(str == "Hello");
    QVERIFY(cstr.isEmpty());

    {
        QString str1("Foo");
        str1.prepend(str1);
        QCOMPARE(str1, QString("FooFoo"));
        str1.append(str1);
        QCOMPARE(str1, QString("FooFooFooFoo"));
        str1.insert(2, str1);
        QCOMPARE(str1, QString("FoFooFooFooFoooFooFooFoo"));
        str1.replace(3, 3, str1);
        QCOMPARE(str1, QString("FoFFoFooFooFooFoooFooFooFooooFooFoooFooFooFoo"));
        str1 = "FooFoo";
        str1.replace(char('F'), str1);
        QCOMPARE(str1, QString("FooFooooFooFoooo"));

        str1 = "Foo";
        str1.replace("Foo", str1);
        QCOMPARE(str1, QString("Foo"));
        str1.replace(str1, str1);
        QCOMPARE(str1, QString("Foo"));

        str1 = "Foo";
        str1.replace("Foo", str1, Qt::CaseInsensitive);
        QCOMPARE(str1, QString("Foo"));
        str1.replace(str1, str1);
        QCOMPARE(str1, QString("Foo"));

        str1 = "FooFoo";
        str1.reserve(100);
        str1.replace("oo", str1);
        QCOMPARE(str1, QString("FFooFooFFooFoo"));

        str1 = "Bar";
        str1.replace("FooFoo", str1);
        QCOMPARE(str1, QString("Bar"));

        str1.replace(str1, "xxx");
        QCOMPARE(str1, QString("xxx"));
        str1.replace(str1, QString("yyy"));
        QCOMPARE(str1, QString("yyy"));
        str1 += str1;
        QCOMPARE(str1, QString("yyyyyy"));
    }
}


void tst_Collections::bitArray()
{
    QBitArray ba(20);
    QVERIFY(ba.testBit(17) == false);
    ba.setBit(17);
    QVERIFY(ba.size() == 20);
    QVERIFY(ba.testBit(17)==true);
    QVERIFY(!ba.testBit(16));
    ba[4] = true;
    QVERIFY(ba.testBit(4));
    QVERIFY(ba[4]);
    int sum = 0;
    for(int i = 0; i < 20; i++)
	sum += ba.testBit(i) ? 1 : 0;
    QVERIFY(sum == 2);

    ba = QBitArray(7, true);
    QVERIFY(ba.size() == 7);
    QVERIFY(ba[5]);

    ba = QBitArray(3);
    ba[0] = ba[2] = true;

    QBitArray nba(3);
    nba[1] = true;

    QVERIFY(~ba == nba);

};

struct CacheFoo
{
    CacheFoo(int v):val(v) { counter++; }
    ~CacheFoo() { counter--; }
    int val;
    static int counter;
    bool isDetached() const { return val != 2; }
};

int CacheFoo::counter = 0;

void tst_Collections::cache()
{
    {
	CacheFoo* cf;
	QCache<int, CacheFoo> cache(120);
	int i;
	for (i = 0; i < 30; i++) {
	    cf = cache.object(10);
	    cache.insert(i, new CacheFoo(i), i);
	}

	QVERIFY(cache.contains(10));
	QVERIFY(!cache.contains(1));
	QVERIFY(!cache.contains(2));
	delete cache.take(10);
    }
    {
	QCache<int, QString> cache(120);
	int i;
	QString two;
	for (i = 0; i < 30; i++) {
	    QString s = QString::number(i);
	    cache.insert(i, new QString(s), i);
	    if (i == 2)
		two = s;
	}
	QVERIFY(!cache.contains(3));
	QVERIFY(cache.contains(2));
    }
    {
	QCache<int, int> cache(100);
	cache.insert(2, new int(2));
	*cache[2] = 3;
	QVERIFY(*cache.object(2) == 3);
    }

    QVERIFY(CacheFoo::counter == 0);

}

void tst_Collections::regexp()
{
    QRegExp rx("^\\d\\d?$");
    QVERIFY(rx.indexIn("123") == -1);
    QVERIFY(rx.indexIn("-6") == -1);
    QVERIFY(rx.indexIn("6") == 0) ;
}


void tst_Collections::varLengthArray()
{
    {
	QVarLengthArray<int, 256> sa(128);
	QVERIFY(sa.data() == &sa[0]);
	sa[0] = 0xfee;
	sa[10] = 0xff;
	QVERIFY(sa[0] == 0xfee);
	QVERIFY(sa[10] == 0xff);
	sa.resize(512);
	QVERIFY(sa.data() == &sa[0]);
	QVERIFY(sa[0] == 0xfee);
	QVERIFY(sa[10] == 0xff);
	QVERIFY(sa.size() == 512);
	sa.reserve(1024);
	QVERIFY(sa.capacity() == 1024);
	QVERIFY(sa.size() == 512);
    }
    {
	QVarLengthArray<QString> sa(10);
	sa[0] = "Hello";
	sa[9] = "World";
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
	sa.reserve(512);
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
	sa.resize(512);
	QVERIFY(*sa.data() == "Hello");
	QVERIFY(sa[9] == "World");
    }
    {
        int arr[2] = {1, 2};
        QVarLengthArray<int> sa(10);
        QCOMPARE(sa.size(), 10);
        sa.append(arr, 2);
        QCOMPARE(sa.size(), 12);
        QCOMPARE(sa[10], 1);
        QCOMPARE(sa[11], 2);
    }
    {
        QString arr[2] = { QString("hello"), QString("world") };
        QVarLengthArray<QString> sa(10);
        QCOMPARE(sa.size(), 10);
        sa.append(arr, 2);
        QCOMPARE(sa.size(), 12);
        QCOMPARE(sa[10], QString("hello"));
        QCOMPARE(sa[11], QString("world"));

        sa.append(arr, 1);
        QCOMPARE(sa.size(), 13);
        QCOMPARE(sa[12], QString("hello"));

        sa.append(arr, 0);
        QCOMPARE(sa.size(), 13);
    }
    {
        // assignment operator and copy constructor

        QVarLengthArray<int> sa(10);
        sa[5] = 5;

        QVarLengthArray<int> sa2(10);
        sa2[5] = 6;
        sa2 = sa;
        QCOMPARE(sa2[5], 5);

        QVarLengthArray<int> sa3(sa);
        QCOMPARE(sa3[5], 5);
    }
}

void tst_Collections::pair()
{
    QPair<double, int> p;
    QVERIFY(p.first == 0.0);
    QVERIFY(p.second == 0);

    QPair<int, QString> a(1, "Zebra"), b(2, "Ying"), c(3, "Yang"), d(3, "Ying"), e(5, "Alabama");
    QVERIFY(a.first == 1);
    QVERIFY(a.second == "Zebra");
    QVERIFY(a == qMakePair(1, QString("Zebra")));

    QVERIFY(a == a && b == b && c == c && d == d && e == e);
    QVERIFY(a != b && a != c && a != d && a != e && b != c && b != d && b != e && c != d && c != e
           && d != e);

    QVERIFY(a < b && b < c && c < d && d < e);
    QVERIFY(a <= b && b <= c && c <= d && d <= e);

    QVERIFY(e > d && d > c && c > b && b > a);
    QVERIFY(e >= d && d >= c && c >= b && b >= a);

    QVERIFY(!(a > b || b > c || c > d || d > e));
    QVERIFY(!(a >= b || b >= c || c >= d || d >= e));

    QVERIFY(!(e < d || d < c || c < b || b < a));
    QVERIFY(!(e <= d || d <= c || c <= b || b <= a));

    QVERIFY(a <= a && b <= b && c <= c && d <= d && e <= e);
    QVERIFY(!(a < a || b < b || c < c || d < d || e < e));

    QVERIFY(a >= a && b >= b && c >= c && d >= d && e >= e);
    QVERIFY(!(a > a || b > b || c > c || d > d || e > e));
}

void tst_Collections::cleanupHandler()
{
#ifdef QT3_SUPPORT
    LargeStatic * f1 = 0;
    LargeStatic * f2 = 0;
    {
	Q3CleanupHandler<LargeStatic> cleanup;
	f1 = new LargeStatic;
	f2 = new LargeStatic;
	cleanup.add(&f1);
	cleanup.add(&f2);
    }
    QVERIFY(f1 == 0 && f2 == 0);
#else
    QSKIP("No Qt3 support", SkipAll);
#endif
}

/*
    These test that Java-style mutable iterators don't trash shared
    copy (the setSharable() mechanism).
*/

template <class Container>
void populate(Container &);

template <>
void populate(QList<int> &container)
{
    container << 1 << 2 << 4 << 8;
}

template <>
void populate(QLinkedList<int> &container)
{
    container << 1 << 2 << 4 << 8;
}

template <>
void populate(QVector<int> &container)
{
    container << 1 << 2 << 4 << 8;
}

template <>
void populate(QMap<int, int> &container)
{
    container.insert(1, 1);
    container.insert(2, 2);
    container.insert(4, 4);
    container.insert(8, 8);
}

template <>
void populate(QHash<int, int> &container)
{
    container.insert(1, 1);
    container.insert(2, 2);
    container.insert(4, 4);
    container.insert(8, 8);
}

template <class Container>
bool isSharable(const Container &container)
{
    Container copy = container;
    return !container.isDetached();
}

template <class Container, class ContainerMutableIterator>
void testContainer()
{
    /*
        Verify that shared_null's 'sharable' flag is set to true.
    */
    {
        Container c1;
        QVERIFY(!c1.isDetached());

        Container c2 = c1;
        QVERIFY(!c1.isDetached());
        QVERIFY(!c2.isDetached());
    }

    /*
        Verify that the 'sharable' flag is true while no mutable
        iterator is active.
    */
    {
        Container c1;
        populate(c1);
        QVERIFY(c1.size() == 4);
        QVERIFY(c1.isDetached());

        Container c2 = c1;
        QVERIFY(c1.size() == 4);
        QVERIFY(c2.size() == 4);
        QVERIFY(!c1.isDetached());
        QVERIFY(!c2.isDetached());
    }

    /*
        Verify that the 'sharable' flag is set to false by the
        mutable iterator.
    */
    {
        Container c1;
        populate(c1);
        QVERIFY(c1.size() == 4);
        QVERIFY(c1.isDetached());

        ContainerMutableIterator i(c1);
        i.next();

        Container c2 = c1;
        QVERIFY(c1.size() == 4);
        QVERIFY(c2.size() == 4);
        QVERIFY(c1.isDetached());
        QVERIFY(c2.isDetached());

        i.remove();
        QVERIFY(c1.size() == 3);
        QVERIFY(c2.size() == 4);
    }

    /*
        Verify that the 'sharable' flag is reset to true by the
        mutable iterator's destructor.
    */
    {
        Container c1;
        populate(c1);
        QVERIFY(c1.size() == 4);
        QVERIFY(c1.isDetached());

        {
            ContainerMutableIterator i(c1);
            i.next();
        }

        Container c2 = c1;
        QVERIFY(c1.size() == 4);
        QVERIFY(c2.size() == 4);
        QVERIFY(!c1.isDetached());
        QVERIFY(!c2.isDetached());
    }

    /*
        Verify that the 'sharable' flag only affects the original
        object, not the copies.
    */
    {
        Container c1;
        populate(c1);
        QVERIFY(c1.size() == 4);
        QVERIFY(c1.isDetached());

        Container c2 = c1;
        QVERIFY(isSharable(c2));

        ContainerMutableIterator i(c1);
        QVERIFY(!isSharable(c1));
        QVERIFY(isSharable(c2));

        Container c3 = c1;
        QVERIFY(!isSharable(c1));
        QVERIFY(isSharable(c2));
        QVERIFY(isSharable(c3));
        QVERIFY(c1.isDetached());
        QVERIFY(c2.isDetached());
        QVERIFY(c3.isDetached());

        Container c4;
        c4 = c1;
        QVERIFY(!isSharable(c1));
        QVERIFY(isSharable(c2));
        QVERIFY(isSharable(c4));
        QVERIFY(c1.isDetached());
        QVERIFY(c2.isDetached());
        QVERIFY(c4.isDetached());

        c3 = c2;
        QVERIFY(!isSharable(c1));
        QVERIFY(isSharable(c2));
        QVERIFY(isSharable(c3));
        QVERIFY(c1.isDetached());
        QVERIFY(!c2.isDetached());
        QVERIFY(!c3.isDetached());
    }
}

#define TEST_SEQUENTIAL_CONTAINER(Container) \
    testContainer<Q##Container<int>, QMutable##Container##Iterator<int> >()

#define TEST_ASSOCIATIVE_CONTAINER(Container) \
    testContainer<Q##Container<int, int>, QMutable##Container##Iterator<int, int> >()

void tst_Collections::sharableQList()
{
    TEST_SEQUENTIAL_CONTAINER(List);
}

void tst_Collections::sharableQLinkedList()
{
    TEST_SEQUENTIAL_CONTAINER(LinkedList);
}

void tst_Collections::sharableQVector()
{
    TEST_SEQUENTIAL_CONTAINER(Vector);
}

void tst_Collections::sharableQMap()
{
    TEST_ASSOCIATIVE_CONTAINER(Map);
}

void tst_Collections::sharableQHash()
{
    TEST_ASSOCIATIVE_CONTAINER(Hash);
}

static int getList_calls = 0;
QList<int> getList()
{
    ++getList_calls;
    QList<int> list;
    list << 1 << 2 << 3 << 4 << 5 << 6;
    return list;
}


void tst_Collections::q_foreach()
{
    QList<int> list;
    list << -2 << -1 << 0 << 1 << 2;

    int sum = 0;
    int j = 0;
    foreach(int i, list) {
        QCOMPARE(i, list.at(j));
        sum += i;
        ++j;
    }
    QCOMPARE(sum, 0);

    // again, but without scope
    foreach(int i, list)
        sum += i;
    QCOMPARE(sum, 0);

    foreach(int i, list) {
        sum += i;
        if (i == 0)
            break;
    }
    QCOMPARE(sum, -3);

    sum = 0;
    foreach(int i, list) {
        if (i < 0)
            continue;
        sum += i;
    }
    QCOMPARE(sum, 3);

    sum = 0;
    getList_calls = 0;
    foreach(int i, getList())
        sum += i;
    QCOMPARE(sum, 21);
    QCOMPARE(getList_calls, 1);
}


void tst_Collections::conversions()
{
#define STUFF "A" << "C" << "B" << "A"

    {
        QList<QString> list1;
        list1 << STUFF;

        QVector<QString> vect1 = list1.toVector();
        QCOMPARE(list1.size(), 4);
        QVERIFY(vect1 == (QVector<QString>() << STUFF));

        QList<QString> list2 = vect1.toList();
        QCOMPARE(list2.size(), 4);
        QVERIFY(list2 == (QList<QString>() << STUFF));

        QSet<QString> set1 = list1.toSet();
        QCOMPARE(set1.size(), 3);
        QVERIFY(set1.contains("A"));
        QVERIFY(set1.contains("B"));
        QVERIFY(set1.contains("C"));
        QVERIFY(!set1.contains("D"));

        QList<QString> list3 = set1.toList();
        QCOMPARE(list3.size(), 3);
        QVERIFY(list3.contains("A"));
        QVERIFY(list3.contains("B"));
        QVERIFY(list3.contains("C"));
        QVERIFY(!list3.contains("D"));

        QVERIFY(QList<int>().toVector().isEmpty());
        QVERIFY(QList<int>().toSet().isEmpty());
        QVERIFY(QVector<int>().toList().isEmpty());
        QVERIFY(QSet<int>().toList().isEmpty());
    }

    {
        QList<QString> list1;
        list1 << STUFF;

        QVector<QString> vect1 = QVector<QString>::fromList(list1);
        QCOMPARE(list1.size(), 4);
        QVERIFY(vect1 == (QVector<QString>() << STUFF));

        QList<QString> list2 = QList<QString>::fromVector(vect1);
        QCOMPARE(list2.size(), 4);
        QVERIFY(list2 == (QList<QString>() << STUFF));

        QSet<QString> set1 = QSet<QString>::fromList(list1);
        QCOMPARE(set1.size(), 3);
        QVERIFY(set1.contains("A"));
        QVERIFY(set1.contains("B"));
        QVERIFY(set1.contains("C"));
        QVERIFY(!set1.contains("D"));

        QList<QString> list3 = QList<QString>::fromSet(set1);
        QCOMPARE(list3.size(), 3);
        QVERIFY(list3.contains("A"));
        QVERIFY(list3.contains("B"));
        QVERIFY(list3.contains("C"));
        QVERIFY(!list3.contains("D"));

        QVERIFY(QVector<int>::fromList(QList<int>()).isEmpty());
        QVERIFY(QSet<int>::fromList(QList<int>()).isEmpty());
        QVERIFY(QList<int>::fromVector(QVector<int>()).isEmpty());
        QVERIFY(QList<int>::fromSet(QSet<int>()).isEmpty());
    }
#undef STUFF
}

void tst_Collections::javaStyleIterators()
{
    QStringList list;
    list << "a" << "b" << "c";
    QMutableStringListIterator i(list);
    while (i.hasNext()) {
        i.next();
        i.setValue("");
    }
    while (i.hasPrevious()) {
        i.previous();
        QVERIFY(i.value().isEmpty());
#if QT_VERSION >= 0x040100
        i.value() = "x";
        QCOMPARE(i.value(), QString("x"));
#endif
    }
}

template <class Container>
void testLinkedListLikeStlIterators()
{
    Container fake;
    typename Container::value_type t;
    fake << t;

    typename Container::iterator i1 = fake.begin(), i2 = i1 + 1;
    typename Container::const_iterator c1 = i1, c2 = c1 + 1;

    QVERIFY(i1 == i1);
    QVERIFY(i1 == c1);
    QVERIFY(c1 == i1);
    QVERIFY(c1 == c1);
    QVERIFY(i2 == i2);
    QVERIFY(i2 == c2);
    QVERIFY(c2 == i2);
    QVERIFY(c2 == c2);

    QVERIFY(i1 != i2);
    QVERIFY(i1 != c2);
    QVERIFY(c1 != i2);
    QVERIFY(c1 != c2);
    QVERIFY(i2 != i1);
    QVERIFY(i2 != c1);
    QVERIFY(c2 != i1);
    QVERIFY(c2 != c1);
}

template <class Container>
void testListLikeStlIterators()
{
    testLinkedListLikeStlIterators<Container>();

    Container fake;
    typename Container::value_type t;
    fake << t;

    typename Container::iterator i1 = fake.begin(), i2 = i1 + 1;
    typename Container::const_iterator c1 = i1, c2 = c1 + 1;

    QVERIFY(i1 < i2);
    QVERIFY(i1 < c2);
    QVERIFY(c1 < i2);
    QVERIFY(c1 < c2);
    QVERIFY(!(i2 < i1));
    QVERIFY(!(i2 < c1));
    QVERIFY(!(c2 < i1));
    QVERIFY(!(c2 < c1));
    QVERIFY(!(i1 < i1));
    QVERIFY(!(i1 < c1));
    QVERIFY(!(c1 < i1));
    QVERIFY(!(c1 < c1));
    QVERIFY(!(i2 < i2));
    QVERIFY(!(i2 < c2));
    QVERIFY(!(c2 < i2));
    QVERIFY(!(c2 < c2));

    QVERIFY(i2 > i1);
    QVERIFY(i2 > c1);
    QVERIFY(c2 > i1);
    QVERIFY(c2 > c1);
    QVERIFY(!(i1 > i2));
    QVERIFY(!(i1 > c2));
    QVERIFY(!(c1 > i2));
    QVERIFY(!(c1 > c2));
    QVERIFY(!(i1 > i1));
    QVERIFY(!(i1 > c1));
    QVERIFY(!(c1 > i1));
    QVERIFY(!(c1 > c1));
    QVERIFY(!(i2 > i2));
    QVERIFY(!(i2 > c2));
    QVERIFY(!(c2 > i2));
    QVERIFY(!(c2 > c2));

    QVERIFY(!(i1 >= i2));
    QVERIFY(!(i1 >= c2));
    QVERIFY(!(c1 >= i2));
    QVERIFY(!(c1 >= c2));
    QVERIFY(i2 >= i1);
    QVERIFY(i2 >= c1);
    QVERIFY(c2 >= i1);
    QVERIFY(c2 >= c1);
    QVERIFY(i1 >= i1);
    QVERIFY(i1 >= c1);
    QVERIFY(c1 >= i1);
    QVERIFY(c1 >= c1);
    QVERIFY(i2 >= i2);
    QVERIFY(i2 >= c2);
    QVERIFY(c2 >= i2);
    QVERIFY(c2 >= c2);

    QVERIFY(!(i2 <= i1));
    QVERIFY(!(i2 <= c1));
    QVERIFY(!(c2 <= i1));
    QVERIFY(!(c2 <= c1));
    QVERIFY(i1 <= i2);
    QVERIFY(i1 <= c2);
    QVERIFY(c1 <= i2);
    QVERIFY(c1 <= c2);
    QVERIFY(i1 <= i1);
    QVERIFY(i1 <= c1);
    QVERIFY(c1 <= i1);
    QVERIFY(c1 <= c1);
    QVERIFY(i2 <= i2);
    QVERIFY(i2 <= c2);
    QVERIFY(c2 <= i2);
    QVERIFY(c2 <= c2);
}

template <class Container>
void testMapLikeStlIterators()
{
    Container fake;
    QString k;
    QString t;
    fake.insert(k, t);

    typename Container::iterator i1 = fake.begin(), i2 = i1 + 1;
    typename Container::const_iterator c1 = i1, c2 = c1 + 1;

    QVERIFY(i1 == i1);
    QVERIFY(i1 == c1);
    QVERIFY(c1 == i1);
    QVERIFY(c1 == c1);
    QVERIFY(i2 == i2);
    QVERIFY(i2 == c2);
    QVERIFY(c2 == i2);
    QVERIFY(c2 == c2);

    QVERIFY(i1 != i2);
    QVERIFY(i1 != c2);
    QVERIFY(c1 != i2);
    QVERIFY(c1 != c2);
    QVERIFY(i2 != i1);
    QVERIFY(i2 != c1);
    QVERIFY(c2 != i1);
    QVERIFY(c2 != c1);
}

void tst_Collections::constAndNonConstStlIterators()
{
    testListLikeStlIterators<QList<int> >();
    testListLikeStlIterators<QStringList >();
    testLinkedListLikeStlIterators<QLinkedList<int> >();
    testListLikeStlIterators<QVector<int> >();
    testMapLikeStlIterators<QMap<QString, QString> >();
    testMapLikeStlIterators<QMultiMap<QString, QString> >();
    testMapLikeStlIterators<QHash<QString, QString> >();
    testMapLikeStlIterators<QMultiHash<QString, QString> >();
}

#ifndef QT_NO_STL
void tst_Collections::vector_stl_data()
{
    QTest::addColumn<QStringList>("elements");

    QTest::newRow("empty") << QStringList();
    QTest::newRow("one") << (QStringList() << "Hei");
    QTest::newRow("two") << (QStringList() << "Hei" << "Hopp");
    QTest::newRow("three") << (QStringList() << "Hei" << "Hopp" << "Sann");
}

void tst_Collections::vector_stl()
{
    QFETCH(QStringList, elements);

    QVector<QString> vector;
    for (int i = 0; i < elements.count(); ++i)
        vector << elements.at(i);

    std::vector<QString> stdVector = vector.toStdVector();

    QCOMPARE(int(stdVector.size()), elements.size());

    std::vector<QString>::const_iterator it = stdVector.begin();
    for (uint j = 0; j < stdVector.size() && it != stdVector.end(); ++j, ++it)
        QCOMPARE(*it, vector[j]);

    QCOMPARE(QVector<QString>::fromStdVector(stdVector), vector);
}

void tst_Collections::linkedlist_stl_data()
{
    list_stl_data();
}

void tst_Collections::linkedlist_stl()
{
    QFETCH(QStringList, elements);

    QLinkedList<QString> list;
    for (int i = 0; i < elements.count(); ++i)
        list << elements.at(i);

    std::list<QString> stdList = list.toStdList();

    QCOMPARE(int(stdList.size()), elements.size());

    std::list<QString>::const_iterator it = stdList.begin();
    QLinkedList<QString>::const_iterator it2 = list.constBegin();
    for (uint j = 0; j < stdList.size(); ++j, ++it, ++it2)
        QCOMPARE(*it, *it2);

    QCOMPARE(QLinkedList<QString>::fromStdList(stdList), list);
}

void tst_Collections::list_stl_data()
{
    QTest::addColumn<QStringList>("elements");

    QTest::newRow("empty") << QStringList();
    QTest::newRow("one") << (QStringList() << "Hei");
    QTest::newRow("two") << (QStringList() << "Hei" << "Hopp");
    QTest::newRow("three") << (QStringList() << "Hei" << "Hopp" << "Sann");
}

void tst_Collections::list_stl()
{
    QFETCH(QStringList, elements);

    QList<QString> list;
    for (int i = 0; i < elements.count(); ++i)
        list << elements.at(i);

    std::list<QString> stdList = list.toStdList();

    QCOMPARE(int(stdList.size()), elements.size());

    std::list<QString>::const_iterator it = stdList.begin();
    for (uint j = 0; j < stdList.size() && it != stdList.end(); ++j, ++it)
        QCOMPARE(*it, list[j]);

    QCOMPARE(QList<QString>::fromStdList(stdList), list);
}
#endif

template <typename T>
T qtInit(T * = 0)
{
    return T();
}

void tst_Collections::q_init()
{
    QCOMPARE(qtInit<int>(), 0);
    QCOMPARE(qtInit<double>(), 0.0);
    QCOMPARE(qtInit<QString>(), QString());
    QCOMPARE(qtInit<int *>(), static_cast<int *>(0));
    QCOMPARE(qtInit<double *>(), static_cast<double *>(0));
    QCOMPARE(qtInit<QString *>(), static_cast<QString *>(0));
    QCOMPARE(qtInit<Pod>().i1, 0);
    QCOMPARE(qtInit<Pod>().i2, 0);
}

void tst_Collections::pointersize()
{
    QCOMPARE(int(sizeof(void *)), QT_POINTER_SIZE);
}

class LessThanComparable
{
public:
    bool operator<(const LessThanComparable &) const { return true; }
};

class EqualsComparable
{
public:
    bool operator==(const EqualsComparable &) const { return true; }
};

uint qHash(const EqualsComparable &)
{
    return 0;
}

/*
    The following functions instatiates every member functions in the
    Qt containers that requires either operator== or operator<.
    They are ordered in a concept inheritance tree:

    Container
        MutableIterationContainer
            Sequence (QLinkedList)
                Random Access (QVector, QList, QQueue, QStack)
            Pair Associative (QHash, QMap)
        Associative (QSet)
*/
template <typename ContainerType, typename ValueType>
void instantiateContainer()
{
    const ValueType value = ValueType();
    ContainerType container;
    const ContainerType constContainer(container);
 
#ifndef QT_NO_STL
    typename ContainerType::const_iterator constIt;
    constIt = constContainer.begin();
    container.constBegin();
    
    constIt = constContainer.end();
    container.constEnd();
#endif
    container.clear();
    container.contains(value);
    container.count();
    container.empty();
    container.isEmpty();
    container.size();

    container != constContainer;
    container == constContainer;
    container = constContainer;
}

template <typename ContainerType, typename ValueType>
void instantiateMutableIterationContainer()
{
    instantiateContainer<ContainerType, ValueType>();
    ContainerType container;
 
#ifndef QT_NO_STL
    typename ContainerType::iterator it;
    it = container.begin();
    it = container.end();
#endif
    
    // QSet lacks count(T).
    const ValueType value = ValueType();
    container.count(value);
}

template <typename ContainerType, typename ValueType>
void instantiateSequence()
{
    instantiateMutableIterationContainer<ContainerType, ValueType>();

// QVector lacks removeAll(T)
//    ValueType value = ValueType();
//    ContainerType container;
//    container.removeAll(value);
}

template <typename ContainerType, typename ValueType>
void instantiateRandomAccess()
{
    instantiateSequence<ContainerType, ValueType>();
    
    ValueType value = ValueType();
    ContainerType container;
    container.indexOf(value);
    container.lastIndexOf(value);
}

template <typename ContainerType, typename ValueType>
void instantiateAssociative()
{
    instantiateContainer<ContainerType, ValueType>();

    const ValueType value = ValueType();
    ContainerType container;
    const ContainerType constContainer(container);
   
    container.reserve(1);
    container.capacity();
    container.squeeze();

    container.remove(value);
    container.values();
    
    container.unite(constContainer);
    container.intersect(constContainer);
    container.subtract(constContainer);
    
    container != constContainer;
    container == constContainer;
    container & constContainer;
    container &= constContainer;
    container &= value;
    container + constContainer;
    container += constContainer;
    container += value;
    container - constContainer;
    container -= constContainer;
    container -= value;
    container | constContainer;
    container |= constContainer;
    container |= value;
}

template <typename ContainerType, typename KeyType, typename ValueType>
void instantiatePairAssociative()
{
    instantiateMutableIterationContainer<ContainerType, KeyType>();
    
    typename ContainerType::iterator it;
    typename ContainerType::const_iterator constIt;
    const KeyType key = KeyType();
    const ValueType value = ValueType();
    ContainerType container;
    const ContainerType constContainer(container);
    
    it = container.insert(key, value);
    container.erase(it);
    container.find(key);
    container.constFind(key);
    constContainer.find(key);
    
    container.key(value);
    container.keys();
    constContainer.keys();
    container.remove(key);
    container.take(key);
    container.unite(constContainer);
    container.value(key);
    container.value(key, value);
    container.values();
    container.values(key);
    container[key];
    const int foo = constContainer[key];
    Q_UNUSED(foo);
}

/*
    Instantiate all Qt containers using a datatype that
    defines the minimum amount of operators.
*/
void tst_Collections::containerInstantiation()
{
    // Instantiate QHash member functions.
    typedef QHash<EqualsComparable, int> Hash;
    instantiatePairAssociative<Hash, EqualsComparable, int>();
    
    Hash hash;
    hash.reserve(1);
    hash.capacity();
    hash.squeeze();
    
    // Instantiate QMap member functions.
    typedef QMap<LessThanComparable, int> Map;
    instantiatePairAssociative<Map, LessThanComparable, int>();
    
    // Instantiate QSet member functions.
    typedef QSet<EqualsComparable> Set;
    instantiateAssociative<Set, EqualsComparable>();
    
    //Instantiate QLinkedList member functions.
    typedef QLinkedList<EqualsComparable> LinkedList;
    instantiateSequence<LinkedList, EqualsComparable> ();
    {
        EqualsComparable value;
        LinkedList list;
        list.removeAll(value); 
    }
    
    //Instantiate QList member functions.
    typedef QList<EqualsComparable> List;
    instantiateRandomAccess<List, EqualsComparable>();
    {
        EqualsComparable value;
        List list;
        list.removeAll(value); 
    }
    
    //Instantiate QVector member functions.
    typedef QVector<EqualsComparable> Vector;
    instantiateRandomAccess<Vector, EqualsComparable>();
    
    //Instantiate QQueue member functions.
    typedef QQueue<EqualsComparable> Queue;
    instantiateRandomAccess<Queue, EqualsComparable>();
    
    //Instantiate QStack member functions.
    typedef QStack<EqualsComparable> Stack;
    instantiateRandomAccess<Stack, EqualsComparable>();
}

#if QT_VERSION >= 0x040101
void tst_Collections::qtimerList()
{
    QList<double> foo;
    const int N = 10000;

    foo.append(99.9);
    foo.append(99.9);
    foo.append(99.9);

    for(int i = 0; i < N; i++) {
        foo.removeFirst();
        foo.insert(1, 99.9);
    }

    QList<double>::Iterator end = foo.end();
    for (int i = 0; i < (N / 2) - 10; ++i) {
        foo.prepend(99.9);
        if (foo.end() != end)
            return;
    }
    QFAIL("QList preallocates too much memory");
}
#endif

template <typename Container>
void testContainerTypedefs(Container container)
{
    Q_UNUSED(container)
    { typedef typename Container::value_type Foo; }
    { typedef typename Container::iterator Foo; }
    { typedef typename Container::const_iterator Foo; }
  	{ typedef typename Container::reference Foo; }
	{ typedef typename Container::const_reference Foo; }
	{ typedef typename Container::pointer Foo; }
	{ typedef typename Container::difference_type Foo; }
	{ typedef typename Container::size_type Foo; }
}

template <typename Container>
void testPairAssociativeContainerTypedefs(Container container)
{
    Q_UNUSED(container)

//  TODO: Not sure how to define value_type for our associative containers
//    { typedef typename Container::value_type Foo; }
//    { typedef typename Container::const_iterator Foo; }
//    { typedef typename Container::reference Foo; }
//	  { typedef typename Container::const_reference Foo; }
//	  { typedef typename Container::pointer Foo; }

	{ typedef typename Container::difference_type Foo; }
	{ typedef typename Container::size_type Foo; }
    { typedef typename Container::iterator Foo; }
    { typedef typename Container::key_type Foo; }
    { typedef typename Container::mapped_type Foo; }
// TODO
//    { typedef typename Container::key_compare Foo; }
//    { typedef typename Container::value_comare Foo; }
}

template <typename Container>
void testSetContainerTypedefs(Container container)
{
    Q_UNUSED(container)
    { typedef typename Container::iterator Foo; }
    { typedef typename Container::const_iterator Foo; }
  	{ typedef typename Container::reference Foo; }
	{ typedef typename Container::const_reference Foo; }
	{ typedef typename Container::pointer Foo; }
	{ typedef typename Container::difference_type Foo; }
	{ typedef typename Container::size_type Foo; }
    { typedef typename Container::key_type Foo; }
}

/*
    Compile-time test that verifies that the Qt containers 
    have STL-compatable typedefs.
*/
void tst_Collections::containerTypedefs()
{
    testContainerTypedefs(QVector<int>());
    testContainerTypedefs(QStack<int>());
    testContainerTypedefs(QList<int>());
    testContainerTypedefs(QLinkedList<int>());
    testContainerTypedefs(QQueue<int>());
   
    testPairAssociativeContainerTypedefs(QMap<int, int>());
    testPairAssociativeContainerTypedefs(QMultiMap<int, int>());
    testPairAssociativeContainerTypedefs(QHash<int, int>());

    testSetContainerTypedefs(QSet<int>());
}

class Key1;
class T1;
class T2;

void tst_Collections::forwardDeclared()
{
    { typedef QHash<Key1, T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QMultiHash<Key1, T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QMap<Key1, T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QMultiMap<Key1, T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QPair<T1, T2> C; C *x = 0; Q_UNUSED(x) }
    { typedef QList<T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QLinkedList<T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QVector<T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) Q_UNUSED(i) Q_UNUSED(j) }
    { typedef QStack<T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) Q_UNUSED(i) Q_UNUSED(j) }
    { typedef QQueue<T1> C; C *x = 0; C::iterator i; C::const_iterator j; Q_UNUSED(x) }
    { typedef QSet<T1> C; C *x = 0; /* C::iterator i; */ C::const_iterator j; Q_UNUSED(x) }
}

QTEST_APPLESS_MAIN(tst_Collections)
#include "tst_collections.moc"
