/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.h#30 $
**
** Definition of QCursor class
**
** Created : 940219
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

#ifndef QCURSOR_H
#define QCURSOR_H

#ifndef QT_H
#include "qpoint.h"
#include "qshared.h"
#endif // QT_H


struct QCursorData;				// internal cursor data


class Q_EXPORT QCursor					// cursor class
{
public:
    QCursor();					// create default arrow cursor
    QCursor( int shape );
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    int		  shape()   const;
    void	  setShape( int );

    const QBitmap *bitmap() const;
    const QBitmap *mask()   const;
    QPoint	  hotSpot() const;

    HANDLE	  handle()  const;

    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );

    static void	  initialize();
    static void	  cleanup();

private:
    void	  update() const;
    QCursorData	 *data;
    QCursor	 *find_cur(int);
};


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.x(), p.y() );
}


/*****************************************************************************
  Cursor shape identifiers (correspond to global cursor objects)
 *****************************************************************************/

enum QCursorShape {
    ArrowCursor, UpArrowCursor, CrossCursor, WaitCursor, IbeamCursor,
    SizeVerCursor, SizeHorCursor, SizeBDiagCursor, SizeFDiagCursor,
    SizeAllCursor, BlankCursor, SplitVCursor, SplitHCursor,
    LastCursor=SplitHCursor, BitmapCursor=24 };


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern Q_EXPORT const QCursor arrowCursor;	// standard arrow cursor
extern Q_EXPORT const QCursor upArrowCursor;	// upwards arrow
extern Q_EXPORT const QCursor crossCursor;	// crosshair
extern Q_EXPORT const QCursor waitCursor;	// hourglass/watch
extern Q_EXPORT const QCursor ibeamCursor;	// ibeam/text entry
extern Q_EXPORT const QCursor sizeVerCursor;	// vertical resize
extern Q_EXPORT const QCursor sizeHorCursor;	// horizontal resize
extern Q_EXPORT const QCursor sizeBDiagCursor;	// diagonal resize (/)
extern Q_EXPORT const QCursor sizeFDiagCursor;	// diagonal resize (\)
extern Q_EXPORT const QCursor sizeAllCursor;	// all directions resize
extern Q_EXPORT const QCursor blankCursor;	// blank/invisible cursor
extern Q_EXPORT const QCursor splitVCursor;	// vertical bar with left-rigth arrows
extern Q_EXPORT const QCursor splitHCursor;	// horizontal bar with up-down arrows


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QCursor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCursor & );


#endif // QCURSOR_H
