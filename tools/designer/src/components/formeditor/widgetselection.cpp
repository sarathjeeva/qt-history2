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

#include "widgetselection.h"
#include "formwindow.h"
#include "formwindowmanager.h"

#include <qextensionmanager.h>
#include <abstractwidgetfactory.h>
#include <layoutinfo.h>
#include <qdesigner_command.h>
#include <taskmenu.h>

#include <QtGui/QMenu>
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QStylePainter>

#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

#define NO_TOPWIDGET

class TopWidget: public InvisibleWidget
{
    Q_OBJECT
public:
    TopWidget(QWidget *parent)
        : InvisibleWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
};

WidgetHandle::WidgetHandle(FormWindow *parent, WidgetHandle::Type t, WidgetSelection *s)
    : InvisibleWidget(parent)
{
    active = true;
    widget = 0;
    type = t;
    setMouseTracking(false);
    formWindow = parent;
    sel = s;

    if (type == TaskMenu) {
        setBackgroundRole(QPalette::Button);
        setFixedSize(12, 12);
    } else {
        setBackgroundRole(active ? QPalette::Text : QPalette::Dark);
        setFixedSize(6, 6);
    }

    updateCursor();
}

void WidgetHandle::updateCursor()
{
    if (!active) {
        setCursor(Qt::ArrowCursor);
        return;
    }

    switch (type) {
    case LeftTop:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Top:
        setCursor(Qt::SizeVerCursor);
        break;
    case RightTop:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case RightBottom:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case LeftBottom:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Left:
        setCursor(Qt::SizeHorCursor);
        break;
    case TaskMenu:
        setCursor(Qt::ArrowCursor);
        break;
    default:
        Q_ASSERT(0);
    }
}

AbstractFormEditor *WidgetHandle::core() const
{
    if (AbstractFormWindow *fw = formWindow)
        return fw->core();

    return 0;
}

void WidgetHandle::setActive(bool a)
{
    active = a;
    if (type != TaskMenu) {
        setBackgroundRole(active ? QPalette::Text : QPalette::Dark);
    }
    updateCursor();
}

void WidgetHandle::setWidget(QWidget *w)
{
    widget = w;
}

void WidgetHandle::paintEvent(QPaintEvent *)
{
    FormWindow *fw = formWindow;
    if (fw->currentWidget() != widget)
        return;

    AbstractFormWindowManager *m = fw->core()->formWindowManager();

    QStylePainter p(this);
    if (type == TaskMenu) {
        QStyleOptionToolButton option;
        option.init(this);
        option.state |= QStyle::State_Raised;
        option.arrowType = Qt::RightArrow;
        option.toolButtonStyle = Qt::ToolButtonIconOnly;
        option.features = QStyleOptionToolButton::Arrow;
        option.subControls = QStyle::SC_ToolButton;
        p.drawComplexControl(QStyle::CC_ToolButton, option);
    } else {
        p.setPen(m->activeFormWindow() == fw ? Qt::blue : Qt::red);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void WidgetHandle::mousePressEvent(QMouseEvent *e)
{
    e->accept();

    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;

    if (!(widget && e->button() == Qt::LeftButton))
        return;

    if (!(active || type == TaskMenu))
        return;

    QWidget *container = widget->parentWidget();

    oldPressPos = container->mapFromGlobal(e->globalPos());
    geom = origGeom = widget->geometry();

    if (type == TaskMenu && e->button() == Qt::LeftButton) {
        Q_ASSERT(sel->taskMenuExtension());

        QMenu m(this);
        foreach (QAction *a, sel->taskMenuExtension()->taskActions()) {
            m.addAction(a);
        }
        m.exec(e->globalPos());
    }

}

int WidgetHandle::adjustPoint(int x, int dx)
{ return (x / dx) * dx + 1; }

void WidgetHandle::mouseMoveEvent(QMouseEvent *e)
{
    if (!(widget && active && e->buttons() & Qt::LeftButton))
        return;

    if (type == TaskMenu)
        return;

    //e->accept();

    QWidget *container = widget->parentWidget();

    QPoint rp = container->mapFromGlobal(e->globalPos());
    QPoint d = rp - oldPressPos;
    oldPressPos = rp;

    QRect pr = container->rect();
    QPoint grid = formWindow->grid();

    switch (type) {

    case TaskMenu:
        break;

    case LeftTop: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dx = widget->width() - w;
        int dy = widget->height() - h;

        trySetGeometry(widget, widget->x() + dx, widget->y() + dy, w, h);
    } break;

    case Top: {
        if (rp.y() > pr.height() - 2 * height())
            return;

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dy = widget->height() - h;
        trySetGeometry(widget, widget->x(), widget->y() + dy, widget->width(), h);
    } break;

    case RightTop: {
        if (rp.x() < 2 * width() || rp.y() > pr.height() - 2 * height())
            return;

        int h = geom.height() - d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dy = widget->height() - h;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        trySetGeometry(widget, widget->x(), widget->y() + dy, w, h);
    } break;

    case Right: {
        if (rp.x() < 2 * width())
            return;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        tryResize(widget, w, widget->height());
    } break;

    case RightBottom: {
        if (rp.x() < 2 * width() || rp.y() < 2 * height())
            return;

        int w = geom.width() + d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(widget, w, h);
    } break;

    case Bottom: {
        if (rp.y() < 2 * height())
            return;

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        tryResize(widget, widget->width(), h);
    } break;

    case LeftBottom: {
        if (rp.x() > pr.width() - 2 * width() || rp.y() < 2 * height())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int h = geom.height() + d.y();
        geom.setHeight(h);
        h = adjustPoint(h, grid.y());

        int dx = widget->width() - w;

        trySetGeometry(widget, widget->x() + dx, widget->y(), w, h);
    } break;

    case Left: {
        if (rp.x() > pr.width() - 2 * width())
            return;

        int w = geom.width() - d.x();
        geom.setWidth(w);
        w = adjustPoint(w, grid.x());

        int dx = widget->width() - w;

        trySetGeometry(widget, widget->x() + dx, widget->y(), w, widget->height());
    } break;

    default: break;

    } // end switch

    sel->updateGeometry();

    if (LayoutInfo::layoutType(formWindow->core(), widget) != LayoutInfo::NoLayout)
        formWindow->updateChildSelections(widget);
}

void WidgetHandle::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !active)
        return;

    if (type == TaskMenu)
        return;

    e->accept();

    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;

    if (geom != widget->geometry()) {
        SetPropertyCommand *cmd = new SetPropertyCommand(formWindow);
        cmd->init(widget, "geometry", widget->geometry());
        cmd->setOldValue(origGeom);
        formWindow->commandHistory()->push(cmd);
        formWindow->emitSelectionChanged();
    }
}

void WidgetHandle::trySetGeometry(QWidget *w, int x, int y, int width, int height)
{
    if (!formWindow->hasFeature(FormWindow::EditFeature))
        return;

    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 2 * formWindow->grid().x());

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 2 * formWindow->grid().y());

    if (qMax(minw, width) > w->maximumWidth() ||
         qMax(minh, height) > w->maximumHeight())
        return;

    if (width < minw && x != w->x())
        x -= minw - width;

    if (height < minh && y != w->y())
        y -= minh - height;

    w->setGeometry(x, y, qMax(minw, width), qMax(minh, height));
}

void WidgetHandle::tryResize(QWidget *w, int width, int height)
{
    int minw = qMax(w->minimumSizeHint().width(), w->minimumSize().width());
    minw = qMax(minw, 16);

    int minh = qMax(w->minimumSizeHint().height(), w->minimumSize().height());
    minh = qMax(minh, 16);

    w->resize(qMax(minw, width), qMax(minh, height));
}

// ------------------------------------------------------------------------

WidgetSelection::WidgetSelection(FormWindow *parent, QHash<QWidget *, WidgetSelection *> *selDict)
    : selectionDict(selDict)
{
    taskMenu = 0;
    formWindow = parent;
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        handles.insert(i, new WidgetHandle(formWindow, (WidgetHandle::Type)i, this));
    }

    m_topWidget = 0;

    hide();
}

void WidgetSelection::setWidget(QWidget *w, bool updateDict)
{
    taskMenu = qt_extension<ITaskMenu*>(core()->extensionManager(), w);

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        Q_ASSERT(m_topWidget->parentWidget() != 0);
        m_topWidget->parentWidget()->setAttribute(Qt::WA_ContentsPropagated, false);
    }

    delete m_topWidget;
    m_topWidget = 0;
#endif


    if (!w) {
        hide();
        if (updateDict)
            selectionDict->remove(wid);
        wid = 0;
        return;
    }

    wid = w;
    bool active = !wid->parentWidget() || LayoutInfo::layoutType(formWindow->core(), wid->parentWidget()) == LayoutInfo::NoLayout;
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = handles[ i ];
        if (h) {
            h->setWidget(wid);
            h->setActive(active);
        }
    }

#ifndef NO_TOPWIDGET
    if (wid) {
        wid->setAttribute(Qt::WA_ContentsPropagated, true);
        m_topWidget = new TopWidget(wid);
        QPalette p = m_topWidget->palette();
        p.setColor(m_topWidget->backgroundRole(), QColor(255, 0, 0, 32));
        m_topWidget->setPalette(p);
    }
#endif

    updateGeometry();
    show();

    if (updateDict)
        selectionDict->insert(w, this);
}

bool WidgetSelection::isUsed() const
{
    return wid != 0;
}

void WidgetSelection::updateGeometry()
{
    if (!wid || !wid->parentWidget())
        return;

    QPoint p = wid->parentWidget()->mapToGlobal(wid->pos());
    p = formWindow->mapFromGlobal(p);
    QRect r(p, wid->size());

    int w = 6;
    int h = 6;

    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *hndl = handles[ i ];
        if (!hndl)
            continue;
        switch (i) {
        case WidgetHandle::LeftTop:
            hndl->move(r.x() - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::Top:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::RightTop:
            hndl->move(r.x() + r.width() - w / 2, r.y() - h / 2);
            break;
        case WidgetHandle::Right:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        case WidgetHandle::RightBottom:
            hndl->move(r.x() + r.width() - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::Bottom:
            hndl->move(r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::LeftBottom:
            hndl->move(r.x() - w / 2, r.y() + r.height() - h / 2);
            break;
        case WidgetHandle::Left:
            hndl->move(r.x() - w / 2, r.y() + r.height() / 2 - h / 2);
            break;
        case WidgetHandle::TaskMenu:
            hndl->move(r.x() + r.width() - w / 2, r.y() + h - h / 2);
            break;
        default:
            break;
        }
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        m_topWidget->setGeometry(wid->rect());
    }
#endif
}

void WidgetSelection::hide()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = handles[ i ];
        if (h)
            h->hide();
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget)
        m_topWidget->hide();
#endif
}

void WidgetSelection::show()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = handles[ i ];
        if (h) {
            if (i == WidgetHandle::TaskMenu) {
                h->setVisible(taskMenuExtension() != 0);
                h->raise();
            } else {
                h->show();
                h->raise();
            }
        }
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget) {
        m_topWidget->show();
        m_topWidget->raise();
    }
#endif
}

void WidgetSelection::update()
{
    for (int i = WidgetHandle::LeftTop; i < WidgetHandle::TypeCount; ++i) {
        WidgetHandle *h = handles[ i ];
        if (h)
            h->update();
    }

#ifndef NO_TOPWIDGET
    if (m_topWidget)
        m_topWidget->update();
#endif
}

QWidget *WidgetSelection::widget() const
{
    return wid;
}

AbstractFormEditor *WidgetSelection::core() const
{
    if (formWindow)
        return formWindow->core();

    return 0;
}

#include "widgetselection.moc"
