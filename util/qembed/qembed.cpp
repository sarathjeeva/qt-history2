/****************************************************************************
** $Id: //depot/qt/main/util/qembed/qembed.cpp#11 $
**
** Utility program for embedding binary data into a C/C++ source code.
** It reads a binary file and generates a C array with the binary data.
** The C code is written to standard output.
**
** Author  : Haavard Nord
** Created : 951017
**
** Copyright (C) 1995-1998 by Troll Tech AS.  All rights reserved.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that this copyright notice appears in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#include <qstring.h>
#include <qfile.h>
#include <qlist.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <ctype.h>
#include <stdlib.h>

void    embedData( const QByteArray &input, QFile *output );
QString convertFileNameToCIdentifier( const char * );

char header[] = "/* Generated by qembed */\n";

struct Embed {
	uint    size;
	QString name;
	QString cname;
};

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	qWarning( "Usage:\n\t%s files", argv[0] );
	return 1;
    }

    QFile output;
    bool  output_hdr = FALSE;
    output.open( IO_WriteOnly, stdout );
    QTextStream cout( &output );


    QList<Embed> list;
    list.setAutoDelete( TRUE );

  // Embed data for all input files

    long l = random();
    cout << "#ifndef _" << l << endl;
    cout << "#define _" << l << endl;

    for ( int i=1; i<argc; i++ ) {
	QFile f( argv[i] );
	if ( !f.open(IO_ReadOnly) ) {
	    qWarning( "Cannot open file %s, ignoring it", argv[i] );
	    continue;
	}
	QByteArray a( f.size() );
	if ( f.readBlock(a.data(), f.size()) != (int)f.size() ) {
	    qWarning( "Cannot read file %s, ignoring it", argv[i] );
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
	cout << s.sprintf( "static const unsigned int  %s_len = %d;\n",
			   (const char *)e->cname, e->size );
	cout << s.sprintf( "static const unsigned char %s_data[] = {",
			   (const char *)e->cname );
	embedData( a, &output );
	cout << "\n};\n\n";
	f.close();
    }

  // Generate summery

    if ( list.count() > 0 ) {
	cout << "struct Embed {\n    unsigned int         size;";
	cout << "\n    const unsigned char *data;";
	cout << "\n    const char          *name;\n} embed_vec[] = {\n";
	Embed *e = list.first();
	while ( e ) {
	    cout << "    { " << e->size << ", " << e->cname << "_data, "
		 << "\"" << e->name << "\" },\n";
	    e = list.next();
	}
	cout << "    { 0, 0, 0 }\n};\n";
    }

    cout << "#endif" << endl;

    return 0;
}


QString convertFileNameToCIdentifier( const char *s )
{
    QString r = s;
    int len = r.length();
    if ( len > 0 && !isalpha( (char)r[0].latin1() ) )
	r[0] = '_';
    for ( int i=1; i<len; i++ ) {
	if ( !isalnum( (char)r[i].latin1() ) )
	    r[i] = '_';
    }
    return r;
}


void embedData( const QByteArray &input, QFile *output )
{
    static char hexdigits[] = "0123456789abcdef";
    QString s;
    int nbytes = input.size();
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    output->writeBlock( (const char*)s, s.length() );
	    s.truncate( 0 );
	}
	int v = (int)((uchar)input[i]);
	s += "0x";
	s += hexdigits[(v >> 4) & 15];
	s += hexdigits[v & 15];
	if ( i < nbytes-1 )
	    s += ',';
    }
    if ( s.length() )
	output->writeBlock( (const char*)s, s.length() );
}
