/****************************************************************************
** $Id: $
**
** Implementation of the Qt Designer integration plugin
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#include <qsettings.h>
#include <qapplication.h>

void QActiveXSelect::init()
{
    activex = 0;
    QApplication::setOverrideCursor( WaitCursor );
    QSettings controls;
    QStringList clsids = controls.subkeyList( "/Classes/CLSID" );
    for ( QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it ) {
	QString clsid = *it;
	QStringList subkeys = controls.subkeyList( "/Classes/CLSID/" + clsid );
	if ( subkeys.contains( "Control" ) ) {
	    QString name = controls.readEntry( "/Classes/CLSID/" + clsid + "/Default" );
	    ActiveXList->insertItem( name );
	}
    }
    ActiveXList->sort();
    QApplication::restoreOverrideCursor();

    ActiveXList->setFocus();
}


void QActiveXSelect::controlSelected( const QString &ctrl )
{
    ActiveX->setText( ctrl );
}

void QActiveXSelect::openLater()
{
    if ( !activex || !activex->isNull() || !designer ) {
	if ( designer )
	    designer->release();
	delete this;
	return;
    }
    if ( exec() ) {
	activex->setControl( ActiveX->text() );
	DesignerFormWindow *form = designer->currentForm();
	if ( form ) {
	    form->setPropertyChanged( activex, "control", TRUE );
	}
	designer->release();
	delete this;
    }
}

void QActiveXSelect::setActiveX( QAxWidget *ax )
{
    activex = ax;
}

void QActiveXSelect::setDesigner( DesignerInterface *des )
{
    designer = des;
    designer->addRef();
}


QString QActiveXSelect::selectedControl()
{
    return ActiveX->text();
}
