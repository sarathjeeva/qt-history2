/****************************************************************************
** $Id: //depot/qt/main/util/qembed/qembed.cpp#1 $
**
** Utility program for embedding binary data into a C/C++ source code.
**
** Author  : Haavard Nord
** Created : 951017
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This utility reads a binary file and generates a C array with the binary
** data.  The C code is written to standard output.
*****************************************************************************/

#include <qstring.h>
#include <qfile.h>
#include <qlist.h>
#include <qtstream.h>
#include <qdatetm.h>
#include <ctype.h>

RCSTAG("$Id $")


void    embedData( const QByteArray &input, QFile *output );
QString convertFileNameToCIdentifier( const char * );

char header[] = "/* Generated by qembed */\n";


int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	warning( "Usage:\n\t%s files", argv[0] );
	return 1;
    }

    QFile output;
    bool  output_hdr = FALSE;
    output.open( IO_WriteOnly, stdout );
    QTextStream cout( &output );

    struct Embed {
	uint    size;
	QString name;
	QString cname;
    };
    QListT<Embed> list;
    list.setAutoDelete( TRUE );

  // Embed data for all input files

    for ( int i=1; i<argc; i++ ) {
	QFile f( argv[i] );
	if ( !f.open(IO_ReadOnly) ) {
	    warning( "Cannot open file %s, ignoring it", argv[i] );
	    continue;
	}
	QByteArray a( f.size() );
	if ( f.readBlock(a.data(), f.size()) != f.size() ) {
	    warning( "Cannot read file %s, ignoring it", argv[i] );
	    f.close();
	    continue;
	}
	Embed *e = new Embed;
	e->size = f.size();
	e->name = argv[i];
	e->cname = convertFileNameToCIdentifier( argv[i] );
	list.append( e );

	if ( !output_hdr ) {
	    output_hdr = TRUE;
	    output.writeBlock( header, strlen(header) );
	}

	QString s;
	cout << s.sprintf( "static unsigned int  %s_len = %d;\n",
			   (const char *)e->cname, e->size );
	cout << s.sprintf( "static unsigned char %s_data[] = {",
			   (const char *)e->cname );
	embedData( a, &output );
	cout << "\n};\n\n";
	f.close();
    }

  // Generate summery

    if ( list.count() > 0 ) {
	cout << "struct {\n    unsigned int   size;";
	cout << "\n    unsigned char *data;";
	cout << "\n    const char    *name;\n} embed_vec[] = {\n";
	Embed *e = list.first();
	while ( e ) {
	    cout << "    { " << e->size << ", " << e->cname << "_data, "
		 << "\"" << e->name << "\" },\n";
	    e = list.next();
	}
	cout << "    { 0, 0 }\n};\n";
    }

    return 0;
}


QString convertFileNameToCIdentifier( const char *s )
{
    QString r = s;
    int len = r.length();
    for ( int i=0; i<len; i++ ) {
	if ( !isalnum(r[i]) )
	    r[i] = '_';
    }
    return r;
}


void embedData( const QByteArray &input, QFile *output )
{
    static char hexdigits[] = "0123456789abcdef";
    QString s( 100 );
    int nbytes = input.size();
    char *p = s.data();
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    strcpy( p, "\n    " );
	    output->writeBlock( s.data(), s.length() );
	    p = s.data();
	}
	int v = (int)((uchar)input[i]);
	*p++ = '0';
	*p++ = 'x';
	*p++ = hexdigits[(v >> 4) & 15];
	*p++ = hexdigits[v & 15];
	if ( i < nbytes-1 )
	    *p++ = ',';
    }
    if ( p > s.data() ) {
	*p = '\0';
	output->writeBlock( s.data(), s.length() );
    }
}
