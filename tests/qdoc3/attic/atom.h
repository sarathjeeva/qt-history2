/*
  atom.h
*/

#ifndef ATOM_H
#define ATOM_H

#include <qstring.h>

class Atom
{
public:
    enum Type { AbstractBegin, AbstractEnd, Alias, AliasArg, BaseName,
		BriefBegin, BriefEnd, C, CaptionBegin,
		CaptionEnd, CitationBegin, CitationEnd, Code,
		FootnoteBegin,
		FootnoteEnd, FormatBegin, FormatEnd, GeneratedList,
		Image, Link, LinkNode, ListBegin, ListItemNumber,
		ListItemBegin, ListItemEnd, ListEnd, Nop, ParagraphBegin,
		ParagraphEnd, RawFormat, RawString, SectionBegin, SectionEnd,
		SectionHeadingBegin, SectionHeadingEnd,
		SidebarBegin, SidebarEnd,
		String, TableBegin, TableEnd, TableOfContents,
		Target, UnknownCommand, Last = UnknownCommand };

    Atom( Type type, const QString& string = "" )
	: nex( 0 ), typ( type ), str( string ) { }
    Atom( Atom *prev, Type type, const QString& string = "" )
	: nex( prev->nex ), typ( type ), str( string ) { prev->nex = this; }

    void appendChar( QChar ch ) { str += ch; }
    void appendString( const QString& string ) { str += string; }
    void chopString() { str.truncate( str.length() - 1 ); }
    Atom *next() { return nex; }

    const Atom *next() const { return nex; }
    Type type() const { return typ; }
    QString typeString() const;
    const QString& string() const { return str; }

private:
    Atom *nex;
    Type typ;
    QString str;
};

#endif
