/****************************************************************************
** $Id: //depot/qt/main/examples/table/main.cpp#2 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qtable.h>
#include <qapplication.h>
#include <qwmatrix.h>

// Qt logo

static const char *qtlogo_xpm[] = {
    "45 36 13 1",
    "  c #000000",
    ". c #999999",
    "X c #333366",
    "o c #6666CC",
    "O c #333333",
    "@ c #666699",
    "# c #000066",
    "$ c #666666",
    "% c #3333CC",
    "& c #000033",
    "* c #9999CC",
    "= c #333399",
    "+ c None",
    "+++++++++++++++++++++++++++++++++++++++++++++",
    "+++++++++++++++.$OOO$.+++++++++++++++++++++++",
    "+++++++++++++$         O.++++++++++++++++++++",
    "+++++++++++.O            $+++++++++++++++++++",
    "++++++++++.    $.++.$     O++++++++++++++++++",
    "+++++++++.   O.+++++++$    O+++++++++++++++++",
    "+++++++++O   ++++++++++$    $++++++++++++++++",
    "++++++++$   .+++++++++++O    .+++++++++++++++",
    "+++++++.   O+++++++++++++    O++++++.++++++++",
    "+++++++$   .+++++++++++++$    .+++.O ++++++++",
    "+++++++    +++++++++++++++    O+++.  ++++++++",
    "++++++.  &Xoooo*++++++++++$    +++.  ++++++++",
    "++++++@=%%%%%%%%%%*+++++++.    .++.  ++++++++",
    "+++**oooooo**++*o%%%%o+++++    $++O  ++++++++",
    "+*****$OOX@oooo*++*%%%%%*++O   $+.   OOO$++++",
    "+.++....$O$+*ooooo*+*o%%%%%O   O+$   $$O.++++",
    "*+++++$$....+++*oooo**+*o%%#   O++O  ++++++**",
    "++++++O  $.....++**oooo**+*X   &o*O  ++++*ooo",
    "++++++$   O++.....++**oooo*X   &%%&  ..*o%%*+",
    "++++++$    ++++.....+++**ooO   $*o&  @oo*++++",
    "++++++.    .++++++.....+++*O   Xo*O  .+++++++",
    "+++++++    O+++++++++......    .++O  ++++++++",
    "+++++++O    +++.$$$.++++++.   O+++O  ++++++++",
    "+++++++.    $$OO    O.++++O   .+++O  ++++++++",
    "++++++++O    .+++.O   $++.   O++++O  ++++++++",
    "++++++++.    O+++++O   $+O   +++++O  ++++++++",
    "+++++++++.    O+++++O   O   .+++++O  .+++++++",
    "++++++++++$     .++++O     .++++.+$  O+.$.+++",
    "+++++++++++.      O$$O    .+++++...      ++++",
    "+++++++++++++$            O+++++$$+.O O$.++++",
    "+++++++++++++++$OO  O$.O   O.++. .+++++++++++",
    "+++++++++++++++++++++++.     OO  .+++++++++++",
    "++++++++++++++++++++++++.       O++++++++++++",
    "+++++++++++++++++++++++++.      .++++++++++++",
    "++++++++++++++++++++++++++.O  O.+++++++++++++",
    "+++++++++++++++++++++++++++++++++++++++++++++"
};


// Constants

const int numRows = 100;				// Tablesize: number of rows
const int numCols = 100;				// Tablesize: number of columns

// The program starts here.

int main( int argc, char **argv )
{
    QApplication a(argc,argv);			

    QTable v( numRows, numCols );
    QPixmap pix( qtlogo_xpm );

    float factor = (float) v.rowHeight( 3 ) / pix.height();
    
    QWMatrix wm;
    wm.scale( factor, factor );
    pix = pix.xForm( wm );

    v.setPixmap( 3, 3, pix );
    v.setText( 3, 3, "A Pixmap" );

    QStringList l;
    l << "one" << "two" << "three" << "four";

    for ( int i = 0; i < numRows; ++i )
	v.setItem( i, 5, new QComboTableItem( &v, l, TRUE ) );
    for ( int j = 0; j < numRows; ++j )
	v.setItem( j, 1, new QCheckTableItem( &v, "Check me" ) );

    a.setMainWidget( &v );
    v.show();
    return a.exec();
}
