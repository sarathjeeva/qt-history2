/*
  emitter.cpp
*/

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <stdlib.h>

#include "binarywriter.h"
#include "bookparser.h"
#include "config.h"
#include "doc.h"
#include "emitter.h"
#include "html.h"
#include "htmlwriter.h"
#include "messages.h"
#include "stringset.h"

// defined in cppparser.cpp
extern void parseCppHeaderFile( DocEmitter *emitter, const QString& fileName );
extern void parseCppSourceFile( DocEmitter *emitter, const QString& fileName );

// defined in htmlparser.cpp
extern void parseHtmlFile( DocEmitter *emitter, const QString& fileName );

static int compareMtime( const void *n1, const void *n2 )
{
    if ( n1 == 0 || n2 == 0 )
	return 0;

    QFileInfo f1( *(QString *) n1 );
    QFileInfo f2( *(QString *) n2 );
    if ( f1.lastModified() < f2.lastModified() )
	return -1;
    else if ( f1.lastModified() > f2.lastModified() )
	return 1;
    else
	return 0;
}

static QString protect( const QString& str, QChar metaCh )
{
    /*
      Suppose metaCh is '|' and str is '\ is not |'. The result should be
      '\\ is not \|' or, in C++ notation, "\\\\ is not \\|".
    */
    QString t = str;
    t.replace( QRegExp(QString("\\\\")), QString("\\\\") );
    t.replace( QRegExp(QString("\\") + metaCh), QString("\\") + metaCh );
    return t;
}

static void emitHtmlHeaderFile( const QString& headerFilePath,
				const QString& htmlFileName )
{
    QFile f( headerFilePath );
    if ( !f.open(IO_ReadOnly) ) {
	message( 1, "Cannot open %s", headerFilePath.latin1() );
	return;
    }

    QTextStream t( &f );
    QString fullText = t.read();
    f.close();

    Location loc( headerFilePath );
    HtmlWriter out( loc, htmlFileName );
    QString headerFileName = QFileInfo( f ).fileName();

    out.setTitle( headerFileName + QString(" Include File") );
    out.setHeading( headerFileName );
    out.printfMeta( "<p>This is the verbatim text of the %s include file. It"
		   " is provided only for illustration; the copyright remains"
		   " with %s.\n", headerFileName.latin1(),
		   config->company().latin1() );
    out.putsMeta( "<hr>\n<pre>\n" );
    out.puts( fullText );
    out.putsMeta( "</pre>\n" );
}

/*
  Returns a string suitable for writing to the propertydocs file (XML).
*/
static QString fixedPropertyDoc( const QString& html, const QString& className )
{
    QString t = html;
    t.replace( QRegExp(QString("href=\"#")),
	       QString("href=\"%1#").arg(config->classRefHref(className)) );
    return htmlProtect( t, FALSE );
}

void BookEmitter::start( const Resolver *resolver )
{
    QStringList bookFiles = config->findAll( QString("*.book"),
					     config->bookDirList() );
    QStringList::ConstIterator s = bookFiles.begin();
    while ( s != bookFiles.end() ) {
	parseBookFile( *s, Html | Sgml, resolver );
	++s;
    }
}

void DocEmitter::start()
{
    QStringList::ConstIterator s;

    QStringList headerFiles = config->findAll( QString("*.h"),
					       config->includeDirList() );
    s = headerFiles.begin();
    while ( s != headerFiles.end() ) {
	parseCppHeaderFile( this, *s );
	++s;
    }
    nailDownDecls();

    QStringList sourceFiles =
	    config->findAll( QString("*.cpp"), config->sourceDirList() ) +
	    config->findAll( QString("*.doc"), config->docDirList() );
    int i = 0;
    int n = sourceFiles.count();
    QString *files = new QString[n];
    s = sourceFiles.begin();
    while ( s != sourceFiles.end() && i < n ) {
	files[i++] = *s;
	++s;
    }
    qsort( files, n, sizeof(QString), compareMtime );
    i = n;
    while ( i-- > 0 )
	parseCppSourceFile( this, files[i] );
    delete[] files;
    files = 0;

    // pick up old output for the supervisor
    QStringList outputFiles;
    if ( config->supervisor() ) {
	outputFiles = config->findAll( QString("*.html"), config->outputDir() );
	s = outputFiles.begin();
	while ( s != outputFiles.end() ) {
	    parseHtmlFile( this, *s );
	    ++s;
	}
    }

    nailDownDocs();
    emitHtml();

    if ( config->lint() )
	lint();
}

void DocEmitter::addGroup( DefgroupDoc *doc )
{
    groupdefs.insert( doc->name(), doc );
    grmap.insert( doc->name(), doc->title() );
    addHtmlFile( doc->fileName() );
}

void DocEmitter::addGroupie( Doc *groupie )
{
    if ( config->isInternal() || !groupie->isInternal() ) {
	StringSet::ConstIterator group = groupie->groups().begin();
	while ( group != groupie->groups().end() ) {
	    groupiemap[*group].insert( QString("<a href=\"%1\">%2</a>")
					       .arg(groupie->fileName())
					       .arg(groupie->name()),
				       groupie );
	    ++group;
	}
    }
}

void DocEmitter::addPage( PageDoc *doc )
{
    pages.append( doc );
    addHtmlFile( doc->fileName() );
}

void DocEmitter::addExample( ExampleDoc *doc )
{
    examples.append( doc );
    addHtmlFile( config->verbatimHref(doc->fileName()) );
    eglist.insert( doc->fileName() );
}

void DocEmitter::addHtmlChunk( const QString& link, const HtmlChunk& chk )
{
    chkmap.insert( link, chk );
}

void DocEmitter::addLink( const QString& link, const QString& text )
{
    lmap[text].insert( link );
}

void DocEmitter::nailDownDecls()
{
    root.buildMangledSymbolTables();
    root.buildPlainSymbolTables( FALSE );
    root.fillInDecls();

    res = new DeclResolver( &root );
    Doc::setResolver( res );
}

void DocEmitter::nailDownDocs()
{
    root.fillInDocs();
    root.destructSymbolTables();
    root.buildPlainSymbolTables( TRUE );

    BinaryWriter headerfilesynonyms( QString("headerfilesynonyms") );

    /*
      Extract miscellaneous information about classes.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while ( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 &&
	     (config->isInternal() || !(*child)->isInternal()) ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;

	    /*
	      Fetch properties for class.
	    */
	    QValueList<PropertyDecl *>::ConstIterator p =
		    classDecl->properties().begin();
	    while ( p != classDecl->properties().end() ) {
		QString key = classDecl->name() + QChar( '/' ) + (*p)->name();
		QString link = config->classRefHref( classDecl->name() );
		if ( (*p)->doc() != 0 )
		    link += QChar( '#' ) + (*p)->ref();

		// avoid Q_OVERRIDE
		if ( !(*p)->readFunction().isEmpty() ) {
		    pmap.insert( key, link );
		    if ( (*p)->doc() != 0 )
			props.insert( (*p)->fullName(), *p );
		}
		++p;
	    }

	    /*
	      Add header files to list.
	    */
	    if ( !classDecl->headerFile().isEmpty() ) {
		hlist.insert( classDecl->headerFile() );

		if ( !classDecl->isObsolete() ) {
		    QString got = classDecl->headerFile();
		    QString expected = classDecl->name().lower() + ".h";
		    if ( expected != got && got.contains("_qws") == 0 &&
			 expected != "qt.h" )
			headerfilesynonyms.puts( (expected + " " + got + "\n")
						.latin1() );
		}
	    }
	    if ( classDecl->classDoc() != 0 &&
		 !classDecl->classDoc()->headers().isEmpty() )
		hlist = reunion( hlist, classDecl->classDoc()->headers() );

	    if ( !classDecl->isObsolete() && classDecl->classDoc() != 0 ) {
		clist.insert( classDecl->name(), classDecl->whatsThis() );
		if ( classDecl->classDoc()->mainClass() )
		    mainclist.insert( classDecl->name(),
				      classDecl->whatsThis() );
		wmap[classDecl->whatsThis()].insert( classDecl->name() );
	    }

	    QString parent;

	    /*
	      Build the class hierarchy.
	    */
	    if ( !classDecl->superTypes().isEmpty() ) {
		QString firstp = classDecl->superTypes().first().base();
		Decl *parentDecl = resolvePlain( firstp );
		if ( parentDecl != 0 )
		    parent = parentDecl->name();
	    }
	    chierarchy[parent].insert( classDecl->name() );

	    /*
	      Build the function index.
	    */
	    QValueList<Decl *>::ConstIterator grandChild =
		    classDecl->children().begin();
	    while ( grandChild != classDecl->children().end() ) {
		if ( (*grandChild)->kind() == Decl::Function ) {
		    FunctionDecl *funcDecl = (FunctionDecl *) *grandChild;
		    if ( !funcDecl->isConstructor() &&
			 !funcDecl->isDestructor() &&
			 !funcDecl->isInternal() &&
			 !funcDecl->isObsolete() )
			findex[funcDecl->name()].insert( classDecl->name() );
		}
		++grandChild;
	    }

	    addHtmlFile( config->classRefHref(classDecl->name()) );
	    addHtmlFile( config->classMembersHref(classDecl->name()) );
	}
	++child;
    }

    StringSet::ConstIterator s = hlist.begin();
    while ( s != hlist.end() ) {
	addHtmlFile( config->verbatimHref(*s) );
	++s;
    }
}

void DocEmitter::emitHtml() const
{
    QString htmlFileName;

    HtmlWriter::setStyle( config->style() );
    HtmlWriter::setPostHeader( config->postHeader() );
    HtmlWriter::setAddress( config->address() );

    res->setExampleFileList( eglist );
    res->setHeaderFileList( hlist );
    res->setHtmlFileList( htmllist );
    res->setHtmlChunkMap( chkmap );

    Doc::setHeaderFileList( hlist );
    Doc::setFunctionIndex( findex );
    Doc::setClassLists( clist, mainclist );
    Doc::setGroupMap( grmap );
    Doc::setClassHierarchy( chierarchy );

    XmlSection rootSection;
    QValueList<XmlSection> classSections;
    QValueList<XmlSection> exampleSections;
    QValueList<XmlSection> otherSections;

    /*
      Generate the verbatim header files.
    */
    StringSet::ConstIterator s = hlist.begin();
    while ( s != hlist.end() ) {
	htmlFileName = config->verbatimHref( *s );

	if ( config->generateFile(htmlFileName) ) {
	    QString headerFilePath =
		    config->findDepth( *s, config->includeDirList() );
	    if ( headerFilePath.isEmpty() )
		message( 1, "Cannot find header file '%s'", (*s).latin1() );
	    else
		emitHtmlHeaderFile( headerFilePath, htmlFileName );
	}
	++s;
    }

    /*
      Examples are generated first, so that the documentation can link
      to them.
    */
    QValueList<ExampleDoc *>::ConstIterator ex = examples.begin();
    while ( ex != examples.end() ) {
	htmlFileName = config->verbatimHref( (*ex)->fileName() );

	if ( config->generateFile(htmlFileName) ) {
	    HtmlWriter out( (*ex)->location(), htmlFileName );
	    if ( (*ex)->title().isEmpty() )
		out.setHeading( (*ex)->fileName() + QString(" Example File") );
	    else
		out.setHeading( (*ex)->title() );
	    (*ex)->printHtml( out );

	    QString fn = QStringList::split( '/', (*ex)->fileName() ).last();

	    XmlSection exampleSection;
	    exampleSection.title = out.heading();
	    exampleSection.ref = htmlFileName;
	    exampleSection.keywords.append( qMakePair(fn, htmlFileName) );
	    exampleSections.append( exampleSection );
	}
	++ex;
    }

    /*
      Generate class documentation.
    */
    QValueList<Decl *>::ConstIterator child = root.children().begin();
    while ( child != root.children().end() ) {
	if ( (*child)->kind() == Decl::Class && (*child)->doc() != 0 ) {
	    ClassDecl *classDecl = (ClassDecl *) *child;
	    htmlFileName = config->classRefHref( classDecl->name() );

	    if ( config->generateFile(htmlFileName) &&
		 (config->isInternal() || !classDecl->isInternal()) ) {
		res->setCurrentClass( classDecl );
		HtmlWriter out( (*child)->doc()->location(), htmlFileName );
		classDecl->printHtmlLong( out );

		XmlSection classSection;
		classSection.title = out.heading();
		classSection.ref = htmlFileName;

		XmlSection membersSection;
		membersSection.title = "List of All Member Functions";
		membersSection.ref =
			config->classMembersHref( classDecl->name() );
		appendXmlSubSection( &classSection, membersSection );

		if ( hlist.contains(classDecl->headerFile()) ) {
		    XmlSection headerSection;
		    headerSection.title = "Header File";
		    headerSection.ref =
			    config->verbatimHref( classDecl->headerFile() );
		    appendXmlSubSection( &classSection, headerSection );
		}

		classSections.append( classSection );
	    }
	}
	++child;
    }
    res->setCurrentClass( (ClassDecl *) 0 );

    QMap<QString, DefgroupDoc *>::ConstIterator def = groupdefs.begin();
    QMap<QString, QMap<QString, Doc *> >::ConstIterator groupies =
	    groupiemap.begin();
    QMap<QString, Doc *>::ConstIterator c;

    /*
      A COBOL programmer wrote this clever loop. If it weren't for C, he would
      be programming in OBOL.
    */
    while ( def != groupdefs.end() || groupies != groupiemap.end() ) {
	if ( def != groupdefs.end() ) {
	    if ( groupies == groupiemap.end() || def.key() < groupies.key() ) {
		warning( 2, (*def)->location(), "Empty group '%s'",
			 def.key().latin1() );
		++def;
	    }
	}
	if ( groupies != groupiemap.end() ) {
	    if ( def == groupdefs.end() || groupies.key() < def.key() ) {
		c = (*groupies).begin();
		while ( c != (*groupies).end() ) {
		    if ( *c != 0 )
			warning( 3, (*c)->location(),
				 "Undefined group '%s'",
				 groupies.key().latin1() );
		    ++c;
		}
		++groupies;
	    } else if ( groupies.key() == def.key() ) {
		/*
		  Bingo! At this point *def is the doc and *groupies is a QMap
		  with class- or page-name keys and Doc * values.
		*/
		htmlFileName = config->defgroupHref( (*def)->name() );

		if ( config->generateFile(htmlFileName) ) {
		    HtmlWriter out( (*def)->location(), htmlFileName );
		    out.setHeading( (*def)->title() );
		    (*def)->printHtml( out );

		    QMap<QString, QString> list;
		    c = (*groupies).begin();
		    while ( c != (*groupies).end() ) {
			list.insert( c.key(), (*c)->whatsThis() );
			++c;
		    }
		    out.putsMeta( Doc::htmlNormalList(list) );

		    XmlSection defgroupSection;
		    defgroupSection.title = out.heading();
		    defgroupSection.ref = htmlFileName;
		    otherSections.append( defgroupSection );
		}
		++def;
		++groupies;
	    }
	}
    }

    QValueList<PageDoc *>::ConstIterator pa = pages.begin();
    while ( pa != pages.end() ) {
	htmlFileName = (*pa)->fileName();

	if ( config->generateFile(htmlFileName) ) {
	    HtmlWriter out( (*pa)->location(), htmlFileName );
	    out.setHeading( (*pa)->title() );
	    (*pa)->printHtml( out );

	    XmlSection pageSection;
	    pageSection.title = out.heading();
	    pageSection.ref = htmlFileName;
	    otherSections.append( pageSection );
	}
	++pa;
    }

    /*
      Write four special files: index, propertyindex, titleindex, and
      whatsthis.
    */
    if ( lmap.count() > 1 ) {
	BinaryWriter index( QString("index") );
	QMap<QString, StringSet>::ConstIterator x = lmap.begin();
	while ( x != lmap.end() ) {
	    StringSet::ConstIterator s = (*x).begin();
	    while ( s != (*x).end() ) {
		index.puts( QString("\"%1\" %2\n")
				    .arg(protect(x.key(), QChar('"')))
				    .arg(*s)
				    .latin1() );
		++s;
	    }
	    ++x;
	}
    }

    if ( pmap.count() > 0 ) {
	BinaryWriter propertyindex( QString("propertyindex") );
	QMap<QString, QString>::ConstIterator p = pmap.begin();
	while ( p != pmap.end() ) {
	    propertyindex.puts( QString("\"%1\" %2\n")
					.arg(p.key()).arg(*p)
					.latin1() );
	    ++p;
	}
    }

    if ( HtmlWriter::titleMap().count() > 1 ) {
	BinaryWriter titleindex( QString("titleindex") );
	QMap<QString, StringSet>::ConstIterator t =
		HtmlWriter::titleMap().begin();
	while ( t != HtmlWriter::titleMap().end() ) {
	    StringSet::ConstIterator s = (*t).begin();
	    while ( s != (*t).end() ) {
		titleindex.puts( QString("%1 | %2\n")
					 .arg(protect(t.key(), QChar('|')))
					 .arg(*s)
					 .latin1() );
		++s;
	    }
	    ++t;
	}
    }

    if ( wmap.count() > 1 ) {
	BinaryWriter whatsthis( QString("whatsthis") );
	QMap<QString, StringSet>::ConstIterator w = wmap.begin();
	while ( w != wmap.end() ) {
	    StringSet::ConstIterator s = (*w).begin();
	    while ( s != (*w).end() ) {
		whatsthis.puts( QString("%1. | %2\n")
					.arg(protect(w.key(), QChar('|')))
					.arg(*s)
					.latin1() );
		++s;
	    }
	    ++w;
	}
    }

    /*
      Write another special file: propertydocs. It's an XML file.
    */
    if ( props.count() > 0 ) {
	BinaryWriter propertydocs( QString("propertydocs") );

	propertydocs.puts( "<!DOCTYPE PROP><PROP>\n" );

	QMap<QString, PropertyDecl *>::ConstIterator p = props.begin();
	while ( p != props.end() ) {
	    QString className = (*p)->context()->name();

	    propertydocs.puts( "<property>\n    <name>" );
	    propertydocs.puts( p.key().latin1() );

	    res->setCurrentClass( 0 );
	    propertydocs.puts( QString("</name>\n    <doc href=\"%1\">")
			       .arg(res->resolve((*p)->fullName()))
			       .latin1() );

	    res->setCurrentClass( (ClassDecl *) (*p)->context() );
	    propertydocs.puts(
		    fixedPropertyDoc((*p)->propertyDoc()->finalHtml(),
				     className).latin1() );
	    propertydocs.puts( "</doc>\n</property>\n" );
	    ++p;
	}
	propertydocs.puts( "</PROP>\n" );
    }

    /*
      Write the <product>.xml file for Assistant.
    */
    qHeapSort( classSections );
    qHeapSort( otherSections );
    qHeapSort( exampleSections );
    appendXmlSubSections( &rootSection, classSections );
    appendXmlSubSections( &rootSection, otherSections );
    appendXmlSubSections( &rootSection, exampleSections );

    if ( lmap.count() > 1 ) {
	QMap<QString, QValueList<QPair<QString, QString> > > keywordMap;
	QMap<QString, StringSet>::ConstIterator ell = lmap.begin();
	while ( ell != lmap.end() ) {
	    QString keyword = QStringList::split( "::", ell.key() ).last();
	    StringSet::ConstIterator s = (*ell).begin();
	    while ( s != (*ell).end() ) {
		QString link = *s;
		QString ref = QStringList::split( "#", link ).first();
		keywordMap[ref].append( qMakePair(keyword, link) );
		++s;
	    }
	    ++ell;
	}
	QValueList<XmlSection>::Iterator ss = rootSection.subsections->begin();
	while ( ss != rootSection.subsections->end() ) {
	    (*ss).keywords += keywordMap[(*ss).ref];
	    ++ss;
	}
    }

    rootSection.title = config->product() + " Reference Documentation";
    rootSection.ref = "index.html";
    generateXmlSections( rootSection, config->product().lower() + ".xml",
			 config->product().lower() + "/reference" );
}

void DocEmitter::addHtmlFile( const QString& fileName )
{
    if ( htmllist.contains(fileName) )
	message( 1, "HTML file '%s' overwritten", fileName.latin1() );
    else if ( fileName.right(5) != QString(".html") )
	message( 1, "HTML file '%s' lacking '.html' extension",
		 fileName.latin1() );
    htmllist.insert( fileName );
}

/*
  
*/

static StringSet anames;
static StringSet ahrefs;

static void lintHtmlFile( const QString& filePath )
{
    static QRegExp anchor( QString(
	    "<a[ \n\t]+(name|href)[ \n\t]*=[ \n\t]*"
	    "(\"[^\"]+\"|[^\"> \n\t]+)[ \n\t]*>") );

    QFile f( filePath );
    if ( !f.open(IO_ReadOnly) ) {
	message( 0, "Cannot open file '%s'", filePath.latin1() );
	return;
    }

    QTextStream t( &f );
    QString fullText = t.read();
    f.close();

    QString fileName = filePath.mid( filePath.findRev(QChar('/')) + 1 );
    anames.insert( fileName );

    int k = 0;
    while ( (k = anchor.search(fullText, k)) != -1 ) {
	QString link = anchor.cap( 2 );
	if ( link[0] == QChar('"') )
	    link = link.mid( 1, link.length() - 2 );
	link = link.lower();

	if ( anchor.cap(1) == QString("name") ) {
	    int n = anames.count();
	    anames.insert( fileName + QChar('#') + link );
	    if ( n == (int) anames.count() )
		message( 0, "Two '<a name=%s>' in file '%s' (case insensitive)",
			 link.latin1(), fileName.latin1() );
	} else {
	    if ( link[0] == QChar('#') )
		ahrefs.insert( fileName + link );
	    else
		ahrefs.insert( link );
	}
	k += anchor.matchedLength();
    }
}

void DocEmitter::lint() const
{
    StringSet::ConstIterator h = htmllist.begin();
    while ( h != htmllist.end() ) {
	lintHtmlFile( config->outputDir() + QChar('/') + *h );
	++h;
    }

    StringSet diff = difference( ahrefs, anames );
    StringSet::ConstIterator href = diff.begin();
    while ( href != diff.end() ) {
	if ( !(*href).startsWith("file:") && !(*href).startsWith("ftp:") &&
	     !(*href).startsWith("http:") && !(*href).startsWith("mailto:") )
	    message( 0, "Broken link '%s'", (*href).latin1() );
	++href;
    }
}
