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
#include <qpopupmenu.h>
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

static const char * breakpoint_xpm[] = {
    "14 14 4 1",
    "       c None",
    ".      c #FFFFFF",
    "+      c #8B0000",
    "@      c yellow",
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

static const char * step_xpm[] = {
"16 16 128 2",
"  	c None",
". 	c #B4B6BF",
"+ 	c #7893D8",
"@ 	c #8D95BF",
"# 	c #B8BFC1",
"$ 	c #B6D1E6",
"% 	c #7193E6",
"& 	c #8893C2",
"* 	c #B3BDC4",
"= 	c #AAD2EC",
"- 	c #9AD0FF",
"; 	c #6690EF",
"> 	c #8894C8",
", 	c #AFBAC4",
"' 	c #95BFEC",
") 	c #99CBFF",
"! 	c #8EC3FF",
"~ 	c #6D95F0",
"{ 	c #8792CA",
"] 	c #9DA7C3",
"^ 	c #8BA2E3",
"/ 	c #809AE0",
"( 	c #8398D1",
"_ 	c #93A0CC",
": 	c #ACB3CB",
"< 	c #B4B9C4",
"[ 	c #B6BAC4",
"} 	c #93A4CC",
"| 	c #82B0F5",
"1 	c #8BBCFF",
"2 	c #8EC0FF",
"3 	c #8FC1FF",
"4 	c #6594F4",
"5 	c #7381CC",
"6 	c #81A7E9",
"7 	c #D0F5FF",
"8 	c #C1EBFF",
"9 	c #AEDAFF",
"0 	c #A2D1FC",
"a 	c #A3C8F3",
"b 	c #AACAE6",
"c 	c #B4CFE9",
"d 	c #ADCCF9",
"e 	c #84B2FF",
"f 	c #82B4FF",
"g 	c #86B7FF",
"h 	c #88B7FF",
"i 	c #83B4FF",
"j 	c #5F8AF3",
"k 	c #7585C8",
"l 	c #77A4F3",
"m 	c #ABDFFF",
"n 	c #9CCAFF",
"o 	c #96C7FF",
"p 	c #97C8FF",
"q 	c #95C5FF",
"r 	c #9DCCFF",
"s 	c #A0CDFF",
"t 	c #90C0FF",
"u 	c #82AFFF",
"v 	c #7FAFFF",
"w 	c #7DAEFF",
"x 	c #79AAFF",
"y 	c #6C9EFF",
"z 	c #4366EB",
"A 	c #6894F2",
"B 	c #93C6FF",
"C 	c #82B3FF",
"D 	c #7AABFF",
"E 	c #73A5FF",
"F 	c #71A3FF",
"G 	c #6C9DFF",
"H 	c #699BFF",
"I 	c #76A8FF",
"J 	c #7EB0FF",
"K 	c #7BADFF",
"L 	c #74A5FF",
"M 	c #608BFF",
"N 	c #3462FF",
"O 	c #2444E5",
"P 	c #577AE0",
"Q 	c #5D90FF",
"R 	c #4C7AFF",
"S 	c #3B66FF",
"T 	c #335CF9",
"U 	c #365AF1",
"V 	c #3858E5",
"W 	c #3959E0",
"X 	c #416CF9",
"Y 	c #75A5FF",
"Z 	c #78A9FF",
"` 	c #74A4FF",
" .	c #6191FF",
"..	c #3059FF",
"+.	c #1B37F1",
"@.	c #6A75C7",
"#.	c #828BC1",
"$.	c #4358D8",
"%.	c #374BDA",
"&.	c #4759CA",
"*.	c #636CC4",
"=.	c #8489C0",
"-.	c #9DA1C1",
";.	c #A3A6BF",
">.	c #7486CB",
",.	c #6E98F5",
"'.	c #719EFF",
").	c #608DFF",
"!.	c #315EFF",
"~.	c #1432F4",
"{.	c #5C63C8",
"].	c #B1B4B9",
"^.	c #B3BABB",
"/.	c #ABB4C3",
"(.	c #7299E9",
"_.	c #5486FF",
":.	c #224EFF",
"<.	c #1733F2",
"[.	c #7079C5",
"}.	c #5C7DE9",
"|.	c #2450FF",
"1.	c #1B39EC",
"2.	c #7077C5",
"3.	c #3A54E1",
"4.	c #1E36EA",
"5.	c #858CBF",
"6.	c #525FCB",
"7.	c #727CBC",
"                                ",
"                . + @           ",
"                # $ % &         ",
"                * = - ; >       ",
"                , ' ) ! ~ {     ",
"] ^ / ( _ : < [ } | 1 2 3 4 5   ",
"6 7 8 9 0 a b c d e f g h i j k ",
"l m n o p q r s t u v u w x y z ",
"A B C D E F G H I J K D L M N O ",
"P Q R S T U V W X Y Z `  ...+.@.",
"#.$.%.&.*.=.-.;.>.,.'.).!.~.{.  ",
"  ].^.          /.(._.:.<.[.    ",
"                  }.|.1.2.      ",
"                  3.4.5.        ",
"                  6.7.          ",
"                                "};

static QPixmap *errorPixmap = 0;
static QPixmap *breakpointPixmap = 0;
static QPixmap *stepPixmap = 0;

MarkerWidget::MarkerWidget( ViewManager *parent )
    : QWidget( parent, 0, WRepaintNoErase | WNorthWestGravity | WResizeNoErase ), viewManager( parent )
{
    if ( !errorPixmap ) {
	errorPixmap = new QPixmap( error_xpm );
	breakpointPixmap = new QPixmap( breakpoint_xpm );
	stepPixmap = new QPixmap( step_xpm );
    }
}

void MarkerWidget::paintEvent( QPaintEvent * )
{
    buffer.fill( backgroundColor() );

    QTextParag *p = ( (Editor*)viewManager->currentView() )->document()->firstParag();
    QPainter painter( &buffer );
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	if ( !p->isVisible() ) {
	    p = p->next();
	    continue;
	}
	if ( p->rect().y() + p->rect().height() - yOffset < 0 ) {
	    p = p->next();
	    continue;
	}	
	if ( p->rect().y() - yOffset > height() )
	    break;
	ParagData *paragData = (ParagData*)p->extraData();
	if ( paragData ) {
	    switch ( paragData->marker ) {
	    case ParagData::Error:
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - errorPixmap->height() ) / 2 -
				    yOffset, *errorPixmap );
		break;
	    case ParagData::Breakpoint:
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - breakpointPixmap->height() ) / 2 -
				    yOffset, *breakpointPixmap );
		break;
	    default:
		break;
	    }
	    switch ( paragData->lineState ) {
	    case ParagData::FunctionStart:
		painter.setPen( colorGroup().foreground() );
		painter.setBrush( colorGroup().base() );
		painter.drawLine( 24, p->rect().y() - yOffset,
				  24, p->rect().y() + p->rect().height() - yOffset );
		painter.drawRect( 20, p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset, 9, 9 );
		painter.drawLine( 22, p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset + 4,
				  26, p->rect().y() +
				  ( p->rect().height() - 9 ) / 2 - yOffset + 4 );
		if ( !paragData->functionOpen )
		    painter.drawLine( 24,
				      p->rect().y() + ( p->rect().height() - 9 ) / 2 - yOffset + 2,
				      24,
				      p->rect().y() +
				      ( p->rect().height() - 9 ) / 2 - yOffset + 6 );
		break;
	    case ParagData::InFunction:
		painter.setPen( colorGroup().foreground() );
		painter.drawLine( 24, p->rect().y() - yOffset,
				  24, p->rect().y() + p->rect().height() - yOffset );
		break;
	    case ParagData::FunctionEnd:
		painter.setPen( colorGroup().foreground() );
		painter.drawLine( 24, p->rect().y() - yOffset,
				  24, p->rect().y() + p->rect().height() - yOffset );
		painter.drawLine( 24, p->rect().y() + p->rect().height() - yOffset,
				  28, p->rect().y() + p->rect().height() - yOffset );
		break;
	    default:
		break;
	    }
	    if ( paragData->step )
		painter.drawPixmap( 3, p->rect().y() +
				    ( p->rect().height() - breakpointPixmap->height() ) / 2 -
				    yOffset, *stepPixmap );
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

void MarkerWidget::mousePressEvent( QMouseEvent *e )
{
    if ( !( (Editor*)viewManager->currentView() )->supportsBreakPoints() )
	return;
    QTextParag *p = ( (Editor*)viewManager->currentView() )->document()->firstParag();
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
	    QTextParagData *d = p->extraData();
	    if ( !d )
		return;
	    ParagData *data = (ParagData*)d;
	    if ( e->x() < 20 ) {
		if ( data->marker == ParagData::Breakpoint )
		    data->marker = ParagData::NoMarker;
		else
		    data->marker = ParagData::Breakpoint;
	    } else {
		if ( data->lineState == ParagData::FunctionStart ) {
		    if ( data->functionOpen )
			emit collapseFunction( p );
		    else
			emit expandFunction( p );
		}
	    }
	    break;
	}
	p = p->next();
    }
    doRepaint();
    emit markersChanged();
}

void MarkerWidget::contextMenuEvent( QContextMenuEvent *e )
{
    QPopupMenu m;

    int toggleBreakPoint;

    QTextParag *p = ( (Editor*)viewManager->currentView() )->document()->firstParag();
    int yOffset = ( (Editor*)viewManager->currentView() )->contentsY();
    while ( p ) {
	if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
	    if ( ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint )
		toggleBreakPoint = m.insertItem( tr( "Clear Breakpoint" ) );
	    else
		toggleBreakPoint = m.insertItem( tr( "Set Breakpoint" ) );
	    m.insertSeparator();
	    break;
	}
	p = p->next();
    }

    const int collapseAll = m.insertItem( tr( "Collapse All" ) );
    const int expandAll = m.insertItem( tr( "Expand All" ) );
    const int collapseFunctions = m.insertItem( tr( "Collapse all Functions" ) );
    const int expandFunctions = m.insertItem( tr( "Expand all Functions" ) );

    int res = m.exec( e->globalPos() );
    if ( res == -1)
	return;

    if ( res == collapseAll ) {
	emit collapse( TRUE );
    } else if ( res == collapseFunctions ) {
	emit collapse( FALSE );
    } else if ( res == expandAll ) {
	emit expand( TRUE );
    } else if ( res == expandFunctions ) {
	emit expand( FALSE );
    } else if ( res == toggleBreakPoint ) {
	if ( ( (ParagData*)p->extraData() )->marker == ParagData::Breakpoint )
	    ( (ParagData*)p->extraData() )->marker = ParagData::NoMarker;
	else
	    ( (ParagData*)p->extraData() )->marker = ParagData::Breakpoint;
    }
    doRepaint();
    emit markersChanged();
}
