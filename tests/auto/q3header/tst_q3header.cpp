/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include "q3header.h"

//TESTED_CLASS=
//TESTED_FILES=compat/widgets/q3header.h compat/widgets/q3header.cpp

class tst_Q3Header : public QObject
{
    Q_OBJECT
public:
    tst_Q3Header();


public slots:
    void initTestCase();
    void cleanupTestCase();
private slots:
    void bug_setOffset();
#if QT_VERSION >= 0x040101
    void nullStringLabel();
#endif

private:
    Q3Header *testW;
};

tst_Q3Header::tst_Q3Header()

{
}

void tst_Q3Header::initTestCase()
{
    // Create the test class
    testW = new Q3Header( 0, "testObject" );
}

void tst_Q3Header::cleanupTestCase()
{
    delete testW;
}

/*!  info/arc-15/30171 described a bug in setOffset(). Supposedly
  fixed in change 59949. Up to Qt 3.0.2 the horizontal size was used
  to determine whether scrolling was possible at all.  Could be merged
  into a general setOffset() test that goes through several variations
  of sizes and orientation.
*/
void tst_Q3Header::bug_setOffset()
{
    // create a vertical header which is wider than high.
    testW->setOrientation( Qt::Vertical );
    testW->addLabel( "111111111111111111111111111111111111" );
    testW->addLabel( "222222222222222222222222222222222222" );
    testW->addLabel( "333333333333333333333333333333333333" );
    testW->addLabel( "444444444444444444444444444444444444" );
    testW->setFixedSize( testW->headerWidth() * 2, testW->headerWidth() / 2 );

    // we'll try to scroll down a little bit
    int offs = testW->sectionSize( 0 ) / 2;
    testW->setOffset( offs );

    // and check whether we succeeded. In case the method used width()
    // for the visible header length offset() would be 0.
    QCOMPARE( testW->offset(), offs );
}

#if QT_VERSION >= 0x040101
// Task 95640
void tst_Q3Header::nullStringLabel()
{
    QString oldLabel = testW->label(0);
    testW->setLabel(0, QString());
    QCOMPARE(testW->label(0), QString());
    testW->setLabel(0, oldLabel);
    QCOMPARE(testW->label(0), oldLabel);
    QCOMPARE(testW->label(testW->addLabel(QString())), QString());
    QCOMPARE(testW->label(testW->addLabel(QString("Foo"))), QString("Foo"));
    testW->removeLabel(testW->count()-1);
    QCOMPARE(testW->label(testW->addLabel(QString())), QString());
    QCOMPARE(testW->label(testW->addLabel(QString(""))), QString(""));
}
#endif

QTEST_MAIN(tst_Q3Header)
#include "tst_q3header.moc"

