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

#ifndef QCALENDARWIDGET_H
#define QCALENDARWIDGET_H

#include <QtGui/qwidget.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_CALENDARWIDGET

class QDate;
class QTextCharFormat;
class QCalendarWidgetPrivate;

class Q_GUI_EXPORT QCalendarWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(Qt::DayOfWeek)
    Q_ENUMS(HorizontalHeaderFormat)
    Q_ENUMS(VerticalHeaderFormat)
    Q_ENUMS(SelectionMode)
    Q_PROPERTY(QDate selectedDate READ selectedDate WRITE setSelectedDate)
    Q_PROPERTY(QDate minimumDate READ minimumDate WRITE setMinimumDate)
    Q_PROPERTY(QDate maximumDate READ maximumDate WRITE setMaximumDate)
    Q_PROPERTY(Qt::DayOfWeek firstDayOfWeek READ firstDayOfWeek WRITE setFirstDayOfWeek)
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible)
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)
    Q_PROPERTY(HorizontalHeaderFormat horizontalHeaderFormat READ horizontalHeaderFormat WRITE setHorizontalHeaderFormat)
    Q_PROPERTY(VerticalHeaderFormat verticalHeaderFormat READ verticalHeaderFormat WRITE setVerticalHeaderFormat)
    Q_PROPERTY(bool headerVisible READ isHeaderVisible WRITE setHeaderVisible STORED false DESIGNABLE false) // obsolete
    Q_PROPERTY(bool navigationBarVisible READ isNavigationBarVisible WRITE setNavigationBarVisible)
    Q_PROPERTY(bool dateEditEnabled READ isDateEditEnabled WRITE setDateEditEnabled)
    Q_PROPERTY(int dateEditAcceptDelay READ dateEditAcceptDelay WRITE setDateEditAcceptDelay)

public:
    enum HorizontalHeaderFormat {
        NoHorizontalHeader,
        SingleLetterDayNames,
        ShortDayNames,
        LongDayNames
    };

    enum VerticalHeaderFormat {
        NoVerticalHeader,
        ISOWeekNumbers
    };

    enum SelectionMode {
        NoSelection,
        SingleSelection
    };

    explicit QCalendarWidget(QWidget *parent = 0);
    ~QCalendarWidget();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    QDate selectedDate() const;

    int yearShown() const;
    int monthShown() const;

    QDate minimumDate() const;
    void setMinimumDate(const QDate &date);

    QDate maximumDate() const;
    void setMaximumDate(const QDate &date);

    Qt::DayOfWeek firstDayOfWeek() const;
    void setFirstDayOfWeek(Qt::DayOfWeek dayOfWeek);

    // ### Qt 5: eliminate these two
    bool isHeaderVisible() const;
    void setHeaderVisible(bool show); 

    inline bool isNavigationBarVisible() const { return isHeaderVisible(); }

    bool isGridVisible() const;

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode mode);

    HorizontalHeaderFormat horizontalHeaderFormat() const;
    void setHorizontalHeaderFormat(HorizontalHeaderFormat format);

    VerticalHeaderFormat verticalHeaderFormat() const;
    void setVerticalHeaderFormat(VerticalHeaderFormat format);

    QTextCharFormat headerTextFormat() const;
    void setHeaderTextFormat(const QTextCharFormat &format);

    QTextCharFormat weekdayTextFormat(Qt::DayOfWeek dayOfWeek) const;
    void setWeekdayTextFormat(Qt::DayOfWeek dayOfWeek, const QTextCharFormat &format);

    QMap<QDate, QTextCharFormat> dateTextFormat() const;
    QTextCharFormat dateTextFormat(const QDate &date) const;
    void setDateTextFormat(const QDate &date, const QTextCharFormat &color);

    bool isDateEditEnabled() const;
    void setDateEditEnabled(bool enable);

    int dateEditAcceptDelay() const;
    void setDateEditAcceptDelay(int delay);

protected:
    bool event(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent * event);
    void keyPressEvent(QKeyEvent * event);

    virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

public Q_SLOTS:
    void setSelectedDate(const QDate &date);
    void setDateRange(const QDate &min, const QDate &max);
    void setCurrentPage(int year, int month);
    void setGridVisible(bool show);
    void setNavigationBarVisible(bool visible);
    void showNextMonth();
    void showPreviousMonth();
    void showNextYear();
    void showPreviousYear();
    void showSelectedDate();
    void showToday();

Q_SIGNALS:
    void selectionChanged();
    void clicked(const QDate &date);
    void activated(const QDate &date);
    void currentPageChanged(int year, int month);

private:
    Q_DECLARE_PRIVATE(QCalendarWidget)
    Q_DISABLE_COPY(QCalendarWidget)

    Q_PRIVATE_SLOT(d_func(), void _q_slotChangeDate(const QDate &date, bool changeMonth))
    Q_PRIVATE_SLOT(d_func(), void _q_editingFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_prevMonthClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_nextMonthClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_yearEditingFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_yearClicked())
    Q_PRIVATE_SLOT(d_func(), void _q_monthChanged(QAction *act))

};

#endif // QT_NO_CALENDARWIDGET

QT_END_HEADER

#endif // QCALENDARWIDGET_H

