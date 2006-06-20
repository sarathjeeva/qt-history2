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
#include <QtSql/QtSql>
#include <QtGui/QtGui>
#include <QSortFilterProxyModel>

/*
    To add a model to be tested add the header file to the includes
    and impliment what is needed in the four functions below.

    You can add more then one model, several Qt models and included as examples.

    In tst_qitemmodel.cpp a new ModelsToTest object is created for each test.

    When you have errors fix the first ones first.  Later tests depend upon them working
*/

class ModelsToTest {

public:
    ModelsToTest();

    QAbstractItemModel *createModel(const QString &modelType);
    QModelIndex populateTestArea(QAbstractItemModel *model);
    void cleanupTestArea(QAbstractItemModel *model);

    enum Read {
        ReadOnly, // wont perform remove(), insert(), and setData()
        ReadWrite
    };
    enum Contains {
        Empty,  // Confirm that rowCount() == 0 etc throughout the test
        HasData // Confirm that rowCount() != 0 etc throughout the test
    };

    struct test {
            test(QString m, Read r, Contains c) : modelType(m), read(r), contains(c){};

            QString modelType;
            Read read;
            Contains contains;
    };

    QList<test> tests;

    static void setupDatabase();
};


/*!
    Add new tests, they can be the same model, but in a different state.

    The name of the model is passed to createModel
    If readOnly is true the remove tests will be skipped.  Example: QDirModel is disabled.
    If createModel returns an empty model.  Example: QDirModel does not
 */
ModelsToTest::ModelsToTest()
{
    setupDatabase();

    tests.append(test("QDirModel", ReadOnly, HasData));
    tests.append(test("QStringListModel", ReadWrite, HasData));
    tests.append(test("QStringListModelEmpty", ReadWrite, Empty));

    tests.append(test("QStandardItemModel", ReadWrite, HasData));
    tests.append(test("QStandardItemModelEmpty", ReadWrite, Empty));

    // QSortFilterProxyModel test uses QStandardItemModel so test it first
    tests.append(test("QSortFilterProxyModel", ReadWrite, HasData));
    tests.append(test("QSortFilterProxyModelEmpty", ReadWrite, Empty));
    tests.append(test("QSortFilterProxyModelRegExp", ReadWrite, HasData));

    tests.append(test("QListModel", ReadWrite, HasData));
    tests.append(test("QListModelEmpty", ReadWrite, Empty));
    tests.append(test("QTableModel", ReadWrite, HasData));
    tests.append(test("QTableModelEmpty", ReadWrite, Empty));

#if QT_VERSION >= 0x040200
#define TABLEFEATURE  ReadWrite
#else
#define TABLEFEATURE  ReadOnly
#endif
    tests.append(test("QTreeModel", TABLEFEATURE, HasData));
    tests.append(test("QTreeModelEmpty", TABLEFEATURE, Empty));

    tests.append(test("QSqlQueryModel", ReadOnly, HasData));
    tests.append(test("QSqlQueryModelEmpty", ReadOnly, Empty));

    // Fails on remove
    tests.append(test("QSqlTableModel", ReadOnly, HasData));
}

/*!
    Return a new modelType.
 */
QAbstractItemModel *ModelsToTest::createModel(const QString &modelType)
{
    if (modelType == "QStringListModelEmpty")
        return new QStringListModel();

    if (modelType == "QStringListModel") {
        QStringListModel *model = new QStringListModel();
        populateTestArea(model);
        return model;
    }

    if (modelType == "QStandardItemModelEmpty") {
        return new QStandardItemModel();
    }

    if (modelType == "QStandardItemModel") {
        QStandardItemModel *model = new QStandardItemModel();
        populateTestArea(model);
        return model;
    }

    if (modelType == "QSortFilterProxyModelEmpty") {
        QSortFilterProxyModel *model = new QSortFilterProxyModel;
        QStandardItemModel *standardItemModel = new QStandardItemModel;
        model->setSourceModel(standardItemModel);
        return model;
    }

    if (modelType == "QSortFilterProxyModelRegExp") {
        QSortFilterProxyModel *model = new QSortFilterProxyModel;
        QStandardItemModel *standardItemModel = new QStandardItemModel;
        model->setSourceModel(standardItemModel);
        populateTestArea(model);
        model->setFilterRegExp(QRegExp("(^$|0.*)"));
        return model;
    }

    if (modelType == "QSortFilterProxyModel") {
        QSortFilterProxyModel *model = new QSortFilterProxyModel;
        QStandardItemModel *standardItemModel = new QStandardItemModel;
        model->setSourceModel(standardItemModel);
        populateTestArea(model);
        return model;
    }

    if (modelType == "QDirModel") {
        QDirModel *model = new QDirModel();
        model->setReadOnly(false);
        return model;
    }

    if (modelType == "QSqlQueryModel") {
        QSqlQueryModel *model = new QSqlQueryModel();
        populateTestArea(model);
        return model;
    }

    if (modelType == "QSqlQueryModelEmpty") {
        QSqlQueryModel *model = new QSqlQueryModel();
        return model;
    }

    if (modelType == "QSqlTableModel") {
        QSqlTableModel *model = new QSqlTableModel();
        populateTestArea(model);
        return model;
    }

    if (modelType == "QListModelEmpty")
        return (new QListWidget)->model();

    if (modelType == "QListModel") {
        QListWidget *widget = new QListWidget;
        populateTestArea(widget->model());
        return widget->model();
    }

    if (modelType == "QTableModelEmpty")
        return (new QTableWidget)->model();

    if (modelType == "QTableModel") {
        QTableWidget *widget = new QTableWidget;
        populateTestArea(widget->model());
        return widget->model();
    }

    if (modelType == "QTreeModelEmpty") {
        QTreeWidget *widget = new QTreeWidget;
        return widget->model();
    }

    if (modelType == "QTreeModel") {
        QTreeWidget *widget = new QTreeWidget;
        populateTestArea(widget->model());
        return widget->model();
    }

    Q_ASSERT(false);
    return 0;
}

/*!
    Fills model with some test data at least twenty rows and if possible twenty or more columns.
    Return the parent of a row/column that can be tested.

    NOTE: If readOnly is true tests will try to remove and add rows and columns.
    If you have a tree model returning not the root index will help catch more errors.
 */
QModelIndex ModelsToTest::populateTestArea(QAbstractItemModel *model)
{
    if (QStringListModel *stringListModel = qobject_cast<QStringListModel *>(model)) {
        QString alphabet = "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z";
        stringListModel->setStringList( alphabet.split(",") );
        return QModelIndex();
    }

    if (qobject_cast<QStandardItemModel *>(model)) {
        // Basic tree StandardItemModel
        QModelIndex parent;
        for (int i = 0; i < 4; ++i) {
            parent = model->index(0, 0, parent);
            model->insertRows(0, 26+i, parent);
            model->insertColumns(0, 26+i, parent);
            // Fill in some values to make it easier to debug
            for (int x = 0; x < 26+i; ++x) {
                for (int y = 0; y < 26+i; ++y) {
                    QModelIndex index = model->index(x, y, parent);
                    model->setData(index, QString("%1_%2_%3").arg(x).arg(y).arg(i));
                    model->setData(index, QVariant(QColor(Qt::blue)), Qt::TextColorRole);
                }
            }
        }
        return model->index(0,0);
    }

    if (qobject_cast<QSortFilterProxyModel *>(model)) {
        QAbstractItemModel *realModel = (qobject_cast<QSortFilterProxyModel *>(model))->sourceModel();
        // Basic tree StandardItemModel
        QModelIndex parent;
        for (int i = 0; i < 4; ++i) {
            parent = realModel->index(0, 0, parent);
            realModel->insertRows(0, 26+i, parent);
            realModel->insertColumns(0, 26+i, parent);
            // Fill in some values to make it easier to debug
            for (int x = 0; x < 26+i; ++x) {
                for (int y = 0; y < 26+i; ++y) {
                    QModelIndex index = realModel->index(x, y, parent);
                    realModel->setData(index, QString("%1_%2_%3").arg(x).arg(y).arg(i));
                    realModel->setData(index, QVariant(QColor(Qt::blue)), Qt::TextColorRole);
                }
            }
        }
        QModelIndex returnIndex = model->index(0,0);
        Q_ASSERT(returnIndex.isValid());
        return returnIndex;
    }

    if (QDirModel *dirModel = qobject_cast<QDirModel *>(model)) {
        // Don't risk somthing bad happening, assert if this fails
        Q_ASSERT(QDir(QDir::currentPath()).mkdir("test"));
        for (int i = 0; i < 26; ++i)
            Q_ASSERT(QDir(QDir::currentPath()).mkdir(QString("test/foo_%1").arg(i)));
        return dirModel->index(QDir::currentPath()+"/test");
    }

    if (QSqlQueryModel *queryModel = qobject_cast<QSqlQueryModel *>(model)) {
        QSqlQuery q;
        q.exec("CREATE TABLE test(id int primary key, name varchar(30))");
        q.prepare("INSERT INTO test(id, name) values (?, ?)");
        for (int i = 0; i < 1024; ++i) {
            q.addBindValue(i);
            q.addBindValue("Mr. Smith" + QString::number(i));
            q.exec();
        }
        if (QSqlTableModel *tableModel = qobject_cast<QSqlTableModel *>(model)) {
            tableModel->setTable("test");
            tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
            tableModel->select();
        } else {
            queryModel->setQuery("select * from test");
        }
        return QModelIndex();
    }

    if (QListWidget *listWidget = qobject_cast<QListWidget *>(model->parent())) {
        int items = 100;
        while (items--)
            listWidget->addItem(QString("item %1").arg(items));
        return QModelIndex();
    }

    if (QTableWidget *tableWidget = qobject_cast<QTableWidget *>(model->parent())) {
        tableWidget->setColumnCount(20);
        tableWidget->setRowCount(20);
        return QModelIndex();
    }

    if (QTreeWidget *treeWidget = qobject_cast<QTreeWidget *>(model->parent())) {
        int topItems = 20;
        treeWidget->setColumnCount(1);
        QTreeWidgetItem *parent;
        while (topItems--){
            parent = new QTreeWidgetItem(treeWidget, QStringList(QString("top %1").arg(topItems)));
            for (int i = 0; i < 20; ++i)
                new QTreeWidgetItem(parent, QStringList(QString("child %1").arg(topItems)));
        }
        return QModelIndex();
    }

    Q_ASSERT(false);
    return QModelIndex();
}

/*!
    If you need to cleanup from populateTest() do it here.
    Note that this is called after every test even if populateTestArea isn't called.
 */
void ModelsToTest::cleanupTestArea(QAbstractItemModel *model)
{
    if (qobject_cast<QDirModel *>(model))
    {
        if (QDir(QDir::currentPath()+"/test").exists())
        {
            for (int i = 0; i < 26; ++i)
                QDir::current().rmdir(QString("test/foo_%1").arg(i));
            Q_ASSERT(QDir::current().rmdir("test"));
        }
    } else if (qobject_cast<QSqlQueryModel *>(model)) {
        QSqlQuery q("DROP TABLE test");
    }
}

void ModelsToTest::setupDatabase()
{
    if (!QSqlDatabase::database().isValid()) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(":memory:");
        if (!db.open()) {
            qWarning() << "Unable to open database" << db.lastError();
            return;
        }
    }
}

