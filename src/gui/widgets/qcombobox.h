/****************************************************************************
**
** Definition of QComboBox class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_COMBOBOX


class QStringList;
class QLineEdit;
class QValidator;
class QListBox;
class QComboBoxPrivate;
class QWheelEvent;

class Q_GUI_EXPORT QComboBox : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QComboBox);

    Q_ENUMS( Policy )
    Q_PROPERTY( bool editable READ editable WRITE setEditable )
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( QString currentText READ currentText WRITE setCurrentText DESIGNABLE false )
    Q_PROPERTY( int currentItem READ currentItem WRITE setCurrentItem )
    Q_PROPERTY( bool autoResize READ autoResize WRITE setAutoResize DESIGNABLE false )
    Q_PROPERTY( int sizeLimit READ sizeLimit WRITE setSizeLimit )
    Q_PROPERTY( int maxCount READ maxCount WRITE setMaxCount )
    Q_PROPERTY( Policy insertionPolicy READ insertionPolicy WRITE setInsertionPolicy )
    Q_PROPERTY( bool autoCompletion READ autoCompletion WRITE setAutoCompletion )
    Q_PROPERTY( bool duplicatesEnabled READ duplicatesEnabled WRITE setDuplicatesEnabled )
    Q_OVERRIDE( bool autoMask DESIGNABLE true SCRIPTABLE true )

public:
    QComboBox( QWidget* parent=0, const char* name=0 );
    QComboBox( bool rw, QWidget* parent=0, const char* name=0 );
    ~QComboBox();

    int		count() const;

    void	insertStringList( const QStringList &, int index=-1 );

    void	insertItem( const QString &text, int index=-1 );
    void	insertItem( const QPixmap &pixmap, int index=-1 );
    void	insertItem( const QPixmap &pixmap, const QString &text, int index=-1 );

    void	removeItem( int index );

    int		currentItem() const;
    virtual void setCurrentItem( int index );

    QString 	currentText() const;
    virtual void setCurrentText( const QString& );

    QString 	text( int index ) const;
    const QPixmap *pixmap( int index ) const;

    void	changeItem( const QString &text, int index );
    void	changeItem( const QPixmap &pixmap, int index );
    void	changeItem( const QPixmap &pixmap, const QString &text, int index );

    bool	autoResize()	const;
    virtual void setAutoResize( bool );
    QSize	sizeHint() const;

    void	setPalette( const QPalette & );
    void	setFont( const QFont & );
    void	setEnabled( bool );

    virtual void setSizeLimit( int );
    int		sizeLimit() const;

    virtual void setMaxCount( int );
    int		maxCount() const;

    enum Policy { NoInsertion, AtTop, AtCurrent, AtBottom,
		  AfterCurrent, BeforeCurrent };

    virtual void setInsertionPolicy( Policy policy );
    Policy	insertionPolicy() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    virtual void setListBox( QListBox * );
    QListBox *	listBox() const;

    virtual void setLineEdit( QLineEdit *edit );
    QLineEdit*	lineEdit() const;

    virtual void setAutoCompletion( bool );
    bool	autoCompletion() const;

    bool	eventFilter( QObject *object, QEvent *event );

    void	setDuplicatesEnabled( bool enable );
    bool	duplicatesEnabled() const;

    bool	editable() const;
    void	setEditable( bool );

    virtual void popup();

    void	hide();

public slots:
    void	clear();
    void	clearValidator();
    void	clearEdit();
    virtual void setEditText( const QString &);

signals:
    void	activated( int index );
    void	highlighted( int index );
    void	activated( const QString &);
    void	highlighted( const QString &);
    void	textChanged( const QString &);

private slots:
    void	internalActivate( int );
    void	internalHighlight( int );
    void	internalClickTimeout();
    void	returnPressed();

protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	focusInEvent( QFocusEvent *e );
    void	focusOutEvent( QFocusEvent *e );
#ifndef QT_NO_WHEELEVENT
    void	wheelEvent( QWheelEvent *e );
#endif
    void        changeEvent( QEvent * );

    void	updateMask();

private:

#if defined(Q_DISABLE_COPY)
    QComboBox( const QComboBox & );
    QComboBox &operator=( const QComboBox & );
#endif
};


#endif // QT_NO_COMBOBOX

#endif // QCOMBOBOX_H
