//
// Qt Tutorial 2
//
//

#include <qapp.h>
#include <qpushbt.h>
#include <qfont.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QPushButton quit( "Quit" );
    quit.resize( 75, 30 );
    quit.setFont( QFont( "Times", 18, QFont::Bold ) );

    QObject::connect( &quit, SIGNAL(clicked()), &a, SLOT(quitApp()) );

    a.setMainWidget( &quit );
    quit.show();
    return a.exec();
}
