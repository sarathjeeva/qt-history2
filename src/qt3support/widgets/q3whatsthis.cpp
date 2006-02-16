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

#include "q3whatsthis.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qevent.h"

/*! \class Q3WhatsThis
    \compat
*/

/*!
    Constructs a new "What's This?" object for \a widget.
*/
Q3WhatsThis::Q3WhatsThis(QWidget *widget)
    : QObject(widget)
{
    if (widget)
        widget->installEventFilter(this);
}

/*!
    Destroys the "What's This?" object.
*/
Q3WhatsThis::~Q3WhatsThis()
{
}

/*!
    \internal

    Handles "What's This?" events.
*/
bool Q3WhatsThis::eventFilter(QObject *o, QEvent *e)
{
    if (o != parent() || !o->isWidgetType())
        return false;

    if (e->type() == QEvent::WhatsThis) {
        QString s = text(static_cast<QHelpEvent*>(e)->pos());
        if (!s.isEmpty())
            QWhatsThis::showText(static_cast<QHelpEvent*>(e)->globalPos(), s, static_cast<QWidget*>(o));
    } else if (e->type() == QEvent::QueryWhatsThis) {
        QString s = text(static_cast<QHelpEvent*>(e)->pos());
        if (s.isEmpty())
            return false;
    } else if (e->type() == QEvent::WhatsThisClicked) {
        QString href = static_cast<QWhatsThisClickedEvent*>(e)->href();
        if (clicked(href))
            QWhatsThis::hideText();
    } else {
        return false;
    }
    return true;
}

/*!
    This virtual function returns the text for position \a pos in the
    widget that this "What's This?" object documents. If there is no
    "What's This?" text for the position, an empty string is returned.

    The default implementation returns an empty string.
*/
QString Q3WhatsThis::text(const QPoint & /* pos */)
{
    if (parent() && parent()->isWidgetType())
        return static_cast<QWidget*>(parent())->whatsThis();
    return QString();
}

/*!
    This virtual function is called when the user clicks inside the
    "What's this?" window. \a href is the link the user clicked on, or
    an empty string if there was no link.

    If the function returns true (the default), the "What's this?"
    window is closed, otherwise it remains visible.

    The default implementation ignores \a href and returns true.
*/
bool Q3WhatsThis::clicked(const QString & /* href */)
{
    return true;
}

/*!
    \fn void Q3WhatsThis::enterWhatsThisMode()

    Enters "What's This?" mode and returns immediately.

    Qt will install a special cursor and take over mouse input until
    the user clicks somewhere. It then shows any help available and
    ends "What's This?" mode. Finally, Qt removes the special cursor
    and help window and then restores ordinary event processing, at
    which point the left mouse button is no longer pressed.

    The user can also use the Esc key to leave "What's This?" mode.

    \sa inWhatsThisMode(), leaveWhatsThisMode()
*/

/*!
    \fn bool Q3WhatsThis::inWhatsThisMode()

    Returns true if the application is in "What's This?" mode;
    otherwise returns false.

    \sa enterWhatsThisMode(), leaveWhatsThisMode()
*/

/*!
    \fn void Q3WhatsThis::add(QWidget *widget, const QString &text)

    Adds \a text as "What's This?" help for \a widget. If the text is
    rich text formatted (i.e. it contains markup) it will be rendered
    with the default stylesheet QStyleSheet::defaultSheet().

    The text is destroyed if the widget is later destroyed, so it need
    not be explicitly removed.

    \sa remove()
*/

/*!
    \fn void Q3WhatsThis::remove(QWidget *widget)

    Removes the "What's This?" help associated with the \a widget.
    This happens automatically if the widget is destroyed.

    \sa add()
*/

/*!
    \fn void Q3WhatsThis::leaveWhatsThisMode(const QString& text = QString(), const QPoint& pos = QCursor::pos(), QWidget* widget = 0)

    This function is used internally by widgets that support
    QWidget::customWhatsThis(); applications do not usually call it.
    An example of such a widget is Q3PopupMenu: menus still work
    normally in "What's This?" mode but also provide help texts for
    individual menu items.

    If \a text is not empty, a "What's This?" help window is
    displayed at the global screen position \a pos. If widget \a widget is
    not 0 and has its own dedicated QWhatsThis object, this object
    will receive clicked() messages when the user clicks on hyperlinks
    inside the help text.

    \sa inWhatsThisMode(), enterWhatsThisMode(), clicked()
*/

/*!
    \fn void Q3WhatsThis::display(const QString &text, const QPoint &pos, QWidget *widget)

    Display \a text in a help window at the global screen position \a
    pos.

    If widget \a widget is not 0 and has its own dedicated QWhatsThis
    object, this object will receive clicked() messages when the user
    clicks on hyperlinks inside the help text.

    \sa clicked()
*/

/*!
    Creates a QToolButton preconfigured to enter "What's This?" mode
    when clicked. You will often use this with a tool bar as \a
    parent:

    \code
	(void)QWhatsThis::whatsThisButton( my_help_tool_bar );
    \endcode
*/
QToolButton *Q3WhatsThis::whatsThisButton(QWidget * parent)
{
    return QWhatsThis::whatsThisButton(parent);
}
