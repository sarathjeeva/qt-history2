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

#include "qgroupbox.h"
#ifndef QT_NO_GROUPBOX
#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qlayout.h"
#include "qradiobutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qstylepainter.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include <private/qwidget_p.h>

class QGroupBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGroupBox)
public:

    void skip();
    void init();
    void calculateFrame();
    QString title;
    int align;
#ifndef QT_NO_SHORTCUT
    int shortcutId;
#endif
    
    void fixFocus();
    void setChildrenEnabled(bool b);
    bool flat;
    bool checkable;
    bool checked;
    bool hover;
    QStyle::SubControl pressedControl;

    QStyleOptionGroupBox getStyleOption() const;
};

QStyleOptionGroupBox QGroupBoxPrivate::getStyleOption() const
{
    Q_Q(const QGroupBox);
    QStyleOptionGroupBox option;
    option.init(q);
    option.text = title;
    option.lineWidth = 1;
    option.midLineWidth = 0;
    option.textAlignment = Qt::Alignment(align);
    option.activeSubControls |= pressedControl;
    option.subControls = QStyle::SC_None;

    if (hover)
        option.state |= QStyle::State_MouseOver;
    else
        option.state &= ~QStyle::State_MouseOver;

    if (flat)
        option.features |= QStyleOptionFrameV2::Flat;

    if (checkable) {
        option.subControls |= QStyle::SC_GroupBoxCheckBox;
        option.state |= (checked ? QStyle::State_On : QStyle::State_Off);
        if (pressedControl == QStyle::SC_GroupBoxCheckBox || pressedControl == QStyle::SC_GroupBoxLabel)
            option.state |= QStyle::State_Sunken;
    }

    if (!q->testAttribute(Qt::WA_SetPalette))
        option.textColor = QColor(q->style()->styleHint(QStyle::SH_GroupBox_TextLabelColor, &option, q));

    if (!title.isEmpty())
        option.subControls |= QStyle::SC_GroupBoxLabel;
    
    return option;
}

/*!
    \class QGroupBox qgroupbox.h
    \brief The QGroupBox widget provides a group box frame with a title.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A group box provides a frame, a title and a keyboard shortcut, and
    displays various other widgets inside itself. The title is on top,
    the keyboard shortcut moves keyboard focus to one of the group
    box's child widgets.

    QGroupBox also lets you set the \l title (normally set in the
    constructor) and the title's alignment(). If setCheckable(true) is
    called then the group box is isCheckable(), and it can be
    setChecked(). Checkable group boxes child widgets are enabled or
    disabled depending on whether or not the group box is isChecked().

    To minimize space consumption, you can remove the right, left and
    bottom edges of the frame with setFlat().

    \inlineimage plastique-groupbox.png Screenshot in Plastique style
    \inlineimage windows-groupbox.png Screenshot in Windows style

    \sa QButtonGroup
*/



/*!
    Constructs a group box widget with no title and parent \a parent.
*/

QGroupBox::QGroupBox(QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
}

/*!
    Constructs a group box with the title \a title and parent \a
    parent.
*/

QGroupBox::QGroupBox(const QString &title, QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
    setTitle(title);
}


/*!
    Destroys the group box.
*/
QGroupBox::~QGroupBox()
{
}

void QGroupBoxPrivate::init()
{
    align = Qt::AlignLeft;
#ifndef QT_NO_SHORTCUT
    shortcutId = 0;
#endif
    flat = false;
    checkable = false;
    checked = true;
    hover = false;
    pressedControl = QStyle::SC_None;
    calculateFrame();
}


void QGroupBox::setTitle(const QString &title)
{
    Q_D(QGroupBox);
    if (d->title == title)                                // no change
        return;
    d->title = title;
#ifndef QT_NO_SHORTCUT
    releaseShortcut(d->shortcutId);
    d->shortcutId = grabShortcut(QKeySequence::mnemonic(title));
#endif
    d->calculateFrame();

    update();
    updateGeometry();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
#endif
}

/*!
    \property QGroupBox::title
    \brief the group box title text.

    The group box title text will have a focus-change keyboard
    shortcut if the title contains \&, followed by a letter.

    \code
        g->setTitle("&User information");
    \endcode
    This produces "\underline{U}ser information"; \key Alt+U moves the keyboard
    focus to the group box.

    There is no default title text.
*/

QString QGroupBox::title() const
{
    Q_D(const QGroupBox);
    return d->title;
}

/*!
    \property QGroupBox::alignment
    \brief the alignment of the group box title.

    The title is always placed on the upper frame line. The horizontal
    alignment can be specified by the alignment parameter.

    The alignment is one of the following flags:
    \list
    \i Qt::AlignLeft aligns the title text to the left.
    \i Qt::AlignRight aligns the title text to the right.
    \i Qt::AlignHCenter aligns the title text centered.
    \endlist

    The default alignment is Qt::AlignLeft.

    \sa Qt::Alignment
*/
Qt::Alignment QGroupBox::alignment() const
{
    Q_D(const QGroupBox);
    return QFlag(d->align);
}

void QGroupBox::setAlignment(int alignment)
{
    Q_D(QGroupBox);
    d->align = alignment;
    updateGeometry();
    update();
}

/*! \reimp
*/
void QGroupBox::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateGeometry();
}

/*! \reimp
*/

void QGroupBox::paintEvent(QPaintEvent *)
{
    Q_D(QGroupBox);
    QStylePainter paint(this);
    paint.drawComplexControl(QStyle::CC_GroupBox, d->getStyleOption());
}

/*! \reimp  */
bool QGroupBox::event(QEvent *e)
{
#ifndef QT_NO_SHORTCUT
    Q_D(QGroupBox);
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            d->fixFocus();
            return true;
        }
    }
#endif
    QStyleOptionGroupBox box = d->getStyleOption();
    switch (e->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
        QStyle::SubControl control = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                                    static_cast<QHoverEvent *>(e)->pos(),
                                                                    this);
        d->hover = (control == QStyle::SC_GroupBoxLabel || control == QStyle::SC_GroupBoxCheckBox);
        update();
        break;
    }
    case QEvent::HoverLeave:
        d->hover = false;
        update();
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

/*!\reimp */
void QGroupBox::childEvent(QChildEvent *c)
{
    Q_D(QGroupBox);
    if (c->type() != QEvent::ChildAdded || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
    if (d->checkable) {
        if (d->checked) {
            if (!w->testAttribute(Qt::WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(Qt::WA_ForceDisabled, false);
            }
        }
    }
}


/*!
    \internal

    This private slot finds a widget in this group box that can accept
    focus, and gives the focus to that widget.
*/

void QGroupBoxPrivate::fixFocus()
{
    Q_Q(QGroupBox);
    QWidget *fw = q->focusWidget();
    if (!fw) {
        QWidget * best = 0;
        QWidget * candidate = 0;
        QWidget * w = q;
        while ((w = w->nextInFocusChain()) != q) {
            if (q->isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus && w->isVisibleTo(q)) {
                if (!best && qobject_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
                    // we prefer a checked radio button or a widget that
                    // already has focus, if there is one
                    best = w;
                else
                    if (!candidate)
                        // but we'll accept anything that takes focus
                        candidate = w;
            }
        }
        if (best)
            fw = best;
        else
            if (candidate)
                fw = candidate;
    }
    if (fw)
        fw->setFocus();
}


/*
    Sets the right frame rect depending on the title.
*/
void QGroupBoxPrivate::calculateFrame()
{
    Q_Q(QGroupBox);
    QStyleOptionGroupBox box = getStyleOption();
    QRect contentsRect = q->style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxContents, q);
    QRect frameRect = q->style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxFrame, q);
    q->setContentsMargins(contentsRect.left() - frameRect.left(), contentsRect.top() - frameRect.top(),
                          frameRect.right() - contentsRect.right(), frameRect.bottom() - contentsRect.bottom());
}



/*! \reimp
 */
void QGroupBox::focusInEvent(QFocusEvent *)
{ // note no call to super
    Q_D(QGroupBox);
    d->fixFocus();
}


/*!
  \reimp
*/
QSize QGroupBox::minimumSizeHint() const
{
    Q_D(const QGroupBox);
    QStyleOptionGroupBox option = d->getStyleOption();

    int baseWidth = fontMetrics().width(d->title + QLatin1Char(' '));
    int baseHeight = fontMetrics().height();
    if (d->checkable) {
        baseWidth += style()->pixelMetric(QStyle::PM_IndicatorWidth);
        baseWidth += style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
        baseHeight = qMax(baseHeight, style()->pixelMetric(QStyle::PM_IndicatorHeight));
    }

    QSize size = QWidget::minimumSizeHint().expandedTo(QSize(baseWidth, baseHeight));
    return style()->sizeFromContents(QStyle::CT_GroupBox, &option, size, this);
}

/*!
    \property QGroupBox::flat
    \brief whether the group box is painted flat or has a frame

    By default a group box has a surrounding frame, with the title
    being placed on the upper frame line. In flat mode the right, left
    and bottom frame lines are omitted, and only the thin line at the
    top is drawn.

    \sa title
*/
bool QGroupBox::isFlat() const
{
    Q_D(const QGroupBox);
    return d->flat;
}

void QGroupBox::setFlat(bool b)
{
    Q_D(QGroupBox);
    if (d->flat == b)
        return;
    d->flat = b;
    updateGeometry();
    update();
}


/*!
    \property QGroupBox::checkable
    \brief Whether the group box has a checkbox in its title.

    If this property is true, the group box has a checkbox. If the
    checkbox is checked (which is the default), the group box's
    children are enabled.

    setCheckable() controls whether or not the group box has a
    checkbox, and isCheckable() controls whether the checkbox is
    checked or not.
*/
void QGroupBox::setCheckable(bool checkable)
{
    Q_D(QGroupBox);

    bool wasCheckable = d->checkable;
    d->checkable = checkable;
    
    if (checkable) {
        setChecked(true);
        if (!wasCheckable) {
            d->setChildrenEnabled(true);
            updateGeometry();
        }
    } else {
        if (wasCheckable) {
            d->setChildrenEnabled(true);
            updateGeometry();
        }
        d->setChildrenEnabled(true);
    }

    if (wasCheckable != checkable)
        update();
}

bool QGroupBox::isCheckable() const
{
    Q_D(const QGroupBox);
    return d->checkable;
}


bool QGroupBox::isChecked() const
{
    Q_D(const QGroupBox);
    return d->checkable && d->checked;
}


/*!
    \fn void QGroupBox::toggled(bool on)

    If the group box has a check box (see \l isCheckable()) this signal
    is emitted when the check box is toggled. \a on is true if the check
    box is checked; otherwise it is false.
*/

/*!
    \property QGroupBox::checked
    \brief Whether the group box's checkbox is checked.

    If the group box has a check box (see \l isCheckable()), and the
    check box is checked, the group box's children
    are enabled. If the checkbox is unchecked the children are
    disabled.
*/
void QGroupBox::setChecked(bool b)
{
    Q_D(QGroupBox);
    if (d->checkable) {
        if (d->checked != b)
            update();
        bool wasToggled = (b != d->checked);
        d->checked = b;
        if (wasToggled)
            d->setChildrenEnabled(b);
    }
}

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void QGroupBoxPrivate::setChildrenEnabled(bool b)
{
    Q_Q(QGroupBox);
    QObjectList childs = q->children();
    if (childs.isEmpty())
        return;
    for (int i = 0; i < childs.size(); ++i) {
        QObject *o = childs.at(i);
        if (o->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(Qt::WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(Qt::WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \reimp */
void QGroupBox::changeEvent(QEvent *ev)
{
    Q_D(QGroupBox);
    if(ev->type() == QEvent::EnabledChange) {
        if (d->checkable && isEnabled()) {
            // we are being enabled - disable children
            if (!d->checked)
                d->setChildrenEnabled(false);
        }
    } else if(ev->type() == QEvent::FontChange || ev->type() == QEvent::StyleChange) {
        updateGeometry();
        d->calculateFrame();
    }
    QWidget::changeEvent(ev);
}

/*! \reimp */
void QGroupBox::mousePressEvent(QMouseEvent *event)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    d->pressedControl = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                       event->pos(), this);
    update();
}

/*! \reimp */
void QGroupBox::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    QStyle::SubControl pressed = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                                event->pos(), this);
    if (d->pressedControl != pressed) {
        d->pressedControl = pressed;
        update();
    }
}

/*! \reimp */
void QGroupBox::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    bool toggle = (d->pressedControl == QStyle::SC_GroupBoxLabel
                   || d->pressedControl == QStyle::SC_GroupBoxCheckBox);
    d->pressedControl = QStyle::SC_None;
    if (toggle)
        setChecked(!d->checked);
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(name);
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(const QString &title, QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(name);
    d->init();
    setTitle(title);
}
#endif // QT3_SUPPORT

#include "moc_qgroupbox.cpp"

#endif //QT_NO_GROUPBOX
