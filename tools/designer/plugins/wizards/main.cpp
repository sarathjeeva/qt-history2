#define Q_INIT_INTERFACES
#define Q_INITGUID
#include <qcleanuphandler.h>
#include "../../shared/templatewizardiface.h"
#include <qwidget.h>
#include "sqlformwizardimpl.h"

class StandardTemplateWizardInterface : public TemplateWizardInterface
{
public:
    StandardTemplateWizardInterface();

    QUnknownInterface *queryInterface( const QGuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;

    void setup( const QString &templ, QWidget *widget, const QValueList<DatabaseConnection> &dbConnections );

private:
    unsigned long ref;

};

StandardTemplateWizardInterface::StandardTemplateWizardInterface()
    : ref( 0 )
{
}

QStringList StandardTemplateWizardInterface::featureList() const
{
    QStringList list;

    list << "QSqlDialog" << "QDesignerSqlDialog" << "QSqlWidget" << "QDesignerSqlWidget" << "QMainWindow";

    return list;
}

void StandardTemplateWizardInterface::setup( const QString &templ, QWidget *widget, const QValueList<DatabaseConnection> &dbConnections )
{
    if ( templ == "QDesignerSqlWidget" ||
	 templ == "QDesignerSqlDialog" ||
	 templ == "QSqlWidget" ||
	 templ == "QSqlDialog" ) {
	SqlFormWizard *wizard = new SqlFormWizard( widget, dbConnections, 0, 0, TRUE );
	wizard->exec();
    }
}

QUnknownInterface *StandardTemplateWizardInterface::queryInterface( const QGuid& guid )
{
    QUnknownInterface *iface = 0;

    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( guid == IID_TemplateWizardInterface )
	iface = (TemplateWizardInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long StandardTemplateWizardInterface::addRef()
{
    return ref++;
}

unsigned long StandardTemplateWizardInterface::release()
{
    if ( !--ref )
	delete this;

    return ref;
}

Q_EXPORT_INTERFACE( StandardTemplateWizardInterface )
