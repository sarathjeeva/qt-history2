/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "fortuneserver.h"

QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QPushButton)

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

private:
    QLabel *statusLabel;
    QPushButton *quitButton;
    FortuneServer server;
};

#endif
