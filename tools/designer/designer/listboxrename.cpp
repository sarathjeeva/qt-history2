/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#include <qheader.h>
#include <qlineedit.h>
#include <qtimer.h>
#include "listboxrename.h"

class EditableListBoxItem : public QListBoxItem
{
public:
    void setText( const QString & text )
    {
	QListBoxItem::setText( text );
    }
};

ListBoxRename::ListBoxRename( QListBox * eventSource, const char * name = 0 )
    : QObject( eventSource, name ),
      clickedItem( 0 ), activity( FALSE )
{
    src = eventSource;
    src->installEventFilter( this );
    ed = new QLineEdit( src->viewport() );
    ed->hide();
    ed->setFrame( FALSE );

    QObject::connect( ed, SIGNAL( returnPressed() ),
		      this, SLOT( renameClickedItem() ) );
}

bool ListBoxRename::eventFilter( QObject *, QEvent * event )
{   
    if ( event->type() == QEvent::MouseButtonPress ) {

	QPoint pos = ((QMouseEvent *) event)->pos();

	if ( clickedItem &&
	     clickedItem->isSelected() &&
	     (clickedItem == src->itemAt( pos )) ) {
	    QTimer::singleShot( 500, this, SLOT( showLineEdit() ) );
	    activity = FALSE; // we want no drags or clicks for 500 ms before we start the renaming
	} else { // new item clicked
	    activity = TRUE;
	    clickedItem = src->itemAt( pos );
	    ed->hide();
	}
	
    } else if ( ( event->type() == QEvent::MouseMove ) &&
		( ((QMouseEvent *) event)->state() & Qt::LeftButton ) ) {
	activity = TRUE;  // drag
    } else if ( (event->type() == QEvent::KeyPress ) &&
		( ((QKeyEvent *) event)->key() == Qt::Key_F2 ) ) {
	activity = FALSE;
	showLineEdit();
    }
    return FALSE;
}

void ListBoxRename::showLineEdit()
{
    if ( !clickedItem || activity )
	return;
    QRect rect = src->itemRect( clickedItem );
    ed->resize( rect.right() - rect.left() - 1,
		rect.bottom() - rect.top() - 1 );
    ed->move( rect.left() + 1, rect.top() + 1 );
    ed->setText( clickedItem->text() );
    ed->selectAll();
    ed->show();
    ed->setFocus();
}

void ListBoxRename::renameClickedItem()
{
    if ( clickedItem && ed ) {
	( (EditableListBoxItem *) clickedItem )->setText( ed->text() );
	emit itemTextChanged( ed->text() );
    }
    ed->hide();
    clickedItem = 0;
}
