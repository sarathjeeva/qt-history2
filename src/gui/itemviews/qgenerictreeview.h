#ifndef QGENERICTREEVIEW_H
#define QGENERICTREEVIEW_H

#ifndef QT_H
#include <qabstractitemview.h>
#endif

class QGenericTreeViewPrivate;
class QGenericHeader;

class Q_GUI_EXPORT QGenericTreeView : public QAbstractItemView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGenericTreeView);

public:
    QGenericTreeView(QGenericItemModel *model, QWidget *parent = 0, const char *name = 0);
    ~QGenericTreeView();

    QGenericHeader *header() const;
    void setHeader(QGenericHeader *header);

    int indentation() const;
    void setIndentation(int i);

    bool isColumnHidden(int column) const;

public slots:
    void hideColumn(int column);

protected slots:
    virtual void columnWidthChanged(int col, int oldSize, int newSize);
    virtual void columnCountChanged(int oldCount, int newCount);
    virtual void contentsChanged();

protected:
    void contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void contentsRemoved(const QModelIndex &parent, const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void startItemsLayout();
    bool doItemsLayout(int num);
    
    void drawContents(QPainter *painter, int cx, int cy, int cw, int ch);    
    virtual void drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const;
    virtual void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;

    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState state);
    QModelIndex itemAt(int x, int y) const;

    QRect itemRect(const QModelIndex &item) const;

    QItemSelectionModel::SelectionBehavior selectionBehavior() const;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionUpdateMode mode);
    QRect selectionRect(const QItemSelection *selection) const;

//    void resizeEvent(QResizeEvent *e);
    void contentsMousePressEvent(QMouseEvent *e);
    void updateGeometries();
};

#endif

