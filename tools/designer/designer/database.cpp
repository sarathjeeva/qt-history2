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

#include "database.h"
#include "formwindow.h"

QDesignerSqlWidget::QDesignerSqlWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
}
    
void QDesignerSqlWidget::prev()
{
}

void QDesignerSqlWidget::next()
{
}

void QDesignerSqlWidget::first()
{
}

void QDesignerSqlWidget::last()
{
}

void QDesignerSqlWidget::newRecord()
{
}

void QDesignerSqlWidget::paintEvent( QPaintEvent *e )
{
    if ( parentWidget() && parentWidget()->inherits( "FormWindow" ) )
	( (FormWindow*)parentWidget() )->paintGrid( this, e );
}




QDesignerSqlDialog::QDesignerSqlDialog( QWidget *parent, const char *name )
    : QDialog( parent, name )
{
}
    
void QDesignerSqlDialog::prev()
{
}

void QDesignerSqlDialog::next()
{
}

void QDesignerSqlDialog::first()
{
}

void QDesignerSqlDialog::last()
{
}

void QDesignerSqlDialog::newRecord()
{
}

void QDesignerSqlDialog::paintEvent( QPaintEvent *e )
{
    if ( parentWidget() && parentWidget()->inherits( "FormWindow" ) )
	( (FormWindow*)parentWidget() )->paintGrid( this, e );
}
