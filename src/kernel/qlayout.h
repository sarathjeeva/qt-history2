/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayout.h#24 $
**
** Definition of layout classes
**
** Created : 960416
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QLAYOUT_H
#define QLAYOUT_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

class QMenuBar;
class QWidget;
struct QLayoutData;
class QLayoutArray;
class QLayoutBox;

class Q_EXPORT QLayout : public QObject
{
    Q_OBJECT
public:
    QLayout( QWidget *parent, int border=0, int autoBorder=-1,
	     const char *name=0 );
    QLayout( int autoBorder=-1, const char *name=0 );
    virtual ~QLayout();
    int defaultBorder() const { return insideSpacing; }

    enum { unlimited = QCOORD_MAX };

    void freeze( int w, int h );
    void freeze() { freeze( 0, 0 ); }

    virtual void  setMenuBar( QMenuBar *w );

    QWidget *mainWidget();
    QMenuBar *menuBar() const { return menubar; }
    bool isTopLevel() const { return topLevel; }
    const QRect &geometry() { return rect; }
#if 1	//OBSOLETE
    bool activate() { return FALSE; }
#endif
    virtual bool fixedWidth();
    virtual bool fixedHeight();
    virtual QSize minSize() = 0;
    //    virtual void clearCache();
    virtual void setGeometry( const QRect& );
protected:
    bool  eventFilter( QObject *, QEvent * );
    virtual void paintEvent( QPaintEvent * );
    virtual void childRemoved( QWidget * ) = 0;
    void addChildLayout( QLayout *l );
private:
    int insideSpacing;
    int outsideBorder;
    bool    topLevel;
    QRect rect;
    QLayoutData *extraData;
    QMenuBar *menubar;
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLayout( const QLayout & );
    QLayout &operator=( const QLayout & );
#endif

};


class Q_EXPORT QGridLayout : public QLayout
{
    Q_OBJECT
public:
    QGridLayout( QWidget *parent, int nRows, int nCols, int border=0,
		 int autoBorder = -1, const char *name=0 );
    QGridLayout( int nRows, int nCols, int autoBorder = -1,
		 const char *name=0 );
    ~QGridLayout();

    QSize minSize();

    virtual void setRowStretch( int row, int stretch );
    virtual void setColStretch( int col, int stretch );

    int numRows() const;
    int numCols() const;
    bool fixedWidth();
    bool fixedHeight();
    //    void clearCache();

    void add( QWidget*, int row, int col );
    void add( QWidget*, int row1, int row2, int col1, int col2 );
    void add( QLayout*, int row, int col );
    //    void add( QSize, int row, int col );

    // void setAlignment( QWidget* );

#if 1	//OBSOLETE
    void addWidget( QWidget *, int row, int col, int align = 0 );
    void addMultiCellWidget( QWidget *, int fromRow, int toRow,
			       int fromCol, int toCol, int align = 0 );
    void addLayout( QLayout *layout, int row, int col);
    void addRowSpacing( int row, int minsize );
    void addColSpacing( int col, int minsize );
    void expand( int rows, int cols );
#endif
    enum Corner { TopLeft, TopRight, BottomLeft, BottomRight };
    void setOrigin( Corner );
protected:
    void childRemoved( QWidget * );
    void setGeometry( const QRect& );
    void add( QLayoutBox*, int row, int col );
private:
    void init( int rows, int cols );
    QLayoutArray *array;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGridLayout( const QGridLayout & );
    QGridLayout &operator=( const QGridLayout & );
#endif
};


class Q_EXPORT QBoxLayout : public QGridLayout
{
    Q_OBJECT
public:
    enum Direction { LeftToRight, RightToLeft, TopToBottom, BottomToTop,
		     Down = TopToBottom, Up = BottomToTop };

    QBoxLayout( QWidget *parent, Direction, int border=0,
		int autoBorder = -1, const char *name=0 );

    QBoxLayout(	Direction, int autoBorder = -1,
		const char *name=0 );

    ~QBoxLayout();

    Direction direction() const { return dir; }

#if 1	//OBSOLETE
    void addSpacing( int size );
    void addStretch( int stretch = 0 );
    void addWidget( QWidget *, int stretch = 0, int alignment = AlignCenter );
    void addLayout( QLayout *layout, int stretch = 0 );
    void addStrut( int );
#endif
private:
    QRect lastKnownGeom;
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
		int autoBorder = -1, const char *name=0 );

    QHBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QHBoxLayout();
};



class Q_EXPORT QVBoxLayout : public QBoxLayout
{
    Q_OBJECT
public:
    QVBoxLayout( QWidget *parent, int border=0,
		int autoBorder = -1, const char *name=0 );

    QVBoxLayout( int autoBorder = -1, const char *name=0 );

    ~QVBoxLayout();
};




#endif // QLAYOUT_H
