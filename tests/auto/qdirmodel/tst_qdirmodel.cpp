/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qdirmodel.h>
#include <qapplication.h>
#include <qdir.h>
#include <qdebug.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qdirmodel.h gui/itemviews/qdirmodel.cpp

class tst_QDirModel : public QObject
{
    Q_OBJECT

public:

    tst_QDirModel();
    virtual ~tst_QDirModel();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    /*
    void construct();
    void rowCount();
    void columnCount();
    void t_data();
    void setData();
    void hasChildren();
    void isEditable();
    void isDragEnabled();
    void isDropEnabled();
    void sort();
    */
    bool rowsAboutToBeRemoved_init(const QString &test_path, const QStringList &initial_files);
    bool rowsAboutToBeRemoved_cleanup(const QString &test_path);
    void rowsAboutToBeRemoved_data();
    void rowsAboutToBeRemoved();

    void mkdir_data();
    void mkdir();

    void rmdir_data();
    void rmdir();
    
    void filePath();

    void hidden();

    void fileName();
    void fileName_data();
#if QT_VERSION >= 0x040102
    void filter();
};

// Testing get/set functions
void tst_QDirModel::getSetCheck()
{
    QDirModel obj1;
    // QFileIconProvider * QDirModel::iconProvider()
    // void QDirModel::setIconProvider(QFileIconProvider *)
    QFileIconProvider *var1 = new QFileIconProvider;
    obj1.setIconProvider(var1);
    QCOMPARE(var1, obj1.iconProvider());
    obj1.setIconProvider((QFileIconProvider *)0);
    QCOMPARE((QFileIconProvider *)0, obj1.iconProvider());
    delete var1;

    // bool QDirModel::resolveSymlinks()
    // void QDirModel::setResolveSymlinks(bool)
    obj1.setResolveSymlinks(false);
    QCOMPARE(false, obj1.resolveSymlinks());
    obj1.setResolveSymlinks(true);
    QCOMPARE(true, obj1.resolveSymlinks());

    // bool QDirModel::lazyChildCount()
    // void QDirModel::setLazyChildCount(bool)
    obj1.setLazyChildCount(false);
    QCOMPARE(false, obj1.lazyChildCount());
    obj1.setLazyChildCount(true);
    QCOMPARE(true, obj1.lazyChildCount());
}
#endif

Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QModelIndexList)

tst_QDirModel::tst_QDirModel()

{
}

tst_QDirModel::~tst_QDirModel()
{
}

void tst_QDirModel::initTestCase()
{
}

void tst_QDirModel::cleanupTestCase()
{
    QDir current;
    current.rmdir(".qtest_hidden");
}

void tst_QDirModel::init()
{
}

void tst_QDirModel::cleanup()
{
}

/*
  tests
*/
/*
void tst_QDirModel::construct()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QFileInfo info(QDir::currentPath() + "/test/file03.tst");
    QCOMPARE(model.filePath(index), info.absoluteFilePath());
}

void tst_QDirModel::rowCount()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());
    QCOMPARE(model.rowCount(index), 4);
}

void tst_QDirModel::columnCount()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());
    QCOMPARE(model.columnCount(index), 4);
}

void tst_QDirModel::t_data()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());
    QCOMPARE(model.rowCount(index), 4);

    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QCOMPARE(model.data(index).toString(), QString::fromLatin1("file03.tst"));
    QCOMPARE(model.rowCount(index), 0);
}

void tst_QDirModel::setData()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());

    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QVERIFY(!model.setData(index, "file0X.tst", Qt::EditRole));
}

void tst_QDirModel::hasChildren()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());

    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QVERIFY(!model.hasChildren(index));
}

void tst_QDirModel::isEditable()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());

    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QVERIFY(!(model.flags(index) & Qt::ItemIsEditable));
}

void tst_QDirModel::isDragEnabled()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());

    index = model.index(2, 0, index);
    QVERIFY(index.isValid());
    QVERIFY(model.flags(index) & Qt::ItemIsDragEnabled);
}

void tst_QDirModel::isDropEnabled()
{
    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    QVERIFY(index.isValid());

    index = model.index(2, 0, index);
    QVERIFY(!(model.flags(index) & Qt::ItemIsDropEnabled));
}

void tst_QDirModel::sort()
{
    QDirModel model;
    QModelIndex parent = model.index(QDir::currentPath() + "/test");
    QVERIFY(parent.isValid());

    QModelIndex index = model.index(0, 0, parent);
    QCOMPARE(model.data(index).toString(), QString::fromLatin1("file01.tst"));

    index = model.index(3, 0, parent);
    QCOMPARE(model.data(index).toString(), QString::fromLatin1("file04.tst"));

    model.sort(0, Qt::DescendingOrder);
    parent = model.index(QDir::currentPath() + "/test");

    index = model.index(0, 0, parent);
    QCOMPARE(model.data(index).toString(), QString::fromLatin1("file04.tst"));

    index = model.index(3, 0, parent);
    QCOMPARE(model.data(index).toString(), QString::fromLatin1("file01.tst"));
}
*/

void tst_QDirModel::mkdir_data()
{
    QTest::addColumn<QString>("dirName"); // the directory to be made under <currentpath>/dirtest
    QTest::addColumn<bool>("mkdirSuccess");
    QTest::addColumn<int>("rowCount");

    QTest::newRow("okDirName") << QString("test2") << true << 2;
    QTest::newRow("existingDirName") << QString("test1") << false << 1;
    QTest::newRow("nameWithSpace") << QString("ab cd") << true << 2;
    QTest::newRow("emptyDirName") << QString("") << false << 1;
    QTest::newRow("nullDirName") << QString() << false << 1;
    
/*
    QTest::newRow("recursiveDirName") << QString("test2/test3") << false << false;
    QTest::newRow("singleDotDirName") << QString("./test3") << true << true;
    QTest::newRow("outOfTreeDirName") << QString("../test4") << false << false;
    QTest::newRow("insideTreeDirName") << QString("../dirtest/test4") << true << true;
    QTest::newRow("insideTreeDirName2") << QString("./././././../dirtest/./../dirtest/test4") << true << true;
    QTest::newRow("absoluteDirName") << QString(QDir::currentPath() + "/dirtest/test5") << true << true;
    QTest::newRow("outOfTreeDirName") << QString(QDir::currentPath() + "/test5") << false << false;

    // Directory names only illegal on Windows
#ifdef Q_WS_WIN
    QTest::newRow("illegalDirName") << QString("*") << false << false;
    QTest::newRow("illegalDirName2") << QString("|") << false << false;
    QTest::newRow("onlySpace") << QString(" ") << false << false;
#endif
    */
}

void tst_QDirModel::mkdir()
{
    QFETCH(QString, dirName);
    QFETCH(bool, mkdirSuccess);
    QFETCH(int, rowCount);

    QDirModel model;
    model.setReadOnly(false);

    QModelIndex parent = model.index(QDir::currentPath() + "/dirtest");    
    QVERIFY(parent.isValid());
    QCOMPARE(model.rowCount(parent), 1); // start out with only 'test1' - in's in the depot

    QModelIndex index = model.mkdir(parent, dirName);
    bool success = index.isValid();
    int rows = model.rowCount(parent);

    if (success && !model.rmdir(index))
        QVERIFY(QDir(QDir::currentPath() + "/dirtests").rmdir(dirName));

    QCOMPARE(rows, rowCount);
    QCOMPARE(success, mkdirSuccess);
}

void tst_QDirModel::rmdir_data()
{
    QTest::addColumn<QString>("dirName"); // <currentpath>/dirtest/dirname
    QTest::addColumn<bool>("rmdirSuccess");
    QTest::addColumn<int>("rowCount");

    QTest::newRow("okDirName") << QString("test2") << true << 2;
    QTest::newRow("existingDirName") << QString("test1") << false << 1;
    QTest::newRow("nameWithSpace") << QString("ab cd") << true << 2;
    QTest::newRow("emptyDirName") << QString("") << false << 1;
    QTest::newRow("nullDirName") << QString() << false << 1;
}

void tst_QDirModel::rmdir()
{
    QFETCH(QString, dirName);
    QFETCH(bool, rmdirSuccess);
    QFETCH(int, rowCount);

    QDirModel model;
    model.setReadOnly(false);

    QModelIndex parent = model.index(QDir::currentPath() + "/dirtest");
    QVERIFY(parent.isValid());
    QCOMPARE(model.rowCount(parent), 1); // start out with only 'test1' - in's in the depot

    QModelIndex index;
    if (rmdirSuccess) {
        index = model.mkdir(parent, dirName);
        QVERIFY(index.isValid());
    }

    int rows = model.rowCount(parent);
    bool success = model.rmdir(index);

    if (!success) { // cleanup
        QDir dirtests(QDir::currentPath() + "/dirtests/");
        dirtests.rmdir(dirName);
    }

    QCOMPARE(rows, rowCount);
    QCOMPARE(success, rmdirSuccess);
}

void tst_QDirModel::rowsAboutToBeRemoved_data()
{
    QTest::addColumn<QString>("test_path");
    QTest::addColumn<QStringList>("initial_files");
    QTest::addColumn<int>("remove_row");
    QTest::addColumn<QStringList>("remove_files");
    QTest::addColumn<QStringList>("expected_files");

    QString test_path = "test2";
    QStringList initial_files = (QStringList()
                                 << "file1.tst"
                                 << "file2.tst"
                                 << "file3.tst"
                                 << "file4.tst");

    QTest::newRow("removeFirstRow")
        << test_path
        << initial_files
        << 0
        << (QStringList() << "file1.tst")
        << (QStringList() << "file2.tst" << "file3.tst" << "file4.tst");

    QTest::newRow("removeMiddle")
        << test_path
        << initial_files
        << 1
        << (QStringList() << "file2.tst")
        << (QStringList() << "file1.tst" << "file3.tst" << "file4.tst");

    QTest::newRow("removeLastRow")
        << test_path
        << initial_files
        << 3
        << (QStringList() << "file4.tst")
        << (QStringList() <<  "file1.tst" << "file2.tst" << "file3.tst");

}

bool tst_QDirModel::rowsAboutToBeRemoved_init(const QString &test_path, const QStringList &initial_files)
{
    QString path = QDir::currentPath() + "/" + test_path;
    if (!QDir::current().mkdir(test_path) && false) { // FIXME
        qDebug() << "failed to create dir" << path;
        return false;
    }

    for (int i = 0; i < initial_files.count(); ++i) {
        QFile file(path + "/" + initial_files.at(i));
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to open file" << initial_files.at(i);
            return false;
        }
        if (!file.resize(1024)) {
            qDebug() << "failed to resize file" << initial_files.at(i);
            return false;
        }
        if (!file.flush()) {
            qDebug() << "failed to flush file" << initial_files.at(i);
            return false;
        }
    }

    return true;
}

bool tst_QDirModel::rowsAboutToBeRemoved_cleanup(const QString &test_path)
{
    QString path = QDir::currentPath() + "/" + test_path;
    QDir dir(path, "*", QDir::SortFlags(QDir::Name|QDir::IgnoreCase), QDir::Files);
    QStringList files = dir.entryList();
    
    for (int i = 0; i < files.count(); ++i) {
        if (!dir.remove(files.at(i))) {
            qDebug() << "failed to remove file" << files.at(i);
            return false;
        }
    }

    if (!QDir::current().rmdir(test_path) && false) { // FIXME
        qDebug() << "failed to remove dir" << test_path;
        return false;
    }

    return true;
}

void tst_QDirModel::rowsAboutToBeRemoved()
{
    QFETCH(QString, test_path);
    QFETCH(QStringList, initial_files);
    QFETCH(int, remove_row);
    QFETCH(QStringList, remove_files);
    QFETCH(QStringList, expected_files);

    rowsAboutToBeRemoved_cleanup(test_path); // clean up first
    QVERIFY(rowsAboutToBeRemoved_init(test_path, initial_files));

    QDirModel model;
    model.setReadOnly(false);

    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy(&model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)));

    QModelIndex parent = model.index(test_path);
    QVERIFY(parent.isValid());
        
    // remove the file
    {
        QModelIndex index = model.index(remove_row, 0, parent);
        QVERIFY(index.isValid());
        QVERIFY(model.remove(index));
    }

    QCOMPARE(spy.count(), 1);

    // Compare the result
    for (int row = 0; row < expected_files.count(); ++row) {
        QModelIndex index = model.index(row, 0, parent);
        QString str = index.data().toString();
        QCOMPARE(str, expected_files.at(row));
    }

    QVERIFY(rowsAboutToBeRemoved_cleanup(test_path));
}

void tst_QDirModel::hidden()
{
#ifdef Q_OS_UNIX
    QDir current;
    current.mkdir(".qtest_hidden");

    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/.qtest_hidden");
    //QVERIFY(!index.isValid()); // hidden items are not listed, but if you specify a valid path, it will give a valid index

    current.mkdir(".qtest_hidden/qtest_visible");
    QModelIndex index2 = model.index(QDir::currentPath() + "/.qtest_hidden/qtest_visible");
    QVERIFY(index2.isValid());

    QDirModel model2;
    model2.setFilter(model2.filter() | QDir::Hidden);
    index = model2.index(QDir::currentPath() + "/.qtest_hidden");
    QVERIFY(index.isValid());
#else
    QSKIP("Test not implemented on non-Unixes", SkipAll);
#endif
}

void tst_QDirModel::fileName_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("result");

    QTest::newRow("invalid") << "" << "";
    //QTest::newRow("root") << "/" << "/";
    //QTest::newRow("home") << "/home" << "home";
    // TODO add symlink test too
}

void tst_QDirModel::fileName()
{
    QDirModel model;

    QFETCH(QString, path);
    QFETCH(QString, result);
    QCOMPARE(model.fileName(model.index(path)), result);
}

#if 0
void tst_QDirModel::unreadable()
{
#ifdef Q_OS_UNIX
    //QFile current("qtest_unreadable");
    //QVERIFY(current.setPermissions(QFile::WriteOwner));

    QDirModel model;
    QModelIndex index = model.index(QDir::currentPath() + "/qtest_unreadable");
    QVERIFY(!index.isValid());

    QDirModel model2;
    model2.setFilter(model2.filter() | QDir::Hidden);
    index = model2.index(QDir::currentPath() + "/qtest_unreadable");
    QVERIFY(index.isValid());
#else
    QSKIP("Test not implemented on non-Unixes", SkipAll);
#endif
}
#endif

void tst_QDirModel::filePath()
{
    QFile::remove("test.lnk");
    QVERIFY(QFile("tst_qdirmodel.cpp").link("test.lnk"));
    QDirModel model;
    model.setResolveSymlinks(false);
    QModelIndex index = model.index(QDir::currentPath() + "/test.lnk");
    QVERIFY(index.isValid());
    QCOMPARE(model.filePath(index), QDir::currentPath() + "/test.lnk");
    model.setResolveSymlinks(true);
    QCOMPARE(model.filePath(index), QDir::currentPath() + "/tst_qdirmodel.cpp");
}

#if QT_VERSION >= 0x040102
void tst_QDirModel::filter()
{
    QDirModel model;
    model.setNameFilters(QStringList() << "*.nada");
    QModelIndex index = model.index(QDir::currentPath() + "/test");
    Q_ASSERT(model.rowCount(index) == 0);
    QModelIndex index2 = model.index(QDir::currentPath() + "/test/file01.tst");
    Q_ASSERT(!index2.isValid());
    Q_ASSERT(model.rowCount(index) == 0);
}
#endif

QTEST_MAIN(tst_QDirModel)
#include "tst_qdirmodel.moc"
