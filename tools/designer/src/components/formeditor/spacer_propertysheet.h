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

#ifndef SPACER_PROPERTYSHEET_H
#define SPACER_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>
#include <spacer_widget_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class SpacerPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit SpacerPropertySheet(Spacer *object, QObject *parent = 0);
    virtual ~SpacerPropertySheet();

    virtual void setProperty(int index, const QVariant &value);
    virtual bool isVisible(int index) const;

    virtual bool dynamicPropertiesAllowed() const;
};

typedef QDesignerPropertySheetFactory<Spacer, SpacerPropertySheet> SpacerPropertySheetFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SPACER_PROPERTYSHEET_H
