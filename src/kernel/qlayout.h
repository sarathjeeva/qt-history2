/****************************************************************************
**
** Definition of layout classes.
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

#ifndef QLAYOUT_H
#define QLAYOUT_H

#ifndef QT_H
#include "qobject.h"
#include "qsizepolicy.h"
#include "qwidget.h"
#endif // QT_H

#include <limits.h>

#ifndef QT_NO_LAYOUT

#if 0
Q_OBJECT
#endif

static const int QLAYOUTSIZE_MAX = INT_MAX/256/16;

class QGridLayoutBox;
class QGridLayoutData;
class QLayout;
class QLayoutItem;
struct QLayoutData;
class QMenuBar;
class QSpacerItem;
class QWidget;

class Q_GUI_EXPORT QGLayoutIterator
{
public:
    virtual ~QGLayoutIterator();
    virtual QLayoutItem *next() = 0;
    virtual QLayoutItem *current() = 0;
    virtual QLayoutItem *takeCurrent() = 0;
    QAtomic ref;
};

class Q_GUI_EXPORT QLayoutIterator
{
public:
    inline QLayoutIterator( QGLayoutIterator *i ) : it(i) {}
    inline QLayoutIterator(const QLayoutIterator &i) : it(i.it) {
	if (it)
	    ++it->ref;
    }
    inline ~QLayoutIterator() { if (it && !--it->ref) delete it; }
    inline QLayoutIterator &operator=( const QLayoutIterator &i ) {
	QGLayoutIterator *x = i.it;
	if (x)
	    ++x->ref;
	x = qAtomicSetPtr(&it, x);
	if (!--x->ref)
	    delete it;
	return *this;
    }
    inline QLayoutItem *operator++() { return it ? it->next() : 0; }
    inline QLayoutItem *current() { return it ? it->current() : 0; }
    inline QLayoutItem *takeCurrent() { return it ? it->takeCurrent() : 0; }
    void deleteCurrent();

private:
    QGLayoutIterator *it;
};

class Q_GUI_EXPORT QLayoutItem
{
public:
    QLayoutItem( Qt::Alignment alignment = 0 ) : align( alignment ) { }
    virtual ~QLayoutItem();
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSize() const = 0;
    virtual QSize maximumSize() const = 0;
    virtual QSizePolicy::ExpandData expanding() const = 0;
    virtual void setGeometry( const QRect& ) = 0;
    virtual QRect geometry() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool hasHeightForWidth() const;
    virtual int heightForWidth( int ) const;
    virtual int minimumHeightForWidth( int ) const;
    virtual void invalidate();

    virtual QWidget *widget();
    virtual QLayoutIterator iterator();
    virtual QLayout *layout();
    virtual QSpacerItem *spacerItem();

    Qt::Alignment alignment() const { return align; }
    virtual void setAlignment( Qt::Alignment a );

protected:
    Qt::Alignment align;
};

class Q_GUI_EXPORT QSpacerItem : public QLayoutItem
{
public:
    QSpacerItem( int w, int h,
		 QSizePolicy::SizeType hData = QSizePolicy::Minimum,
		 QSizePolicy::SizeType vData = QSizePolicy::Minimum )
	: width( w ), height( h ), sizeP( hData, vData ) { }
    void changeSize( int w, int h,
		     QSizePolicy::SizeType hData = QSizePolicy::Minimum,
		     QSizePolicy::SizeType vData = QSizePolicy::Minimum );
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& );
    QRect geometry() const;
    QSpacerItem *spacerItem();

private:
    int width;
    int height;
    QSizePolicy sizeP;
    QRect rect;
};

class Q_GUI_EXPORT QWidgetItem : public QLayoutItem
{
public:
    QWidgetItem( QWidget *w ) : wid( w ) { }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSizePolicy::ExpandData expanding() const;
    bool isEmpty() const;
    void setGeometry( const QRect& );
    QRect geometry() const;
    virtual QWidget *widget();

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

private:
    QWidget *wid;
};

class Q_GUI_EXPORT QLayout : public QObject, public QLayoutItem
{
    Q_OBJECT
    Q_ENUMS( ResizeMode )
    Q_PROPERTY( int margin READ margin WRITE setMargin )
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )
    Q_PROPERTY( ResizeMode resizeMode READ resizeMode WRITE setResizeMode )

public:
    enum ResizeMode { Auto, FreeResize, Minimum, Fixed };

    QLayout( QWidget *parent, int margin = 0, int spacing = -1,
	     const char *name = 0 );
    QLayout( QLayout *parentLayout, int spacing = -1, const char *name = 0 );
    QLayout( int spacing = -1, const char *name = 0 );
    ~QLayout();

    int margin() const { return outsideBorder; }
    int spacing() const { return insideSpacing; }

    virtual void setMargin( int );
    virtual void setSpacing( int );

    int defaultBorder() const { return insideSpacing; }
    void freeze( int w, int h );
    void freeze() { setResizeMode( Fixed ); }

    void setResizeMode( ResizeMode );
    ResizeMode resizeMode() const;

#ifndef QT_NO_MENUBAR
    virtual void setMenuBar( QMenuBar *w );
    QMenuBar *menuBar() const { return menubar; }
#endif

    QWidget *parentWidget() const;
    bool isTopLevel() const { return topLevel; }

    void invalidate();
    QRect geometry() const;
    bool activate();
    void update();

    void addWidget( QWidget *w );
    virtual void addItem( QLayoutItem * ) = 0;

    void removeWidget( QWidget *w );
    void removeItem( QLayoutItem * );

    QSizePolicy::ExpandData expanding() const;
    QSize minimumSize() const;
    QSize maximumSize() const;
    void setGeometry( const QRect& ) = 0;
    QLayoutIterator iterator() = 0;
    bool isEmpty() const;

    int totalHeightForWidth( int w ) const;
    QSize totalMinimumSize() const;
    QSize totalMaximumSize() const;
    QSize totalSizeHint() const;
    QLayout *layout();

    bool supportsMargin() const { return marginImpl; }

    void setEnabled( bool );
    bool isEnabled() const;

protected:
    bool eventFilter( QObject *, QEvent * );
    void childEvent( QChildEvent *e );
    void addChildLayout( QLayout *l );
    void addChildWidget( QWidget *w );
    void deleteAllItems();

    void setSupportsMargin( bool );
    QRect alignmentRect( const QRect& ) const;

private:
    void init();
    static void activateRecursiveHelper(QLayoutItem *item);
    int insideSpacing;
    int outsideBorder;
    uint topLevel : 1;
    uint enabled : 1;
    uint frozen : 1;
    uint activated : 1;
    uint marginImpl : 1;
    uint autoMinimum : 1;
    uint autoResizeMode : 1;
    uint autoNewChild : 1;
    QRect rect;
    QLayoutData *extraData;
#ifndef QT_NO_MENUBAR
    QMenuBar *menubar;
#endif

private:
#if defined(Q_DISABLE_COPY)
    QLayout( const QLayout & );
    QLayout &operator=( const QLayout & );
#endif

    static void propagateSpacing( QLayout *layout );
#ifndef QT_NO_COMPAT
public:
    inline QWidget *mainWidget() const { return parentWidget(); }
    inline void remove(QWidget *w) { removeWidget(w); }
    void add(QWidget *w) { addWidget(w); }

    inline void setAutoAdd( bool a ) { autoNewChild = a; }
    inline bool autoAdd() const { return autoNewChild; }
#endif
};

inline void QLayoutIterator::deleteCurrent()
{
    delete takeCurrent();
}

class Q_GUI_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
public:
    QGridLayout( QWidget *parent, int nRows = 1, int nCols = 1, int border = 0,
		 int spacing = -1, const char *name = 0 );
    QGridLayout( int nRows = 1, int nCols = 1, int spacing = -1,
		 const char *name = 0 );
    QGridLayout( QLayout *parentLayout, int nRows = 1, int nCols = 1,
		 int spacing = -1, const char *name = 0 );
    ~QGridLayout();

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    // ### remove 'virtual' in 4.0 (or add 'virtual' to set{Row,Col}Spacing())
    virtual void setRowStretch( int row, int stretch );
    virtual void setColStretch( int col, int stretch );
    int rowStretch( int row ) const;
    int colStretch( int col ) const;

    void setRowSpacing( int row, int minSize );
    void setColSpacing( int col, int minSize );
    int rowSpacing( int row ) const;
    int colSpacing( int col ) const;

    int numRows() const;
    int numCols() const;
    QRect cellGeometry( int row, int col ) const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    int minimumHeightForWidth( int ) const;

    QSizePolicy::ExpandData expanding() const;
    void invalidate();

    inline void addWidget(QWidget *w) { QLayout::addWidget(w); }
    void addWidget(QWidget *, int row, int col, Alignment = 0);
    void addWidget(QWidget *, int row, int col, int rowSpan, int colSpan, Alignment = 0);
    void addLayout(QLayout *, int row, int col, Alignment = 0);
    void addLayout(QLayout *, int row, int col, int rowSpan, int colSpan, Alignment = 0);

    void setOrigin( Corner );
    Corner origin() const;
    QLayoutIterator iterator();
    void setGeometry( const QRect& );

    void addItem( QLayoutItem *item, int row, int col, int rowSpan = 1, int colSpan = 1, Alignment = 0 );
protected:
    bool findWidget( QWidget* w, int *r, int *c );
    void addItem( QLayoutItem * );

private:
#if defined(Q_DISABLE_COPY)
    QGridLayout( const QGridLayout & );
    QGridLayout &operator=( const QGridLayout & );
#endif

    void init( int rows, int cols );
    QGridLayoutData *data;
#ifndef QT_NO_COMPAT
public:
    void expand( int rows, int cols );
    inline void addRowSpacing( int row, int minsize ) { addItem(new QSpacerItem(0,minsize), row, 0); }
    inline void addColSpacing( int col, int minsize ) { addItem(new QSpacerItem(minsize,0), 0, col); }
    inline void addMultiCellWidget(QWidget *w, int fromRow, int toRow, int fromCol, int toCol, Alignment align = 0)
	{ addWidget(w, fromRow, fromCol, toRow - fromRow + 1, toCol - fromCol + 1, align); }
    inline void addMultiCell(QLayoutItem *l, int fromRow, int toRow, int fromCol, int toCol, Alignment align = 0)
	{ addItem(l, fromRow, fromCol, toRow - fromRow + 1, toCol - fromCol + 1, align); }
    inline void addMultiCellLayout(QLayout *layout, int fromRow, int toRow, int fromCol, int toCol, Alignment align = 0)
	{ addLayout(layout, fromRow, fromCol, toRow - fromRow + 1, toCol - fromCol + 1, align); }
#endif
};

class QBoxLayoutData;
class QDockWindow;

class Q_GUI_EXPORT QBoxLayout : public QLayout
{
    Q_OBJECT
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
		     Down = TopToBottom, Up = BottomToTop };

    QBoxLayout( QWidget *parent, Direction, int border = 0, int spacing = -1,
		const char *name = 0 );
    QBoxLayout( QLayout *parentLayout, Direction, int spacing = -1,
		const char *name = 0 );
    QBoxLayout( Direction, int spacing = -1, const char *name = 0 );
    ~QBoxLayout();


    Direction direction() const { return dir; }
    void setDirection( Direction );

    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, Alignment alignment = 0 );
    void addLayout( QLayout *layout, int stretch = 0 );
    void addStrut( int );
    void addItem( QLayoutItem * );

    void insertSpacing( int index, int size );
    void insertStretch( int index, int stretch = 0 );
    void insertWidget( int index, QWidget *widget, int stretch = 0,
		       Alignment alignment = 0 );
    void insertLayout( int index, QLayout *layout, int stretch = 0 );

    bool setStretchFactor( QWidget *w, int stretch );
    bool setStretchFactor( QLayout *l, int stretch );
    bool setAlignment( QWidget *w, Alignment alignment );
    bool setAlignment( QLayout *l, Alignment alignment );
    void setAlignment( Alignment alignment );

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    int minimumHeightForWidth( int ) const;

    QSizePolicy::ExpandData expanding() const;
    void invalidate();
    QLayoutIterator iterator();
    void setGeometry( const QRect& );

    int findWidget( QWidget* w );

protected:
    void insertItem( int index, QLayoutItem * );

private:
    friend class QDockWindow;
#if defined(Q_DISABLE_COPY)
    QBoxLayout( const QBoxLayout & );
    QBoxLayout &operator=( const QBoxLayout & );
#endif

    void setupGeom();
    void calcHfw( int );
    QBoxLayoutData *data;
    Direction dir;
    QBoxLayout *createTmpCopy();
};

class Q_GUI_EXPORT QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout( QWidget *parent, int border = 0,
		 int spacing = -1, const char *name = 0 );
    QHBoxLayout( QLayout *parentLayout,
		 int spacing = -1, const char *name = 0 );
    QHBoxLayout( int spacing = -1, const char *name = 0 );

    ~QHBoxLayout();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHBoxLayout( const QHBoxLayout & );
    QHBoxLayout &operator=( const QHBoxLayout & );
#endif
};

class Q_GUI_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout( QWidget *parent, int border = 0,
		 int spacing = -1, const char *name = 0 );
    QVBoxLayout( QLayout *parentLayout,
		 int spacing = -1, const char *name = 0 );
    QVBoxLayout( int spacing = -1, const char *name = 0 );

    ~QVBoxLayout();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVBoxLayout( const QVBoxLayout & );
    QVBoxLayout &operator=( const QVBoxLayout & );
#endif
};

#endif // QT_NO_LAYOUT
#endif // QLAYOUT_H
