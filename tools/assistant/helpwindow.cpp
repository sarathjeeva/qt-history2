#include "helpwindow.h"
#include <qurl.h>
#include <qmessagebox.h>
#include <qdragobject.h>
#include <qdir.h>
#include <qfile.h>

HelpWindow::HelpWindow( QWidget *parent, const char *name )
    : QTextBrowser( parent, name )
{
}

void HelpWindow::setSource( const QString &name )
{
    if ( name.left( 2 ) != "p:" ) {
	QUrl u( context(), name );
	if ( !u.isLocalFile() ) {
	    QMessageBox::information( this, tr( "Help" ), tr( "Can't load and display non-local file\n"
							      "%1" ).arg( name ) );
	    return;	
	}

	QTextBrowser::setSource( name );
    } else {
	QString txt;
	const QMimeSource* m = mimeSourceFactory()->data( name.mid( 2 ).left( name.mid( 2 ).find( '#' ) ), context() );
	if ( !m || !QTextDrag::decode( m, txt ) ) {
	    qWarning("QTextBrowser: cannot decode %s", name.mid( 2 ).latin1() );
	    return;
	}	
	
	int i = txt.find( "name=\"" + name.mid( name.find( '#' ) + 1 ) );
	i = txt.findRev( "<h3 class", i );
	QString s( "<a name=\"" + name.mid( name.find( '#' ) + 1 ) + "\"><p><table><tr><td bgcolor=gray>" );
	txt.insert( i, s );
	int j = txt.find( "<h3 class", i + 1 + s.length() );
	if ( j == -1 ) {
	    j = txt.find( "<!-- eo", i + 1 + s.length() );
	} else {
	    int k = txt.find( "<hr>", i + 1 + s.length() );
	    j = QMIN( j, k );
	}
	txt.insert( j, "</td></tr></table>" );
	QString home = QDir::homeDirPath();
	QFile f( home + "/.designer/tmp_" +
		 name.mid( name.find( '#' ) + 1 ) + "_" + name.mid( 2 ).left( name.mid( 2 ).find( '#' ) ) );
	if ( !f.open( IO_WriteOnly ) )
	    return;
	QTextStream ts( &f );
	ts << txt;
	f.close();
	QTextBrowser::setSource( home + "/.designer/tmp_" + name.mid( name.find( '#' ) + 1 ) + "_" + name.mid( 2 ));
    }
}

