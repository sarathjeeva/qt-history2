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

#ifndef BUDDYEDITOR_PLUGIN_H
#define BUDDYEDITOR_PLUGIN_H

#include "buddyeditor_global.h"
#include <abstractformeditorplugin.h>

#include <QtCore/QPointer>

class QT_BUDDYEDITOR_EXPORT BuddyEditorPlugin: public QObject, public AbstractFormEditorPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractFormEditorPlugin)
public:
    BuddyEditorPlugin();
    virtual ~BuddyEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(AbstractFormEditor *core);

    virtual AbstractFormEditor *core() const;

private:
    QPointer<AbstractFormEditor> m_core;
    bool m_initialized;
};

#endif // BUDDYEDITOR_PLUGIN_H
