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

#ifndef QCOMBOBOX_P_H
#define QCOMBOBOX_P_H

#include <qlistview.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qbasictimer.h>
#include <qabstractslider.h>
#include <qstyle.h>
#include <qpair.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <private/qwidget_p.h>
#include <limits.h>

class Scroller : public QWidget
{
    Q_OBJECT

public:
    Scroller(QAbstractSlider::SliderAction action, QWidget *parent)
        : QWidget(parent), sliderAction(action) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }
    QSize sizeHint() const {
        return QSize(20, style()->pixelMetric(QStyle::PM_MenuScrollerHeight));
    }

protected:
    void enterEvent(QEvent *) {
        timer.start(100, this);
    }
    void leaveEvent(QEvent *) {
        timer.stop();
    }
    void timerEvent(QTimerEvent *e) {
        if (e->timerId() == timer.timerId())
            emit doScroll(sliderAction);
    }
    void hideEvent(QHideEvent *) {
        timer.stop();
    }
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        QStyleOptionMenuItem menuOpt;
        menuOpt.palette = palette();
        menuOpt.state = QStyle::Style_None;
        menuOpt.checkType = QStyleOptionMenuItem::NotCheckable;
        menuOpt.menuRect = rect();
        menuOpt.rect = rect();
        menuOpt.maxIconWidth = 0;
        menuOpt.tabWidth = 0;
        menuOpt.menuItemType = QStyleOptionMenuItem::Scroller;
        if (sliderAction == QAbstractSlider::SliderSingleStepAdd)
            menuOpt.state = QStyle::Style_Down;
        p.eraseRect(rect());
        style()->drawControl(QStyle::CE_MenuScroller, &menuOpt, &p);
    }

signals:
    void doScroll(int action);

private:
    QAbstractSlider::SliderAction sliderAction;
    QBasicTimer timer;
};

class ListViewContainer : public QFrame
{
    Q_OBJECT

public:
    ListViewContainer(QListView *listView, QWidget *parent = 0);
    QListView *listView() const;

public slots:
    void scrollListView(int action);
    void updateScrollers();
    void setCurrentIndex(const QModelIndex &index, Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void hideEvent(QHideEvent *e);

signals:
    void itemSelected(const QModelIndex &);
    void containerDisappearing();

private:
    QListView *list;
    Scroller *top;
    Scroller *bottom;
};

class MenuDelegate : public QAbstractItemDelegate
{
public:
    MenuDelegate(QObject *parent, QComboBox *cmb) : QAbstractItemDelegate(parent), mCombo(cmb) {}

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        painter->eraseRect(option.rect);
        QApplication::style()->drawControl(QStyle::CE_MenuItem, &opt, painter, 0);
    }
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        QVariant value = index.model()->data(index, QAbstractItemModel::FontRole);
        QFont fnt = value.isValid() ? value.toFont() : option.font;
        return QApplication::style()->sizeFromContents(
            QStyle::CT_MenuItem, &opt, option.rect.size(), QFontMetrics(fnt), 0);
    }

private:
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const {
        QStyleOptionMenuItem menuOption;
        menuOption.palette = option.palette;
        menuOption.state = QStyle::Style_None;
        if (option.state & QStyle::Style_Enabled)
            menuOption.state |= QStyle::Style_Enabled;
        if (option.state & QStyle::Style_Selected)
            menuOption.state |= QStyle::Style_Selected;
        menuOption.checkType = QStyleOptionMenuItem::NonExclusive;
        menuOption.checked = mCombo->currentItem() == index.row();
        menuOption.menuItemType = QStyleOptionMenuItem::Normal;
        menuOption.icon = index.model()->data(index, QAbstractItemModel::DecorationRole).toIconSet();
        menuOption.text = index.model()->data(index, QAbstractItemModel::DisplayRole).toString();
        menuOption.tabWidth = 0;
        menuOption.maxIconWidth = 0;
        if (!menuOption.icon.isNull())
            menuOption.maxIconWidth
                = menuOption.icon.pixmap(Qt::SmallIconSize, QIcon::Normal).width() + 4;
        menuOption.menuRect = option.rect;
        menuOption.rect = option.rect;
        return menuOption;
    }

    QComboBox *mCombo;
};

class QComboBoxPrivate: public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QComboBox)
public:
    QComboBoxPrivate()
        : QWidgetPrivate(),
          model(0),
          lineEdit(0),
          container(0),
          insertionPolicy(QComboBox::AtBottom),
          autoCompletion(true),
          duplicatesEnabled(false),
          autoHide(true),
          arrowDown(false),
          sizeLimit(10),
          maxCount(INT_MAX),
          skipCompletion(false) {}
    ~QComboBoxPrivate() {}
    void init();
    QStyleOptionComboBox getStyleOption() const;
    void updateLineEditGeometry();
    void returnPressed();
    void complete();
    void itemSelected(const QModelIndex &item);
    bool contains(const QString &text, int role);
    void emitActivated(const QModelIndex&);
    void emitHighlighted(const QModelIndex&);
    void resetButton();

    QAbstractItemModel *model;
    QLineEdit *lineEdit;
    ListViewContainer *container;
    QComboBox::InsertionPolicy insertionPolicy;
    bool autoCompletion;
    bool duplicatesEnabled;
    bool autoHide;
    bool arrowDown;
    int sizeLimit;
    int maxCount;
    bool skipCompletion;
    mutable QSize sizeHint;
    QPersistentModelIndex currentIndex;
    QPersistentModelIndex root;
};

#endif //QCOMBOBOX_P_H
