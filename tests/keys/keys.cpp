//
// Qt Example Application: keys
//
// Demonstrates Qt key events.
//

#include <qapplication.h>
#include <qfile.h>
#include <qaccel.h>
#include <qmultilineedit.h>

class Main : public QWidget {
public:
    Main(QWidget* parent=0) :
	QWidget(parent),
	log(this)
    {
	log.setReadOnly(TRUE);
	//log.setFont(QFont("courier",10));
	dump(0,(QKeyEvent*)0);
    }

    void resizeEvent(QResizeEvent*)
    {
	qDebug("Main got resized");
	log.setGeometry(0,0,width()/2,height());
    }

    void keyPressEvent(QKeyEvent* e)
    {
	dump("Kv", e);
    }

    void keyReleaseEvent(QKeyEvent* e)
    {
	dump("K^", e);
    }

    void mousePressEvent(QMouseEvent* e)
    {
	dump("Mv", e);
    }

    void mouseDoubleClickEvent(QMouseEvent* e)
    {
	dump("Mv2", e);
	if ( parentWidget() ) hide();
    }

    void mouseReleaseEvent(QMouseEvent* e)
    {
	dump("M^", e);
    }

    void dump(const char* type, QKeyEvent* e)
    {
	QString line;
	if (!type)
	    line.sprintf("%2s %6s %3s    %3s %4s %3s %s", "", "key", "asc", "sta", "uni", "rep", "name");
	else {
	    line.sprintf("%2s %6x %3x(%c) %3x %02x%02x%3s %s ", type, e->key(), e->ascii(),
		(e->ascii() ? e->ascii() : ' '),
		e->state(), e->text()[0].row(), e->text()[0].cell(),
		(e->isAutoRepeat() ? "Y" : ""),
		QAccel::keyToString(e->key()).ascii()
		);
	    line += e->text();
	}
	log.insertLine( line );
	log.setCursorPosition(999999,0);
    }

    void dump(const char* type, QMouseEvent* e)
    {
	QString line;
	line.sprintf("%2s %6s %3s %c  %3x", type, "", "",
		' ', e->state());
	log.insertLine( line );
	log.setCursorPosition(999999,0);
    }

    void text(const char* t)
    {
	log.insertLine( t );
	log.setCursorPosition(999999,0);
    }

private:
    QMultiLineEdit log;
};

Main *m;

void myout( QtMsgType type, const char *msg )
{
    m->text(msg);
#if 0
    static QFile* f = 0;
    if ( !f ) {
	f = new QFile("out.log");
	f->open(IO_WriteOnly);
    }
    f->writeBlock(msg,strlen(msg));
#endif
}


int main( int argc, char **argv )
{
#define T(x) debug(#x " = %d",sizeof(x));
T(QObject)
T(QWidget)
T(QFont)
T(QPalette)
T(QCursor)
T(QRect)
T(QColor)
T(WId)
T(QPaintDevice)
T(QWExtra)
#undef T
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );
    //QApplication::setFont( QFont("Helvetica") );
    QFont f("Times New Roman (Cyrillic)");
    QApplication::setFont(f);

    #define T(x) ASSERT(QAccel::keyToString(QAccel::stringToKey(x))==x)
    T("A");
    T("Ctrl+D");
    T("Ctrl++");
    T("Ctrl+Backspace");
    T("Ctrl+Del");
    T("Ctrl+F1");
    T("F1");
    T("F10");
    T("Home");
    T("e");

    m = new Main;
    qInstallMsgHandler(myout);
    m->setCaption("Test");
    a.setMainWidget( m );
    m->show();
    return a.exec();
}
