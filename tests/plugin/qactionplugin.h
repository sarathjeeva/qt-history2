#ifndef QACTIONPLUGIN_H
#define QACTIONPLUGIN_H

#include <qplugin.h>
#include <qpluginmanager.h>

#include "qactioninterface.h"
#include "qactionfactory.h"

class QActionPlugIn : public QActionInterface, public QPlugIn
{
public:
    QActionPlugIn( const QString& filename, LibraryPolicy = Default, const char* fn = 0 );

    QCString queryPlugInInterface() const { return "QActionInterface"; }

    QAction* create( const QString& classname, QObject* parent = 0 );
};

class QActionPlugInManager : public QPlugInManager<QActionPlugIn>, public QActionFactory
{
public:
    QActionPlugInManager( const QString& path = QString::null, const QString& filter = "*.dll; *.so", 
	QPlugIn::LibraryPolicy = QPlugIn::Default, const char* fn = 0 );

    QString factoryName() const { return "QActionPlugInManager"; }

private:
    QAction* newAction( const QString& classname, QObject* parent = 0 );
    QStringList actions();
};

#endif // QACTIONPLUGIN_H
