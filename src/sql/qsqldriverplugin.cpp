/****************************************************************************
**
** Implementation of QSqlDriverPlugin class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsqldriverplugin.h"

#ifndef QT_NO_SQL
#ifndef QT_NO_COMPONENT

#include "qsqldriverinterface_p.h"

/*!
    \class QSqlDriverPlugin qsqldriverplugin.h
    \brief The QSqlDriverPlugin class provides an abstract base for custom QSqlDriver plugins.

    \ingroup plugins
    \mainclass

    The SQL driver plugin is a simple plugin interface that makes it
    easy to create your own SQL driver plugins that can be loaded
    dynamically by Qt.

    Writing a SQL plugin is achieved by subclassing this base class,
    reimplementing the pure virtual functions keys() and create(), and
    exporting the class with the \c Q_EXPORT_PLUGIN macro. See the SQL
    plugins that come with Qt for example implementations (in the
    \c{plugins/src/sqldrivers} subdirectory of the source
    distribution). Read the \link plugins-howto.html plugins
    documentation\endlink for more information on plugins.
*/

/*!
    \fn QStringList QSqlDriverPlugin::keys() const

    Returns the list of drivers (keys) this plugin supports.

    These keys are usually the class names of the custom drivers that
    are implemented in the plugin.

    \sa create()
*/

/*!
    \fn QSqlDriver* QSqlDriverPlugin::create( const QString& key )

    Creates and returns a QSqlDriver object for the driver key \a key.
    The driver key is usually the class name of the required driver.

    \sa keys()
*/

class QSqlDriverPluginPrivate : public QSqlDriverFactoryInterface
{
public:
    QSqlDriverPluginPrivate( QSqlDriverPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QSqlDriverPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QSqlDriver *create( const QString &key );

private:
    QSqlDriverPlugin *plugin;
};

QSqlDriverPluginPrivate::~QSqlDriverPluginPrivate()
{
    delete plugin;
}

QRESULT QSqlDriverPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QSqlDriverFactory )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QSqlDriverPluginPrivate::featureList() const
{
    return plugin->keys();
}

QSqlDriver *QSqlDriverPluginPrivate::create( const QString &key )
{
    return plugin->create( key );
}

/*!
    Constructs a SQL driver plugin. This is invoked automatically by
    the \c Q_EXPORT_PLUGIN macro.
*/

QSqlDriverPlugin::QSqlDriverPlugin()
    : QGPlugin( d = new QSqlDriverPluginPrivate( this ) )
{
}

/*!
    Destroys the SQL driver plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QSqlDriverPlugin::~QSqlDriverPlugin()
{
    // don't delete d, as this is deleted by d
}

#endif // QT_NO_COMPONENT
#endif // QT_NO_SQL
