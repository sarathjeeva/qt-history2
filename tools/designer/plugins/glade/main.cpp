/**********************************************************************
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include <filterinterface.h>
#include <qapplication.h>

#include "glade2ui.h"

class GladeFilter : public ImportFilterInterface, public QLibraryInterface
{
public:
    GladeFilter();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **iface );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStringList import( const QString& filter, const QString& filename );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    unsigned long ref;
};

GladeFilter::GladeFilter()
: ref( 0 )
{
}

QRESULT GladeFilter::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ImportFilterInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ImportFilter )
	*iface = (ImportFilterInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

unsigned long GladeFilter::addRef()
{
    return ref++;
}

unsigned long GladeFilter::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

QStringList GladeFilter::featureList() const
{
    QStringList list;
    list << "Glade Files (*.glade)" ;
    return list;
}

QStringList GladeFilter::import( const QString &, const QString& filename )
{
    Glade2Ui g;
    return g.convertGladeFile( filename );
}

bool GladeFilter::init()
{
    return TRUE;
}

void GladeFilter::cleanup()
{
}

bool GladeFilter::canUnload() const
{
    return TRUE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( GladeFilter )
}
