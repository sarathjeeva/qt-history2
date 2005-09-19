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

#include "default_actionprovider.h"
#include "invisible_widget_p.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

QDesignerActionProvider::QDesignerActionProvider(QWidget *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{
    Q_ASSERT(m_widget != 0);

    m_indicator = new InvisibleWidget(m_widget);
    QPalette p;
    p.setColor(m_indicator->backgroundRole(), Qt::red);
    m_indicator->setPalette(p);
    m_indicator->hide();
}

QDesignerActionProvider::~QDesignerActionProvider()
{
}

QRect QDesignerActionProvider::actionGeometry(QAction *action) const
{
    if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(m_widget))
        return menuBar->actionGeometry(action);
    else if (QMenu *menu = qobject_cast<QMenu*>(m_widget))
        return menu->actionGeometry(action);
    else if (QToolBar *toolBar = qobject_cast<QToolBar*>(m_widget))
        return toolBar->actionGeometry(action);

    Q_ASSERT(0);
    return QRect();
}

QAction *QDesignerActionProvider::actionAt(const QPoint &pos) const
{
    if (QMenuBar *menuBar = qobject_cast<QMenuBar*>(m_widget))
        return menuBar->actionAt(pos);
    else if (QMenu *menu = qobject_cast<QMenu*>(m_widget))
        return menu->actionAt(pos);
    else if (QToolBar *toolBar = qobject_cast<QToolBar*>(m_widget))
        return toolBar->actionAt(pos);

    Q_ASSERT(0);
    return 0;
}

void QDesignerActionProvider::adjustIndicator(const QPoint &pos)
{
    if (pos == QPoint(-1, -1)) {
        m_indicator->hide();
        return;
    }

    if (QAction *action = actionAt(pos)) {
        QRect g = actionGeometry(action);

        // ### g.moveTopLeft(g.topRight());
        g.setWidth(2);

        // ### m_indicator->setGeometry(10, 10, 30, 30);

        m_indicator->setGeometry(g);
        m_indicator->show();
        m_indicator->raise();
    } else {
        m_indicator->hide();
    }
}

// ---- QDesignerActionProviderFactory ----
QDesignerActionProviderFactory::QDesignerActionProviderFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerActionProviderFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerActionProviderExtension))
        return 0;

    if (qobject_cast<QMenu*>(object)
            || qobject_cast<QMenuBar*>(object)
            || qobject_cast<QToolBar*>(object))
        return new QDesignerActionProvider(qobject_cast<QWidget*>(object), parent);

    return 0;
}
