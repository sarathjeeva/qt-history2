/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.cpp#3 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#include "qdropsite.h"
#include "qwidget.h"

class QDropSitePrivate : public QObject {
    QDropSite* s;

public:
    QDropSitePrivate( QWidget* parent, QDropSite* site ) :
	QObject(parent),
	s(site)
    {
	parent->installEventFilter(this);
    }

    bool eventFilter( QObject*, QEvent* );
};

bool QDropSitePrivate::eventFilter( QObject *, QEvent * e )
{
    if ( e->type() == Event_Drop ) {
	s->dropEvent( (QDropEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragEnter ) {
	s->dragEnterEvent( (QDragEnterEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragMove ) {
	s->dragMoveEvent( (QDragMoveEvent *)e );
	return TRUE;
    } else if ( e->type() == Event_DragLeave ) {
	s->dragLeaveEvent( (QDragLeaveEvent *)e );
	return TRUE;
    } else {
	return FALSE;
    }
}


QDropSite::QDropSite( QWidget* parent )
{
    d = new QDropSitePrivate(parent,this);
    parent->setAcceptDrops( TRUE );
}

QDropSite::~QDropSite()
{
    delete d; // not really needed
}

void QDropSite::dragEnterEvent( QDragEnterEvent * )
{
}

void QDropSite::dragMoveEvent( QDragMoveEvent * )
{
}

void QDropSite::dragLeaveEvent( QDragLeaveEvent * )
{
}

void QDropSite::dropEvent( QDropEvent * )
{
}
