#include "qwidgetplugin.h"
#include <qwidget.h>

/*!
  \class QWidgetPlugIn qdefaultplugin.h

  \brief A plugin loader implementing the QWidgetInterface
*/

/*!
  Constructs a default plugin with file \a file and policy \a pol.
*/
QWidgetPlugIn::QWidgetPlugIn( const QString& file, LibraryPolicy pol )
    : QPlugIn( file, pol )
{
}

/*! \reimp
*/
QWidget* QWidgetPlugIn::create( const QString& classname, QWidget* parent, const char* name )
{
    if ( !use() )
	return 0;

    QWidget* w = ((QWidgetInterface*)iface())->create( classname, parent, name );
    guard( w );
    return w;
}

/*! \reimp
*/
QStringList QWidgetPlugIn::widgets()
{
    if ( !use() )
	return QStringList();
    QStringList list = ((QWidgetInterface*)iface())->widgets();

    return list;
}

/*!
  \class QDefaultPlugInManager qdefaultpluginmanager.h

  \brief Implements a QPlugInManager that handles plugins for custom widgets

  \sa QPlugInManager
*/

/*!
  Creates a QDefaultPlugInManager.

  \sa QPlugInManager
*/
QWidgetPlugInManager::QWidgetPlugInManager( const QString& path, QPlugIn::LibraryPolicy pol )
: QPlugInManager<QWidgetPlugIn>( path, pol )
{
}

/*! \reimp
*/
bool QWidgetPlugInManager::addPlugIn( QPlugIn* p )
{
    QWidgetPlugIn* plugin = (QWidgetPlugIn*)p;
   
    bool useful = FALSE;

    QStringList list = plugin->widgets();
    for ( QStringList::Iterator w = list.begin(); w != list.end(); w++ ) {
	useful = TRUE;
#ifdef CHECK_RANGE
	if ( plugDict[*w] )
	    qWarning("%s: Widget %s already defined!", plugin->library().latin1(), (*w).latin1() );
	else
#endif
	    plugDict.insert( *w, plugin );
    }

    return useful;
}

/*! \reimp
*/
bool QWidgetPlugInManager::removePlugIn( QPlugIn* p )
{
    QWidgetPlugIn* plugin = (QWidgetPlugIn*)p;
    bool res = TRUE;

    QStringList wl = plugin->widgets();
    for ( QStringList::Iterator w = wl.begin(); w != wl.end(); w++ )
        res = res && plugDict.remove( *w );

    return res;
}

/*! \reimp
*/
QWidget* QWidgetPlugInManager::newWidget( const QString& classname, QWidget* parent, const char* name )
{
    QWidgetPlugIn* plugin = (QWidgetPlugIn*)plugDict[ classname ];
    if ( plugin )
	return plugin->create( classname, parent, name );
    return 0;
}

/*!
  Returns a list of all widget classes supported by registered plugins.
*/
QStringList QWidgetPlugInManager::widgets()
{
    QStringList list;
    QDictIterator<QPlugIn> it (libDict);

    while( it.current() ) {
	QStringList widgets = ((QWidgetPlugIn*)it.current())->widgets();
	for ( QStringList::Iterator w = widgets.begin(); w != widgets.end(); w++ )
	    list << *w;
	++it;
    }

    return list;
}


