/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QtCore>
#include <viewstotest.cpp>
#include <stdlib.h>

#if defined(Q_OS_WIN)
#include <time.h>
#define random rand
#define srandom srand
#endif

/*!
    See viewstotest.cpp for instructions on how to have your view tested with these tests.

    Each test such as visualRect have a _data() function which populate the QTest data with
    tests specified by viewstotest.cpp and any extra data needed for that perticular test.

    setupWithNoTestData() fills QTest data with only the tests it is used by most tests.

    There are some basic qDebug statements sprikled about that might be helpfull for
    fixing your issues.
 */
class tst_QItemView : public QObject
{
    Q_OBJECT

public:
    tst_QItemView() {};
    virtual ~tst_QItemView() {};

public slots:
    void init();
    void cleanup();

private slots:
    void nonDestructiveBasicTest_data();
    void nonDestructiveBasicTest();

    void spider_data();
    void spider();

    void resize_data();
    void resize();

    void visualRect_data();
    void visualRect();

    void indexAt_data();
    void indexAt();

    void scrollTo_data();
    void scrollTo();

private:
    void setupWithNoTestData();
    void populate();
    void walkScreen(QAbstractItemView *view);

    QAbstractItemView *view;
    QAbstractItemModel *treeModel;
    ViewsToTest *testViews;
};

/*!
 * Views should not make invalid requests, sense a model might not check all the bad cases.
 */
class CheckerModel : public QStandardItemModel
{
    Q_OBJECT

public:
    CheckerModel() : QStandardItemModel() {};

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole ) const {
        Q_ASSERT(index.isValid());
        return QStandardItemModel::data(index, role);
    };

    Qt::ItemFlags flags(const QModelIndex & index) const {
        Q_ASSERT(index.isValid());
        return QStandardItemModel::flags(index);
    };

    QModelIndex parent ( const QModelIndex & child ) const {
        Q_ASSERT(child.isValid());
        return QStandardItemModel::parent(child);
    };

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {
        Q_ASSERT(section >= 0);
        if (orientation == Qt::Horizontal) { Q_ASSERT(section <= columnCount());};
        if (orientation == Qt::Vertical) { Q_ASSERT(section <= rowCount());};
        return QStandardItemModel::headerData(section, orientation, role);
    }

    QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const {
        Q_ASSERT(row >= 0 && row <= rowCount(parent));
        Q_ASSERT(column >= 0 && column <= columnCount(parent));
        return QStandardItemModel::index(row, column, parent);
    };

    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) {
        Q_ASSERT(index.isValid());
        return QStandardItemModel::setData(index, value, role);
    }

    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) {
        Q_ASSERT(column >= 0 && column <= columnCount());
        QStandardItemModel::sort(column, order);
    };

    QModelIndexList match ( const QModelIndex & start, int role, const QVariant & value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap ) ) const {
        Q_ASSERT(hits > 0);
        Q_ASSERT(value.isValid());
        return QAbstractItemModel::match(start, role, value, hits, flags);
    };

    bool setHeaderData ( int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole ) {
        Q_ASSERT(section >= 0);
        return QAbstractItemModel::setHeaderData(section, orientation, value, role);
    };
};

void tst_QItemView::init()
{
    testViews = new ViewsToTest();
    populate();
}

void tst_QItemView::cleanup()
{
    delete testViews;
    delete view;
    delete treeModel;
    view = 0;
    testViews = 0;
    treeModel = 0;
}

void tst_QItemView::setupWithNoTestData()
{
    ViewsToTest testViews;
    QTest::addColumn<QString>("viewType");
    QTest::addColumn<bool>("displays");
#if QT_VERSION >= 0x040200
    QTest::addColumn<int>("vscroll");
    QTest::addColumn<int>("hscroll");
#endif
    for (int i = 0; i < testViews.tests.size(); ++i) {
        QString view = testViews.tests.at(i).viewType;
        QString test = view + " ScrollPerPixel";
        bool displayIndexes = (testViews.tests.at(i).display == ViewsToTest::DisplayRoot);
        QTest::newRow(test.toLatin1().data()) << view << displayIndexes
#if QT_VERSION >= 0x040200
                                              << (int)QAbstractItemView::ScrollPerPixel
                                              << (int)QAbstractItemView::ScrollPerPixel
#endif
                                              ;
    }
    for (int i = 0; i < testViews.tests.size(); ++i) {
        QString view = testViews.tests.at(i).viewType;
        QString test = view + " ScrolPerItem";
        bool displayIndexes = (testViews.tests.at(i).display == ViewsToTest::DisplayRoot);
        QTest::newRow(test.toLatin1().data()) << view << displayIndexes
#if QT_VERSION >= 0x040200
                                              << (int)QAbstractItemView::ScrollPerItem
                                              << (int)QAbstractItemView::ScrollPerItem
#endif
                                              ;
    }
}

void tst_QItemView::populate()
{
    treeModel = new CheckerModel;
    QModelIndex parent;
    for (int i = 0; i < 4; ++i) {
        parent = treeModel->index(0, 0, parent);
        treeModel->insertRows(0, 26+i, parent);
        treeModel->insertColumns(0, 26+i, parent);
        // Fill in some values to make it easier to debug
        for (int x = 0; x < treeModel->rowCount(); ++x) {
            for (int y = 0; y < treeModel->columnCount(); ++y) {
                QModelIndex index = treeModel->index(x, y, parent);
                treeModel->setData(index, QString("%1_%2_%3").arg(x).arg(y).arg(i));
                treeModel->setData(index, QVariant(QColor(Qt::blue)), Qt::TextColorRole);
            }
        }
    }
}

void tst_QItemView::nonDestructiveBasicTest_data()
{
    setupWithNoTestData();
}

/*!
    nonDestructiveBasicTest trys to call a number of the basic functions (not all)
    to make sure the view doesn't segfault, testing the functions that makes sense.
 */
void tst_QItemView::nonDestructiveBasicTest()
{
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif

    // setSelectionModel() will assert
    //view->setSelectionModel(0);
    // setItemDelegate() will assert
    //view->setItemDelegate(0);

    // setSelectionMode
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::SingleSelection);
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::ContiguousSelection);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::ExtendedSelection);
    view->setSelectionMode(QAbstractItemView::MultiSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::MultiSelection);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    QCOMPARE(view->selectionMode(), QAbstractItemView::NoSelection);

    // setSelectionBehavior
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectItems);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectRows);
    view->setSelectionBehavior(QAbstractItemView::SelectColumns);
    QCOMPARE(view->selectionBehavior(), QAbstractItemView::SelectColumns);

    // setEditTriggers
    view->setEditTriggers(QAbstractItemView::EditKeyPressed);
    QCOMPARE(view->editTriggers(), QAbstractItemView::EditKeyPressed);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QCOMPARE(view->editTriggers(), QAbstractItemView::NoEditTriggers);
    view->setEditTriggers(QAbstractItemView::CurrentChanged);
    QCOMPARE(view->editTriggers(), QAbstractItemView::CurrentChanged);
    view->setEditTriggers(QAbstractItemView::DoubleClicked);
    QCOMPARE(view->editTriggers(), QAbstractItemView::DoubleClicked);
    view->setEditTriggers(QAbstractItemView::SelectedClicked);
    QCOMPARE(view->editTriggers(), QAbstractItemView::SelectedClicked);
    view->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    QCOMPARE(view->editTriggers(), QAbstractItemView::AnyKeyPressed);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    QCOMPARE(view->editTriggers(), QAbstractItemView::AllEditTriggers);

    // setAutoScroll
    view->setAutoScroll(false);
    QCOMPARE(view->hasAutoScroll(), false);
    view->setAutoScroll(true);
    QCOMPARE(view->hasAutoScroll(), true);

    // setTabKeyNavigation
    view->setTabKeyNavigation(false);
    QCOMPARE(view->tabKeyNavigation(), false);
    view->setTabKeyNavigation(true);
    QCOMPARE(view->tabKeyNavigation(), true);

    // setDropIndicatorShown
    view->setDropIndicatorShown(false);
    QCOMPARE(view->showDropIndicator(), false);
    view->setDropIndicatorShown(true);
    QCOMPARE(view->showDropIndicator(), true);

    // setDragEnabled
    view->setDragEnabled(false);
    QCOMPARE(view->dragEnabled(), false);
    view->setDragEnabled(true);
    QCOMPARE(view->dragEnabled(), true);

    // setAlternatingRowColors
    view->setAlternatingRowColors(false);
    QCOMPARE(view->alternatingRowColors(), false);
    view->setAlternatingRowColors(true);
    QCOMPARE(view->alternatingRowColors(), true);

    // setIconSize
    view->setIconSize(QSize(16, 16));
    QCOMPARE(view->iconSize(), QSize(16, 16));
    view->setIconSize(QSize(32, 32));
    QCOMPARE(view->iconSize(), QSize(32, 32));
    // Should this happen?
    view->setIconSize(QSize(-1, -1));
    QCOMPARE(view->iconSize(), QSize(-1, -1));

    QCOMPARE(view->currentIndex(), QModelIndex());
    QCOMPARE(view->rootIndex(), QModelIndex());

    view->keyboardSearch("");
    view->keyboardSearch("foo");
    view->keyboardSearch("1");

    QCOMPARE(view->visualRect(QModelIndex()), QRect());

    view->scrollTo(QModelIndex());

    QCOMPARE(view->sizeHintForIndex(QModelIndex()), QSize());
    QCOMPARE(view->indexAt(QPoint(-1, -1)), QModelIndex());

    if (!view->model()){
        QCOMPARE(view->indexAt(QPoint(10, 10)), QModelIndex());
        QCOMPARE(view->sizeHintForRow(0), -1);
        QCOMPARE(view->sizeHintForColumn(0), -1);
    } else if (view->itemDelegate()){
        view->sizeHintForRow(0);
        view->sizeHintForColumn(0);
    }
    view->openPersistentEditor(QModelIndex());
    view->closePersistentEditor(QModelIndex());

    view->reset();
    view->setRootIndex(QModelIndex());
    view->doItemsLayout();
    view->selectAll();
    // edit() causes warning by default
    //view->edit(QModelIndex());
    view->clearSelection();
    view->setCurrentIndex(QModelIndex());
}

void tst_QItemView::spider_data()
{
    setupWithNoTestData();
}

void touch(QWidget *widget, Qt::KeyboardModifier modifier, Qt::Key keyPress){
    int width = widget->width();
    int height = widget->height();
    for (int i = 0; i < 5; ++i) {
        QTest::mouseClick(widget, Qt::LeftButton, modifier, QPoint(random() % width, random() % height));
        QTest::mouseDClick(widget, Qt::LeftButton, modifier, QPoint(random() % width, random() % height));
        QPoint press(random() % width, random() % height);
        QPoint releasePoint(random() % width, random() % height);
        QTest::mousePress(widget, Qt::LeftButton, modifier, press);
        QTest::mouseMove(widget, releasePoint);
        if (random() % 1 == 0)
            QTest::mouseRelease(widget, Qt::LeftButton, 0, releasePoint);
        else
            QTest::mouseRelease(widget, Qt::LeftButton, modifier, releasePoint);
        QTest::keyClick(widget, keyPress);
    }
}

/*!
    This is a basic stress testing application that tries a few basics such as clicking around
    the screen, and key presses.

    The main goal is to catch any easy segfaults, not to test every case.
  */
void tst_QItemView::spider()
{
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif
    view->setModel(treeModel);
    view->show();

    srandom(time(0));
    touch(view->viewport(), Qt::NoModifier, Qt::Key_Left);
    touch(view->viewport(), Qt::ShiftModifier, Qt::Key_Enter);
    touch(view->viewport(), Qt::ControlModifier, Qt::Key_Backspace);
    touch(view->viewport(), Qt::AltModifier, Qt::Key_Up);
}

void tst_QItemView::resize_data()
{
    setupWithNoTestData();
}

/*!
    The main goal is to catch any infinite loops from layouting
  */
void tst_QItemView::resize()
{
    return;
    // This test needs to be re-thought out, it takes too long and
    // doesn't really catch theproblem.
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif
    view->setModel(treeModel);
    view->show();

    for (int w = 100; w < 400; w+=10) {
        for (int h = 100; h < 400; h+=10) {
            view->resize(w, h);
            QTest::qWait(1);
            qApp->processEvents();
        }
    }
}

void tst_QItemView::visualRect_data()
{
    setupWithNoTestData();
}

void tst_QItemView::visualRect()
{
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif
    QCOMPARE(view->visualRect(QModelIndex()), QRect());

    // Add model
    view->setModel(treeModel);
    QCOMPARE(view->visualRect(QModelIndex()), QRect());

    QModelIndex topIndex = treeModel->index(0,0);

    QFETCH(bool, displays);
    if (!displays){
        QVERIFY(view->visualRect(topIndex) == QRect());
        return;
    }

    QVERIFY(view->visualRect(topIndex) != QRect());
    view->show();
    QVERIFY(view->visualRect(topIndex) != QRect());

    QVERIFY(topIndex == view->indexAt(view->visualRect(topIndex).center()));
    QVERIFY(topIndex == view->indexAt(view->visualRect(topIndex).bottomLeft()));
    QVERIFY(topIndex == view->indexAt(view->visualRect(topIndex).bottomRight()));
    QVERIFY(topIndex == view->indexAt(view->visualRect(topIndex).topLeft()));
    QVERIFY(topIndex == view->indexAt(view->visualRect(topIndex).topRight()));

    QModelIndex hiddenIndex = testViews->hiddenIndex(view);
    QVERIFY(view->visualRect(hiddenIndex) == QRect());
}

void tst_QItemView::walkScreen(QAbstractItemView *view)
{
    QModelIndex hiddenIndex = view->model() ? testViews->hiddenIndex(view) : QModelIndex();
    int width = view->width();
    int height = view->height();
    for (int w = 0; w < width; ++w)
    {
        for (int h = 0; h < height; ++h)
        {
            QPoint point(w, h);
            QModelIndex index = view->indexAt(point);

            // If we have no model then we should *never* get a valid index
            if (!view->model() || !view->isVisible())
                QVERIFY(!index.isValid());
            // index should not be the hidden one
            if (hiddenIndex.isValid())
                QVERIFY(hiddenIndex != index);
            // If we are valid then check the visualRect for that index
            if (index.isValid()){
                QRect visualRect = view->visualRect(index);
                if (!visualRect.contains(point))
                    qDebug() << point << visualRect;
                QVERIFY(visualRect.contains(point));
            }
        }
    }
}

void walkIndex(QModelIndex index, QAbstractItemView *view)
{
    QRect visualRect = view->visualRect(index);
    //if (index.column() == 0)
    //qDebug() << index << visualRect;
    int width = visualRect.width();
    int height = visualRect.height();

    for (int w = 0; w < width; ++w)
    {
        for (int h = 0; h < height; ++h)
        {
            QPoint point(visualRect.x()+w, visualRect.y()+h);
            if (view->indexAt(point) != index) {
                qDebug() << "index" << index << "visualRect" << visualRect << point << view->indexAt(point);
            }
            QVERIFY(view->indexAt(point) == index);
        }
    }

}

/*!
    A model that returns an index of parent X should also return X when asking
    for the parent of the index.

    This recursive function does pretty extensive testing on the whole model in an
    effort to catch edge cases.

    This function assumes that rowCount(), columnCount() and index() work.  If they have
    a bug it will point it out, but the above tests should have already found the basic bugs
    because it is easier to figure out the problem in those tests then this one.
 */
void checkChildren(QAbstractItemView *currentView, const QModelIndex &parent = QModelIndex(), int currentDepth=0)
{
    QAbstractItemModel *currentModel = currentView->model();

    int rows = currentModel->rowCount(parent);
    int columns = currentModel->columnCount(parent);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < columns; ++c) {
            QModelIndex index = currentModel->index(r, c, parent);
            walkIndex(index, currentView);
            if (QTest::currentTestFailed())
                return;

            // recursivly go down
            if (currentModel->hasChildren(index) && currentDepth < 2) {
                checkChildren(currentView, index, ++currentDepth);
                // Because this is recursive we will return at the first failure rather then
                // reporting it over and over
                if (QTest::currentTestFailed())
                    return;
            }
        }
    }
}


void tst_QItemView::indexAt_data()
{
    setupWithNoTestData();
}

void tst_QItemView::indexAt()
{
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif
    view->show();
    view->setModel(treeModel);
#if 0
    checkChildren(view);

    QModelIndex index = view->model()->index(0, 0);
    while (view->model()->hasChildren(index))
        index = view->model()->index(0, 0, index);
    QCOMPARE(view->model()->hasChildren(index), false);
    view->setRootIndex(index);
    //qDebug() << view->indexAt(QPoint(view->width()/2, view->height()/2)) << view->rootIndex();
    QPoint p(1, view->height()/2);
    QModelIndex idx = view->indexAt(p);
    QCOMPARE(idx, view->rootIndex());
#endif
}

void tst_QItemView::scrollTo_data()
{
    setupWithNoTestData();
}

void tst_QItemView::scrollTo()
{
    QFETCH(QString, viewType);
    view = testViews->createView(viewType);
#if QT_VERSION >= 0x040200
    QFETCH(int, vscroll);
    QFETCH(int, hscroll);
    view->setVerticalScrollMode((QAbstractItemView::ScrollMode)vscroll);
    view->setHorizontalScrollMode((QAbstractItemView::ScrollMode)hscroll);
#endif
    view->setModel(treeModel);
    view->show();

    QModelIndex parent;
    for (int row = 0; row < treeModel->rowCount(parent); ++row) {
        for (int column = 0; column < treeModel->columnCount(parent); ++column) {
            QModelIndex idx = treeModel->index(row, column, parent);
            view->scrollTo(idx);
            QRect rect = view->visualRect(idx);
            view->scrollTo(idx);
            QCOMPARE(rect, view->visualRect(idx));
        }
    }

    QModelIndex idx = treeModel->index(0, 0, parent);
    view->scrollTo(idx);
    QRect rect = view->visualRect(idx);
    view->scrollToBottom();
    view->scrollTo(idx);
    QCOMPARE(rect, view->visualRect(idx));
}

QTEST_MAIN(tst_QItemView)
#include "tst_qitemview.moc"
