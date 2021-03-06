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

#ifndef QDESIGNER_PROPERTYCOMMENTCOMMAND_H
#define QDESIGNER_PROPERTYCOMMENTCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT SetPropertyCommentCommand: public QDesignerFormWindowCommand
{
    SetPropertyCommentCommand(const SetPropertyCommentCommand &);
    SetPropertyCommentCommand& operator=(const SetPropertyCommentCommand &);

public:
    explicit SetPropertyCommentCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QObject *object, const QString &propertyName, const QString &newCommentValue);

    typedef QList<QObject *> ObjectList;
    bool init(const ObjectList &list, const QString &propertyName, const QString &newCommentValue);

    int id() const;
    bool mergeWith(const QUndoCommand *other);

    virtual void redo();
    virtual void undo();

private:
    bool add(QObject *object);
    void setDescription();

    struct Entry {
        Entry(QObject* object, const QString &oldCommentValue);
        QPointer<QObject> m_object;
        QString m_oldCommentValue;
    };

    typedef QList<Entry> EntryList;
    EntryList m_Entries;

    QVariant::Type m_propertyType;
    QString m_propertyName;
    QString m_newCommentValue;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_PROPERTYCOMMENTCOMMAND_H
