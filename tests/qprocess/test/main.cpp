#include <iostream.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include "some.h"
#include "guicat.h"

int main( int argc, char **argv )
{
    if ( argc > 1 ) {
	if ( QString( "-cat" ) == argv[1] ) {
	    char ch;
	    while( !cin.eof() )
	    {
		cin >> ch;
		cout << ch;
		cout.flush();
		cerr << (char)(ch +1);
		cerr.flush();
	    }
	    return ch;
	} else if ( QString( "-guicat" ) == argv[1] ) {
	    QApplication a( argc, argv );
	    GuiCat gc;
	    a.setMainWidget( &gc );
	    gc.show();
	    return a.exec();
	}
    } else {
	QApplication a( argc, argv );
	SomeFactory factory;
	QVBox vb;

	QPushButton *newProcess;
	// start process
	newProcess = new QPushButton( "Start Process (cat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess0()) );
	newProcess = new QPushButton( "Start Process (guicat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess1()) );
	newProcess = new QPushButton( "Start Process (p4)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess2()) );
	// launch process
	newProcess = new QPushButton( "Launch Process (cat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess0()) );
	newProcess = new QPushButton( "Launch Process (guicat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess1()) );
	newProcess = new QPushButton( "Launch Process (p4)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess2()) );

	QPushButton *quit = new QPushButton ( "Quit", &vb );
	QObject::connect( quit, SIGNAL(clicked()),
		&factory, SLOT(quit()) );
	QObject::connect( &factory, SIGNAL(quitted()),
		&a, SLOT(quit()) );

	QCheckBox *cb;
	cb = new QCheckBox( "Stdout", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectStdout(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Stderr", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectStderr(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Exit Notify", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectExit(bool)) );
	cb->setChecked( TRUE );

	a.setMainWidget( &vb );
	vb.show();
	return a.exec();
    }
    return 0;
}
