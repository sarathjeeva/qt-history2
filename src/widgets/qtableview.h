/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qtableview.h#3 $
**
** Definition of QTableView class
**
** Author  : Eirik Eng
** Created : 941115
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QTABLEVW_H
#define QTABLEVW_H

#include "qframe.h"

class QScrollBar;
class CornerSquare;


class QTableView : public QFrame
{
    Q_OBJECT
public:
    void	setBackgroundColor( const QColor & );
    void	setPalette( const QPalette & );
    void	show();

    void	repaint( bool erase=TRUE );
    void	repaint( int x, int y, int w, int h, bool erase=TRUE );
    void	repaint( const QRect &, bool erase=TRUE );

protected:
    QTableView( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QTableView();

    int		numRows()	const;
    void	setNumRows( int );
    int		numCols()	const;
    void	setNumCols( int );

    int		topCell()	const;
    void	setTopCell( int row );
    int		leftCell()	const;
    void	setLeftCell( int col );
    void	setTopLeftCell( int row, int col );

    int		xOffset()	const;
    void	setXOffset( int );
    int		yOffset()	const;
    void	setYOffset( int );
    void	setOffset( int x, int y, bool update = TRUE );

    virtual int cellWidth( int col );
    virtual int cellHeight( int row );
    int		cellWidth()	const;
    int		cellHeight()	const;
    void	setCellWidth( int );
    void	setCellHeight( int );

    virtual int totalWidth();
    virtual int totalHeight();

    ulong	tableFlags()	const;
    bool	testTableFlags( ulong f ) const;
    void	setTableFlags( ulong f );
    void	clearTableFlags( ulong f = ~0 );

    bool	autoUpdate()	 const;
    void	setAutoUpdate( bool );

    void	updateCell( int row, int column, bool erase=TRUE );

    QRect	cellUpdateRect() const;
    QRect	viewRect()	 const;

    int		lastRowVisible() const;
    int		lastColVisible() const;

    bool	rowIsVisible( int row ) const;
    bool	colIsVisible( int col ) const;

private slots:
    void	horSbValue( int );
    void	horSbSliding( int );
    void	horSbSlidingDone();
    void	verSbValue( int );
    void	verSbSliding( int );
    void	verSbSlidingDone();

protected:
    virtual void paintCell( QPainter *, int row, int col ) = 0;
    virtual void setupPainter( QPainter * );

    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );

    int		findRow( int yPos ) const;
    int		findCol( int xPos ) const;

    bool	rowYPos( int row, int *xPos ) const;
    bool	colXPos( int col, int *yPos ) const;

    int		maxXOffset();
    int		maxYOffset();
    int		maxColOffset();
    int		maxRowOffset();

    int		maxViewX()	const;
    int		maxViewY()	const;
    int		viewWidth()	const;
    int		viewHeight()	const;

    void	scroll( int xPixels, int yPixels );
    void	updateScrollBars();

private:
    void	coverCornerSquare( bool );
    void	snapToGrid( bool horizontal, bool vertical );
    void	setHorScrollBar( bool on, bool update = TRUE );
    void	setVerScrollBar( bool on, bool update = TRUE );
    void	updateView();
    int		findRawRow( int yPos, int *cellMaxY, int *cellMinY = 0,
			    bool goOutsideView = FALSE ) const;
    int		findRawCol( int xPos, int *cellMaxX, int *cellMinX = 0,
			    bool goOutsideView = FALSE ) const;
    int		maxColsVisible() const;

    void	updateScrollBars( uint );
    void	updateFrameSize();

    void	doAutoScrollBars();
    void	showOrHideScrollBars();

    int		nRows;
    int		nCols;
    int		xOffs, yOffs;
    int		xCellOffs, yCellOffs;
    short	xCellDelta, yCellDelta;
    short	cellH, cellW;

    uint	doUpdate		: 1;
    uint	eraseInPaint		: 1;
    uint	verSliding		: 1;
    uint	verSnappingOff		: 1;
    uint	horSliding		: 1;
    uint	horSnappingOff		: 1;
    uint	coveringCornerSquare	: 1;
    uint	sbDirty			: 8;
    uint	inSbUpdate		: 1;

    ulong	tFlags;
    QRect	cellUpdateR;

    QScrollBar *vScrollBar;
    QScrollBar *hScrollBar;
    CornerSquare *cornerSquare;
};


const ulong Tbl_vScrollBar	   = 0x00000001;
const ulong Tbl_hScrollBar	   = 0x00000002;
const ulong Tbl_autoVScrollBar	   = 0x00000004;
const ulong Tbl_autoHScrollBar	   = 0x00000008;
const ulong Tbl_autoScrollBars	   = 0x0000000C;

const ulong Tbl_clipCellPainting   = 0x00000100;
const ulong Tbl_cutCellsV	   = 0x00000200;
const ulong Tbl_cutCellsH	   = 0x00000400;
const ulong Tbl_cutCells	   = 0x00000600;

const ulong Tbl_scrollLastHCell	   = 0x00000800;
const ulong Tbl_scrollLastVCell	   = 0x00001000;
const ulong Tbl_scrollLastCell	   = 0x00001800;

const ulong Tbl_smoothHScrolling   = 0x00002000;
const ulong Tbl_smoothVScrolling   = 0x00004000;
const ulong Tbl_smoothScrolling	   = 0x00006000;

const ulong Tbl_snapToHGrid	   = 0x00008000;
const ulong Tbl_snapToVGrid	   = 0x00010000;
const ulong Tbl_snapToGrid	   = 0x00018000;


inline int QTableView::numRows() const
{ return nRows; }

inline int QTableView::numCols() const
{ return nCols; }

inline int QTableView::topCell() const
{ return yCellOffs; }

inline int QTableView::leftCell() const
{ return xCellOffs; }

inline int QTableView::xOffset() const
{ return xOffs; }

inline int QTableView::yOffset() const
{ return yOffs; }

inline int QTableView::cellHeight() const
{ return cellH; }

inline int QTableView::cellWidth() const
{ return cellW; }

inline ulong QTableView::tableFlags() const
{ return tFlags; }

inline bool QTableView::testTableFlags( ulong f ) const
{ return (tFlags & f) != 0; }

inline QRect QTableView::cellUpdateRect() const
{ return cellUpdateR; }

inline bool QTableView::autoUpdate() const
{ return doUpdate; }

inline void QTableView::repaint( bool erase )
{ repaint( rect(), erase ); }

inline void QTableView::repaint( int x, int y, int w, int h, bool erase )
{ repaint( QRect(x,y,w,h), erase ); }

inline void QTableView::updateScrollBars()
{ updateScrollBars( 0 ); }


#endif // QTABLEVW_H
