/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ECHODIALOG_H
#define ECHODIALOG_H

#include <QWidget>

#include "echointerface.h"

QT_DECLARE_CLASS(QString)
QT_DECLARE_CLASS(QLineEdit)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QPushButton)
QT_DECLARE_CLASS(QGridLayout)

class EchoWindow : public QWidget
{
    Q_OBJECT

public:
    EchoWindow();

private slots:
    void sendEcho();

private:
    void createGUI();
    bool loadPlugin();

    EchoInterface *echoInterface;
    QLineEdit *lineEdit;
    QLabel *label;
    QPushButton *button;
    QGridLayout *layout;
};

#endif
