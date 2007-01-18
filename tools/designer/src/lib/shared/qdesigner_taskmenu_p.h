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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_TASKMENU_H
#define QDESIGNER_TASKMENU_H

#include "shared_global_p.h"
#include <QtDesigner/taskmenu.h>

#include <QtDesigner/default_extensionfactory.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QList>

class QDesignerFormWindowInterface;

class QWidget;
class QSignalMapper;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT QDesignerTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    QDesignerTaskMenu(QWidget *widget, QObject *parent);
    virtual ~QDesignerTaskMenu();

    QWidget *widget() const;

    virtual QList<QAction*> taskActions() const;

protected:
    QDesignerFormWindowInterface *formWindow() const;
    void changeRichTextProperty(const QString &propertyName);
    
    QAction *createAction(const QString &text, QObject *receiver, const char *receiverSlot);
    QAction *createSeparator();

private slots:
    void changeObjectName();
    void promoteToCustomWidget(const QString &customClassName);
    
    void demoteFromCustomWidget();
    void changeToolTip();
    void changeWhatsThis();
    void changeStyleSheet();
    void createMenuBar();
    void addToolBar();
    void createStatusBar();
    void removeStatusBar();
    void editPromotedWidgets();

private:
    void promoteTo(QDesignerFormWindowInterface *fw, const QString &customClassName);
    void createPromotionActions(QDesignerFormWindowInterface *formWindow) const;
    
    typedef QList<QPointer<QWidget> > PromotionSelectionList;
    PromotionSelectionList promotionSelectionList(QDesignerFormWindowInterface *formWindow) const;
    
    QPointer<QWidget> m_widget;
    QAction *m_separator;
    QAction *m_separator2;
    QAction *m_separator3;
    QAction *m_changeObjectNameAction;
    QAction *m_changeToolTip;
    QAction *m_changeWhatsThis;
    QAction *m_changeStyleSheet;

    QAction *m_addMenuBar;
    QAction *m_addToolBar;
    QAction *m_addStatusBar;
    QAction *m_removeStatusBar;
    QAction *m_promotionEditAction;

    mutable QSignalMapper *m_promotionMapper;
    mutable QList<QAction*> m_promotionActions;
    
};

class QDESIGNER_SHARED_EXPORT QDesignerTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    QDesignerTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

} // namespace qdesigner_internal

#endif // QDESIGNER_TASKMENU_H
