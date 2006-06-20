/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QDebug>


#include <qmap.h>

//TESTED_CLASS=
//TESTED_FILES=corelib/tools/qmap.h corelib/tools/qmap.cpp

class tst_QMap : public QObject
{
    Q_OBJECT

public:
    tst_QMap();

public slots:
    void init();
private slots:
    void count();
    void clear();
    void beginEnd();
    void key();

    void operator_eq();

    void empty();
    void contains();
    void find();
    void constFind();
    void lowerUpperBound();
    void mergeCompare();
    void take();

    void iterators();
    void keys_values_uniqueKeys();
};

tst_QMap::tst_QMap()
{
}

typedef QMap<QString, QString> StringMap;

class MyClass
{
public:
    MyClass() {
       ++count;
//     qDebug("creating MyClass count=%d", count);
    }
    MyClass( const QString& c) {
	count++; str = c;
// 	qDebug("creating MyClass '%s' count = %d", str.latin1(), count);
    }
    ~MyClass() {
	count--;
// 	qDebug("deleting MyClass '%s' count = %d", str.latin1(), count);
    }
    MyClass( const MyClass& c ) {
	count++; str = c.str;
// 	qDebug("creating MyClass '%s' count = %d", str.latin1(), count);
    }
    MyClass &operator =(const MyClass &o) {
// 	qDebug("copying MyClass '%s'", o.str.latin1());
	str = o.str; return *this;
    }

    QString str;
    static int count;
};

int MyClass::count = 0;

typedef QMap<QString, MyClass> MyMap;

void tst_QMap::init()
{
    MyClass::count = 0;
}

void tst_QMap::count()
{
    {
	MyMap map;
	MyMap map2( map );
	QCOMPARE( map.count(), 0 );
	QCOMPARE( map2.count(), 0 );
	QCOMPARE( MyClass::count, int(0) );
	// detach
	map2["Hallo"] = MyClass( "Fritz" );
	QCOMPARE( map.count(), 0 );
        QCOMPARE( map2.count(), 1 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 1 );
#endif
    }
    QCOMPARE( MyClass::count, int(0) );

    {
	typedef QMap<QString, MyClass> Map;
	Map map;
	QCOMPARE( map.count(), 0);
	map.insert( "Torben", MyClass("Weis") );
	QCOMPARE( map.count(), 1 );
	map.insert( "Claudia", MyClass("Sorg") );
	QCOMPARE( map.count(), 2 );
	map.insert( "Lars", MyClass("Linzbach") );
	map.insert( "Matthias", MyClass("Ettrich") );
	map.insert( "Sue", MyClass("Paludo") );
	map.insert( "Eirik", MyClass("Eng") );
	map.insert( "Haavard", MyClass("Nord") );
	map.insert( "Arnt", MyClass("Gulbrandsen") );
	map.insert( "Paul", MyClass("Tvete") );
	QCOMPARE( map.count(), 9 );
	map.insert( "Paul", MyClass("Tvete 1") );
	map.insert( "Paul", MyClass("Tvete 2") );
	map.insert( "Paul", MyClass("Tvete 3") );
	map.insert( "Paul", MyClass("Tvete 4") );
	map.insert( "Paul", MyClass("Tvete 5") );
	map.insert( "Paul", MyClass("Tvete 6") );

	QCOMPARE( map.count(), 9 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	Map map2( map );
	QVERIFY( map2.count() == 9 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	map2.insert( "Kay", MyClass("Roemer") );
	QVERIFY( map2.count() == 10 );
	QVERIFY( map.count() == 9 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 19 );
#endif

	map2 = map;
	QVERIFY( map.count() == 9 );
	QVERIFY( map2.count() == 9 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	map2.insert( "Kay", MyClass("Roemer") );
	QVERIFY( map2.count() == 10 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 19 );
#endif

	map2.clear();
	QVERIFY( map.count() == 9 );
	QVERIFY( map2.count() == 0 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	map2 = map;
	QVERIFY( map.count() == 9 );
	QVERIFY( map2.count() == 9 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	map2.clear();
	QVERIFY( map.count() == 9 );
	QVERIFY( map2.count() == 0 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 9 );
#endif

	map.remove( "Lars" );
	QVERIFY( map.count() == 8 );
	QVERIFY( map2.count() == 0 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 8 );
#endif

	map.remove( "Mist" );
	QVERIFY( map.count() == 8 );
	QVERIFY( map2.count() == 0 );
#ifndef Q_CC_SUN
	QCOMPARE( MyClass::count, 8 );
#endif
    }
    QVERIFY( MyClass::count == 0 );

    {
	typedef QMap<QString,MyClass> Map;
	Map map;
	map["Torben"] = MyClass("Weis");
#ifndef Q_CC_SUN
	QVERIFY( MyClass::count == 1 );
#endif
	QVERIFY( map.count() == 1 );

	(void)map["Torben"].str;
	(void)map["Lars"].str;
#ifndef Q_CC_SUN
	QVERIFY( MyClass::count == 2 );
#endif
	QVERIFY( map.count() == 2 );

	const Map& cmap = map;
	(void)cmap["Depp"].str;
#ifndef Q_CC_SUN
	QVERIFY( MyClass::count == 2 );
#endif
	QVERIFY( map.count() == 2 );
	QVERIFY( cmap.count() == 2 );
    }
    QCOMPARE( MyClass::count, 0 );
    {
	for ( int i = 0; i < 100; ++i )
	{
	    QMap<int, MyClass> map;
	    for (int j = 0; j < i; ++j)
		map.insert(j, MyClass(QString::number(j)));
	}
	QCOMPARE( MyClass::count, 0 );
    }
    QCOMPARE( MyClass::count, 0 );
}

void tst_QMap::clear()
{
    {
	MyMap map;
	map.clear();
	QVERIFY( map.isEmpty() );
	map.insert( "key", MyClass( "value" ) );
	map.clear();
	QVERIFY( map.isEmpty() );
	map.insert( "key0", MyClass( "value0" ) );
	map.insert( "key0", MyClass( "value1" ) );
	map.insert( "key1", MyClass( "value2" ) );
	map.clear();
	QVERIFY( map.isEmpty() );
    }
    QCOMPARE( MyClass::count, int(0) );
}

void tst_QMap::beginEnd()
{
    StringMap m0;
    QVERIFY( m0.begin() == m0.end() );
    QVERIFY( m0.begin() == m0.begin() );

    // sample string->string map
    StringMap map;
    QVERIFY( map.constBegin() == map.constEnd() );
    map.insert( "0", "a" );
    map.insert( "1", "b" );

    // make a copy. const function shouldn't detach
    StringMap map2 = map;
    QVERIFY( map.constBegin() == map2.constBegin() );
    QVERIFY( map.constEnd() == map2.constEnd() );

    // test iteration
    QString result;
    for ( StringMap::ConstIterator it = map.constBegin();
	  it != map.constEnd(); ++it )
	result += *it;
    QCOMPARE( result, QString( "ab" ) );

    // maps should still be identical
    QVERIFY( map.constBegin() == map2.constBegin() );
    QVERIFY( map.constEnd() == map2.constEnd() );

    // detach
    map2.insert( "2", "c" );
    QVERIFY( map.constBegin() == map.constBegin() );
    QVERIFY( map.constBegin() != map2.constBegin() );
}

void tst_QMap::key()
{
    {
        QMap<QString, int> map1;
        QCOMPARE(map1.key(1), QString());

        map1.insert("one", 1);
        QCOMPARE(map1.key(1), QString("one"));
        QCOMPARE(map1.key(2), QString());

        map1.insert("two", 2);
        QCOMPARE(map1.key(1), QString("one"));
        QCOMPARE(map1.key(2), QString("two"));
        QCOMPARE(map1.key(3), QString());

        map1.insert("deux", 2);
        QCOMPARE(map1.key(1), QString("one"));
        QCOMPARE(map1.key(2), QString("deux"));
        QCOMPARE(map1.key(3), QString());

        map1.insert("duo", 2);
        QCOMPARE(map1.key(2), QString("deux"));
    }

    {
        QMap<int, QString> map2;
        QCOMPARE(map2.key("one"), 0);

        map2.insert(1, "one");
        QCOMPARE(map2.key("one"), 1);
        QCOMPARE(map2.key("two"), 0);

        map2.insert(2, "two");
        QCOMPARE(map2.key("one"), 1);
        QCOMPARE(map2.key("two"), 2);
        QCOMPARE(map2.key("three"), 0);

        map2.insert(3, "two");
        QCOMPARE(map2.key("one"), 1);
        QCOMPARE(map2.key("two"), 2);
        QCOMPARE(map2.key("three"), 0);

        map2.insert(-1, "two");
        QCOMPARE(map2.key("two"), -1);
    }
}

void tst_QMap::operator_eq()
{
    {
        // compare for equality:
        QMap<int, int> a;
        QMap<int, int> b;

        QVERIFY(a == b);
        QVERIFY(!(a != b));
        
        a.insert(1,1);
        b.insert(1,1);
        QVERIFY(a == b);
        QVERIFY(!(a != b));

        a.insert(0,1);
        b.insert(0,1);
        QVERIFY(a == b);
        QVERIFY(!(a != b));

        // compare for inequality:
        a.insert(42,0);
        QVERIFY(a != b);
        QVERIFY(!(a == b));

        a.insert(65, -1);
        QVERIFY(a != b);
        QVERIFY(!(a == b));

        b.insert(-1, -1);
        QVERIFY(a != b);
        QVERIFY(!(a == b));                
    }

    {
        // a more complex map
        QMap<QString, QString> a;
        QMap<QString, QString> b;

        QVERIFY(a == b);
        QVERIFY(!(a != b));

        a.insert("Hello", "World");
        QVERIFY(a != b);
        QVERIFY(!(a == b));

        b.insert("Hello", "World");
        QVERIFY(a == b);
        QVERIFY(!(a != b));

        a.insert("Goodbye", "cruel world");
        QVERIFY(a != b);
        QVERIFY(!(a == b));

        b.insert("Goodbye", "cruel world");

        // what happens if we insert nulls?
        a.insert(QString(), QString());
        QVERIFY(a != b);
        QVERIFY(!(a == b));

        // empty keys and null keys match:
        b.insert(QString(""), QString());
        QVERIFY(a == b);
        QVERIFY(!(a != b));
    }

    {
        // task 102658

        QMap<QString, int> a;
        QMap<QString, int> b;

        a.insert("otto", 1);
        b.insert("willy", 1);
        QVERIFY(a != b);
        QVERIFY(!(a == b));
    }   
}

void tst_QMap::empty()
{
    QMap<int, QString> map1;

    QVERIFY(map1.isEmpty());

    map1.insert(1, "one");
    QVERIFY(!map1.isEmpty());

    map1.clear();
    QVERIFY(map1.isEmpty());

}

void tst_QMap::contains()
{
    QMap<int, QString> map1;
    int i;

    map1.insert(1, "one");
    QVERIFY(map1.contains(1));

    for(i=2; i < 100; ++i)
        map1.insert(i, "teststring");
    for(i=99; i > 1; --i)
        QVERIFY(map1.contains(i));

    map1.remove(43);
    QVERIFY(!map1.contains(43));
}

void tst_QMap::find()
{
    QMap<int, QString> map1;
    QString testString="Teststring %0";
    QString compareString;
    int i,count=0;

    QVERIFY(map1.find(1) == map1.end());

    map1.insert(1,"Mensch");
    map1.insert(1,"Mayer");
    map1.insert(2,"Hej");

    QVERIFY(map1.find(1).value() == "Mayer");
    QVERIFY(map1.find(2).value() == "Hej");

    for(i = 3; i < 10; ++i) {
        compareString = testString.arg(i);
        map1.insertMulti(4, compareString);
    }

    QMap<int, QString>::const_iterator it=map1.find(4);

    for(i = 9; i > 2 && it != map1.end() && it.key() == 4; --i) {
        compareString = testString.arg(i);
        QVERIFY(it.value() == compareString);
        ++it;
        ++count;
    }
    QCOMPARE(count, 7);
}

void tst_QMap::constFind()
{
    QMap<int, QString> map1;
    QString testString="Teststring %0";
    QString compareString;
    int i,count=0;

    QVERIFY(map1.constFind(1) == map1.constEnd());

    map1.insert(1,"Mensch");
    map1.insert(1,"Mayer");
    map1.insert(2,"Hej");
    
    QVERIFY(map1.constFind(4) == map1.constEnd());
    
    QVERIFY(map1.constFind(1).value() == "Mayer");
    QVERIFY(map1.constFind(2).value() == "Hej");

    for(i = 3; i < 10; ++i) {
        compareString = testString.arg(i);
        map1.insertMulti(4, compareString);
    }

    QMap<int, QString>::const_iterator it=map1.constFind(4);

    for(i = 9; i > 2 && it != map1.constEnd() && it.key() == 4; --i) {
        compareString = testString.arg(i);
        QVERIFY(it.value() == compareString);
        ++it;
        ++count;
    }
    QCOMPARE(count, 7);
}

void tst_QMap::lowerUpperBound()
{
    QMap<int, QString> map1;

    map1.insert(1, "one");
    map1.insert(5, "five");
    map1.insert(10, "ten");


    //Copied from documentation

    QCOMPARE(map1.upperBound(0).key(), 1);      // returns iterator to (1, "one")
    QCOMPARE(map1.upperBound(1).key(), 5);      // returns iterator to (5, "five")
    QCOMPARE(map1.upperBound(2).key(), 5);      // returns iterator to (5, "five")
    QVERIFY(map1.upperBound(10) == map1.end());     // returns end()
    QVERIFY(map1.upperBound(999) == map1.end());    // returns end()

    QCOMPARE(map1.lowerBound(0).key(), 1);      // returns iterator to (1, "one")
    QCOMPARE(map1.lowerBound(1).key(), 1);      // returns iterator to (1, "one")
    QCOMPARE(map1.lowerBound(2).key(), 5);      // returns iterator to (5, "five")
    QCOMPARE(map1.lowerBound(10).key(), 10);     // returns iterator to (10, "ten")
    QVERIFY(map1.lowerBound(999) == map1.end());    // returns end()
}

void tst_QMap::mergeCompare()
{
    QMap<int, QString> map1, map2, map3;

    map1.insert(1,"ett");
    map1.insert(3,"tre");
    map1.insert(5,"fem");

    map2.insert(2,"tvo");
    map2.insert(4,"fyra");

    map1.unite(map2);

    QVERIFY(map1.value(1) == "ett");
    QVERIFY(map1.value(2) == "tvo");
    QVERIFY(map1.value(3) == "tre");
    QVERIFY(map1.value(4) == "fyra");
    QVERIFY(map1.value(5) == "fem");

    map3.insert(1, "ett");
    map3.insert(2, "tvo");
    map3.insert(3, "tre");
    map3.insert(4, "fyra");
    map3.insert(5, "fem");

    QVERIFY(map1 == map3);
}

void tst_QMap::take()
{
    QMap<int, QString> map;

    map.insert(2, "zwei");
    map.insert(3, "drei");

    QVERIFY(map.take(3) == "drei");
    QVERIFY(!map.contains(3));
}

void tst_QMap::iterators()
{
    QMap<int, QString> map;
    QString testString="Teststring %1";
    int i;

    for(i = 1; i < 100; ++i)
        map.insert(i, testString.arg(i));

    //STL-Style iterators

    QMap<int, QString>::iterator stlIt = map.begin();
    QVERIFY(stlIt.value() == "Teststring 1");

    stlIt+=5;
    QVERIFY(stlIt.value() == "Teststring 6");

    stlIt++;
    QVERIFY(stlIt.value() == "Teststring 7");

    stlIt-=3;
    QVERIFY(stlIt.value() == "Teststring 4");

    stlIt--;
    QVERIFY(stlIt.value() == "Teststring 3");

    for(stlIt = map.begin(), i = 1; stlIt != map.end(), i < 100; ++stlIt, ++i)
            QVERIFY(stlIt.value() == testString.arg(i));

    //STL-Style const-iterators

    QMap<int, QString>::const_iterator cstlIt = map.constBegin();
    QVERIFY(cstlIt.value() == "Teststring 1");

    cstlIt+=5;
    QVERIFY(cstlIt.value() == "Teststring 6");

    cstlIt++;
    QVERIFY(cstlIt.value() == "Teststring 7");

    cstlIt-=3;
    QVERIFY(cstlIt.value() == "Teststring 4");

    cstlIt--;
    QVERIFY(cstlIt.value() == "Teststring 3");

    for(cstlIt = map.begin(), i = 1; cstlIt != map.constEnd(), i < 100; ++cstlIt, ++i)
            QVERIFY(cstlIt.value() == testString.arg(i));

    //Java-Style iterators

    QMapIterator<int, QString> javaIt(map);

    i = 0;
    while(javaIt.hasNext()) {
        ++i;
        javaIt.next();
        QVERIFY(javaIt.value() == testString.arg(i));
    }

    ++i;
    while(javaIt.hasPrevious()) {
        --i;
        javaIt.previous();
        QVERIFY(javaIt.value() == testString.arg(i));
    }

    /*
        I've removed findNextKey() and findPreviousKey() from the API
        for Qt 4.0 beta 1.
    */

#if 0
    QVERIFY(javaIt.findNextKey(50));
    QVERIFY(javaIt.value() == "Teststring 50");
#endif

    i = 51;
    while(javaIt.hasPrevious()) {
        --i;
        javaIt.previous();
        QVERIFY(javaIt.value() == testString.arg(i));
    }

#if 0
    QVERIFY(javaIt.findNextKey(50));
    QVERIFY(javaIt.value() == "Teststring 50");

    QVERIFY(javaIt.hasPrevious());
    QVERIFY(javaIt.findPreviousKey(20));
    QCOMPARE(javaIt.value(), QString("Teststring 20"));
#endif
}

void tst_QMap::keys_values_uniqueKeys()
{
    QMap<QString, int> map;
#if QT_VERSION >= 0x040200
    QVERIFY(map.uniqueKeys().isEmpty());
#endif
    QVERIFY(map.keys().isEmpty());
    QVERIFY(map.values().isEmpty());

    map.insertMulti("alpha", 1);
    QVERIFY(map.keys() == (QList<QString>() << "alpha"));
#if QT_VERSION >= 0x040200
    QVERIFY(map.uniqueKeys() == map.keys());
#endif
    QVERIFY(map.values() == (QList<int>() << 1));

    map.insertMulti("beta", -2);
    QVERIFY(map.keys() == (QList<QString>() << "alpha" << "beta"));
#if QT_VERSION >= 0x040200
    QVERIFY(map.keys() == map.uniqueKeys());
#endif
    QVERIFY(map.values() == (QList<int>() << 1 << -2));

    map.insertMulti("alpha", 2);
#if QT_VERSION >= 0x040200
    QVERIFY(map.uniqueKeys() == (QList<QString>() << "alpha" << "beta"));
#endif
    QVERIFY(map.keys() == (QList<QString>() << "alpha" << "alpha" << "beta"));
    QVERIFY(map.values() == (QList<int>() << 2 << 1 << -2));

    map.insertMulti("beta", 4);
#if QT_VERSION >= 0x040200
    QVERIFY(map.uniqueKeys() == (QList<QString>() << "alpha" << "beta"));
#endif
    QVERIFY(map.keys() == (QList<QString>() << "alpha" << "alpha" << "beta" << "beta"));
    QVERIFY(map.values() == (QList<int>() << 2 << 1 << 4 << -2));
}

QTEST_APPLESS_MAIN(tst_QMap)
#include "tst_qmap.moc"
