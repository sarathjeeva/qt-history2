#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <qstring.h>
#include <qnamespace.h>

struct QTextEngine;
class QFont;

class Q_EXPORT QTextItem
{
public:
    int width() const;
    int ascent() const;
    int descent() const;
    int baselineAdjustment() const;

    enum Edge {
	Leading,
	Trailing
    };
    /* cPos gets set to the valid position */
    int cursorToX( int *cPos, Edge edge = Leading );
    int xToCursor( int x );

    bool isRightToLeft() const;
    bool isObject() const;

    void setWidth( int w );
    void setAscent( int a );
    void setDescent( int d );
    void setBaselineAdjustment( int adjust );

    void setFont( const QFont & f );

private:
    friend class QTextLayout;
    QTextItem( int i, QTextEngine *e ) : item( i ), engine( e ) {}
    int item;
    QTextEngine *engine;
};


class QPainter;

class Q_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout( const QString &string, QPainter * = 0 );
    virtual ~QTextLayout();

    enum LineBreakStrategy {
	AtWordBoundaries,
	AtCharBoundaries
    };

    /* add an additional item boundary eg. for style change */
    void setBoundary( int strPos );

    bool validCursorPosition( int strPos );

    int numItems() const;
    QTextItem itemAt( int i );

    void beginLayout();
    void beginLine( int width );

    QTextItem nextItem();
    /* ## maybe also currentItem() */
    void setLineWidth( int newWidth );
    int availableWidth() const;

    /* returns true if completely added */
    bool addCurrentItem();

    void endLine( int x, int y, Qt::AlignmentFlags alignment );
    void endLayout();

private:
    /* disable copy and assignment */
    QTextLayout( const QTextLayout & ) {}
    void operator = ( const QTextLayout & ) {}

    friend class QTextItem;
    QTextEngine *d;
};


/*
  class QPainter {
      .....
      void drawTextItem( int x, int y, QTextItem *item );
  };
*/

#endif
