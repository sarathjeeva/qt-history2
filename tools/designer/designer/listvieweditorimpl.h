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

#ifndef LISTVIEWEDITORIMPL_H
#define LISTVIEWEDITORIMPL_H

#include "listvieweditor.h"

#include <qmap.h>
#include <qpixmap.h>

class FormWindow;

class ListViewEditor : public ListViewEditorBase
{
    Q_OBJECT

public:
    ListViewEditor( QWidget *parent, QListView *lv, FormWindow *fw );

signals:
    void itemRenamed(const QString &);

protected slots:
    void applyClicked();
    void columnClickable(bool);
    void columnDownClicked();
    void columnPixmapChosen();
    void columnPixmapDeleted();
    void columnResizable(bool);
    void columnTextChanged(const QString &);
    void columnUpClicked();
    void currentColumnChanged(QListBoxItem*);
    void currentItemChanged(QListViewItem*);
    void deleteColumnClicked();
    void itemColChanged(int);
    void itemDeleteClicked();
    void itemDownClicked();
    void itemNewClicked();
    void itemNewSubClicked();
    void itemPixmapChoosen();
    void itemPixmapDeleted();
    void itemTextChanged(const QString &);
    void itemUpClicked();
    void itemLeftClicked();
    void itemRightClicked();
    void newColumnClicked();
    void okClicked();
    void initTabPage(const QString &page);
    void emitItemRenamed(QListViewItem*, int, const QString&); // signal relay

private:
    struct Column
    {
	QListBoxItem *item;
	QString text;
	QPixmap pixmap;
	bool clickable, resizable;
	Q_DUMMY_COMPARISON_OPERATOR( Column )
    };

private:
    void setupColumns();
    void setupItems();
    Column *findColumn( QListBoxItem *i );
    void transferItems( QListView *from, QListView *to );
    void displayItem( QListViewItem *i, int col );

private:
    QListView *listview;
    QList<Column> columns;
    int numColumns;
    FormWindow *formwindow;

};


#endif
