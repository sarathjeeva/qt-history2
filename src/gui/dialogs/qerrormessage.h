/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QERRORMESSAGE_H
#define QERRORMESSAGE_H

#include "qdialog.h"

#ifndef QT_NO_ERRORMESSAGE

class QErrorMessagePrivate;

class Q_GUI_EXPORT QErrorMessage: public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QErrorMessage)
public:
    QErrorMessage(QWidget* parent = 0);
    ~QErrorMessage();

    static QErrorMessage * qtHandler();

public slots:
    void message(const QString &);

protected:
    void done(int);

private:
    Q_DISABLE_COPY(QErrorMessage)
};

#endif // QT_NO_ERRORMESSAGE

#endif
