#include <qapplication.h>
#include "mainwindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainWindow *w = new MainWindow;
    a.setMainWidget( w );
    w->show();
    return a.exec();
}
