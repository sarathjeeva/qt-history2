/****************************************************************************
**
** Implementation of the internal Qt classes dealing with rich text.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qrichtext_p.h"

#ifndef QT_NO_RICHTEXT

QTextCommand::~QTextCommand() {}
QTextCommand::Commands QTextCommand::type() const { return Invalid; }


#ifndef QT_NO_TEXTCUSTOMITEM
Q3TextCustomItem::~Q3TextCustomItem() {}
void Q3TextCustomItem::adjustToPainter( QPainter* p){ if ( p ) width = 0; }
Q3TextCustomItem::Placement Q3TextCustomItem::placement() const { return PlaceInline; }

bool Q3TextCustomItem::ownLine() const { return FALSE; }
void Q3TextCustomItem::resize( int nwidth ){ width = nwidth; }
void Q3TextCustomItem::invalidate() {}

bool Q3TextCustomItem::isNested() const { return FALSE; }
int Q3TextCustomItem::minimumWidth() const { return 0; }

QString Q3TextCustomItem::richText() const { return QString::null; }

bool Q3TextCustomItem::enter( Q3TextCursor *, Q3TextDocument*&, Q3TextParagraph *&, int &, int &, int &, bool )
{
    return TRUE;
}
bool Q3TextCustomItem::enterAt( Q3TextCursor *, Q3TextDocument *&, Q3TextParagraph *&, int &, int &, int &, const QPoint & )
{
    return TRUE;
}
bool Q3TextCustomItem::next( Q3TextCursor *, Q3TextDocument *&, Q3TextParagraph *&, int &, int &, int & )
{
    return TRUE;
}
bool Q3TextCustomItem::prev( Q3TextCursor *, Q3TextDocument *&, Q3TextParagraph *&, int &, int &, int & )
{
    return TRUE;
}
bool Q3TextCustomItem::down( Q3TextCursor *, Q3TextDocument *&, Q3TextParagraph *&, int &, int &, int & )
{
    return TRUE;
}
bool Q3TextCustomItem::up( Q3TextCursor *, Q3TextDocument *&, Q3TextParagraph *&, int &, int &, int & )
{
    return TRUE;
}
#endif // QT_NO_TEXTCUSTOMITEM

void Q3TextFlow::setPageSize( int ps ) { pagesize = ps; }
#ifndef QT_NO_TEXTCUSTOMITEM
bool Q3TextFlow::isEmpty() { return leftItems.isEmpty() && rightItems.isEmpty(); }
#else
bool Q3TextFlow::isEmpty() { return TRUE; }
#endif

#ifndef QT_NO_TEXTCUSTOMITEM
void Q3TextTableCell::invalidate() { cached_width = -1; cached_sizehint = -1; }

void Q3TextTable::invalidate() { cachewidth = -1; }
#endif

Q3TextParagraphData::~Q3TextParagraphData() {}
void Q3TextParagraphData::join( Q3TextParagraphData * ) {}

Q3TextFormatter::~Q3TextFormatter() {}
void Q3TextFormatter::setWrapEnabled( bool b ) { wrapEnabled = b; }
void Q3TextFormatter::setWrapAtColumn( int c ) { wrapColumn = c; }



int Q3TextCursor::x() const
{
    if ( idx >= para->length() )
	return 0;
    Q3TextStringChar *c = para->at( idx );
    int curx = c->x;
    if ( !c->rightToLeft &&
	 c->c.isSpace() &&
	 idx > 0 &&
	 para->at( idx - 1 )->c != '\t' &&
	 !c->lineStart &&
	 ( para->alignment() & Qt::AlignJustify ) == Qt::AlignJustify )
	curx = para->at( idx - 1 )->x + para->string()->width( idx - 1 );
    if ( c->rightToLeft )
	curx += para->string()->width( idx );
    return curx;
}

int Q3TextCursor::y() const
{
    int dummy, line;
    para->lineStartOfChar( idx, &dummy, &line );
    return para->lineY( line );
}

int Q3TextCursor::globalX() const { return totalOffsetX() + para->rect().x() + x(); }
int Q3TextCursor::globalY() const { return totalOffsetY() + para->rect().y() + y(); }

Q3TextDocument *Q3TextCursor::document() const
{
    return para ? para->document() : 0;
}

void Q3TextCursor::gotoPosition( Q3TextParagraph* p, int index )
{
    if ( para && p != para ) {
	while ( para->document() != p->document() && !indices.isEmpty() )
	    pop();
	Q_ASSERT( indices.isEmpty() || para->document() == p->document() );
    }
    para = p;
    if ( index < 0 || index >= para->length() ) {
	qWarning( "Q3TextCursor::gotoParagraph Index: %d out of range", index );
	if ( index < 0 || para->length() == 0 )
	    index = 0;
	else
	    index = para->length() - 1;
    }

    tmpX = -1;
    idx = index;
    fixCursorPosition();
}

bool Q3TextDocument::hasSelection( int id, bool visible ) const
{
    return ( selections.find( id ) != selections.end() &&
	     ( !visible ||
	       ( (Q3TextDocument*)this )->selectionStartCursor( id ) !=
	       ( (Q3TextDocument*)this )->selectionEndCursor( id ) ) );
}

void Q3TextDocument::setSelectionStart( int id, const Q3TextCursor &cursor )
{
    Q3TextDocumentSelection sel;
    sel.startCursor = cursor;
    sel.endCursor = cursor;
    sel.swapped = FALSE;
    selections[ id ] = sel;
}

Q3TextParagraph *Q3TextDocument::paragAt( int i ) const
{
    Q3TextParagraph* p = curParag;
    if ( !p || p->paragId() > i )
	p = fParag;
    while ( p && p->paragId() != i )
	p = p->next();
    ((Q3TextDocument*)this)->curParag = p;
    return p;
}


Q3TextFormat::~Q3TextFormat()
{
}

Q3TextFormat::Q3TextFormat()
    : fm( QFontMetrics( fn ) ), linkColor( TRUE ), logicalFontSize( 3 ), stdSize( qApp->font().pointSize() )
{
    ref = 0;

    usePixelSizes = FALSE;
    if ( stdSize == -1 ) {
	stdSize = qApp->font().pixelSize();
	usePixelSizes = TRUE;
    }

    missp = FALSE;
    ha = AlignNormal;
    collection = 0;
}

Q3TextFormat::Q3TextFormat( const QStyleSheetItem *style )
    : fm( QFontMetrics( fn ) ), linkColor( TRUE ), logicalFontSize( 3 ), stdSize( qApp->font().pointSize() )
{
    ref = 0;

    usePixelSizes = FALSE;
    if ( stdSize == -1 ) {
	stdSize = qApp->font().pixelSize();
	usePixelSizes = TRUE;
    }

    missp = FALSE;
    ha = AlignNormal;
    collection = 0;
    fn = QFont( style->fontFamily(),
		style->fontSize(),
		style->fontWeight(),
		style->fontItalic() );
    fn.setUnderline( style->fontUnderline() );
    fn.setStrikeOut( style->fontStrikeOut() );
    col = style->color();
    fm = QFontMetrics( fn );
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.lineSpacing();
    asc = fm.ascent() + (fm.leading()+1)/2;
    dsc = fm.descent();
    missp = FALSE;
    ha = AlignNormal;
    memset( widths, 0, 256 );
    generateKey();
    addRef();
}

Q3TextFormat::Q3TextFormat( const QFont &f, const QColor &c, Q3TextFormatCollection *parent )
    : fn( f ), col( c ), fm( QFontMetrics( f ) ), linkColor( TRUE ),
      logicalFontSize( 3 ), stdSize( f.pointSize() )
{
    ref = 0;
    usePixelSizes = FALSE;
    if ( stdSize == -1 ) {
	stdSize = f.pixelSize();
	usePixelSizes = TRUE;
    }
    collection = parent;
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.lineSpacing();
    asc = fm.ascent() + (fm.leading()+1)/2;
    dsc = fm.descent();
    missp = FALSE;
    ha = AlignNormal;
    memset( widths, 0, 256 );
    generateKey();
    addRef();
}

Q3TextFormat::Q3TextFormat( const Q3TextFormat &f )
    : fm( f.fm )
{
    ref = 0;
    collection = 0;
    fn = f.fn;
    col = f.col;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdSize = f.stdSize;
    usePixelSizes = f.usePixelSizes;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    ha = f.ha;
    k = f.k;
    linkColor = f.linkColor;
    addRef();
}

Q3TextFormat& Q3TextFormat::operator=( const Q3TextFormat &f )
{
    ref = 0;
    collection = f.collection;
    fn = f.fn;
    col = f.col;
    fm = f.fm;
    leftBearing = f.leftBearing;
    rightBearing = f.rightBearing;
    memset( widths, 0, 256 );
    hei = f.hei;
    asc = f.asc;
    dsc = f.dsc;
    stdSize = f.stdSize;
    usePixelSizes = f.usePixelSizes;
    logicalFontSize = f.logicalFontSize;
    missp = f.missp;
    ha = f.ha;
    k = f.k;
    linkColor = f.linkColor;
    addRef();
    return *this;
}

void Q3TextFormat::update()
{
    fm = QFontMetrics( fn );
    leftBearing = fm.minLeftBearing();
    rightBearing = fm.minRightBearing();
    hei = fm.lineSpacing();
    asc = fm.ascent() + (fm.leading()+1)/2;
    dsc = fm.descent();
    memset( widths, 0, 256 );
    generateKey();
}


QPainter* Q3TextFormat::pntr = 0;
QFontMetrics* Q3TextFormat::pntr_fm = 0;
int Q3TextFormat::pntr_ldg=-1;
int Q3TextFormat::pntr_asc=-1;
int Q3TextFormat::pntr_hei=-1;
int Q3TextFormat::pntr_dsc=-1;

void Q3TextFormat::setPainter( QPainter *p )
{
    pntr = p;
}

QPainter*  Q3TextFormat::painter()
{
    return pntr;
}

void Q3TextFormat::applyFont( const QFont &f )
{
    QFontMetrics fm( pntr->fontMetrics() );
    if ( !pntr_fm
	|| pntr_fm->painter != pntr
	|| pntr_fm->d != fm.d
	|| !pntr->font().isCopyOf( f ) ) {
	pntr->setFont( f );
	delete pntr_fm;
	pntr_fm = new QFontMetrics( pntr->fontMetrics() );
	pntr_ldg = pntr_fm->leading();
	pntr_asc = pntr_fm->ascent()+(pntr_ldg+1)/2;
	pntr_hei = pntr_fm->lineSpacing();
	pntr_dsc = -1;
    }
}

int Q3TextFormat::minLeftBearing() const
{
    if ( !pntr || !pntr->isActive() )
	return leftBearing;
    applyFont( fn );
    return pntr_fm->minLeftBearing();
}

int Q3TextFormat::minRightBearing() const
{
    if ( !pntr || !pntr->isActive() )
	return rightBearing;
    applyFont( fn );
    return pntr_fm->minRightBearing();
}

int Q3TextFormat::height() const
{
    if ( !pntr || !pntr->isActive() )
	return hei;
    applyFont( fn );
    return pntr_hei;
}

int Q3TextFormat::ascent() const
{
    if ( !pntr || !pntr->isActive() )
	return asc;
    applyFont( fn );
    return pntr_asc;
}

int Q3TextFormat::descent() const
{
    if ( !pntr || !pntr->isActive() )
	return dsc;
    applyFont( fn );
    if ( pntr_dsc < 0 )
	pntr_dsc = pntr_fm->descent();
    return pntr_dsc;
}

int Q3TextFormat::leading() const
{
    if ( !pntr || !pntr->isActive() )
	return fm.leading();
    applyFont( fn );
    return pntr_ldg;
}

void Q3TextFormat::generateKey()
{
    k = getKey( fn, col, isMisspelled(), vAlign() );
}

QString Q3TextFormat::getKey( const QFont &fn, const QColor &col, bool misspelled, VerticalAlignment a )
{
    QString k = fn.key();
    k += '/';
    k += QString::number( (uint)col.rgb() );
    k += '/';
    k += QString::number( (int)misspelled );
    k += '/';
    k += QString::number( (int)a );
    return k;
}

QString Q3TextString::toString( const QVector<Q3TextStringChar> &data )
{
    QString s;
    int l = data.size();
    s.setUnicode( 0, l );
    const Q3TextStringChar *c = data.data();
    QChar *uc = (QChar *)s.unicode();
    while ( l-- )
	*(uc++) = (c++)->c;

    return s;
}

void Q3TextParagraph::setSelection( int id, int start, int end )
{
    QMap<int, Q3TextParagraphSelection>::ConstIterator it = selections().find( id );
    if ( it != mSelections->end() ) {
	if ( start == ( *it ).start && end == ( *it ).end )
	    return;
    }

    Q3TextParagraphSelection sel;
    sel.start = start;
    sel.end = end;
    (*mSelections)[ id ] = sel;
    setChanged( TRUE, TRUE );
}

void Q3TextParagraph::removeSelection( int id )
{
    if ( !hasSelection( id ) )
	return;
    if ( mSelections )
	mSelections->remove( id );
    setChanged( TRUE, TRUE );
}

int Q3TextParagraph::selectionStart( int id ) const
{
    if ( !mSelections )
	return -1;
    QMap<int, Q3TextParagraphSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return -1;
    return ( *it ).start;
}

int Q3TextParagraph::selectionEnd( int id ) const
{
    if ( !mSelections )
	return -1;
    QMap<int, Q3TextParagraphSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return -1;
    return ( *it ).end;
}

bool Q3TextParagraph::hasSelection( int id ) const
{
    return mSelections ? mSelections->contains( id ) : FALSE;
}

bool Q3TextParagraph::fullSelected( int id ) const
{
    if ( !mSelections )
	return FALSE;
    QMap<int, Q3TextParagraphSelection>::ConstIterator it = mSelections->find( id );
    if ( it == mSelections->end() )
	return FALSE;
    return ( *it ).start == 0 && ( *it ).end == str->length() - 1;
}

int Q3TextParagraph::lineY( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "Q3TextParagraph::lineY: line %d out of range!", l );
	return 0;
    }

    if ( !isValid() )
	( (Q3TextParagraph*)this )->format();

    QMap<int, QTextLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->y;
}

int Q3TextParagraph::lineBaseLine( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "Q3TextParagraph::lineBaseLine: line %d out of range!", l );
	return 10;
    }

    if ( !isValid() )
	( (Q3TextParagraph*)this )->format();

    QMap<int, QTextLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->baseLine;
}

int Q3TextParagraph::lineHeight( int l ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "Q3TextParagraph::lineHeight: line %d out of range!", l );
	return 15;
    }

    if ( !isValid() )
	( (Q3TextParagraph*)this )->format();

    QMap<int, QTextLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    return ( *it )->h;
}

void Q3TextParagraph::lineInfo( int l, int &y, int &h, int &bl ) const
{
    if ( l > (int)lineStarts.count() - 1 ) {
	qWarning( "Q3TextParagraph::lineInfo: line %d out of range!", l );
	qDebug( "%d %d", (int)lineStarts.count() - 1, l );
	y = 0;
	h = 15;
	bl = 10;
	return;
    }

    if ( !isValid() )
	( (Q3TextParagraph*)this )->format();

    QMap<int, QTextLineStart*>::ConstIterator it = lineStarts.begin();
    while ( l-- > 0 )
	++it;
    y = ( *it )->y;
    h = ( *it )->h;
    bl = ( *it )->baseLine;
}


void Q3TextParagraph::setAlignment( int a )
{
    if ( a == (int)align )
	return;
    align = a;
    invalidate( 0 );
}

Q3TextFormatter *Q3TextParagraph::formatter() const
{
    if ( hasdoc )
	return document()->formatter();
    if ( pseudoDocument()->pFormatter )
	return pseudoDocument()->pFormatter;
    return ( ( (Q3TextParagraph*)this )->pseudoDocument()->pFormatter = new Q3TextFormatterBreakWords );
}

void Q3TextParagraph::setTabArray( int *a )
{
    delete [] tArray;
    tArray = a;
}

void Q3TextParagraph::setTabStops( int tw )
{
    if ( hasdoc )
	document()->setTabStops( tw );
    else
	tabStopWidth = tw;
}

QMap<int, Q3TextParagraphSelection> &Q3TextParagraph::selections() const
{
    if ( !mSelections )
	((Q3TextParagraph *)this)->mSelections = new QMap<int, Q3TextParagraphSelection>;
    return *mSelections;
}

#ifndef QT_NO_TEXTCUSTOMITEM
QList<Q3TextCustomItem *> &Q3TextParagraph::floatingItems() const
{
    if ( !mFloatingItems )
	((Q3TextParagraph *)this)->mFloatingItems = new QList<Q3TextCustomItem *>;
    return *mFloatingItems;
}
#endif

Q3TextStringChar::~Q3TextStringChar()
{
    if ( format() )
	format()->removeRef();
    if ( type ) // not Regular
	delete p.custom;
}

Q3TextParagraphPseudoDocument::Q3TextParagraphPseudoDocument():pFormatter(0),commandHistory(0), minw(0),wused(0),collection(){}
Q3TextParagraphPseudoDocument::~Q3TextParagraphPseudoDocument(){ delete pFormatter; delete commandHistory; }


#endif //QT_NO_RICHTEXT
