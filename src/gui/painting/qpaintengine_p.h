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

#ifndef QPAINTENGINE_P_H
#define QPAINTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qpainter.h"
#include "QtGui/qpaintengine.h"
#include "QtGui/qregion.h"
#include "private/qobject_p.h"

QT_BEGIN_NAMESPACE

class QPaintDevice;

class QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QPaintEngine)
public:
    QPaintEnginePrivate() : pdev(0), q_ptr(0) { }
    virtual ~QPaintEnginePrivate() { }
    QPaintDevice *pdev;
    QPaintEngine *q_ptr;
    QRegion systemClip;

private:
    QRect systemRect;
};

QT_END_NAMESPACE

#endif // QPAINTENGINE_P_H
