/****************************************************************************
** $Id: //depot/qt/main/examples/dragdrop/dropsite.h#1 $
**
** Drop site example implementation
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef DROPSITE_H
#define DROPSITE_H

#include <qlabel.h>
#include "qdropsite.h"

class DropSite: public QLabel, QDropSite
{
    Q_OBJECT
public:
    DropSite( QWidget * parent = 0, const char * name = 0 );
    ~DropSite();

signals:
    void message( const char * );

protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );

    // this is a normal even
    void mousePressEvent( QMouseEvent * );
};


#endif
