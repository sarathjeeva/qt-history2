#include "qgenericlistview.h"
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <private/qobject_p.h>

template <typename T>
int bsearch(const QVector<T> &vec, const T &item, int first, int last)
{
    T val;
    int mid;
    while (true) {
	mid = first + ((last - first) >> 1);
	val = vec[mid];
	if (val == item || (last - first) < 2)
	    return mid;
	if (val > item)
	    last = mid;
	if (val < item)
	    first = mid;
    }
}

template <class T>
class BinTree
{
public:
    struct Node
    {
	enum Type { None = 0, VerticalPlane = 1, HorizontalPlane = 2, Both = 3 };
	inline Node() : pos(0), type(None) {}
	uint pos : 30;
	uint type : 2;
    };

    typedef void callback(QVector<int> &leaf, const QRect &area, uint visited, void *data);

    inline BinTree() : depth_(6), visited(0) {}

    void create(int n);
    void destroy();
    static void insert(QVector<int> &leaf, const QRect &area, uint visited, void *data);
    static void remove(QVector<int> &leaf, const QRect &area, uint visited, void *data);

    inline void climbTree(const QRect &rect, callback *function, void *data);

    inline void init(const QRect &area, typename BinTree::Node::Type type);
    inline void reserve(int size);

    inline int itemCount() const;
    inline const T &const_item(int idx) const;
    inline T &item(int idx);
    inline T *itemPtr(int idx);

    inline void setItemPosition(int x, int y, int idx);

    inline void appendItem(T &item);

    inline void insertItem(T &item, const QRect &rect, int idx);
    inline void removeItem(const QRect &rect, int idx);
    inline void moveItem(const QPoint &dest, const QRect &rect, int idx);

    inline int leafCount() const;
    inline const QVector<int> &const_leaf(int idx) const;
    inline QVector<int> &leaf(int idx);
    inline void clearLeaf(int idx);

    inline int nodeCount() const;
    inline const Node &node(int idx) const;
    inline int parentIndex(int idx) const;
    inline int firstChildIndex(int idx) const;

private:
    void climbTree(const QRect &rect, callback *function, void *data, int index);
    void init(const QRect &area, int depth, typename BinTree::Node::Type type, int index);

    uint depth_ : 8;
    mutable uint visited : 16;
    QVector<T> itemVector;
    QVector<Node> nodeVector;
    mutable QVector< QVector<int> > leafVector; // the leaves are just indices into the itemVector
};

template <class T>
void BinTree<T>::climbTree(const QRect &rect, callback *function, void *data)
{
    ++visited;
    climbTree(rect, function, data, 0);
}

template <class T>
void BinTree<T>::init(const QRect &area, typename BinTree::Node::Type type)
{
    init(area, depth_, type, 0);
}

template <class T>
void BinTree<T>::reserve(int size)
{
    itemVector.reserve(size);
}

template <class T>
int BinTree<T>::itemCount() const
{
    return itemVector.count();
}

template <class T>
const T &BinTree<T>::const_item(int idx) const
{
    return itemVector[idx];
}

template <class T>
T &BinTree<T>::item(int idx)
{
    return itemVector[idx];
}

template <class T>
T *BinTree<T>::itemPtr(int idx)
{
    return &itemVector[idx];
}

template <class T>
void BinTree<T>::setItemPosition(int x, int y, int idx)
{
    item(idx).x = x;
    item(idx).y = y;
}

template <class T>
void BinTree<T>::appendItem(T &item)
{
    itemVector.append(item);
}

template <class T>
void BinTree<T>::insertItem(T &item, const QRect &rect, int idx)
{
    itemVector.insert(idx + 1, 1, item); // insert after idx
    climbTree(rect, &insert, (void*)idx, 0);
}

template <class T>
void BinTree<T>::removeItem(const QRect &rect, int idx)
{
    climbTree(rect, &remove, (void*)idx, 0);
    itemVector.remove(idx, 1);
}

template <class T>
void BinTree<T>::moveItem(const QPoint &dest, const QRect &rect, int idx)
{
    climbTree(rect, &remove, (void*)idx, 0);
    item(idx).x = dest.x();
    item(idx).y = dest.y();
    climbTree(QRect(dest, rect.size()), &insert, (void*)idx, 0);
}

template <class T>
int BinTree<T>::leafCount() const
{
    return leafVector.count();
}

template <class T>
const QVector<int> &BinTree<T>::const_leaf(int idx) const
{
    return leafVector.at(idx);
}

template <class T>
QVector<int> &BinTree<T>::leaf(int idx)
{
    return leafVector[idx];
}

template <class T>
void BinTree<T>::clearLeaf(int idx) { leafVector[idx].clear(); }

template <class T>
int BinTree<T>::nodeCount() const
{
    return nodeVector.count();
}

template <class T>
const typename BinTree<T>::Node &BinTree<T>::node(int idx) const
{
    return nodeVector[idx];
}

template <class T>
int BinTree<T>::parentIndex(int idx) const
{
    return (idx & 1) ? ((idx - 1) / 2) : ((idx - 2) / 2);
}

template <class T>
int BinTree<T>::firstChildIndex(int idx) const
{
    return ((idx * 2) + 1);
}

template <class T>
void BinTree<T>::create(int n)
{
    // simple heuristics to find the best tree depth
    int c;
    for (c = 0; n; ++c)
	n = n / 10;
    depth_ = c << 1;
    nodeVector.resize((1 << depth_) - 1); // resize to number of nodes
    leafVector.resize(1 << depth_); // resize to number of leaves
}

template <class T>
void BinTree<T>::destroy()
{
    leafVector.clear();
    nodeVector.resize(0);
    itemVector.resize(0);
}

template <class T>
void BinTree<T>::insert(QVector<int> &leaf, const QRect &, uint, void *data)
{
    leaf.push_back((int)data);
}

template <class T>
void BinTree<T>::remove(QVector<int> &leaf, const QRect &, uint, void *data)
{
    int idx = (int)data;
    for (int i = 0; i < (int)leaf.count(); ++i) {
	if (leaf[i] == idx) {
	    for (; i < (int)leaf.count() - 1; ++i)
		leaf[i] = leaf[i + 1];
	    leaf.pop_back();
	    return;
	}
    }
}

template <class T>
void BinTree<T>::climbTree(const QRect &area, callback *function, void *data, int index)
{
    int tvs = nodeCount();
    if (index >= tvs) { // leaf
	int idx = index - tvs;
	function(leaf(idx), area, visited, data);
	return;
    }

    typename Node::Type t = (typename Node::Type) node(index).type;

    int pos = node(index).pos;
    int idx = firstChildIndex(index);
    if (t == Node::VerticalPlane) {
	if (area.left() < pos)
	    climbTree(area, function, data, idx); // back
	if (area.right() >= pos)
	    climbTree(area, function, data, idx + 1); // front
    } else {
	if (area.top() < pos)
	    climbTree(area, function, data, idx); // back
	if ( area.bottom() >= pos )
	    climbTree(area, function, data, idx + 1); // front
    }
}

template <class T>
void BinTree<T>::init(const QRect &area, int depth, typename BinTree::Node::Type type, int index)
{
    typename Node::Type t = Node::None; // t should never have this value
    if (type == Node::Both) // if both planes are specified, use 2d bsp
	t = (depth & 1) ? Node::HorizontalPlane : Node::VerticalPlane;
    else
	t = type;
    QPoint center = area.center();
    nodeVector[index].pos = (t == Node::VerticalPlane ? center.x() : center.y());
    nodeVector[index].type = t;

    QRect front = area;
    QRect back = area;

    if (t == Node::VerticalPlane) {
	front.setLeft(center.x());
	back.setRight(center.x() - 1); // front includes the center
    } else { // if ( t == Node::HorizontalPlane ) {
	front.setTop(center.y());
	back.setBottom(center.y() - 1);
    }

    int idx = firstChildIndex(index);
    if (--depth) {
	init(back, depth, type, idx);
	init(front, depth, type, idx + 1);
    }
}

class QGenericListViewItem
{
public:
    inline QGenericListViewItem()
	: x(-1), y(-1), w(-1), h(-1), indexHint(-1), visited(0xffff) {}
    inline QGenericListViewItem(const QGenericListViewItem &other)
	: x(other.x), y(other.y), w(other.w), h(other.h),
	  indexHint(other.indexHint), visited(other.visited) {}
    inline QGenericListViewItem(QRect r, int i)
	: x(r.x()), y(r.y()), w(r.width()), h(r.height()),
	  indexHint(i), visited(0xffff) {}

    inline bool operator==(const QGenericListViewItem &other) const
    {
	return (x == other.x && y == other.y && w == other.w && h == other.h &&
		indexHint == other.indexHint);
    }
    inline bool operator!=(const QGenericListViewItem &other) const { return !(*this == other); }

    inline QRect rect() { return QRect(x, y, w, h); }
    inline bool isValid() { return (x > -1) && (y > -1) && (w > -1) && (h > -1) && (indexHint > -1); }

    int x, y;
    short w, h;
    mutable int indexHint;
    uint visited : 16;
};

class QGenericListViewPrivate
{
public:
    QGenericListViewPrivate(QGenericListView *owner)
	: q(owner),
	  flow(QGenericListView::TopToBottom),
	  wrap(QGenericListView::Off),
	  movement(QGenericListView::Static),
	  spacing(5),
	  arrange(false),
	  layoutStart(0),
	  translate(0),
	  layoutWraps(0) {}
    ~QGenericListViewPrivate() {}

    void prepareItemsLayout();
    void intersectingDynamicSet(const QRect &area) const;
    void intersectingStaticSet(const QRect &area);
    void createItems(int to);
    void drawDraggedItems(QPainter *painter, const QPoint &pos) const;

    QGenericListViewItem indexToListViewItem(const QModelIndex &index) const;
    inline QModelIndex listViewItemToIndex(const QGenericListViewItem item) const
    { return q->model()->index(itemIndex(item), 0, q->root()); }
    int itemIndex(const QGenericListViewItem item) const;
    static void addLeaf(QVector<int> &leaf,
 			const QRect &area, uint visited, void *data);
    void createStaticRow(int &x, int &y, int &dy, int &wraps, int i,
			 const QRect &bounds, int spacing, int delta);
    void createStaticColumn(int &x, int &y, int &dx, int &wraps, int i,
			    const QRect &bounds, int spacing, int delta);
    void initStaticLayout(int &x, int &y, int first, const QRect &bounds);

    QGenericListView *q;
    QGenericListView::Flow flow;
    QGenericListView::Wrap wrap;
    QGenericListView::Movement movement;
    uint spacing : 16;
    uint arrange : 1;
    int layoutStart;
    int translate;
    QSize gridSize;
    QRect layoutBounds;
    QSize contentsSize; // used for static
    // used for intersecting set1
    mutable QVector<QModelIndex> intersectVector;
    // used when items are movable
    BinTree<QGenericListViewItem> tree;
    // used when items are static
    QVector<int> xposVector;
    QVector<int> yposVector;
    QVector<int> wrapVector;
    int layoutWraps;
    // used when dragging
    QVector<QModelIndex> draggedItems; // indices to the tree.itemVector
    mutable QRect draggedItemsRect;
    mutable QPoint draggedItemsPos;
    QSize itemSize; // used when all items are of the same height
};

QGenericListView::QGenericListView(QGenericItemModel *model, QWidget *parent, const char *name)
    : QAbstractItemView(model, parent, name),
      d(new QGenericListViewPrivate(this))
{
    d->prepareItemsLayout(); // initialize structures
}

QGenericListView::~QGenericListView()
{
    delete d;
}

void QGenericListView::setMovement(Movement movement)
{
    d->movement = movement;
    d->prepareItemsLayout();
    if (isVisible())
 	startItemsLayout();
}

void QGenericListView::setFlow(Flow flow)
{
    d->flow = flow;
    d->prepareItemsLayout();
    if (isVisible())
 	startItemsLayout();
}

void QGenericListView::setWrapping(Wrap wrap)
{
    d->wrap = wrap;
    if (isVisible())
 	startItemsLayout();
}

void QGenericListView::setSpacing(int space)
{
    d->spacing = space;
    if (isVisible())
 	startItemsLayout();
}

void QGenericListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode)
{
    if (d->movement == Static)
	d->intersectingStaticSet(rect);
    else
	d->intersectingDynamicSet(rect);

    if (d->intersectVector.isEmpty())
	return;
    //qHeapSort( items ); // FIXME: should be sorted by row

    QItemSelection *selection = new QItemSelection;
    QModelIndex tl;
    QModelIndex br;
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
	if (!tl.isValid() && !br.isValid()) {
	    tl = br = *it;
	} else if ((*it).row() == (tl.row() - 1)) {
	    tl = *it; // expand current range
	} else if ((*it).row() == (br.row() + 1)) {
	    br = (*it); // expand current range
	} else {
	    selection->select(tl, br, model()); // select current range
	    tl = br = *it; // start new range
	}
    }
    if (tl.isValid() && br.isValid())
 	selection->select(tl, br, model());

    selectionModel()->select(selection, mode, selectionBehavior());
}

void QGenericListView::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // FIXME: do something here
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
}

void QGenericListView::contentsInserted(const QModelIndex &topLeft, const QModelIndex &)
{
    QModelIndex parent = model()->parent(topLeft);
    if (parent != root())
	return;

    if (isVisible())
	startItemsLayout();

    bool needMore = false;
    if ((d->flow == TopToBottom && d->wrap == Off) || (d->flow == LeftToRight && d->wrap == On))
        needMore = viewport()->height() >= contentsHeight();
    else
        needMore = viewport()->width() >= contentsWidth();
    if (needMore)
        QApplication::postEvent(model(), new QMetaCallEvent(QEvent::InvokeSlot,
					 model()->metaObject()->indexOfSlot("fetchMore()"), this));
}

void QGenericListView::contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    contentsInserted(topLeft, bottomRight); // does the same thing
}

void QGenericListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (!model()->canDecode(e)) {
	e->ignore();
	return;
    }

    QPoint pos = e->pos();
    d->draggedItemsPos = pos;
    updateContents(d->draggedItemsRect);

    QModelIndex item = itemAt(pos.x(), pos.y());
    if (item.isValid()) {
	if (model()->isDropEnabled(item))
	    e->accept();
	else
	    e->ignore();
    } else {
	e->accept();
    }

    qApp->processEvents(); // make sure we can draw items
}

void QGenericListView::contentsDropEvent(QDropEvent *e)
{
    if (e->source() == this && d->movement == Free
	 /*&& e->action() == QDropEvent::Move*/) {
	QPoint delta = e->pos() - dragRect().topLeft();
	QList<QModelIndex> items = selectionModel()->selectedItems();
        int i;
	for (i = 0; i < items.count(); ++i) {
            QModelIndex pos = items.at(i);
	    QRect rect = itemRect(pos);
	    moveItem(pos.row(), QPoint(rect.x() + delta.x(), rect.y() + delta.y()));
	    updateContents(rect);
	    updateItem(pos);
	}
    } else {
	QAbstractItemView::contentsDropEvent(e);
    }
}

QDragObject *QGenericListView::dragObject()
{
    // This function does the same thing as in QAbstractItemView,
    //  plus adding viewitems to the draggedItems list. We need these items to draw the drag items
    QItemViewDragObject *dragObject = new QItemViewDragObject(this, "DragObject");
    QModelIndexList items = selectionModel()->selectedItems();
    dragObject->set(items);
    QModelIndexList::ConstIterator it = items.begin();
    for (; it != items.end(); ++it)
	if (model()->isDragEnabled(*it))
	    d->draggedItems.push_back(*it);
    return dragObject;
}

void QGenericListView::startDrag()
{
    QAbstractItemView::startDrag();
    // clear dragged items
    d->draggedItems.clear();
    updateContents(d->draggedItemsRect);
}

void QGenericListView::getViewOptions(QItemOptions *options) const
{
    QAbstractItemView::getViewOptions(options);
    options->smallItem = !d->wrap;
    options->iconAlignment = (d->wrap ? Qt::AlignTop : Qt::AlignLeft | Qt::AlignVCenter);
    options->textAlignment = (d->wrap ? Qt::AlignCenter : Qt::AlignLeft | Qt::AlignVCenter);
}

void QGenericListView::drawContents(QPainter *painter, int cx, int cy, int cw, int ch)
{
    QRect area(cx, cy, cw, ch);
    clearArea(painter, area);

    // fill the intersectVector
    if (d->movement == Static)
	d->intersectingStaticSet(area);
    else
	d->intersectingDynamicSet(area);

    QItemOptions options;
    getViewOptions(&options);

    int x, y;
    QModelIndex current = currentItem();
    QItemDelegate *delegate = itemDelegate();
    QItemSelectionModel *selections = selectionModel();
    bool focus = viewport()->hasFocus() && current.isValid();
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
 	options.itemRect = itemRect(*it);
 	options.selected = selections ? selections->isSelected(*it) : false;
 	options.focus = (focus && current == *it);
	x = options.itemRect.x();
	y = options.itemRect.y();
 	painter->translate(x, y);
 	delegate->paint(painter, options, *it);
 	painter->translate(-x, -y);
    }

    if (!d->draggedItems.isEmpty())
   	d->drawDraggedItems(painter, d->draggedItemsPos);
    if (state() == QAbstractItemView::Selecting)
 	drawSelectionRect(painter, dragRect().normalize());
}

QModelIndex QGenericListView::itemAt(int x, int y) const
{
    if (d->movement == Static)
	d->intersectingStaticSet(QRect(x, y, 1, 1));
    else
	d->intersectingDynamicSet(QRect(x, y, 1, 1));
    return (d->intersectVector.empty() ? QModelIndex() : d->intersectVector.first());
}

QModelIndex QGenericListView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState)
{
    QRect area = itemRect(currentItem());
    int iw = area.width();
    int ih = area.height();
    int cw = contentsWidth();
    int ch = contentsHeight();
    QPoint pos = area.topLeft();
    d->intersectVector.clear();

    switch (cursorAction) {
    case MoveLeft:
	area.moveRight(area.left() - 1);
	while (d->intersectVector.count() == 0) {
	    if (d->movement == Static)
		d->intersectingStaticSet(area);
	    else
		d->intersectingDynamicSet(area);
	    if ( area.left() <= 0 )
		area.setRect(cw - iw, area.top() - ih - 1, iw, ih);
	    else
		area.moveRight(area.left() - 1);
	    if (area.top() > ch || area.bottom() < 0)
		break;
	}
	break;
    case MoveRight:
	area.moveLeft(area.right() + 1);
	while (d->intersectVector.count() == 0) {
	    if (d->movement == Static)
		d->intersectingStaticSet(area);
	    else
		d->intersectingDynamicSet(area);
	    if (area.right() >= cw)
		area.setRect(0, area.bottom() + 1, iw, ih);
	    else
		area.moveLeft(area.right() + 1);
	    if (area.top() > ch || area.bottom() < 0)
		break;
	}
	break;
    case MovePageUp:
	// FIXME: take layout into account
	area.moveTop(area.top() - viewport()->height() + (ih << 1));
	if (area.top() < 0)
	    area.moveTop(ch + (ih << 1));
	// FIXME: if the last line is not filled, it will wrap to the bottom
    case MoveUp:
	area.moveBottom(area.top() - 1);
	while (d->intersectVector.count() == 0) {
	    if (d->movement == Static)
		d->intersectingStaticSet(area);
	    else
		d->intersectingDynamicSet(area);
	    if (area.top() <= 0)
		area.setRect(area.left() - iw - 1, ch - ih, iw, ih);
	    else
		area.moveBottom(area.top() - 1);
	    if (area.left() > cw || area.right() < 0)
		break;
	}
	break;

    case MovePageDown:
	// FIXME: take layout into account
 	area.moveTop(area.top() + viewport()->height() - (ih << 1));
	if (area.top() > ch)
	    area.moveTop(ch - (ih << 1));
	// FIXME: if the last line is not filled, it will wrap to the top
    case MoveDown:
	area.moveTop(area.bottom() + 1);
	while (d->intersectVector.count() == 0) {
	    if (d->movement == Static)
		d->intersectingStaticSet(area);
	    else
		d->intersectingDynamicSet(area);
	    if (area.bottom() >= ch)
		area.setRect(area.right() + 1, 0, iw, ih);
	    else
		area.moveTop(area.bottom() + 1);
	    if (area.left() > cw || area.right() < 0)
		break;
	}
	break;

    case MoveHome:
	return model()->index(0, 0, root());

    case MoveEnd:
	return model()->index(d->layoutStart - 1, 0, root());
    }

    int dist = 0;
    int minDist = 0;
    QModelIndex closest;
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
	dist = (d->indexToListViewItem(*it).rect().topLeft() - pos).manhattanLength();
	if (dist < minDist || minDist == 0) {
	    minDist = dist;
	    closest = *it;
	}
    }

    return closest;
}

QRect QGenericListView::itemRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
	return QRect();
    QRect rect = d->indexToListViewItem(item).rect();
    if (d->wrap == QGenericListView::Off && d->movement == QGenericListView::Static)
	if (d->flow == QGenericListView::TopToBottom)
 	    rect.setWidth(contentsWidth());
	else
	    rect.setHeight(contentsHeight());
    return rect;
}

QRect QGenericListView::selectionRect(const QItemSelection *selection) const
{
    // FIXME: slow temporary fix
    QList<QModelIndex> items = selection->items(model());
    QList<QModelIndex>::iterator it = items.begin();
    QRect rect;
    for (; it != items.end(); ++it)
	rect |= itemRect(*it);
    items.clear();
//     if ( d->drawSelectionRect )
// 	return rect | d->selectionRect | d->oldSelectionRect;
    return rect;
}

void QGenericListView::startItemsLayout()
{
    // we should keep the original layout rect if and only start layout once
//    if ( /*d->movement == Free && arrangeItemsDone()*/ d->layoutBounds.isValid() )
//	return;

    d->layoutStart = 0;
    d->layoutWraps = 0;
    d->translate = 0;

    d->layoutBounds = viewport()->rect();
    int sbx = style().pixelMetric(QStyle::PM_ScrollBarExtent);
    d->layoutBounds.setWidth(d->layoutBounds.width() - sbx);
    d->layoutBounds.setHeight(d->layoutBounds.height() - sbx);

//     if (itemDelegate()->sameHeight() && itemDelegate()->sameWidth()) {
// 	d->itemSize = itemRect(model()->index(0, 0, root())).size();  // FIXME: atually use this
// 	doItemsLayout(model()->rowCount(root()));
//     } else {
	QAbstractItemView::startItemsLayout(); // do delayed layout
//    }
}

void QGenericListView::stopItemsLayout()
{
    if (d->movement == Static) {
	d->wrapVector.resize(d->wrapVector.count());
	if (d->flow == LeftToRight)
	    d->yposVector.resize(d->yposVector.count());
	else // TopToBottom
	    d->xposVector.resize(d->xposVector.count());
    }
}

bool QGenericListView::doItemsLayout(int delta)
{
    int max = model()->rowCount(root()) - 1;
    int first = d->layoutStart;
    int last = first + qMin(delta - 1, max);// - 1;

    if (max <= 0)
 	return true; // nothing to do

    if (d->movement == Static) {
	doStaticLayout(d->layoutBounds, first, last);
    } else {
	if (last >= d->tree.itemCount())
	    d->createItems(last + 1);
	doDynamicLayout(d->layoutBounds, first, last);
    }

    d->layoutStart = last + 1;

    if (d->layoutStart >= max) {
	stopItemsLayout();
	return true; // done
    }

    return false; // not done
}

void QGenericListView::doItemsLayout(const QRect &bounds,
				     const QModelIndex &first,
				     const QModelIndex &last)
{
    if (first.row() >= last.row() || !first.isValid() || !last.isValid())
	return;
    if (d->movement == Static)
	doStaticLayout(bounds, first.row(), last.row());
    else
	doDynamicLayout(bounds, first.row(), last.row());
}

void QGenericListView::doStaticLayout(const QRect &bounds, int first, int last)
{
    int x = 0;
    int y = 0;
    d->initStaticLayout(x, y, first, bounds);

    int delta = last - first + 1;
    int spacing = d->spacing;
    int layoutWraps = d->layoutWraps;
    bool wrap = d->wrap;
    QModelIndex item;
    QFontMetrics fontMetrics(font());
    QItemOptions options;
    getViewOptions(&options);
    QItemDelegate *delegate = itemDelegate();
    QSize hint;
    QRect rect = bounds;
/*
    if (d->movement == QGenericListView::Static && d->wrap == QGenericListView::Off)
	if (d->flow == QGenericListView::TopToBottom)
	    rect.setWidth(qMax(contentsWidth(), rect.width()));
	else
	    rect.setHeight(qMax(contentsHeight(), rect.height()));
*/
    if (d->flow == LeftToRight) {
	int w = bounds.width();
	int dy = (d->gridSize.height() > 0 ? d->gridSize.height() : d->translate);
	int dx;
	for (int i = first; i <= last ; ++i) {

	    item = model()->index(i, 0, root());
	    hint = delegate->sizeHint(fontMetrics, options, item);
	    dx = (d->gridSize.width() > 0 ? d->gridSize.width() : hint.width());

	    if (wrap && (x + spacing + dx >= w))
		d->createStaticRow(x, y, dy, layoutWraps, i, bounds, spacing, delta);

	    d->xposVector.push_back(x);
	    dy = (hint.height() > dy ? hint.height() : dy);
	    x += spacing + dx;
	}
	// used when laying out next batch
	d->xposVector.push_back(x);
	d->translate = dy;
	rect.setRight(x);
    } else { // d->flow == TopToBottom
	int h = bounds.height();
	int dx = (d->gridSize.width() > 0 ? d->gridSize.width() : d->translate);
	int dy;
	for (int i = first; i <= last ; ++i) {

	    item = model()->index(i, 0, root());
	    hint = delegate->sizeHint(fontMetrics, options, item);
	    dy = (d->gridSize.height() > 0 ? d->gridSize.height() : hint.height());

	    if (wrap && (y + spacing + dy >= h))
		d->createStaticColumn(x, y, dx, layoutWraps, i, bounds, spacing, delta);

	    d->yposVector.push_back(y);
	    dx = (hint.width() > dx ? hint.width() : dx);
	    y += spacing + dy;
	}
	// used when laying out next batch
	d->yposVector.push_back(y);
	d->translate = dx;
	rect.setBottom(y);
    }

    if (d->layoutWraps < layoutWraps) {
	d->layoutWraps = layoutWraps;
	d->wrapVector.push_back(last + 1);
    }

    d->contentsSize = rect.size();
    resizeContents(rect.width(), rect.height());
    QRect visibleRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    if (visibleRect.intersects(rect))
  	updateContents(visibleRect);
}

void QGenericListView::doDynamicLayout(const QRect &bounds, int first, int last)
{
    int spacing = d->spacing;
    int x, y;

    if (first == 0) {
	x = bounds.x() + spacing;
	y = bounds.y() + spacing;
	d->tree.reserve(model()->rowCount(root()));
    } else {
	int p = first - 1;
	const QGenericListViewItem item = d->tree.item(p);
	x = item.x;
	y = item.y;
	if (d->flow == LeftToRight)
	    x += (d->gridSize.width() > 0 ? d->gridSize.width() : item.w + spacing);
	else
	    y += (d->gridSize.height() > 0 ? d->gridSize.height() : item.h + spacing);
    }

    bool wrap = d->wrap;
    QRect rect;
    QModelIndex bottomRight = model()->bottomRight(root());
    QGenericListViewItem *item = 0;

    if (d->flow == LeftToRight) {
	int w = bounds.width();
	int dy = (d->gridSize.height() > 0 ? d->gridSize.height() : d->translate);
	for (int i = first; i <= last; ++i) {
	    item = d->tree.itemPtr(i);
	    int dx = (d->gridSize.width() > 0 ? d->gridSize.width() : item->w);
	    // create new layout row
	    if (wrap && (x + spacing + dx >= w)) {
		x = bounds.x() + spacing;
		y += spacing + dy;
	    }
	    item->x = x;
	    item->y = y;
	    x += spacing + dx;
	    dy = (item->h > dy ? item->h : dy);
	    rect |= item->rect();
	}
	d->translate = dy;
    } else { // TopToBottom
	int h = bounds.height();
	int dx = (d->gridSize.width() > 0 ? d->gridSize.width() : d->translate);
	for (int i = first; i <= last; ++i) {
	    item = d->tree.itemPtr(i);
	    int dy = (d->gridSize.height() > 0 ? d->gridSize.height() : item->h);
	    if (wrap && (y + spacing + dy >= h)) {
		y = bounds.y() + spacing;
		x += spacing + dx;
	    }
	    item->x = x;
	    item->y = y;
	    y += spacing + dy;
	    dx = (item->w > dx ? item->w : dx);
	    rect |= item->rect();
	}
	d->translate = dx;
    }

    int insertFrom = first;
    resizeContents(rect.right(), rect.bottom());

    if (first == 0 || last >= bottomRight.row()) { // resize tree

	// remove all items from the tree
	int leafCount = d->tree.leafCount();
	for (int i = 0; i < leafCount; ++i)
	    d->tree.clearLeaf(i); // FIXME: use fill( 0 )
	insertFrom = 0;

	int h = contentsHeight();
	int w = contentsWidth();

	// initialize tree
	// we have to get the bounding rect of the items before we can initialize the tree
	BinTree<QGenericListViewItem>::Node::Type type =
	    BinTree<QGenericListViewItem>::Node::Both; // use 2D bsp by default
 	if (h / w >= 3)    // simple heuristics to get better bsp
 	    type = BinTree<QGenericListViewItem>::Node::HorizontalPlane;
 	else if (w / h >= 3)
 	    type = BinTree<QGenericListViewItem>::Node::VerticalPlane;
	d->tree.init(QRect(0, 0, w, h), type); // build tree for bounding rect (not contents rect)
    }

    // insert items in tree
    for (int i = insertFrom; i <= last; i++)
	d->tree.climbTree(d->tree.item(i).rect(), &BinTree<QGenericListViewItem>::insert, (void *)i);

    QRect visibleRect(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    if (visibleRect.intersects(rect))
	updateContents(visibleRect);
}

bool QGenericListView::supportsDragAndDrop() const
{
    return true;
}

void QGenericListView::insertItem(int index, QGenericListViewItem &item)
{
    d->tree.insertItem(item, item.rect(), index);
}

void QGenericListView::removeItem(int index)
{
    d->tree.removeItem(d->tree.item(index).rect(), index);
}

void QGenericListView::moveItem(int index, const QPoint &dest)
{
    // does not impact on the bintree itself or the contents rect
    QGenericListViewItem *item = d->tree.itemPtr(index);
    QRect rect = item->rect();
    d->tree.moveItem(dest, rect, index);

    // resize the contents area
    rect = item->rect();
    int w = item->x + rect.width();
    int h = item->y + rect.height();
    w = w > contentsWidth() ? w : contentsWidth();
    h = h > contentsHeight() ? h : contentsHeight();
    resizeContents(w, h);
}

void QGenericListView::updateGeometries()
{
#if 1
    if (d->movement == QGenericListView::Static && d->wrap == QGenericListView::Off)
	if (d->flow == QGenericListView::TopToBottom)
	    resizeContents(qMax(visibleWidth(), d->contentsSize.width()), contentsHeight());
	else
	    resizeContents(contentsWidth(), qMax(visibleHeight(), d->contentsSize.height()));
//    qDebug("contentsSize %d", d->contentsSize.width());
#endif
}

void QGenericListViewPrivate::prepareItemsLayout()
{
    // initailization of data structs
    int rowCount = q->model()->rowCount(q->root());
    if (movement == QGenericListView::Static) {
	tree.destroy();
	if (flow == QGenericListView::LeftToRight) {
	    xposVector.resize(qMax(rowCount/* - 1*/, 0));
	    yposVector.clear();
	} else { // TopToBottom
	    yposVector.resize(qMax(rowCount/* - 1*/, 0));
	    xposVector.clear();
	}
	wrapVector.clear();
    } else {
	wrapVector.clear();
	xposVector.clear();
	yposVector.clear();
	tree.create(rowCount);
    }
}

void QGenericListViewPrivate::intersectingStaticSet(const QRect &area)
{
    intersectVector.clear();
    QGenericItemModel *model = q->model();

    QModelIndex root = q->root();
    int first, last, count, i, j;
    bool wraps = wrapVector.count() > 1;
    if (flow == QGenericListView::LeftToRight) {
	if (yposVector.count() == 0)
 	    return;
	j = bsearch<int>(yposVector, area.top(), 0, layoutWraps + 1); // index to the first xpos
	for (; j < (layoutWraps + 1) && yposVector.at(j) < area.bottom(); ++j) {
	    first = wrapVector.at(j);
	    count = (wraps ? wrapVector.at(j + 1) - wrapVector.at(j) : layoutStart);
	    last = first + count;
	    i = bsearch<int>(xposVector, area.left(), first, first + count);
	    for (; i < last && xposVector.at(i) < area.right(); ++i)
		intersectVector.push_back(model->index(i, 0, root));
	}
    } else { // flow == TopToBottom
 	if (xposVector.count() == 0)
 	    return;
	j = bsearch<int>(xposVector, area.left(), 0, layoutWraps + 1); // index to the first xpos
	for (; j < (layoutWraps + 1) && xposVector.at(j) < area.right(); ++j) {
	    first = wrapVector.at(j);
	    count = (wraps ? wrapVector.at(j + 1) - wrapVector.at(j) : layoutStart);
	    last = first + count;
	    i = bsearch<int>(yposVector, area.top(), first, first + count);
	    for (; i < last && yposVector.at(i) < area.bottom(); ++i)
		intersectVector.push_back(model->index(i, 0, root));
	}
    }
}

void QGenericListViewPrivate::intersectingDynamicSet(const QRect &area) const
{
    intersectVector.clear();
//    intersectVector.reserve(500); // FIXME
    QGenericListViewPrivate *that = (QGenericListViewPrivate*)this; // FIXME
    that->tree.climbTree(area, &QGenericListViewPrivate::addLeaf, (void *)this);
}

void QGenericListViewPrivate::createItems(int to)
{
    int count = tree.itemCount();
    QSize size;
    QItemOptions options;
    q->getViewOptions(&options);
    QFontMetrics fontMetrics(q->font());
    QGenericItemModel *model = q->model();
    QItemDelegate *delegate = q->itemDelegate();
    QModelIndex root = q->root();
    for (int i = count; i < to; ++i) {
	size = delegate->sizeHint(fontMetrics, options, model->index(i, 0, root)); // FIXME: optimized size
	QGenericListViewItem item(QRect(0, 0, size.width(), size.height()), i); // default pos
	tree.appendItem(item);
    }
}

void QGenericListViewPrivate::drawDraggedItems(QPainter *painter, const QPoint &pos) const
{
    // FIXME: should we move this to QAbstractItemView?
    // draw the drag items
    int x = 0, y = 0;
    QItemOptions options;
    q->getViewOptions(&options);
    QItemDelegate *delegate = q->itemDelegate();
    QPoint delta = pos - q->dragRect().topLeft();
    QVector<QModelIndex>::const_iterator it = draggedItems.begin();
    QGenericListViewItem item = indexToListViewItem(*it);
    draggedItemsRect.setRect(item.x + delta.x(), item.y + delta.y(), item.w, item.h);
    for (; it != draggedItems.end(); ++it) {
	item = indexToListViewItem(*it);
 	x = item.x + delta.x();
 	y = item.y + delta.y();
	options.itemRect.setRect(x, y, item.w, item.h);
	painter->translate(x, y);
 	delegate->paint(painter, options, *it);
 	painter->translate(-x, -y);
	draggedItemsRect |= options.itemRect;
    }
}

QGenericListViewItem QGenericListViewPrivate::indexToListViewItem(const QModelIndex &item) const
{
    if (!item.isValid())
	return QGenericListViewItem();

    if (movement == QGenericListView::Free)
	return tree.const_item(item.row());

    // movement == Static
    QItemOptions options;
    q->getViewOptions(&options);
    QSize hint = q->itemDelegate()->sizeHint(q->fontMetrics(), options, item);
    int i = bsearch<int>(wrapVector, item.row(), 0, layoutWraps + 1);
    QPoint pos;
    if (flow == QGenericListView::LeftToRight) {
	pos.setX(xposVector.at(item.row()));
	pos.setY(yposVector.at(i));
    } else { // TopToBottom
	pos.setY(yposVector.at(item.row()));
	pos.setX(xposVector.at(i));
    }
    return QGenericListViewItem(QRect(pos, hint), item.row());
}

int QGenericListViewPrivate::itemIndex(const QGenericListViewItem item) const
{
    int i = item.indexHint;
    if (movement == QGenericListView::Static || i >= tree.itemCount() || tree.const_item(i) == item)
	return i;

    int j = i;
    int c = tree.itemCount();
    bool a = true;
    bool b = true;

    while (a || b) {
	if (a) {
	    if (tree.const_item(i) == item) {
		tree.const_item(i).indexHint = i;
		return i;
	    }
	    a = ++i < c;
	}
	if (b) {
	    if (tree.const_item(j) == item) {
		tree.const_item(j).indexHint = j;
		return j;
	    }
	    b = --j > -1;
	}
    }
    return -1;
}

void QGenericListViewPrivate::addLeaf(QVector<int> &leaf, const QRect &area,
				      uint visited, void *data)
{
    QGenericListViewItem *vi;
    QGenericListViewPrivate *_this = (QGenericListViewPrivate *)data; // FIXME: cast away const
    for (int i = 0; i < (int)leaf.count(); ++i) {
	int idx = leaf.at(i);
	if (idx < 0)
	    continue;
	vi = _this->tree.itemPtr(idx);
	if (vi->rect().intersects(area) && vi->visited != visited) {
	    _this->intersectVector.push_back(_this->listViewItemToIndex(*vi));
	    vi->visited = visited;
	}
    }
}

void QGenericListViewPrivate::createStaticRow(int &x, int &y, int &dy, int &wraps, int i,
					      const QRect &bounds, int spacing, int delta)
{
    x = bounds.left() + spacing;
    y += spacing + dy;
    ++wraps;
    if ((int)yposVector.size() < (wraps + 2)) {
	int s = yposVector.size() + delta;
	yposVector.resize(s); // FIXME: reserve
	wrapVector.resize(s);
    }
    yposVector[wraps] = y;
    wrapVector[wraps] = i;
    dy = 0;
}

void QGenericListViewPrivate::createStaticColumn(int &x, int &y, int &dx, int &wraps, int i,
						 const QRect &bounds, int spacing, int delta)
{
    y = bounds.top() + spacing;
    x += spacing + dx;
    ++wraps;
    if ((int)xposVector.size() < (wraps + 2)) {
	int s = xposVector.size() + delta;
	xposVector.resize(s); // FIXME: reserve
	wrapVector.resize(s);
    }
    xposVector[wraps] = x;
    wrapVector[wraps] = i;
    dx = 0;
}

void QGenericListViewPrivate::initStaticLayout(int &x, int &y, int first, const QRect &bounds)
{
    if (first == 0) {
	x = bounds.left() + spacing;
	xposVector.clear();
	if (flow == QGenericListView::TopToBottom)
	    xposVector.push_back(x);
	y = bounds.top() + spacing;
	yposVector.clear();
	if (flow == QGenericListView::LeftToRight)
	    yposVector.push_back(y);
	layoutWraps = 0;
	wrapVector.clear();
	wrapVector.push_back(0);
    } else if (wrap) {
	if (flow == QGenericListView::LeftToRight) {
	    x = xposVector.back();
	    xposVector.pop_back();
	} else {
	    x = xposVector.at(layoutWraps);
	}
	if (flow == QGenericListView::TopToBottom) {
	    y = yposVector.back();
	    yposVector.pop_back();
	} else {
	    y = yposVector.at(layoutWraps);
	}
    } else {
	if (flow == QGenericListView::LeftToRight) {
	    x = xposVector.back();
	    xposVector.pop_back();
	} else {
	    x = bounds.left() + spacing;
	}
	if (flow == QGenericListView::TopToBottom) {
	    y = yposVector.back();
	    yposVector.pop_back();
	} else {
	    y = bounds.top() + spacing;
	}
    }
}
