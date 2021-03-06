/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef STATISTICS_H
#define STATISTICS_H

#include "ui_statistics.h"
#include <QVariant>

QT_BEGIN_NAMESPACE

class Statistics : public QDialog, public Ui::Statistics
{
    Q_OBJECT

public:
    Statistics(QWidget *parent = 0, Qt::WindowFlags fl = 0);
    ~Statistics() {}

public slots:
    virtual void updateStats(int w1, int c1, int cs1, int w2, int c2, int cs2);

protected slots:
    virtual void languageChange();
};

QT_END_NAMESPACE

#endif // STATISTICS_H
