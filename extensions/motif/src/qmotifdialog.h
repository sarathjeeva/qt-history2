/****************************************************************************
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOTIFDIALOG_H
#define QMOTIFDIALOG_H

#include <qdialog.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#undef Bool
#undef Int

class QMotifDialogPrivate;

class QMotifDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMotifDialog);

public:
    // obsolete
    enum DialogType {
	Prompt,
	Selection,
	Command,
	FileSelection,
	Template,
	Error,
	Information,
	Message,
	Question,
	Warning,
	Working
    };
    // obsolete
    QMotifDialog( DialogType dialogtype,
		  Widget parent, ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, Qt::WFlags flags = 0 );
    // obsolete
    QMotifDialog( Widget parent, ArgList args = NULL, Cardinal argcount = 0,
		  const char *name = 0, bool modal = FALSE, Qt::WFlags flags = 0 );

    QMotifDialog( Widget parent, const char *name = 0,
		  bool modal = FALSE, Qt::WFlags flags = 0 );
    QMotifDialog( QWidget *parent, const char *name = 0,
		  bool modal = FALSE, Qt::WFlags flags = 0 );

    virtual ~QMotifDialog();

    Widget shell() const;
    Widget dialog() const;

    void show();
    void hide();

    static void acceptCallback( Widget, XtPointer, XtPointer );
    static void rejectCallback( Widget, XtPointer, XtPointer );

public slots:
    void accept();
    void reject();

protected:
    bool event( QEvent * );

private:
    void init( Widget parent = NULL, ArgList args = NULL, Cardinal argcount = 0);

    void realize( Widget w );
    void insertChild( Widget w );
    void deleteChild( Widget w );

    friend void qmotif_dialog_realize( Widget, XtValueMask *, XSetWindowAttributes *);
    friend void qmotif_dialog_insert_child( Widget );
    friend void qmotif_dialog_delete_child( Widget );
    friend void qmotif_dialog_change_managed( Widget );
};

#endif // QMOTIFDIALOG_H
