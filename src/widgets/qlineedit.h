/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlineedit.h#56 $
**
** Definition of QLineEdit widget class
**
** Created : 941011
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

#ifndef QLINEEDIT_H
#define QLINEEDIT_H

struct QLineEditPrivate;

class QComboBox;
class QValidator;


#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#endif // QT_H


class QLineEdit : public QWidget
{
    Q_OBJECT
public:
    QLineEdit( QWidget *parent=0, const char *name=0 );
   ~QLineEdit();

    const char *text() const;
    int		maxLength()	const;
    virtual void	setMaxLength( int );

    virtual void	setFrame( bool );
    bool	frame() const;

    enum	EchoMode { Normal, NoEcho, Password };
    virtual void	setEchoMode( EchoMode );
    EchoMode 	echoMode() const;

    virtual void	setValidator( QValidator * );
    QValidator * validator() const;

    QSize	sizeHint() const;

    virtual void	setEnabled( bool );
    virtual void	setFont( const QFont & );
    virtual void	setPalette( const QPalette & );

    virtual void	setSelection( int, int );
    virtual void	setCursorPosition( int );
    int		cursorPosition() const;

    bool	validateAndSet( const char *, int, int, int );

 public slots:
    virtual void	setText( const char * );
    void	selectAll();
    void	deselect();

    void	clearValidator();

    void	insert( const char * );

    void	clear();

signals:
    void	textChanged( const char * );
    void	returnPressed();

protected:
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
    void	resizeEvent( QResizeEvent * );
    void	leaveEvent( QEvent * );

    bool	event( QEvent * );

    bool	hasMarkedText() const;
    QString	markedText() const;


    void	repaintArea( int, int );

private slots:
    void	clipboardChanged();
    void	blinkSlot();
    void	dragScrollSlot();

private:
    // obsolete
    void	paint( const QRect& clip, bool frame = FALSE );
    void	pixmapPaint( const QRect& clip );
    // kept
    void	paintText( QPainter *, const QSize &, bool frame = FALSE );
    // to be replaced by publics
    void	cursorLeft( bool mark, int steps = 1 );
    void	cursorRight( bool mark, int steps = 1 );
    void	backspace();
    void	del();
    void	home( bool mark );
    void	end( bool mark );
    // kept
    void	newMark( int pos, bool copy=TRUE );
    void	markWord( int pos );
    void	copyText();
    int		lastCharVisible() const;
    int		minMark() const;
    int		maxMark() const;

    QString	tbuf;
    QLineEditPrivate * d;
    int		cursorPos;
    int		offset;
    int		maxLen;
    int		markAnchor;
    int		markDrag;
    bool	cursorOn;
    bool	dragScrolling;
    bool	scrollingLeft;

private:	// Disabled copy constructor and operator=
    QLineEdit( const QLineEdit & );
    QLineEdit &operator=( const QLineEdit & );

    friend class QComboBox;
};


#endif // QLINEEDIT_H
