/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "tabdialog.h"
#include <qapplication.h>
#include <qstring.h>

int main( int argc, char **argv )
{

    QApplication a( argc, argv );

    TabDialog tabdialog( 0, "tabdialog", QString( argc < 2 ? "." : argv[1] ) );
    tabdialog.resize( 450, 350 );
    tabdialog.setWindowTitle( "Qt Example - Tabbed Dialog" );
    a.setMainWidget( &tabdialog );
    tabdialog.show();

    return a.exec();
}
