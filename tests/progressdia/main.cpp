#include <qapplication.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qprogressdialog.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtimer.h>

class ProgressDiaTest : public QVBox
{
    Q_OBJECT
    
public:
    ProgressDiaTest();
    
private slots:
    void normal();
    void backward();
    void multiple();
       
private:
    QCheckBox *modal;
    QProgressDialog *pd;
    QStringList files;
    QStringList::Iterator it;
    int xx;
    
};

ProgressDiaTest::ProgressDiaTest()
    : QVBox(), pd( 0 )
{
    QPushButton *b = new QPushButton( "Normal Progressdia", this );
    connect( b, SIGNAL( clicked() ),
	     this, SLOT( normal() ) );
    b = new QPushButton( "Backwards Progressdia", this );
    connect( b, SIGNAL( clicked() ),
	     this, SLOT( backward() ) );
    b = new QPushButton( "Multiple Progressdia", this );
    connect( b, SIGNAL( clicked() ),
	     this, SLOT( multiple() ) );
    modal = new QCheckBox( "Open Progressdia modally", this );

    QDir dir( "/lib" );
    files = dir.entryList();
}

void ProgressDiaTest::normal()
{
    if ( !pd ) {
	pd = new QProgressDialog( this, "progress dia", modal->isChecked() );
	pd->setTotalSteps( files.count() );
	pd->setProgress( 0 );
	it = files.begin();
	pd->show();
    } else if ( it == files.end() ) {
	pd->setProgress( pd->totalSteps() );
	pd = 0;
	return;
    } else if ( pd->wasCancelled() ) {
	pd = 0;
	return;
    }	
    
    pd->setLabelText( *it );
    pd->setProgress( pd->progress() + 1 );
    ++it;
    
    QTimer::singleShot( 100, this, SLOT( normal() ) );
}

void ProgressDiaTest::backward()
{
    if ( !pd ) {
	pd = new QProgressDialog( this, "progress dia", modal->isChecked() );
	pd->setTotalSteps( files.count() );
	pd->setProgress( 0 );
	pd->setAutoReset( FALSE );
	pd->setProgress( files.count() );
	it = files.begin();
	pd->show();
    } else if ( it == files.end() ) {
	pd->reset();
	pd = 0;
	return;
    } else if ( pd->wasCancelled() ) {
 	pd = 0;
	return;
    }	
    
    pd->setLabelText( *it );
    pd->setProgress( pd->progress() - 1 );
    ++it;
    
    QTimer::singleShot( 100, this, SLOT( backward() ) );
}

void ProgressDiaTest::multiple()
{
    if ( !pd ) {
	pd = new QProgressDialog( this, "progress dia", modal->isChecked() );
	pd->setTotalSteps( files.count() );
	pd->setProgress( 0 );
	it = files.begin();
	pd->setAutoClose( FALSE );
	pd->show();
	xx = 0;
    } else if ( it == files.end() ) {
	pd->setProgress( pd->totalSteps() );
	++xx;

	if ( xx > 2 ) {
	    pd->setAutoClose( TRUE );
	    pd->reset();
	    pd = 0;
	    return;
	}	
    } else if ( pd->wasCancelled() ) {
	pd = 0;
	return;
    }	
    
    pd->setLabelText( *it );
    pd->setProgress( pd->progress() + 1 );
    ++it;
    
    QTimer::singleShot( 100, this, SLOT( multiple() ) );
}

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    ProgressDiaTest d;
    
    a.setMainWidget( &d );
    d.show();

    return a.exec();
}

#include "main.moc"
