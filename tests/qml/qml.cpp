#include "qml.h"
#include <qapplication.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qpainter.h>

#include <qstack.h>
#include <stdio.h>



QMLStyle::QMLStyle( const QString& name )
{
    stylename = name.lower();
    init();
}

const QMLStyle& QMLStyle::nullStyle()
{
    static QMLStyle* nullstyle = 0;
    if (!nullstyle)
	nullstyle = new QMLStyle(0);
    return *nullstyle;
}


void QMLStyle::init()
{
    disp = display_inline;

    fontstyle = style_undefined;
    fontweight = weight_undefined;
    fontsize = -1;
    ncolumns = 1;

}

QString QMLStyle::name() const
{
    return stylename;
}

QMLStyle::Display QMLStyle::display() const
{
    return disp;
}

void QMLStyle::setDisplay(Display d)
{
    disp=d;
}


QMLStyle::FontStyle QMLStyle::fontStyle() const
{
    return fontstyle;
}

void QMLStyle::setFontStyle(FontStyle s)
{
    fontstyle=s;
}

int QMLStyle::fontWeight() const
{
    return fontweight;
}

void QMLStyle::setFontWeight(int w)
{
    fontweight=w;
}

int QMLStyle::fontSize() const
{
    return fontsize;
}

void QMLStyle::setFontSize(int s)
{
    fontsize=s;
}

int QMLStyle::numberOfColumns() const
{
    return ncolumns;
}

void QMLStyle::setNumberOfColumns(int ncols)
{
    if (ncols > 0)
	ncolumns = ncols;
}


//************************************************************************




QMLStyleSheet::QMLStyleSheet()
{
    init();
}

QMLStyleSheet::~QMLStyleSheet()
{
    delete defaultstyle;
}

void QMLStyleSheet::init()
{
    styles.setAutoDelete( TRUE );

    defaultstyle = new QMLStyle("");
    defaultstyle->setDisplay(QMLStyle::display_inline);
    defaultstyle->setFontStyle(QMLStyle::style_normal);
    defaultstyle->setFontWeight(QMLStyle::weight_normal);
    defaultstyle->setFontSize(12);

    QMLStyle*  style;

    style = new QMLStyle( "qml" );
    style->setDisplay(QMLStyle::display_block);
    insert(style);

    style = new QMLStyle( "em" );
    style->setFontStyle( QMLStyle::style_italic);
    insert(style);

    style = new QMLStyle( "large" );
    style->setFontSize( 24 );
    insert(style);

    style = new QMLStyle( "b" );
    style->setFontWeight( QMLStyle::weight_bold);
    insert(style);

    style = new QMLStyle( "h1" );
    style->setFontWeight( QMLStyle::weight_bold);
    style->setFontSize(24);
    style->setDisplay(QMLStyle::display_block);
    insert(style);

    style = new QMLStyle( "p" );
    style->setDisplay(QMLStyle::display_block);
    //     style->setNumberOfColumns(2);
    insert(style);
    style = new QMLStyle( "p2" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(2);
    insert(style);
    style = new QMLStyle( "p3" );
    style->setDisplay(QMLStyle::display_block);
    style->setNumberOfColumns(3);
    insert(style);
    insert(new QMLStyle("img"));
}


QMLStyle& QMLStyleSheet::defaultStyle() const
{
    return *defaultstyle;
}

QMLStyleSheet& QMLStyleSheet::defaultSheet()
{
    static QMLStyleSheet* defaultsheet = 0;
    if (!defaultsheet)
	defaultsheet = new QMLStyleSheet();
    return *defaultsheet;
}
void QMLStyleSheet::insert( QMLStyle* style)
{
    styles.insert(style->name(), style);
}

const QMLStyle& QMLStyleSheet::style(const char* name) const
{
    if (!name)
	return QMLStyle::nullStyle();
    QMLStyle* s = styles[name];
    return s?*s:QMLStyle::nullStyle();
}

QMLContainer* QMLStyleSheet::tag( const QMLStyle& style,
				  const QDict<QString> *, const QMLContext*   ) const
{
    if (style.display() == QMLStyle::display_block)
	return new QMLBox( style );

    return new QMLContainer( style );
}

// QMLNode* QMLStyleSheet::emptyTag( const QMLStyle& style, QMLContainer* parent,
// 		   const QDict<QString> *attr, const QMLContext* context ) const
// {
//     if (style.name() == "img")
// 	return new QMLImage(attr, context, parent);
//     else
// 	return 0;
// }



//************************************************************************



QMLNode::QMLNode()
{
    next = 0;
    isSimpleNode = 1;
    isLastSibling = 0;
    isContainer = 0;
    isBox = 0;
    isSelected = 0;
    isSelectionDirty = 0;
}


QMLNode::~QMLNode()
{
}



/*!
  depthFirstSearch traversal for the tag tree
 */
QMLNode* QMLNode::depthFirstSearch(QMLNode* tag, QMLContainer* &parent, bool down)
{
     if (down) {
 	if (tag->isContainer && ((QMLContainer*)tag)->child){
	    parent = (QMLContainer*)tag;
 	    return ((QMLContainer*)tag)->child;
 	}
 	return depthFirstSearch(tag, parent, FALSE);
     }
     else {
 	if (tag == this){
 	    return 0;
 	}
 	if (!tag->isLastSibling && tag->next){
	    return tag->next;
 	}
	QMLContainer* p = (QMLContainer*)tag->next;
 	if (p){
	    parent = p->parent;
 	    return depthFirstSearch(p, parent, FALSE);
 	}
     }
     return 0;
}

/*!
  extends the depthFirstSearch traversal so that only tags that include a layout are
  returned
*/

QMLNode* QMLNode::nextLayout(QMLNode* tag, QMLContainer* &parent){
    QMLNode* t;

    if (tag != this && tag->isBox)
 	t = depthFirstSearch(tag, parent, FALSE);
     else
 	t = depthFirstSearch(tag, parent);
     if (t) {
 	if (t->isContainer && !t->isBox)
 	    return nextLayout(t, parent);
     }
     return t;
}

QMLNode* QMLNode::nextLeaf(QMLNode* tag, QMLContainer* &parent){
    do {
	tag = depthFirstSearch(tag, parent);

    } while (tag && tag->isContainer);

     return tag;
}




QMLContainer* QMLNode::parent() const
{
    if (isContainer)
	return ((QMLContainer*)this)->parent;
    else {
	QMLNode* n = lastSibling();
	if (n) return (QMLContainer*)n->next;
    }
    return 0;
}

QMLBox* QMLNode::box() const
{
    QMLContainer* par = parent();
    if (!par)
	return 0;
    else
	return par->box();
}

QMLNode* QMLNode::previous() const
{
    QMLContainer* par = parent();
    QMLNode* result = par->child;
    if (result == this)
	return 0;
    while (result->next && result->next != this)
	result = result->next;
    return result;
}

QMLNode* QMLNode::lastSibling() const
{
    QMLNode* n = (QMLNode*) this;

    while (n && !n->isLastSibling)
	n = n->next;
    return n;
}

QMLNode* QMLNode::nextSibling() const
{
    if (isLastSibling)
	return 0;
    return next;
}


//************************************************************************

QMLRow::QMLRow()
{
    x = y = width = height = base = 0;
    start = end = 0;
    parent = 0;


    dirty = TRUE;
}

QMLRow::QMLRow( QMLContainer* box, QPainter* p, QMLNode* &t, QMLContainer* &par, int w)
{
    x = y = width = height = base = 0;
    start = end = 0;
    dirty = TRUE;

    width = w;

    start = t;
    parent = par;

    int tx = 0;
    int rh = 0;
    int rbase = 0;

    if (t->isBox) {
	QMLBox* b = (QMLBox*)t;
	//todo move / layout box
// 	width = b->width;
	height = b->height;
	base = height;
	end = t;
	t = box->nextLayout(t, par);
	return;
    }

    QMLNode* i = t;

    // do word wrap
    QMLContainer* lastPar = par;
    QMLNode* lastSpace = t;
    int lastHeight = rh;
    int lastBase = rbase;
    bool noSpaceFound = TRUE;


    while (i && !i->isBox) {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	if (!i->isNull())
	    tx += fm.width(i->c);
	if (tx > width)
	    break;
	rh = QMAX( rh, fm.height() );
	rbase = QMAX( rbase, fm.ascent() );
	QMLNode* cur = i;
	i = box->nextLayout(i, par);
	
// 	fprintf(stderr, "%c", cur->c.cell);
	
	// break (a) after a space, (b) before a box, (c) if we have
	// to or (d) at the end of a box.
	if (cur->isSpace() || (i&&i->isBox) || noSpaceFound || !i){
	    lastPar = par;
	    lastSpace = cur;
	    lastHeight = rh;
	    lastBase = rbase;
	    if (noSpaceFound && cur->isSpace())
		noSpaceFound = FALSE;
	    }	
    }
    end = lastSpace;
    i = box->nextLayout(lastSpace, lastPar);
    rh = lastHeight;

    par = lastPar;

    height = rh;
    base = rbase;

    t = i;
}


bool QMLRow::intersects(int xr, int yr, int wr, int hr)
{
    int mx = x;
    int my = y;

    return ( QMAX( mx, xr ) <= QMIN( mx+width, xr+wr ) &&
	     QMAX( my, yr ) <= QMIN( my+height, yr+hr ) );

}

QMLRow::~QMLRow()
{
}



void QMLRow::draw(QMLContainer* box, QPainter* p, int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, QPixmap* backgroundPixmap,  bool onlyDirty, bool onlySelection)
{

    if (!intersects(cx-obx, cy-oby, cw,ch))
  	return;


    if (start->isBox) {
	//we have to draw the box
	((QMLBox*)start)->draw(p, obx+x, oby+y, ox, oy, cx, cy, cw, ch,
			       backgroundRegion, cg, backgroundPixmap, dirty?FALSE:onlyDirty, onlySelection);
	dirty = FALSE;
	return;
    }

    QRegion r(x+obx-ox, y+oby-oy, width, height);

     backgroundRegion = backgroundRegion.subtract(r);

    if (onlyDirty) {
	if (!dirty)
	    return;
    }

    if (!onlyDirty && !onlySelection) {
	if (backgroundPixmap)
	    p->drawTiledPixmap(x+obx-ox, y+oby-oy, width, height, *backgroundPixmap, x+obx, y+oby);
	else
	    p->fillRect(x+obx-ox, y+oby-oy, width, height, cg.base());
    }

    dirty = FALSE;
    QMLNode* t = start;
    QMLContainer* par = parent;

    int tx = x;
    do {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	QString s;
	if (!t->isNull())
	    s += t->c;
	QMLNode* tmp;
	bool select = t->isSelected;
	bool selectionDirty = t->isSelectionDirty;
	t->isSelectionDirty = 0;
	while ( t != end && (tmp = t->nextSibling() ) && tmp->isSimpleNode
		&& ((bool)tmp->isSelected) == select
		&& ((bool) tmp->isSelectionDirty) == selectionDirty
		&& !t->isSpace()
		) {
	    t = tmp;
	    tmp->isSelectionDirty = 0;
	    if (!t->isNull())
		s += t->c;
	}
	int tw = fm.width( s );
	

	if (!onlySelection || selectionDirty) {
	    p->setPen( cg.text() );
	
 	    if (select) {
 		if (t==end)
 		    p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.highlight());
 		else
 		    p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.highlight());
		p->setPen( cg.highlightedText() );
 	    }
 	    else if (onlyDirty || onlySelection) {
  		if (t==end){
		    if (backgroundPixmap)
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, width-(tx-x), height, *backgroundPixmap, tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, width-(tx-x), height, cg.base());
  		}
  		else {
		    if (backgroundPixmap)
			p->drawTiledPixmap(tx+obx-ox, y+oby-oy, tw, height, *backgroundPixmap, tx+obx, y+oby);
		    else
			p->fillRect(tx+obx-ox, y+oby-oy, tw, height, cg.base());
  		}
 	    }
	
	    p->drawText(tx+obx-ox, y+oby-oy+base, s);
	}
	tx += tw;
	if (t == end)
	    break;
	t = box->nextLayout(t, par);
    } while ( t );

}

QMLNode* QMLRow::hitTest(QMLContainer* box, QPainter* p, int obx, int oby, int xarg, int yarg)
{
    if (!intersects(xarg-obx, yarg-oby, 0,0))
 	return 0;

    if (start->isBox) {
	return ((QMLBox*)start)->hitTest(p, obx+x, oby+y, xarg, yarg);
    }

    QMLNode* t = start;
    QMLContainer* par = parent;
    int tx = 0;
    QMLNode* result = t;
    do {
	p->setFont( par->font() );
	QFontMetrics fm = p->fontMetrics();
	tx += fm.width( t->c );
	result = t;
	t = box->nextLayout(t, par);
    } while (result != end && obx + x + tx <= xarg);
	
    return result;

}

bool QMLRow::locate(QMLContainer* box, QPainter* p, QMLNode* node, int &lx, int &ly, int &lh)
{
    if (start->isBox) { // a box row
	if (node == start) {
	    lx = x;
	    ly = y;
	    lh = height;
	    return TRUE;
	}
	return FALSE;
    }



    QMLNode* t = start;
    QMLContainer* par = parent;

    while (t && t != node && t != end)
	t = box->nextLayout(t, par);
    if (t != node ) {
	return FALSE; // nothing found
    }

    t = start;
    par = parent;
    lx = x;
    QFontMetrics fm = p->fontMetrics();
    while (t != node) {
	p->setFont( par->font() );
	fm = p->fontMetrics();
	lx += fm.width( t->c );
	t = box->nextLayout(t, par);
    };
    p->setFont( par->font() );
    fm = p->fontMetrics();
    ly = y + base - fm.ascent();
    lh = fm.height();

    return TRUE;
}



//************************************************************************
QMLContainer::QMLContainer( const QMLStyle &stl)
{
    isSimpleNode = 0;
    isContainer = 1;
    style = &stl;
    fnt = 0;
    parent = 0;
    child = 0;
}

QMLContainer::~QMLContainer()
{
    delete fnt;
}


QMLContainer* QMLContainer::copy()
{
    QMLContainer* result = new QMLContainer(*style);
    return result;
}

void QMLContainer::split(QMLNode* node)
{
    debug("split");
    QMLContainer* c2 = copy();

    QMLNode* prev = node->previous(); // slow!
    if (!node->isContainer) {
	QMLNode* n = new QMLNode;
	n->c = QChar::null;
	n->isLastSibling = 1;
	n->next = this;
	if (prev)
	    prev->next = n;
	else
	    child = n;
    }
    else {
	if (prev){
	    prev->isLastSibling = 1;
	    prev->next = this;
	}
	else
	    child = 0;
    }

    c2->child = node;
    c2->parent = parent;

    c2->next = next;
    next = c2;
    c2->isLastSibling = isLastSibling;
    isLastSibling = 0;

    if (!isBox)
	parent->split(c2);
    else
	c2->reparentSubtree();
}


QMLBox* QMLContainer::box() const
{
    QMLContainer* result = (QMLContainer*) this;
    while (result && !result->isBox)
	result = result->parent;
    return (QMLBox*)result;
}

QMLBox* QMLContainer::parentBox() const
{
    QMLContainer* result = (QMLContainer*) parent;
    while (result && !result->isBox)
	result = result->parent;
    return (QMLBox*)result;
}


QMLNode* QMLContainer::lastChild() const
{
    if (!child)
	return 0;
    return child->lastSibling();
}


void QMLContainer::reparentSubtree()
{
    QMLNode* n = child;
    while (n) {
	if (n->isContainer) {
	    delete  ((QMLContainer*)n)->fnt;
	    ((QMLContainer*)n)->fnt = 0;
	    ((QMLContainer*)n)->parent = this;
	     ((QMLContainer*)n)->reparentSubtree();
	}
	if (n->isLastSibling) {
	    n->next = this;
	    break;
	}
	n = n->next;
    }
}



QFont QMLContainer::font() const
{
    if (fnt)
	return *fnt;

    QMLContainer* that = (QMLContainer*) this;
    QFont* tmpfont = parent?new QFont(parent->font()) : new QFont("times");
    tmpfont->setPointSize( fontSize() );
    tmpfont->setWeight( fontWeight() );
    QMLStyle::FontStyle s = fontStyle();
    if ( s == QMLStyle::style_italic)
 	tmpfont->setItalic( TRUE );
    if ( s == QMLStyle::style_oblique)
	tmpfont->setItalic( TRUE );
    that->fnt = tmpfont;
    return *fnt;
}

int QMLContainer::fontWeight() const
{
    int w = style->fontWeight();
    if ( w == QMLStyle::weight_undefined && parent )
	w = parent->fontWeight();
    return w;
}

QMLStyle::FontStyle QMLContainer::fontStyle() const
{
    QMLStyle::FontStyle s = style->fontStyle();
    if ( s == QMLStyle::style_undefined && parent )
	s = parent->fontStyle();
    return s;
}

int QMLContainer::fontSize() const
{
    int w = style->fontSize();
    if ( w == -1 && parent )
	w = parent->fontSize();
    return w;
}


//************************************************************************

QMLBox::QMLBox( const QMLStyle &stl)
    :QMLContainer(stl)
{
    rows.setAutoDelete(true);
    isSimpleNode = 0;
    isBox = 1;
    width = height = 0;
}

QMLContainer* QMLBox::copy()
{
    QMLBox* result = new QMLBox(*style);
    return result;
}

QMLBox::~QMLBox()
{
}

// bool intersects(int x, int y, int w, int h, int x2, int y2,int w2, int h2) {

//     return QMAX( x, x2 ) <= QMIN( x+w, x2+w2 ) &&
// 	     QMAX( y, y2 ) <= QMIN( y+h, y2+h2 ) );
// }


#define IN16BIT(x) QMAX( (2<<15)-1, x)

void QMLBox::draw(QPainter *p,  int obx, int oby, int ox, int oy, int cx, int cy, int cw, int ch,
		  QRegion& backgroundRegion, const QColorGroup& cg, QPixmap* backgroundPixmap, bool onlyDirty, bool onlySelection)
{
    for (QMLRow* row = rows.first(); row; row = rows.next()) {
	row->draw(this, p, obx, oby, ox, oy, cx, cy, cw, ch, backgroundRegion, cg, backgroundPixmap, onlyDirty, onlySelection);
    }

}


void QMLBox::resize(QPainter* p, int newWidth)
{
    if (newWidth == width) // no need to resize
	return;

//     debug("box %p resize to %d", this, newWidth);

    QList<QMLRow> newRows;

    width = newWidth;
    height = 0;
    int h = 0;

    int ncols = style->numberOfColumns();
    int colwidth = newWidth / ncols;
    if (colwidth < 10)
	colwidth = 10;

    QMLContainer* par = this;
    QMLNode* n = nextLayout( this, par);
    QMLRow* row = 0;
    while (n) {
	if (n->isBox){
	    ((QMLBox*)n)->resize(p, colwidth-10); // todo this can be done in word wrap?!
	}
	row = new QMLRow(this, p, n, par, colwidth-10);
	row->x = 5;
	row->y = h;
	newRows.append(row);
	h += row->height;
    }

    // do multi columns if required. Also check with the old rows to
    // optimize the refresh
    row = newRows.first();
    QMLRow* old = rows.first();
    height = 0;
    h /= ncols;
    for (int col = 0; col < ncols; col++) {
	int colheight = 0;
	for (; row && colheight < h; row = newRows.next()) {
	    row->x = col  * colwidth + 5;
	    row->y = colheight;
	
	    if (old) {
		if (old->start == row->start && old->end == row->end
		&& old->height == row->height && old->width == old->width
		&& old->x == row->x && old->y == row->y) // TODO row operator==
		    row->dirty = old->dirty;
		old = rows.next();
	    }
	
	    colheight += row->height;
	}
	height = QMAX( height, colheight );
    }

    rows.clear();
    rows = newRows;

}


void QMLBox::update(QPainter* p, QMLRow* r)
{

    if (r) { // optimization
	QMLRow* row;
	QMLRow* prev = 0;
	
	//todo drop QList and connect the rows directly
	for ( row = rows.first(); row && row != r; row = rows.next()) {
	    prev = row;
	}
	bool fast_exit = TRUE;
	if (prev) {
	    QMLContainer* par = prev->parent;
	    QMLNode* n = prev->start;
	    QMLRow tr (this, p, n, par, prev->width);
	    fast_exit &= prev->end == tr.end;
	}
	if (fast_exit) {
	    QMLContainer* par = r->parent;
	    QMLNode* n = r->start;
	    QMLRow tr (this, p, n, par, r->width);
	    fast_exit &= r->end == tr.end && r->height == tr.height;
	}
	if (fast_exit) {
	    r->dirty = TRUE;
	    return;
	}
    }

    int oldHeight = height;
    int oldWidth = width;

    width = 0; // to force rebreak
    resize(p, oldWidth);
	
    if (height != oldHeight) { // we have to inform our parent
	QMLBox* b = parentBox();
	if (b){
	    b->update( p ); // TODO SLOW
	}
    }
}

QMLRow* QMLBox::locate(QPainter* p, QMLNode* node, int &lx, int &ly, int &lh, int&lry, int &lrh)
{
	
    QMLRow* row;
    for ( row = rows.first(); row; row = rows.next()) {
	if (row->locate(this, p, node, lx, ly, lh) ) {
	    lry = row->y;
	    lrh = row->height;
	    break;
	}
    }
    if (row) {
	QMLBox* b = parentBox();
	if (b) {
	    int mx, my, mh, mry, mrh;
	    mx = my = mh = mry = mrh = 0;
	    (void) b->locate(p, this, mx, my, mh, mry, mrh);
	    lx += mx;
	    ly += my;
	    lry += my;
	}
    }
    return row;
}

QMLNode* QMLBox::hitTest(QPainter* p, int obx, int oby, int xarg, int yarg)
{
    QMLRow* row;
    QMLNode* result = 0;
    for ( row = rows.first(); row; row = rows.next()) {
	result = row->hitTest(this, p, obx, oby, xarg, yarg);
	if (result)
	    break;
    }
    return result;
}


//************************************************************************

QMLCursor::QMLCursor(QMLDocument* doc)
{
    document = doc;
    node = doc;
    nodeParent = 0;
    hasSelection = FALSE;
    selectionDirty = FALSE;

    while (node && node->isContainer)
	node = document->depthFirstSearch( node, nodeParent);


    x = y = height = rowY = rowHeight = 0;
    row = 0;
    xline = 0;
    yline = 0;
    ylineOffsetClean = FALSE;
}

void QMLCursor::draw(QPainter* p,  int ox, int oy, int cx, int cy, int cw, int ch)
{
    if ( QMAX( x, cx ) <= QMIN( x+width(), cx+cw ) &&
	 QMAX( y, cy ) <= QMIN( y+height, cy+ch ) ) {

	p->drawLine(x-ox, y-oy, x-ox, y-oy+height-1);
	//	p->drawLine(x+1-ox, y-oy, x+1-ox, y-oy+height-1);
    }
}


void QMLCursor::clearSelection()
{
    if (!hasSelection)
	return;

    QMLNode* i = document;
    QMLContainer* ip = 0;
    while (i && i->isContainer)
	i = document->depthFirstSearch( i, ip);

    while ( i ) {
	if (i->isSelected) {
	    i->isSelected = 0;
	    i->isSelectionDirty = 1;
	}
	i = document->nextLeaf( i, ip );
    }

    selectionDirty = TRUE;
    hasSelection = FALSE;
}

void QMLCursor::goTo(QMLNode* n, QMLContainer* par, bool select)
{
    if (select){
	selectionDirty = TRUE;
	hasSelection = TRUE;

	QMLNode* other = n;
	QMLContainer* otherParent = par;

	QMLNode* i1 = node;
	QMLContainer* i1p = nodeParent;
	QMLNode* i2 = other;
	QMLContainer* i2p = otherParent;
	
	while (i1 != other && i2 != node){
	    if (i1) i1 = document->nextLeaf(i1, i1p);
	    if (i2) i2 = document->nextLeaf(i2, i2p);
	}
	QMLNode* start = 0;
	QMLContainer* startParent = 0;
	QMLNode* end = 0;
	if (i1 == other) {
	    start = node;
	    startParent = nodeParent;
	    end = other;
	}
	else {
	    start = other;
	    startParent = otherParent;
	    end = node;
	}

	while (start != end ) {
	    start->isSelected = start->isSelected?0:1;
	    start->isSelectionDirty = 1;
	    start = document->nextLeaf( start, startParent );
	}
    }

    node = n;
    nodeParent = par;
}

void QMLCursor::calculatePosition(QPainter* p)
{
    row = nodeParent->box()->locate(p, node, x, y, height, rowY, rowHeight);
    xline = x;
    yline = y;
    ylineOffsetClean = FALSE;
}

void QMLCursor::goTo(QPainter* p, int xarg, int yarg, bool select)
{
    QMLNode* n = document->hitTest(p, 0, 0, xarg, yarg);
    if (n)
	goTo(n, n->parent(), select);
    calculatePosition(p);
}


void QMLCursor::insert(QPainter* p, const QChar& c)
{
    QMLNode* n = new QMLNode;
    n->c = c;
    if (nodeParent->child == node) {
	n->next = node;
	nodeParent->child = n;
	//	row = 0;
    } else {
 	QMLNode* prev = node->previous(); // slow!
 	n->next = node;
 	prev->next = n;
    }
    QMLBox* b = node->box();
    if (b) {
	if (row && row->start == node){
	    row->start = n;
	}
	b->update(p, row);
    }
    calculatePosition(p);
}


void QMLCursor::enter(QPainter* p)
{

    nodeParent->split(node);

    QMLBox* b = nodeParent->box();
    b->update(p);
    b->next->box()->update( p );

    nodeParent = node->parent();
    calculatePosition(p);
}

void QMLCursor::del(QPainter* p)
{
    QMLNode* curNode = node;
    QMLContainer* curParent = nodeParent;
    QMLRow* curRow = row;
    
    right( p );
    if ( node == curNode )
	return;
    
    QMLBox* nodeBox = node->box();
    QMLBox* curBox = curNode->box();

    QMLNode* prev = curNode->previous();
    if (prev && prev->next == curNode) {
	prev->next = curNode->next;
	prev->isLastSibling = curNode->isLastSibling;
    }
    if (curParent->child == curNode){
	if (curNode->isLastSibling)
	    curParent->child = 0;
	else
	    curParent->child = curNode->next;
    }
    curRow->dirty = TRUE;

    if ( nodeBox != curBox ) {
	curBox->next = nodeBox->next;
	curBox->isLastSibling = nodeBox->isLastSibling;
	QMLNode* curLast = curBox->child?curBox->child->lastSibling():0;
	if (curLast) {
	    curLast->next = nodeBox->child;
	    curLast->isLastSibling = 0;
	}
	else 
	    curBox->child = nodeBox->child;
	curBox->reparentSubtree();
	if (nodeParent == nodeBox) // correct the cursor position, if necessary
	    nodeParent = curBox;
	nodeBox->child = 0;
	delete nodeBox;
    }
    
    curBox->update(p, (curRow->start!=curNode && curRow->end != curNode)?curRow:0);
    
    if (nodeBox != curBox) {
	QMLBox* b = curBox->parentBox();
	if (b){
	    b->update( p );
	}
    }

    delete curNode;
    calculatePosition(p);
}

void QMLCursor::backSpace(QPainter* p)
{
    QMLNode* curNode = node;
    left( p );
    if ( node == curNode )
	return;
    del(p);
}

void QMLCursor::right(QPainter* p, bool select)
{
    QMLContainer* np = nodeParent;
    QMLNode* n = document->nextLeaf(node, np);
    if (n)
	goTo(n, np, select);
    calculatePosition(p);
}

void QMLCursor::left(QPainter* p, bool select)
{
    QMLContainer* tmpParent = 0;

    QMLContainer* np = nodeParent;
    while (np->parent && document->nextLeaf(np, tmpParent) == node)
	np = np->parent;


    QMLNode* n = 0;
    QMLNode* tmp = np->nextLeaf(np, tmpParent);

    while (tmp != node) {
	n = tmp;
	np = tmpParent;
	tmp = document->nextLeaf(tmp, tmpParent);
    }
    if (n)
	goTo(n, np, select);
    calculatePosition(p);
}

void QMLCursor::up(QPainter* p, bool select)
{
    QMLNode* tmp = node;
    int ty = rowY - 1;
    while (ty > 0 && (!tmp || tmp == node)) {
	tmp = document->hitTest(p, 0, 0, xline, ty--);
    }
    if (tmp)
	goTo(tmp, tmp->parent(), select );
    int oldXline = xline;
    calculatePosition(p);
    xline = oldXline;
}

void QMLCursor::down(QPainter* p, bool select)
{
    QMLNode* tmp = node;
    int ty = rowY + rowHeight + 1;
    while (ty < document->height && (!tmp || tmp == node)) {
	tmp = document->hitTest(p, 0, 0, xline, ty++);
    }
    if (tmp)
	goTo(tmp, tmp->parent(), select );
    int oldXline = xline;
    calculatePosition(p);
    xline = oldXline;
}

void QMLCursor::home(QPainter* p, bool select)
{
    goTo(row->start, row->parent, select );
    calculatePosition(p);
}

void QMLCursor::end(QPainter* p, bool select)
{
    goTo(row->end, row->end->parent(), select );
    calculatePosition(p);
}


//************************************************************************


QMLContext::QMLContext()
{
    images.setAutoDelete( TRUE );
}

void QMLContext::insert(QString name, const QPixmap& pm)
{
    images.insert(name, new QPixmap(pm));
}

QPixmap* QMLContext::image(const QString &name) const
{
    return  images[name];
}


//************************************************************************


QMLDocument::QMLDocument(const QString &doc,  const QMLContext* context,
			 const QMLStyleSheet* sheet )
    : QMLBox( (sheet_ = sheet)?(sheet_->defaultStyle()):(sheet_ = &QMLStyleSheet::defaultSheet())->defaultStyle())
{
    cursor = 0;
    context_ = context;
    valid = TRUE;
    openChar = new QChar('<');
    closeChar = new QChar('>');
    slashChar = new QChar('/');
    int pos = 0;
    parse(this, 0, doc, pos);
    cursor = new QMLCursor(this);
}

QMLDocument::~QMLDocument()
{
    delete openChar;
    delete closeChar;
    delete slashChar;
}



void QMLDocument::dump()
{
}



bool QMLDocument::isValid() const
{
    return valid;
}


void QMLDocument::parse (QMLContainer* current, QMLNode* lastChild, const QString &doc, int& pos)
{
//     eatSpace(doc, pos);
    while ( valid && pos < int(doc.length() )) {
	bool sep = FALSE;
	if (hasPrefix(doc, pos, *openChar) ){
	    if (hasPrefix(doc, pos+1, *slashChar)) {
		if (current->isBox){ // todo this inserts a hitable null character
		    //		    debug("insert star");
		    QMLNode* n = new QMLNode;
		    n->c = QChar::null;
		    QMLNode* l = lastChild;
		    if (!l)
			current->child = n;
		    else {
			l->isLastSibling = 0;
			l->next = n;
		    }
		    n->next = current;
		    n->isLastSibling = 1;
		    lastChild = n;
		    l = n;
		}
		return;
	    }
	    QString tagname = parseOpenTag(doc, pos);
	    sep = eatSpace(doc, pos);
	    const QMLStyle& style = sheet_->style(tagname);
	    //hack
	    QDict<QString>* attr = 0;
	    if (tagname == "img"){
		attr = new QDict<QString>;
		attr->setAutoDelete( TRUE );
		attr->insert("source", new QString("qt.bmp"));
	    }
	    QMLNode* emptytag = 0;//sheet_->emptyTag(style, current, attr, context_);
	    if (emptytag) {
		sep |= eatSpace(doc, pos);
	    }
	    else {
		QMLContainer* tag = sheet_->tag(style);
		valid &= tag != 0;
		if (valid) {
		    QMLNode* l = lastChild;
		    if (!l){
			current->child  = tag;
			tag->isLastSibling = 1;
		    }
		    else {
			l->next = tag;
			l->isLastSibling = 0;
		    }
		
		    tag->parent = current; //TODO
		    tag ->next = current;
		    tag->isLastSibling = 1;
		    lastChild = tag;
		
		    // todo parse attributes
		    parse(tag, 0, doc, pos);
 		    sep |= eatSpace(doc, pos);
		    valid = (hasPrefix(doc, pos, *openChar)
			     && hasPrefix(doc, pos+1, *slashChar)
			     && eatCloseTag(doc, pos, tagname) );
		    if (!valid)
			return;
// 		    sep |= (eatSpace(doc, pos));
		}
	    }
// 	    if (sep) {
//    		    QMLNode* n = new QMLNode;
// 		    n->c = ' ';
// 		    QMLNode* l = lastChild;
// 		    if (!l)
// 			current->child = n;
// 		    else {
// 			l->isLastSibling = 0;
// 			l->next = n;
// 		    }
// 		    n->next = current;
// 		    n->isLastSibling = 1;
// 		    lastChild = n;
// 		    l = n;
// 	    }
	}
	else {
	    QString word = parsePlainText(doc, pos);
	    if (valid){
		QMLNode* l = lastChild;
   		for (int i = 0; i < int(word.length()); i++){
   		    QMLNode* n = new QMLNode;
		    n->c = word[i];
		    if (!l)
			current->child = n;
		    else {
			l->isLastSibling = 0;
			l->next = n;
		    }
		    n->next = current;
		    n->isLastSibling = 1;
		    lastChild = n;
		    l = n;
   		}
		sep |= eatSpace(doc, pos);
	    }
	}
    }

}

bool QMLDocument::eatSpace(const QString& doc, int& pos)
{
    int old_pos = pos;
    while (pos < int(doc.length()) && doc[pos].isSpace())
	pos++;
    return old_pos < pos;
}

bool QMLDocument::eat(const QString& doc, int& pos, const QChar& c)
{
    valid &= (bool) (doc[pos] == c);
    if (valid)
	pos++;
    return valid;
}


QString QMLDocument::parseWord(const QString& doc, int& pos)
{
    QString s;
    while( doc[pos] != *closeChar && doc[pos] != *openChar
	   && !doc[pos].isSpace() && pos < int(doc.length()) ) {
	s += doc[pos];
	pos++;
    }
    valid &= pos <= int(doc.length());
    return s;
}

QString QMLDocument::parsePlainText(const QString& doc, int& pos)
{
    QString s;
    while( doc[pos] != *closeChar && doc[pos] != *openChar
	   && pos < int(doc.length()) ) {
	if (doc[pos].isSpace()){
	    while (pos+1 < int(doc.length() ) && doc[pos+1].isSpace() ){
		pos++;
	    }
	}
	s += doc[pos];
	pos++;
    }
    valid &= pos <= int(doc.length());
    return s;
}


bool QMLDocument::hasPrefix(const QString& doc, int pos, const QChar& c)
{
    return valid && doc[pos] ==c;
}

QString QMLDocument::parseOpenTag(const QString& doc, int& pos)
{
    pos++;
    QString tag = parseWord(doc, pos).lower();
    eatSpace(doc, pos);
    eat(doc, pos, *closeChar);
    return tag;
}
bool QMLDocument::eatCloseTag(const QString& doc, int& pos, const QString& open)
{
    pos++;
    pos++;
    QString tag = parseWord(doc, pos).lower();
    eatSpace(doc, pos);
    eat(doc, pos, *closeChar);
    valid &= tag == open;
    return valid;
}



const QMLStyleSheet& QMLDocument::styleSheet() const
{
    return *sheet_;
}


//************************************************************************

QMLView::QMLView()
    : QScrollView(0,0)
{
    setVScrollBarMode( AlwaysOn );
    cursor_hidden = FALSE;
    cursorTimer = new QTimer( this );
    cursorTimer->start(200, TRUE);
    connect( cursorTimer, SIGNAL( timeout() ), this, SLOT( cursorTimerDone() ));
    backgroundPixmap = 0;
    //backgroundPixmap = new QPixmap("bg.ppm");
	
     viewport()->setBackgroundMode(NoBackground); //PaletteBase);
    //    viewport()->setBackgroundPixmap(*bg);

    QPixmap pm("qt.bmp");
    QMLContext* context = new QMLContext();
    context->insert("qt.bmp", pm);

    //    QMLStyleSheet::defaultSheet().defaultStyle().setFontSize(14);
    //    doc = new QMLDocument("Hallo<em>emph</em>Welt", context);

  // QString text = "<p>Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. </p><p>Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. Das ist dreispaltiger Text. </p>";

    QString text = "<p>Hello <EM>this is <B>bold</B> italic</EM> this is <B>bold   </B> :-) </p><H1>And this is a pretty long <EM>heading</EM> in 24 point font!</H1><p>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text.  This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p>";

		      //This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p><p2>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. <h1>This is a heading inside the p2 environment</h1>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. </p2><p3>This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text. This is another huge paragraph, it contains more or less stupid text.</p3>";

//     QString text = "Hello <EM>this is <B>bold</B> italic</EM> this is <B>bold  </B>:-) <H1> And this is a <EM>heading</EM>!</H1> And here the text continues.";

                 text += text;
                 text += text;
              text += text;
             text += text;
//              text += text;
//             text += text;
//            text += text;
//           text += text;
//           text += text;
//           text += text;

      debug("string length %d", text.length());

     doc = new QMLDocument(text, context);

}

void QMLView::keyPressEvent( QKeyEvent * e)
{

    hideCursor();
    bool select = e->state() & Qt::ShiftButton;

    if (!select) {
	doc->cursor->clearSelection();
	updateSelection();
    }

    if (e->key() == Key_Right
	|| e->key() == Key_Left
	|| e->key() == Key_Up
	|| e->key() == Key_Down
	|| e->key() == Key_Home
	|| e->key() == Key_End
	|| e->key() == Key_PageUp
	|| e->key() == Key_PageDown
	) {
	// cursor movement

	int oldCursorY = doc->cursor->y + doc->cursor->height/2;
	{
	    QPainter p( viewport() );
	    switch (e->key()) {
	    case Key_Right:
		doc->cursor->right(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Left:
		doc->cursor->left(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Up:
		doc->cursor->up(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Down:
		doc->cursor->down(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_Home:
		doc->cursor->home(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_End:
		doc->cursor->end(&p, select);
		p.end();
		ensureVisible(doc->cursor->x, doc->cursor->y);
		break;
	    case Key_PageUp:
		p.end();
		{
		    int oldContentsY = contentsY();
		    if (!doc->cursor->ylineOffsetClean)
			doc->cursor->yline-=oldContentsY;
		    scrollBy( 0, -viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = doc->cursor->xline;
		    int oldYline = doc->cursor->yline;
 		    doc->cursor->goTo( &p, oldXline, oldYline +  1 + contentsY(), select);
		    doc->cursor->xline = oldXline;
		    doc->cursor->yline = oldYline;
		    doc->cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    case Key_PageDown:
		p.end();
		{
		    int oldContentsY = contentsY();
		    if (!doc->cursor->ylineOffsetClean)
			doc->cursor->yline-=oldContentsY;
		    scrollBy( 0, viewport()->height() );
		    if (oldContentsY == contentsY() )
			break;
		    p.begin(viewport());
		    int oldXline = doc->cursor->xline;
		    int oldYline = doc->cursor->yline;
 		    doc->cursor->goTo( &p, oldXline, oldYline + 1 + contentsY(), select);
		    doc->cursor->xline = oldXline;
		    doc->cursor->yline = oldYline;
		    doc->cursor->ylineOffsetClean = TRUE;
		    p.end();
		}
		break;
	    }
	}
	updateSelection(oldCursorY, doc->cursor->y + doc->cursor->height/2);
	showCursor();
    }
    else {
	
	if (e->key() == Key_Return || e->key() == Key_Enter ) {
	    {
		QPainter p( viewport() );
		debug("enter");
		doc->cursor->enter( &p );
		QRegion r(0, 0, viewport()->width(), viewport()->height());
		doc->draw(&p, 0, 0, contentsX(), contentsY(),
			  contentsX(), contentsY(),
			  viewport()->width(), viewport()->height(),
			  r, colorGroup(), backgroundPixmap, TRUE);
		p.setClipRegion(r);
		if (backgroundPixmap)
		    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
				      *backgroundPixmap, contentsX(), contentsY());
		else
		    p.fillRect(0, 0, viewport()->width(), viewport()->height(), colorGroup().base());
	    }
	}
	else if (e->key() == Key_Delete) {
	    QPainter p( viewport() );
	    doc->cursor->del( &p );
	    QRegion r(0, 0, viewport()->width(), viewport()->height());
	    doc->draw(&p, 0, 0, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height(),
		      r, colorGroup(), backgroundPixmap, TRUE);
	    p.setClipRegion(r);
	    if (backgroundPixmap)
		p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
				  *backgroundPixmap, contentsX(), contentsY());
	    else
		p.fillRect(0, 0, viewport()->width(), viewport()->height(), colorGroup().base());
	}
	else if (e->key() == Key_Backspace) {
	    QPainter p( viewport() );
	    doc->cursor->backSpace( &p );
	    QRegion r(0, 0, viewport()->width(), viewport()->height());
	    doc->draw(&p, 0, 0, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height(),
		      r, colorGroup(), backgroundPixmap, TRUE);
	    p.setClipRegion(r);
	    if (backgroundPixmap)
		p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
				  *backgroundPixmap, contentsX(), contentsY());
	    else
		p.fillRect(0, 0, viewport()->width(), viewport()->height(), colorGroup().base());
	}
	else if (!e->text().isEmpty() ){
	    // other keys
		QPainter p( viewport() );
		for (unsigned int i = 0; i < e->text().length(); i++)
		    doc->cursor->insert( &p, e->text()[(int)i] );
		//TODO this is the wrong way. use repaint to schedule events more clever
		QRegion r(0, 0, viewport()->width(), viewport()->height());
		doc->draw(&p, 0, 0, contentsX(), contentsY(),
			  contentsX(), contentsY(),
			  viewport()->width(), viewport()->height(),
			  r, colorGroup(), backgroundPixmap, TRUE);
		p.setClipRegion(r);
		if (backgroundPixmap)
		    p.drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
				      *backgroundPixmap, contentsX(), contentsY());
		else
		    p.fillRect(0, 0, viewport()->width(), viewport()->height(), colorGroup().base());
	}
	showCursor();
	resizeContents(doc->width, doc->height);
	ensureVisible(doc->cursor->x, doc->cursor->y);
    }
}

void QMLView::updateSelection(int oldY, int newY)
{
    if (!doc->cursor || !doc->cursor->selectionDirty)
	return;

    QPainter p(viewport());
    int minY = oldY>=0?QMAX(QMIN(oldY, newY), contentsY()):contentsY();
    int maxY = newY>=0?QMIN(QMAX(oldY, newY), contentsY()+viewport()->height()):contentsY()+viewport()->height();
    QRegion r;
    doc->draw(&p, 0, 0, contentsX(), contentsY(),
	      contentsX(), minY,
	      viewport()->width(), maxY-minY,
	      r, colorGroup(), backgroundPixmap, FALSE, TRUE);
}

void QMLView::viewportMousePressEvent( QMouseEvent * e)
{
    hideCursor();
    doc->cursor->clearSelection();
    updateSelection();
    {
	QPainter p( viewport() );
	doc->cursor->goTo( &p, contentsX() + e->x(), contentsY() + e->y());
    }
    showCursor();
}

void QMLView::viewportMouseMoveEvent( QMouseEvent * e)
{
    if (e->state() & LeftButton) {
	hideCursor();
	{
	    QPainter p(viewport());
	    doc->cursor->goTo( &p, e->pos().x() + contentsX(),
			       e->pos().y() + contentsY(), TRUE);
	}
	updateSelection();
	if (doc->cursor->y + doc->cursor->height > contentsY() + viewport()->height()) {
// 	    debug("yes %d", contentsY()+viewport()->height()-doc->cursor->y-doc->cursor->height);
	    scrollBy(0, doc->cursor->y + doc->cursor->height-contentsY()-viewport()->height());
	}
	else if (doc->cursor->y < contentsY())
	    scrollBy(0, doc->cursor->y - contentsY() );
	showCursor();
    }
}

void QMLView::drawContentsOffset(QPainter*p, int ox, int oy,
				    int cx, int cy, int cw, int ch)
{

//     static int c = 0;
    //    p->drawRect(cx-ox,cy-oy,cw,ch);

    //    p->setClipRect( cx-ox, cy-oy, cw, ch );

    //    p->fillRect(cx-ox, cy-oy, cw, ch, colorGroup().base());
//     p->drawTiledPixmap(cx-ox, cy-oy, cw, ch, bg, ox, oy);


    QRegion r(cx-ox, cy-oy, cw, ch);
    doc->draw(p, 0, 0, ox, oy, cx, cy, cw, ch, r, colorGroup(), backgroundPixmap);

    p->setClipRegion(r);

    if (backgroundPixmap)
	p->drawTiledPixmap(0, 0, viewport()->width(), viewport()->height(),
			   *backgroundPixmap, ox, oy);
    else
	p->fillRect(0, 0, viewport()->width(), viewport()->height(), colorGroup().base());

    qApp->syncX();

    p->setClipping( FALSE );

    if (!cursor_hidden)
	doc->cursor->draw(p, ox, oy, cx, cy, cw, ch);
}

void QMLView::cursorTimerDone()
{
    if (cursor_hidden) {
	if (QMLView::hasFocus())
	    showCursor();
	else
	    cursorTimer->start(400, TRUE);
    }
    else {
	hideCursor();
    }
}


void QMLView::showCursor()
{
    cursor_hidden = FALSE;
    QPainter p( viewport() );
    doc->cursor->draw(&p, contentsX(), contentsY(),
		      contentsX(), contentsY(),
		      viewport()->width(), viewport()->height());
    cursorTimer->start(400, TRUE);
}

void QMLView::hideCursor()
{
    cursor_hidden = TRUE;
    repaintContents(doc->cursor->x, doc->cursor->y,
		    doc->cursor->width(), doc->cursor->height);
    cursorTimer->start(300, TRUE);
}

void QMLView::resizeEvent(QResizeEvent*e)
{
    QScrollView::resizeEvent(e);
    {
	QPainter p( this );
	doc->resize(&p, viewport()->width());
	doc->cursor->calculatePosition(&p);
    }
    resizeContents(doc->width, doc->height);
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QMLView t;
    a.setMainWidget(&t);
    t.show();

    return a.exec();
}

