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

/*!
    delegate.cpp

    A delegate that allows the user to change integer values from the model
    using a spin box widget.
*/

#include <QAbstractItemModel>
#include <QSpinBox>
#include <QStyleOptionViewItem>
#include <QWidget>

#include "delegate.h"

/*!
    Constructs a delegate based on the standard item delegate widget, and
    initializes the pointer to its editor widget.
*/

SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    spinBox = 0;
}

/*!
    Returns the same editor type (a widget) for every model and index
    specified.
*/

QItemDelegate::EditorType SpinBoxDelegate::editorType(const QAbstractItemModel * /* model */,
    const QModelIndex & /* index */) const
{
    return QItemDelegate::Widget;
}

/*!
    Returns an editor widget (a spin box) that restricts values from the
    model to integers in the range [0, 100]. We call the standard interface
    functions for this class to ensure that the editor is updated in a
    consistent way.

    If editing is never allowed by the model, we return 0 to indicate that
    no editor widget was created.
*/

QWidget *SpinBoxDelegate::editor(BeginEditAction action, QWidget *parent,
    const QStyleOptionViewItem &option, const QAbstractItemModel *model,
    const QModelIndex &index)
{
    if (action != QItemDelegate::NeverEdit) {
        spinBox = new QSpinBox(parent);
        spinBox->setMinimum(0);
        spinBox->setMaximum(100);
        setEditorData(spinBox, model, index);
        updateEditorGeometry(spinBox, option, model, index);

        return spinBox;
    }
    return 0;
}

/*!
    Releases the editor. This involves writing the contents of the editor
    widget to the model before destroying it.
*/

void SpinBoxDelegate::releaseEditor(EndEditAction action, QWidget *editor,
    QAbstractItemModel *model, const QModelIndex &index)
{
    qDebug("releaseEditor: %d", action);
    qDebug("(Accepted: %d", QItemDelegate::Accepted);
    qDebug(" Cancelled: %d)", QItemDelegate::Cancelled);
    qDebug("data: %d", spinBox->value());

    if (action == QItemDelegate::Accepted)
        setModelData(editor, model, index);

    editor->close();
    delete editor;
}

/*!
    Reads data from the model, and writes it to the editor widget, assuming
    that it is a spin box.
*/

void SpinBoxDelegate::setEditorData(QWidget *editor,
    const QAbstractItemModel *model, const QModelIndex &index) const
{
    int value = model->data(index, QAbstractItemModel::DisplayRole).toInt();

    ((QSpinBox*)editor)->setValue(value);
}

/*!
    Reads the contents of the spin box, and writes it to the model.
*/

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                  const QModelIndex &index) const
{
    if (spinBox == editor)
        model->setData(index, QAbstractItemModel::EditRole, spinBox->value());
}

/*!
    Updates the editor widget's geometry using the information supplied in
    the style option. This is the minimum that the delegate must do in this
    case.
*/

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QAbstractItemModel* /* model */,
    const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}
