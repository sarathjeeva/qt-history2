 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "markerwidget.h"
#include "viewmanager.h"
#include <qrichtext_p.h>
#include "editor.h"
#include <qpainter.h>
#include "paragdata.h"

static const char * error_xpm[] = {
    "14 14 4 1",
    "       c None",
    ".      c #FFFFFF",
    "+      c #8B0000",
    "@      c #FF0000",
    "              ",
    "     ....     ",
    "    .++++.    ",
    "   .+@@@@+.   ",
    "  .+@@@@@@+.  ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    " .+@@@@@@@@+. ",
    "  .+@@@@@@+.  ",
    "   .+@@@@+.   ",
    "    .++++.    ",
    "     ....     ",
    "              "};

static QPixmap *errorPixmap = 0;

MarkerWidget::MarkerWidget( ViewManager *parent )
    : QWidget( parent ), viewManager( parent )
{
    if ( !errorPixmap ) {
	errorPixmap = new QPixmap( error_xpm );
    }
}

void MarkerWidget::paintEvent( QPaintEvent * )
{
    buffer.fill( backgroundColor() );

    QTextParag *p = ( (Editor*)viewManager->currentView() )->document()->firstParag();
    QPainter painter( &buffer );
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	ParagData *paragData = (ParagData*)p->extraData();
	if ( paragData ) {
	    switch ( paragData->marker ) {
	    case ParagData::Error:
		painter.drawPixmap( ( width() - errorPixmap->width() ) / 2, p->rect().y() - yOffset, *errorPixmap );
		break;
	    default:
		break;
	    }
	}
	p = p->next();
    }
    painter.end();

    bitBlt( this, 0, 0, &buffer );
}

void MarkerWidget::resizeEvent( QResizeEvent *e )
{
    buffer.resize( e->size() );
    QWidget::resizeEvent( e );
}
