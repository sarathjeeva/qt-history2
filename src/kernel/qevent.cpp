/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qevent.cpp#21 $
**
** Implementation of event classes
**
** Author  : Haavard Nord
** Created : 931029
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qevent.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qevent.cpp#21 $";
#endif


void qRemovePostedEvent( QEvent * );		// defined in qapp_xxx.cpp


void QEvent::peErrMsg()				// posted event error message
{
#if defined(CHECK_STATE)
    const char *n = 0;
    switch ( t ) {				// convert type to msg string
	case Event_Timer:
	    n = "Timer";
	    break;
	case Event_MouseButtonPress:
	    n = "MouseButtonPress";
	    break;
	case Event_MouseButtonRelease:
	    n = "MouseButtonRelease";
	    break;
	case Event_MouseButtonDblClick:
	    n = "MouseButtonDblClick";
	    break;
	case Event_MouseMove:
	    n = "MouseMove";
	    break;
	case Event_KeyPress:
	    n = "KeyPress";
	    break;
	case Event_KeyRelease:
	    n = "KeyRelease";
	    break;
	case Event_FocusIn:
	    n = "FocusIn";
	    break;
	case Event_FocusOut:
	    n = "FocusOut";
	    break;
	case Event_Enter:
	    n = "Enter";
	    break;
	case Event_Leave:
	    n = "Leave";
	    break;
	case Event_Paint:
	    n = "Paint";
	    break;
	case Event_Move:
	    n = "Move";
	    break;
	case Event_Resize:
	    n = "Resize";
	    break;
	case Event_Create:
	    n = "Create";
	    break;
	case Event_Destroy:
	    n = "Destroy";
	    break;
	case Event_Close:
	    n = "Close";
	    break;
	case Event_Quit:
	    n = "Quit";
	    break;
    }
    if ( n )
	warning( "QEvent: Posted event %s cannot be stack variable, ignored",
		 n );
    else
	warning( "QEvent: Posted event %d cannot be stack variable, ignored",
		 t );
#endif
    qRemovePostedEvent( this );
}


/*----------------------------------------------------------------------------   \class QEvent qevent.h
  \brief The QEvent class is base class of all
  event classes. Event objects contain event parameters.

  \ingroup event

  The \link QApplication::exec() main event loop\endlink of Qt fetches
  native window system events from the event queue, translates them
  to Qt events and sends the translated events to application objects.

  Generally, events come from the underlying window system, but it is also
  possible to manually send events through the QApplication class using
  QApplication::sendEvent() and QApplication::postEvent().

  Only classes that inherit QObject and reimplement the virtual
  QObject::event() function may receive events.

  The QWidget class reimplements the event() function to
  dispatch an event to an appropriate event handler (virtual function) on
  basis of the event type.

  QWidget::keyPressEvent() and QWidget::mouseMoveEvent() are examples of
  widget event handlers.

  The basic QEvent contains only an event type parameter.  Subclasses of
  QEvent contain additional parameters that descripe the particular event.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QEvent::QEvent( int type )
  Contructs an event object with a \e type. The file qevent.h lists
  all event types.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QEvent::~QEvent()
  Destroys the event.  Reports an error if the event has been
  \link QApplication::postEvent() posted\endlink.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QEvent::type() const
  Returns the event type.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QTimerEvent qevent.h
  \brief The QTimerEvent class contains parameters that describe a
  timer event.

  \ingroup event

  Timer events are sent at regular intervals to objects that have
  started one or more timers.  Each timer has a unique identifier.

  If the interval is zero, the event will be sent on every iteration
  of the event loop.

  The QTimer class provides a high-level programming interface with one-shot
  timers and timer signals instead of events.

  The event handler QWidget::timerEvent() receives timer events.

  \sa QObject::startTimer(), QObject::killTimer(), QObject::killTimers()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QTimerEvent::QTimerEvent( int timerId )
  Constructs a timer event object with the timer identifier set to \e timerId.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QTimerEvent::timerId() const
  Returns the unique timer identifier, which is the same identifier as
  returned from QObject::startTimer().
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QMouseEvent qevent.h
  \brief The QMouseEvent class contains parameters that describe a mouse event.

  \ingroup event

  Mouse events occur when a mouse button is pressed or released inside a
  widget, or when the mouse cursor is moved.
  Mouse move events will only occur when some mouse button is pressed down,
  unless \link QWidget::setMouseTracking() mouse tracking\endlink has been
  enabled.

  Qt make an automatic mouse grab when a mouse button is pressed inside a
  widget, and the widget will continue to receive mouse events until the
  last mouse button is released.

  The QWidget::disable() function disables mouse and keyboard events for a
  widget, and QWidget::enable() enables mouse and keyboard events.

  The QCursor widget has static functions for reading and setting the
  position of the mouse cursor.

  The event handlers QWidget::mousePressEvent(), QWidget::mouseReleaseEvent(),
  QWidget::mouseDoubleClickEvent() and QWidget::mouseMoveEvent() receive
  mouse events.

  \sa QWidget::setMouseTracking(), QWidget::grabMouse()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QMouseEvent::QMouseEvent( int type, const QPoint &pos, int button, int state )

  Constructs a mouse event object.

  The type parameter must be \c Event_MouseButtonPress,
  \c Event_MouseButtonRelease,
  \c Event_MouseButtonDblClick or \c Event_MouseMove.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QPoint &QMouseEvent::pos() const
  Returns the position of the mouse relative to the widget that received the
  event.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QMouseEvent::button() const
  Returns the button that caused the event.

  Possible return values are \c LeftButton, \c RightButton, \c MidButton and
  \c NoButton.

  The button value is always \c NoButton (0) when a mouse move event is
  received.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QMouseEvent::state() const
  Returns the current button state (a combination of mouse buttons and keyboard
  modifiers).

  The returned value is \c LeftButton, \c RightButton, \c MidButton,
  \c ShiftButton, \c ControlButton and \c AltButton OR'ed together.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QKeyEvent qevent.h
  \brief The QKeyEvent class contains parameters that describe a key event.

  \ingroup event

  Key events occur when a key is pressed or released when a widget has
  keyboard input focus.

  A key event contain a special accept flag which tells whether the receiver
  wants the key.  You should answer QKeyEvent::ignore() if the key press
  or release is not handled by your widget.

  The QWidget::disable() function disables mouse and keyboard events for a
  widget, and QWidget::enable() enables mouse and keyboard events.

  The event handlers QWidget::keyPressEvent() and QWidget::keyReleaseEvent()
  receive key events.

  \sa QFocusEvent, QWidget::grabKeyboard()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QKeyEvent::QKeyEvent( int type, int key, uchar ascii, int state )
  Constructs a key event object.

  The \e type parameter must be \c Event_KeyPress or \c Event_KeyRelease.

  If \e key is 0, the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).

  The accept flag is set to TRUE.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QKeyEvent::key() const
  Returns the code if the key that was pressed or released.

  The header file qkeycode.h lists the possible keyboard codes.  These codes
  are independent of the underlying window system.

  Key code 0 means that the event is not a result of a known key (e.g. it
  may be the result of a compose sequence or keyboard macro).
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn uchar QKeyEvent::ascii() const
  Returns the ASCII code of the key that was pressed or released.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QKeyEvent::state() const
  Returns the keyboard modifier flags.

  The returned value is \c ShiftButton, \c ControlButton and \c AltButton
  OR'ed together.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QKeyEvent::isAccepted() const
  Returns TRUE if the receiver of the event wants to keep the key.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QKeyEvent::accept()
  Sets the accept flag of the key event object.

  Setting the accept parameter indicates that the receiver of the event wants
  the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa ignore()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QKeyEvent::ignore()
  Clears the accept flag parameter of the key event object.

  Clearing the accept parameter indicates that the event receiver does
  not want the key event. Unwanted key events are sent to the parent widget.

  The accept flag is set by default.

  \sa accept()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QFocusEvent qevent.h
  \brief The QFocusEvent class contains event parameters for widget focus
  events.

  \ingroup event

  Focus events are sent to widgets when the keyboard input focus changes.

  The event handlers QWidget::focusInEvent() and QWidget::focusOutEvent()
  receive focus events.

  \sa QWidget::setFocus(), QWidget::setAcceptFocus()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QFocusEvent::QFocusEvent( int type )
  Constructs a focus event object.

  The \e type parameter must be either \e Event_FocusIn or \e Event_FocusOut.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QFocusEvent::gotFocus() const
  Returns TRUE if the widget received the text input focus.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QFocusEvent::lostFocus() const
  Returns TRUE if the widget lost the text input focus.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QPaintEvent qevent.h
  \brief The QPaintEvent class contains event parameters for paint events.

  \ingroup event

  Paint events are sent to widgets that need to update themselves, for instance
  when a part of a widget is exposed because an overlying widget is moved away.

  The event handler QWidget::paintEvent() receives paint events.

  \sa QWidget::update(), QWidget::repaint()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPaintEvent::QPaintEvent( const QRect &paintRect )
  Constructs a paint event object with the rectangle that should be updated.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QRect &QPaintEvent::rect() const
  Returns the rectangle that should be updated.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QMoveEvent qevent.h
  \brief The QMoveEvent class contains event parameters for move events.

  \ingroup event

  Move events are sent to widgets that have been moved to a new position
  relative to their parent.

  The event handler QWidget::moveEvent() receives move events.

  \sa QWidget::move(), QWidget::setGeometry()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QMoveEvent::QMoveEvent( const QPoint &pos, const QPoint &oldPos )
  Constructs a move event with the new and old widget positions.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QPoint &QMoveEvent::pos() const
  Returns the new position of the widget, which is the same as
  QWidget::pos().
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QPoint &QMoveEvent::oldPos() const
  Returns the old position of the widget.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QResizeEvent qevent.h
  \brief The QResizeEvent class contains event parameters for resize events.

  \ingroup event

  Resize events are sent to widgets that have been resized.

  The event handler QWidget::resizeEvent() receives resize events.

  \sa QWidget::resize(), QWidget::setGeometry()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QResizeEvent::QResizeEvent( const QSize &size, const QSize &oldSize )
  Constructs a resize event with the new and old widget sizes.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QSize &QResizeEvent::size() const
  Returns the new size of the widget, which is the same as
  QWidget::size().
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QSize &QResizeEvent::oldSize() const
  Returns the old size of the widget.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QCloseEvent qevent.h
  \brief The QCloseEvent class contains parameters that describe a close event.

  \ingroup event

  Close events are sent to widgets the user wants to close, for instance
  by choosing "close" from a window menu.

  Close events contain a special accept flag which tells whether the receiver
  wants the widget to be closed.  When a widget is closed, it is normally
  deleted.  This is true for all widgets except the
  \link QApplication::setMainWidget() main widget\endlink.

  The event handler QWidget::closeEvent() receives close events.

  The default implementation of this event handler accepts the close event.
  If you do not want your widget to be closed and deleted, you should
  reimplement this event handler.

  Alternatively, you can hide you widget but not delete it:
  \code
    void MyWidget::closeEvent( QCloseEvent *e )
    {
	e->ignore();				// do NOT delete this widget
	hide();					// hide it instead
    }
  \endcode

  \sa QWidget::close()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QCloseEvent::QCloseEvent()
  Constructs a close event object with the accept parameter flag set to TRUE.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QCloseEvent::isAccepted() const
  Returns TRUE if the receiver of the event has agreed to close the widget.
  \sa accept(), ignore()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QCloseEvent::accept()
  Sets the accep flag of the close event object.

  Setting the accept flag indicates that the receiver of this event agrees
  to close the widget.

  The accept flag is set by default.

  \warning If you choose to accept in QWidget::closeEvent(),
  the widget will be deleted.

  \sa ignore()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QCloseEvent::ignore()
  Clears the accept flag of the close event object.

  Clearing the accept flag indicates that the receiver of this event does not
  want the widget to close.

  The accept flag is set by default.

  \sa accept()
 ----------------------------------------------------------------------------*/
