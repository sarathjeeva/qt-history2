/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qmap.h>
#include <qstringlistmodel.h>
#include <qstringlist.h>
#include <iostream>
#include <qlistview.h>
#include "qmodellistener.h"
#include <qstringlistmodel.h>

//TESTED_CLASS=
//TESTED_FILES=gui/itemviews/qabstractitemmodel.h gui/itemviews/qabstractitemmodel.cpp

void QModelListener::rowsAboutToBeRemovedOrInserted(const QModelIndex & parent, int start, int end )
{
    int i;
    for (i = 0; start + i <= end; i++)
    {
        QModelIndex mIndex = m_pModel->index(start + i, 0, parent);
        QVariant var = m_pModel->data(mIndex, Qt::DisplayRole);
        QString str = var.toString();
        
        QCOMPARE(str, m_pAboutToStringlist->at(i));
    }
}

void QModelListener::rowsRemovedOrInserted(const QModelIndex & parent, int start, int end )
{
    int i;
    // Can the rows that *are* removed be iterated now ?

    // What about rowsAboutToBeInserted - what will the indices be?
    // will insertRow() overwrite existing, or insert (and conseq. grow the model?)
    // What will the item then contain? empty data?

    // RemoveColumn. Does that also fire the rowsRemoved-family signals?

    for (i = 0; i < m_pExpectedStringlist->size(); i++)
    {
        QModelIndex mIndex = m_pModel->index(i, 0, parent);
        QVariant var = m_pModel->data(mIndex, Qt::DisplayRole);
        QString str = var.toString();
        
        //qDebug() << "index: " << i << " start: " << start << "end: " << end;
        QCOMPARE(str, m_pExpectedStringlist->at(i));
    }
}


class tst_QStringListModel : public QObject
{
    Q_OBJECT

public:

    tst_QStringListModel();
    virtual ~tst_QStringListModel();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:

    void rowsAboutToBeRemoved_rowsRemoved();
    void rowsAboutToBeRemoved_rowsRemoved_data();

    void rowsAboutToBeInserted_rowsInserted();
    void rowsAboutToBeInserted_rowsInserted_data();
};


tst_QStringListModel::tst_QStringListModel()

{
}

tst_QStringListModel::~tst_QStringListModel()
{
}

void tst_QStringListModel::initTestCase()
{
}

void tst_QStringListModel::cleanupTestCase()
{
}

void tst_QStringListModel::init()
{
}

void tst_QStringListModel::cleanup()
{
}

/*
  tests
*/


void tst_QStringListModel::rowsAboutToBeRemoved_rowsRemoved_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("count");
    QTest::addColumn<QStringList>("aboutto");
    QTest::addColumn<QStringList>("res");

    QStringList strings0;   strings0    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto0;   aboutto0    << "Two" << "Three";
    QStringList res0;       res0        << "One" << "Four" << "Five";
    QTest::newRow( "data0" )   << strings0 << 1 << 2 << aboutto0 << res0;

    QStringList strings1;   strings1    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto1;   aboutto1    << "One" << "Two";
    QStringList res1;       res1        << "Three" << "Four" << "Five";
    QTest::newRow( "data1" )   << strings1 << 0 << 2 << aboutto1 << res1;

    QStringList strings2;   strings2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto2;   aboutto2    << "Four" << "Five";
    QStringList res2;       res2        << "One" << "Two" << "Three";
    QTest::newRow( "data2" )   << strings2 << 3 << 2 << aboutto2 << res2;

    QStringList strings3;   strings3    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto3;   aboutto3    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList res3;       res3        ;
    QTest::newRow( "data3" )   << strings3 << 0 << 5 << aboutto3 << res3;

    /* Not sure if this is a valid test */
    QStringList strings4;   strings4    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto4;   aboutto4    << "Five" << "";
    QStringList res4;       res4        << "One" << "Two" << "Three" << "Four";
    QTest::newRow( "data4" )   << strings4 << 4 << 2 << aboutto4 << res4;

    /*
     * Keep this, template to add more data
    QStringList strings2;   strings2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto2;   aboutto2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList res2;       res2        << "One" << "Two" << "Three" << "Four" << "Five";
    QTest::newRow( "data2" )   << strings2 << 0 << 5 << aboutto2 << res2;
*/

}

void tst_QStringListModel::rowsAboutToBeRemoved_rowsRemoved()
{
    QFETCH(QStringList, input);
    QFETCH(int, row);
    QFETCH(int, count);
    QFETCH(QStringList, aboutto);
    QFETCH(QStringList, res);

    QStringListModel *model = new QStringListModel(input);
    QModelListener *pListener = new QModelListener(&aboutto, &res, model);
    pListener->connect(model,       SIGNAL( rowsAboutToBeRemoved(const QModelIndex & , int , int )), 
                       pListener,   SLOT(   rowsAboutToBeRemovedOrInserted(const QModelIndex & , int , int ))    );

    pListener->connect(model,       SIGNAL( rowsRemoved(const QModelIndex & , int , int )), 
                       pListener,   SLOT(   rowsRemovedOrInserted(const QModelIndex & , int , int ))    );

    model->removeRows(row,count);
    // At this point, control goes to our connected slots inn this order: 
    // 1. rowsAboutToBeRemovedOrInserted
    // 2. rowsRemovedOrInserted
    // Control returns here

    delete pListener;
    delete model;

}

void tst_QStringListModel::rowsAboutToBeInserted_rowsInserted_data()
{
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<int>("row");
    QTest::addColumn<int>("count");
    QTest::addColumn<QStringList>("aboutto");
    QTest::addColumn<QStringList>("res");

    QStringList strings0;   strings0    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto0;   aboutto0    << "Two" << "Three";
    QStringList res0;       res0        << "One" << "" << "" << "Two" << "Three" << "Four" << "Five";
    QTest::newRow( "data0" )   << strings0 << 1 << 2 << aboutto0 << res0;

    QStringList strings1;   strings1    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto1;   aboutto1    << "One" << "Two";
    QStringList res1;       res1        << "" << "" << "One" << "Two" << "Three" << "Four" << "Five";
    QTest::newRow( "data1" )   << strings1 << 0 << 2 << aboutto1 << res1;

    QStringList strings2;   strings2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto2;   aboutto2    << "Four" << "Five";
    QStringList res2;       res2        << "One" << "Two" << "Three" << "" << "" << "Four" << "Five";
    QTest::newRow( "data2" )   << strings2 << 3 << 2 << aboutto2 << res2;

    QStringList strings3;   strings3    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto3;   aboutto3    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList res3;       res3        << "" << "" << "" << "" << "" << "One" << "Two" << "Three" << "Four" << "Five";
    QTest::newRow( "data3" )   << strings3 << 0 << 5 << aboutto3 << res3;

    /*
     * Keep this, template to add more data
    QStringList strings2;   strings2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList aboutto2;   aboutto2    << "One" << "Two" << "Three" << "Four" << "Five";
    QStringList res2;       res2        << "One" << "Two" << "Three" << "Four" << "Five";
    QTest::newRow( "data2" )   << strings2 << 0 << 5 << aboutto2 << res2;
*/

}

void tst_QStringListModel::rowsAboutToBeInserted_rowsInserted()
{
    QFETCH(QStringList, input);
    QFETCH(int, row);
    QFETCH(int, count);
    QFETCH(QStringList, aboutto);
    QFETCH(QStringList, res);

    QStringListModel *model = new QStringListModel(input);
    QModelListener *pListener = new QModelListener(&aboutto, &res, model);
    connect(model,       SIGNAL( rowsAboutToBeInserted(const QModelIndex & , int , int )), 
                       pListener,   SLOT(   rowsAboutToBeRemovedOrInserted(const QModelIndex & , int , int ))    );

    connect(model,       SIGNAL( rowsInserted(const QModelIndex & , int , int )), 
                       pListener,   SLOT(   rowsRemovedOrInserted(const QModelIndex & , int , int ))    );

    model->insertRows(row,count);
    // At this point, control goes to our connected slots inn this order: 
    // 1. rowsAboutToBeRemovedOrInserted
    // 2. rowsRemovedOrInserted
    // Control returns here

    delete pListener;
    delete model;

}


QTEST_MAIN(tst_QStringListModel)
#include "tst_qstringlistmodel.moc"

