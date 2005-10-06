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

#include "qtabwidget.h"
#ifndef QT_NO_TABWIDGET
#include "private/qwidget_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qlayout.h"
#include "qstackedwidget.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qstylepainter.h"
#include "qtabbar.h"
#include "qtoolbutton.h"

/*!
    \class QTabWidget qtabwidget.h
    \brief The QTabWidget class provides a stack of tabbed widgets.

    \ingroup organizers
    \ingroup advanced
    \mainclass

    A tab widget provides a tab bar (see QTabBar) and a "page area"
    that is used to display pages related to each tab. By default, the
    tab bar is shown above the page area, but different configurations
    are available (see \l{TabPosition}). Each tab is associated with a
    different widget (called a page). Only the current page is shown in
    the page area; all the other pages are hidden. The user can show a
    different page by clicking on its tab or by pressing its
    Alt+\e{letter} shortcut if it has one.

    The normal way to use QTabWidget is to do the following in the
    constructor:
    \list 1
    \i Create a QTabWidget.
    \i Create a QWidget for each of the pages in the tab dialog, but
       do not specify a parent widget - the tab widget will reparent
       the page widget later.
    \i Insert children into the page widget, set up geometry management
    for it and use addTab() (or insertTab()) to set up a tab with an
    optional keyboard shortcut.
    \i Connect to the signals and slots.
    \endlist

    The position of the tabs is defined by \l tabPosition, their shape
    by \l tabShape.

    The signal currentChanged() is emitted when the user selects a
    page.

    The current page index is available as currentIndex(), the current
    page widget with currentWidget().  You can retrieve a pointer to a
    page widget with a given index using widget(), and can find the
    index position of a widget with indexOf(). Use setCurrentWidget()
    or setCurrentIndex() to show a particular page.

    You can change a tab's text and icon using setTabText() or
    setTabIcon(). A tab can be removed with removeTab().

    Each tab is either enabled or disabled at any given time (see
    setTabEnabled()). If a tab is enabled, the tab text is drawn
    normally and the user can select that tab. If it is disabled, the
    tab is drawn in a different way and the user cannot select that
    tab. Note that even if a tab is disabled, the page can still be
    visible, for example if all of the tabs happen to be disabled.

    Tab widgets can be a very good way to split up a complex dialog.
    An alternative is to use a QStackedWidget for which you provide some
    means of navigating between pages, for example, a QToolBar or a
    QListWidget.

    Most of the functionality in QTabWidget is provided by a QTabBar
    (at the top, providing the tabs) and a QStackedWidget (most of the
    area, organizing the individual pages).

    \inlineimage windows-tabwidget.png Screenshot in Windows style
    \inlineimage macintosh-tabwidget.png Screenshot in macintosh style

    \sa QTabBar QStackedWidget QToolBox
*/

/*!
    \enum QTabWidget::TabPosition

    This enum type defines where QTabWidget draws the tab row:

    \value North  The tabs are drawn above the pages.
    \value South  The tabs are drawn below the pages.
    \value West  The tabs are drawn to the left of the pages.
    \value East  The tabs are drawn to the right of the pages.
    \omitvalue Bottom
    \omitvalue Top
*/

/*!
    \enum QTabWidget::TabShape

    This enum type defines the shape of the tabs:
    \value Rounded  rounded look (normal)
    \value Triangular  triangular look
*/

/* undocumented now
  \obsolete

  \fn void QTabWidget::selected(const QString &tabLabel);

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/


/*!
    \fn void QTabWidget::currentChanged(int index);

    This signal is emitted whenever the current page index changes.
    The parameter is the new current page \a index position.

    \sa currentWidget() currentIndex
*/


class QTabWidgetPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QTabWidget)
public:
    QTabWidgetPrivate();
    ~QTabWidgetPrivate();
    void updateTabBarPosition();
    void showTab(int);
    void removeTab(int);
    void init();
    QStyleOptionTabWidgetFrame getStyleOption() const;

    QTabBar *tabs;
    QStackedWidget *stack;
    QRect panelRect;
    bool dirty;
    QTabWidget::TabPosition pos;
    QTabWidget::TabShape shape;
    int alignment;
    QWidget *leftCornerWidget;
    QWidget *rightCornerWidget;
};

QTabWidgetPrivate::QTabWidgetPrivate()
    : tabs(0), stack(0), dirty(true),
      pos(QTabWidget::North), shape(QTabWidget::Rounded),
      leftCornerWidget(0), rightCornerWidget(0)
{}

QTabWidgetPrivate::~QTabWidgetPrivate()
{}

void QTabWidgetPrivate::init()
{
    Q_Q(QTabWidget);
    stack = new QStackedWidget(q);
    stack->setLineWidth(0);
    QObject::connect(stack, SIGNAL(widgetRemoved(int)), q, SLOT(removeTab(int)));
    QTabBar *tabBar = new QTabBar(q);
    tabBar->setDrawBase(false);
    q->setTabBar(tabBar);

#ifdef Q_OS_TEMP
    pos = QTabWidget::South;
#endif

    q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    q->setFocusPolicy(Qt::TabFocus);
    q->setFocusProxy(tabs);
}

QStyleOptionTabWidgetFrame QTabWidgetPrivate::getStyleOption() const
{
    Q_Q(const QTabWidget);
    QStyleOptionTabWidgetFrame option;
    option.init(q);

    int exth = q->style()->pixelMetric(QStyle::PM_TabBarBaseHeight, 0, q);
    QSize t(0, stack->frameWidth());
    if (tabs->isVisibleTo(const_cast<QTabWidget *>(q)))
        t = tabs->sizeHint();

    if (rightCornerWidget)
        option.rightCornerWidgetSize
            = rightCornerWidget->sizeHint().boundedTo(t - QSize(exth, exth));
    else
        option.rightCornerWidgetSize = QSize(0, 0);

    if (leftCornerWidget)
        option.leftCornerWidgetSize
            = leftCornerWidget->sizeHint().boundedTo(t - QSize(exth, exth));
    else
        option.leftCornerWidgetSize = QSize(0, 0);

    switch (pos) {
    case QTabWidget::North:
        option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedNorth : QTabBar::TriangularNorth;
        break;
    case QTabWidget::South:
        option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedSouth : QTabBar::TriangularSouth;
        break;
    case QTabWidget::West:
        option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedWest : QTabBar::TriangularWest;
        break;
    case QTabWidget::East:
        option.shape = shape == QTabWidget::Rounded ? QTabBar::RoundedEast : QTabBar::TriangularEast;
        break;
    }
    option.tabBarSize = t;

    return option;
}

/*!
    Constructs a tabbed widget with parent \a parent.
*/
QTabWidget::QTabWidget(QWidget *parent)
    : QWidget(*new QTabWidgetPrivate, parent, 0)
{
    Q_D(QTabWidget);
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QTabWidget::QTabWidget(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(*new QTabWidgetPrivate, parent, f)
{
    Q_D(QTabWidget);
    setObjectName(QString::fromAscii(name));
    d->init();
}
#endif

/*!
    Destroys the tabbed widget.
*/
QTabWidget::~QTabWidget()
{
}

/*!
    Adds another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as a shortcut for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes a shortcut which will
    move the focus to this tab.

    \sa insertTab()
*/
int QTabWidget::addTab(QWidget *child, const QString &label)
{
    return insertTab(-1, child, label);
}


/*!
    \overload

    Adds another tab and page to the tab view.

    This function is the same as addTab(), but with an additional \a
    icon.
*/
int QTabWidget::addTab(QWidget *child, const QIcon& icon, const QString &label)
{
    return insertTab(-1, child, icon, label);
}


/*!
    Inserts another tab and page to the tab view.

    The new page is \a w; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as a shortcut for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes a shortcut which will
    move the focus to this tab.

    If \a index is out of range, the tab is simply appended.
    Otherwise it is inserted at the specified position.

    If you call insertTab() after show(), the screen will flicker and
    the user may be confused.

    \sa addTab()
*/
int QTabWidget::insertTab(int index, QWidget *w, const QString &label)
{
    return insertTab(index, w, QIcon(), label);
}


/*!
    \overload

    Inserts another tab and page to the tab view.

    This function is the same as insertTab(), but with an additional
    \a icon.
*/
int QTabWidget::insertTab(int index, QWidget *w, const QIcon& icon, const QString &label)
{
    Q_D(QTabWidget);
    if(!w)
        return -1;
    index = d->tabs->insertTab(index, icon, label);
    d->stack->insertWidget(index, w);
    setUpLayout();
    tabInserted(index);
    return index;
}


/*!
    Defines a new \a label for the page at position \a index's tab.
*/
void QTabWidget::setTabText(int index, const QString &label)
{
    Q_D(QTabWidget);
    d->tabs->setTabText(index, label);
    setUpLayout();
}

/*!
    Returns the label text for the tab on the page at position \a index.
*/

QString QTabWidget::tabText(int index) const
{
    Q_D(const QTabWidget);
    return d->tabs->tabText(index);
}

/*!
    \overload

    Sets the \a icon for the tab at position \a index.
*/
void QTabWidget::setTabIcon(int index, const QIcon &icon)
{
    Q_D(QTabWidget);
    d->tabs->setTabIcon(index, icon);
    setUpLayout();
}

/*!
    Returns the label text for the tab on the page at position \a index.
*/

QIcon QTabWidget::tabIcon(int index) const
{
    Q_D(const QTabWidget);
    return d->tabs->tabIcon(index);
}

/*!
    Returns true if the the page at position \a index is enabled; otherwise returns false.

    \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabWidget::isTabEnabled(int index) const
{
    Q_D(const QTabWidget);
    return d->tabs->isTabEnabled(index);
}

/*!
    If \a enable is true, the page at position \a index is enabled; otherwise the page at position \a index is
    disabled. The page's tab is redrawn appropriately.

    QTabWidget uses QWidget::setEnabled() internally, rather than
    keeping a separate flag.

    Note that even a disabled tab/page may be visible. If the page is
    visible already, QTabWidget will not hide it; if all the pages are
    disabled, QTabWidget will show one of them.

    \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabWidget::setTabEnabled(int index, bool enable)
{
    Q_D(QTabWidget);
    d->tabs->setTabEnabled(index, enable);
}

/*!
  \fn void QTabWidget::setCornerWidget(QWidget *widget, Qt::Corner corner)

  Sets the given \a widget to be shown in the specified \a corner of the
  tab widget.

  Only the horizontal element of the \a corner will be used.

  \sa cornerWidget(), setTabPosition()
*/
void QTabWidget::setCornerWidget(QWidget * w, Qt::Corner corner)
{
    Q_D(QTabWidget);
    if (!w)
        return;
    if ((uint)corner & 1)
        d->rightCornerWidget = w;
    else
        d->leftCornerWidget = w;
}

/*!
    Returns the widget shown in the \a corner of the tab widget or 0.
*/
QWidget * QTabWidget::cornerWidget(Qt::Corner corner) const
{
    Q_D(const QTabWidget);
    if ((uint)corner & 1)
        return d->rightCornerWidget;
    return d->leftCornerWidget;
}

/*!
   Removes the page at position \a index from this stack of
   widgets. Does not delete the page widget.
*/
void QTabWidget::removeTab(int index)
{
    Q_D(QTabWidget);
    if (QWidget *w = d->stack->widget(index))
        d->stack->removeWidget(w);
}

/*!
    Returns a pointer to the page currently being displayed by the tab
    dialog. The tab dialog does its best to make sure that this value
    is never 0 (but if you try hard enough, it can be).

    \sa currentIndex(), setCurrentWidget()
*/

QWidget * QTabWidget::currentWidget() const
{
    Q_D(const QTabWidget);
    return d->stack->currentWidget();
}

/*!
    Makes \a widget the current widget. The \a widget used must be a page in
    this tab widget.

    \sa addTab(), setCurrentIndex(), currentWidget()
 */
void QTabWidget::setCurrentWidget(QWidget *widget)
{
    Q_D(const QTabWidget);
    d->tabs->setCurrentIndex(indexOf(widget));
}


/*!
    \property QTabWidget::currentIndex
    \brief the index position of the current tab page

*/

int QTabWidget::currentIndex() const
{
    Q_D(const QTabWidget);
    return d->tabs->currentIndex();
}

void QTabWidget::setCurrentIndex(int index)
{
    Q_D(QTabWidget);
    d->tabs->setCurrentIndex(index);
}


/*!
    Returns the index position of the page occupied by the widget \a
    w, or -1 if the widget cannot be found.
*/
int QTabWidget::indexOf(QWidget* w) const
{
    Q_D(const QTabWidget);
    return d->stack->indexOf(w);
}


/*!
    \reimp
*/
void QTabWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    setUpLayout();
}

/*!
    Replaces the dialog's QTabBar heading with the tab bar \a tb. Note
    that this must be called \e before any tabs have been added, or
    the behavior is undefined.

    \sa tabBar()
*/
void QTabWidget::setTabBar(QTabBar* tb)
{
    Q_D(QTabWidget);
    Q_ASSERT(tb);

    if (tb->parentWidget() != this) {
        tb->setParent(this);
        tb->show();
    }
    delete d->tabs;
    d->tabs = tb;
    setFocusProxy(d->tabs);
    connect(d->tabs, SIGNAL(currentChanged(int)),
             this,    SLOT(showTab(int)));
    setUpLayout();
}


/*!
    Returns the current QTabBar.

    \sa setTabBar()
*/
QTabBar* QTabWidget::tabBar() const
{
    Q_D(const QTabWidget);
    return d->tabs;
}

/*!
    Ensures that the selected tab's page is visible and appropriately
    sized.
*/

void QTabWidgetPrivate::showTab(int index)
{
    Q_Q(QTabWidget);
    if (index < stack->count() && index >= 0) {
        stack->setCurrentIndex(index);
        emit q->currentChanged(index);
#ifdef QT3_SUPPORT
        emit q->currentChanged(stack->widget(index));
#endif
    }
}

void QTabWidgetPrivate::removeTab(int index)
{
    Q_Q(QTabWidget);
    tabs->removeTab(index);
    q->setUpLayout();
    q->tabRemoved(index);
}

/*
    Set up the layout.
    Get subrect from the current style, and set the geometry for the
    stack widget, tab bar and corner widgets.
*/
void QTabWidget::setUpLayout(bool onlyCheck)
{
    Q_D(QTabWidget);
    if (onlyCheck && !d->dirty)
        return; // nothing to do

    if (!isVisible()) {
        d->dirty = true;
        return; // we'll do it later
    }
    QStyleOptionTabWidgetFrame option = d->getStyleOption();

    QRect tabRect = style()->subElementRect(QStyle::SE_TabWidgetTabBar, &option, this);
    d->panelRect = style()->subElementRect(QStyle::SE_TabWidgetTabPane, &option, this);
    QRect contentsRect = style()->subElementRect(QStyle::SE_TabWidgetTabContents, &option, this);
    QRect leftCornerRect = style()->subElementRect(QStyle::SE_TabWidgetLeftCorner, &option, this);
    QRect rightCornerRect = style()->subElementRect(QStyle::SE_TabWidgetRightCorner, &option, this);

    d->tabs->setGeometry(tabRect);
    d->stack->setGeometry(contentsRect);
    if (d->leftCornerWidget)
        d->leftCornerWidget->setGeometry(leftCornerRect);
    if (d->rightCornerWidget)
        d->rightCornerWidget->setGeometry(rightCornerRect);

    if (!onlyCheck)
        update();
    updateGeometry();
}

/*!
    \reimp
*/
QSize QTabWidget::sizeHint() const
{
    Q_D(const QTabWidget);
    QSize lc(0, 0), rc(0, 0);
    QStyleOption opt(0);
    opt.init(this);
    opt.state = QStyle::State_None;

    if (d->leftCornerWidget)
        lc = d->leftCornerWidget->sizeHint();
    if(d->rightCornerWidget)
        rc = d->rightCornerWidget->sizeHint();
    if (!d->dirty) {
        QTabWidget *that = (QTabWidget*)this;
        that->setUpLayout(true);
    }
    QSize s(d->stack->sizeHint());
    QSize t(d->tabs->sizeHint());
    if(!style()->styleHint(QStyle::SH_TabBar_PreferNoArrows, &opt, d->tabs))
        t = t.boundedTo(QSize(200,200));
    else
        t = t.boundedTo(QApplication::desktop()->size());
    QSize sz;
    if (d->pos == North || d->pos == South)
        sz = QSize(qMax(s.width(), t.width() + rc.width() + lc.width()),
                   s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))));
    else
        sz = QSize(s.width() + (qMax(rc.width(), qMax(lc.width(), t.width()))),
                   qMax(s.height(), t.height() + rc.height() + lc.height()));
    return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this)
                    .expandedTo(QApplication::globalStrut());
}


/*!
    \reimp

    Returns a suitable minimum size for the tab widget.
*/
QSize QTabWidget::minimumSizeHint() const
{
    Q_D(const QTabWidget);
    QSize lc(0, 0), rc(0, 0);

    if(d->leftCornerWidget)
        lc = d->leftCornerWidget->minimumSizeHint();
    if(d->rightCornerWidget)
        rc = d->rightCornerWidget->minimumSizeHint();
    if (!d->dirty) {
        QTabWidget *that = (QTabWidget*)this;
        that->setUpLayout(true);
    }
    QSize s(d->stack->minimumSizeHint());
    QSize t(d->tabs->minimumSizeHint());

    QSize sz(qMax(s.width(), t.width() + rc.width() + lc.width()),
              s.height() + (qMax(rc.height(), qMax(lc.height(), t.height()))));
    QStyleOption opt(0);
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    return style()->sizeFromContents(QStyle::CT_TabWidget, &opt, sz, this)
                    .expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
 */
void QTabWidget::showEvent(QShowEvent *)
{
    setUpLayout();
}

void QTabWidgetPrivate::updateTabBarPosition()
{
    Q_Q(QTabWidget);
    switch (pos) {
    case QTabWidget::North:
        tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedNorth
                                                    : QTabBar::TriangularNorth);
        break;
    case QTabWidget::South:
        tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedSouth
                                                    : QTabBar::TriangularSouth);
        break;
    case QTabWidget::West:
        tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedWest
                                                    : QTabBar::TriangularWest);
        break;
    case QTabWidget::East:
        tabs->setShape(shape == QTabWidget::Rounded ? QTabBar::RoundedEast
                                                    : QTabBar::TriangularEast);
        break;
    }
    q->setUpLayout();
}

/*!
    \property QTabWidget::tabPosition
    \brief the position of the tabs in this tab widget

    Possible values for this property are QTabWidget::North and
    QTabWidget::South.

    \sa TabPosition
*/
QTabWidget::TabPosition QTabWidget::tabPosition() const
{
    Q_D(const QTabWidget);
    return d->pos;
}

void QTabWidget::setTabPosition(TabPosition pos)
{
    Q_D(QTabWidget);
    if (d->pos == pos)
        return;
    d->pos = pos;
    d->updateTabBarPosition();
}

/*!
    \property QTabWidget::tabShape
    \brief the shape of the tabs in this tab widget

    Possible values for this property are QTabWidget::Rounded
    (default) or QTabWidget::Triangular.

    \sa TabShape
*/

QTabWidget::TabShape QTabWidget::tabShape() const
{
    Q_D(const QTabWidget);
    return d->shape;
}

void QTabWidget::setTabShape(TabShape s)
{
    Q_D(QTabWidget);
    if (d->shape == s)
        return;
    d->shape = s;
    d->updateTabBarPosition();
}

/*!
    \reimp
 */
void QTabWidget::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange)
        setUpLayout();
    QWidget::changeEvent(ev);
}


/*!
    \reimp
 */
void QTabWidget::keyPressEvent(QKeyEvent *e)
{
    Q_D(QTabWidget);
    if ((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) &&
        count() > 1 && e->modifiers() & Qt::ControlModifier) {
        int page = currentIndex();
        if (e->key() == Qt::Key_Backtab || e->modifiers() & Qt::ShiftModifier) {
            page--;
            if (page < 0)
                page = count() - 1;
        } else {
            page++;
            if (page >= count())
                page = 0;
        }
        setCurrentIndex(page);
        if (!qApp->focusWidget())
            d->tabs->setFocus();
    } else {
        e->ignore();
    }
}

/*!
    Returns the tab page at index position \a index or 0 if the \a
    index is out of range.
*/
QWidget *QTabWidget::widget(int index) const
{
    Q_D(const QTabWidget);
    return d->stack->widget(index);
}

/*!
    \property QTabWidget::count
    \brief the number of tabs in the tab bar
*/
int QTabWidget::count() const
{
    Q_D(const QTabWidget);
    return d->tabs->count();
}

#ifndef QT_NO_TOOLTIP
/*!
    Sets the tab tool tip for the page at position \a index to \a tip.

    \sa  tabToolTip()
*/
void QTabWidget::setTabToolTip(int index, const QString & tip)
{
    Q_D(QTabWidget);
    d->tabs->setTabToolTip(index, tip);
}

/*!
    Returns the tab tool tip for the page at position \a index or
    an empty string if no tool tip has been set.

    \sa setTabToolTip()
*/
QString QTabWidget::tabToolTip(int index) const
{
    Q_D(const QTabWidget);
    return d->tabs->tabToolTip(index);
}
#endif // QT_NO_TOOLTIP

#ifndef QT_NO_WHATSTHIS
/*!
    \since 4.1

    Sets the What's This help text for the page at position \a index
    to \a text.
*/
void QTabWidget::setTabWhatsThis(int index, const QString &text)
{
    Q_D(QTabWidget);
    d->tabs->setTabWhatsThis(index, text);
}

/*!
    \since 4.1

    Returns the What's This help text for the page at position \a index,
    or an empty string if no help text has been set.
*/
QString QTabWidget::tabWhatsThis(int index) const
{
    Q_D(const QTabWidget);
    return d->tabs->tabWhatsThis(index);
}
#endif // QT_NO_WHATSTHIS

/*!
  This virtual handler is called after a new tab was added or
  inserted at position \a index.

  \sa tabRemoved()
 */
void QTabWidget::tabInserted(int index)
{
    Q_UNUSED(index)
}

/*!
  This virtual handler is called after a tab was removed from
  position \a index.

  \sa tabInserted()
 */
void QTabWidget::tabRemoved(int index)
{
    Q_UNUSED(index)
}

/*!
    \fn void QTabWidget::paintEvent(QPaintEvent *event)

    Paints the tab widget's tab bar in response to the paint \a event.
*/
void QTabWidget::paintEvent(QPaintEvent *)
{
    Q_D(QTabWidget);
    QStylePainter p(this);
    QStyleOptionTabWidgetFrame opt;
    opt = d->getStyleOption();
    opt.rect = d->panelRect;
    p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
}

/*!
    \fn void QTabWidget::insertTab(QWidget * widget, const QString
    &label, int index)

    Use insertTab(index, widget, label) instead.
*/

/*!
    \fn void QTabWidget::insertTab(QWidget *widget, const QIcon& icon, const QString &label, int index)

    Use insertTab(index, widget, icon, label) instead.
*/

/*!
    \fn void QTabWidget::changeTab(QWidget *widget, const QString
    &label)

    Use setTabText() instead.

*/

/*!
    \fn void QTabWidget::changeTab(QWidget *widget, const QIcon& icon, const QString &label)

    Use setTabText() and setTabIcon() instead.
*/

/*!
    \fn bool QTabWidget::isTabEnabled( QWidget *widget) const

    Use isTabEnabled(tabWidget->indexOf(widget)) instead.
*/

/*!
    \fn void QTabWidget::setTabEnabled(QWidget *widget, bool b)

    Use isTabEnabled(tabWidget->indexOf(widget), b) instead.
*/

/*!
    \fn QString QTabWidget::tabLabel(QWidget *widget) const

    Use tabText(tabWidget->indexOf(widget)) instead.
*/

/*!
    \fn void QTabWidget::setTabLabel(QWidget *widget, const QString
    &label)

    Use setTabText(tabWidget->indexOf(widget), label) instead.
*/

/*!
    \fn QIcon QTabWidget::tabIconSet(QWidget * widget) const

    Use tabIcon(tabWidget->indexOf(widget)) instead.
*/

/*!
    \fn void QTabWidget::setTabIconSet(QWidget * widget, const QIcon & icon)

    Use setTabIcon(tabWidget->indexOf(widget), icon) instead.
*/

/*!
    \fn void QTabWidget::removeTabToolTip(QWidget * widget)

    Use setTabToolTip(tabWidget->indexOf(widget), QString()) instead.
*/

/*!
    \fn void QTabWidget::setTabToolTip(QWidget * widget, const QString & tip)

    Use setTabToolTip(tabWidget->indexOf(widget), tip) instead.
*/

/*!
    \fn QString QTabWidget::tabToolTip(QWidget * widget) const

    Use tabToolTip(tabWidget->indexOf(widget)) instead.
*/

/*!
    \fn QWidget * QTabWidget::currentPage() const

    Use currentWidget() instead.
*/

/*!
    \fn QWidget *QTabWidget::page(int index) const

    Use widget() instead.
*/

/*!
    \fn QString QTabWidget::label(int index) const

    Use tabText() instead.
*/

/*!
    \fn int QTabWidget::currentPageIndex() const

    Use currentIndex() instead.
*/

/*!
    \fn int QTabWidget::margin() const

    This function is kept only to make old code compile.
    This functionality is no longer supported by QTabWidget.

    \sa contentsRect(), setContentsMargins()
*/

/*!
    \fn void QTabWidget::setMargin(int margin)

    This function is kept only to make old code compile.
    This functionality is no longer supported by QTabWidget.

    \sa contentsRect(), setContentsMargins()
*/

/*!
    \fn void QTabWidget::setCurrentPage(int index)

    Use setCurrentIndex() instead.
*/

/*!
    \fn void QTabWidget::showPage(QWidget *widget)

    Use setCurrentIndex(indexOf(widget)) instead.
*/

/*!
    \fn void QTabWidget::removePage(QWidget *widget)

    Use removeTab(indexOf(widget)) instead.
*/

/*!
    \fn void QTabWidget::currentChanged(QWidget *widget)

    Use currentChanged(int) instead.
*/


#include "moc_qtabwidget.cpp"

#endif //QT_NO_TABWIDGET
