/*
  config.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <limits.h>
#include <stdlib.h>

#include "config.h"
#include "messages.h"

Config *config;

static bool isCSym( QChar ch )
{
    return ch.isLetterOrNumber() || ch.unicode() == '_';
}

static QString eval( const QString& str )
{
    QString t = str;
    int left = -1;
    while ( (left = t.findRev(QChar('$'), left)) != -1 ) {
	int right = left + 1;
	while ( right < (int) t.length() && isCSym(t[right]) )
	    right++;

	QString key = t.mid( left + 1, right - left - 1 );
	char *val = getenv( key.latin1() );
	if ( val != 0 )
	    t.replace( left, right - left, QString(val) );

	if ( --left == -1 )
	    break;
    }
    return t;
}

static QString singleton( const QString& key, const QStringList& val )
{
    if ( val.count() != 1 ) {
	warning( 2, "Entry '%s' should contain exactly one value (found %d)",
		 key.latin1(), val.count() );
	if ( val.isEmpty() )
	    return QString( "" );
	else
	    return val.first();
    }
    return val.first();
}

static bool isYes( const QString& val )
{
    return val == QString( "yes" ) || val == QString( "true" );
}

static bool isYes( const QString& key, const QStringList& val )
{
    if ( val.count() != 1 ||
	 !QRegExp(QString("yes|no|true|false")).match(val.first()) ) {
	warning( 2, "Entry '%s' in configuration file should be 'yes' or 'no'",
		 key.latin1() );
	return FALSE;
    }
    return val.first() == QString( "yes" ) || val.first() == QString( "true" );
}

static const char *toYN( bool yes )
{
    static const char ny[2][4] = { "no", "yes" };

    return ny[yes ? 1 : 0];
}

static void setPattern( QRegExp *rx, const QString& pattern, bool plus )
{
    QString t = pattern.stripWhiteSpace();
    t.replace( QRegExp(QString("\\s*(?:\\s|[,;]\\s*)")), QChar('|') );

    if ( plus && !rx->pattern().isNull() && rx->isValid() )
	rx->setPattern( rx->pattern() + QChar('|') + t );
    else
	rx->setPattern( t );
}

Config::Config( int argc, char **argv )
    : maxSim( 16 ), maxAll( 64 ), wlevel( 2 ), bas( "" ), modshort( "" ),
      modlong( "" ), co( "" ), vers( "" ), verssym( "" ), posth( "" ),
      foot( "" ), addr( "" ), styl( "" ), falsesym( QChar('0') ),
      serial( FALSE ), internal( FALSE ), readh( TRUE ), autoh( TRUE ),
      super( FALSE ), dotHtml( ".html" ), membersDotHtml( "-members.html" )
{
    QString confFilePath( "./qdoc.conf" );
    int i;

    i = 1;
    while ( i < argc ) {
	QString opt( argv[i++] );

	if ( opt == QString("--help") || opt == QString("-h") ) {
	    argv[i - 1][0] = '\0';
	    showHelp();
	} else if ( opt == QString("--help-short") || opt == QString("-H") ) {
	    argv[i - 1][0] = '\0';
	    showHelpShort();
	} else if ( opt == QString("--version") || opt == QString("-v") ) {
	    argv[i - 1][0] = '\0';
	    showVersion();
	} else if ( opt == QString("--") ) {
	    argv[i - 1][0] = '\0';
	    if ( i < argc )
		confFilePath = QString( argv[i] );
	    while ( i < argc )
		argv[i++][0] = '\0';
	} else if ( opt.left(1) != QChar('-') ) {
	    confFilePath = opt;
	    argv[i - 1][0] = '\0';
	    break;
	}
    }

    QFile f( confFilePath );
    if ( !f.open(IO_ReadOnly) ) {
	warning( 1, "Cannot open configuration file '%s'",
		 confFilePath.latin1() );
	return;
    }

    QTextStream t( &f );
    yyIn = t.read();
    f.close();
    int yyPos = 0;

    while ( yyPos < (int) yyIn.length() ) {
	QString key;
	QStringList val;
	if ( !matchLine(&key, &val) )
	    break;

	if ( key == QString("ADDRESS") ) {
	    addr = val.join( QChar(' ') );
	} else if ( key == QString("AUTOHREFS") ) {
	    autoh = isYes( singleton(key, val) );
	} else if ( key == QString("BASE") ) {
	    bas = val.join( QChar(' ') );
	} else if ( key == QString("COMPANY") ) {
	    co = val.join( QChar(' ') );
	} else if ( key == QString("DEFINE") ) {
	    defsym.setPattern( val.join(QChar('|')) );
	} else if ( key == QString("DOCDIRS") ) {
	    docdirs = val;
	} else if ( key == QString("EXAMPLEDIRS") ) {
	    exampledirs = val;
	} else if ( key == QString("FALSE") ) {
	    falsesym.setPattern( val.join(QChar('|')) );
	} else if ( key == QString("FOOTER") ) {
	    foot = val.join( QChar(' ') );
	} else if ( key == QString("INCLUDEDIRS") ) {
	    includedirs = val;
	} else if ( key == QString("INTERNAL") ) {
	    internal = isYes( key, val );
	} else if ( key == QString("MAXSIMILAR") ) {
	    maxSim = singleton( key, val ).toInt();
	} else if ( key == QString("MAXWARNINGS") ) {
	    maxAll = singleton( key, val ).toInt();
	} else if ( key == QString("MODULELONG") ) {
	    modlong = val.join( QChar(' ') );
	} else if ( key == QString("MODULESHORT") ) {
	    modshort = val.join( QChar(' ') );
	} else if ( key == QString("ONLY") ) {
	    onlysym.setPattern( val.join(QChar('|')) );
	} else if ( key == QString("OUTPUTDIR") ) {
	    outputdir = singleton( key, val );
	} else if ( key == QString("POSTHEADER") ) {
	    posth = val.join( QChar(' ') );
	} else if ( key == QString("READHEADERS") ) {
	    readh = isYes( key, val );
	} else if ( key == QString("SERIALCOMMA") ) {
	    serial = isYes( key, val );
	} else if ( key == QString("SOURCEDIRS") ) {
	    sourcedirs = val;
	} else if ( key == QString("STYLE") ) {
	    styl = val.join( QChar(' ') );
	} else if ( key == QString("SUPERVISOR") ) {
	    super = isYes( key, val );
	} else if ( key == QString("VERSION") ) {
	    vers = val.join( QChar(' ') );
	} else if ( key == QString("VERSIONSYM") ) {
	    verssym = singleton( key, val );
	} else if ( key == QString("WARNINGLEVEL") ) {
	    wlevel = singleton( key, val ).toInt();
	    if ( wlevel < 0 )
		wlevel = 0;
	} else {
	    warning( 2, "Unknown entry '%s' in configuration file",
		     key.latin1() );
	    break;
	}
    }

    i = 1;
    while ( i < argc ) {
	QString opt( argv[i++] );
	QString val;
	bool plus = FALSE;
	bool unknown = FALSE;

	if ( opt.left(2) == QString("--") ) {
	    int k = opt.find( QChar('=') );
	    if ( k == -1 ) {
		i++;
		if ( i == argc ) {
		    warning( 1, "Expected '%s+=foo', '%s=foo' or '%s foo' on"
			     "command line", opt.latin1() );
		    showHelp();
		    break;
		}
		val = argv[i++];
	    } else {
		val = opt.mid( k + 1 );
		opt.truncate( k );
		if ( opt.right(1) == QChar('+') ) {
		    plus = TRUE;
		    opt.truncate( k - 1 );
		}
	    }

	    if ( opt == QString("--auto-hrefs") ) {
		autoh = isYes( val );
	    } else if ( opt == QString("--base") ) {
		bas = val;
	    } else if ( opt == QString("--define") ) {
		setPattern( &defsym, val, plus );
	    } else if ( opt == QString("--false") ) {
		setPattern( &falsesym, val, plus );
	    } else if ( opt == QString("--internal") ) {
		internal = isYes( val );
	    } else if ( opt == QString("--max-similar") ) {
		if ( !plus )
		    maxSim = 0;
		maxSim += val.toInt();
	    } else if ( opt == QString("--max-warnings") ) {
		if ( !plus )
		    maxAll = 0;
		maxAll += val.toInt();
	    } else if ( opt == QString("--only") ) {
		setPattern( &onlysym, val, plus );
	    } else if ( opt == QString("--output-dir") ) {
		outputdir = val;
	    } else if ( opt == QString("--read-headers") ) {
		readh = isYes( val );
	    } else if ( opt == QString("--serial-comma") ) {
		serial = isYes( val );
	    } else if ( opt == QString("--supervisor") ) {
		super = isYes( val );
	    } else if ( opt == QString("--warning-level") ) {
		if ( !plus )
		    wlevel = 0;
		wlevel += val.toInt();
	    } else {
		unknown = TRUE;
	    }
	} else if ( opt.left(1) == QChar('-') ) {
	    if ( opt == QString("-a") ) {
		autoh = TRUE;
	    } else if ( opt == QString("-A") ) {
		autoh = FALSE;
	    } else if ( opt.left(2) == QString("-D") ) {
		setPattern( &defsym, opt.mid(2), TRUE );
	    } else if ( opt.left(2) == QString("-F") ) {
		setPattern( &falsesym, opt.mid(2), TRUE );
	    } else if ( opt == QString("-i") ) {
		internal = TRUE;
	    } else if ( opt == QString("-I") ) {
		internal = FALSE;
	    } else if ( opt.left(2) == QString("-m") ) {
		maxSim = opt.mid( 2 ).toInt();
	    } else if ( opt.left(2) == QString("-M") ) {
		maxAll = opt.mid( 2 ).toInt();
	    } else if ( opt.left(2) == QString("-O") ) {
		setPattern( &onlysym, opt.mid(2), TRUE );
	    } else if ( opt == QString("-s") ) {
		super = TRUE;
	    } else if ( opt == QString("-S") ) {
		super = FALSE;
	    } else if ( QRegExp(QString("-W[0-4]")).match(opt) ) {
		wlevel = opt[2].unicode() - QChar( '0' ).unicode();
	    } else if ( opt == QString("-Wnone") ) {
	        wlevel = 0;
	    } else if ( opt == QString("-Wall") ) {
		wlevel = 4;
		maxSim = 262144;
		maxAll = 262144;
	    } else {
		unknown = TRUE;
	    }
	} else if ( !opt.isEmpty() ) {
	    warning( 1, "Command-line argument '%s' ignored", opt.latin1() );
	    showHelp();
	}
	if ( unknown ) {
	    warning( 1, "Unknown command-line option '%s'", opt.latin1() );
	    showHelp();
	}
    }

    if ( onlysym.pattern().isEmpty() || !onlysym.isValid() )
	onlysym.setPattern( QString(".*") );

    setMaxSimilarMessages( maxSim );
    setMaxMessages( maxAll );
    setWarningLevel( wlevel );

    yyIn = QString::null;
}

void Config::setVersion( const QString& version )
{
    if ( vers.isEmpty() ) {
	vers = version;
	addr.replace( QRegExp(QString("\\\\version")), vers );
    }
}

QString Config::verbatimHref( const QString& sourceFileName ) const
{
    QString t = sourceFileName;
    t.replace( QRegExp(QString("[./]")), QChar('-') );
    t += dotHtml;
    return t;
}

QString Config::classRefHref( const QString& className ) const
{
    return className.lower() + dotHtml;
}

QString Config::classMembersHref( const QString& className ) const
{
    return className.lower() + membersDotHtml;
}

QString Config::defgroupHref( const QString& groupName ) const
{
    return groupName.lower() + dotHtml;
}

QString Config::findDepth( const QString& name,
			   const QStringList& dirList ) const
{
    QStringList::ConstIterator s;
    QString filePath;

    s = dirList.begin();
    while ( s != dirList.end() ) {
	QDir dir( *s );
	if ( dir.exists(name) )
	    return dir.filePath( name );
	++s;
    }
    return QString::null;
}

bool Config::isTrue( const QString& condition ) const
{
    return !falsesym.match( condition );
}

bool Config::isDef( const QString& symbol ) const
{
    return defsym.match( symbol );
}

bool Config::processClass( const QString& className ) const
{
    return onlysym.match( className );
}

bool Config::matchLine( QString *key, QStringList *val )
{
    static QRegExp *keyX = 0;
    static QRegExp *valXOrY = 0;
    static QRegExp *backslashX = 0;

    if ( keyX == 0 ) {
	keyX = new QRegExp( QString(
		       "^[ \t]*([A-Z_a-z][A-Z_a-z0-9]*)[ \t]*=[ \t]*") );
	valXOrY = new QRegExp( QString(
			  "^(?:([^\" \n\t]*)|\"((?:[^\"\\\\]|\\\\.)*)\")"
			  "(?:[ \t]|\\\\[ \t]*\n)*") );
	backslashX = new QRegExp( QString("\\\\(.)") );
    }

    if ( keyX->search(yyIn.mid(yyPos)) != -1 ) {
	*key = keyX->cap( 1 );
	yyPos += keyX->matchedLength();

	while ( valXOrY->search(yyIn.mid(yyPos)) != -1 ) {
	    if ( !valXOrY->cap(1).isEmpty() ) {
		val->append( eval(valXOrY->cap(1)) );
	    } else if ( !valXOrY->cap(2).isEmpty() ) {
		QString t = valXOrY->cap( 2 );
		int k = 0;
		while ( (k = t.find(*backslashX, k)) != -1 ) {
		    t.replace( k, 2, backslashX->cap(1) );
		    k += 2;
		}
		val->append( t );
	    }
	    if ( valXOrY->matchedLength() == 0 )
		break;
	    yyPos += valXOrY->matchedLength();
	}
	if ( yyIn.mid(yyPos, 1) == QChar('\n') ) {
	    yyPos++;
	    return TRUE;
	}
    }

    if ( yyPos < (int) yyIn.length() )
	warning( 1, "Bad syntax in configuration file at line %d\n",
		 yyIn.left(yyPos).contains(QChar('\n')) + 1 );
    return FALSE;
}

void Config::showHelp()
{
    /*
      The output is meant to look like that of gcc.
    */
    printf( "Usage: qdoc [options] [qdoc.conf]\n"
	    "Long options:\n"
	    "  --auto-hrefs=<yes|no>    Automatically add hrefs [%s]\n"
	    "  --base=<url>             Set documentation base URL\n"
	    "  --define=<regexp>        Define preprocessor symbols\n"
	    "  --false=<regexp>         Define false preprocessor predicates\n"
	    "  --help                   Display this information\n"
	    "  --help-short             Display short options\n"
	    "  --internal=<yes|no>      Generate internal documentation [%s]\n"
	    "  --max-similar=<num>      Limit number of similar warnings [%d]\n"
	    "  --max-warnings=<num>     Limit number of warnings [%d]\n"
	    "  --only=<regexp>          Process only specified classes\n"
	    "  --output-dir=<path>      Set output directory\n"
	    "  --read-headers=<yes|no>  Read local includes in examples [%s]\n"
	    "  --serial-comma=<yes|no>  Use serial comma in enumerations [%s]\n"
	    "  --supervisor=<yes|no>    Compare with previous run [%s]\n"
	    "  --version                Display version of qdoc\n"
	    "  --warning-level=<num>    Set warning level (0 to 4) [%d]\n",
	    toYN(autoh), toYN(internal), maxSim, maxAll, toYN(readh),
	    toYN(serial), toYN(super), wlevel );
    exit( EXIT_SUCCESS );
}

void Config::showHelpShort()
{
    printf( "Usage: qdoc [options] [qdoc.conf]\n"
	    "Short options:\n"
	    "  -a vs. -A                Automatically add hrefs [%s]\n"
	    "  -D<regexp>               Define preprocessor symbols\n"
	    "  -F<regexp>               Define false preprocessor predicates\n"
	    "  -h                       Display long options\n"
	    "  -H                       Display this information\n"
	    "  -i vs. -I                Generate internal documentation [%s]\n"
	    "  -m<num>                  Limit number of similar warnings [%d]\n"
	    "  -M<num>                  Limit number of warnings [%d]\n"
	    "  -O<regexp>               Process only specified classes\n"
	    "  -s vs. -S                Compare with previous run [%s]\n"
	    "  -v                       Display version of qdoc\n"
	    "  -W<num>                  Set warning level (0 to 4) [%d]\n"
	    "  -Wall                    Enable all warnings\n"
	    "  -Wnone                   Disable all warnings\n",
	    toYN(autoh), toYN(internal), maxSim, maxAll, toYN(super),
	    wlevel );
    exit( EXIT_SUCCESS );
}

void Config::showVersion()
{
    printf( "qdoc version 1.91\n" );   // $\lim_{t\rightarrow\infty} v(t) = 2$
    exit( EXIT_SUCCESS );
}
