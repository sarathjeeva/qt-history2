#include <qapplication.h>
#include <qsqldatabase.h>
#include "resultwindow.h"

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QSqlConnection::addDatabase( qApp->argv()[1],
				 qApp->argv()[2],
				 qApp->argv()[3],
				 qApp->argv()[4],
				 qApp->argv()[5]);
    ResultWindow* rw = new ResultWindow();
    qDebug("After creating ResultWindow");
    a.setMainWidget( rw );
    rw->show();
    int x = a.exec();
    delete rw;
    qDebug("ending program");
    return x;
};

