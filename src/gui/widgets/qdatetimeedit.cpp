#include "qabstractspinbox_p.h"
#include "qabstractspinbox.h"
#include "qdatetimeedit.h"
#include <qlineedit.h>
#include <qevent.h>
#include <math.h>

class QDateTimeEditPrivate : public QAbstractSpinBoxPrivate
{
    Q_DECLARE_PUBLIC(QDateTimeEdit);
public:
    enum SectionFlags {
	NoSection = 0x0000,
	AMPMSection = 0x0001,
	MSecsSection = 0x0002,
	SecondsSection = 0x0004,
	MinutesSection = 0x0008,
	HoursSection = 0x0010,
        TimeMask = (AMPMSection|MSecsSection|SecondsSection|HoursSection),
	Internal = 0x8000,
	AMPMLowerCaseSection = AMPMSection|Internal,
	DaysSection = 0x0100,
	MonthsSection = 0x0200,
	YearsSection = 0x0400,
	MonthsShortNameSection = MonthsSection|Internal,
	YearsTwoDigitsSection = YearsSection|Internal,
        DateMask = (DaysSection|MonthsSection|YearsSection),
        FirstSection = 0x1000|Internal,
        LastSection = 0x2000|Internal
    }; // duplicated from qdatetimeedit.h

    struct SectionNode {
	SectionFlags section;
	int pos;
    };

    QDateTimeEditPrivate();

    void emitSignals();
    QCoreVariant mapTextToValue(QString *str, QValidator::State *state) const;
    QString mapValueToText(const QCoreVariant &n) const;
    void editorCursorPositionChanged(int lastpos, int newpos);
    QValidator::State validate(QString *input, int *pos, QCoreVariant *val) const;

    void clearSection(SectionFlags s);

    int sectionLength(SectionFlags s) const;
    int sectionPos(SectionFlags s) const;
    SectionNode sectionNode(SectionFlags t) const;
    QCoreVariant stepBy(SectionFlags s, int steps, bool test = false) const;
    QString sectionText(const QString &text, SectionFlags s) const;
    int getDigit(const QCoreVariant &dt, SectionFlags s) const;
    void setDigit(QCoreVariant *t, SectionFlags s, int newval) const;
    QString toString(const QCoreVariant &var) const;
    QCoreVariant fromString(QString *var, QValidator::State *state) const;
    int sectionValue(SectionFlags s, QString *txt, QValidator::State *state) const;
    int absoluteMax(SectionFlags s) const;
    int absoluteMin(SectionFlags s) const;
    SectionFlags sectionAt(int index) const;
    SectionFlags closestSection(int index, bool forward) const;
    SectionNode nextPrevSection(SectionFlags current, bool forward) const;
    SectionFlags addSection(QList<SectionNode> *list, SectionFlags ds, int pos);
    bool parseFormat(const QString &format);
    void setSelected(SectionFlags s, bool forward = false);

    static QString sectionName(int s);
    static QString stateName(int s);

    QString format;
    QList<SectionNode> sections;
    SectionNode first, last;
    QStringList separators;
    QDateTimeEdit::Section display;
    mutable int cachedday;
    mutable SectionFlags currentsection;
    SectionFlags oldsection;
};

#define d d_func()
#define q q_func()

// --- QDateTimeEdit ---

/*!
    \class QDateTimeEdit qdatetimeedit.h
    \brief The QDateTimeEdit class provides a date/time editor.

    \ingroup basic
    \mainclass

    QDateTimeEdit allows the user to edit dates by using the keyboard or
    the arrow keys to increase/decrease date and time values. The
    arrow keys can be used to move from section to section within the
    QDateTimeEdit box. Dates/times appear in accordance with the
    format set. See setFormat().

    \code
    QDateTimeEdit *dateEdit = new QDateTimeEdit(QDate::currentDate());
    dateEdit->setMinimumDate(QDate::currentDate().addDaysSection(-365));
    dateEdit->setMaximumDate(QDate::currentDate().addDaysSection(365));
    dateEdit->setFormat("yyyy.MM.dd");
    \endcode

    Here we've created a new QDateTimeEdit object initialised with today's
    date and restricted the valid date range to today plus or minus
    365 days. We've set the order to month, day, year. If the auto
    advance property is true (as we've set it here) when the user
    completes a section of the date, e.g. enters two digits for the
    month, they are automatically taken to the next section.

    The maximum and minimum values for a date value in the date editor
    default to the maximum and minimum values for a QDate. You can
    change this by calling setMinimumDate(), setMaximumDate(),
    setMinimumTime() and setMaximumTime()
*/

/*!
    Constructs an empty date time editor which is a child of \a
    parent. The format is set to "yyyy.MM.dd_hh:mm.ss"
*/

QDateTimeEdit::QDateTimeEdit(QWidget *parent, WFlags f)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent, f)
{
    d->minimum = QCoreVariant(DATETIME_MIN);
    d->maximum = QCoreVariant(DATETIME_MAX);
    d->value = d->minimum;
    if (!setFormat("yyyy.MM.dd_hh:mm.ss"))
	qFatal("Could not parse format 'yyyy.MM.dd_hh:mm.ss'");
}

/*!
    Constructs an empty date time editor which is a child of \a
    parent. The value is set to \a datetime. The format is set to
    "yyyy.MM.dd_hh:mm.ss"
*/

QDateTimeEdit::QDateTimeEdit(const QDateTime &datetime, QWidget *parent, WFlags f)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent, f)
{
    d->minimum = QCoreVariant(DATETIME_MIN);
    d->maximum = QCoreVariant(DATETIME_MAX);
    d->value = datetime.isValid() ? QCoreVariant(datetime) : d->getZeroVariant();
    if (!setFormat("yyyy.MM.dd_hh:mm.ss")) // ### should I default to yyyy.MM.dd if datetime.time().isNull()?
	qFatal("Could not parse format 'yyyy.MM.dd_hh:mm.ss'");
}

/*!
    Constructs an empty date time editor which is a child of \a
    parent. The value is set to \a time. The format is set to "hh:mm.ss".
*/

QDateTimeEdit::QDateTimeEdit(const QTime &time, QWidget *parent, WFlags f)
    : QAbstractSpinBox(*new QDateTimeEditPrivate, parent, f)
{
    d->minimum = QCoreVariant(DATETIME_MIN);
    d->maximum = QCoreVariant(DATETIME_MAX);
    d->value = QCoreVariant(QDateTime(QDate(2000, 01, 01), time.isValid() ? time : QTime())); // ### hack
    if (!setFormat("hh:mm.ss"))
	qFatal("Could not parse format 'hh:mm.ss'");
}

/*!
    \property QDateTimeEdit::dateTime
    \brief the QDateTime that is set in the QDateTimeEdit

    \sa setDateTime()
*/

QDateTime QDateTimeEdit::dateTime() const
{
    return d->value.toDateTime();
}

void QDateTimeEdit::setDateTime(const QDateTime &datetime)
{
    if (datetime.isValid())
        d->setValue(QCoreVariant(datetime), EmitIfChanged);
}

/*!
    \property QDateTimeEdit::date
    \brief the QDate that is set in the QDateTimeEdit

    \sa setDate()
*/

QDate QDateTimeEdit::date() const
{
    return d->value.toDate();
}

void QDateTimeEdit::setDate(const QDate &date)
{
    if (date.isValid())
        d->setValue(QCoreVariant(QDateTime(date, d->value.toTime())), EmitIfChanged);
}

/*!
    \property QDateTimeEdit::time
    \brief the QTime that is set in the QDateTimeEdit

    \sa setTime()
*/

QTime QDateTimeEdit::time() const
{
    return d->value.toTime();
}

void QDateTimeEdit::setTime(const QTime &time)
{
    if (time.isValid())
        d->setValue(QCoreVariant(QDateTime(d->value.toDate(), time)), EmitIfChanged);
}

/*!
    \property QDateTimeEdit::dateTime
    \brief the QDateTime that is set in the QDateTimeEdit

    \sa setDateTime()
*/

/*!
    \property QDateTimeEdit::minimumDate

    \brief the minimum date of the date time edit

    When setting this property the \l QDateTimeEdit::maximumDate is
    adjusted if necessary, to ensure that the range remains valid. If \a min is
    not a valid QDate object this function does nothing.

    The default minimum value can be restored with clearMinimum()

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(), setMaximumTime(),
    clearMinimum(), setTimeRange(), setDateRange(), QDate::isValid()
*/

QDate QDateTimeEdit::minimumDate() const
{
    return d->minimum.toDate();
}

void QDateTimeEdit::setMinimumDate(const QDate &min)
{
    if (min.isValid())
        d->setBoundary(Minimum, QCoreVariant(QDateTime(min, d->minimum.toTime())));
}

void QDateTimeEdit::clearMinimumDate()
{
    d->setBoundary(Minimum, QCoreVariant(QDateTime(DATE_MIN, d->minimum.toTime()))); // something to prevent this from being invalid by default?
}

/*!
    \property QDateTimeEdit::maximumDate

    \brief the maximum date of the date time edit

    When setting this property the \l QDateTimeEdit::minimumDate is
    adjusted if necessary to ensure that the range remains valid. If \a max is
    not a valid QDate object this function does nothing.

    The default minimum value can be restored with clearMinimumDate()

    \sa setMinimumDate(), maximumDate(), setMaximumDate(), clearMinimumDate(),
    setMinimumTime(), maximumTime(), setMaximumTime(), clearMinimumTime(),
    setTimeRange(), setDateRange(), QDate::isValid()
*/

QDate QDateTimeEdit::maximumDate() const
{
    return d->maximum.toDate();
}

void QDateTimeEdit::setMaximumDate(const QDate &max)
{
    if (max.isValid())
        d->setBoundary(Maximum, QCoreVariant(QDateTime(max, d->minimum.toTime())));
}

void QDateTimeEdit::clearMaximumDate()
{
    d->setBoundary(Maximum, QCoreVariant(QDateTime(DATE_MAX, d->maximum.toTime())));
}

/*!
    \property QDateTimeEdit::minimumTime

    \brief the minimum time of the date time edit

    When setting this property the \l QDateTimeEdit::maximumTime is
    adjusted if necessary, to ensure that the range remains valid. If \a min is
    not a valid QTime object this function does nothing.

    The default minimum value can be restored with clearMinimumTime()

    \sa setMinimumTime(), maximumTime(), setMaximumTime(),
    clearMinimumTime(), setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setTimeRange(), setDateRange(), QTime::isValid()
*/

QTime QDateTimeEdit::minimumTime() const
{
    return d->minimum.toTime();
}

void QDateTimeEdit::setMinimumTime(const QTime &min)
{
    if (min.isValid())
        d->setBoundary(Minimum, QCoreVariant(QDateTime(d->minimum.toDate(), min)));
}

void QDateTimeEdit::clearMinimumTime()
{
    d->setBoundary(Minimum, QCoreVariant(QDateTime(d->minimum.toDate(), TIME_MIN)));
}

/*!
    \property QDateTimeEdit::maximumTime

    \brief the maximum time of the date time edit

    When setting this property the \l QDateTimeEdit::maximumTime is adjusted if
    necessary to ensure that the range remains valid. If \a max is not a valid
    QTime object this function does nothing.

    The default minimum value can be restored with clearMinimumDate()

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), setTimeRange(),
    setDateRange(), QTime::isValid()
*/

QTime QDateTimeEdit::maximumTime() const
{
    return d->maximum.toTime();
}

void QDateTimeEdit::setMaximumTime(const QTime &max)
{
    if (max.isValid())
        d->setBoundary(Maximum, QCoreVariant(QDateTime(d->maximum.toDate(), max)));
}

void QDateTimeEdit::clearMaximumTime()
{
    d->setBoundary(Maximum, QCoreVariant(QDateTime(d->maximum.toDate(), TIME_MAX)));
}

/*!
    Convenience function to set minimum and maximum date with one
    function call.

    setDateRange(min, max);

       is analogous to:

    setMinimumDate(min);
    setMaximumDate(max);

    If either \a min or \a max are not valid this function does nothing.

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), QDate::isValid()
*/

void QDateTimeEdit::setDateRange(const QDate &min, const QDate &max)
{
    if (min.isValid() && max.isValid()) {
        d->setBoundary(Minimum, QCoreVariant(QDateTime(qMin(min, max), d->minimum.toTime())));
        d->setBoundary(Maximum, QCoreVariant(QDateTime(qMax(min, max), d->maximum.toTime())));
    }
}

/*!
    Convenience function to set minimum and maximum time with one
    function call.

    setTimeRange(min, max);

       is analogous to:

    setMinimumTime(min);
    setMaximumTime(max);

    If either \a min or \a max are not valid this function does nothing.

    \sa setMinimumDate(), maximumDate(), setMaximumDate(),
    clearMinimumDate(), setMinimumTime(), maximumTime(),
    setMaximumTime(), clearMinimumTime(), QTime::isValid()
*/

void QDateTimeEdit::setTimeRange(const QTime &min, const QTime &max)
{
    if (min.isValid() && max.isValid()) {
        d->setBoundary(Minimum, QCoreVariant(QDateTime(d->maximum.toDate(), qMin(min, max))));
        d->setBoundary(Maximum, QCoreVariant(QDateTime(d->maximum.toDate(), qMax(min, max))));
    }
}

/*!
    \property QDateTimeEdit::display

    \brief the currently displayed fields of the date time edit.

    Returns a bit set of the displayed sections for this format.
    \a setFormat(), format()
*/

QDateTimeEdit::Section QDateTimeEdit::display() const
{
    return (Section)d->display;
}

/*!
    \property QDateTimeEdit::currentSection

    \brief the current section of the spinbox.
    \a setCurrentSection()
*/

QDateTimeEdit::SectionFlags QDateTimeEdit::currentSection() const
{
    return (SectionFlags)(d->currentsection & (~QDateTimeEditPrivate::Internal));
}

void QDateTimeEdit::setCurrentSection(SectionFlags section)
{
    const QDateTimeEditPrivate::SectionFlags s = (QDateTimeEditPrivate::SectionFlags)section;
    switch (s) {
    case QDateTimeEditPrivate::FirstSection: d->edit->setCursorPosition(0); break;
    case QDateTimeEditPrivate::LastSection: d->edit->setCursorPosition(d->edit->text().size()); break;
    case QDateTimeEditPrivate::NoSection: break;
    default: d->edit->setCursorPosition(d->sectionNode((QDateTimeEditPrivate::SectionFlags)section).pos);
    }
}

/*!
    Returns a the text of \a section

    \a text(), cleanText(), currentSection()
*/

QString QDateTimeEdit::sectionText(SectionFlags s) const
{
    return d->sectionText(d->edit->displayText(), (QDateTimeEditPrivate::SectionFlags)s);
}

/*!
    \property QDateTimeEdit::format

    \brief the format used to display the time/date of the date time edit.

    This format is a subset of the format described in QDateTime::toString()

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i hh
         \i the hour with a leading zero (00..23 or 01..12 if AM/PM display)
    \row \i mm \i the minute with a leading zero (00..59)
    \row \i ss \i the second whith a leading zero (00..59)
    \row \i zzz \i the milliseconds with leading zeroes (000..999)
    \row \i AP
         \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
         \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \row \i dd \i the day as number with a leading zero (01-31)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
         \i the abbreviated localized month name (e.g. 'Jan'..'Dec').
            Uses QDate::shortMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number (1752-8000)
    \endtable

    All other input characters will be ignored and can be used as delimiters.

    Example format strings (assuming that the QDate is the
    20<sup><small>th</small></sup> July 1969):
    \table
    \header \i Format \i Result
    \row \i dd.MM.yyyy    \i 20.07.1969
    \row \i ddd MMMM d yy \i Sun July 20 69
    \endtable

    If you specify an invalid format the format will not be set.

    Multiple instances of the same field is not allowed. E.g.

    setFormat("yyyy.MM.yy"); // not allowed

    a format with no valid fields is not allowed either. E.g.

    setFormat("s.M.y"); // not allowed

    \sa QDateTime::toString(), setFormat(), display()
*/

QString QDateTimeEdit::format() const
{
    return d->format;
}

// ### Think of something clever for locale/default.
bool QDateTimeEdit::setFormat(const QString &format)
{
    if (d->parseFormat(format)) {
        d->sizehintdirty = true;
	d->update();
        d->currentsection = (d->sections.front().pos > 0 ? QDateTimeEditPrivate::FirstSection : d->sections.front().section);
        d->edit->setCursorPosition(0);
	return true;
    }
    return false;
}

/*!
    This virtual function is used by the date time edit whenever it
    needs to display date \a v.

    If you reimplement this, you may also need to reimplement
    mapTextToValue().

    \sa mapTextToValue()
*/

QString QDateTimeEdit::mapDateTimeToText(const QDateTime &date) const
{
    return date.isValid() ? d->toString(QCoreVariant(date)) : QString();
}

/*!
    This virtual function is used by the date time edit whenever it
    needs to interpret text entered by the user as a value.

    \sa mapDateTimeToText()
*/

QDateTime QDateTimeEdit::mapTextToDateTime(QString *txt, QValidator::State *state) const
{
    const QDateTime dt = d->fromString(txt, state).toDateTime();
    if (state && *state == QValidator::Acceptable && (dt < d->minimum.toDateTime() || dt > d->maximum.toDateTime())) {
        *state = QValidator::Invalid;
    }
    return dt;
}

/*
    !\reimp
*/

void QDateTimeEdit::keyPressEvent(QKeyEvent *e)
{
//    const QDateTimeEditPrivate::SectionFlags s = d->currentsection;
    bool select = true;
    bool fixcursor = !e->text().isEmpty();
    if ((e->key() == Qt::Key_Backspace || (e->key() == Qt::Key_H && e->key() & Qt::ControlButton)) && !d->edit->hasSelectedText()) {
        const QDateTimeEditPrivate::SectionNode &sn = d->sectionNode(d->currentsection);
        const int cur = d->edit->cursorPosition();
        if (cur == sn.pos && cur > d->sections.front().pos) {
//            const QDateTimeEditPrivate::SectionNode &s = d->nextPrevSection(sn.section, false);
//            d->edit->setCursorPosition(s.pos + d->sectionLength(s.section));
            e->accept();
            return;
        }
    }
    switch((Qt::Key)e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        d->refresh(AlwaysEmit);
        d->setSelected(d->currentsection);
        return;

    case Qt::Key_Left:
    case Qt::Key_Right:
        if (!(e->state() & Qt::ControlButton)) {
            int sellength = d->edit->selectedText().size();
            if (sellength == 0 || sellength != d->sectionLength(d->currentsection))
                break;
            select = false;
        }
    case Qt::Key_Backtab:
    case Qt::Key_Tab: {
        const QDateTimeEditPrivate::SectionNode newSection =
            d->nextPrevSection(d->currentsection,
                               (e->key() == Qt::Key_Right || (e->key() == Qt::Key_Tab && !(e->state() & Qt::ShiftButton))));
        switch (newSection.section) {
        case QDateTimeEditPrivate::NoSection: case QDateTimeEditPrivate::FirstSection: case QDateTimeEditPrivate::LastSection: break;
        default:
            if (select) {
                d->setSelected(newSection.section); break;
            } else {
                d->edit->setCursorPosition(e->key() == Qt::Key_Right
                                           ? newSection.pos
                                           : d->sectionPos(d->currentsection));
            }
        }
        if (!select)
            d->edit->deselect();
        e->accept();
        return; }
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Prior:
    case Qt::Key_Next:
        fixcursor = true;
    default:
        if (fixcursor && d->currentsection == QDateTimeEditPrivate::FirstSection) {
            setCurrentSection((SectionFlags)d->sections.front().section);
        }
        break;
    }

    QAbstractSpinBox::keyPressEvent(e);
}

/*
    !\reimp
*/

void QDateTimeEdit::wheelEvent(QWheelEvent *e)
{
    const QDateTimeEditPrivate::SectionFlags s = d->sectionAt(qMax(0, d->edit->cursorPositionAt(e->pos()) - 1));
    // ### cursorPositionAt seems to give me one to many characters
    if (s != d->currentsection)
        d->edit->setCursorPosition(d->sectionNode(s).pos);
    switch (s) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        QAbstractSpinBox::wheelEvent(e);
        break;
    }
}

/*
    !\reimp
*/

void QDateTimeEdit::focusInEvent(QFocusEvent *e)
{
    QAbstractSpinBox::focusInEvent(e);
    QDateTimeEditPrivate::SectionFlags s;
    switch(QFocusEvent::reason()) {
    case QFocusEvent::Shortcut:
    case QFocusEvent::Tab: s = d->sections.front().section; break;
    case QFocusEvent::Backtab: s = d->sections.at(d->sections.size() - 1).section; break;
    default: return;
    }

    d->setSelected(s);
}

/*
    !\reimp
*/

bool QDateTimeEdit::focusNextPrevChild(bool next)
{
    const QDateTimeEditPrivate::SectionFlags newSection = d->nextPrevSection(d->currentsection, next).section;
    switch (newSection) {
    case QDateTimeEditPrivate::NoSection:
    case QDateTimeEditPrivate::FirstSection:
    case QDateTimeEditPrivate::LastSection:
        break;
    default:
        if (newSection != QDateTimeEditPrivate::NoSection) {
            d->setSelected(newSection);
            return true;
        }
    }
    return QAbstractSpinBox::focusNextPrevChild(next);
}

/*
    !\reimp
*/

void QDateTimeEdit::stepBy(int steps)
{
    const QDateTimeEditPrivate::SectionFlags s = d->currentsection;
    d->setValue(d->stepBy(s, steps, false), EmitIfChanged);
    d->setSelected(s);
}

/*
    !\reimp
*/

QDateTimeEdit::StepEnabled QDateTimeEdit::stepEnabled() const
{
    switch (d->currentsection) {
    case QDateTimeEditPrivate::NoSection: case QDateTimeEditPrivate::FirstSection: case QDateTimeEditPrivate::LastSection: return 0;
    default: break;
    }
    if (d->wrapping)
        return (StepEnabled)(StepUpEnabled | StepDownEnabled);

    QAbstractSpinBox::StepEnabled ret = 0;

    QCoreVariant v = d->stepBy(d->currentsection, 1, true);
    if (v != d->value) {
	ret |= QAbstractSpinBox::StepUpEnabled;
    }
    v = d->stepBy(d->currentsection, -1, true);
    if (v != d->value) {
	ret |= QAbstractSpinBox::StepDownEnabled;
    }

    return ret;
}

// --- QDateTimeEditPrivate ---

/*!
    \internal
    Constructs a QDateTimeEditPrivate object
*/

QDateTimeEditPrivate::QDateTimeEditPrivate()
{
    type = QCoreVariant::DateTime;
    display = (QDateTimeEdit::Section)0;
    cachedday = -1;
    currentsection = oldsection = NoSection;
    first.section = FirstSection;
    first.pos = 0;
    last.section = LastSection;
    last.pos = -1;
    useprivate = true;
}

/*!
    \internal
    \reimp
*/

void QDateTimeEditPrivate::emitSignals()
{
    if (value.toDate().isValid()) {
        emit q->dateTimeChanged(value.toDateTime());
        if (display & DateMask)
            emit q->dateChanged(value.toDate());
    }

    if (display & TimeMask)
        emit q->timeChanged(value.toTime());
}

/*!
    \internal
    \reimp
*/

QString QDateTimeEditPrivate::mapValueToText(const QCoreVariant &f) const
{
    return q->mapDateTimeToText(f.toDateTime());
}

/*!
    \internal
    \reimp
*/

QCoreVariant QDateTimeEditPrivate::mapTextToValue(QString *text, QValidator::State *state) const
{
    return QCoreVariant(q->mapTextToDateTime(text, state));
}

/*!
    \internal
    \reimp
*/

void QDateTimeEditPrivate::editorCursorPositionChanged(int oldpos, int newpos)
{
    if (ignorecursorpositionchanged)
        return;
    ignorecursorpositionchanged = true;
    SectionFlags s = sectionAt(newpos);
//    SectionFlags old = oldsection;
    oldsection = sectionAt(oldpos);
    int c = newpos;

    if (!d->dragging) {
        const int selstart = d->edit->selectionStart();
        const SectionFlags selSection = sectionAt(selstart);
        const int l = sectionLength(selSection);

        if (s == NoSection) {
            if (l > 0 && selstart == sectionPos(selSection) && d->edit->selectedText().length() == l) {
                s = selSection;
                setSelected(selSection, true);
                c = -1;
            } else {
                const SectionNode &sn = sectionNode(closestSection(newpos, oldpos < newpos));
                c = sn.pos + (oldpos < newpos ? 0 : qMax(0, sectionLength(sn.section) - 1));
                edit->setCursorPosition(c);
                s = sn.section;
            }
        }
    }
//     qDebug("(%s)currentsection = %s (%s)oldsection = %s",
//            sectionName(currentsection).latin1(),
//            sectionName(s).latin1(),
//            sectionName(old).latin1(),
//            sectionName(oldsection).latin1());
    if (currentsection != s) {
        QString tmp = edit->displayText();
        QCoreVariant v = getZeroVariant();
        int pos = d->edit->cursorPosition();
        if (validate(&tmp, &pos, &v) != QValidator::Acceptable) {
            refresh(EmitIfChanged);
            if (c == -1) {
                setSelected(s, true);
            } else {
                edit->setCursorPosition(c);
            }
        }
    }
    currentsection = s;
    ignorecursorpositionchanged = false;

}

/*!
    \internal
    Gets the digit from a variant. E.g.

    QVariant var(QDate(2004, 02, 02));
    int digit = getDigit(var, Year);
    // digit = 2004
*/

int QDateTimeEditPrivate::getDigit(const QCoreVariant &t, SectionFlags s) const
{
    switch(s) {
    case HoursSection: {
        int h = t.toTime().hour();
        if (display & AMPMSection) {
            h = h % 12;
            return h == 0 ? 12 : h;
        } else {
            return t.toTime().hour();
        }
    }
    case MinutesSection: return t.toTime().minute();
    case SecondsSection: return t.toTime().second();
    case MSecsSection: return t.toTime().msec();
    case YearsSection: return t.toDate().year();
    case MonthsShortNameSection:
    case MonthsSection: return t.toDate().month();
    case DaysSection: return t.toDate().day();
    default: break;
    }
    qFatal("%s passed to getDigit. This should never happen", sectionName(s).latin1());
    return -1;
}

/*!
    \internal
    Sets a digit in a variant. E.g.

    QVariant var(QDate(2004, 02, 02));
    int digit = getDigit(var, Year);
    // digit = 2004
    setDigit(&var, Year, 2005);
    digit = getDigit(var, Year);
    // digit = 2005
*/

void QDateTimeEditPrivate::setDigit(QCoreVariant *v, SectionFlags section, int newVal) const
{
    int year, month, day, hour, minute, second, msec;
    const QDateTime &dt = v->toDateTime();
    year = dt.date().year();
    month = dt.date().month();
    day = dt.date().day();
    hour = dt.time().hour();
    minute = dt.time().minute();
    second = dt.time().second();
    msec = dt.time().msec();

    switch(section) {
    case HoursSection: hour = newVal; break;
    case MinutesSection: minute = newVal; break;
    case SecondsSection: second = newVal; break;
    case MSecsSection: msec = newVal; break;
    case YearsSection: year = newVal; break;
    case MonthsSection:
    case MonthsShortNameSection: month = newVal; break;
    case DaysSection: day = newVal; break;
    case AMPMSection:
    case AMPMLowerCaseSection: hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12); break;
    default:
        qFatal("%s passed to setDigit. This should never happen", sectionName(section).latin1());
        break;
    }

    if (section != DaysSection)
	day = qMax(cachedday, day);

    if (!QDate::isValid(year, month, day)) {
	if (year <= DATE_MIN.year() && (month < DATE_MIN.month() || (month == DATE_MIN.month() && day < DATE_MIN.day()))) {
	    month = DATE_MIN.month();
	    day = DATE_MIN.day();
	} else {
	    day = qMin(day, QDate(year, month, 1).daysInMonth());
	}
    }

    if (section == DaysSection)
	cachedday = day;

    *v = QCoreVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
}


/*!
    \internal

    Internal function called by QDateTimeEdit::stepBy(). Also takes a
    SectionFlags for which section to step on and a bool \a test for
    whether or not to modify the internal cachedday variable. This is
    necessary because the function is called from the const function
    QDateTimeEdit::stepEnabled() as well as QDateTimeEdit::stepBy().
*/

QCoreVariant QDateTimeEditPrivate::stepBy(SectionFlags s, int steps, bool test) const
{
    QCoreVariant v = value;
    QString str = edit->displayText();

    int val;
    // to make sure it behaves reasonably when typing something and then stepping in non-tracking mode
    if (!test && pendingemit) {
        if (validate(&str, 0, &v) != QValidator::Acceptable) {
            v = value;
        }
        val = getDigit(v, s);
    } else {
        QValidator::State state;
        val = sectionValue(s, &str, &state);
        if (state == QValidator::Invalid) {
            return value;
        }
    }

    if (s == HoursSection && display & AMPMSection) {
	if (val % 12 == 0 && steps < 0) {
	    val = wrapping ? val + 12 + steps : 12;
	} else if (val % 12 == 11 && steps > 0) {
	    val = wrapping ? val - 12 + steps : val;
	} else {
	    val += steps;
	}
    } else {
	val += steps;
    }
    const int min = absoluteMin(s);
    const int max = absoluteMax(s);

    if (val < min) {
	val = (wrapping ? max : min);
    } else if (val > max) {
	val = (wrapping ? min : max);
    }

    int tmp = v.toDate().day();
    if (!test && s == DaysSection)
	cachedday = val;

    setDigit(&v, s, val); // if this sets year or month it will make sure that days is lowered if needed.
    if (!test && tmp != v.toDate().day()) // this should not happen when called from stepEnabled
	cachedday = tmp;

    return bound(v, value, steps);
}


/*!
    \internal

    Returns the absolute maximum for a section
*/

inline int QDateTimeEditPrivate::absoluteMax(SectionFlags s) const
{
    switch(s) {
    case HoursSection: return (display & AMPMSection ? 12 : 23);
    case MinutesSection:
    case SecondsSection: return 59;
    case MSecsSection: return 999;
    case YearsTwoDigitsSection: return 99;
    case YearsSection: return 7999;
    case MonthsSection:
    case MonthsShortNameSection: return 12;
    case DaysSection: return 31;
    case AMPMSection:
    case AMPMLowerCaseSection: return 1;
    default: break;
    }
    qFatal("%s passed to max. This should never happen", sectionName(s).latin1());
    return -1;

}

/*!
    \internal

    Returns the absolute minimum for a section
*/

inline int QDateTimeEditPrivate::absoluteMin(SectionFlags s) const
{
    switch(s) {
    case HoursSection: return (display & AMPMSection ? 1 : 0);
    case MinutesSection:
    case SecondsSection:
    case MSecsSection: return 0;
    case YearsTwoDigitsSection: return 0;
    case YearsSection: return 1752;
    case MonthsSection:
    case MonthsShortNameSection:
    case DaysSection: return 1;
    case AMPMSection:
    case AMPMLowerCaseSection: return 0;
    default: break;
    }
    qFatal("%s passed to min. This should never happen", sectionName(s).latin1());
    return -1;
}

/*!
    \internal

    Returns a copy of the sectionNode for the SectionFlags \a s.
*/

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::sectionNode(SectionFlags s) const
{
    if (s == FirstSection) {
        return first;
    } else if (s == LastSection) {
        return last;
    }

    for (int i=0; i<sections.size(); ++i)
	if (sections.at(i).section == s)
	    return sections.at(i);
    SectionNode sn;
    sn.section = NoSection;
    sn.pos = -1;
    return sn;
}

/*!
    \internal

    Returns the starting position for section \a s.
*/

int QDateTimeEditPrivate::sectionPos(SectionFlags s) const
{
    if (s == FirstSection) {
        return first.pos;
    } else if (s == LastSection) {
        return last.pos;
    }

    for (int i=0; i<sections.size(); ++i)
	if (sections.at(i).section == s)
	    return sections.at(i).pos;
    return -1;
}

/*!
    \internal

    Adds a section to \a list. If this section already exists returns an error. Used by parseFormat()
*/

QDateTimeEditPrivate::SectionFlags QDateTimeEditPrivate::addSection(QList<SectionNode> *list, SectionFlags ds, int pos)
{
    for (int i=0; i<list->size(); ++i) {
	if (list->at(i).section == ds)
	    return ds;
    }
    SectionNode s;
    s.section = ds;
    s.pos = pos;
    *list << s;
    return NoSection;
}


/*!
    \internal

    Selects the section \a s. If \a forward is false selects backwards.
*/

void QDateTimeEditPrivate::setSelected(SectionFlags s, bool forward)
{
    switch (s) {
    case NoSection:
    case LastSection:
    case FirstSection:
        return;
    default: break;
    }
    if (forward) {
        edit->setSelection(d->sectionPos(s), d->sectionLength(s));
    } else {
        edit->setSelection(d->sectionPos(s) + d->sectionLength(s), -d->sectionLength(s));
    }
}

/*!
    \internal

    Parses the format \a newFormat. If successful, returns true and
    sets up the format. Else keeps the old format and returns false.
*/

bool QDateTimeEditPrivate::parseFormat(const QString &newFormat) // ### I do not escape yet
{
    SectionFlags error = NoSection;
    QList<SectionNode> list;
    QDateTimeEdit::Section newDisplay = 0;
    QStringList newSeparators;
    int i, index = 0;
    for (i = 0; error == NoSection && i<(int)newFormat.length(); ++i) {
	if (i + 1 < newFormat.length()) {
	    switch (newFormat.at(i).cell()) {
	    case 'h':
		if (newFormat.at(i+1) == 'h') {
                    error = addSection(&list, HoursSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::HoursSection;
		    continue;
		}
		break;
	    case 'm':
		if (newFormat.at(i+1) == 'm') {
                    error = addSection(&list, MinutesSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::MinutesSection;
		    continue;
		}
		break;
	    case 's':
		if (newFormat.at(i+1) == 's') {
                    error = addSection(&list, SecondsSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::SecondsSection;
		    continue;
		}
		break;
	    case 'z':
		if (i + 2 <(int)newFormat.length() && newFormat.at(i+1) == 'z' && newFormat.at(i+2) == 'z') {
		    error = addSection(&list, MSecsSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = (i += 2) + 1;
		    newDisplay |= QDateTimeEdit::MSecsSection;
		    continue;
		}
		break;
	    case 'A':
	    case 'a': {
		const bool cap = newFormat.at(i).latin1() == 'A';
		if (newFormat.at(i+1) == (cap ? 'P' : 'p')) {
		    error = addSection(&list, cap ? AMPMSection : AMPMLowerCaseSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::AMPMSection;
		    continue;
		}
		break; }
	    case 'y':
		if (newFormat.at(i+1) == 'y') {
		    const bool four = (i + 3 <(int)newFormat.length() && newFormat.at(i+2) == 'y' && newFormat.at(i+3) == 'y');
		    error = addSection(&list, four ? YearsSection : YearsTwoDigitsSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = (i += (four ? 3 : 1)) + 1;
		    newDisplay |= QDateTimeEdit::YearsSection;
		    continue;
		}
		break;
	    case 'M':
		if (newFormat.at(i+1) == 'M') {
		    const bool three = (i + 2 <(int)newFormat.length() && newFormat.at(i+2) == 'M');
		    error = addSection(&list, three ? MonthsShortNameSection : MonthsSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = (i += (three ? 2 : 1)) + 1;
		    newDisplay |= QDateTimeEdit::MonthsSection;
		    continue;
		}
		break;

	    case 'd':
		if (newFormat.at(i+1) == 'd') {
		    error = addSection(&list, DaysSection, i);
                    newSeparators << newFormat.mid(index, i - index);
                    index = ++i + 1;
		    newDisplay |= QDateTimeEdit::DaysSection;
		    continue;
		}
		break;
	    default: break;
	    }
	}
    }
    if (error != NoSection) {
	qWarning("Could not parse format. '%s' occurs twice in '%s'.",
		 sectionName(error).latin1(), newFormat.latin1());
	return false;
    } else if (list.isEmpty()) {
	qWarning("Could not parse format. No sections in format '%s'.",
		 newFormat.latin1());
	return false;
    }


    newSeparators << (index < newFormat.size() ? newFormat.mid(index) : QString());

    format = newFormat;
    separators = newSeparators;

    display = newDisplay;
    last.pos = newFormat.size();

    sections = list;
    return true;
}

/*!
    \internal

    Returns the section at index \a index or NoSection if there are no sections there.
*/

QDateTimeEditPrivate::SectionFlags QDateTimeEditPrivate::sectionAt(int index) const
{
    if (index < separators.front().size()) {
        return (index == 0 ? FirstSection : NoSection);
    } else if (format.size() - index < separators.back().size() + 1) {
        return (index == last.pos ? LastSection : NoSection);
    }
    for (int i=0; i<sections.size(); ++i) {
	int tmp = sections.at(i).pos;
        if (index < tmp + sectionLength(sections.at(i).section)) {
            return (index < tmp ? NoSection : sections.at(i).section);
        }
    }
    qWarning("%d index return NoSection. This should not happen", index);
    return NoSection;
}

/*!
    \internal

    Returns the closest section of index \a index. Searches forward
    for a section if \a forward is true. Otherwise searches backwards.
*/

QDateTimeEditPrivate::SectionFlags QDateTimeEditPrivate::closestSection(int index, bool forward) const
{
    if (index < separators.front().size()) {
        return forward ? sections.front().section : FirstSection;
    } else if (last.pos - index < separators.back().size() + 1) {
        return forward ? LastSection : sections.back().section;
    }
    for (int i=0; i<sections.size(); ++i) {
	int tmp = sections.at(i).pos;
        if (index < tmp + sectionLength(sections.at(i).section)) {
            if (index < tmp && !forward)
                return sections.at(i-1).section;
            return sections.at(i).section;
        } else if (i == sections.size() - 1 && index > tmp) {
            return sections.at(i).section;
        }
    }
    qWarning("2index return NoSection. This should not happen");
    return NoSection;
}

/*!
    \internal

    Returns a copy of the section that is before or after \a current, depending on \a forward.
*/

QDateTimeEditPrivate::SectionNode QDateTimeEditPrivate::nextPrevSection(SectionFlags current, bool forward) const
{
    if (current == FirstSection) {
        return (forward ? sections.front() : first);
    } else if (current == LastSection) {
        return (forward ? last : sections.back());
    }
    for (int i=0; i<sections.size(); ++i) {
	if (sections.at(i).section == current) {
	    int index = i + (forward ? 1 : -1);
	    if (index >= 0 && index < sections.size()) {
		return sections.at(index);
	    } else {
		break;
	    }
	}
    }
    return (forward ? last : first);
}

/*!
    \internal

    Clears the text of section \a s.
*/


void QDateTimeEditPrivate::clearSection(SectionFlags s)
{
    int cursorPos = d->edit->cursorPosition();
    bool blocked = d->edit->blockSignals(true);
    QString t = d->edit->text();
    t.replace(sectionPos(s), sectionLength(s), QString().fill(' ', sectionLength(s)));
    d->edit->setText(t);
    d->edit->setCursorPosition(cursorPos);
    d->edit->blockSignals(blocked);
}

/*!
    \internal

    Returns the length of section \a s.
*/

inline int QDateTimeEditPrivate::sectionLength(SectionFlags s) const
{
    switch(s) {
    case FirstSection:
    case NoSection:
    case LastSection: return 0;

    case HoursSection:
    case MinutesSection:
    case SecondsSection:
    case AMPMSection:
    case AMPMLowerCaseSection:
    case DaysSection:
    case MonthsSection:
    case YearsTwoDigitsSection: return 2;

    case MonthsShortNameSection:
    case MSecsSection: return 3;

    case YearsSection: return 4;

    case Internal:
    case TimeMask:
    case DateMask: qWarning("Invalid section %s", sectionName(s).latin1());
    }
    return -1;
}

/*!
    \internal

    Returns the text of section \a s. This function operates on the
    arg text rather than edit->text().
*/


QString QDateTimeEditPrivate::sectionText(const QString &text, SectionFlags s) const
{
    const SectionNode sn = sectionNode(s);
    return sn.section == NoSection ? QString() : text.mid(sn.pos, sectionLength(s));
}

/*!
    \internal

    Returns the string representation of \a var according to the current format.
*/

QString QDateTimeEditPrivate::toString(const QCoreVariant &var) const
{
    QString ret = format;
    for (int i=0; i<sections.size(); ++i) {
	int l = sectionLength(sections.at(i).section);
	ret.remove(sections.at(i).pos, l);
	if (sections.at(i).section == AMPMSection || sections.at(i).section == AMPMLowerCaseSection) {
	    QString input = var.toTime().hour() > 11 ? "pm" : "am"; // ### might be wrong
	    ret.insert(sections.at(i).pos, sections.at(i).section == AMPMSection ? input.toUpper() : input);
	} else if (sections.at(i).section == MonthsShortNameSection) {
	    ret.insert(sections.at(i).pos, QDate::shortMonthName(var.toDate().month()));
	} else {
	    ret.insert(sections.at(i).pos, QString::number(getDigit(var, sections.at(i).section)).rightJustified(l, '0'));
	}
    }
    return ret;
}

/*!
    \internal

    Returns the value of \a text parsed according to the current
    format. If stateptr is not 0 also sets it to the correct state.
*/

QCoreVariant QDateTimeEditPrivate::fromString(QString *text, QValidator::State *stateptr) const
{
    QCoreVariant ret = getZeroVariant();
    QValidator::State state = QValidator::Acceptable;
    int year, month, day, hour, min, sec, msec;
    year = month = day = hour = min = sec = msec = 0;
    for (int i=0; state != QValidator::Invalid && i<sections.size(); ++i) {
	const SectionFlags s = sections.at(i).section;
	QValidator::State tmpstate;
        int num = sectionValue(s, text, &tmpstate);
	state = qMin(state, tmpstate);
        if (state == QValidator::Acceptable) {
            setDigit(&ret, s, num);
        }
    }

    if (oldsection != DaysSection && day < cachedday)
	day = cachedday;

    if (state == QValidator::Acceptable && (display & DateMask) && !QDate::isValid(year, month, day)) {
        if (year <= DATE_MIN.year() && month < DATE_MIN.month() || (month == DATE_MIN.month() && day < DATE_MIN.day())) {
	    month = DATE_MIN.month();
	    day = DATE_MIN.day();
	} else {
	    day = qMin(day, QDate(year, month, 1).daysInMonth());
	}
    }

    if (oldsection == DaysSection)
	cachedday = day;

    if (stateptr)
	*stateptr = state;

//    qDebug("fromString: '%s' => '%s' (%s)", text->latin1(), ret.toString().latin1(), stateName(state).latin1());

    return ret;
}

/*!
    \internal

    Parses the part of \a text that corresponds to \a s and returns
    the value of that field. Sets *stateptr to the right state if
    stateptr != 0.
*/

inline int QDateTimeEditPrivate::sectionValue(SectionFlags s, QString *text, QValidator::State *stateptr) const
{
    QValidator::State state = QValidator::Invalid;
    int num = 0;
    QString st = sectionText(*text, s);
    if (st.trimmed().isEmpty()) {
        state = QValidator::Intermediate;
    } else {
        const int index = sectionNode(s).pos;
        const int length = sectionLength(s);
        const bool done = !st.contains(' ');
        switch(s) {
        case AMPMSection:
        case AMPMLowerCaseSection: {
            st = st.toLower();
            text->replace(index, length, (s == AMPMSection ? text->mid(index, length).toUpper() : text->mid(index, length).toLower()));
            if (done) {
                num = (st == "am" ? 0 : (st == "pm" ? 1 : -1));
                state = (num == -1 ? QValidator::Invalid : QValidator::Acceptable);
            } else {
                state = QValidator::Intermediate;
            }
            break;
        }
        case MonthsShortNameSection: {
            if (!done) {
                state = QValidator::Intermediate;
                for (int i=0; i<st.size(); ++i) {
                    if (!st.at(i).isLetter() && st.at(i) != ' ') {
                        state = QValidator::Invalid;
                        break;
                    }
                }
            } else {
                state = QValidator::Invalid;
                st = st.toLower();
                for (int j=1; j<=12; ++j) {
                    if (st == QDate::shortMonthName(j).toLower()) {
                        num = j;
                        state = QValidator::Acceptable;
                        st[0] = st.at(0).toUpper();
                        text->replace(index, length, st);
                        break;
                    }
                }
            }
            break;
        }
        case YearsTwoDigitsSection: num = 1900;
        case YearsSection:
        case MonthsSection:
        case HoursSection:
        case MinutesSection:
        case SecondsSection:
        case MSecsSection:
        case DaysSection: {
            bool ok;
            num += (int)(st.toUInt(&ok));
            if (!ok) {
                state = QValidator::Invalid;
            } else {
                const int min = absoluteMin(s);
                const int max = absoluteMax(s);
                if (num < min || num > max) {
                    state = done ? QValidator::Invalid : QValidator::Intermediate;
                } else {
                    state = QValidator::Acceptable;
                }
            }
            break;
        }
        default: qFatal("NoSection or Internal. This should never happen"); break; }
    }

    if (stateptr)
        *stateptr = state;

    return (state == QValidator::Acceptable ? num : -1);
}

/*!
    \internal
    \reimp
*/

QValidator::State QDateTimeEditPrivate::validate(QString *input, int *pos, QCoreVariant *val) const
{
    SectionNode sn;
    int diff = input->size() - format.size();
    if (diff > 0) {
        const SectionFlags s = (pos ? closestSection(*pos - 1, false) : currentsection);
        if (s == FirstSection && s == LastSection) {
//            qDebug("invalid because s == %s", sectionName(s).latin1());
            return QValidator::Invalid;
        }
        sn = sectionNode(s);
        const int sectionstart = sn.pos;
        const int sectionlength = sectionLength(s);

        QString sub = input->mid(sectionstart, sectionlength + diff);
        if (sub.count(' ') < diff) {
//            qDebug("sub is '%s' diff is %d sub.count is %d", sub.latin1(), diff, sub.count(' '));
            return QValidator::Invalid;
        }

        sub.remove(' ');
        input->replace(sectionstart, sectionlength + diff, sub.leftJustified(sectionlength, ' '));
    } else if (diff < 0) {
        const SectionFlags s = (pos ? closestSection(*pos, false) : currentsection);
        if (s == FirstSection && s == LastSection) {
//            qDebug(".invalid because s == %s", sectionName(s).latin1());
            return QValidator::Invalid;
        }
        sn = sectionNode(s);
        const int sectionstart = sn.pos;
        const int sectionlength = sectionLength(s);

        QString sub = input->mid(sectionstart, sectionlength + diff);
        sub.remove(' ');
        input->replace(sectionstart, sectionlength + diff, sub.leftJustified(sectionlength, ' '));

        sn = sectionNode(currentsection);
    }
    int index = 0;

    for (int i=0; i<sections.size(); ++i) {
        sn = sections.at(i);
        if (input->mid(index, sn.pos - index) != separators.at(i)) {
//            qDebug("invalid because '%s' != '%s'", input->mid(index, sn.pos - index).latin1(), separators.at(i).latin1());
            return QValidator::Invalid;
        }
        index = sn.pos + sectionLength(sn.section);
    }

    if (sn.pos + sectionLength(sn.section) < input->size()
        && input->mid(sn.pos + sectionLength(sn.section)) != separators.back()) {
//         qDebug(".invalid because '%s' != '%s'",
//                input->mid(sn.pos + sectionLength(sn.section)).latin1(),
//                separators.back().latin1());
        return QValidator::Invalid;
    }

    QValidator::State state;
    if (val) {
        *val = mapTextToValue(input, &state);
    } else {
        mapTextToValue(input, &state);
    }
//    qDebug("'%s' => '%s' (%s)", input->latin1(), (!val ? "foo" : val->toString().latin1()), stateName(state).latin1());
    return state;
}

/*!
    \internal
    For debugging. Returns the name of the section \a s.
*/

QString QDateTimeEditPrivate::sectionName(int s)
{
    switch(s) {
    case QDateTimeEditPrivate::AMPMSection: return "AMPMSection";
    case QDateTimeEditPrivate::AMPMLowerCaseSection: return "AMPMLowerCaseSection";
    case QDateTimeEditPrivate::DaysSection: return "DaysSection";
    case QDateTimeEditPrivate::HoursSection: return "HoursSection";
    case QDateTimeEditPrivate::MSecsSection: return "MSecsSection";
    case QDateTimeEditPrivate::MinutesSection: return "MinutesSection";
    case QDateTimeEditPrivate::MonthsSection: return "MonthsSection";
    case QDateTimeEditPrivate::MonthsShortNameSection: return "MonthsShortNameSection";
    case QDateTimeEditPrivate::SecondsSection: return "SecondsSection";
    case QDateTimeEditPrivate::YearsSection: return "YearsSection";
    case QDateTimeEditPrivate::YearsTwoDigitsSection: return "YearsTwoDigitsSection";
    case QDateTimeEditPrivate::NoSection: return "NoSection";
    case QDateTimeEditPrivate::FirstSection: return "FirstSection";
    case QDateTimeEditPrivate::LastSection: return "LastSection";
    default: return "Unknown section " + QString::number(s);
    }
}

/*!
    \internal
    For debugging. Returns the name of the state \a s.
*/

QString QDateTimeEditPrivate::stateName(int s)
{
    switch(s) {
    case QValidator::Invalid: return "Invalid";
    case QValidator::Intermediate: return "Intermediate";
    case QValidator::Acceptable: return "Acceptable";
    default: return "Unknown state " + QString::number(s);
    }
}


#include "moc_qdatetimeedit.cpp"
