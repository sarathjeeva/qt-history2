/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef VARIABLEDIALOGIMPL_H
#define VARIABLEDIALOGIMPL_H

#include "variabledialog.h"

class FormWindow;
class QListView;

class VariableDialog : public VariableDialogBase
{
    Q_OBJECT
public:
    VariableDialog( FormWindow *fw, QWidget* parent = 0 );
    ~VariableDialog();

protected slots:
    void okClicked();
    void addVariable();
    void deleteVariable();
    void nameChanged();
    void accessChanged();
    void currentItemChanged( QListViewItem *i );

private:
    FormWindow *formWindow;
};

#endif
