/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.h#17 $
**
** Definition of QListView widget class
**
** Created : 970809
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QLISTVIEW_H
#define QLISTVIEW_H

class QStrList;
class QPixmap;
class QFont;

class QListView;
struct QListViewPrivate;


#include "qscrollview.h"


class QListViewItem
{
public:
    QListViewItem( QListView * parent );
    QListViewItem( QListViewItem * parent );
    QListViewItem( QListView * parent, const char * firstLabel, ... );
    QListViewItem( QListViewItem * parent, const char * firstLabel, ... );
    virtual ~QListViewItem();

    virtual void insertItem( QListViewItem * );
    virtual void removeItem( QListViewItem * );

    int height() const { return ownHeight; }
    virtual void invalidateHeight();
    int totalHeight() const;

    virtual const char * text( int ) const;

    virtual const char * key( int ) const;
    virtual void sortChildItems( int, bool );

    int children() const { return childCount; }

    bool isOpen() const { return open && childCount>0; } // ###
    virtual void setOpen( bool );
    virtual void setup();

    virtual void setSelected( bool );
    bool isSelected() const { return selected; }

    virtual void paintCell( QPainter *,  const QColorGroup & cg,
			    int column, int width, bool showFocus ) const;
    virtual void paintBranches( QPainter * p, const QColorGroup & cg,
				int w, int y, int h, GUIStyle s ) const;

    const QListViewItem * firstChild() const { return childItem; }
    const QListViewItem * nextSibling() const { return siblingItem; }

    QListViewItem * itemAbove();
    QListViewItem * itemBelow();

    virtual QListView *listView() const;

    virtual void setSelectable( bool enable );
    bool isSelectable() const { return selectable; }

    virtual void setExpandable( bool );
    bool isExpandable() { return expandable; }

protected:
    void enforceSortOrder();
    void setHeight( int );

private:
    void init();
    int ownHeight;
    int maybeTotalHeight;
    int childCount;

    uint lsc: 15;
    uint lso: 1;
    uint open : 1;
    uint selected : 1;
    uint selectable: 1;
    uint configured: 1;
    uint expandable: 1;

    QListViewItem * parentItem;
    QListViewItem * siblingItem;
    QListViewItem * childItem;

    QStrList * columnTexts;

    friend QListView;
};


class QListView: public QScrollView
{
    Q_OBJECT
public:
    QListView( QWidget * parent = 0, const char * name = 0 );
    ~QListView();

    int treeStepSize() const;
    virtual void setTreeStepSize( int );

    virtual void insertItem( QListViewItem * );
    virtual void clear();

    virtual void setColumn( const char * label, int size, int column=-1 );

    void show();

    QListViewItem * itemAt( QPoint screenPos ) const;
    QRect itemRect( QListViewItem * ) const;
    int itemPos( QListViewItem * );

    virtual void setMultiSelection( bool enable );
    bool isMultiSelection() const;
    virtual void setSelected( QListViewItem *, bool );
    bool isSelected( QListViewItem * ) const;

    virtual void setCurrentItem( QListViewItem * );
    QListViewItem * currentItem() const;

    const QListViewItem * firstChild() const;

    virtual void setAllColumnsShowFocus( bool );
    bool allColumnsShowFocus() const;

    virtual void setSorting( int column, bool increasing = TRUE );

    void setStyle( GUIStyle );
    void setFont( const QFont & );
    void setPalette( const QPalette & );

    bool eventFilter( QObject * o, QEvent * );

 public slots:
    void triggerUpdate();

signals:
    void sizeChanged();

    void selectionChanged();
    void selectionChanged( QListViewItem * );
    void currentChanged( QListViewItem * );

    void doubleClicked( QListViewItem * );
    void returnPressed( QListViewItem * );
    void rightButtonClicked( QListViewItem *, const QPoint&, int );

protected:
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );

    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void keyPressEvent( QKeyEvent *e );

    void drawContentsOffset( QPainter *, int ox, int oy,
			     int cx, int cy, int cw, int ch );

protected slots:
    void updateContents();

private slots:
    void changeSortColumn( int );

private:
    void buildDrawableList() const;
    void reconfigureItems();

    QListViewPrivate * d;

    friend QListViewItem;
};


#endif // QLISTVIEW_H
