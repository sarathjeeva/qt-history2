/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfiledialog_win.cpp#8 $
**
** Implementation of QFileDialog Windows-specific functionality
**
** Created : 990601
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qfiledialog.h"
#include "qapplication.h"
#include "qt_windows.h"
#include "qregexp.h"
#include "qbuffer.h"
#include "qstringlist.h"

extern Qt::WindowsVersion qt_winver;	// defined in qapplication_win.cpp

const int maxNameLen = 255;
const int maxMultiLen = 16383;

// Returns the wildcard part of a filter.
static
QString extractFilter( const QString& rawFilter )
{
    QString result;
    QRegExp r( QString::fromLatin1("([a-zA-Z0-9\\.\\*\\?\\ \\+\\;\\#]*)$") );
    int len;
    int index = r.match( rawFilter, 0, &len );
    if ( index >= 0 ) {
	result = rawFilter.mid( index+1, len-2 );
    } else {
	result = rawFilter;
    }
    return result.replace(QRegExp(" "),";");
}

// Makes a list of filters from ;;-separated text.
static QStringList makeFiltersList( const QString &filter )
{
    QString f( filter );

    if ( f.isEmpty( ) )
	f = QFileDialog::tr( "All Files (*.*)" );

    if ( f.isEmpty() )
	return QStringList();

    int i = f.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( f.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = f.find( sep, 0 );
	}
    }

    return QStringList::split( sep, f  );
}

// Makes a NUL-oriented Windows filter from a Qt filter.
static
QString winFilter( const QString& filter )
{
    QStringList filterLst = makeFiltersList( filter );
    QStringList::Iterator it = filterLst.begin();
    QString winfilters;
    for ( ; it != filterLst.end(); ++it ) {
        winfilters += *it;
        winfilters += QChar::null;
        winfilters += extractFilter(*it);
        winfilters += QChar::null;
    }
    winfilters += QChar::null;
    return winfilters;
}



// Static vars for OFNA funcs:
static QCString aInitDir;
static QCString aInitSel;
static QCString aTitle;
static QCString aFilter;
// Use ANSI strings and API

static
OPENFILENAMEA* makeOFNA( QWidget* parent,
			 const QString& initialSelection,
			 const QString& initialDirectory,
			 const QString& title,
			 const QString& filters,
			 QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    aTitle = title.local8Bit();
    aInitDir = QDir::convertSeparators( initialDirectory ).local8Bit();
    if ( initialSelection.isEmpty() )
	aInitSel = "";
    else
	aInitSel = QDir::convertSeparators( initialSelection ).local8Bit();
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    aInitSel.resize( maxLen + 1 );		// make room for return value
    aFilter = filters.local8Bit();

    OPENFILENAMEA* ofn = new OPENFILENAMEA;
    memset( ofn, 0, sizeof(OPENFILENAMEA) );

    ofn->lStructSize	= sizeof(OPENFILENAMEA);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= aFilter.data();
    ofn->lpstrFile	= aInitSel.data();
    ofn->nMaxFile	= maxLen;
    ofn->lpstrInitialDir = aInitDir.data();
    ofn->lpstrTitle	= aTitle.data();
    ofn->Flags		= ( OFN_NOCHANGEDIR | OFN_HIDEREADONLY );

    if ( mode == QFileDialog::ExistingFile ||
	 mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST );
    if ( mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_ALLOWMULTISELECT | OFN_EXPLORER );

    return ofn;
}


static
void cleanUpOFNA( OPENFILENAMEA** ofn )
{
    delete *ofn;
    *ofn = 0;
}


// Static vars for OFN funcs:
static TCHAR* tTitle;
static TCHAR* tInitDir;
static TCHAR* tInitSel;
static TCHAR* tFilter;

static
OPENFILENAME* makeOFN( QWidget* parent,
		       const QString& initialSelection,
		       const QString& initialDirectory,
		       const QString& title,
		       const QString& filters,
		       QFileDialog::Mode mode )
{
    if ( parent )
	parent = parent->topLevelWidget();
    else
	parent = qApp->mainWidget();

    QString initDir = QDir::convertSeparators( initialDirectory );
    QString initSel = QDir::convertSeparators( initialSelection );

    tTitle = (TCHAR*)qt_winTchar_new( title );
    tInitDir = (TCHAR*)qt_winTchar_new( initDir );
    tFilter = (TCHAR*)qt_winTchar_new( filters );
    int maxLen = mode == QFileDialog::ExistingFiles ? maxMultiLen : maxNameLen;
    tInitSel = new TCHAR[maxLen+1];
    tInitSel[0] = 0;
    if ( initSel.length() > 0 && initSel.length() <= (uint)maxLen )
	memcpy( tInitSel, qt_winTchar( initSel, TRUE ),
		(initSel.length()+1) * sizeof(TCHAR) );

    OPENFILENAME* ofn = new OPENFILENAME;
    memset( ofn, 0, sizeof(OPENFILENAME) );

    ofn->lStructSize	= sizeof(OPENFILENAME);
    ofn->hwndOwner	= parent ? parent->winId() : 0;
    ofn->lpstrFilter	= tFilter;
    ofn->lpstrFile	= tInitSel;
    ofn->nMaxFile	= maxLen;
    ofn->lpstrInitialDir = tInitDir;
    ofn->lpstrTitle	= tTitle;
    ofn->Flags		= ( OFN_NOCHANGEDIR | OFN_HIDEREADONLY );

    if ( mode == QFileDialog::ExistingFile ||
	 mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST );
    if ( mode == QFileDialog::ExistingFiles )
	ofn->Flags |= ( OFN_ALLOWMULTISELECT | OFN_EXPLORER );

    return ofn;
}


static
void cleanUpOFN( OPENFILENAME** ofn )
{
    delete *ofn;
    *ofn = 0;
    delete tFilter;
    tFilter = 0;
    delete tTitle;
    tTitle = 0;
    delete tInitDir;
    tInitDir = 0;
    delete tInitSel;
    tInitSel = 0;
}

QString QFileDialog::winGetOpenFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption )
{
    QString result;

    QString isel = initialSelection;
    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    QFileInfo fi( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
	isel = fi.fileName();
    }

    QString title = caption;
    if ( title.isNull() )
	title = tr("Open");

    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, isel,
				       *initialDirectory, title,
				       winFilter(filter), ExistingFile );
	if ( GetOpenFileNameA( ofn ) )
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	cleanUpOFNA( &ofn );
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, isel,
				     *initialDirectory, title,
				     winFilter(filter), ExistingFile );
	if ( GetOpenFileName( ofn ) )
	    result = qt_winQString( ofn->lpstrFile );
	cleanUpOFN( &ofn );
    }

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	return fi.absFilePath();
    }
}


QString QFileDialog::winGetSaveFileName( const QString &initialSelection,
					 const QString &filter,
					 QString* initialDirectory,
					 QWidget *parent, const char* /*name*/,
					 const QString& caption )
{
    QString result;

    QString isel = initialSelection;
    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    QFileInfo fi( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
	isel = fi.fileName();
    }

    QString title = caption;
    if ( title.isNull() )
	title = tr("Save As");

    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, isel,
				       *initialDirectory, title,
				       winFilter(filter), AnyFile );
	if ( GetSaveFileNameA( ofn ) )
	    result = QString::fromLocal8Bit( ofn->lpstrFile );
	cleanUpOFNA( &ofn );
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, isel,
				     *initialDirectory, title,
				     winFilter(filter), AnyFile );
	if ( GetSaveFileName( ofn ) )
	    result = qt_winQString( ofn->lpstrFile );
	cleanUpOFN( &ofn );
    }

    if ( result.isEmpty() ) {
	return result;
    }
    else {
	QFileInfo fi( result );
	*initialDirectory = fi.dirPath();
	return fi.absFilePath();
    }
}



QStringList QFileDialog::winGetOpenFileNames( const QString &filter,
					      QString* initialDirectory,
					      QWidget *parent,
					      const char* /*name*/,
					      const QString& caption )
{
    QStringList result;
    QFileInfo fi;
    QDir dir;

    if ( initialDirectory && initialDirectory->left( 5 ) == "file:" )
	initialDirectory->remove( 0, 5 );
    fi = QFileInfo( *initialDirectory );

    if ( initialDirectory && !fi.isDir() ) {
	*initialDirectory = fi.dirPath( TRUE );
    }

    QString title = caption;
    if ( title.isNull() )
	title = tr("Open");

    if ( qt_winver & WV_DOS_based ) {
	// Use ANSI strings and API
	OPENFILENAMEA* ofn = makeOFNA( parent, QString::null,
				       *initialDirectory, title,
				       winFilter( filter ), ExistingFiles );
	if ( GetOpenFileNameA( ofn ) ) {
	    QCString fileOrDir = ofn->lpstrFile;
	    int offset = fileOrDir.length() + 1;
	    if ( ofn->lpstrFile[offset] == '\0' ) {
		// Only one file selected; has full path
		fi.setFile( QString::fromLocal8Bit( fileOrDir ) );
		QString res = fi.absFilePath();
		if ( !res.isEmpty() )
		    result.append( res );
	    }
	    else {
		// Several files selected; first string is path
		dir.setPath( QString::fromLocal8Bit( fileOrDir ) );
		QCString f;
		while( !( f = ofn->lpstrFile + offset).isEmpty() ) {
		    fi.setFile( dir, QString::fromLocal8Bit( f ) );
		    QString res = fi.absFilePath();
		    if ( !res.isEmpty() )
			result.append( res );
		    offset += f.length() + 1;
		}
	    }
	    cleanUpOFNA( &ofn );
	}
    }
    else {
	// Use Unicode or ANSI strings and API
	OPENFILENAME* ofn = makeOFN( parent, QString::null,
				     *initialDirectory, title,
				     winFilter( filter ), ExistingFiles );
	if ( GetOpenFileName( ofn ) ) {
	    QString fileOrDir = qt_winQString( ofn->lpstrFile );
	    int offset = fileOrDir.length() + 1;
	    if ( ofn->lpstrFile[offset] == 0 ) {
		// Only one file selected; has full path
		fi.setFile( fileOrDir );
		QString res = fi.absFilePath();
		if ( !res.isEmpty() )
		    result.append( res );
	    }
	    else {
		// Several files selected; first string is path
		dir.setPath( fileOrDir );
		QString f;
		while( !(f=qt_winQString(ofn->lpstrFile+offset)).isEmpty() ) {
		    fi.setFile( dir, f );
		    QString res = fi.absFilePath();
		    if ( !res.isEmpty() )
			result.append( res );
		    offset += f.length() + 1;
		}
	    }
	}
	cleanUpOFN( &ofn );
    }
    *initialDirectory = fi.dirPath();
    return result;
}

