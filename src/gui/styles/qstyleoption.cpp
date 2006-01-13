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

#include "qstyleoption.h"
#include "qapplication.h"
#ifdef Q_WS_MAC
# include "private/qt_mac_p.h"
#endif
#ifndef QT_NO_DEBUG
#include <qdebug.h>
#endif

/*!
    \class QStyleOption
    \brief The QStyleOption class stores the parameters used by QStyle functions.

    \ingroup appearance

    QStyleOption and its subclasses contain all the information that
    QStyle functions need to draw a graphical element.

    For performance reasons, there are few member functions and the
    access to the member variables is direct (i.e., using the \c . or
    \c -> operator). This low-level feel makes the structures
    straightforward to use and emphasizes that these are simply
    parameters used by the style functions.

    The caller of a QStyle function usually creates QStyleOption
    objects on the stack. This combined with Qt's extensive use of
    \l{implicit sharing} for types such as QString, QPalette, and
    QColor ensures that no memory allocation needlessly takes place.

    The following code snippet shows how to use a specific
    QStyleOption subclass to paint a push button:

    \code
        void MyPushButton::paintEvent(QPaintEvent *)
        {
            QStyleOptionButton option;
            option.initFrom(this);
            option.state = isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
            if (isDefault())
                option.features |= QStyleOptionButton::DefaultButton;
            option.text = text();
            option.icon = icon();

            QPainter painter(this);
            style().drawControl(QStyle::CE_PushButton, &option, &painter, this);
        }
    \endcode

    In our example, the control is a QStyle::CE_PushButton, and
    according to the QStyle::drawControl() documentation the
    corresponding class is QStyleOptionButton.

    When reimplementing QStyle functions that take a QStyleOption
    parameter, you often need to cast the QStyleOption to a subclass.
    For safety, you can use qstyleoption_cast<T>() to ensure that the pointer
    type is correct. For example:

    \code
        void MyStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget)
        {
            if (element == PE_FocusRect) {
                const QStyleOptionFocusRect *focusRectOption =
                        qstyleoption_cast<const QStyleOptionFocusRect *>(option);
                if (focusRectOption) {
                    ...
                }
            } else {
                ...
            }
        }
    \endcode

    qstyleoption_cast<T>() will return 0 if the object to which \c option
    points isn't of the correct type.

    \sa QStyle, QStylePainter
*/

/*!
    \enum QStyleOption::OptionType

    This enum is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.

    \value SO_Default QStyleOption
    \value SO_FocusRect \l QStyleOptionFocusRect
    \value SO_Button \l QStyleOptionButton
    \value SO_Tab \l QStyleOptionTab
    \value SO_TabWidgetFrame \l QStyleOptionTabWidgetFrame
    \value SO_TabBarBase \l QStyleOptionTabBarBase
    \value SO_MenuItem \l QStyleOptionMenuItem
    \value SO_Complex \l QStyleOptionComplex
    \value SO_Slider \l QStyleOptionSlider
    \value SO_Frame \l QStyleOptionFrame \l QStyleOptionFrameV2
    \value SO_GroupBox \l QStyleOptionGroupBox
    \value SO_ProgressBar \l QStyleOptionProgressBar \l QStyleOptionProgressBarV2
    \value SO_Q3ListView \l QStyleOptionQ3ListView
    \value SO_Q3ListViewItem \l QStyleOptionQ3ListViewItem
    \value SO_Header \l QStyleOptionHeader
    \value SO_Q3DockWindow \l QStyleOptionQ3DockWindow
    \value SO_DockWidget \l QStyleOptionDockWidget
    \value SO_SpinBox \l QStyleOptionSpinBox
    \value SO_ToolButton \l QStyleOptionToolButton
    \value SO_ComboBox \l QStyleOptionComboBox
    \value SO_ToolBox \l QStyleOptionToolBox
    \value SO_ToolBar \l QStyleOptionToolBar
    \value SO_RubberBand \l QStyleOptionRubberBand
    \value SO_TitleBar \l QStyleOptionTitleBar
    \value SO_ViewItem \l QStyleOptionViewItem (used in Interviews)
    \value SO_CustomBase Reserved for custom QStyleOptions;
                         all custom controls values must be above this value
    \value SO_ComplexCustomBase Reserved for custom QStyleOptions;
                         all custom complex controls values must be above this value
*/

/*!
    Constructs a QStyleOption with version \a version and type \a
    type.

    The version has no special meaning for QStyleOption; it can be
    used by subclasses to distinguish between different version of
    the same option type.

    The \l state member variable is initialized to
    QStyle::State_None.

    \sa version, type
*/

QStyleOption::QStyleOption(int version, int type)
    : version(version), type(type), state(QStyle::State_None),
      direction(QApplication::layoutDirection()), fontMetrics(QFont())
{
}


/*!
    Destroys the style option object.
*/
QStyleOption::~QStyleOption()
{
}

/*!
    \fn void QStyleOption::initFrom(const QWidget *widget)
    \since 4.1

    Initializes the \l state, \l direction, \l rect, \l palette, and
    \l fontMetrics member variables based on \a widget.

    This function is provided only for convenience. You can also
    initialize the variables manually if you want.

    \sa QWidget::layoutDirection(), QWidget::rect(),
        QWidget::palette(), QWidget::fontMetrics()
*/

/*!
    \obsolete

    Use initFrom(\a widget) instead.
*/
void QStyleOption::init(const QWidget *widget)
{
    state = QStyle::State_None;
    if (widget->isEnabled())
        state |= QStyle::State_Enabled;
    if (widget->hasFocus())
        state |= QStyle::State_HasFocus;
    if (widget->window()->testAttribute(Qt::WA_KeyboardFocusChange))
        state |= QStyle::State_KeyboardFocusChange;
    if (widget->underMouse())
        state |= QStyle::State_MouseOver;
    if (widget->window()->isActiveWindow())
        state |= QStyle::State_Active;
#ifdef QT_KEYPAD_NAVIGATION
    if (widget->hasEditFocus())
        state |= QStyle::State_HasEditFocus;
#endif

    direction = widget->layoutDirection();
    rect = widget->rect();
    palette = widget->palette();
    fontMetrics = widget->fontMetrics();
}

/*!
   Constructs a copy of \a other.
*/
QStyleOption::QStyleOption(const QStyleOption &other)
    : version(Version), type(Type), state(other.state),
      direction(other.direction), rect(other.rect), fontMetrics(other.fontMetrics),
      palette(other.palette)
{
}

/*!
    Assign \a other to this QStyleOption.
*/
QStyleOption &QStyleOption::operator=(const QStyleOption &other)
{
    state = other.state;
    direction = other.direction;
    rect = other.rect;
    fontMetrics = other.fontMetrics;
    palette = other.palette;
    return *this;
}

/*!
    \variable QStyleOption::Type

    Equals SO_Default.
*/

/*!
    \variable QStyleOption::Version

    Equals 1.
*/

/*!
    \variable QStyleOption::palette
    \brief the palette that should be used when painting the control
*/

/*!
    \variable QStyleOption::direction
    \brief the text layout direction that should be used when drawing text in the control
*/

/*!
    \variable QStyleOption::fontMetrics
    \brief the font metrics that should be used when drawing text in the control
*/

/*!
    \variable QStyleOption::rect
    \brief the area that should be used for various calculations and painting.

    This can have different meanings for different types of elements.
    For example, for \l QStyle::CE_PushButton it would be the
    rectangle for the entire button, while for \l
    QStyle::CE_PushButtonLabel it would be just the area for the push
    button label.
*/

/*!
    \variable QStyleOption::state
    \brief the style flags that are used when drawing the control

    \sa QStyle::drawPrimitive(), QStyle::drawControl(), QStyle::drawComplexControl(),
        QStyle::State
*/

/*!
    \variable QStyleOption::type
    \brief the option type of the style option

    \sa OptionType
*/

/*!
    \variable QStyleOption::version
    \brief the version of the style option

    This value can be used by subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast<T>(), you
    normally don't need to check it.
*/

/*!
    \class QStyleOptionFocusRect
    \brief The QStyleOptionFocusRect class is used to describe the
    parameters for drawing a focus rectangle with QStyle.
*/

/*!
    Constructs a QStyleOptionFocusRect. The members variables are
    initialized to default values.
*/

QStyleOptionFocusRect::QStyleOptionFocusRect()
    : QStyleOption(Version, SO_FocusRect)
{
    state |= QStyle::State_KeyboardFocusChange; // assume we had one, will be corrected in initFrom()
}

/*!
    \internal
*/
QStyleOptionFocusRect::QStyleOptionFocusRect(int version)
    : QStyleOption(version, SO_FocusRect)
{
    state |= QStyle::State_KeyboardFocusChange;  // assume we had one, will be corrected in initFrom()
}

/*!
    \variable QStyleOptionFocusRect::Type

    Equals SO_FocusRect.
*/

/*!
    \variable QStyleOptionFocusRect::Version

    Equals 1.
*/

/*!
    \fn QStyleOptionFocusRect::QStyleOptionFocusRect(const QStyleOptionFocusRect &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionFocusRect::backgroundColor
    \brief The background color on which the focus rectangle is being drawn.
*/

/*!
    \class QStyleOptionFrame

    \brief The QStyleOptionFrame class is used to describe the
    parameters for drawing a frame.

    QStyleOptionFrame is used for drawing several built-in Qt widget,
    including QFrame, QGroupBox, QLineEdit, and QMenu. Note that to
    describe the parameters necessary for drawing a frame in Qt 4.1 or
    above, you must use the QStyleOptionFrameV2 subclass.

    An instance of the QStyleOptionFrame class has \l type SO_Frame
    and \l version 1.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.  The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionFrame and QStyleOptionFrameV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionFrameV2, QStyleOption
*/

/*!
    Constructs a QStyleOptionFrame. The members variables are
    initialized to default values.
*/

QStyleOptionFrame::QStyleOptionFrame()
    : QStyleOption(Version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionFrame::QStyleOptionFrame(int version)
    : QStyleOption(version, SO_Frame), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionFrame::QStyleOptionFrame(const QStyleOptionFrame &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionFrame::Type

    Equals SO_Frame.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.
*/

/*!
    \variable QStyleOptionFrame::Version

    Equals 1.

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.
*/

/*!
    \variable QStyleOptionFrame::lineWidth
    \brief The line width for drawing the frame.

    \sa QFrame::lineWidth
*/

/*!
    \variable QStyleOptionFrame::midLineWidth
    \brief The mid-line width for drawing the frame. This is usually used in
    drawing sunken or raised frames.

    \sa QFrame::midLineWidth
*/

/*!
    \class QStyleOptionFrameV2

    \brief The QStyleOptionFrameV2 class is used to describe the
    parameters necessary for drawing a frame in Qt 4.1 or above.

    \since 4.1

    QStyleOptionFrameV2 inherits QStyleOptionFrame which is used for
    drawing several built-in Qt widget, including QFrame, QGroupBox,
    QLineEdit, and QMenu.

    An instance of the QStyleOptionFrameV2 class has \l type SO_Frame
    and \l version 2.  The type is used internally by QStyleOption,
    its subclasses, and qstyleoption_cast() to determine the type of
    style option. In general you do not need to worry about this
    unless you want to create your own QStyleOption subclass and your
    own styles. The version is used by QStyleOption subclasses to
    implement extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionFrame and QStyleOptionFrameV2. One way to achieve this
    is to use the QStyleOptionFrameV2 copy constructor. For example:

    \code
        if (const QStyleOptionFrame *frameOption =
               qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            QStyleOptionFrameV2 frameOptionV2(*frameOption);

            // draw the frame using frameOptionV2
        }
    \endcode

    In the example above: If the \c frameOption's version is 1, \l
    FrameFeature is set to \l None for \c frameOptionV2. If \c
    frameOption's version is 2, the constructor will simply copy the
    \c frameOption's \l FrameFeature value.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionFrame, QStyleOption
*/

/*!
    Constructs a QStyleOptionFrameV2.
*/
QStyleOptionFrameV2::QStyleOptionFrameV2()
    : QStyleOptionFrame(Version), features(None)
{
}

/*!
    \fn QStyleOptionFrameV2::QStyleOptionFrameV2(const QStyleOptionFrameV2 &other)

    Constructs a QStyleOptionFrameV2 copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionFrameV2::QStyleOptionFrameV2(int version)
    : QStyleOptionFrame(version), features(None)
{
}

/*!
    Constructs a QStyleOptionFrameV2 copy of the \a other style option
    which can be either of the QStyleOptionFrameV2 or
    QStyleOptionFrame types.

    If the \a other style option's version is 1, the new style option's \l
    FrameFeature value is set to \l QStyleOptionFrameV2::None. If its
    version is 2, its \l FrameFeature value is simply copied to the
    new style option.

    \sa version
*/
QStyleOptionFrameV2::QStyleOptionFrameV2(const QStyleOptionFrame &other)
{
    QStyleOptionFrame::operator=(other);

    const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
    features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
    version = Version;
}

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionFrameV2 or
    QStyleOptionFrame types.

    If the \a{other} style option's version is 1, this style option's
    \l FrameFeature value is set to \l QStyleOptionFrameV2::None. If
    its version is 2, its \l FrameFeature value is simply copied to
    this style option.
*/
QStyleOptionFrameV2 &QStyleOptionFrameV2::operator=(const QStyleOptionFrame &other)
{
    QStyleOptionFrame::operator=(other);

    const QStyleOptionFrameV2 *f2 = qstyleoption_cast<const QStyleOptionFrameV2 *>(&other);
    features = f2 ? f2->features : FrameFeatures(QStyleOptionFrameV2::None);
    version = Version;
    return *this;
}

/*!
    \enum QStyleOptionFrameV2::FrameFeature

    This enum describles the different types of features a frame can have.

    \value None Indicates a normal frame.
    \value Flat Indicates a flat frame.
*/


/*!
    \class QStyleOptionGroupBox

    \brief The QStyleOptionGroupBox class describes the parameters for
    drawing a group box.

    \since 4.1

    The QStyleOptionGroupBox class is used to draw the group box'
    frame, title, and optional check box.

    It holds the lineWidth and the midLineWidth for drawing the panel,
    the group box's \l {text}{title} and the title's \l
    {textAlignment}{alignment} and \l {textColor}{color}.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption

*/

/*!
    \variable QStyleOptionGroupBox::lineWidth

    \brief The line width for drawing the panel.

    \sa QFrame::lineWidth
*/

/*!
    \variable QStyleOptionGroupBox::midLineWidth
    \brief The mid-line width for drawing the panel. This is usually used in
    drawing sunken or raised group box frames.

    \sa QFrame::midLineWidth
*/

/*!
    \variable QStyleOptionGroupBox::text

    The text of the group box.

    \sa QGroupBox::title
*/

/*!
    \variable QStyleOptionGroupBox::textAlignment

    The alignment of the group box title.

    \sa QGroupBox::alignment
*/

/*!
    \variable QStyleOptionGroupBox::textColor

    The color of the group box title.
*/

/*!
    Constructs a QStyleOptionGroupBox. The members variables are
    initialized to default values.
*/
QStyleOptionGroupBox::QStyleOptionGroupBox()
    : QStyleOptionComplex(Version, Type), features(QStyleOptionFrameV2::None),
      textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionGroupBox::QStyleOptionGroupBox(const QStyleOptionGroupBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionGroupBox::QStyleOptionGroupBox(int version)
    : QStyleOptionComplex(version, Type), features(QStyleOptionFrameV2::None),
      textAlignment(Qt::AlignLeft), lineWidth(0), midLineWidth(0)
{
}

/*!
    \class QStyleOptionHeader
    \brief The QStyleOptionHeader class is used to describe the
    parameters for drawing a header.

    The QStyleOptionHeader class is used for drawing the item views'
    header pane, header sort arrow, and header label.
*/

/*!
    Constructs a QStyleOptionHeader. The members variables are
    initialized to default values.
*/

QStyleOptionHeader::QStyleOptionHeader()
    : QStyleOption(QStyleOptionHeader::Version, SO_Header),
      section(0), textAlignment(0), iconAlignment(0),
      position(QStyleOptionHeader::Beginning),
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
      orientation(Qt::Horizontal)
{
}

/*!
    \internal
*/
QStyleOptionHeader::QStyleOptionHeader(int version)
    : QStyleOption(version, SO_Header),
      section(0), textAlignment(0), iconAlignment(0),
      position(QStyleOptionHeader::Beginning),
      selectedPosition(QStyleOptionHeader::NotAdjacent), sortIndicator(None),
      orientation(Qt::Horizontal)
{
}

/*!
    \variable QStyleOptionHeader::orientation
    \brief the header's orientation (horizontal or vertical)

    \sa Qt::Orientation
*/

/*!
    \fn QStyleOptionHeader::QStyleOptionHeader(const QStyleOptionHeader &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionHeader::Type

    Equals SO_Header.
*/

/*!
    \variable QStyleOptionHeader::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionHeader::section
    \brief Which section of the header is being painted.
*/

/*!
    \variable QStyleOptionHeader::text
    \brief The text of the header.
*/

/*!
    \variable QStyleOptionHeader::textAlignment
    \brief The alignment flags for the text of the header.

    \sa Qt::Alignment
*/

/*!
    \variable QStyleOptionHeader::icon
    \brief The icon of the header.
*/

/*!
    \variable QStyleOptionHeader::iconAlignment
    \brief The alignment flags for the icon of the header.

    \sa Qt::Alignment
*/

/*!
    \variable QStyleOptionHeader::position
    \brief the section's position in relation to the other sections
*/

/*!
    \variable QStyleOptionHeader::selectedPosition
    \brief the section's position in relation to the selected section
*/

/*!
    \variable QStyleOptionHeader::sortIndicator
    \brief the direction the sort indicator should be drawn
*/

/*!
    \enum QStyleOptionHeader::SectionPosition

    This enum lets you know where the section's position is in relation to the other sections.

    \value Beginning At the beginining of the header
    \value Middle In the middle of the header
    \value End At the end of the header
    \value OnlyOneSection Only one header section
*/

/*!
    \enum QStyleOptionHeader::SelectedPosition

    This enum lets you know where the section's position is in relation to the selected section.

    \value NotAdjacent Not adjacent to the selected section
    \value NextIsSelected The next section is selected
    \value PreviousIsSelected The previous section is selected
    \value NextAndPreviousAreSelected Both the next and previous section are selected
*/

/*!
    \enum QStyleOptionHeader::SortIndicator

    Indicates which direction the sort indicator should be drawn
    \value None No sort indicator is needed
    \value SortUp Draw an up indicator
    \value SortDown Draw a down indicator
*/

/*!
    \class QStyleOptionButton
    \brief The QStyleOptionButton class is used to describe the
    parameters for drawing buttons.

    The QStyleOptionButton class is used to draw \l QPushButton, \l
    QCheckBox, and \l QRadioButton.

    \sa QStyleOptionToolButton
*/

/*!
    \enum QStyleOptionButton::ButtonFeature

    This enum describles the different types of features a push button can have.

    \value None	Indicates a normal push button.
    \value Flat Indicates a flat push button.
    \value HasMenu Indicates that the button has a drop down menu.
    \value DefaultButton Indicates that the button is a default button.
    \value AutoDefaultButton Indicates that the button is an auto default button.

    \sa features
*/

/*!
    Constructs a QStyleOptionButton. The members variables are
    initialized to default values.
*/

QStyleOptionButton::QStyleOptionButton()
    : QStyleOption(QStyleOptionButton::Version, SO_Button), features(None)
{
}

/*!
    \internal
*/
QStyleOptionButton::QStyleOptionButton(int version)
    : QStyleOption(version, SO_Button), features(None)
{
}

/*!
    \fn QStyleOptionButton::QStyleOptionButton(const QStyleOptionButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionButton::Type

    Equals SO_Button.
*/

/*!
    \variable QStyleOptionButton::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionButton::features
    \brief The features for the button

    This variable is a bitwise OR of the features that describe this button.

    \sa ButtonFeature
*/

/*!
    \variable QStyleOptionButton::text
    \brief The text of the button.
*/

/*!
    \variable QStyleOptionButton::icon
    \brief The icon of the button.

    \sa iconSize
*/

/*!
    \variable QStyleOptionButton::iconSize
    \brief The size of the icon for the button
*/


#ifndef QT_NO_TOOLBAR
/*!
    \class QStyleOptionToolBar

    \brief The QStyleOptionToolBar class is used to describe the
    parameters for drawing a toolbar.

    \since 4.1

    The QStyleOptionToolBar class holds the lineWidth and the
    midLineWidth for drawing the widget. It also stores information
    about which \l {toolBarArea}{area} the toolbar should be located
    in, whether it is movable or not, which position the toolbar line
    should have (positionOfLine), and the toolbar's position within
    the line (positionWithinLine).

    In addition, the class provides a couple of enums: The
    ToolBarFeature enum is used to describe whether a toolbar is
    movable or not, and the ToolBarPosition enum is used to describe
    the position of a toolbar line, as well as the toolbar's position
    within the line.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOption
*/

/*!
    Constructs a QStyleOptionToolBar. The members variables are
    initialized to default values.
*/

QStyleOptionToolBar::QStyleOptionToolBar()
    : QStyleOption(Version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
      toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{
}

/*!
    \fn QStyleOptionToolBar::QStyleOptionToolBar(const QStyleOptionToolBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionToolBar::QStyleOptionToolBar(int version)
: QStyleOption(version, SO_ToolBar), positionOfLine(OnlyOne), positionWithinLine(OnlyOne),
  toolBarArea(Qt::TopToolBarArea), features(None), lineWidth(0), midLineWidth(0)
{

}

/*!
    \enum QStyleOptionToolBar::ToolBarPosition

    \image qstyleoptiontoolbar-position.png

    This enum is used to describe the position of a toolbar line, as
    well as the toolbar's position within the line.

    The order of the positions within a line starts at the top of a
    vertical line, and from the left within a horizontal line. The
    order of the positions for the lines is always from the the
    parent widget's boundary edges.

    \value Beginning The toolbar is located at the beginning of the line,
           or the toolbar line is the first of several lines. There can
           only be one toolbar (and only one line) with this position.
    \value Middle The toolbar is located in the middle of the line,
           or the toolbar line is in the middle of several lines. There can
           several toolbars (and lines) with this position.
    \value End The toolbar is located at the end of the line,
           or the toolbar line is the last of several lines. There can
           only be one toolbar (and only one line) with this position.
    \value OnlyOne There is only one toolbar or line. This is the default value
           of the positionOfLine and positionWithinLine variables.

    \sa positionWithinLine, positionOfLine
*/

/*!
    \enum QStyleOptionToolBar::ToolBarFeature

    This enum is used to describe whether a toolbar is movable or not.

    \value None The toolbar cannot be moved. The default value.
    \value Movable The toolbar is movable, and a handle will appear when
           holding the cursor over the toolbar's boundary.

    \sa features, QToolBar::isMovable()
*/

/*!
    \variable QStyleOptionToolBar::positionOfLine

    This variable holds the position of the toolbar line.

    The default value is QStyleOptionToolBar::OnlyOne.
*/

/*!
    \variable QStyleOptionToolBar::positionWithinLine

    This variable holds the position of the toolbar within a line.

    The default value is QStyleOptionToolBar::OnlyOne.
*/

/*!
    \variable QStyleOptionToolBar::toolBarArea

    This variable holds the location for drawing the toolbar.

    The default value is Qt::TopToolBarArea.

    \sa Qt::ToolBarArea
*/

/*!
    \variable QStyleOptionToolBar::features

    This variable holds whether the toolbar is movable or not.

    The default value is \l None.
*/

/*!
    \variable QStyleOptionToolBar::lineWidth

    This variable holds the line width for drawing the toolbar.

    The default value is 0.
*/

/*!
    \variable QStyleOptionToolBar::midLineWidth

    This variable holds the mid-line width for drawing the toolbar.

    The default value is 0.
*/


#endif

#ifndef QT_NO_TABBAR
/*!
    \class QStyleOptionTab
    \brief The QStyleOptionTab class is used to describe the
    parameters for drawing a tab bar.

    The QStyleOptionTab class is used for drawing several built-in Qt
    widgets including \l QTabBar and the panel for \l QTabWidget. Note
    that to describe the parameters necessary for drawing a frame in
    Qt 4.1 or above, you must use the QStyleOptionFrameV2 subclass.

    An instance of the QStyleOptiontabV2 class has type \l SO_Tab and
    \l version 1. The type is used internally by QStyleOption, its
    subclasses, and qstyleoption_cast() to determine the type of style
    option. In general you do not need to worry about this unless you
    want to create your own QStyleOption subclass and your own
    styles. The version is used by QStyleOption subclasses to
    implement extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionTab and QStyleOptionTabV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionTabV2, QStyleOption
*/

/*!
    Constructs a QStyleOptionTab object. The members variables are
    initialized to default values.
*/

QStyleOptionTab::QStyleOptionTab()
    : QStyleOption(QStyleOptionTab::Version, SO_Tab),
      shape(QTabBar::RoundedNorth),
      row(0),
      position(Beginning),
      selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \internal
*/
QStyleOptionTab::QStyleOptionTab(int version)
    : QStyleOption(version, SO_Tab),
      shape(QTabBar::RoundedNorth),
      row(0),
      position(Beginning),
      selectedPosition(NotAdjacent), cornerWidgets(QStyleOptionTab::NoCornerWidgets)
{
}

/*!
    \fn QStyleOptionTab::QStyleOptionTab(const QStyleOptionTab &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionTab::Type

    Equals SO_Tab.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.
*/

/*!
    \variable QStyleOptionTab::Version

    Equals 1.

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.
*/

/*! \enum QStyleOptionTab::TabPosition

    \value Beginning The tab is the first tab in the tab bar.
    \value Middle The tab is neither the first nor the last tab in the tab bar.
    \value End The tab is the last tab in the tab bar.
    \value OnlyOneTab The tab is both the first and the last tab in the tab bar.

    \sa position
*/

/*!
    \enum QStyleOptionTab::CornerWidget

    These flags indicate the corner widgets in a tab.

    \value NoCornerWidgets  There are no corner widgets
    \value LeftCornerWidget  Left corner widget
    \value RightCornerWidget Right corner widget

    \sa cornerWidgets
*/

/*! \enum QStyleOptionTab::SelectedPosition

    \value NotAdjacent The tab is not adjacent to a selected tab (or is the selected tab).
    \value NextIsSelected The next tab (typically the tab on the right) is selected.
    \value PreviousIsSelected The previous tab (typically the tab on the left) is selected.

    \sa selectedPosition
*/

/*!
    \variable QStyleOptionTab::selectedPosition

    \brief The position of the selected tab in relation to this tab. Some styles
    need to draw a tab differently depending on whether or not it is adjacent
    to the selected tab.
*/

/*!
    \variable QStyleOptionTab::cornerWidgets

    \brief Information on the cornerwidgets of the tab bar.

    \sa CornerWidget
*/


/*!
    \variable QStyleOptionTab::shape
    \brief The tab shape used to draw the tab.
    \sa QTabBar::Shape
*/

/*!
    \variable QStyleOptionTab::text
    \brief The text of the tab.
*/

/*!
    \variable QStyleOptionTab::icon
    \brief The icon for the tab.
*/

/*!
    \variable QStyleOptionTab::row
    \brief which row the tab is currently in

    0 indicates the front row.

    Currently this property can only be 0.
*/

/*!
    \variable QStyleOptionTab::position
    \brief the position of the tab in the tab bar
*/

/*!
    \class QStyleOptionTabV2
    \brief The QStyleOptionTabV2 class is used to describe the
    parameters necessary for drawing a tabs in Qt 4.1 or above.

    \since 4.1

    An instance of the QStyleOptionTabV2 class has type \l SO_Tab and
    \l version 2. The type is used internally by QStyleOption, its
    subclasses, and qstyleoption_cast() to determine the type of style
    option. In general you do not need to worry about this unless you
    want to create your own QStyleOption subclass and your own
    styles. The version is used by QStyleOption subclasses to
    implement extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionTab and QStyleOptionTabV2. One way to achieve this is
    to use the QStyleOptionTabV2 copy constructor. For example:

    \code
        if (const QStyleOptionTab *tabOption =
               qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QStyleOptionTabV2 tabV2(*tabOption);

            // draw the tab using tabV2
        }
    \endcode

    in the example above: If \c tabOption's version is 1, the extra
    member (\l iconSize) will be set to an invalid size for \c tabV2.
    If \c tabOption's version is 2, the constructor will simply copy
    the \c tab's iconSize.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionTab, QStyleOption
*/

/*!
    \variable QStyleOptionTabV2::iconSize

    The size for the icons. If this size is invalid and you need an
    icon size, you can use QStyle::pixelMetric() to find the default
    icon size for tab bars.

    \sa QTabBar::iconSize() QStyle::pixelMetric()
*/

/*!
    Constructs a QStyleOptionTabV2.
*/
QStyleOptionTabV2::QStyleOptionTabV2()
    : QStyleOptionTab(Version)
{
}

/*!
    \internal
*/
QStyleOptionTabV2::QStyleOptionTabV2(int version)
    : QStyleOptionTab(version)
{
}

/*!
    \fn QStyleOptionTabV2::QStyleOptionTabV2(const QStyleOptionTabV2 &other)

    Constructs a copy of the \a other style option.
*/

/*!
    Constructs a QStyleOptionTabV2 copy of the \a other style option
    which can be either of the QStyleOptionTabV2 or QStyleOptionTab
    types.

    If the other style option's version is 1, the new style option's
    \c iconSize is set to an invalid value. If its version is 2, its
    \c iconSize value is simply copied to the new style option.
*/
QStyleOptionTabV2::QStyleOptionTabV2(const QStyleOptionTab &other)
    : QStyleOptionTab(Version)
{
    if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other)) {
        *this = *tab;
    } else {
        *((QStyleOptionTab *)this) = other;
        version = Version;
    }
}

/*!
    Assigns the \a other style option to this QStyleOptionTabV2. The
    \a other style option can be either of the QStyleOptionTabV2 or
    QStyleOptionTab types.

    If the other style option's version is 1, this style option's \c
    iconSize is set to an invalid size. If its version is 2, its \c
    iconSize value is simply copied to this style option.
*/
QStyleOptionTabV2 &QStyleOptionTabV2::operator=(const QStyleOptionTab &other)
{
    QStyleOptionTab::operator=(other);

    if (const QStyleOptionTabV2 *tab = qstyleoption_cast<const QStyleOptionTabV2 *>(&other))
        iconSize = tab->iconSize;
    else
        iconSize = QSize();
    return *this;
}


#endif // QT_NO_TABBAR

/*!
    \class QStyleOptionProgressBar

    \brief The QStyleOptionProgressBar class is used to describe the
    parameters necessary for drawing a progress bar.

    Since Qt 4.1, Qt uses the QStyleOptionProgressBarV2 subclass for
    drawing QProgressBar.

    An instance of the QStyleOptionProgressBar class has type
    SO_ProgressBar and version 1.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.  The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionProgressBar and QStyleOptionProgressBarV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionProgressBarV2, QStyleOption
*/

/*!
    Constructs a QStyleOptionProgressBar. The members variables are
    initialized to default values.
*/

QStyleOptionProgressBar::QStyleOptionProgressBar()
    : QStyleOption(QStyleOptionProgressBar::Version, SO_ProgressBar),
      minimum(0), maximum(0), progress(0), textAlignment(0), textVisible(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBar::QStyleOptionProgressBar(int version)
    : QStyleOption(version, SO_ProgressBar),
      minimum(0), maximum(0), progress(0), textAlignment(0), textVisible(false)
{
}

/*!
    \fn QStyleOptionProgressBar::QStyleOptionProgressBar(const QStyleOptionProgressBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionProgressBar::Type

    Equals SO_ProgressBar.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles.
*/

/*!
    \variable QStyleOptionProgressBar::Version

    Equals 1.

    The version is used by QStyleOption subclasses to implement
    extensions without breaking compatibility. If you use
    qstyleoption_cast(), you normally don't need to check it.
*/

/*!
    \variable QStyleOptionProgressBar::minimum
    \brief The minimum value for the progress bar

    This is the minimum value in the progress bar.
    \sa QProgressBar::minimum
*/

/*!
    \variable QStyleOptionProgressBar::maximum
    \brief The maximum value for the progress bar

    This is the maximum value in the progress bar.
    \sa QProgressBar::maximum
*/

/*!
    \variable QStyleOptionProgressBar::text
    \brief The text for the progress bar.

    The progress bar text is usually just the progress expressed as a string.
    An empty string indicates that the progress bar has not started yet.

    \sa QProgressBar::text
*/

/*!
    \variable QStyleOptionProgressBar::textVisible
    \brief A flag indicating whether or not text is visible.

    If this flag is true then the text is visible. Otherwise, the text is not visible.

    \sa QProgressBar::textVisible
*/


/*!
    \variable QStyleOptionProgressBar::textAlignment
    \brief The text alignment for the text in the QProgressBar

    This can be used as a guide on where the text should be in the progressbar.
*/

/*!
    \variable QStyleOptionProgressBar::progress
    \brief the current progress for the progress bar.

    The current progress. A value of QStyleOptionProgressBar::minimum - 1
    indicates that the progress hasn't started yet.

    \sa QProgressBar::value
*/

/*!
    \class QStyleOptionProgressBarV2
    \brief The QStyleOptionProgressBarV2 class is used to describe the
    parameters necessary for drawing a progress bar in Qt 4.1 or above.

    \since 4.1

    An instance of this class has \l type SO_ProgressBar and \l
    version 2.

    The type is used internally by QStyleOption, its subclasses, and
    qstyleoption_cast() to determine the type of style option. In
    general you do not need to worry about this unless you want to
    create your own QStyleOption subclass and your own styles. The
    version is used by QStyleOption subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast(),
    you normally don't need to check it.

    If you create your own QStyle subclass, you should handle both
    QStyleOptionProgressBar and QStyleOptionProgressBarV2. One way
    to achieve this is to use the QStyleOptionProgressBarV2 copy
    constructor. For example:

    \code
        if (const QStyleOptionProgressBar *progressBarOption =
               qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
               QStyleOptionProgressBarV2 progressBarV2(*progressBarOption);

            // draw the progress bar using progressBarV2
        }
    \endcode

    In the example above: If the \c progressBarOption's version is 1,
    the extra members (\l orientation, \l invertedAppearance, and \l
    bottomToTop) are set to default values for \c progressBarV2. If
    the \c progressBarOption's version is 2, the constructor will
    simply copy the extra members to progressBarV2.

    For an example demonstrating how style options can be used, see
    the \l {widgets/styles}{Styles} example.

    \sa QStyleOptionProgressBar, QStyleOption
*/

/*!
    Constructs a QStyleOptionProgressBarV2. The members variables are
    initialized to default values.
*/

QStyleOptionProgressBarV2::QStyleOptionProgressBarV2()
    : QStyleOptionProgressBar(2),
      orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

/*!
    \internal
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(int version)
    : QStyleOptionProgressBar(version),
      orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
}

/*!
    Constructs a copy of the \a other style option which can be either
    of the QStyleOptionProgressBar and QStyleOptionProgressBarV2
    types.

    If the \a{other} style option's version is 1, the extra members (\l
    orientation, \l invertedAppearance, and \l bottomToTop) are set
    to default values for the new style option. If \a{other}'s version
    is 2, the extra members are simply copied.

    \sa version
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBar &other)
    : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
    const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
    if (pb2)
        *this = *pb2;
    else
        *((QStyleOptionProgressBar *)this) = other;
}

/*!
    Constructs a copy of the \a other style option.
*/
QStyleOptionProgressBarV2::QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2 &other)
    : QStyleOptionProgressBar(2), orientation(Qt::Horizontal), invertedAppearance(false), bottomToTop(false)
{
    *this = other;
}

/*!
    Assigns the \a other style option to this style option. The \a
    other style option can be either of the QStyleOptionProgressBarV2
    or QStyleOptionProgressBar types.

    If the \a{other} style option's version is 1, the extra members
    (\l orientation, \l invertedAppearance, and \l bottomToTop) are
    set to default values for this style option. If \a{other}'s
    version is 2, the extra members are simply copied to this style
    option.
*/
QStyleOptionProgressBarV2 &QStyleOptionProgressBarV2::operator=(const QStyleOptionProgressBar &other)
{
    QStyleOptionProgressBar::operator=(other);

    const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(&other);
    orientation = pb2 ? pb2->orientation : Qt::Horizontal;
    invertedAppearance = pb2 ? pb2->invertedAppearance : false;
    bottomToTop = pb2 ? pb2->bottomToTop : false;
    return *this;
}

/*!
    \variable QStyleOptionProgressBarV2::orientation
    \brief the progress bar's orientation (horizontal or vertical)

    \sa QProgressBar::orientation
*/

/*!
    \variable QStyleOptionProgressBarV2::invertedAppearance
    \brief whether the progress bar's appearance is inverted

    \sa QProgressBar::invertedAppearance
*/

/*!
    \variable QStyleOptionProgressBarV2::bottomToTop
    \brief whether the text reads from bottom to top when the progress bar is vertical

    \sa QProgressBar::textDirection
*/

/*!
    \class QStyleOptionMenuItem
    \brief The QStyleOptionMenuItem class is used to describe the
    parameter necessary for drawing a menu item.

    The QStyleOptionMenuItem is used for drawing menu items from \l
    QMenu. It is also used for drawing other menu-related widgets.
*/

/*!
    Constructs a QStyleOptionMenuItem. The members variables are
    initialized to default values.
*/

QStyleOptionMenuItem::QStyleOptionMenuItem()
    : QStyleOption(QStyleOptionMenuItem::Version, SO_MenuItem), menuItemType(Normal),
      checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \internal
*/
QStyleOptionMenuItem::QStyleOptionMenuItem(int version)
    : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
      checkType(NotCheckable), checked(false), menuHasCheckableItems(true), maxIconWidth(0), tabWidth(0)
{
}

/*!
    \fn QStyleOptionMenuItem::QStyleOptionMenuItem(const QStyleOptionMenuItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionMenuItem::Type

    Equals SO_MenuItem.
*/

/*!
    \variable QStyleOptionMenuItem::Version

    Equals 1.
*/

/*!
    \enum QStyleOptionMenuItem::MenuItemType

    These values indicate the type of menu item that the structure describes.

    \value Normal A normal menu item.
    \value DefaultItem A menu item that is the default action as specified with \l QMenu::defaultAction().
    \value Separator A menu separator.
    \value SubMenu Indicates the menu item points to a sub-menu.
    \value Scroller A popup menu scroller (currently only used on Mac OS X).
    \value TearOff A tear-off handle for the menu.
    \value Margin The margin of the menu.
    \value EmptyArea The empty area of the menu.
*/

/*!
    \enum QStyleOptionMenuItem::CheckType

    These enums are used to indicate whether or not a check mark should be
    drawn for the item, or even if it should be drawn at all.

    \value NotCheckable The item is not checkable.
    \value Exclusive The item is an exclusive check item (like a radio button).
    \value NonExclusive The item is a non-exclusive check item (like a check box).

    \sa QAction::checkable, QAction::checked, QActionGroup::exclusive
*/

/*!
    \variable QStyleOptionMenuItem::menuItemType

    \brief the type of menu item

    \sa MenuItemType
*/

/*!
    \variable QStyleOptionMenuItem::checkType
    \brief The type of checkmark of the menu item
    \sa CheckType
*/

/*!
    \variable QStyleOptionMenuItem::checked
    \brief whether the menu item is checked or not.
*/

/*!
    \variable QStyleOptionMenuItem::menuHasCheckableItems
    \brief whether the menu as a whole has checkable items or not.

    If this option is set to false, then the menu has no checkable
    items. This makes it possible for GUI styles to save some
    horizontal space that would normally be used for the check column.
*/

/*!
    \variable QStyleOptionMenuItem::menuRect
    \brief The rectangle for the entire menu.
*/

/*!
    \variable QStyleOptionMenuItem::text
    \brief The text for the menu item.

    Note that the text format is something like this "Menu
    text\bold{\\t}Shortcut".

    If the menu item doesn't have a shortcut, it will just contain
    the menu item's text.
*/

/*!
    \variable QStyleOptionMenuItem::icon
    \brief The icon for the menu item.
*/

/*!
    \variable QStyleOptionMenuItem::maxIconWidth
    \brief the maximum icon width for the icon in the menu item.

    This can be used for drawing the icon into the correct place or
    properly aligning items. The variable must be set regardless of
    whether or not the menu item has an icon.
*/

/*!
    \variable QStyleOptionMenuItem::tabWidth
    \brief The tab width for the menu item.

    The tab width is the distance between the text of the menu item
    and the shortcut.
*/


/*!
    \variable QStyleOptionMenuItem::font
    \brief The font used for the menu item text.

    This is the font that should be used for drawing the menu text minus the
    shortcut. The shortcut is usually drawn using the painter's font.
*/

/*!
    \class QStyleOptionComplex
    \brief The QStyleOptionComplex class is used to hold parameters that are
    common to all complex controls.

    This class is not used on its own. Instead it is used to derive other
    complex control options, for example \l QStyleOptionSlider and
    \l QStyleOptionSpinBox.
*/

/*!
    Constructs a QStyleOptionComplex of type \a type and version \a
    version. Usually this constructor is called by subclasses.

    The \l subControls member is initialized to \l QStyle::SC_All.
    The \l activeSubControls member is initialized to \l
    QStyle::SC_None.
*/

QStyleOptionComplex::QStyleOptionComplex(int version, int type)
    : QStyleOption(version, type), subControls(QStyle::SC_All), activeSubControls(QStyle::SC_None)
{
}

/*!
    \fn QStyleOptionComplex::QStyleOptionComplex(const QStyleOptionComplex &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionComplex::Type

    Equals SO_Complex.
*/

/*!
    \variable QStyleOptionComplex::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionComplex::subControls
    \brief The sub-controls that need to be painted.

    This is a bitwise OR of the various sub-controls that need to be drawn for the complex control.

    \sa QStyle::SubControl
*/

/*!
    \variable QStyleOptionComplex::activeSubControls
    \brief The sub-controls that are active for the complex control.

    This a bitwise OR of the various sub-controls that are active (pressed) for the complex control.

    \sa QStyle::SubControl
*/

#ifndef QT_NO_SLIDER
/*!
    \class QStyleOptionSlider
    \brief The QStyleOptionSlider class is used to describe the
    parameters needed for drawing a slider.

    The QStyleOptionSlider class is used for drawing \l QSlider and
    \l QScrollBar.
*/

/*!
    Constructs a QStyleOptionSlider. The members variables are
    initialized to default values.
*/

QStyleOptionSlider::QStyleOptionSlider()
    : QStyleOptionComplex(Version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
      tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
      dialWrapping(false)
{
}

/*!
    \internal
*/
QStyleOptionSlider::QStyleOptionSlider(int version)
    : QStyleOptionComplex(version, SO_Slider), orientation(Qt::Horizontal), minimum(0), maximum(0),
      tickPosition(QSlider::NoTicks), tickInterval(0), upsideDown(false),
      sliderPosition(0), sliderValue(0), singleStep(0), pageStep(0), notchTarget(0.0),
      dialWrapping(false)
{
}

/*!
    \fn QStyleOptionSlider::QStyleOptionSlider(const QStyleOptionSlider &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionSlider::Type

    Equals SO_Slider.
*/

/*!
    \variable QStyleOptionSlider::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionSlider::orientation
    \brief the slider's orientation (horizontal or vertical)

    \sa Qt::Orientation
*/

/*!
    \variable QStyleOptionSlider::minimum
    \brief The minimum value for the slider.
*/

/*!
    \variable QStyleOptionSlider::maximum
    \brief The maximum value for the slider.
*/

/*!
    \variable QStyleOptionSlider::tickPosition
    \brief the position of the slider's tick marks, if any.

    \sa QSlider::TickPosition
*/

/*!
    \variable QStyleOptionSlider::tickInterval
    \brief The interval that should be drawn between tick marks.
*/

/*!
    \variable QStyleOptionSlider::notchTarget
    \brief The number of pixel between notches

    \sa QDial::notchTarget()
*/

/*!
    \variable QStyleOptionSlider::dialWrapping
    \brief Indicates whether or not the dial should wrap or not

    \sa QDial::wrapping()
*/

/*!
    \variable QStyleOptionSlider::upsideDown
    \brief Indicates slider control orientation.

    Normally a slider increases as it moves up or to the right; upsideDown
    indicates that it should do the opposite (increase as it moves down or to
    the left).

    \sa QStyle::sliderPositionFromValue(), QStyle::sliderValueFromPosition(),
        QAbstractSlider::invertedAppearance
*/

/*!
    \variable QStyleOptionSlider::sliderPosition
    \brief The position of the slider handle.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderValue. Otherwise, it will have the current position
    of the handle.

    \sa QAbstractSlider::tracking, sliderValue
*/

/*!
    \variable QStyleOptionSlider::sliderValue
    \brief The value of the slider.

    If the slider has active feedback (i.e.,
    QAbstractSlider::tracking is true), this value will be the same
    as \l sliderPosition. Otherwise, it will have the value the
    slider had before the mouse was pressed.

    \sa QAbstractSlider::tracking sliderPosition
*/

/*!
    \variable QStyleOptionSlider::singleStep
    \brief The size of the single step of the slider.

    \sa QAbstractSlider::singleStep
*/

/*!
    \variable QStyleOptionSlider::pageStep
    \brief The size of the page step of the slider.

    \sa QAbstractSlider::pageStep
*/
#endif // QT_NO_SLIDER

#ifndef QT_NO_SPINBOX
/*!
    \class QStyleOptionSpinBox
    \brief The QStyleOptionSpinBox class is used to describe the
    parameters necessary for drawing a spin box.

    The QStyleOptionSpinBox is used for drawing QSpinBox and QDateTimeEdit.
*/

/*!
    Constructs a QStyleOptionSpinBox. The members variables are
    initialized to default values.
*/

QStyleOptionSpinBox::QStyleOptionSpinBox()
    : QStyleOptionComplex(Version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
      stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \internal
*/
QStyleOptionSpinBox::QStyleOptionSpinBox(int version)
    : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
      stepEnabled(QAbstractSpinBox::StepNone), frame(false)
{
}

/*!
    \fn QStyleOptionSpinBox::QStyleOptionSpinBox(const QStyleOptionSpinBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionSpinBox::Type

    Equals SO_SpinBox.
*/

/*!
    \variable QStyleOptionSpinBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionSpinBox::buttonSymbols
    \brief The type of button symbols to draw for the spin box.

    \sa QAbstractSpinBox::ButtonSymbols
*/

/*!
    \variable QStyleOptionSpinBox::stepEnabled
    \brief Indicates which buttons of the spin box are enabled.

    \sa QAbstractSpinBox::StepEnabled
*/

/*!
    \variable QStyleOptionSpinBox::frame
    \brief Indicates whether whether the spin box has a frame.

*/
#endif // QT_NO_SPINBOX

/*!
    \class QStyleOptionQ3ListViewItem
    \brief The QStyleOptionQ3ListViewItem class is used to describe an
    item drawn in a Q3ListView.

    This is used by the compatibility Q3ListView to draw its items.
    It should be avoided for new classes.

    \sa Q3ListView, Q3ListViewItem
*/

/*!
    \enum QStyleOptionQ3ListViewItem::Q3ListViewItemFeature

    This enum describes the features a list view item can have.

    \value None A standard item.
    \value Expandable The item has children that can be shown.
    \value MultiLine The item is more than one line tall.
    \value Visible The item is visible.
    \value ParentControl The item's parent is a type of item control (Q3CheckListItem::Controller).

    \sa features, Q3ListViewItem::isVisible(), Q3ListViewItem::multiLinesEnabled(),
        Q3ListViewItem::isExpandable()
*/

/*!
    Constructs a QStyleOptionQ3ListViewItem. The members variables are
    initialized to default values.
*/

QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem()
    : QStyleOption(Version, SO_Q3ListViewItem), features(None), height(0), totalHeight(0),
      itemY(0), childCount(0)
{
}

/*!
    \internal
*/
QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem(int version)
    : QStyleOption(version, SO_Q3ListViewItem), features(None), height(0), totalHeight(0),
      itemY(0), childCount(0)
{
}

/*!
    \fn QStyleOptionQ3ListViewItem::QStyleOptionQ3ListViewItem(const QStyleOptionQ3ListViewItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::Type

    Equals SO_Q3ListViewItem.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3ListViewItem::features
    \brief The features for this item

    This variable is a bitwise OR of the features of the item.

    \sa Q3ListViewItemFeature
*/

/*!
    \variable QStyleOptionQ3ListViewItem::height
    \brief The height of the item

    This doesn't include the height of the item's children.

    \sa Q3ListViewItem::height()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::totalHeight
    \brief The total height of the item, including its children

    \sa Q3ListViewItem::totalHeight()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::itemY
    \brief The Y-coordinate for the item

    \sa Q3ListViewItem::itemPos()
*/

/*!
    \variable QStyleOptionQ3ListViewItem::childCount
    \brief The number of children the item has.
*/

/*!
    \class QStyleOptionQ3ListView
    \brief The QStyleOptionQ3ListView class is used to describe the
    parameters for drawing a Q3ListView.

    The class is used for drawing the compat \l Q3ListView. It is not
    recommended for use in new code.
*/

/*!
    Creates a QStyleOptionQ3ListView. The members variables are
    initialized to default values.
*/

QStyleOptionQ3ListView::QStyleOptionQ3ListView()
    : QStyleOptionComplex(Version, SO_Q3ListView), sortColumn(0), itemMargin(0), treeStepSize(0),
      rootIsDecorated(false)
{
}

/*!
    \internal
*/
QStyleOptionQ3ListView::QStyleOptionQ3ListView(int version)
    : QStyleOptionComplex(version, SO_Q3ListView), sortColumn(0), itemMargin(0), treeStepSize(0),
      rootIsDecorated(false)
{
}

/*!
    \fn QStyleOptionQ3ListView::QStyleOptionQ3ListView(const QStyleOptionQ3ListView &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3ListView::Type

    Equals SO_Q3ListView.
*/

/*!
    \variable QStyleOptionQ3ListView::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3ListView::items
    \brief A list of items in the \l Q3ListView.

    This is a list of \l {QStyleOptionQ3ListViewItem}s. The first item
    can be used for most of the calculation that are needed for
    drawing a list view. Any additional items are the children of
    this first item, which may be used for additional information.

    \sa QStyleOptionQ3ListViewItem
*/

/*!
    \variable QStyleOptionQ3ListView::viewportPalette
    \brief The palette of Q3ListView's viewport.
*/

/*!
    \variable QStyleOptionQ3ListView::viewportBGRole
    \brief The background role of \l Q3ListView's viewport.

    \sa QWidget::backgroundRole()
*/

/*!
    \variable QStyleOptionQ3ListView::sortColumn
    \brief The sort column of the list view.

    \sa Q3ListView::sortColumn()
*/

/*!
    \variable QStyleOptionQ3ListView::itemMargin
    \brief The margin for items in the list view.

    \sa Q3ListView::itemMargin()
*/

/*!
    \variable QStyleOptionQ3ListView::treeStepSize
    \brief The number of pixel to offset children items from their parents.

    \sa Q3ListView::treeStepSize()
*/

/*!
    \variable QStyleOptionQ3ListView::rootIsDecorated
    \brief Whether root items are decorated

    \sa Q3ListView::rootIsDecorated()
*/

/*!
    \class QStyleOptionQ3DockWindow
    \brief The QStyleOptionQ3DockWindow class is used to describe the
    parameters for drawing various parts of a \l Q3DockWindow.

    This class is used for drawing the old Q3DockWindow and its
    parts. It is not recommended for new classes.
*/

/*!
    Constructs a QStyleOptionQ3DockWindow. The member variables are
    initialized to default values.
*/

QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow()
    : QStyleOption(Version, SO_Q3DockWindow), docked(false), closeEnabled(false)
{
}

/*!
    \internal
*/
QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow(int version)
    : QStyleOption(version, SO_Q3DockWindow), docked(false), closeEnabled(false)
{
}

/*!
    \fn QStyleOptionQ3DockWindow::QStyleOptionQ3DockWindow(const QStyleOptionQ3DockWindow &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionQ3DockWindow::Type

    Equals SO_Q3DockWindow.
*/

/*!
    \variable QStyleOptionQ3DockWindow::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionQ3DockWindow::docked
    \brief Indicates that the dock window is currently docked.
*/

/*!
    \variable QStyleOptionQ3DockWindow::closeEnabled
    \brief Indicates that the dock window has a close button.
*/

/*!
    \class QStyleOptionDockWidget
    \brief The QStyleOptionDockWidget class is used to describe the
    parameters for drawing a dock window.
*/

/*!
    Constructs a QStyleOptionDockWidget. The member variables are
    initialized to default values.
*/

QStyleOptionDockWidget::QStyleOptionDockWidget()
    : QStyleOption(Version, SO_DockWidget), movable(false)
{
}

/*!
    \internal
*/
QStyleOptionDockWidget::QStyleOptionDockWidget(int version)
    : QStyleOption(version, SO_DockWidget), closable(false),
      movable(false), floatable(false)
{
}

/*!
    \fn QStyleOptionDockWidget::QStyleOptionDockWidget(const QStyleOptionDockWidget &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionDockWidget::Type

    Equals SO_DockWidget.
*/

/*!
    \variable QStyleOptionDockWidget::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionDockWidget::title
    \brief The title of the dock window
*/

/*!
    \variable QStyleOptionDockWidget::closable
    \brief Indicates that the dock window is closable.
*/

/*!
    \variable QStyleOptionDockWidget::movable
    \brief Indicates that the dock window is movable.
*/

/*!
    \variable QStyleOptionDockWidget::floatable
    \brief Indicates that the dock window is floatable.
*/

/*!
    \class QStyleOptionToolButton
    \brief The QStyleOptionToolButton class is used to describe the
    parameters for drawing a tool button.

    The QStyleOptionToolButton class is used for drawing QToolButton.
*/

/*!
    \enum QStyleOptionToolButton::ToolButtonFeature
    Describes the various features that a tool button can have.

    \value None A normal tool button.
    \value Arrow The tool button is an arrow.
    \value Menu The tool button has a menu.
    \value PopupDelay There is a delay to showing the menu.

    \sa features, QToolButton::toolButtonStyle(), QToolButton::popupMode()
*/

/*!
    Constructs a QStyleOptionToolButton. The members variables are
    initialized to default values.
*/

QStyleOptionToolButton::QStyleOptionToolButton()
    : QStyleOptionComplex(Version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
    , toolButtonStyle(Qt::ToolButtonIconOnly)
{
}

/*!
    \internal
*/
QStyleOptionToolButton::QStyleOptionToolButton(int version)
    : QStyleOptionComplex(version, SO_ToolButton), features(None), arrowType(Qt::DownArrow)
    , toolButtonStyle(Qt::ToolButtonIconOnly)

{
}

/*!
    \fn QStyleOptionToolButton::QStyleOptionToolButton(const QStyleOptionToolButton &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionToolButton::Type

    Equals SO_ToolButton.
*/

/*!
    \variable QStyleOptionToolButton::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionToolButton::features
    \brief The features of the tool button.

    This variable is a bitwise OR describing the features of the button.

    \sa ToolButtonFeature
*/

/*!
    \variable QStyleOptionToolButton::icon
    \brief The icon for the tool button.

    \sa iconSize
*/

/*!
    \variable QStyleOptionToolButton::iconSize
    \brief the size of the icon for the tool button
*/

/*!
    \variable QStyleOptionToolButton::text
    \brief The text of the tool button.

    This value is only used if toolButtonStyle is Qt::ToolButtonTextUnderIcon,
    Qt::ToolButtonTextBesideIcon, or Qt::ToolButtonTextOnly
*/

/*!
    \variable QStyleOptionToolButton::arrowType
    \brief The direction of the arrow for the tool button

    This value is only used if \l features includes \l Arrow.
*/

/*!
    \variable QStyleOptionToolButton::toolButtonStyle
    \brief Used to describe the appearance of a tool button

    \sa QToolButton::toolButtonStyle()
*/

/*!
    \variable QStyleOptionToolButton::pos
    \brief The position of the tool button
*/

/*!
    \variable QStyleOptionToolButton::font
    \brief The font that is used for the text.

    This value is only used if toolButtonStyle is Qt::ToolButtonTextUnderIcon,
    Qt::ToolButtonTextBesideIcon, or Qt::ToolButtonTextOnly
*/

/*!
    \class QStyleOptionComboBox
    \brief The QStyleOptionComboBox class is used to describe the
    parameter for drawing a combobox.

    The QStyleOptionComboBox class is used for drawing QComboBox.
*/

/*!
    Creates a QStyleOptionComboBox. The members variables are
    initialized to default values.
*/

QStyleOptionComboBox::QStyleOptionComboBox()
    : QStyleOptionComplex(Version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \internal
*/
QStyleOptionComboBox::QStyleOptionComboBox(int version)
    : QStyleOptionComplex(version, SO_ComboBox), editable(false), frame(true)
{
}

/*!
    \fn QStyleOptionComboBox::QStyleOptionComboBox(const QStyleOptionComboBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionComboBox::Type

    Equals SO_ComboBox.
*/

/*!
    \variable QStyleOptionComboBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionComboBox::editable
    \brief whether or not the combobox is editable or not.

    \sa QComboBox::isEditable()
*/


/*!
    \variable QStyleOptionComboBox::frame
    \brief Indicates whether whether the combo box has a frame.
*/

/*!
    \variable QStyleOptionComboBox::currentText
    \brief The text for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::currentIcon
    \brief The icon for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::iconSize
    \brief The icon size for the current item of the combo box
*/

/*!
    \variable QStyleOptionComboBox::popupRect
    \brief The popup rectangle for the combobox.

    This variable is currently unused, you can safely ignore it,

    \sa SC_ComboBoxListBoxPopup
*/

/*!
    \class QStyleOptionToolBox
    \brief The QStyleOptionToolBox class is used to describe the
    parameters needed for drawing a tool box.

    The QStyleOptionToolBox class is used for drawing QToolBox.
*/

/*!
    Creates a QStyleOptionToolBox. The members variables are
    initialized to default values.
*/

QStyleOptionToolBox::QStyleOptionToolBox()
    : QStyleOption(Version, SO_ToolBox)
{
}

/*!
    \internal
*/
QStyleOptionToolBox::QStyleOptionToolBox(int version)
    : QStyleOption(version, SO_ToolBox)
{
}

/*!
    \fn QStyleOptionToolBox::QStyleOptionToolBox(const QStyleOptionToolBox &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionToolBox::Type

    Equals SO_ToolBox.
*/

/*!
    \variable QStyleOptionToolBox::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionToolBox::icon
    \brief The icon for the tool box tab.
*/

/*!
    \variable QStyleOptionToolBox::text
    \brief The text for the tool box tab.
*/

#ifndef QT_NO_RUBBERBAND
/*!
    \class QStyleOptionRubberBand
    \brief The QStyleOptionRubberBand class is used to describe the
    parameters needed for drawing a rubber band.

    The QStyleOptionRubberBand class is used for drawing QRubberBand.
*/

/*!
    Creates a QStyleOptionRubberBand. The members variables are
    initialized to default values.
*/

QStyleOptionRubberBand::QStyleOptionRubberBand()
    : QStyleOption(Version, SO_RubberBand)
{
}

/*!
    \internal
*/
QStyleOptionRubberBand::QStyleOptionRubberBand(int version)
    : QStyleOption(version, SO_RubberBand)
{
}

/*!
    \fn QStyleOptionRubberBand::QStyleOptionRubberBand(const QStyleOptionRubberBand &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionRubberBand::Type

    Equals SO_RubberBand.
*/

/*!
    \variable QStyleOptionRubberBand::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionRubberBand::shape
    \brief The shape of the rubber band.
*/

/*!
    \variable QStyleOptionRubberBand::opaque
    \brief Whether the rubber band is required to be drawn in an opque style.
*/
#endif // QT_NO_RUBBERBAND

/*!
    \class QStyleOptionTitleBar
    \brief The QStyleOptionTitleBar class is used to describe the
    parameters for drawing a title bar.

    The QStyleOptionTitleBar class is used to draw the title bars of
    QWorkspace's MDI children.
*/

/*!
    Constructs a QStyleOptionTitleBar. The members variables are
    initialized to default values.
*/

QStyleOptionTitleBar::QStyleOptionTitleBar()
    : QStyleOptionComplex(Version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}

/*!
    \fn QStyleOptionTitleBar::QStyleOptionTitleBar(const QStyleOptionTitleBar &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionTitleBar::Type

    Equals SO_TitleBar.
*/

/*!
    \variable QStyleOptionTitleBar::Version

    Equals 1.
*/

/*!
    \internal
*/
QStyleOptionTitleBar::QStyleOptionTitleBar(int version)
    : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0)
{
}


/*!
    \variable QStyleOptionTitleBar::text
    \brief The text of the title bar.
*/

/*!
    \variable QStyleOptionTitleBar::icon
    \brief The icon for the title bar.
*/

/*!
    \variable QStyleOptionTitleBar::titleBarState
    \brief The state of the title bar.

    This is basically the window state of the underlying widget.

    \sa QWidget::windowState()
*/

/*!
    \variable QStyleOptionTitleBar::titleBarFlags
    \brief The widget flags for the title bar.

    \sa Qt::WFlags
*/

/*!
    \class QStyleOptionViewItem
    \brief The QStyleOptionViewItem class is used to describe the
    parameters used to draw an item in a view widget.

    The QStyleOptionViewItem class is used by Qt's model/view classes
    to draw their items.

    \sa {model-view-programming.html}{Model/View Programming}
*/

/*!
    \enum QStyleOptionViewItem::Position

    This enum describes the position of the item's decoration.

    \value Left On the left of the text.
    \value Right On the right of the text.
    \value Top Above the text.
    \value Bottom Below the text.

    \sa decorationPosition
*/

/*!
    \variable QStyleOptionViewItem::showDecorationSelected

    \brief Whether the decoration should be highlighted on selected items.

    If this option is true, the branch and any decorations on selected items
    should be highlighted, indicating that the item is selected; otherwise, no
    highlighting is required.

    \sa QStyle::SH_ItemView_ShowDecorationSelected, QAbstractItemView
*/

/*!
    \variable QStyleOptionViewItem::textElideMode

    \brief Where ellipsis should be added for text that is too long to fit
    into an item.

    \sa Qt::TextElideMode, QStyle::SH_ItemView_EllipsisLocation
*/

/*!
    Constructs a QStyleOptionViewItem. The members variables are
    initialized to default values.
*/

QStyleOptionViewItem::QStyleOptionViewItem()
    : QStyleOption(Version, SO_ViewItem),
      displayAlignment(0), decorationAlignment(0),
      textElideMode(Qt::ElideMiddle), decorationPosition(Left),
      showDecorationSelected(false)
{
}

/*!
    \internal
*/
QStyleOptionViewItem::QStyleOptionViewItem(int version)
    : QStyleOption(version, SO_ViewItem),
      displayAlignment(0), decorationAlignment(0),
      textElideMode(Qt::ElideMiddle), decorationPosition(Left),
      showDecorationSelected(false)
{
}

/*!
    \fn QStyleOptionViewItem::QStyleOptionViewItem(const QStyleOptionViewItem &other)

    Constructs a copy of the \a other style option.
*/

/*!
    \variable QStyleOptionViewItem::Type

    Equals SO_ViewItem.
*/

/*!
    \variable QStyleOptionViewItem::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionViewItem::displayAlignment
    \brief The alignment of the display value for the item.
*/

/*!
    \variable QStyleOptionViewItem::decorationAlignment
    \brief The alignment of the decoration for the item.
*/

/*!
    \variable QStyleOptionViewItem::decorationPosition
    \brief The position of the decoration for the item.

    \sa Position
*/

/*!
    \variable QStyleOptionViewItem::decorationSize
    \brief The size of the decoration for the item.

    \sa decorationAlignment, decorationPosition
*/

/*!
    \variable QStyleOptionViewItem::font
    \brief The font used for the item

    \sa QFont
*/

/*!
    \fn T qstyleoption_cast<T>(const QStyleOption *option)
    \relates QStyleOption

    Returns a T or 0 depending on the \l{QStyleOption::type}{type}
    and \l{QStyleOption::version}{version} of \a option.

    Example:

    \code
        void MyStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget)
        {
            if (element == PE_FocusRect) {
                const QStyleOptionFocusRect *focusRectOption =
                        qstyleoption_cast<const QStyleOptionFocusRect *>(option);
                if (focusRectOption) {
                    ...
                }
            }
            ...
        }
    \endcode

    \sa QStyleOption::type, QStyleOption::version
*/

/*!
    \fn T qstyleoption_cast<T>(QStyleOption *option)
    \overload
    \relates QStyleOption

    Returns a T or 0 depending on the type of \a option.
*/

#ifndef QT_NO_TABWIDGET
/*!
    \class QStyleOptionTabWidgetFrame
    \brief The QStyleOptionTabWidgetFrame class is used to describe the
    parameters for drawing the frame around a tab widget.

    QStyleOptionTabWidgetFrame is used for drawing QTabWidget.
*/

/*!
    Constructs a QStyleOptionTabWidgetFrame. The members variables
    are initialized to default values.
*/
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame()
    : QStyleOption(Version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
      shape(QTabBar::RoundedNorth)
{
}

/*!
    \fn QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other)

    Constructs a copy of \a other.
*/

/*! \internal */
QStyleOptionTabWidgetFrame::QStyleOptionTabWidgetFrame(int version)
    : QStyleOption(version, SO_TabWidgetFrame), lineWidth(0), midLineWidth(0),
      shape(QTabBar::RoundedNorth)
{
}

/*!
    \variable QStyleOptionTabWidgetFrame::Type

    Equals SO_TabWidgetFrame.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::lineWidth
    \brief The line width for drawing the panel.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::midLineWidth
    \brief The mid-line width for drawing the panel. This is usually used in
    drawing sunken or raised frames.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::shape
    \brief The tab shape used to draw the tabs.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::tabBarSize
    \brief The size of the tab bar.
*/

/*!
    \variable QStyleOptionTabWidgetFrame::rightCornerWidgetSize
    \brief The size of the right-corner widget.
*/

/*! \variable QStyleOptionTabWidgetFrame::leftCornerWidgetSize
    \brief The size of the left-corner widget.
*/
#endif // QT_NO_TABWIDGET

#ifndef QT_NO_TABBAR

/*!
    \class QStyleOptionTabBarBase
    \brief The QStyleOptionTabBarBase class is used to describe the
    the base of a tabbar. That is the part that the tabbar usually overlaps with.

   This is drawn by a standalone QTabBar (one that isn't part of a QTabWidget).

   \sa QTabBar::drawBase()
*/

/*!
    Construct a QStyleOptionTabBarBase. The members are given default values.
*/
QStyleOptionTabBarBase::QStyleOptionTabBarBase()
    : QStyleOption(Version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*! \internal */
QStyleOptionTabBarBase::QStyleOptionTabBarBase(int version)
    : QStyleOption(version, SO_TabBarBase), shape(QTabBar::RoundedNorth)
{
}

/*!
    \fn QStyleOptionTabBarBase::QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other)

    Constructs a copy of \a other.
*/
/*!
    \variable QStyleOptionTabBarBase::Type

    Equals SO_TabBarBase.
*/

/*!
    \variable QStyleOptionTabBarBase::Version

    Equals 1.
*/

/*!
    \variable QStyleOptionTabBarBase::shape

    The shape of the tabbar.
*/

/*!
    \variable QStyleOptionTabBarBase::tabBarRect

    The rectangle containing all the tabs.
*/

/*!
    \variable QStyleOptionTabBarBase::selectedTabRect

    The rectangle containing the selected tab

    This is within the bounds of the tabBarRect.
*/

#endif // QT_NO_TABBAR

#ifndef QT_NO_SIZEGRIP
/*!
    Constructs a QStyleOptionSizeGrip
*/
QStyleOptionSizeGrip::QStyleOptionSizeGrip()
    : QStyleOptionComplex(Version, Type), corner(Qt::BottomRightCorner)
{
}

/*!
    \fn QStyleOptionSizeGrip::QStyleOptionSizeGrip(const QStyleOptionSizeGrip &other)
    \since 4.2
    Constructs a copy of the \a other style option.
*/

/*!
    \internal
*/
QStyleOptionSizeGrip::QStyleOptionSizeGrip(int version)
    : QStyleOptionComplex(version, Type), corner(Qt::BottomRightCorner)
{
}

/*!
    \variable QStyleOptionSizeGrip::corner

    The corner in which the size grip is located
*/
#endif // QT_NO_SIZEGRIP

/*!
    \class QStyleHintReturn
    \brief The QStyleHintReturn class provides style hints that return more
    than basic data types.

    \ingroup appearance

    QStyleHintReturn and its subclasses are used to pass information
    from a style back to the querying widget. This is most useful
    when the return value from QStyle::styleHint() does not provide enough
    detail; for example, when a mask is to be returned.

    \omit
    ### --Sam
    \endomit
*/

/*!
    \enum QStyleHintReturn::HintReturnType

    \value SH_Default QStyleHintReturn
    \value SH_Mask \l QStyle::SH_RubberBand_Mask QStyle::SH_FocusFrame_Mask
*/

/*!
    \variable QStyleHintReturn::Type

    Equals SH_Default.
*/

/*!
    \variable QStyleHintReturn::Version

    Equals 1.
*/

/*!
    \variable QStyleHintReturn::type
    \brief the type of the style hint container

    \sa HintReturnType
*/

/*!
    \variable QStyleHintReturn::version
    \brief the version of the style hint return container

    This value can be used by subclasses to implement extensions
    without breaking compatibility. If you use qstyleoption_cast<T>(), you
    normally don't need to check it.
*/

/*!
    Constructs a QStyleHintReturn with version \a version and type \a
    type.

    The version has no special meaning for QStyleHintReturn; it can be
    used by subclasses to distinguish between different version of
    the same hint type.

    \sa QStyleOption::version, QStyleOption::type
*/

QStyleHintReturn::QStyleHintReturn(int version, int type)
    : version(version), type(type)
{
}

/*!
    \internal
*/

QStyleHintReturn::~QStyleHintReturn()
{

}

/*!
    \class QStyleHintReturnMask
    \brief The QStyleHintReturnMask class provides style hints that return a QRegion.

    \ingroup appearance

    \omit
    ### --Sam
    \endomit
*/

/*!
    \variable QStyleHintReturnMask::region

    \brief The returned region.

    This variable contains the region for style hints that return a QRegion.
*/

/*!
    Constructs a QStyleHintReturnMask. The member variables are
    initialized to default values.
*/
QStyleHintReturnMask::QStyleHintReturnMask() : QStyleHintReturn(Version, Type)
{
}

/*!
    \variable QStyleHintReturnMask::Type

    Equals SH_Mask.
*/

/*!
    \variable QStyleHintReturnMask::Version

    Equals 1.
*/

/*!
    \fn T qstyleoption_cast<T>(const QStyleHintReturn *hint)
    \relates QStyleHintReturn

    Returns a T or 0 depending on the \l{QStyleHintReturn::type}{type}
    and \l{QStyleHintReturn::version}{version} of \a hint.

    Example:

    \code
        int MyStyle::styleHint(StyleHint stylehint, const QStyleOption *opt,
                               const QWidget *widget, QStyleHintReturn* returnData) const;
        {
            if (stylehint == SH_RubberBand_Mask) {
                const QStyleHintReturnMask *maskReturn =
                        qstyleoption_cast<const QStyleHintReturnMask *>(hint);
                if (maskReturn) {
                    ...
                }
            }
            ...
        }
    \endcode

    \sa QStyleHintReturn::type, QStyleHintReturn::version
*/

/*!
    \fn T qstyleoption_cast<T>(QStyleHintReturn *hint)
    \overload
    \relates QStyleHintReturn

    Returns a T or 0 depending on the type of \a hint.
*/

#if !defined(QT_NO_DEBUG) && !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType)
{
    switch (optionType) {
    case QStyleOption::SO_Default:
        debug << "SO_Default"; break;
    case QStyleOption::SO_FocusRect:
        debug << "SO_FocusRect"; break;
    case QStyleOption::SO_Button:
        debug << "SO_Button"; break;
    case QStyleOption::SO_Tab:
        debug << "SO_Tab"; break;
    case QStyleOption::SO_MenuItem:
        debug << "SO_MenuItem"; break;
    case QStyleOption::SO_Frame:
        debug << "SO_Frame"; break;
    case QStyleOption::SO_ProgressBar:
        debug << "SO_ProgressBar"; break;
    case QStyleOption::SO_ToolBox:
        debug << "SO_ToolBox"; break;
    case QStyleOption::SO_Header:
        debug << "SO_Header"; break;
    case QStyleOption::SO_Q3DockWindow:
        debug << "SO_Q3DockWindow"; break;
    case QStyleOption::SO_DockWidget:
        debug << "SO_DockWidget"; break;
    case QStyleOption::SO_Q3ListViewItem:
        debug << "SO_Q3ListViewItem"; break;
    case QStyleOption::SO_ViewItem:
        debug << "SO_ViewItem"; break;
    case QStyleOption::SO_TabWidgetFrame:
        debug << "SO_TabWidgetFrame"; break;
    case QStyleOption::SO_TabBarBase:
        debug << "SO_TabBarBase"; break;
    case QStyleOption::SO_RubberBand:
        debug << "SO_RubberBand"; break;
    case QStyleOption::SO_Complex:
        debug << "SO_Complex"; break;
    case QStyleOption::SO_Slider:
        debug << "SO_Slider"; break;
    case QStyleOption::SO_SpinBox:
        debug << "SO_SpinBox"; break;
    case QStyleOption::SO_ToolButton:
        debug << "SO_ToolButton"; break;
    case QStyleOption::SO_ComboBox:
        debug << "SO_ComboBox"; break;
    case QStyleOption::SO_Q3ListView:
        debug << "SO_Q3ListView"; break;
    case QStyleOption::SO_TitleBar:
        debug << "SO_TitleBar"; break;
    case QStyleOption::SO_CustomBase:
        debug << "SO_CustomBase"; break;
    case QStyleOption::SO_GroupBox:
        debug << "SO_GroupBox"; break;
    case QStyleOption::SO_ToolBar:
        debug << "SO_ToolBar"; break;
    case QStyleOption::SO_ComplexCustomBase:
        debug << "SO_ComplexCustomBase"; break;
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, const QStyleOption &option)
{
    debug << "QStyleOption(";
    debug << QStyleOption::OptionType(option.type);
    debug << "," << (option.direction == Qt::RightToLeft ? "RightToLeft" : "LeftToRight");
    debug << "," << option.state;
    debug << "," << option.rect;
    debug << ")";
    return debug;
}
#endif
