/****************************************************************************
** $Id: //depot/qt/main/examples/tooltip/main.cpp#1 $
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "tooltip.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    TellMe mw;
    a.setMainWidget( &mw );
    mw.setCaption( "Dynamic Tool Tip Demonstration" );
    mw.show();
    return a.exec();
}
