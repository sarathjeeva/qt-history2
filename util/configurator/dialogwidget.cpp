/*
** Implementation file for the configurator main dialog widget
*/

#include <qlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qdir.h>
#include <stdlib.h>

#include "dialogwidget.h"

extern QApplication* pApp;

CDialogWidget::CDialogWidget( QWidget* pParent, const char* pName, WFlags f ) :
    QWidget( pParent, pName, f | WDestructiveClose ),
    m_bShared( true ),
    m_bThreaded( false )
{
    QHBoxLayout* pTopLevelLayout;
    QVBox* pProjectSettingsBox;
    QHBox* pButtonBox;
    QButtonGroup* pThreadGroup;
    QButtonGroup* pLibGroup;
    QGroupBox* pConfigGroup;
    QGroupBox* pOutputGroup;
    QRadioButton* pThreaded;
    QRadioButton* pNonThreaded;
    QRadioButton* pSharedLib;
    QRadioButton* pStaticLib;
    QLabel* pOutNameLabel;
    QLabel* pPlatformLabel;
    QLabel* pCompilerLabel;
    QLabel* pOptionsLabel;
    QPushButton* pCloseButton;
    QPushButton* pGenerateButton;
	QCheckBox* pDebug;
/*
** Create widgets and layouts
*/
	pTopLevelLayout = new QHBoxLayout( this );
	pConfigGroup = new QGroupBox( 1, Qt::Horizontal, "Module options", this );
	m_pConfigView = new CConfigView( pConfigGroup );
	pProjectSettingsBox = new QVBox( this);
	pThreadGroup = new QButtonGroup( 1, Qt::Horizontal, "Threading options", pProjectSettingsBox );
	pNonThreaded = new QRadioButton( "Non-threaded", pThreadGroup );
	pThreaded = new QRadioButton( "Threaded", pThreadGroup );
	pThreadGroup->insert( pNonThreaded, 0 );
	pThreadGroup->insert( pThreaded, 1 );
	pLibGroup = new QButtonGroup( 1, Qt::Horizontal, "Linkage options", pProjectSettingsBox );
	pStaticLib = new QRadioButton( "Static library", pLibGroup );
	pSharedLib = new QRadioButton( "Shared library", pLibGroup );
	pLibGroup->insert( pStaticLib, 0 );
	pLibGroup->insert( pSharedLib, 1 );
	pOutputGroup = new QGroupBox( 2, Qt::Horizontal, "Output options", pProjectSettingsBox );
	pOutNameLabel = new QLabel( "Output filename", pOutputGroup );
	m_pOutNameEdit = new QLineEdit( "Makefile", pOutputGroup );
	pPlatformLabel = new QLabel( "Platform", pOutputGroup );
	m_pPlatform = new QComboBox( false, pOutputGroup );
	FillPlatforms();
	pCompilerLabel = new QLabel( "Compiler", pOutputGroup );
	m_pCompiler = new QComboBox( false, pOutputGroup );
	FillCompilers( m_pPlatform->currentText() );
	pOptionsLabel = new QLabel( "Extra options", pOutputGroup );
	m_pOptions = new QLineEdit( pOutputGroup );
	pButtonBox = new QHBox( pProjectSettingsBox );
	pGenerateButton = new QPushButton( "Generate", pButtonBox );
	pCloseButton = new QPushButton( "Close", pButtonBox );
	pDebug = new QCheckBox( "debug", pConfigGroup );
	
	pTopLevelLayout->setSpacing( 3 );
	pTopLevelLayout->addWidget( pConfigGroup );
	pTopLevelLayout->addWidget( pProjectSettingsBox );
	
/*
** Set the initial default state
*/
    pSharedLib->setChecked( true );
    pNonThreaded->setChecked( true );

/*
** Do the plumbing work
*/
    connect( pCloseButton, SIGNAL( clicked() ), (QObject*)pApp, SLOT( closeAllWindows() ) );
    connect( pGenerateButton, SIGNAL( clicked() ), this, SLOT( generate() ) );
    connect( pLibGroup, SIGNAL( clicked( int ) ), this, SLOT( clickedLib( int ) ) );
    connect( pThreadGroup, SIGNAL( clicked( int ) ), this, SLOT( clickedThread( int ) ) );
    connect( m_pPlatform, SIGNAL( activated( const QString& ) ), this, SLOT( FillCompilers( const QString& ) ) );
	connect( pDebug, SIGNAL( toggled( bool ) ), this, SLOT( toggledDebug( bool ) ) );
}

CDialogWidget::~CDialogWidget( )
{
	qDebug( "~CDialogWidget()\n" );
}

void CDialogWidget::generate()
{
	QStringList* pModules;
	QString strQMake;
	QString strConfigs;
	QString strDefines;

	pModules = m_pConfigView->activeModules();

	// Add all the configuration options from the module list
	for ( QStringList::Iterator it = pModules->begin(); it != pModules->end(); ++it )
	{
		strConfigs += m_pConfigView->mapOption( *it ) + " ";
	}

	if ( m_bThreaded )
	{
		strConfigs += "thread ";
	}

	if ( m_bShared )
	{
		if ( m_pPlatform->currentText() == "win32" )
		{
			strDefines += "QT_MAKEDLL ";
		}
		strConfigs += "dll ";
	}
	else
	{
		if ( m_pPlatform->currentText() == "win32" )
		{
			strDefines += "QT_NODLL ";
		}
		strConfigs += "staticlib ";
	}

	if ( m_bDebug )
	{
		strConfigs += "debug ";
	}
	else
	{
		strConfigs += "release ";
	}

	strConfigs.truncate( strConfigs.length() - 1 ); // Remove the trailing space
	strDefines.truncate( strDefines.length() - 1 ); // Remove the trailing space

	strQMake = "qmake qt \"CONFIG+=" + strConfigs + "\" \"DEFINES+=" + strDefines + "\" -mkspec " + m_pPlatform->currentText() + "-" + m_pCompiler->currentText() + " -o " + m_pOutNameEdit->text();
	if ( m_pOptions->text().length() )
	{
		strQMake += " " + m_pOptions->text();
    }
	qDebug( strQMake );
	system( strQMake.latin1() );
}

void CDialogWidget::clickedLib( int nID )
{
    m_bShared = nID;
}

void CDialogWidget::clickedThread( int nID )
{
    m_bThreaded = nID;
}

void CDialogWidget::FillCompilers( const QString& strPlatform )
{
    QString str = getenv("QTDIR");
    str += "/mkspecs";
    QDir mkspecDir( str, strPlatform + "*", QDir::Name | QDir::IgnoreCase, QDir::Files );
    const QFileInfoList* mkspecs = mkspecDir.entryInfoList();
    QFileInfoListIterator mkspecIterator( *mkspecs );
    QFileInfo* mkspec;
	
    m_pCompiler->clear();
    while ( ( mkspec = mkspecIterator.current() ) )
    {
	m_pCompiler->insertItem( mkspec->fileName().mid( strPlatform.length() + 1 ) );
	++mkspecIterator;
    }
}

void CDialogWidget::FillPlatforms()
{
    QString str = getenv("QTDIR");
    str += "/mkspecs";
    QDir mkspecDir( str, QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files );
    const QFileInfoList* mkspecs = mkspecDir.entryInfoList();
    QFileInfoListIterator mkspecIterator( *mkspecs );
    QFileInfo* mkspec;
    QString strLast,strThis;

    while ( ( mkspec = mkspecIterator.current() ) )
    {
	strThis = mkspec->fileName().left( mkspec->fileName().find( '-' ) );
	if ( strThis != strLast )
	{
	    strLast = strThis;
	    m_pPlatform->insertItem( strThis );
	}
	++mkspecIterator;
    }
}

void CDialogWidget::toggledDebug( bool bDebug )
{
	m_bDebug = bDebug;
}