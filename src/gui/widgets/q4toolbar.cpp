#include "q4toolbar.h"

#include "q4mainwindow.h"
#include "q4mainwindowlayout_p.h"
#include "q4toolbarbutton_p.h"

#include <qaction.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qtoolbutton.h>
#include <qmenu.h>

#include <private/qframe_p.h>
#define d d_func()
#define q q_func()

static inline QCOORD pick(Qt::Orientation o, const QPoint &p)
{ return o == Qt::Horizontal ? p.x() : p.y(); }

static inline QCOORD pick(Qt::Orientation o, const QSize &s)
{ return o == Qt::Horizontal ? s.width() : s.height(); }

// ## this should be styled and not inherit from QFrame
class Q4ToolBarSeparator : public QFrame
{
public:
    Q4ToolBarSeparator(Q4ToolBar *parent, Qt::Orientation new_ori) : QFrame(parent)
    {
	setOrientation(new_ori);
    }

    inline QSize sizeHint() const
    {
	int ext = ori == Horizontal ? QFrame::sizeHint().width() : QFrame::sizeHint().height();
	return QSize(ext, ext);
    }
    inline QSize minimumSize() const
    { return sizeHint(); }
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void setOrientation(Qt::Orientation new_ori) {
	if (new_ori == Horizontal)
	    setFrameStyle(QFrame::VLine | QFrame::Sunken);
	else
	    setFrameStyle(QFrame::HLine | QFrame::Sunken);
	ori = new_ori;
    }
    Qt::Orientation orientation() { return ori; }

private:
    Qt::Orientation ori;
};

class Q4ToolBarHandle : public QWidget
{
public:
    Q4ToolBarHandle(Q4ToolBar *parent) : QWidget(parent), state(0)
    {
	setCursor(SizeAllCursor);
    }

    QSize sizeHint() const {
	const int ext = QApplication::style().pixelMetric(QStyle::PM_DockWindowHandleExtent);
	return QSize(ext, ext);
    }
    inline QSize minimumSize() const
    { return sizeHint(); }
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    Qt::Orientation orientation();

    struct DragState {
	QPoint offset;
	bool canDrop;
    };
    DragState *state;
};


void Q4ToolBarHandle::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    QBoxLayout *box = qt_cast<QBoxLayout *>(parentWidget()->layout());
    if (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft)
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive(QStyle::PE_DockWindowHandle, &p, QStyle::visualRect(rect(), this),
			  palette(), flags);
    QWidget::paintEvent(e);
}

void Q4ToolBarHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != LeftButton) return;

    Q_ASSERT(parentWidget());
    Q_ASSERT(!state);
    state = new DragState;

    state->offset = mapTo(parentWidget(), event->pos());
}

void Q4ToolBarHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != LeftButton) return;

    delete state;
    state = 0;
}

void Q4ToolBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    Q_ASSERT(state != 0);

    // see if there is a main window under us, and ask them to place
    // the tool bar accordingly
    QWidget *widget = QApplication::widgetAt(event->globalPos());
    if (widget) {
	while (widget && (!qt_cast<Q4MainWindow *>(widget)))
	    widget = widget->parentWidget();

	if (widget) {
 	    Q4MainWindowLayout *layout = qt_cast<Q4MainWindowLayout *>(widget->layout());
	    Q_ASSERT_X(layout != 0, "Q4MainWindow", "must have a layout");
	    QPoint p = parentWidget()->mapFromGlobal(event->globalPos()) - state->offset;

	    // ### the offset is measured from the widget origin
	    if (orientation() == Vertical)
		p.setX(state->offset.x() + p.x());
	    else
		p.setY(state->offset.y() + p.y());

	    // try re-positioning toolbar
	    layout->dropToolBar(qt_cast<Q4ToolBar *>(parentWidget()), event->globalPos(), p);
	}
    }
}

Qt::Orientation Q4ToolBarHandle::orientation()
{
    QBoxLayout *box = qt_cast<QBoxLayout *>(parentWidget()->layout());
    if (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft)
	return Horizontal;
    return Vertical;
}


// ### move this into the style code and make the extension stylable
static const char * const arrow_v_xpm[] = {
    "7 9 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    ".+++++.",
    "..+++..",
    "+..+..+",
    "++...++",
    ".++.++.",
    "..+++..",
    "+..+..+",
    "++...++",
    "+++.+++"};

static const char * const arrow_h_xpm[] = {
    "9 7 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    "..++..+++",
    "+..++..++",
    "++..++..+",
    "+++..++..",
    "++..++..+",
    "+..++..++",
    "..++..+++"};

class Q4ToolBarExtension : public QToolButton
{
public:
    Q4ToolBarExtension(QWidget *parent);

    void setOrientation(Orientation o);
    QSize sizeHint() const{ return QSize(14, 14); }
    QSize minimumSize() const { return sizeHint(); }
    QSize minimumSizeHint() const { return sizeHint(); }

private:
    Orientation orientation;
};

Q4ToolBarExtension::Q4ToolBarExtension(QWidget *parent)
    : QToolButton(parent)
{
    setAutoRaise(true);
    setOrientation(Horizontal);
}

void Q4ToolBarExtension::setOrientation(Orientation o)
{
    if (o == Horizontal) {
        setIcon(QPixmap((const char **)arrow_h_xpm));
    } else {
        setIcon(QPixmap((const char **)arrow_v_xpm));
   }
}

class Q4ToolBarPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(Q4ToolBar);
public:
    void actionTriggered();
    ToolBarArea currentArea;
    Q4ToolBarExtension *extension;
    Q4ToolBarHandle *handle;
};

void Q4ToolBarPrivate::actionTriggered()
{
    QAction *action = qt_cast<QAction *>(q->sender());
    Q_ASSERT_X(action != 0, "Q4ToolBar::actionTriggered", "internal error");
    emit q->actionTriggered(action);
}


Q4ToolBar::Q4ToolBar(Q4MainWindow *parent)
    : QFrame(*new Q4ToolBarPrivate, parent)
{
    Q_ASSERT_X(parent != 0, "Q4MainWindow", "parent cannot be zero");

    setFrameStyle(QFrame::Panel | QFrame::Raised);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setSpacing(2);
    d->handle = new Q4ToolBarHandle(this);
    d->extension = new Q4ToolBarExtension(this);
    d->extension->hide();
    layout->removeWidget(d->extension);
    setCurrentArea(ToolBarAreaTop);
}

Q4ToolBar::~Q4ToolBar()
{
}

QAction *Q4ToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction *Q4ToolBar::addAction(const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

QAction *Q4ToolBar::addAction(const QString &text,
                              const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

QAction *Q4ToolBar::addAction(const QIconSet &icon, const QString &text,
                              const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

QAction *Q4ToolBar::insertAction(QAction *before, const QString &text)
{
    QAction *action = new QAction(text, this);
    insertAction(before, action);
    return action;
}

QAction *Q4ToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    insertAction(before, action);
    return action;
}

QAction *Q4ToolBar::insertAction(QAction *before, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
    return action;
}

QAction *Q4ToolBar::insertAction(QAction *before, const QIconSet &icon, const QString &text,
				 const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    insertAction(before, action);
    return action;
}

QAction *Q4ToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

QAction *Q4ToolBar::insertSeparator(QAction *before)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
}

void Q4ToolBar::childEvent(QChildEvent *event)
{
    QWidget *widget = qt_cast<QWidget *>(event->child());
    if (!widget) return;
    if (event->added()) layout()->addWidget(widget);
    else if (event->removed()) layout()->removeWidget(widget);
}

void Q4ToolBar::actionEvent(QActionEvent *event)
{
    QAction *action = event->action();

    switch (event->type()) {
    case QEvent::ActionAdded:
	if (action->isSeparator()) {
	    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
	    if (box->direction() == QBoxLayout::LeftToRight
		|| box->direction() == QBoxLayout::RightToLeft)
		(void) new Q4ToolBarSeparator(this, Horizontal);
	    else
		(void) new Q4ToolBarSeparator(this, Vertical);
	} else {
	    Q4ToolBarButton *button = new Q4ToolBarButton(this);
	    button->addAction(action);

            QObject::connect(action, SIGNAL(triggered()),
                             this, SLOT(actionTriggered()));
            if (action->menu()) {
                QObject::connect(action->menu(), SIGNAL(activated(QAction *)),
                                 this, SIGNAL(actionTriggered(QAction *)));
            }
	}
	break;

    case QEvent::ActionChanged:
        action->disconnect(this, SLOT(actionTriggered()));
        QObject::connect(action, SIGNAL(triggered()),
                         this, SLOT(actionTriggered()));
        if (action->menu()) {
            action->menu()->disconnect(this, SIGNAL(actionTriggered(QAction *)));
            QObject::connect(action->menu(), SIGNAL(activated(QAction *)),
                             this, SIGNAL(actionTriggered(QAction *)));
        }
	break;

    case QEvent::ActionRemoved:
	break;

    default:
	Q_ASSERT_X(false, "Q4ToolBar::actionEvent", "internal error");
    }
}

void Q4ToolBar::resizeEvent(QResizeEvent *event)
{
    QList<int> hidden;
    Orientation o;
    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
    if (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft)
	o = Horizontal;
    else
	o = Vertical;

    int i = 1; // tb handle is always the first item in the layout
    while (layout()->itemAt(i)) {
	QWidget *w = layout()->itemAt(i)->widget();
	if (!qt_cast<Q4ToolBarExtension *>(w)) {
	    if (pick(o, w->pos()) + pick(o, w->size())
		> (pick(o, size()) - (d->extension->isShown() ? pick(o, d->extension->size()) : 0)))
	    {
		w->hide();
		hidden.append(i-1);
	    } else {
		w->show();
	    }
	}
	++i;
    }

    if (hidden.size() > 0) {
	if (o == Horizontal)
 	    d->extension->setGeometry(width() - d->extension->sizeHint().width() - frameWidth(),
				      frameWidth(), d->extension->sizeHint().width() - frameWidth()*2, height() - frameWidth()*2);
 	else
 	    d->extension->setGeometry(frameWidth(), height() - d->extension->sizeHint().height() - frameWidth()*2,
				      width() - frameWidth()*2, d->extension->sizeHint().height());

	QMenu *pop = d->extension->popup();
	if (!pop) {
	    pop = new QMenu(this);
	    d->extension->setPopup(pop);
	    d->extension->setPopupDelay(-1);
	    box->removeWidget(pop); // auto-added to the layout - remove it
	}
	pop->clear();
	for(int i = 0; i < hidden.size(); ++i) {
	    // ### needs special handling of custom widgets and combo boxes
	    if (hidden.at(i) < actions().size())
		pop->addAction(actions().at(hidden.at(i)));
	}

	d->extension->show();
    } else if (d->extension->isShown() && hidden.size() == 0) {
	if (d->extension->popup())
	    d->extension->popup()->clear();
	d->extension->hide();
    }
}

Qt::ToolBarArea Q4ToolBar::currentArea() const
{
    return d->currentArea;
}

void Q4ToolBar::setCurrentArea(Qt::ToolBarArea area, bool linebreak)
{
    Q_ASSERT(parentWidget() && qt_cast<Q4MainWindowLayout *>(parentWidget()->layout()));
    Q4MainWindowLayout *mainwin_layout = qt_cast<Q4MainWindowLayout *>(parentWidget()->layout());
    mainwin_layout->add(this, area, linebreak);

    int pos;
    switch (area) {
    case Qt::ToolBarAreaLeft:   pos = 0; break;
    case Qt::ToolBarAreaRight:  pos = 1; break;
    case Qt::ToolBarAreaTop:    pos = 2; break;
    case Qt::ToolBarAreaBottom: pos = 3; break;
    default:
        Q_ASSERT(false);
        break;
    }

    QBoxLayout *box = qt_cast<QBoxLayout *>(layout());
    Q_ASSERT_X(box != 0, "Q4ToolBar::setCurrentArea", "internal error");

    switch (pos) {
    case 0: // Left
    case 1: // Right
	box->setDirection(QBoxLayout::TopToBottom);
	box->setAlignment(AlignTop);
	d->extension->setOrientation(Vertical);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	break;

    case 2: // Top
    case 3: // Bottom
	box->setDirection(QBoxLayout::LeftToRight);
	box->setAlignment(AlignLeft);
	d->extension->setOrientation(Horizontal);
	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	break;

    default:
	Q_ASSERT_X(false, "Q4ToolBar::setCurrentArea", "internal error");
    }

    // change the orientation of any separators
    QLayoutItem *item = 0;
    int i = 0;
    while ((item = box->itemAt(i++))) {
	Q4ToolBarSeparator *sep = qt_cast<Q4ToolBarSeparator *>(item->widget());
	if (sep) {
	    if (box->direction() == QBoxLayout::LeftToRight
		|| box->direction() == QBoxLayout::RightToLeft)
		sep->setOrientation(Horizontal);
	    else
		sep->setOrientation(Vertical);
	}
    }

    // if we're dragging - swap the offset coords around as well
    if (d->handle->state) {
	QPoint p = d->handle->state->offset;
	d->handle->state->offset = QPoint(p.y(), p.x());
    }
    d->currentArea = area;
}

#include "moc_q4toolbar.cpp"
