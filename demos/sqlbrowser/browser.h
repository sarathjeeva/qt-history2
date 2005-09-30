/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include "ui_browserwidget.h"

class ConnectionWidget;
class QTableView;
class QPushButton;
class QTextEdit;

class Browser: public QWidget, private Ui::Browser
{
    Q_OBJECT
public:
    Browser(QWidget *parent = 0);
    virtual ~Browser();

public slots:
    void exec();
    void showTable(const QString &table);
    void addConnection();

    void on_connectionWidget_tableActivated(const QString &table)
    { showTable(table); }
    void on_submitButton_clicked() {
        exec();
        sqlEdit->setFocus();
    }
    void on_clearButton_clicked() {
        sqlEdit->clear();
        sqlEdit->setFocus();
    }

signals:
    void statusMessage(const QString &message);
};

#endif
