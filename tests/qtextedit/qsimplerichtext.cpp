#include "qsimplerichtext.h"
#include "qrichtext_p.h"
#include "qapplication.h"

class QSimpleRichTextData
{
public:
    QTextDocument *doc;
};

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context, const QStyleSheet* sheet )
{
    d = new QSimpleRichTextData;
    d->doc = new QTextDocument( 0 );
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->setStyleSheet( sheet );
    d->doc->setText( text, context );
}

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context,  const QStyleSheet* sheet,
				  const QMimeSourceFactory* factory, int verticalBreak,
				  const QColor& linkColor, bool linkUnderline )
{
    d = new QSimpleRichTextData;
    d->doc = new QTextDocument( 0 );
    d->doc->setFormatter( new QTextFormatterBreakWords( d->doc ) );
    d->doc->setDefaultFont( fnt );
    d->doc->flow()->setPageSize( verticalBreak );
    d->doc->setVerticalBreak( TRUE );
    d->doc->setStyleSheet( sheet );
    d->doc->setMimeSourceFactory( factory );
    d->doc->setLinkColor( linkColor );
    d->doc->setUnderlineLinks( linkUnderline );
    d->doc->setText( text, context );
}

QSimpleRichText::~QSimpleRichText()
{
}

void QSimpleRichText::setWidth( int w )
{
    d->doc->doLayout( 0, w );
}

void QSimpleRichText::setWidth( QPainter *p, int w )
{
    d->doc->doLayout( p, w );
}

int QSimpleRichText::width() const
{
    return d->doc->width();
}

int QSimpleRichText::widthUsed() const
{
    return d->doc->widthUsed();
}

int QSimpleRichText::height() const
{
    return d->doc->height();
}

static uint int_sqrt(uint n)
{
    uint h, p= 0, q= 1, r= n;
    ASSERT( n < 1073741824U );  // UINT_MAX>>2 on 32-bits architecture
    while ( q <= n )
	q <<= 2;
    while ( q != 1 ) {
	q >>= 2;
	h= p + q;
	p >>= 1;
	if ( r >= h ) {
	    p += q;
	    r -= h;
	}
    }
    return p;
}

void QSimpleRichText::adjustSize()
{
    int mw = QApplication::desktop()->width();
    int w = mw;
    d->doc->doLayout( 0,w );
    w = int_sqrt( (5*d->doc->height()) / (3*d->doc->widthUsed() ) );
    d->doc->doLayout( 0, QMIN( w, mw) );

    if ( w*3 < 5*d->doc->height() ) {
	w = int_sqrt(6*d->doc->height()/3*d->doc->widthUsed() );
	d->doc->doLayout( 0,QMIN(w, mw ) );
    }
}

void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRegion& clipRegion,
			    const QColorGroup& cg, const QBrush* paper ) const
{
    if ( paper )
	d->doc->setPaper( paper );
    QColorGroup g = cg;
    if ( d->doc->paper() )
	g.setBrush( QColorGroup::Base, *d->doc->paper() );

    p->translate( x, y );
    d->doc->draw( p, clipRegion, g, paper );
    p->translate( -x, -y );
}

QString QSimpleRichText::context() const
{
    return d->doc->context();
}

QString QSimpleRichText::anchorAt( const QPoint& pos ) const
{
    QTextCursor c( d->doc );
    c.place( pos, d->doc->firstParag() );
    return c.parag()->at( c.index() )->format()->anchorHref();
}

bool QSimpleRichText::inText( const QPoint& pos ) const
{
    if ( pos.y()  > d->doc->height() )
	return FALSE;
    QTextCursor c( d->doc );
    c.place( pos, d->doc->firstParag() );
    return c.totalOffsetX() + c.parag()->at( c.index() )->x + 
	c.parag()->at( c.index() )->format()->width( c.parag()->at( c.index() )->c ) > pos.x();
}
