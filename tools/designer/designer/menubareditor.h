/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MENUBAREDITOR_H
#define MENUBAREDITOR_H

#include <qmenubar.h>
#include <qmap.h>

class PopupMenuEditor;
class MenuBarEditor;
class QActionGroup;

class MenuBarEditorItem : public QObject
{
    Q_OBJECT

    friend class MenuBarEditor;

    MenuBarEditorItem( MenuBarEditor * bar = 0, QObject * parent = 0, const char * name = 0 );

public:
    MenuBarEditorItem( PopupMenuEditor * menu, MenuBarEditor * bar,
		       QObject * parent = 0, const char * name = 0);
    MenuBarEditorItem( QActionGroup * actionGroup, MenuBarEditor * bar,
		       QObject * parent = 0, const char * name = 0);
    MenuBarEditorItem( MenuBarEditorItem * item,
		       QObject * parent = 0, const char * name = 0);
    ~MenuBarEditorItem() { };

    PopupMenuEditor * menu() { return popupMenu; }

    void setMenuText( const QString t ) { text = t; };
    QString menuText() { return text; }

    void setVisible( bool enable ) { visible = enable; }
    bool isVisible() { return visible; }

    void setRemovable( bool enable ) { removable = enable; }
    bool isRemovable() { return removable; }

    bool isSeparator() { return separator; }
protected:
    void setSeparator( bool enable ) { separator = enable; }

private:
    MenuBarEditor * menuBar;
    PopupMenuEditor * popupMenu;
    QString text;
    uint visible : 1;
    uint separator : 1;
    uint removable : 1;
};

class QLineEdit;
class FormWindow;

class MenuBarEditor : public QMenuBar
{
    Q_OBJECT

public:
    MenuBarEditor( FormWindow * fw, QWidget * parent = 0, const char * name = 0 );
    ~MenuBarEditor();

    FormWindow * formWindow();

    MenuBarEditorItem * createItem( int index = -1, bool addToCmdStack = TRUE );
    void insertItem( MenuBarEditorItem * item, int index = -1 );
    void insertItem( const QString &text, PopupMenuEditor * menu, int index = -1 );
    void insertItem( const QString &text, QActionGroup * group, int index = -1 );

    void insertSeparator( int index = -1 );

    void removeItemAt( int index );
    void removeItem( MenuBarEditorItem * item );

    int findItem( MenuBarEditorItem * item );
    int findItem( PopupMenuEditor * menu );
    int findItem( const QPoint &pos );

    MenuBarEditorItem * item( int index = -1 );

    int count() const;
    int current() const;

    void cut( int index );
    void copy( int index );
    void paste( int index );
    void exchange( int a, int b );

    void showLineEdit( int index = -1);
    void showItem( int index = -1 );
    void hideItem( int index = -1 );
    void focusItem( int index = -1 );
    void deleteItem( int index = -1 );

    QSize sizeHint() const;
    QSize minimumSize() const { return sizeHint(); }
    QSize minimumSizeHint() const { return sizeHint(); }
    int heightForWidth( int max_width ) const;

    void show();

    void checkAccels( QMap<QChar, QWidgetList > &accels );

public slots:
    void cut();
    void copy();
    void paste();

protected:
    bool eventFilter( QObject * o, QEvent * e );
    void paintEvent( QPaintEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void dragEnterEvent( QDragEnterEvent * e );
    void dragLeaveEvent( QDragLeaveEvent * e );
    void dragMoveEvent( QDragMoveEvent * e );
    void dropEvent( QDropEvent * e );
    void keyPressEvent( QKeyEvent * e );
    void focusOutEvent( QFocusEvent * e );
    void resizeEvent( QResizeEvent * e ) { QFrame::resizeEvent( e ); }

    void resizeInternals();

    void drawItems( QPainter & p );
    void drawItem( QPainter & p, MenuBarEditorItem * i, int idx, QPoint &pos );
    void drawSeparator( QPainter & p, const QPoint &pos );

    QSize itemSize( MenuBarEditorItem * i );
    void addItemSizeToCoords( MenuBarEditorItem * i, int & x, int & y, int w );

    QPoint itemPos( int index );
    QPoint snapToItem( const QPoint &pos );
    void dropInPlace( MenuBarEditorItem * i, const QPoint &pos );

    void safeDec();
    void safeInc();

    void navigateLeft( bool ctrl );
    void navigateRight( bool ctrl );
    void enterEditMode();
    void leaveEditMode();

    QPixmap createTextPixmap( const QString &text );

private:
    FormWindow * formWnd;
    QLineEdit * lineEdit;
    QWidget * dropLine;
    QList<MenuBarEditorItem*> itemList;
    MenuBarEditorItem addItem;
    MenuBarEditorItem addSeparator;
    MenuBarEditorItem * draggedItem;
    QPoint mousePressPos;
    int currentIndex;
    int itemHeight;
    int separatorWidth;
    int borderSize;
    bool hideWhenEmpty;
    bool hasSeparator;
    bool dropConfirmed;

    enum ClipboardOperation {
	None = 0,
	Cut = 1,
	Copy = 2
    };
    static int clipboardOperation;
    static MenuBarEditorItem * clipboardItem;
};

#endif //MENUBAREDITOR_H
