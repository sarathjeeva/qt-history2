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

#include "actioneditor_p.h"
#include "actionrepository_p.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QListWidget>
#include <QtGui/QSplitter>
#include <QtGui/QAction>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

class ActionGroupDelegate: public QItemDelegate
{
public:
    ActionGroupDelegate(QObject *parent)
        : QItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());

        QItemDelegate::paint(painter, option, index);
    }

    virtual void drawFocus(QPainter */*painter*/, const QStyleOptionViewItem &/*option*/, const QRect &/*rect*/) const {}
};

ActionEditor::ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent, Qt::WindowFlags flags)
    : QDesignerActionEditorInterface(parent, flags),
      m_core(core)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);

    setWindowTitle(tr("Actions"));
    splitter = new QSplitter(Qt::Horizontal, this);
    l->addWidget(splitter);

#if 0 // ### implement me
    m_actionGroups = new QListWidget(splitter);
    m_actionGroups->setItemDelegate(new ActionGroupDelegate(m_actionGroups));
    m_actionGroups->setMovement(QListWidget::Static);
    m_actionGroups->setResizeMode(QListWidget::Fixed);
    m_actionGroups->setIconSize(QSize(48, 48));
    m_actionGroups->setFlow(QListWidget::TopToBottom);
    m_actionGroups->setViewMode(QListWidget::IconMode);
    m_actionGroups->setWrapping(false);
#endif

    m_actionRepository = new ActionRepository(splitter);

    connect(m_actionRepository, SIGNAL(itemClicked(QListWidgetItem*)),
        this, SLOT(slotItemChanged(QListWidgetItem*)));
}

ActionEditor::~ActionEditor()
{
}

void ActionEditor::setFormWindow(QDesignerFormWindowInterface *formWindow)
{
    m_actionRepository->clear();

    if (!formWindow || !formWindow->mainContainer())
        return;

    QList<QAction*> actionList = qFindChildren<QAction*>(formWindow->mainContainer());
    foreach (QAction *action, actionList) {
        if (!core()->metaDataBase()->item(action)
            || action->isSeparator()
            || action->menu())
            continue;

        QListWidgetItem *item = new QListWidgetItem(m_actionRepository);
        item->setText(action->objectName());
        item->setIcon(action->icon());

        QVariant itemData;
        qVariantSetValue(itemData, action);
        item->setData(ActionRepository::ActionRole, itemData);

        QVariant actionData;
        qVariantSetValue(actionData, item);
        action->setData(actionData);

        connect(action, SIGNAL(changed()), this, SLOT(slotActionChanged()));
    }
}

void ActionEditor::slotItemChanged(QListWidgetItem *item)
{
    if (!item)
        return;

    if (QDesignerPropertyEditorInterface *propertyEditor = core()->propertyEditor()) {
        if (QAction *action = qvariant_cast<QAction*>(item->data(ActionRepository::ActionRole))) {
            propertyEditor->setObject(action);
        }
    }
}

void ActionEditor::slotActionChanged()
{
    QAction *action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action != 0);

    QListWidgetItem *item = qvariant_cast<QListWidgetItem*>(action->data());
    Q_ASSERT(item != 0);

    item->setText(action->text());
    item->setIcon(action->icon());
}

QDesignerFormEditorInterface *ActionEditor::core() const
{
    return m_core;
}

void ActionEditor::filter(const QString &f)
{
    m_actionRepository->filter(f);
}
