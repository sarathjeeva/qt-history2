/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qradiobutton.h#28 $
**
** Definition of QRadioButton class
**
** Created : 940222
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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class QRadioButton : public QButton
{
    Q_OBJECT
public:
    QRadioButton( QWidget *parent=0, const char *name=0 );
    QRadioButton( const char *text, QWidget *parent=0, const char *name=0 );

    bool    isChecked() const;
    virtual void    setChecked( bool check );

    QSize    sizeHint() const;

protected:
    bool    hitButton( const QPoint & ) const;
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

    void    resizeEvent( QResizeEvent* );
    void    mouseReleaseEvent( QMouseEvent * );
    void    keyPressEvent( QKeyEvent * );

private:
    void    init();
    uint    noHit : 1;

private:	// Disabled copy constructor and operator=
    QRadioButton( const QRadioButton & );
    QRadioButton &operator=( const QRadioButton & );
};


inline bool QRadioButton::isChecked() const
{ return isOn(); }

inline void QRadioButton::setChecked( bool check )
{ setOn( check ); }


#endif // QRADIOBUTTON_H
