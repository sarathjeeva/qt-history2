/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayout.h#48 $
**
** Definition of layout classes
**
** Created : 960416
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLAYOUT_H
#define QLAYOUT_H

#ifndef QT_H
#include "qabstractlayout.h"
#endif // QT_H

class QLayoutArray;
class QLayoutBox;

#if 0
Q_OBJECT
#endif

class Q_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
public:
    QGridLayout( QWidget *parent, int nRows = 1, int nCols = 1, int border=0,
		 int space = -1, const char *name=0 );
    QGridLayout( int nRows = 1, int nCols = 1, int space = -1,
		 const char *name=0 );
    QGridLayout( QLayout *parentLayout, int nRows = 1, int nCols = 1,
		 int space = -1, const char *name=0 );
    ~QGridLayout();

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    virtual void setRowStretch( int row, int stretch );
    virtual void setColStretch( int col, int stretch );
    int rowStretch( int row ) const;
    int colStretch( int col ) const;

    int numRows() const;
    int numCols() const;
    QRect cellGeometry( int row, int col ) const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;


    QSizePolicy::ExpandData expanding() const;
    void invalidate();

    void addItem( QLayoutItem * );
    void addItem( QLayoutItem *item, int row, int col );
    void addMultiCell( QLayoutItem *, int fromRow, int toRow,
			       int fromCol, int toCol, int align = 0 );
    // void setAlignment( QWidget* );

    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow,
			     int fromCol, int toCol, int align = 0 );
    void addLayout( QLayout *layout, int row, int col);
    void addMultiCellLayout( QLayout *layout, int fromRow, int toRow,
			     int fromCol, int toCol, int align = 0 );
    void addRowSpacing( int row, int minsize );
    void addColSpacing( int col, int minsize );
    void expand( int rows, int cols );

    enum Corner { TopLeft, TopRight, BottomLeft, BottomRight };
    void setOrigin( Corner );
    QLayoutIterator iterator();
    void setGeometry( const QRect& );

protected:
    bool findWidget( QWidget* w, int *r, int *c );
    void add( QLayoutItem*, int row, int col );
private:
    void init( int rows, int cols );
    QLayoutArray *array;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGridLayout( const QGridLayout & );
    QGridLayout &operator=( const QGridLayout & );
#endif
};


class QBoxLayoutData;

class Q_EXPORT QBoxLayout : public QLayout
{
    Q_OBJECT
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
		     Down = TopToBottom, Up = BottomToTop };

    QBoxLayout( QWidget *parent, Direction, int border=0,
		int space = -1, const char *name=0 );

    QBoxLayout( QLayout *parentLayout, Direction, int space = -1,
		const char *name=0 );

    QBoxLayout(	Direction, int space = -1,
		const char *name=0 );

    ~QBoxLayout();

    void addItem( QLayoutItem * );

    Direction direction() const { return dir; }
    void setDirection( Direction );

    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, int alignment = 0 );
    void addLayout( QLayout *layout, int stretch = 0 );
    void addStrut( int );

    void insertSpacing( int index, int size );
    void insertStretch( int index, int stretch = 0 );
    void insertWidget( int index, QWidget *widget, int stretch = 0,
		       int alignment = 0 );
    void insertLayout( int index, QLayout *layout, int stretch = 0 );


    bool setStretchFactor( QWidget*, int stretch );
    bool setStretchFactor( QLayout *l, int stretch );

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize maximumSize() const;

    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;

    QSizePolicy::ExpandData expanding() const;
    void invalidate();
    QLayoutIterator iterator();
    void setGeometry( const QRect& );

    int findWidget( QWidget* w );
protected:
    void insertItem( int index, QLayoutItem * );

private:
    void setupGeom();
    int calcHfw( int );
    QBoxLayoutData *data;
    Direction dir;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QBoxLayout( const QBoxLayout & );
    QBoxLayout &operator=( const QBoxLayout & );
#endif

};


class Q_EXPORT QHBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QHBoxLayout( QWidget *parent, int border=0,
		int space = -1, const char *name=0 );
    QHBoxLayout( QLayout *parentLayout,
		 int space = -1, const char *name=0 );
    QHBoxLayout( int space = -1, const char *name=0 );

    ~QHBoxLayout();

};



class Q_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout( QWidget *parent, int border=0,
		int space = -1, const char *name=0 );
    QVBoxLayout( QLayout *parentLayout,
		 int space = -1, const char *name=0 );
    QVBoxLayout( int space = -1, const char *name=0 );

    ~QVBoxLayout();

};




#endif // QLAYOUT_H
