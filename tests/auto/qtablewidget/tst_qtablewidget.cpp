/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qeventloop.h>
#include <qlist.h>
#include <qpair.h>

#include <qtablewidget.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qtablewidget.h gui/itemviews/qtablewidget.cpp

class QObjectTableItem : public QObject, public QTableWidgetItem
{
    Q_OBJECT
};

class tst_QTableWidget : public QObject
{
    Q_OBJECT

public:
    tst_QTableWidget();
    ~tst_QTableWidget();

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void clear();
    void clearContents();
    void rowCount();
    void columnCount();
    void itemAssignment();
    void item_data();
    void item();
    void takeItem_data();
    void takeItem();
    void selectedItems_data();
    void selectedItems();
    void removeRow_data();
    void removeRow();
    void removeColumn_data();
    void removeColumn();
    void insertRow_data();
    void insertRow();
    void insertColumn_data();
    void insertColumn();
    void itemStreaming_data();
    void itemStreaming();
    void itemOwnership();
    void sortItems_data();
    void sortItems();
    void setItemWithSorting_data();
    void setItemWithSorting();

private:
    QTableWidget *testWidget;
};

typedef QPair<int, int> IntPair;
typedef QList<int> IntList;
typedef QList<IntPair> IntIntList;

Q_DECLARE_METATYPE(IntList)
Q_DECLARE_METATYPE(IntIntList)
Q_DECLARE_METATYPE(QTableWidgetSelectionRange)
Q_DECLARE_METATYPE(QModelIndex)


// Testing get/set functions
void tst_QTableWidget::getSetCheck()
{
    QTableWidget obj1;
    // int QTableWidget::rowCount()
    // void QTableWidget::setRowCount(int)
    obj1.setRowCount(0);
    QCOMPARE(0, obj1.rowCount());
    obj1.setRowCount(INT_MIN);
    QCOMPARE(0, obj1.rowCount()); // Row count can never be negative
//    obj1.setRowCount(INT_MAX);
//    QCOMPARE(INT_MAX, obj1.rowCount());
    obj1.setRowCount(100);
    QCOMPARE(100, obj1.rowCount());
    

    // int QTableWidget::columnCount()
    // void QTableWidget::setColumnCount(int)
    obj1.setColumnCount(0);
    QCOMPARE(0, obj1.columnCount());
    obj1.setColumnCount(INT_MIN);
    QCOMPARE(0, obj1.columnCount()); // Column count can never be negative
    obj1.setColumnCount(1000);
    QCOMPARE(1000, obj1.columnCount());
//    obj1.setColumnCount(INT_MAX);
//    QCOMPARE(INT_MAX, obj1.columnCount());

    // QTableWidgetItem * QTableWidget::currentItem()
    // void QTableWidget::setCurrentItem(QTableWidgetItem *)
    QTableWidgetItem *var3 = new QTableWidgetItem("0,0");
    obj1.setItem(0,0, var3);
    obj1.setItem(1,1, new QTableWidgetItem("1,1"));
    obj1.setItem(2,2, new QTableWidgetItem("2,2"));
    obj1.setItem(3,3, new QTableWidgetItem("3,3"));
    obj1.setCurrentItem(var3);
    QCOMPARE(var3, obj1.currentItem());
    obj1.setCurrentItem((QTableWidgetItem *)0);
    QCOMPARE((QTableWidgetItem *)0, obj1.currentItem());

    // const QTableWidgetItem * QTableWidget::itemPrototype()
    // void QTableWidget::setItemPrototype(const QTableWidgetItem *)
    QTableWidgetItem *var4 = new QTableWidgetItem;
    obj1.setItemPrototype(var4);
    QCOMPARE(var4, obj1.itemPrototype());
    obj1.setItemPrototype((QTableWidgetItem *)0);
    QCOMPARE((QTableWidgetItem *)0, obj1.itemPrototype());
}

tst_QTableWidget::tst_QTableWidget(): testWidget(0)
{
}

tst_QTableWidget::~tst_QTableWidget()
{
}

void tst_QTableWidget::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    testWidget = new QTableWidget();
    testWidget->show();
}

void tst_QTableWidget::cleanupTestCase()
{
    delete testWidget;
}

void tst_QTableWidget::init()
{
    testWidget->clear();
    testWidget->setRowCount(5);
    testWidget->setColumnCount(5);

    for (int row=0; row < testWidget->rowCount(); ++row)
        testWidget->showRow(row);
    for (int column=0; column < testWidget->columnCount(); ++column)
        testWidget->showColumn(column);
}

void tst_QTableWidget::cleanup()
{

}

void tst_QTableWidget::clearContents()
{
    QTableWidgetItem *item = new QTableWidgetItem("test");
    testWidget->setHorizontalHeaderItem(0, item);
    QVERIFY(testWidget->horizontalHeaderItem(0) == item);
    testWidget->clearContents();
    QVERIFY(testWidget->horizontalHeaderItem(0) == item);
}

void tst_QTableWidget::clear()
{
    QTableWidgetItem *item = new QTableWidgetItem("foo");
    testWidget->setItem(0, 0, item);
    testWidget->setItemSelected(item, true);

    QVERIFY(testWidget->item(0, 0) == item);
    QVERIFY(testWidget->isItemSelected(item));


    QPointer<QObjectTableItem> bla = new QObjectTableItem();
    testWidget->setItem(1, 1, bla);

    testWidget->clear();

    QVERIFY(bla.isNull());

    QVERIFY(!testWidget->item(0,0));
    QVERIFY(!testWidget->selectedRanges().count());
    QVERIFY(!testWidget->selectedItems().count());
}

void tst_QTableWidget::rowCount()
{
    int rowCountBefore = 5;
    int rowCountAfter = 10;

    int rowCount = testWidget->rowCount();
    QCOMPARE(rowCount, rowCountBefore);

    testWidget->setRowCount(rowCountAfter);
    rowCount = testWidget->rowCount();
    QCOMPARE(rowCount, rowCountAfter);

    QPersistentModelIndex index(testWidget->model()->index(rowCountAfter - 1, 0,
                                                           testWidget->rootIndex()));
    QCOMPARE(index.row(), rowCountAfter - 1);
    QCOMPARE(index.column(), 0);
    QVERIFY(index.isValid());
    testWidget->setRowCount(rowCountBefore);
    QCOMPARE(index.row(), -1);
    QCOMPARE(index.column(), -1);
    QVERIFY(!index.isValid());

    rowCountBefore = testWidget->rowCount();
    testWidget->setRowCount(-1);
    QCOMPARE(testWidget->rowCount(), rowCountBefore);
}

void tst_QTableWidget::columnCount()
{
    int columnCountBefore = 5;
    int columnCountAfter = 10;

    int columnCount = testWidget->columnCount();
    QCOMPARE(columnCount, columnCountBefore);

    testWidget->setColumnCount(columnCountAfter);
    columnCount = testWidget->columnCount();
    QCOMPARE(columnCount, columnCountAfter);

    QPersistentModelIndex index(testWidget->model()->index(0, columnCountAfter - 1,
                                                           testWidget->rootIndex()));
    QCOMPARE(index.row(), 0);
    QCOMPARE(index.column(), columnCountAfter - 1);
    QVERIFY(index.isValid());
    testWidget->setColumnCount(columnCountBefore);
    QCOMPARE(index.row(), -1);
    QCOMPARE(index.column(), -1);
    QVERIFY(!index.isValid());

    columnCountBefore = testWidget->columnCount();
    testWidget->setColumnCount(-1);
    QCOMPARE(testWidget->columnCount(), columnCountBefore);
}

void tst_QTableWidget::itemAssignment()
{
    QTableWidgetItem itemInWidget("inWidget");
    testWidget->setItem(0, 0, &itemInWidget);
    itemInWidget.setFlags(itemInWidget.flags() | Qt::ItemIsTristate);
    QTableWidgetItem itemOutsideWidget("outsideWidget");

    QVERIFY(itemInWidget.tableWidget());
    QCOMPARE(itemInWidget.text(), QString("inWidget"));
    QVERIFY(itemInWidget.flags() & Qt::ItemIsTristate);

    QVERIFY(!itemOutsideWidget.tableWidget());
    QCOMPARE(itemOutsideWidget.text(), QString("outsideWidget"));
    QVERIFY(!(itemOutsideWidget.flags() & Qt::ItemIsTristate));

    itemOutsideWidget = itemInWidget;
    QVERIFY(!itemOutsideWidget.tableWidget());
    QCOMPARE(itemOutsideWidget.text(), QString("inWidget"));
    QVERIFY(itemOutsideWidget.flags() & Qt::ItemIsTristate);
}

void tst_QTableWidget::item_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<bool>("expectItem");

    QTest::newRow("0x0 take [0,0]") << 0 << 0 << 0 << 0 << false;
    QTest::newRow("0x0 take [4,4]") << 0 << 0 << 4 << 4 << false;
    QTest::newRow("4x4 take [0,0]") << 4 << 4 << 0 << 0 << true;
    QTest::newRow("4x4 take [4,4]") << 4 << 4 << 4 << 4 << false;
    QTest::newRow("4x4 take [2,2]") << 4 << 4 << 2 << 2 << true;
}

void tst_QTableWidget::item()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(bool, expectItem);

    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    for (int r = 0; r < testWidget->rowCount(); ++r)
        for (int c = 0; c < testWidget->columnCount(); ++c)
            testWidget->setItem(r, c, new QTableWidgetItem(QString::number(r * c + c)));

    for (int r = 0; r < testWidget->rowCount(); ++r)
        for (int c = 0; c < testWidget->columnCount(); ++c)
            QCOMPARE(testWidget->item(r, c)->text(), QString::number(r * c + c));

    QTableWidgetItem *item = testWidget->item(row, column);
    QCOMPARE(!!item, expectItem);
    if (expectItem)
        QCOMPARE(item->text(), QString::number(row * column + column));
}

void tst_QTableWidget::takeItem_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<bool>("expectItem");

    QTest::newRow("0x0 take [0,0]") << 0 << 0 << 0 << 0 << false;
    QTest::newRow("0x0 take [4,4]") << 0 << 0 << 4 << 4 << false;
    QTest::newRow("4x4 take [0,0]") << 4 << 4 << 0 << 0 << true;
    QTest::newRow("4x4 take [4,4]") << 4 << 4 << 4 << 4 << false;
    QTest::newRow("4x4 take [2,2]") << 4 << 4 << 2 << 2 << true;
}

void tst_QTableWidget::takeItem()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(bool, expectItem);

    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    for (int r = 0; r < testWidget->rowCount(); ++r)
        for (int c = 0; c < testWidget->columnCount(); ++c)
            testWidget->setItem(r, c, new QTableWidgetItem(QString::number(r * c + c)));

    for (int r = 0; r < testWidget->rowCount(); ++r)
        for (int c = 0; c < testWidget->columnCount(); ++c)
            QCOMPARE(testWidget->item(r, c)->text(), QString::number(r * c + c));

    QTableWidgetItem *item = testWidget->takeItem(row, column);
    QCOMPARE(!!item, expectItem);
    if (expectItem) {
        QCOMPARE(item->text(), QString::number(row * column + column));
        delete item;
    }
    QVERIFY(!testWidget->takeItem(row, column));
}

void tst_QTableWidget::selectedItems_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<IntIntList>("createItems");
    QTest::addColumn<IntList>("hiddenRows");
    QTest::addColumn<IntList>("hiddenColumns");
    QTest::addColumn<QTableWidgetSelectionRange>("selectionRange");
    QTest::addColumn<IntIntList>("expectedItems");

    QTest::newRow("3x3 empty cells, no hidden rows/columns, none selected")
        << 3 << 3
        << IntIntList()
        << IntList()
        << IntList()
        << QTableWidgetSelectionRange()
        << IntIntList();

    QTest::newRow("3x3 empty cells,no hidden rows/columnms, all selected")
        << 3 << 3
        << IntIntList()
        << IntList()
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();

    QTest::newRow("3x3 (1,1) exists, no hidden rows/columnms, all selected")
        << 3 << 3
        << (IntIntList() << IntPair(1,1))
        << IntList()
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << (IntIntList() << IntPair(1,1));

    QTest::newRow("3x3 (1,1) exists, row 1 hidden, all selected")
        << 3 << 3
        << (IntIntList() << IntPair(1,1))
        << (IntList() << 1)
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();

    QTest::newRow("3x3 (1,1) exists, column 1 hidden, all selected")
        << 3 << 3
        << (IntIntList() << IntPair(1,1))
        << IntList()
        << (IntList() << 1)
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();

    QTest::newRow("3x3 all exists, no hidden rows/columns, all selected")
        << 3 << 3
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
            << IntPair(1,0) << IntPair(1,1) << IntPair(1,2)
            << IntPair(2,0) << IntPair(2,1) << IntPair(2,2))
        << IntList()
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
            << IntPair(1,0) << IntPair(1,1) << IntPair(1,2)
            << IntPair(2,0) << IntPair(2,1) << IntPair(2,2));

    QTest::newRow("3x3 all exists, row 1 hidden, all selected")
        << 3 << 3
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
            << IntPair(1,0) << IntPair(1,1) << IntPair(1,2)
            << IntPair(2,0) << IntPair(2,1) << IntPair(2,2))
        << (IntList() << 1)
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
            << IntPair(2,0) << IntPair(2,1) << IntPair(2,2));

    QTest::newRow("3x3 all exists, column 1 hidden, all selected")
        << 3 << 3
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
            << IntPair(1,0) << IntPair(1,1) << IntPair(1,2)
            << IntPair(2,0) << IntPair(2,1) << IntPair(2,2))
        << IntList()
        << (IntList() << 1)
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << (IntIntList()
            << IntPair(0,0) << IntPair(0,2)
            << IntPair(1,0) << IntPair(1,2)
            << IntPair(2,0) << IntPair(2,2));

    QTest::newRow("3x3 none exists, no hidden rows/columns, all selected")
        << 3 << 3
        << IntIntList()
        << IntList()
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();
//         << (IntIntList()
//             << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
//             << IntPair(1,0) << IntPair(1,1) << IntPair(1,2)
//             << IntPair(2,0) << IntPair(2,1) << IntPair(2,2));

    QTest::newRow("3x3 none exists,  row 1 hidden, all selected, filling empty cells")
        << 3 << 3
        << IntIntList()
        << (IntList() << 1)
        << IntList()
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();
//         << (IntIntList()
//             << IntPair(0,0) << IntPair(0,1) << IntPair(0,2)
//             << IntPair(2,0) << IntPair(2,1) << IntPair(2,2));

    QTest::newRow("3x3 none exists,  column 1 hidden, all selected")
        << 3 << 3
        << IntIntList()
        << IntList()
        << (IntList() << 1)
        << QTableWidgetSelectionRange(0, 0, 2, 2)
        << IntIntList();
//         << (IntIntList()
//             << IntPair(0,0) << IntPair(0,2)
//             << IntPair(1,0) << IntPair(1,2)
//             << IntPair(2,0) << IntPair(2,2));
}

void tst_QTableWidget::selectedItems()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(IntIntList, createItems);
    QFETCH(IntList, hiddenRows);
    QFETCH(IntList, hiddenColumns);
    QFETCH(QTableWidgetSelectionRange, selectionRange);
    QFETCH(IntIntList, expectedItems);

    // set dimensions and test they are ok
    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    // create and set items
    foreach (IntPair intPair, createItems) {
        testWidget->setItem(intPair.first, intPair.second,
                            new QTableWidgetItem(QString("Item %1 %2")
                                                 .arg(intPair.first).arg(intPair.second)));
    }
    // hide rows/columns
    foreach (int row, hiddenRows)
        testWidget->setRowHidden(row, true);
    foreach (int column, hiddenColumns)
        testWidget->setColumnHidden(column, true);

    // make sure we don't have any previous selections hanging around
    QVERIFY(!testWidget->selectedRanges().count());
    QVERIFY(!testWidget->selectedItems().count());

    // select range and check that it is set correctly
    testWidget->setRangeSelected(selectionRange, true);
    if (selectionRange.topRow() >= 0) {
        QCOMPARE(testWidget->selectedRanges().count(), 1);
        QCOMPARE(testWidget->selectedRanges().at(0).topRow(), selectionRange.topRow());
        QCOMPARE(testWidget->selectedRanges().at(0).bottomRow(), selectionRange.bottomRow());
        QCOMPARE(testWidget->selectedRanges().at(0).leftColumn(), selectionRange.leftColumn());
        QCOMPARE(testWidget->selectedRanges().at(0).rightColumn(), selectionRange.rightColumn());
    } else {
        QCOMPARE(testWidget->selectedRanges().count(), 0);
    }

    // check that the correct number of items and the expected items are there
    QList<QTableWidgetItem *> selectedItems = testWidget->selectedItems();
    QCOMPARE(selectedItems.count(), expectedItems.count());
    foreach (IntPair intPair, expectedItems)
        QVERIFY(selectedItems.contains(testWidget->item(intPair.first, intPair.second)));

    // check that setItemSelected agrees with selectedItems
    for (int row = 0; row<testWidget->rowCount(); ++row) {
        bool hidden = false;
        foreach (int hiddenRow, hiddenRows){
            if(hiddenRow == row){
                hidden = true;
                break;
            }
        }
        if (hidden)
            continue;
        
        for (int column = 0; column<testWidget->columnCount(); ++column) {
            foreach (int hiddenColumn, hiddenColumns){
                if(hiddenColumn == column){
                    hidden = true;
                    break;
                }
            }
            if (hidden)
                continue;
            
            QTableWidgetItem *item = testWidget->item(row, column);
            if (item && testWidget->isItemSelected(item))
                QVERIFY(selectedItems.contains(item));
        }
    }
}

void tst_QTableWidget::removeRow_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<int>("expectedColumnCount");

    QTest::newRow("Empty") << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("1x1:0") << 1 << 1 << 0 << 0 << 1;
    QTest::newRow("3x3:0") << 3 << 3 << 0 << 2 << 3;
    QTest::newRow("3x3:1") << 3 << 3 << 1 << 2 << 3;
    QTest::newRow("3x3:2") << 3 << 3 << 2 << 2 << 3;
}

void tst_QTableWidget::removeRow()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, expectedRowCount);
    QFETCH(int, expectedColumnCount);

    // set dimensions and test they are ok
    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    // fill table with items
    for (int r = 0; r < rowCount; ++r)
        for (int c = 0; c < columnCount; ++c)
            testWidget->setItem(r, c,
                                new QTableWidgetItem(
                                    QString::number(r) + ":" + QString::number(c)));

    // remove and compare the results
    testWidget->removeRow(row);
    QCOMPARE(testWidget->rowCount(), expectedRowCount);
    QCOMPARE(testWidget->columnCount(), expectedColumnCount);

    // check if the correct items were removed
    for (int r = 0; r < expectedRowCount; ++r)
        for (int c = 0; c < expectedColumnCount; ++c)
            if (r < row)
                QCOMPARE(testWidget->item(r, c)->text(),
                        QString::number(r) + ":" + QString::number(c));
            else
                QCOMPARE(testWidget->item(r, c)->text(),
                        QString::number(r + 1) + ":" + QString::number(c));
}

void tst_QTableWidget::removeColumn_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<int>("expectedColumnCount");

    QTest::newRow("Empty") << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("1x1:0") << 1 << 1 << 0 << 1 << 0;
    QTest::newRow("3x3:0") << 3 << 3 << 0 << 3 << 2;
    QTest::newRow("3x3:1") << 3 << 3 << 1 << 3 << 2;
    QTest::newRow("3x3:2") << 3 << 3 << 2 << 3 << 2;
}

void tst_QTableWidget::removeColumn()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, column);
    QFETCH(int, expectedRowCount);
    QFETCH(int, expectedColumnCount);

    // set dimensions and test they are ok
    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    // fill table with items
    for (int r = 0; r < rowCount; ++r)
        for (int c = 0; c < columnCount; ++c)
            testWidget->setItem(r, c,
                                new QTableWidgetItem(
                                    QString::number(r) + ":" + QString::number(c)));

    // remove and compare the results
    testWidget->removeColumn(column);
    QCOMPARE(testWidget->rowCount(), expectedRowCount);
    QCOMPARE(testWidget->columnCount(), expectedColumnCount);


    // check if the correct items were removed
    for (int r = 0; r < expectedRowCount; ++r)
        for (int c = 0; c < expectedColumnCount; ++c)
            if (c < column)
                QCOMPARE(testWidget->item(r, c)->text(),
                        QString::number(r) + ":" + QString::number(c));
            else
                QCOMPARE(testWidget->item(r, c)->text(),
                        QString::number(r) + ":" + QString::number(c + 1));
}

void tst_QTableWidget::insertRow_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<int>("expectedColumnCount");

    QTest::newRow("Empty")  << 0 << 0 << 0  << 1 << 0;
    QTest::newRow("1x1:0")  << 1 << 1 << 0  << 2 << 1;
    QTest::newRow("3x3:-1") << 3 << 3 << -1 << 3 << 3;
    QTest::newRow("3x3:0")  << 3 << 3 << 0  << 4 << 3;
    QTest::newRow("3x3:1")  << 3 << 3 << 1  << 4 << 3;
    QTest::newRow("3x3:2")  << 3 << 3 << 2  << 4 << 3;
    QTest::newRow("3x3:3")  << 3 << 3 << 3  << 4 << 3;
    QTest::newRow("3x3:4")  << 3 << 3 << 4  << 3 << 3;
}

void tst_QTableWidget::insertRow()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, row);
    QFETCH(int, expectedRowCount);
    QFETCH(int, expectedColumnCount);

    // set dimensions and test they are ok
    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    // insert and compare the results
    testWidget->insertRow(row);
    QCOMPARE(testWidget->rowCount(), expectedRowCount);
    QCOMPARE(testWidget->columnCount(), expectedColumnCount);
}

void tst_QTableWidget::insertColumn_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("column");
    QTest::addColumn<int>("expectedRowCount");
    QTest::addColumn<int>("expectedColumnCount");

    QTest::newRow("Empty")  << 0 << 0 << 0  << 0 << 1;
    QTest::newRow("1x1:0")  << 1 << 1 << 0  << 1 << 2;
    QTest::newRow("3x3:-1") << 3 << 3 << -1 << 3 << 3;
    QTest::newRow("3x3:0")  << 3 << 3 << 0  << 3 << 4;
    QTest::newRow("3x3:1")  << 3 << 3 << 1  << 3 << 4;
    QTest::newRow("3x3:2")  << 3 << 3 << 2  << 3 << 4;
    QTest::newRow("3x3:3")  << 3 << 3 << 3  << 3 << 4;
    QTest::newRow("3x3:4")  << 3 << 3 << 4  << 3 << 3;
}

void tst_QTableWidget::insertColumn()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, column);
    QFETCH(int, expectedRowCount);
    QFETCH(int, expectedColumnCount);

    // set dimensions and test they are ok
    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);
    QCOMPARE(testWidget->rowCount(), rowCount);
    QCOMPARE(testWidget->columnCount(), columnCount);

    // insert and compare the results
    testWidget->insertColumn(column);
    QCOMPARE(testWidget->rowCount(), expectedRowCount);
    QCOMPARE(testWidget->columnCount(), expectedColumnCount);
}

void tst_QTableWidget::itemStreaming_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("toolTip");

    QTest::newRow("Data") << "item text" << "tool tip text";
}

void tst_QTableWidget::itemStreaming()
{
    QFETCH(QString, text);
    QFETCH(QString, toolTip);

    QTableWidgetItem item;
    QCOMPARE(item.text(), QString());
    QCOMPARE(item.toolTip(), QString());

    item.setText(text);
    item.setToolTip(toolTip);
    QCOMPARE(item.text(), text);
    QCOMPARE(item.toolTip(), toolTip);

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out << item;

    QTableWidgetItem item2;
    QCOMPARE(item2.text(), QString());
    QCOMPARE(item2.toolTip(), QString());

    QVERIFY(!buffer.isEmpty());

    QDataStream in(&buffer, QIODevice::ReadOnly);
    in >> item2;
    QCOMPARE(item2.text(), text);
    QCOMPARE(item2.toolTip(), toolTip);
}

void tst_QTableWidget::itemOwnership()
{
    QPointer<QObjectTableItem> item;
    QPointer<QObjectTableItem> headerItem;

    //delete from outside
    item = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    delete item;
    QCOMPARE(testWidget->item(0, 0), (QTableWidgetItem *)0);

    //delete vertical headeritem from outside
    headerItem = new QObjectTableItem();
    testWidget->setVerticalHeaderItem(0, headerItem);
    delete headerItem;
    QCOMPARE(testWidget->verticalHeaderItem(0), (QTableWidgetItem *)0);

    //delete horizontal headeritem from outside
    headerItem = new QObjectTableItem();
    testWidget->setHorizontalHeaderItem(0, headerItem);
    delete headerItem;
    QCOMPARE(testWidget->horizontalHeaderItem(0), (QTableWidgetItem *)0);

    //setItem
    item = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    testWidget->setItem(0, 0, new QTableWidgetItem());
    QVERIFY(item.isNull());

    //setHorizontalHeaderItem
    headerItem = new QObjectTableItem();
    testWidget->setHorizontalHeaderItem(0, headerItem);
    testWidget->setHorizontalHeaderItem(0, new QTableWidgetItem());
    QVERIFY(headerItem.isNull());

    //setVerticalHeaderItem
    headerItem = new QObjectTableItem();
    testWidget->setVerticalHeaderItem(0, headerItem);
    testWidget->setVerticalHeaderItem(0, new QTableWidgetItem());
    QVERIFY(headerItem.isNull());

    //takeItem
    item = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    testWidget->takeItem(0, 0);
    QVERIFY(!item.isNull());
    delete item;

    // removeRow
    item = new QObjectTableItem();
    headerItem = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    testWidget->setVerticalHeaderItem(0, headerItem);
    testWidget->removeRow(0);
    QVERIFY(item.isNull());
    QVERIFY(headerItem.isNull());

    // removeColumn
    item = new QObjectTableItem();
    headerItem = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    testWidget->setHorizontalHeaderItem(0, headerItem);
    testWidget->removeColumn(0);
    QVERIFY(item.isNull());
    QVERIFY(headerItem.isNull());

    // clear
    item = new QObjectTableItem();
    testWidget->setItem(0, 0, item);
    testWidget->clear();
    QVERIFY(item.isNull());
}

void tst_QTableWidget::sortItems_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<int>("sortColumn");
    QTest::addColumn<QStringList>("initial");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<IntList>("rows");

    QTest::newRow("ascending")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            << "1" << "b" << "7" << "3" << "u")
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "1" << "b" << "7" << "3" << "u"
            << "2" << "c" << "9" << "y" << "8"
            << "3" << "d" << "k" << "o" << "6")
        << (IntList() << 0 << 3 << 2 << 1);

    QTest::newRow("descending")
        << 4 << 5
        << static_cast<int>(Qt::DescendingOrder)
        << 0
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            << "1" << "b" << "7" << "3" << "u")
        << (QStringList()
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            << "1" << "b" << "7" << "3" << "u"
            << "0" << "a" << "o" << "8" << "k")
        << (IntList() << 3 << 0 << 1 << 2);

    QTest::newRow("empty table")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0)
        << (QStringList()
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0
            <<  0  <<  0  <<  0  <<  0  << 0)
        << IntList();


    QTest::newRow("half-empty table")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            <<  "0"  <<   0   <<  0  <<  0  << 0
            <<  "3"  <<  "d"  <<  0  <<  0  << 0
            <<  "2"  <<  "c"  <<  0  <<  0  << 0
            <<   0   <<   0   <<  0  <<  0  << 0)
        << (QStringList()
            <<  "0"  <<   0   <<  0  <<  0  << 0
            <<  "2"  <<  "c"  <<  0  <<  0  << 0
            <<  "3"  <<  "d"  <<  0  <<  0  << 0
            <<   0   <<   0   <<  0  <<  0  << 0)
        << (IntList() << 0 << 2 << 1);

    QTest::newRow("empty column, should not sort.")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 3
        << (QStringList()
            <<  "0"  <<   0   <<  0  <<  0  << 0
            <<  "3"  <<  "d"  <<  0  <<  0  << 0
            <<  "2"  <<  "c"  <<  0  <<  0  << 0
            <<   0   <<   0   <<  0  <<  0  << 0)
        << (QStringList()
            <<  "0"  <<   0   <<  0  <<  0  << 0
            <<  "3"  <<  "d"  <<  0  <<  0  << 0
            <<  "2"  <<  "c"  <<  0  <<  0  << 0
            <<   0   <<   0   <<  0  <<  0  << 0)
        << IntList();

    QTest::newRow("descending with null cell, the null cell should be placed at the bottom")
        << 4 << 5
        << static_cast<int>(Qt::DescendingOrder)
        << 0
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            <<  0  << "b" << "7" << "3" << "u")
        << (QStringList()
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            << "0" << "a" << "o" << "8" << "k"
            <<  0  << "b" << "7" << "3" << "u")
        << (IntList() << 2 << 0 << 1);

    QTest::newRow("ascending with null cell, the null cell should be placed at the bottom")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "3" << "d" << "k" << "o" << "6"
            << "2" << "c" << "9" << "y" << "8"
            <<  0  << "b" << "7" << "3" << "u")
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "2" << "c" << "9" << "y" << "8"
            << "3" << "d" << "k" << "o" << "6"
            <<  0  << "b" << "7" << "3" << "u")
        << (IntList() << 0 << 2 << 1);

    QTest::newRow("ascending with null cells, the null cells should be placed at the bottom")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            << "3" << "d" << "k" << "o" << "6"
            << "0" << "a" << "o" << "8" << "k"
            <<  0  << "c" << "9" << "y" << "8"
            <<  0  << "b" << "7" << "3" << "u")
        << (QStringList()
            << "0" << "a" << "o" << "8" << "k"
            << "3" << "d" << "k" << "o" << "6"
            <<  0  << "c" << "9" << "y" << "8"
            <<  0  << "b" << "7" << "3" << "u")
        << (IntList() << 1 << 0);


    QTest::newRow("ascending... Check a bug in PersistentIndexes")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            << "3" << "c" << "9" << "y" << "8"
            << "2" << "b" << "7" << "3" << "u"
            << "4" << "d" << "k" << "o" << "6"
            << "1" << "a" << "o" << "8" << "k"
            )
        << (QStringList()
            << "1" << "a" << "o" << "8" << "k"
            << "2" << "b" << "7" << "3" << "u"
            << "3" << "c" << "9" << "y" << "8"
            << "4" << "d" << "k" << "o" << "6"
            )
        << (IntList() << 2 << 1 << 3 << 0);

    QTest::newRow("ascending with some null cells inbetween")
        << 4 << 5
        << static_cast<int>(Qt::AscendingOrder)
        << 0
        << (QStringList()
            <<  0  << "a" << "o" << "8" << "k"
            << "2" << "c" << "9" << "y" << "8"
            <<  0  << "d" << "k" << "o" << "6"
            << "1" << "b" << "7" << "3" << "u")
        << (QStringList()
            << "1" << "b" << "7" << "3" << "u"
            << "2" << "c" << "9" << "y" << "8"
            <<  0  << "a" << "o" << "8" << "k"
            <<  0  << "d" << "k" << "o" << "6")
        << (IntList() << 1 << 0);

}

void tst_QTableWidget::sortItems()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, sortOrder);
    QFETCH(int, sortColumn);
    QFETCH(QStringList, initial);
    QFETCH(QStringList, expected);
    QFETCH(IntList, rows);

    testWidget->setRowCount(rowCount);
    testWidget->setColumnCount(columnCount);

    QAbstractItemModel *model = testWidget->model();
    QList<QPersistentModelIndex> persistent;

    int ti = 0;
    for (int r = 0; r < rowCount; ++r) {
        for (int c = 0; c < columnCount; ++c) {
        QString str = initial.at(ti++);
            if (!str.isNull()) {
                testWidget->setItem(r, c, new QTableWidgetItem(str));
            }
        }
        if (testWidget->item(r, sortColumn))
            persistent << model->index(r, sortColumn, QModelIndex());
    }

    testWidget->sortItems(sortColumn, static_cast<Qt::SortOrder>(sortOrder));

    int te = 0;
    for (int i = 0; i < rows.count(); ++i) {
        for (int j = 0; j < columnCount; ++j) {
            QString value;
            QTableWidgetItem *itm = testWidget->item(i, j);
            if (itm) {
                value = itm->text();
            }
            QCOMPARE(value, expected.at(te++));
        }
        QCOMPARE(persistent.at(i).row(), rows.at(i));
    }
}

#if QT_VERSION >= 0x040200
void tst_QTableWidget::setItemWithSorting_data()
{
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<int>("columnCount");
    QTest::addColumn<int>("sortOrder");
    QTest::addColumn<int>("sortColumn");
    QTest::addColumn<QStringList>("initialValues");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("column");
    QTest::addColumn<QString>("newValue");
    QTest::addColumn<QStringList>("expectedValues");
    QTest::addColumn<IntList>("expectedRows");
    QTest::addColumn<bool>("reorderingExpected");

    QTest::newRow("2x1 no change (ascending)")
        << 2 << 1
        << static_cast<int>(Qt::AscendingOrder) << 0
        << (QStringList() << "0" << "1")
        << 1 << 0 << "2"
        << (QStringList() << "0" << "2")
        << (IntList() << 0 << 1)
        << false;
    QTest::newRow("2x1 no change (descending)")
        << 2 << 1
        << static_cast<int>(Qt::DescendingOrder) << 0
        << (QStringList() << "1" << "0")
        << 0 << 0 << "2"
        << (QStringList() << "2" << "0")
        << (IntList() << 0 << 1)
        << false;
    QTest::newRow("2x1 reorder (ascending)")
        << 2 << 1
        << static_cast<int>(Qt::AscendingOrder) << 0
        << (QStringList() << "0" << "1")
        << 0 << 0 << "2"
        << (QStringList() << "1" << "2")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("2x1 reorder (descending)")
        << 2 << 1
        << static_cast<int>(Qt::DescendingOrder) << 0
        << (QStringList() << "1" << "0")
        << 1 << 0 << "2"
        << (QStringList() << "2" << "1")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("2x2 no change (ascending)")
        << 2 << 2
        << static_cast<int>(Qt::AscendingOrder) << 0
        << (QStringList()
            << "0" << "00"
            << "1" << "11")
        << 1 << 0 << "2"
        << (QStringList()
            << "0" << "00"
            << "2" << "11")
        << (IntList() << 0 << 1)
        << false;
    QTest::newRow("2x2 reorder (ascending)")
        << 2 << 2
        << static_cast<int>(Qt::AscendingOrder) << 0
        << (QStringList()
            << "0" << "00"
            << "1" << "11")
        << 0 << 0 << "2"
        << (QStringList()
            << "1" << "11"
            << "2" << "00")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("2x2 reorder (ascending, sortColumn = 1)")
        << 2 << 2
        << static_cast<int>(Qt::AscendingOrder) << 1
        << (QStringList()
            << "00" << "0"
            << "11" << "1")
        << 0 << 1 << "2"
        << (QStringList()
            << "11" << "1"
            << "00" << "2")
        << (IntList() << 1 << 0)
        << true;
    QTest::newRow("2x2 no change (column != sortColumn)")
        << 2 << 2
        << static_cast<int>(Qt::AscendingOrder) << 1
        << (QStringList()
            << "00" << "0"
            << "11" << "1")
        << 0 << 0 << "22"
        << (QStringList()
            << "22" << "0"
            << "11" << "1")
        << (IntList() << 0 << 1)
        << false;
    QTest::newRow("8x4 reorder (ascending, sortColumn = 3)")
        << 8 << 4
        << static_cast<int>(Qt::AscendingOrder) << 3
        << (QStringList()
            << "q" << "v" << "u" << "0"
            << "e" << "j" << "i" << "10"
            << "h" << "d" << "c" << "12"
            << "k" << "g" << "f" << "14"
            << "w" << "y" << "x" << "2"
            << "t" << "s" << "o" << "4"
            << "z" << "p" << "r" << "6"
            << "n" << "m" << "l" << "8")
        << 2 << 3 << "5"
        << (QStringList()
            << "q" << "v" << "u" << "0"
            << "e" << "j" << "i" << "10"
            << "k" << "g" << "f" << "14"
            << "w" << "y" << "x" << "2"
            << "t" << "s" << "o" << "4"
            << "h" << "d" << "c" << "5"
            << "z" << "p" << "r" << "6"
            << "n" << "m" << "l" << "8")
        << (IntList() << 0 << 1 << 5 << 2 << 3 << 4 << 6 << 7)
        << true;
}

void tst_QTableWidget::setItemWithSorting()
{
    QFETCH(int, rowCount);
    QFETCH(int, columnCount);
    QFETCH(int, sortOrder);
    QFETCH(int, sortColumn);
    QFETCH(QStringList, initialValues);
    QFETCH(int, row);
    QFETCH(int, column);
    QFETCH(QString, newValue);
    QFETCH(QStringList, expectedValues);
    QFETCH(IntList, expectedRows);
    QFETCH(bool, reorderingExpected);

    for (int i = 0; i < 2; ++i) {
        QTableWidget w(rowCount, columnCount);

        QAbstractItemModel *model = w.model();
        QList<QPersistentModelIndex> persistent;
        
        int ti = 0;
        for (int r = 0; r < rowCount; ++r) {
            for (int c = 0; c < columnCount; ++c) {
                QString str = initialValues.at(ti++);
                w.setItem(r, c, new QTableWidgetItem(str));
            }
            persistent << model->index(r, sortColumn);
        }
        
        w.setSortingEnabled(true);
        w.sortItems(sortColumn, static_cast<Qt::SortOrder>(sortOrder));
        
        QSignalSpy dataChangedSpy(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)));
        QSignalSpy layoutChangedSpy(model, SIGNAL(layoutChanged()));
        
        if (i == 0) {
            // set a new item
            QTableWidgetItem *item = new QTableWidgetItem(newValue);
            w.setItem(row, column, item);
        } else {
            // change the data of existing item
            QTableWidgetItem *item = w.item(row, column);
            item->setText(newValue);
        }
        
        ti = 0;
        for (int r = 0; r < rowCount; ++r) {
            for (int c = 0; c < columnCount; ++c) {
                QString str = expectedValues.at(ti++);
                QCOMPARE(w.item(r, c)->text(), str);
            }
        }
        
        for (int k = 0; k < persistent.count(); ++k)
            QCOMPARE(persistent.at(k).row(), expectedRows.at(k));
        
        if (i == 0)
            QCOMPARE(dataChangedSpy.count(), reorderingExpected ? 0 : 1);
        else
            QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(layoutChangedSpy.count(), reorderingExpected ? 1 : 0);
    }
}
#endif // QT_VERSION

QTEST_MAIN(tst_QTableWidget)
#include "tst_qtablewidget.moc"
