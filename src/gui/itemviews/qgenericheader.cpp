#include "qgenericheader.h"
#include <qvector.h>
#include <qbitarray.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qgenericitemmodel.h>
#include <qitemdelegate.h>
#include <qevent.h>

class QGenericHeaderPrivate
{
public:
    QGenericHeaderPrivate(QGenericHeader *owner, Qt::Orientation orientation)
	: q(owner),
	  state(NoState),
	  offset(0),
	  orientation(orientation),
	  sortIndicatorOrder(Qt::Ascending),
	  sortIndicatorSection(-1),
	  movableSections(false),
	  clickableSections(false),
	  hasStretchSection(false),
 	  sectionIndicator(0) {}

    int sectionHandleAt(int pos);
    void setupSectionIndicator();
    void updateSectionIndictaor();
    QRect sectionHandleRect(int section);

    QGenericHeader *q;
    enum State { NoState, ResizeSection, MoveSection, SelectSection } state;
    int offset;
    
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;

    struct HeaderSection {
	int position;
	int section;
	uint hidden : 1;
	QGenericHeader::ResizeMode mode;
    };
    QVector<HeaderSection> sections; // section = sections.at(index)
    QVector<int> indices; // index = indices.at(section)
    
    int lastPos;
    int section; // used for resizing and moving sections
    int target;
    bool movableSections;
    bool clickableSections;
    bool hasStretchSection;
    QWidget *sectionIndicator;//, *sectionIndicator2;
};

static const int border = 4;
static const int minimum = 15;

/*!
  \class QGenericHeader qgenericheader.h

  \brief This class provides a header row or column, for itemviews.

*/

QGenericHeader::QGenericHeader(QGenericItemModel *model, Orientation o, QWidget *parent, const char *name)
    : QAbstractItemView(model, parent, name), d(new QGenericHeaderPrivate(this, o))
{
    setHScrollBarMode(AlwaysOff);
    setVScrollBarMode(AlwaysOff);
    setFrameStyle(NoFrame);
    setMargins(0, 0, 0, 0);

    viewport()->setMouseTracking(true);
    viewport()->setBackgroundRole(QPalette::Background);
    viewport()->setFocusPolicy(NoFocus);
    setFocusPolicy(NoFocus);

    // FIXME: this should also be called in setModel()
    contentsInserted(model->topLeft(root()), model->bottomRight(root()));
}

QGenericHeader::~QGenericHeader()
{
    delete d;
}

Qt::Orientation QGenericHeader::orientation() const
{
    return d->orientation;
}

int QGenericHeader::offset() const
{
    return d->offset;
}

QSize QGenericHeader::sizeHint() const
{
    if (d->sections.isEmpty())
	    return QSize();
    QModelIndex br = model()->bottomRight(root());
    QItemOptions options;
    getViewOptions(&options);
    int row = orientation() == Horizontal ? 0 : section(count() - 1);
    int col = orientation() == Horizontal ? section(count() - 1) : 0;
    QModelIndex::Type type = orientation() == Horizontal ?
			     QModelIndex::HorizontalHeader :
			     QModelIndex::VerticalHeader;
    QModelIndex item(row, col, 0, type);
    QSize hint = itemDelegate()->sizeHint(fontMetrics(), options, item);
    if (orientation() == Vertical)
	return QSize(hint.width() + border, d->sections.at(count()).position);
    return QSize(d->sections.at(count()).position, hint.height() + border);
}

int QGenericHeader::sectionSizeHint(int section, bool all) const
{
    QItemOptions options;
    getViewOptions(&options);
    QItemDelegate *delegate = itemDelegate();
    int hint = 0;
    int row = orientation() == Vertical ? section : 0;
    int col = orientation() == Vertical ? 0 : section;
    QModelIndex::Type type = orientation() == Horizontal ?
			     QModelIndex::HorizontalHeader :
			     QModelIndex::VerticalHeader;
    QModelIndex header = model()->index(row, col, 0, type);
    if (orientation() == Vertical)
	hint = delegate->sizeHint(fontMetrics(), options, header).height();
    else
	hint = delegate->sizeHint(fontMetrics(), options, header).width();

    if (!all)
	return hint + border;
    
    if (orientation() == Vertical) {
	for (int i = 0; i < model()->columnCount(root()); ++i) {
	    QModelIndex index = model()->index(section, i, root());
	    hint = qMax(delegate->sizeHint(fontMetrics(), options, index).height(), hint);
	}
    } else {
	for (int j = 0; j < model()->rowCount(root()); ++j) {
	    QModelIndex index = model()->index(j, section, root());
	    hint = qMax(delegate->sizeHint(fontMetrics(), options, index).width(), hint);
	}
    }
    return hint + border;
}

void QGenericHeader::setOffset(int o)
{
    d->offset = o;
    viewport()->update();
}

void QGenericHeader::drawContents(QPainter *painter, int cx, int cy, int cw, int ch)
{
    int offset = d->offset;
    painter->translate(orientation() == Horizontal ? -offset : 0, orientation() == Vertical ? -offset : 0);

    QItemOptions options;
    getViewOptions(&options);

    int start, end;
    if (orientation() == Horizontal) {
	start = indexAt(cx + offset);
	end = indexAt(cx + cw + offset);
    } else {
	start = indexAt(cy + offset);
	end = indexAt(cy + ch + offset);
    }
    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);
    start = start == -1 ? 0 : start;
    end = end == -1 ? count() : end;
    
    QModelIndex item;
    if (d->sections.isEmpty())
	return;
    QItemDelegate *delegate = itemDelegate();
    const QGenericHeaderPrivate::HeaderSection *sections = d->sections.constData();
    int x, y;//, f = (d->state == QGenericHeaderPrivate::MoveSection ? index(d->section) : -1);
    for (int i = start; i <= end; ++i) {
	if (sections[i].hidden)
	    continue;
	if (orientation() == Horizontal) {
	    item = model()->index(0, sections[i].section, 0, QModelIndex::HorizontalHeader);
	    options.itemRect.setRect(0, 0, sectionSize(sections[i].section), height());
	    x = sections[i].position;
	    y = 0;
	} else {
	    item = model()->index(sections[i].section, 0, 0, QModelIndex::VerticalHeader);
	    options.itemRect.setRect(0, 0, width(), sectionSize(sections[i].section));
	    x = 0;
	    y = sections[i].position;
	}
//	options.focus = (i == f);
	painter->translate(x, y);
	paintSection(painter, delegate, &options, item);
	painter->translate(-x, -y);
    }
}

void QGenericHeader::paintSection(QPainter *painter, QItemDelegate *delegate, QItemOptions *options,
				  const QModelIndex &item)
{
    static QPoint pt(0, 0);
    QStyle::SFlags flags = QStyle::Style_Off | (orientation() == Horizontal ? QStyle::Style_Horizontal : 0);
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    QStyle::SFlags arrowFlags = flags;
    if (d->clickableSections && (orientation() == Horizontal ?
	  selectionModel()->isColumnSelected(item.column(), model()->parent(item)) :
	  selectionModel()->isRowSelected(item.row(), model()->parent(item))))
	flags |= QStyle::Style_Down;
    else
	flags |= QStyle::Style_Raised;
    style().drawPrimitive(QStyle::PE_HeaderSection, painter, options->itemRect, palette(), flags);
    painter->translate(2, 0);
    delegate->paint(painter, *options, item); // draw item
    painter->translate(-2, 0);
    int section = orientation() == Horizontal ? item.column() : item.row();
    if (sortIndicatorSection() == section) {
	bool allignRight = style().styleHint(QStyle::SH_Header_ArrowAlignment, this) & AlignRight;
	// FIXME: use allignRight and RTL
	QRect arrowRect;
	int height = options->itemRect.height();
	if (orientation() == Qt::Horizontal)
	    arrowRect.setRect(sectionSizeHint(section) - border, 5, height / 2, height - border);
	else
	    arrowRect.setRect(5, sectionSizeHint(section) - border, height / 2, height - border);
	arrowFlags |= (sortIndicatorOrder() == Qt::Ascending ? QStyle::Style_Down : QStyle::Style_Up);
	style().drawPrimitive(QStyle::PE_HeaderArrow, painter, arrowRect, palette(), arrowFlags);
    }
}

int QGenericHeader::indexAt(int position) const
{
    if (count() < 1)
	return -1;
    
    int left = 0;
    int right = count() - 1;
    int idx = (right + 1) / 2;

    const QGenericHeaderPrivate::HeaderSection *sections = d->sections.constData();
    while (right - left) {
	if (sections[idx].position > position)
	    right = idx - 1;
	else
	    left = idx;
	idx = (left + right + 1) / 2;
    }
    return idx;
}

int QGenericHeader::sectionAt(int position) const
{
    int idx = indexAt(position);
    if (idx < 0)
	return -1;
    int p = d->sections.at(idx).position;
    int s = d->sections.at(idx).section;
    if (position >= p && position <= p + sectionSize(s))
 	return s;
    return -1;
}

int QGenericHeader::sectionSize(int section) const
{
    if (section < 0 || section >= d->sections.count())
 	return 0;
    int idx = index(section);
    return d->sections.at(idx + 1).position - d->sections.at(idx).position;
}

int QGenericHeader::sectionPosition(int section) const
{
    if (section < 0 || section >= d->sections.count())
	return 0;
    return d->sections.at(index(section)).position;
}

void QGenericHeader::initializeSections(int start, int end)
{
    int oldCount = count();
    end += 1; // one past the last item, so we get the end position of the last section
    d->sections.resize(end + 1);
    if (oldCount >= count()) {
	viewport()->update();
	emit sectionCountChanged(start, end);
	return;
    }

    int pos = (start <= 0 ? 0 : d->sections.at(start).position);
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data() + start;
    int s = start;
    int num = end - start + 1;
    int size = (orientation() == Horizontal ? 100 : 30); // default size

    // unroll loop - to initialize the arrays as fast as possible
    while (num >= 4) {

	sections[0].hidden = false;
	sections[0].mode = Interactive;
	sections[0].section = s++;
	sections[0].position = pos;
	pos += size;

	sections[1].hidden = false;
	sections[1].mode = Interactive;
	sections[1].section = s++;
	sections[1].position = pos;
	pos += size;

	sections[2].hidden = false;
	sections[2].mode = Interactive;
	sections[2].section = s++;
	sections[2].position = pos;
	pos += size;

	sections[3].hidden = false;
	sections[3].mode = Interactive;
	sections[3].section = s++;
	sections[3].position = pos;
	pos += size;

	sections += 4;
	num -= 4;
    }
    if (num > 0) {
	sections[0].hidden = false;
	sections[0].mode = Interactive;
	sections[0].section = s++;
	sections[0].position = pos;
	pos += size;
	if (num > 1) {
	    sections[1].hidden = false;
	    sections[1].mode = Interactive;
	    sections[1].section = s++;
	    sections[1].position = pos;
	    pos += size;
	    if (num > 2) {
		sections[2].hidden = false;
		sections[2].mode = Interactive;
		sections[2].section = s++;
		sections[2].position = pos;
		pos += size;
	    }
	}
    }
    emit sectionCountChanged(oldCount, count());
    viewport()->update();
}

void QGenericHeader::updateSection(int section)
{
    if (orientation() == Horizontal)
	updateContents(QRect(sectionPosition(section) - d->offset, 0, sectionSize(section), height()));
    else
	updateContents(QRect(0, sectionPosition(section) - d->offset, width(), sectionSize(section)));
}

void QGenericHeader::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex parent = model()->parent(topLeft);
//    Q_ASSERT(parent == model()->parent(bottomRight));
    if (parent != root())
	return;
    QModelIndex::Type type = topLeft.type();
    int from = orientation() == Vertical ? topLeft.row() : topLeft.column();
    int to =  orientation() == Vertical ? bottomRight.row() : bottomRight.column();
    for (int idx = from; idx < to; ++idx) {
	int sec = section(idx);
	QModelIndex item = (orientation() == Vertical ?
			    model()->index(sec, 0, 0, type) :
			    model()->index(0, sec, 0, type));
	if (d->sections.at(idx).mode == Content) { // resize the section to fit the new content
	    QItemOptions options;
	    getViewOptions(&options);
	    int hint = 0;
	    if (orientation() == Vertical)
		hint = itemDelegate()->sizeHint(fontMetrics(), options, item).height();
	    else
		hint = itemDelegate()->sizeHint(fontMetrics(), options, item).width();
	    int size = sectionSize(sec);
	    if (hint > size)
		resizeSection(sec, hint);
	    else // we have to check them all
		resizeSection(sec, sectionSizeHint(sec)); // sloooow
	    resizeSections();
	}
    }
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
}

void QGenericHeader::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.isValid() && bottomRight.isValid()) {
	QModelIndex parent = model()->parent(topLeft);
	if (orientation() == Horizontal)
	    initializeSections(topLeft.column(), bottomRight.column());
	else
	    initializeSections(topLeft.row(), bottomRight.row());
    }
}

void QGenericHeader::contentsRemoved(const QModelIndex &parent,
				     const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (orientation() == Horizontal)
	initializeSections(topLeft.column(), model()->columnCount(parent) - 1);
    else
	initializeSections(topLeft.row(), model()->rowCount(parent) - 1);
}

void QGenericHeader::resizeSections()
{
    ResizeMode mode;
    int secSize;
    int stretchSecs = 0;
    int stretchSize = orientation() == Horizontal ? contentsWidth() : contentsHeight();
    QVector<int> section_sizes;
    int count = qMax(d->sections.count() - 1, 0);
    QGenericHeaderPrivate::HeaderSection *secs = d->sections.data();
    for (int i = 0; i < count; ++i) {
	mode = secs[i].mode;
	if (mode == Stretch) {
	    ++stretchSecs;
	    continue;
	}
	if (mode == Interactive)
	    secSize = sectionSize(secs[i].section);
	else //if (mode == QGenericHeader::Content)
	    secSize = sectionSizeHint(secs[i].section); // warning: slow!
	section_sizes.push_back(secSize);
	stretchSize -= secSize;
    }
    int position = 0;
    int stretchSectionSize = qMax(stretchSecs > 0 ? stretchSize / stretchSecs : 0, minimum);
    for (int i; i < d->sections.count(); ++i) {
	secs[i].position = position;
	mode = secs[i].mode;
	if (mode == Stretch) {
	    position += stretchSectionSize;
	} else {
	    position += section_sizes.front();
	    section_sizes.pop_front();
	}
    }
}

void QGenericHeader::viewportMousePressEvent(QMouseEvent *e)
{
    int pos = orientation() == Horizontal ? e->x() : e->y();
    if (e->state() & ControlButton && d->movableSections) {
	d->section = d->target = sectionAt(pos + d->offset);
	if (d->section == -1) {
	    QScrollView::viewportMousePressEvent(e);
	    return;
	}
	d->state = QGenericHeaderPrivate::MoveSection;
	d->setupSectionIndicator();
	d->updateSectionIndictaor();
//	updateSection(d->section);
    } else {
	int handle = d->sectionHandleAt(pos + d->offset);
	while (handle > -1 && isSectionHidden(handle)) handle--;
	if (handle == -1) {
	    // set state to pressed
 	    int sec = sectionAt(pos + d->offset);
	    updateSection(sec);
 	    emit sectionClicked(sec, e->state());
	    return;
	} else if (resizeMode(handle) == Interactive) {
	    d->state = QGenericHeaderPrivate::ResizeSection;
	    d->lastPos = (orientation() == Horizontal ? e->x() : e->y());
	    d->section = handle;
	}
    }
}

void QGenericHeader::viewportMouseMoveEvent(QMouseEvent *e)
{
    int pos = orientation() == Horizontal ? e->x() : e->y();

    switch (d->state) {
	case QGenericHeaderPrivate::ResizeSection: {
	    int size = sectionSize(d->section) + pos - d->lastPos;
	    if (size > minimum) {
		resizeSection(d->section, size);
		d->lastPos = (orientation() == Horizontal ? e->x() : e->y());
	    }
	} break;
	case QGenericHeaderPrivate::MoveSection: {
	    int sec = sectionAt(pos + d->offset);
	    if (sec > d->section)
		sec++;
	    if (sec > -1) {
		d->target = sec;
		d->updateSectionIndictaor();
//		updateSection(sec);
	    }
	    return;
	} break;
	case QGenericHeaderPrivate::NoState: {
	    int handle = d->sectionHandleAt(pos + d->offset);
	    if (handle != -1 && resizeMode(handle) == Interactive)
		setCursor(orientation() == Horizontal ? SplitHCursor : SplitVCursor);
	    else
		setCursor(ArrowCursor);
	    QScrollView::viewportMouseMoveEvent(e);
	} break;

        case QGenericHeaderPrivate::SelectSection:
	    qDebug("SelectSection is not implemented");
	    break; // FIXME: not implemented
    }
}

void QGenericHeader::viewportMouseReleaseEvent(QMouseEvent *e)
{
    int position = orientation() == Horizontal ? e->x() : e->y();
    switch (d->state) {
    case QGenericHeaderPrivate::MoveSection:
	// keep the value within the header
	position = (position < 0 ? 0 : qMin(position, d->sections.at(count()).position));
	d->target = sectionAt(position + d->offset);
	moveSection(index(d->section), index(d->target));
	d->section = d->target = -1;
	d->updateSectionIndictaor();
	break;
    case QGenericHeaderPrivate::NoState:
    case QGenericHeaderPrivate::ResizeSection:
    case QGenericHeaderPrivate::SelectSection:
	qDebug("State was not implemented");
	break; // FIXME: not implemented
    }
    d->state = QGenericHeaderPrivate::NoState;
    QScrollView::viewportMouseReleaseEvent(e);
}

void QGenericHeader::moveSection(int from, int to)
{
    if (from == to)
	return;

    // if we haven't moved anything previously, initialize the indices array
    int count = d->sections.count();
    if (d->indices.count() != count) {
	d->indices.resize(count);
	for (int s = 0; s < count; ++s)
	    d->indices[s] = s;
    }

    // move sections and indices
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data();
    int *indices = d->indices.data();
    int sec = sections[from].section;
    int idx = from;
    if (to > from) {
	while (idx < to) {
	    sections[idx].section = sections[idx+1].section;
	    indices[sections[idx].section] = idx++;
	}
    } else {
	while (idx > to) {
	    sections[idx].section = sections[idx-1].section;
	    indices[sections[idx].section] = idx--;
	}
    }
    sections[to].section = sec;
    indices[sec] = to;

    // move positions
    if (to > from) {
	for (idx = from; idx < to; ++idx)
	    sections[idx+1].position -= sectionSize(section(idx)) - sectionSize(section(idx+1));
    } else {
	int tmp;
	int size = sectionSize(section(from));
  	for (idx = to; idx < from; ++idx) {
	    tmp = sectionSize(section(idx));
	    sections[idx+1].position = sections[idx].position + size;
	    size = tmp;
	}
    }

    viewport()->update();
    emit sectionIndexChanged(sec, from, to);
}

void QGenericHeader::resizeSection(int section, int size)
{
    int oldSize = sectionSize(section);
    if (oldSize == size)
	return;

    int diff = size - oldSize;
    int idx = index(section);
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data() + idx + 1;
    int num = d->sections.size() - idx - 1;

    while (num >= 4) {
	sections[0].position += diff;
	sections[1].position += diff;
	sections[2].position += diff;
	sections[3].position += diff;
	sections += 4;
	num -= 4;
    }
    if (num > 0) {
	sections[0].position += diff;
	if (num > 1) {
	    sections[1].position += diff;
	    if (num > 2) {
		sections[2].position += diff;
	    }
	}
    }

    int pos = d->sections.at(idx).position - d->offset;
    QRect r;
    if (orientation() == Horizontal)
	r = QRect(pos, 0, width() - pos, height());
    else
	r = QRect(0, pos, width(), height() - pos);
    viewport()->update(r);
    emit sectionSizeChanged(section, oldSize, size);
}

void QGenericHeader::hideSection(int section)
{
    resizeSection(section, 0); // FIXME: see below
    d->sections[index(section)].hidden = true;
}

void QGenericHeader::showSection(int section)
{
    d->sections[index(section)].hidden = false;
    resizeSection(section, 30); // FIXME: when you show a section, you should get the section size bach
}

bool QGenericHeader::isSectionHidden(int section) const
{
    return d->sections[index(section)].hidden;
}

void QGenericHeader::resizeEvent(QResizeEvent *e)
{
    QScrollView::resizeEvent(e);
    resizeContents(visibleWidth(), visibleHeight());
    resizeSections();
}

QModelIndex QGenericHeader::itemAt(int x, int y) const
{
    return (orientation() == Horizontal ?
	    model()->index(0, sectionAt(x), 0, QModelIndex::HorizontalHeader) :
	    model()->index(sectionAt(y), 0, 0, QModelIndex::VerticalHeader));
}

QModelIndex QGenericHeader::moveCursor(QAbstractItemView::CursorAction, ButtonState)
{
    return QModelIndex();
}

QRect QGenericHeader::itemRect(const QModelIndex &item) const
{
    if (!item.isValid() || item.type() == QModelIndex::View)
	return QRect();
    if (orientation() == Horizontal)
	return QRect(sectionPosition(item.column()), 0, sectionSize(item.column()), height());
    return QRect(0, sectionPosition(item.row()), width(), sectionSize(item.row()));
}

QModelIndex QGenericHeader::item(int section) const
{
    if (orientation() == Horizontal)
	return model()->index(0, section, 0, QModelIndex::HorizontalHeader);
    return model()->index(section, 0, 0, QModelIndex::VerticalHeader);
}

QRect QGenericHeader::selectionRect(const QItemSelection *selection) const
{
    QModelIndex bottomRight = model()->bottomRight(0);
    if (orientation() == Horizontal) {
	int left = bottomRight.row();
	int right = 0;
	int rangeLeft, rangeRight;
        int i;
	for (i = 0; i < selection->ranges.count(); ++i) {
            QItemSelectionRange r = selection->ranges.at(i);
 	    if (r.parent().isValid())
 		continue; // we only know about toplevel items
	    // FIXME an item inside the range may be the leftmost or rightmost
	    rangeLeft = index(r.left());
	    rangeRight = index(r.right());
	    if (rangeLeft < left)
		left = rangeLeft;
	    if (rangeRight > right)
		right = rangeRight;
	}
	int leftPos = sectionPosition(left);
	int rightPos = sectionPosition(right) + sectionSize(right);
	return QRect(leftPos, 0, rightPos - leftPos, height());
    }
    // orientation() == Vertical
    int top = bottomRight.column();
    int bottom = 0;
    int rangeTop, rangeBottom;
    int i;
    for (i = 0; i < selection->ranges.count(); ++i) {
        QItemSelectionRange r = selection->ranges.at(i);
 	if (r.parent().isValid())
 	    continue; // we only know about toplevel items
	// FIXME an item inside the range may be the leftmost or rightmost
	rangeTop = index(r.top());
	rangeBottom = index(r.bottom());
	if (rangeTop < top)
	    top = rangeTop;
	if (rangeBottom > bottom)
	    bottom = rangeBottom;
    }
    int topPos = sectionPosition(top);
    int bottomPos = sectionPosition(bottom) + sectionSize(bottom);
    return QRect(0, topPos, width(), bottomPos - topPos);
}

int QGenericHeader::count() const
{
    int c = d->sections.count();
    return c ? c - 1 : 0;
}

int QGenericHeader::index(int section) const
{
    if (d->indices.count() <= 0)
	return section; // nothing has been moved yet
    if (section < 0 || section >= d->indices.count())
	return 0;
    return d->indices.at(section);
}

int QGenericHeader::section(int index) const
{
    if (index < 0 || index >= d->sections.count())
	return 0;
    return d->sections.at(index).section;
}

void QGenericHeader::setMovable(bool movable)
{
    d->movableSections = movable;
}

bool QGenericHeader::isMovable() const
{
    return d->movableSections;
}

void QGenericHeader::setClickable(bool clickable)
{
    d->clickableSections = clickable;
}

bool QGenericHeader::isClickable() const
{
    return d->clickableSections;
}

void QGenericHeader::setResizeMode(ResizeMode mode)
{
    QGenericHeaderPrivate::HeaderSection *sections = d->sections.data();
    for (int i = 0; i < d->sections.count(); ++i)
	sections[i].mode = mode;
    d->hasStretchSection = (mode == Stretch);
}

void QGenericHeader::setResizeMode(ResizeMode mode, int section)
{
    if (section >= d->sections.count())
	return;
    d->sections[index(section)].mode = mode;
    d->hasStretchSection |= (mode == Stretch);
    // FIXME: handle this
}

QGenericHeader::ResizeMode QGenericHeader::resizeMode(int section) const
{
    if (section >= d->sections.count())
	return Interactive;
    return d->sections.at(index(section)).mode;
}

void QGenericHeader::setSortIndicator(int section, SortOrder order)
{
    d->sortIndicatorSection = section;
    d->sortIndicatorOrder = order;
}

int QGenericHeader::sortIndicatorSection() const
{
    return d->sortIndicatorSection;
}

Qt::SortOrder QGenericHeader::sortIndicatorOrder() const
{
    return d->sortIndicatorOrder;
}

int QGenericHeaderPrivate::sectionHandleAt(int position)
{
    int sec = q->sectionAt(position);
    int idx = q->index(sec);
    if (idx > 0 && position < q->sectionPosition(sec) + 5)
	return q->section(idx - 1);
    if (position > q->sectionPosition(sec) + q->sectionSize(sec) - 5)
	return sec;
    return -1;
}

void QGenericHeaderPrivate::setupSectionIndicator()
{
    if (!sectionIndicator) {
	sectionIndicator = new QWidget(q->viewport());
	sectionIndicator->setBackgroundRole(QPalette::Text);
    }
}

void QGenericHeaderPrivate::updateSectionIndictaor()
{
    if (section == -1 || section == target) {
	sectionIndicator->hide();
	return;
    }
    QRect geometry = sectionHandleRect(target);
    sectionIndicator->setGeometry(geometry);
    sectionIndicator->show();
}

QRect QGenericHeaderPrivate::sectionHandleRect(int section)
{
    QRect rect;
    static const int padding = 1;
    int size = 2 * padding + 1;
    int position = q->sectionPosition(section) - offset - padding - 1;
    if (q->orientation() == Qt::Horizontal) {
	rect.setRect(position, 0, size, q->height());
	if (rect.x() < 0)
	    rect.setX(0);
    } else {
	rect.setRect(0, position, q->width(), size);
	if (rect.y() < 0)
	    rect.setY(0);
    }
    return rect;
}
