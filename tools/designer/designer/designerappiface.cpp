#include "designerappiface.h"
#include "mainwindow.h"
#include "project.h"
#include "formwindow.h"
#include "widgetfactory.h"
#include "outputwindow.h"
#include "../shared/widgetdatabase.h"
#include <qvariant.h>
#include <qlistview.h>
#include <qtextedit.h>

DesignerInterfaceImpl::DesignerInterfaceImpl( MainWindow *mw )
    : ref( 0 ), mainWindow( mw )
{
}

QUnknownInterface *DesignerInterfaceImpl::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QComponentInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_DesignerInterface )
	iface = (QUnknownInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long DesignerInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

DesignerProject *DesignerInterfaceImpl::currentProject() const
{
    return mainWindow->currProject()->iFace();
}

QList<DesignerProject> DesignerInterfaceImpl::projectList() const
{
    return QList<DesignerProject>();
}

void DesignerInterfaceImpl::showStatusMessage( const QString & ) const
{
}

DesignerDock *DesignerInterfaceImpl::createDock() const
{
    return 0;
}

DesignerOutputDock *DesignerInterfaceImpl::outputDock() const
{
    return 0;
}







DesignerProjectImpl::DesignerProjectImpl( Project *pr )
    : project( pr )
{
}

DesignerFormWindow *DesignerProjectImpl::currentForm() const
{
    return 0;
}

QList<DesignerFormWindow> DesignerProjectImpl::formList() const
{
    return QList<DesignerFormWindow>();
}

void DesignerProjectImpl::addForm( DesignerFormWindow * )
{
}

void DesignerProjectImpl::removeForm( DesignerFormWindow * )
{
}

QString DesignerProjectImpl::fileName() const
{
    return QString::null;
}

void DesignerProjectImpl::setFileName( const QString & )
{
}

QString DesignerProjectImpl::projectName() const
{
    return QString::null;
}

void DesignerProjectImpl::setProjectName( const QString & )
{
}

QString DesignerProjectImpl::databaseFile() const
{
    return QString::null;
}

void DesignerProjectImpl::setDatabaseFile( const QString & )
{
}

void DesignerProjectImpl::setupDatabases() const
{
    MainWindow::self->editDatabaseConnections();
}

QList<DesignerDatabase> DesignerProjectImpl::databaseConnections() const
{
    QList<DesignerDatabase> lst;
    QList<Project::DatabaseConnection> conns = project->databaseConnections();
    for ( Project::DatabaseConnection *d = conns.first(); d; d = conns.next() ) {
	DesignerDatabaseImpl *db = new DesignerDatabaseImpl( d );
	lst.append( db );
    };
    return lst;
}

void DesignerProjectImpl::addDatabase( DesignerDatabase * )
{
}

void DesignerProjectImpl::removeDatabase( DesignerDatabase * )
{
}

void DesignerProjectImpl::save() const
{
}






DesignerDatabaseImpl::DesignerDatabaseImpl( Project::DatabaseConnection *d )
    : db( d )
{
}

QString DesignerDatabaseImpl::name() const
{
    return db->name;
}

void DesignerDatabaseImpl::setName( const QString & )
{
}

QString DesignerDatabaseImpl::driver() const
{
    return db->driver;
}

void DesignerDatabaseImpl::setDriver( const QString & )
{
}

QString DesignerDatabaseImpl::database() const
{
    return db->dbName;
}

void DesignerDatabaseImpl::setDatabase( const QString & )
{
}

QString DesignerDatabaseImpl::userName() const
{
    return db->username;
}

void DesignerDatabaseImpl::setUserName( const QString & )
{
}

QString DesignerDatabaseImpl::password() const
{
    return db->password;
}

void DesignerDatabaseImpl::setPassword( const QString & )
{
}

QString DesignerDatabaseImpl::hostName() const
{
    return db->hostname;
}

void DesignerDatabaseImpl::setHostName( const QString & )
{
}

QStringList DesignerDatabaseImpl::tables() const
{
    return db->tables;
}

QMap<QString, QStringList> DesignerDatabaseImpl::fields() const
{
    return db->fields;
}

void DesignerDatabaseImpl::open() const
{
    db->open();
}

void DesignerDatabaseImpl::close() const
{
    db->close();
}

void DesignerDatabaseImpl::setFields( const QMap<QString, QStringList> & )
{
}

void DesignerDatabaseImpl::setTables( const QStringList & )
{
}




DesignerFormWindowImpl::DesignerFormWindowImpl( FormWindow *fw )
    : formWindow( fw )
{
}

QString DesignerFormWindowImpl::fileName() const
{
    return QString::null;
}

void DesignerFormWindowImpl::setFileName( const QString & )
{
}

void DesignerFormWindowImpl::save() const
{
}

void DesignerFormWindowImpl::insertWidget( QWidget * )
{
}

QWidget *DesignerFormWindowImpl::create( const char *className, QWidget *parent, const char *name )
{
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( className ), parent, name );
    formWindow->insertWidget( w, TRUE );
    return w;
}

void DesignerFormWindowImpl::removeWidget( QWidget * )
{
}

QWidgetList DesignerFormWindowImpl::widgets() const
{
    return QWidgetList();
}

void DesignerFormWindowImpl::undo()
{
}

void DesignerFormWindowImpl::redo()
{
}

void DesignerFormWindowImpl::cut()
{
}

void DesignerFormWindowImpl::copy()
{
}

void DesignerFormWindowImpl::paste()
{
}

void DesignerFormWindowImpl::adjustSize()
{
}

void DesignerFormWindowImpl::editConnections()
{
}

void DesignerFormWindowImpl::checkAccels()
{
}

void DesignerFormWindowImpl::layoutH()
{
}

void DesignerFormWindowImpl::layoutV()
{
}

void DesignerFormWindowImpl::layoutHSplit()
{
}

void DesignerFormWindowImpl::layoutVSplit()
{
}

void DesignerFormWindowImpl::layoutG()
{
}

void DesignerFormWindowImpl::layoutHContainer()
{
}

void DesignerFormWindowImpl::layoutVContainer()
{
}

void DesignerFormWindowImpl::layoutGContainer()
{
}

void DesignerFormWindowImpl::breakLayout()
{
}

void DesignerFormWindowImpl::selectWidget( QWidget * )
{
}

void DesignerFormWindowImpl::selectAll()
{
}

void DesignerFormWindowImpl::clearSelection()
{
}

bool DesignerFormWindowImpl::isWidgetSelected( QWidget * ) const
{
    return FALSE;
}

QWidgetList DesignerFormWindowImpl::selectedWidgets() const
{
    return QWidgetList();
}

QWidget *DesignerFormWindowImpl::currentWidget() const
{
    return 0;
}

void DesignerFormWindowImpl::setCurrentWidget( QWidget * )
{
}

QList<QAction> DesignerFormWindowImpl::actionList() const
{
    return QList<QAction>();
}

void DesignerFormWindowImpl::addAction( QAction * )
{
}

void DesignerFormWindowImpl::removeAction( QAction * )
{
}

void DesignerFormWindowImpl::preview() const
{
}

void DesignerFormWindowImpl::addConnection( QObject *sender, const char *signal, QObject *receiver, const char *slot )
{
    MetaDataBase::addConnection( formWindow, sender, signal, receiver, slot );
}

void DesignerFormWindowImpl::setProperty( QObject *o, const char *property, const QVariant &value )
{
    QVariant v = o->property( property );
    if ( v.isValid() )
	o->setProperty( property, value );
    else
	MetaDataBase::setFakeProperty( o, property, value );
}

QVariant DesignerFormWindowImpl::property( QObject *o, const char *prop ) const
{
    QVariant v = o->property( prop );
    if ( v.isValid() )
	return v;
    return MetaDataBase::fakeProperty( o, prop );
}

void DesignerFormWindowImpl::setPropertyChanged( QObject *o, const char *property, bool changed )
{
    MetaDataBase::setPropertyChanged( o, property, changed );
}

bool DesignerFormWindowImpl::isPropertyChanged( QObject *o, const char *property ) const
{
    return MetaDataBase::isPropertyChanged( o, property );
}

void DesignerFormWindowImpl::setColumnFields( QObject *o, const QMap<QString, QString> &f )
{
    MetaDataBase::setColumnFields( o, f );
}





DesignerDockImpl::DesignerDockImpl()
{
}

QDockWindow *DesignerDockImpl::dockWindow() const
{
    return 0;
}






DesignerOutputDockImpl::DesignerOutputDockImpl( OutputWindow *ow )
    : outWin( ow )
{
}

QWidget *DesignerOutputDockImpl::addView( const QString & )
{
    return 0;
}

void DesignerOutputDockImpl::appendDebug( const QString &s )
{
    outWin->debugView->append( s );
}

void DesignerOutputDockImpl::clearDebug()
{
}

void DesignerOutputDockImpl::appendError( const QString &s, int l )
{
    QStringList ls;
    ls << s;
    QValueList<int> ll;
    ll << l;
    outWin->setErrorMessages( ls, ll, FALSE );
}

void DesignerOutputDockImpl::clearError()
{
}
