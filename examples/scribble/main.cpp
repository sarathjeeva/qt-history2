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

#include "scribble.h"
#include <qapplication.h>
#include <qdesktopwidget.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Scribble scribble;

    scribble.resize( 500, 350 );
    scribble.setWindowTitle("Qt Example - Scribble");
    a.setMainWidget( &scribble );
    if ( QApplication::desktop()->width() > 550
	 && QApplication::desktop()->height() > 366 )
	scribble.show();
    else
	scribble.showMaximized();
    return a.exec();
}
