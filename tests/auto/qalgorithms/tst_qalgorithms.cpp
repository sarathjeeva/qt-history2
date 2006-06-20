/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <qalgorithms.h>
#include <q3tl.h>
#include <QStringList>
#include <QString>
#include <QVector>

#define Q_TEST_PERFORMANCE 0

using namespace std;

//TESTED_FILES=

class tst_QAlgorithms : public QObject
{
Q_OBJECT

public:
    tst_QAlgorithms();
    ~tst_QAlgorithms();

public slots:
    void init();
    void cleanup();

private slots:
    void qBubbleSort();
    void qHeapSort();
    void test_qLowerBound_data();
    void test_qLowerBound();
    void test_qUpperBound_data();
    void test_qUpperBound();
    void test_qBinaryFind_data();
    void test_qBinaryFind();
    void qBinaryFindOneEntry();
    void swap();
    void sortEmptyList();
    void sortedList();
    void sortAPItest();
    void stableSortTest();
    void stableSortCorrectnessTest_data();
    void stableSortCorrectnessTest();
    void convenienceAPI();

#if Q_TEST_PERFORMANCE
private:
    void performance();
#endif
};

tst_QAlgorithms::tst_QAlgorithms()

{
}

tst_QAlgorithms::~tst_QAlgorithms()
{

}

void tst_QAlgorithms::init()
{
}

void tst_QAlgorithms::cleanup()
{
}


class TestInt
{
public:
    TestInt(int number)  :m_number(number) {} ;
    TestInt() : m_number(0) {};
    bool operator<(const TestInt &other) const { ++TestInt::lessThanRefCount; return (m_number < other.m_number); }
    int m_number;
static long int lessThanRefCount;
};

long int TestInt::lessThanRefCount;


QStringList dataSetTypes = QStringList() << "Random" << "Ascending"
                << "Descending" << "Equal" << "Duplicates" << "Almost Sorted"  ;

template <typename DataType>
QVector<DataType> generateData(QString dataSetType, const int length)
{
    QVector<DataType> container;
    if (dataSetType == "Random") {
        for(int i=0; i < length; ++i)
            container.append(rand());
    }
    else if (dataSetType == "Ascending") {
        for (int i=0; i < length; ++i)
            container.append(i);
    }
    else if (dataSetType == "Descending") {
        for (int i=0; i < length; ++i)
            container.append(length - i);
    }
    else if (dataSetType == "Equal") {
        for (int i=0; i < length; ++i)
            container.append(43);
    }
    else if (dataSetType == "Duplicates") {
        for (int i=0; i < length; ++i)
            container.append(i % 10);
    }
    else if (dataSetType == "Almost Sorted") {
        for (int i=0; i < length; ++i)
            container.append(i);
        for(int i = 0; i<= length / 10; ++i) {
            const int iswap = i * 9;
            DataType tmp = container.at(iswap);
            container[iswap] = container.at(iswap + 1);
            container[iswap + 1] = tmp;
        }
    }
    return container;
}

struct ResultSet
{
    int numSorts;
    long int lessThanRefCount;
};


template <typename ContainerType, typename Algorithm>
ResultSet testRun(ContainerType &container, Algorithm &algorithm, int millisecs)
{
    TestInt::lessThanRefCount = 0;
    int count = 0;
    QTime t;
    t.start();
    while(t.elapsed() < millisecs) {
        ++count;
        algorithm(container);
    }
    ResultSet result;
    result.numSorts = count;
    result.lessThanRefCount = TestInt::lessThanRefCount;
    return result;
}

template <typename ContainerType, typename LessThan>
bool isSorted(ContainerType &container, LessThan lessThan)
{
    for (int i=0; i < container.count() - 1; ++i)
        if (lessThan(container.at(i+1), container.at(i))) {
            return false;
        }
    return true;
}

template <typename ContainerType>
bool isSorted(ContainerType &container)
{
    return isSorted(container, qLess<typename ContainerType::value_type>());
}


#if Q_TEST_PERFORMANCE
void printHeader(QStringList &headers)
{
    cout << setw(10) << setiosflags(ios_base::left) << " ";
    for (int h = 0; h < headers.count(); ++h) {
        cout << setw(20) << setiosflags(ios_base::left) << headers.at(h).toLatin1().constData();
    }
    cout << endl;
}

template <typename ContainerType>
void print(ContainerType testContainer)
{
    typedef typename ContainerType::value_type T;

    foreach(T value, testContainer) {
        cout << value << " ";
    }

    cout << endl;
}

template <typename Algorithm, typename DataType>
QList<ResultSet> testAlgorithm(Algorithm &algorithm,  QStringList dataSetTypes,  int size, int time)
{
    QList<ResultSet> results;
    foreach(QString dataSetType, dataSetTypes) {
        QVector<DataType> container = generateData<DataType>(dataSetType, size);
        results.append(testRun(container, algorithm, time));
        Q_ASSERT(isSorted(container));
    }
    return results;
}

template <typename Algorithm, typename DataType>
void testAlgorithm(Algorithm algorithm, QStringList &dataSetTypes)
{
    QList<int> sizes = QList<int>() << 5 << 15 << 35 << 70 << 200 << 1000 << 10000;
    printHeader(dataSetTypes);
    for (int s = 0; s < sizes.count(); ++s){
        cout << setw(10) <<  setiosflags(ios_base::left)<< sizes.at(s);
        QList<ResultSet> results =
            testAlgorithm<Algorithm, DataType>(algorithm, dataSetTypes, sizes.at(s), 100);
        foreach(ResultSet result, results) {
            stringstream numSorts;
            numSorts << setiosflags(ios_base::left) << setw(10) << result.numSorts;
            stringstream lessThan;
            lessThan << setiosflags(ios_base::left) << setw(10) << result.lessThanRefCount / result.numSorts;
            cout << numSorts.str() << lessThan.str();
        }
        cout << endl;
    }
}
#endif
static bool userFunction1(char ch1, char ch2)
{
    return (ch1 ^ 1) < (ch2 ^ 1);
}

bool userFunction2(const char &ch1, char ch2)
{
    return (ch1 ^ 1) < (ch2 ^ 1);
}

static inline bool userFunction3(char ch1, const char &ch2)
{
    return (ch1 ^ 1) < (ch2 ^ 1);
}

inline bool userFunction4(const char &ch1, const char &ch2)
{
    return (ch1 ^ 1) < (ch2 ^ 1);
}

class UserFunctor1
{
public:
    UserFunctor1(int x = 1) : y(x) {}

    bool operator()(char ch1, char ch2)
    {
        return (ch1 ^ y) < (ch2 ^ y);
    }

    char y;
};

void tst_QAlgorithms::qHeapSort()
{
    char array1[] = "3141592";
    ::qHeapSort((char *)array1, array1 + strlen(array1));
    QVERIFY(strcmp(array1, "1123459") == 0);

    ::qHeapSort((char *)array1, array1 + strlen(array1), qGreater<char>());
    QVERIFY(strcmp(array1, "9543211") == 0);

    ::qHeapSort((char *)array1, array1 + strlen(array1), qLess<char>());
    QVERIFY(strcmp(array1, "1123459") == 0);
    {
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), userFunction1);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), userFunction2);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), userFunction3);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), userFunction4);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        UserFunctor1 userFunctor1;
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), userFunctor1);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qHeapSort((char *)array2, array2 + strlen(array2), UserFunctor1());
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
        ::qHeapSort((char *)array2, array2 + strlen(array2), UserFunctor1(1));
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
        ::qHeapSort((char *)array2, array2 + strlen(array2), UserFunctor1(3));
        QVERIFY(strcmp(array2, "3210765498CBA@ED") == 0);
        ::qHeapSort((char *)array2, array2 + strlen(array2), UserFunctor1(0));
        QVERIFY(strcmp(array2, "0123456789@ABCDE") == 0);
    }
}

void tst_QAlgorithms::qBubbleSort()
{
    char array1[] = "3141592";
    ::qBubbleSort((char *)array1, array1 + strlen(array1));
    QVERIFY(strcmp(array1, "1123459") == 0);

    ::qBubbleSort((char *)array1, array1 + strlen(array1), qGreater<char>());
    QVERIFY(strcmp(array1, "9543211") == 0);

    ::qBubbleSort((char *)array1, array1 + strlen(array1), qLess<char>());
    QVERIFY(strcmp(array1, "1123459") == 0);

    {
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), userFunction1);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), userFunction2);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), userFunction3);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), userFunction4);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        UserFunctor1 userFunctor1;
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), userFunctor1);
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
    }

    {
        char array2[] = "0123456789@ABCDE";
        ::qBubbleSort((char *)array2, array2 + strlen(array2), UserFunctor1());
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
        ::qBubbleSort((char *)array2, array2 + strlen(array2), UserFunctor1(1));
        QVERIFY(strcmp(array2, "1032547698A@CBED") == 0);
        ::qBubbleSort((char *)array2, array2 + strlen(array2), UserFunctor1(3));
        QVERIFY(strcmp(array2, "3210765498CBA@ED") == 0);
        ::qBubbleSort((char *)array2, array2 + strlen(array2), UserFunctor1(0));
        QVERIFY(strcmp(array2, "0123456789@ABCDE") == 0);
    }
}

void tst_QAlgorithms::swap()
{
    {
        int a = 1, b = 2;
        qSwap(a, b);
        QVERIFY(a == 2);
        QVERIFY(b == 1);

        qSwap(a, a);
        QVERIFY(a == 2);
        QVERIFY(b == 1);

        qSwap(b, b);
        QVERIFY(a == 2);
        QVERIFY(b == 1);

        qSwap(a, b);
        QVERIFY(a == 1);
        QVERIFY(b == 2);

        qSwap(b, a);
        QVERIFY(a == 2);
        QVERIFY(b == 1);
    }

    {
        double a = 1.0, b = 2.0;
        qSwap(a, b);
        QVERIFY(a == 2.0);
        QVERIFY(b == 1.0);

        qSwap(a, a);
        QVERIFY(a == 2.0);
        QVERIFY(b == 1.0);

        qSwap(b, b);
        QVERIFY(a == 2.0);
        QVERIFY(b == 1.0);

        qSwap(a, b);
        QVERIFY(a == 1.0);
        QVERIFY(b == 2.0);

        qSwap(b, a);
        QVERIFY(a == 2.0);
        QVERIFY(b == 1.0);
    }

    {
        QString a = "1", b = "2";
        qSwap(a, b);
        QVERIFY(a == "2");
        QVERIFY(b == "1");

        qSwap(a, a);
        QVERIFY(a == "2");
        QVERIFY(b == "1");

        qSwap(b, b);
        QVERIFY(a == "2");
        QVERIFY(b == "1");

        qSwap(a, b);
        QVERIFY(a == "1");
        QVERIFY(b == "2");

        qSwap(b, a);
        QVERIFY(a == "2");
        QVERIFY(b == "1");
    }

    {
        void *a = 0, *b = 0;
        qSwap(a, b);
    }

    {
        const void *a = 0, *b = 0;
        qSwap(a, b);
    }

    {
        QString *a = 0, *b = 0;
        qSwap(a, b);
    }

    {
        const QString *a = 0, *b = 0;
        qSwap(a, b);
    }

    {
        QString **a = 0, **b = 0;
        qSwap(a, b);
    }

    {
        const QString **a = 0, **b = 0;
        qSwap(a, b);
    }

    {
        QString * const *a = 0, * const *b = 0;
        qSwap(a, b);
    }

    {
        const QString * const *a = 0, * const *b = 0;
        qSwap(a, b);
    }
}

void tst_QAlgorithms::sortEmptyList()
{
    // Only test if it crashes
    QStringList stringList;
    stringList.sort();
    QVERIFY(true);
}

void tst_QAlgorithms::sortedList()
{
    QList<int> list;
    list << 4 << 3 << 6;

    ::qHeapSort(list.begin(), list.end());

    QCOMPARE(list.count(), 3);
    QCOMPARE(list.at(0), 3);
    QCOMPARE(list.at(1), 4);
    QCOMPARE(list.at(2), 6);

    list.insert(qUpperBound(list.begin(), list.end(), 5), 5);
    list.insert(qUpperBound(list.begin(), list.end(), 1), 1);
    list.insert(qUpperBound(list.begin(), list.end(), 8), 8);

    QCOMPARE(list.count(), 6);
    QCOMPARE(list.at(0), 1);
    QCOMPARE(list.at(1), 3);
    QCOMPARE(list.at(2), 4);
    QCOMPARE(list.at(3), 5);
    QCOMPARE(list.at(4), 6);
    QCOMPARE(list.at(5), 8);
}

Q_DECLARE_METATYPE(QList<int>)

void tst_QAlgorithms::test_qLowerBound_data()
{
    QTest::addColumn<QList<int> >("data");
    QTest::addColumn<int>("resultValue");
    QTest::addColumn<int>("resultIndex");
    
    QTest::newRow("sorted-duplicate") << (QList<int>() << 1 << 2 << 2 << 3) << 2 << 1;
}

void tst_QAlgorithms::test_qLowerBound()
{
    QFETCH(QList<int>, data);
    QFETCH(int, resultValue);
    QFETCH(int, resultIndex);


    QCOMPARE(qLowerBound(data.constBegin(), data.constEnd(), resultValue), data.constBegin() + resultIndex);
    QCOMPARE(qLowerBound(data.begin(), data.end(), resultValue), data.begin() + resultIndex);
#if QT_VERSION >= 0x040200
    QCOMPARE(qLowerBound(data, resultValue), data.constBegin() + resultIndex);
    QCOMPARE(qLowerBound(data.constBegin(), data.constEnd(), resultValue, qLess<int>()), data.constBegin() + resultIndex);
#endif
}

void tst_QAlgorithms::test_qUpperBound_data()
{
    QTest::addColumn<QList<int> >("data");
    QTest::addColumn<int>("resultValue");
    QTest::addColumn<int>("resultIndex");
    
    QTest::newRow("sorted-duplicate") << (QList<int>() << 1 << 2 << 2 << 3) << 2 << 3;
}

void tst_QAlgorithms::test_qUpperBound()
{
    QFETCH(QList<int>, data);
    QFETCH(int, resultValue);
    QFETCH(int, resultIndex);

    QCOMPARE(qUpperBound(data.constBegin(), data.constEnd(), resultValue), data.constBegin() + resultIndex);
    QCOMPARE(qUpperBound(data.begin(), data.end(), resultValue), data.begin() + resultIndex);
#if QT_VERSION >= 0x040200
    QCOMPARE(qUpperBound(data, resultValue), data.constBegin() + resultIndex);
    QCOMPARE(qUpperBound(data.constBegin(), data.constEnd(), resultValue, qLess<int>()), data.constBegin() + resultIndex);
#endif
}

void tst_QAlgorithms::test_qBinaryFind_data()
{
    QTest::addColumn<QList<int> >("data");
    QTest::addColumn<int>("resultValue");

    QTest::newRow("sorted-duplicate") << (QList<int>() << 1 << 2 << 2 << 3) << 2;
}

void tst_QAlgorithms::test_qBinaryFind()
{
    QFETCH(QList<int>, data);
    QFETCH(int, resultValue);

    QCOMPARE(*qBinaryFind(data.constBegin(), data.constEnd(), resultValue), resultValue);
    QCOMPARE(*qBinaryFind(data.begin(), data.end(), resultValue), resultValue);
#if QT_VERSION >= 0x040200
    QCOMPARE(*qBinaryFind(data, resultValue), resultValue);
    QCOMPARE(*qBinaryFind(data.constBegin(), data.constEnd(), resultValue, qLess<int>()), resultValue);
#endif
}

void tst_QAlgorithms::qBinaryFindOneEntry()
{
    QList<int> list;
    list << 2;

    QVERIFY(::qBinaryFind(list.constBegin(), list.constEnd(), 2) != list.constEnd());
}


void tst_QAlgorithms::sortAPItest()
{
    QVector<int> testVector = generateData<int>("Random", 101);
    qSort(testVector);
    QVERIFY(isSorted(testVector));
    qSort(testVector.begin(), testVector.end());
    QVERIFY(isSorted(testVector));
    qSort(testVector.begin(), testVector.end(), qLess<int>());
    QVERIFY(isSorted(testVector));

    testVector = generateData<int>("Random", 71);
    qStableSort(testVector);
    QVERIFY(isSorted(testVector));
    qStableSort(testVector.begin(), testVector.end());
    QVERIFY(isSorted(testVector));
    qStableSort(testVector.begin(), testVector.end(), qLess<int>());
    QVERIFY(isSorted(testVector));

    QList<int> testList = generateData<int>("Random", 101).toList();
    qSort(testList);
    QVERIFY(isSorted(testList));
    qSort(testList.begin(), testList.end());
    QVERIFY(isSorted(testList));
    qSort(testList.begin(), testList.end(), qLess<int>());
    QVERIFY(isSorted(testList));

    testList = generateData<int>("Random", 71).toList();
    qStableSort(testList);
    QVERIFY(isSorted(testList));
    qStableSort(testList.begin(), testList.end());
    QVERIFY(isSorted(testList));
    qStableSort(testList.begin(), testList.end(), qLess<int>());
    QVERIFY(isSorted(testList));
}


class StableSortTest
{
public:
    StableSortTest(){};
    StableSortTest(int Major, int Minor) : Major(Major), Minor(Minor) {}
    bool operator<(const StableSortTest &other) const {return (Major < other.Major); }
    bool testMinor(const  StableSortTest &other) const {return  Minor < other.Minor; }

int Major;
int Minor;
};

ostream &operator<<(ostream &out, const StableSortTest& obj)  { out << obj.Major << "-" << obj.Minor; return out; }

QVector<StableSortTest> createStableTestVector()
{
    QVector<StableSortTest> stableTestVector;
	for (int i=500; i>=0; --i) {
        for (int j=0; j<10; ++j) {
            stableTestVector.append(StableSortTest(i, j));
        }
    }
    return stableTestVector;
}

template <typename ContainerType, typename LessThan>
bool isStableSorted(ContainerType &container, LessThan lessThan)
{
    for (int i=0; i < container.count() - 1; ++i) {
        //not sorted?
        if (lessThan(container.at(i + 1), container.at(i)))
            return false;
        // equal?
        if (lessThan(container.at(i),  container.at(i + 1)))
            continue;
        // minor version?
        if(container.at(i + 1).testMinor(container.at(i)))
            return false;
    }
    return true;
}

void tst_QAlgorithms::stableSortTest()
{
    // Selftests:
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        qSort(stableTestVector.begin(), stableTestVector.end(), qLess<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qLess<StableSortTest>()));
        QVERIFY(!isStableSorted(stableTestVector, qLess<StableSortTest>()));
    }
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        qSort(stableTestVector.begin(), stableTestVector.end(), qGreater<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qGreater<StableSortTest>()));
        QVERIFY(!isStableSorted(stableTestVector, qGreater<StableSortTest>()));
    }
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        qSort(stableTestVector.begin(), stableTestVector.end(), qGreater<StableSortTest>());
        QVERIFY(!isSorted(stableTestVector, qLess<StableSortTest>()));
        QVERIFY(!isStableSorted(stableTestVector, qGreater<StableSortTest>()));
    }

    
    // Stable sort with qLess
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        std::stable_sort(stableTestVector.begin(), stableTestVector.end(), qLess<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qLess<StableSortTest>()));
        QVERIFY(isStableSorted(stableTestVector, qLess<StableSortTest>()));
    }
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        qStableSort(stableTestVector.begin(), stableTestVector.end(), qLess<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qLess<StableSortTest>()));
        QVERIFY(isStableSorted(stableTestVector, qLess<StableSortTest>()));
    }

    // Stable sort with qGreater
    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        std::stable_sort(stableTestVector.begin(), stableTestVector.end(), qGreater<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qGreater<StableSortTest>()));
        QVERIFY(isStableSorted(stableTestVector, qGreater<StableSortTest>()));
    }

    {
        QVector<StableSortTest> stableTestVector = createStableTestVector();
        qStableSort(stableTestVector.begin(), stableTestVector.end(), qGreater<StableSortTest>());
        QVERIFY(isSorted(stableTestVector, qGreater<StableSortTest>()));
        QVERIFY(isStableSorted(stableTestVector, qGreater<StableSortTest>()));
    }
}

Q_DECLARE_METATYPE(QVector<int>)

void tst_QAlgorithms::stableSortCorrectnessTest_data()
{
    const int dataSize = 1000;
    QTest::addColumn<QVector<int> >("unsorted");
    QTest::newRow("From documentation") << (QVector<int>() << 33 << 12 << 68 << 6 << 12);
    QTest::newRow("Equal") << (generateData<int>("Equal", dataSize));
    QTest::newRow("Ascending") << (generateData<int>("Ascending", dataSize));
    QTest::newRow("Descending") << (generateData<int>("Descending", dataSize));
    QTest::newRow("Duplicates") << (generateData<int>("Duplicates", dataSize));
    QTest::newRow("Almost Sorted") << (generateData<int>("Almost Sorted", dataSize));
    QTest::newRow("Random") << (generateData<int>("Random", dataSize));
}

void tst_QAlgorithms::stableSortCorrectnessTest()
{
    QFETCH(QVector<int>, unsorted);

    QVector<int> sorted = unsorted;
    qStableSort(sorted.begin(), sorted.end());
    
    // Verify that sorted contains the same numbers as unsorted.
    foreach(int value, unsorted) {
        QVERIFY(sorted.contains(value));
        int unsortedCount = 0;
        qCount(unsorted.begin(), unsorted.end(), value, unsortedCount);
        int sortedCount = 0;
        qCount(sorted.begin(), sorted.end(), value, sortedCount);
        QCOMPARE(sortedCount, unsortedCount); 
    }

    QVERIFY(isSorted(sorted));
}

void tst_QAlgorithms::convenienceAPI()
{
    // Compile-test for QAlgorithm convenience functions.
#if QT_VERSION >= 0x040200
    QList<int> list, list2;

    qCopy(list.begin(), list.end(), list2.begin());
    qCopyBackward(list.begin(), list.end(), list2.begin());
    qEqual(list.begin(), list.end(), list2.begin());

    qFill(list, 1);
    qFill(list.begin(), list.end(), 1);

    qFind(list, 1);
    qFind(list.begin(), list.end(), 1);

    int count1 = 0 , count2 = 0, count3, count4;
    qCount(list, 1, count1);
    qCount(list.begin(), list.end(), 1, count2);
    count3 = qCount(list, 1);
    count4 = qCount(list.begin(), list.end(), 1);
    QCOMPARE(count1, count2);
    QCOMPARE(count2, count3);
    QCOMPARE(count3, count4);

    qSort(list);
    qSort(list.begin(), list.end());
    qSort(list.begin(), list.end(), qLess<int>());

    qStableSort(list);
    qStableSort(list.begin(), list.end());
    qStableSort(list.begin(), list.end(), qLess<int>());

    qLowerBound(list, 1);;
    qLowerBound(list.begin(), list.end(),  1);
    qLowerBound(list.begin(), list.end(), 1, qLess<int>());

    qUpperBound(list, 1);
    qUpperBound(list.begin(), list.end(),  1);
    qUpperBound(list.begin(), list.end(), 1, qLess<int>());

    qBinaryFind(list, 1);
    qBinaryFind(list.begin(), list.end(),  1);
    qBinaryFind(list.begin(), list.end(), 1, qLess<int>());
    
    QList<int *> pointerList;
    qDeleteAll(pointerList);
    qDeleteAll(pointerList.begin(), pointerList.end());
#endif
}

template <typename DataType>
class HeapSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        ::qHeapSort(list);
    }
};

template <typename DataType>
class BubbleSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        ::qBubbleSort(list);
    }
};

template <typename DataType>
class QuickSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        ::qSort(list);
    }
};

template <typename DataType>
class StableSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        ::qStableSort(list);
    }
};

template <typename DataType>
class StlSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        std::sort(list.begin(), list.end());
    }
};

template <typename DataType>
class StlStableSortHelper
{
public:
    void operator()(QVector<DataType> list)
    {
        std::stable_sort(list.begin(), list.end());
    }
};

#if Q_TEST_PERFORMANCE
void tst_QAlgorithms::performance()
{
    cout << endl << "Quick sort" << endl;
    testAlgorithm<QuickSortHelper<TestInt>, TestInt>(QuickSortHelper<TestInt>(), dataSetTypes);
    cout << endl << "stable sort" << endl;
    testAlgorithm<StableSortHelper<TestInt>, TestInt>(StableSortHelper<TestInt>(), dataSetTypes);
    cout << endl << "std::sort" << endl;
    testAlgorithm<StlSortHelper<TestInt>, TestInt>(StlSortHelper<TestInt>(), dataSetTypes);
    cout << endl << "std::stable_sort" << endl;
    testAlgorithm<StlStableSortHelper<TestInt>, TestInt>(StlStableSortHelper<TestInt>(), dataSetTypes);
    cout << "Heap sort" << endl;
    testAlgorithm<HeapSortHelper<TestInt>, TestInt>(HeapSortHelper<TestInt>(), dataSetTypes);
    cout << endl << "Bubble sort" << endl;
    testAlgorithm<BubbleSortHelper<TestInt>, TestInt>(BubbleSortHelper<TestInt>(), dataSetTypes);
/*
    cout << endl << "Sorting lists of ints" << endl;
    cout << "Heap sort" << endl;
    testAlgorithm<HeapSortHelper<int>, int>(HeapSortHelper<int>(), dataSetTypes);
    cout << endl << "Quick sort" << endl;
    testAlgorithm<QuickSortHelper<int>, int>(QuickSortHelper<int>(), dataSetTypes);
    cout << endl << "std::sort" << endl;
    testAlgorithm<StlSortHelper<int>, int>(StlSortHelper<int>(), dataSetTypes);
    cout << endl << "std::stable_sort" << endl;
    testAlgorithm<StlStableSortHelper<int>, int>(StlStableSortHelper<int>(), dataSetTypes);
    cout << endl << "Bubble sort" << endl;
    testAlgorithm<BubbleSortHelper<int>, int>(BubbleSortHelper<int>(), dataSetTypes);
*/
}
#endif

QTEST_APPLESS_MAIN(tst_QAlgorithms)
#include "tst_qalgorithms.moc"

