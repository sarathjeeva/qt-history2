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

#include "qdesigner.h"
#include "qdesigner_widgetbox.h"
#include "qdesigner_workbench.h"
#include "qdesigner_settings.h"

#include <QtDesigner/QDesignerWidgetBoxInterface>
#include <QtDesigner/QDesignerComponents>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

QDesignerWidgetBox::QDesignerWidgetBox(QDesignerWorkbench *workbench)
    : QDesignerToolWindow(workbench)
{
    setObjectName(QLatin1String("WidgetBox"));
    QDesignerWidgetBoxInterface *widget = QDesignerComponents::createWidgetBox(workbench->core(), this);

    workbench->core()->setWidgetBox(widget);

    setCentralWidget(widget);

    setWindowTitle(tr("Widget Box"));
    action()->setObjectName(QLatin1String("__qt_widget_box_tool_action"));
}

QDesignerWidgetBox::~QDesignerWidgetBox()
{
}

QRect QDesignerWidgetBox::geometryHint() const
{
    const QRect g = workbench()->availableGeometry();

    return QRect(workbench()->marginHint(), workbench()->marginHint(),
                 g.width() * 1/4, g.height() * 5/6);
}

Qt::DockWidgetArea QDesignerWidgetBox::dockWidgetAreaHint() const
{
    return Qt::LeftDockWidgetArea;
}

QT_END_NAMESPACE
