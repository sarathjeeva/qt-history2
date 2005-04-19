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

#include "q3cleanuphandler.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qcombobox.h"

#include "q3editorfactory.h"
#include "qdatetimeedit.h"

#ifndef QT_NO_SQL_EDIT_WIDGETS

/*!
    \class Q3EditorFactory qeditorfactory.h
    \brief The Q3EditorFactory class is used to create editor widgets
    for QVariant data types.

    \compat

    Each editor factory provides the createEditor() function which
    given a QVariant will create and return a QWidget that can edit
    that QVariant. For example if you have a QVariant::String type, a
    QLineEdit would be the default editor returned, whereas a
    QVariant::Int's default editor would be a QSpinBox.

    If you want to create different editors for fields with the same
    data type, subclass Q3EditorFactory and reimplement the
    createEditor() function.
*/

/*!
    Constructs an editor factory with parent \a parent.
*/

Q3EditorFactory::Q3EditorFactory (QObject * parent)
    : QObject(parent)
{

}

/*!
    Destroys the object and frees any allocated resources.
*/

Q3EditorFactory::~Q3EditorFactory()
{

}

static Q3EditorFactory * defaultfactory = 0;
static Q3CleanupHandler< Q3EditorFactory > q_cleanup_editor_factory;

/*!
    Returns an instance of a default editor factory.
*/

Q3EditorFactory * Q3EditorFactory::defaultFactory()
{
    if(defaultfactory == 0){
        defaultfactory = new Q3EditorFactory();
        q_cleanup_editor_factory.add(&defaultfactory);
    }

    return defaultfactory;
}

/*!
    Replaces the default editor factory with \a factory.
    \e{Q3EditorFactory takes ownership of factory, and destroys it
    when it is no longer needed.}
*/

void Q3EditorFactory::installDefaultFactory(Q3EditorFactory * factory)
{
    if(factory == 0 || factory == defaultfactory) return;

    if(defaultfactory != 0){
        q_cleanup_editor_factory.remove(&defaultfactory);
        delete defaultfactory;
    }
    defaultfactory = factory;
    q_cleanup_editor_factory.add(&defaultfactory);
}

/*!
    Creates and returns the appropriate editor for the QVariant \a v.
    If the QVariant is invalid, 0 is returned. The \a parent is passed
    to the appropriate editor's constructor.
*/

QWidget * Q3EditorFactory::createEditor(QWidget * parent, const QVariant & v)
{
    QWidget * w = 0;
    switch(v.type()){
        case QVariant::Invalid:
            w = 0;
            break;
        case QVariant::Bool:
            w = new QComboBox(parent, "qt_editor_bool");
            ((QComboBox *) w)->insertItem("False");
            ((QComboBox *) w)->insertItem("True");
            break;
        case QVariant::UInt:
            w = new QSpinBox(0, 999999, 1, parent, "qt_editor_spinbox");
            break;
        case QVariant::Int:
            w = new QSpinBox(-999999, 999999, 1, parent, "qt_editor_int");
            break;
        case QVariant::String:
        case QVariant::Double:
            w = new QLineEdit(parent, "qt_editor_double");
            ((QLineEdit*)w)->setFrame(false);
            break;
        case QVariant::Date: {
            QDateTimeEdit *edit = new QDateTimeEdit(parent);
            edit->setDisplayFormat("yyyy/MM/dd");
            edit->setObjectName("qt_editor_date");
            w = edit; }
            break;
        case QVariant::Time: {
            QDateTimeEdit *edit = new QDateTimeEdit(parent);
            edit->setDisplayFormat("hh:mm");
            edit->setObjectName("qt_editor_time");
            w = edit; }
            break;
        case QVariant::DateTime:
            w = new QDateTimeEdit(parent);
            w->setObjectName("qt_editor_datetime");
            break;
#ifndef QT_NO_LABEL
        case QVariant::Pixmap:
            w = new QLabel(parent, "qt_editor_pixmap");
            break;
#endif
        case QVariant::Palette:
        case QVariant::Color:
        case QVariant::Font:
        case QVariant::Brush:
        case QVariant::Bitmap:
        case QVariant::Cursor:
        case QVariant::Map:
        case QVariant::StringList:
        case QVariant::Rect:
        case QVariant::Size:
        case QVariant::IconSet:
        case QVariant::Point:
        case QVariant::PointArray:
        case QVariant::Region:
        case QVariant::SizePolicy:
        case QVariant::ByteArray:
        default:
            w = new QWidget(parent, "qt_editor_default");
            break;
    }
    return w;
}
#endif // QT_NO_SQL
