#define QT3_SUPPORT
#include <QtTest/QtTest>
#include <QMessageBox>
#include <QDebug>
#include <QPair>
#include <QList>
#include <QPointer>
#include <QTimer>
#include <QApplication>
#include <QPushButton>

#define CONVENIENCE_FUNC_SYMS(func) \
    { \
        int x1 = QMessageBox::func(0, "Foo", "Bar"); \
        int x3 = QMessageBox::func(0, "Foo", "Bar", "Save"); \
        int x6 = QMessageBox::func(0, "Foo", "Bar", "Save", "Save As"); \
        int x7 = QMessageBox::func(0, "Foo", "Bar", "Save", "Save As", "Dont Save"); \
        int x8 = QMessageBox::func(0, "Foo", "Bar", "Save", "Save As", "Dont Save", 1); \
        int x9 = QMessageBox::func(0, "Foo", "Bar", "Save", "Save As", "Dont Save", 1, 2); \
        int x10 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::YesAll, QMessageBox::Yes); \
        int x11 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::YesAll, QMessageBox::Yes, \
                                    QMessageBox::No); \
        qDebug("%d %d %d %d %d %d %d %d", x1, x3, x6, x7, x8, x9, x10, x11); \
        { \
        int x4 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, (int)QMessageBox::No); \
        int x5 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, (int)QMessageBox::No); \
        int x6 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, (int)QMessageBox::No); \
        int x7 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, QMessageBox::No); \
        int x8 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, QMessageBox::No); \
        int x9 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No); \
        int x10 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, (int)QMessageBox::No, (int)QMessageBox::Ok); \
        int x11 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, (int)QMessageBox::No, (int)QMessageBox::Ok); \
        int x12 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, (int)QMessageBox::No, (int)QMessageBox::Ok); \
        int x13 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, QMessageBox::No, (int)QMessageBox::Ok); \
        int x14 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, QMessageBox::No, (int)QMessageBox::Ok); \
        int x15 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, (int)QMessageBox::Ok); \
        int x16 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, (int)QMessageBox::No, QMessageBox::Ok); \
        int x17 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, (int)QMessageBox::No, QMessageBox::Ok); \
        int x18 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, (int)QMessageBox::No, QMessageBox::Ok); \
        int x19 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes, QMessageBox::No, QMessageBox::Ok); \
        int x20 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes, QMessageBox::No, QMessageBox::Ok); \
        int x21 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Ok); \
        qDebug("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21); \
        } \
    }

#define CONVENIENCE_FUNC_SYMS_EXTRA(func) \
    { \
        int x1 = QMessageBox::func(0, "Foo", "Bar", (int)QMessageBox::Yes); \
        int x2 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes); \
        int x3 = QMessageBox::func(0, "Foo", "Bar", QMessageBox::Yes | QMessageBox::Default); \
        qDebug("%d %d %d", x1, x2, x3); \
    }

class tst_QMessageBox : public QObject
{
    Q_OBJECT
public:
    tst_QMessageBox();
    int exec(QMessageBox *msgBox, int key = -1);
    int sendReturn();

public slots:
    void sendKey();

private slots:
    void sanityTest();
    void defaultButton();
    void escapeButton();
    void button();
    void statics();
    void about();
    void detailsText();

    void shortcut();

    void staticSourceCompat();
    void staticBinaryCompat();
    void instanceSourceCompat();
    void instanceBinaryCompat();

    void testSymbols();
private:
    int keyToSend;
};

tst_QMessageBox::tst_QMessageBox() : keyToSend(-1)
{
}

int tst_QMessageBox::exec(QMessageBox *msgBox, int key)
{
    if (key == -1) {
        QTimer::singleShot(1000, msgBox, SLOT(close()));
    } else {
        keyToSend = key;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
    }
    return msgBox->exec();
}

int tst_QMessageBox::sendReturn()
{
    keyToSend = Qt::Key_Return;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    return 0;
}

void tst_QMessageBox::sendKey()
{
    if (keyToSend == -2) {
        QApplication::activeModalWidget()->close();
        return;
    }
    if (keyToSend == -1)
        return;
    QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, keyToSend, Qt::NoModifier);
    qApp->postEvent(QApplication::activeModalWidget(), ke);
    keyToSend = -1;
}

void tst_QMessageBox::sanityTest()
{
    QMessageBox msgBox;
    msgBox.setText("This is insane");
    for (int i = 0; i < 10; i++)
        msgBox.setIcon(QMessageBox::Icon(i));
    msgBox.setIconPixmap(QPixmap());
    msgBox.setIconPixmap(QPixmap("whatever.png"));
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setTextFormat(Qt::PlainText);
    exec(&msgBox);
}

void tst_QMessageBox::button()
{
    QMessageBox msgBox;
    msgBox.addButton("retry", QMessageBox::DestructiveRole);
    QVERIFY(msgBox.button(QMessageBox::Ok) == 0); // not added yet
    QPushButton *b1 = msgBox.addButton(QMessageBox::Ok);
    QCOMPARE(msgBox.button(QMessageBox::Ok), b1);  // just added
    QCOMPARE(msgBox.standardButton(b1), QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    QCOMPARE(msgBox.standardButtons(), QMessageBox::Ok | QMessageBox::Cancel);

    // remove the cancel, should not exist anymore
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QVERIFY(msgBox.button(QMessageBox::Cancel) == 0);
    QVERIFY(msgBox.button(QMessageBox::Yes) != 0);

    // should not crash
    QPushButton *b4 = new QPushButton;
    msgBox.addButton(b4, QMessageBox::DestructiveRole);
    msgBox.addButton(0, QMessageBox::ActionRole);
}

void tst_QMessageBox::defaultButton()
{
    QMessageBox msgBox;
    QVERIFY(msgBox.defaultButton() == 0);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.addButton(QMessageBox::Cancel);
    QVERIFY(msgBox.defaultButton() == 0);
    QPushButton pushButton;
    msgBox.setDefaultButton(&pushButton);
    QVERIFY(msgBox.defaultButton() == 0); // we have not added it yet
    QPushButton *retryButton = msgBox.addButton(QMessageBox::Retry);
    msgBox.setDefaultButton(retryButton);
    QCOMPARE(msgBox.defaultButton(), retryButton);
    exec(&msgBox);
    QCOMPARE(msgBox.clickedButton(), msgBox.button(QMessageBox::Cancel));

    exec(&msgBox, Qt::Key_Enter);
    QCOMPARE(msgBox.clickedButton(), retryButton);
}

void tst_QMessageBox::escapeButton()
{
    QMessageBox msgBox;
    QVERIFY(msgBox.escapeButton() == 0);
    msgBox.addButton(QMessageBox::Ok);
    exec(&msgBox);
    QVERIFY(msgBox.clickedButton() == msgBox.button(QMessageBox::Ok)); // auto detected (one button only)
    msgBox.addButton(QMessageBox::Cancel);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton invalidButton;
    msgBox.setEscapeButton(&invalidButton);
    QVERIFY(msgBox.escapeButton() == 0);
    QPushButton *retryButton = msgBox.addButton(QMessageBox::Retry);

    exec(&msgBox);
    QVERIFY(msgBox.clickedButton() == msgBox.button(QMessageBox::Cancel)); // auto detected (cancel)

    msgBox.setEscapeButton(retryButton);
    QCOMPARE(msgBox.escapeButton(), retryButton);

    // with escape
    exec(&msgBox, Qt::Key_Escape);
    QCOMPARE(msgBox.clickedButton(), retryButton);

    // with close
    exec(&msgBox);
    QCOMPARE(msgBox.clickedButton(), retryButton);
}

void tst_QMessageBox::statics()
{
    QMessageBox::StandardButton (*statics[4])(QWidget *, const QString &,
         const QString&, QMessageBox::StandardButtons buttons,
         QMessageBox::StandardButton);

    statics[0] = QMessageBox::information;
    statics[1] = QMessageBox::critical;
    statics[2] = QMessageBox::question;
    statics[3] = QMessageBox::warning;

    for (int i = 0; i < 4; i++) {
        keyToSend = Qt::Key_Escape;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        QMessageBox::StandardButton sb = (*statics[i])(0, "caption",
            "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help | QMessageBox::Cancel,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Cancel);

        keyToSend = -2; // close()
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption",
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help | QMessageBox::Cancel,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Cancel);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption",
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
           QMessageBox::NoButton);
        QCOMPARE(sb, QMessageBox::Yes);

        keyToSend = Qt::Key_Enter;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        sb = (*statics[i])(0, "caption",
           "text", QMessageBox::Yes | QMessageBox::No | QMessageBox::Help,
            QMessageBox::No);
        QCOMPARE(sb, QMessageBox::No);
    }
}

void tst_QMessageBox::shortcut()
{
#ifdef Q_WS_MAC
    QSKIP("shortcuts are not used on MAC OS X", SkipAll);
#endif
    QMessageBox msgBox;
    msgBox.addButton("O&k", QMessageBox::YesRole);
    msgBox.addButton("&No", QMessageBox::YesRole);
    msgBox.addButton("&Maybe", QMessageBox::YesRole);
    QCOMPARE(exec(&msgBox, Qt::Key_M), 2);
}

void tst_QMessageBox::about()
{
    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBox::about(0, "Caption", "This is an auto test");

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    QMessageBox::aboutQt(0, "Caption");
}

// Old message box enums
const int Old_Ok = 1;
const int Old_Cancel = 2;
const int Old_Yes = 3;
const int Old_No = 4;
const int Old_Abort = 5;
const int Old_Retry = 6;
const int Old_Ignore = 7;
const int Old_YesAll = 8;
const int Old_NoAll = 9;
const int Old_Default = 0x100;
const int Old_Escape = 0x200;

void tst_QMessageBox::staticSourceCompat()
{
    int ret;

    // source compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes, QMessageBox::No);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    QCOMPARE(ret, int(QMessageBox::No));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape);
    QCOMPARE(ret, int(QMessageBox::Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", QMessageBox::Yes | QMessageBox::Escape, QMessageBox::No | QMessageBox::Default);
    QCOMPARE(ret, int(QMessageBox::No));

    // the button text versions
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", "Yes", "No", QString(), 1);
    QCOMPARE(ret, 1);

    if (0) { // dont run these tests since the dialog wont close!
        keyToSend = Qt::Key_Escape;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        ret = QMessageBox::information(0, "title", "text", "Yes", "No", QString(), 1);
        QCOMPARE(ret, -1);

        keyToSend = Qt::Key_Escape;
        QTimer::singleShot(1000, this, SLOT(sendKey()));
        ret = QMessageBox::information(0, "title", "text", "Yes", "No", QString(), 0, 1);
        QCOMPARE(ret, 1);
    }
}

void tst_QMessageBox::instanceSourceCompat()
{
     QMessageBox mb("Application name here",
                    "Saving the file will overwrite the original file on the disk.\n"
                    "Do you really want to save?",
                    QMessageBox::Information,
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape);
    mb.setButtonText(QMessageBox::Yes, "Save");
    mb.setButtonText(QMessageBox::No, "Discard");
    mb.addButton("&Revert", QMessageBox::RejectRole);
    mb.addButton("&Zoo", QMessageBox::ActionRole);

    QCOMPARE(exec(&mb, Qt::Key_Enter), int(QMessageBox::Yes));
    QCOMPARE(exec(&mb, Qt::Key_Escape), int(QMessageBox::Cancel));
#ifdef Q_WS_MAC
    QSKIP("mnemonics are not used on the MAC OS X", SkipAll);
#endif
    QCOMPARE(exec(&mb, Qt::ALT + Qt::Key_R), 0);
    QCOMPARE(exec(&mb, Qt::ALT + Qt::Key_Z), 1);
}

void tst_QMessageBox::staticBinaryCompat()
{
    int ret;

    // binary compat tests for < 4.2
    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Escape, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

    keyToSend = Qt::Key_Enter;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Default, Old_No, 0);
    QCOMPARE(ret, int(Old_Yes));

#if 0
    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes, Old_No | Old_Default, 0);
    QCOMPARE(ret, -1);
#endif

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Escape, Old_No | Old_Default, 0);
    QCOMPARE(ret, Old_Yes);

    keyToSend = Qt::Key_Escape;
    QTimer::singleShot(1000, this, SLOT(sendKey()));
    ret = QMessageBox::information(0, "title", "text", Old_Yes | Old_Default, Old_No | Old_Escape, 0);
    QCOMPARE(ret, Old_No);

}

void tst_QMessageBox::instanceBinaryCompat()
{
     QMessageBox mb("Application name here",
                    "Saving the file will overwrite the original file on the disk.\n"
                    "Do you really want to save?",
                    QMessageBox::Information,
                    Old_Yes | Old_Default,
                    Old_No,
                    Old_Cancel | Old_Escape);
    mb.setButtonText(Old_Yes, "Save");
    mb.setButtonText(Old_No, "Discard");
    QCOMPARE(exec(&mb, Qt::Key_Enter), int(Old_Yes));
    QCOMPARE(exec(&mb, Qt::Key_Escape), int(Old_Cancel));
}

void tst_QMessageBox::testSymbols()
{
    return;

    QMessageBox::Icon icon;
    icon = QMessageBox::NoIcon;
    icon = QMessageBox::Information;
    icon = QMessageBox::Warning;
    icon = QMessageBox::Critical;
    icon = QMessageBox::Question;

    QMessageBox mb1;
    QMessageBox mb2(0);
    QMessageBox mb3(&mb1);
    QMessageBox mb3b("title", "text", QMessageBox::Critical, int(QMessageBox::Yes),
                     int(QMessageBox::No), int(QMessageBox::Cancel), &mb1, Qt::Dialog);

    QMessageBox::Button button = QMessageBox::NoButton;
    button = QMessageBox::Ok;
    button = QMessageBox::Cancel;
    button = QMessageBox::Yes;
    button = QMessageBox::No;
    button = QMessageBox::Abort;
    button = QMessageBox::Retry;
    button = QMessageBox::Ignore;
    button = QMessageBox::YesAll;
    button = QMessageBox::NoAll;
    button = QMessageBox::ButtonMask;
    button = QMessageBox::Default;
    button = QMessageBox::Escape;
    button = QMessageBox::FlagMask;

    mb1.setText("Foo");
    QString text = mb1.text();
    Q_ASSERT(text == "Foo");

    icon = mb1.icon();
    Q_ASSERT(icon == QMessageBox::NoIcon);
    mb1.setIcon(QMessageBox::Question);
    Q_ASSERT(mb1.icon() == QMessageBox::Question);

    QPixmap iconPixmap = mb1.iconPixmap();
    mb1.setIconPixmap(iconPixmap);
    Q_ASSERT(mb1.icon() == QMessageBox::NoIcon);

    QString bt0 = mb1.buttonText(QMessageBox::Ok);
    QString bt1 = mb1.buttonText(QMessageBox::Cancel);
    QString bt2 = mb1.buttonText(QMessageBox::Ok | QMessageBox::Default);

    Q_ASSERT(bt0 == "OK");
    Q_ASSERT(bt1.isEmpty());
    Q_ASSERT(bt2.isEmpty());

    mb2.setButtonText(QMessageBox::Cancel, "Foo");
    mb2.setButtonText(QMessageBox::Ok, "Bar");
    mb2.setButtonText(QMessageBox::Ok | QMessageBox::Default, "Baz");

    Q_ASSERT(mb2.buttonText(QMessageBox::Cancel).isEmpty());
    Q_ASSERT(mb2.buttonText(QMessageBox::Ok) == "Bar");

    Q_ASSERT(mb3b.buttonText(QMessageBox::Yes).endsWith("Yes"));
    Q_ASSERT(mb3b.buttonText(QMessageBox::YesAll).isEmpty());
    Q_ASSERT(mb3b.buttonText(QMessageBox::Ok).isEmpty());

    mb3b.setButtonText(QMessageBox::Yes, "Blah");
    mb3b.setButtonText(QMessageBox::YesAll, "Zoo");
    mb3b.setButtonText(QMessageBox::Ok, "Zoo");

    Q_ASSERT(mb3b.buttonText(QMessageBox::Yes) == "Blah");
    Q_ASSERT(mb3b.buttonText(QMessageBox::YesAll).isEmpty());
    Q_ASSERT(mb3b.buttonText(QMessageBox::Ok).isEmpty());

    Q_ASSERT(mb1.textFormat() == Qt::AutoText);
    mb1.setTextFormat(Qt::PlainText);
    Q_ASSERT(mb1.textFormat() == Qt::PlainText);

    CONVENIENCE_FUNC_SYMS(information);
    CONVENIENCE_FUNC_SYMS_EXTRA(information);
    CONVENIENCE_FUNC_SYMS(question);
    CONVENIENCE_FUNC_SYMS_EXTRA(question);
    CONVENIENCE_FUNC_SYMS(warning);
    CONVENIENCE_FUNC_SYMS(critical);

    QSize sizeHint = mb1.sizeHint();
    Q_ASSERT(sizeHint.width() > 20 && sizeHint.height() > 20);

    // test QT3_SUPPORT stuff

    QMessageBox mb4("title", "text", icon, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default,
                    QMessageBox::Cancel, &mb1, "name", true, Qt::Dialog);
    QMessageBox mb5(&mb1, "name");

    QPixmap pm = QMessageBox::standardIcon(QMessageBox::Question, Qt::GUIStyle(1));
    QPixmap pm2 = QMessageBox::standardIcon(QMessageBox::Question);

    Q_ASSERT(pm.toImage() == iconPixmap.toImage());
    Q_ASSERT(pm2.toImage() == iconPixmap.toImage());

    int ret1 = QMessageBox::message("title", "text");
    int ret2 = QMessageBox::message("title", "text", "OK");
    int ret3 = QMessageBox::message("title", "text", "OK", &mb1);
    int ret4 = QMessageBox::message("title", "text", "OK", &mb1, "name");
    qDebug("%d %d %d %d", ret1, ret2, ret3, ret4);

    bool ret5 = QMessageBox::query("title", "text");
    bool ret6 = QMessageBox::query("title", "text", "Ja");
    bool ret7 = QMessageBox::query("title", "text", "Ja", "Nein");
    bool ret8 = QMessageBox::query("title", "text", "Ja", "Nein", &mb1);
    bool ret9 = QMessageBox::query("title", "text", "Ja", "Nein", &mb1, "name");
    qDebug("%d %d %d %d %d", ret5, ret6, ret7, ret8, ret9);

    Q_UNUSED(ret1);
    Q_UNUSED(ret5);

    QPixmap pm3 = QMessageBox::standardIcon(QMessageBox::NoIcon);
    Q_ASSERT(pm3.isNull());

    pm3 = QMessageBox::standardIcon(QMessageBox::Information);
    Q_ASSERT(!pm3.isNull());

    QMessageBox::about(&mb1, "title", "text");
    QMessageBox::aboutQt(&mb1);
    QMessageBox::aboutQt(&mb1, "title");
}

void tst_QMessageBox::detailsText()
{
    QMessageBox box;
    QString text("This is the details text.");
    box.setDetailedText(text);
    QCOMPARE(box.detailedText(), text);
}

QTEST_MAIN(tst_QMessageBox)
#include "tst_qmessagebox.moc"
