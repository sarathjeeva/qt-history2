/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qcheckbox.h#21 $
**
** Definition of QCheckBox class
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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#ifndef QT_H
#include "qbutton.h"
#endif // QT_H


class QCheckBox : public QButton
{
    Q_OBJECT
public:
    QCheckBox( QWidget *parent=0, const char *name=0 );
    QCheckBox( const char *text, QWidget *parent, const char *name=0 );

    bool    isChecked() const;
    void    setChecked( bool check );

    QSize sizeHint() const;

protected:
    void    drawButton( QPainter * );
    void    drawButtonLabel( QPainter * );

private:	// Disabled copy constructor and operator=
    QCheckBox( const QCheckBox & );
    QCheckBox &operator=( const QCheckBox & );
};


inline bool QCheckBox::isChecked() const
{ return isOn(); }

inline void QCheckBox::setChecked( bool check )
{ setOn( check ); }


#endif // QCHECKBOX_H
