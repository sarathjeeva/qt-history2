#include "qaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "qptrdict.h"
#include "qobject.h"

/*!
  \class QAccessibleInterface qaccessible.h
  \brief The QAccessibleInterface class is an interface that exposes information about accessible objects.
  \preliminary
*/

/*!
  \enum QAccessible::State
  This enum type defines bitflags that can be combined to indicate the state of the accessible object.
  
  \value Disabled
*/

/*!
  \enum QAccessible::Event
  This enum type defines event types why the state of the accessible object has changed.

  \value Focus
*/

/*!
  \enum QAccessible::Role
*/

/*!
  \enum QAccessible::NavDirection

  \value NavDirectionMin
  \value NavUp
  \value NavDown
  \value NavLeft
  \value NavRight
  \value NavNext
  \value NavPrevious
  \value NavFirstChild
  \value NavLastChild
  \value NavDirectionMax
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::hitTest( int x, int y, int *who ) const

  Returns whether the screen coordinates \a x, \a y are within the boundaries of this object.
  If the tested point is on a child of this object which implements an QAccessibilityInterface
  itself, that interface is returned. Otherwise, this interface is returned and the value of \a who
  is set to the identifier of any child element. \a who is set to 0 if the tested point is on
  the the object itself.

  This function returns NULL if the tested point is outside the boundaries of this object.

  All visual objects provide this information.
*/

/*!
  \fn QRect QAccessibleInterface::location( int who ) const

  Returns the object's current location in screen coordinates if \a who is 0,
  or the location of the object's subelement with ID \a who.
  
  All visual objects provide this information.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::navigate( NavDirection direction, int *startEnd ) const

  This function traverses to another object. \a direction specifies in which direction 
  to navigate, and the value of \a startEnd specifies the start point of the navigation, 
  which is either 0 if the navigation starts at the object itself, or an ID of one of 
  the object's subelements.

  The function returns the QAccessibleInterface implementation of the object located at 
  the direction specified, or this QAccessibleInterface if the target object is a subelement
  of this object. \a startEnd is then set to the ID of this subelement.
*/

/*!
  \fn int QAccessibleInterface::childCount() const

  Returns the number of accessible child objects. Every subelement of this
  object that can provide accessibility information is a child, e.g. items
  in a list view.
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::child( int who ) const
*/

/*!
  \fn QAccessibleInterface* QAccessibleInterface::parent() const

  Returns the QAccessibleInterface implementation of the parent object, or NULL if there
  is no such object.

  All objects provide this information.
*/

/*!
  \fn QAccessible::State QAccessibleInterface::state( int who ) const

  Returns the current state of the object if \a who is 0, or the state of 
  the object's subelement element with ID \a who. All objects have a state.

  \sa role()
*/

/*!
  \fn QAccessible::Role QAccessibleInterface::role( int who ) const

  Returns the role of the object if \a who is 0, or the role of the object's 
  subelement with ID \a who. The role of an object is usually static. 
  All accessible objects have a role.

  \sa state()
*/

/*!
  \fn QString QAccessibleInterface::name( int who ) const

  Returns the current name of the object if \a who is 0, or the name of 
  the object's subelement with ID \a who.

  The \e name is a string used by clients to identify, 
  find or announce an accessible object for the user.

  All object have a name that has to be unique within their
  container.

  \sa description(), help()
*/

/*!
  \fn QString QAccessibleInterface::description( int who ) const

  Returns the current description text of the object if \a who is 0, or the
  name of the object's subelement with ID \a who.

  An accessible object's \e description provides a textual description about
  an object's visual appearance. The description is primarily used to provide
  greater context for low-vision or blind users, but is also used for context
  searching or other applications.

  Not all objects have a description. An "Ok" button would not need a description, 
  but a toolbutton that shows a picture of a smiley would.

  \sa name(), help()
*/

/*!
  \fn QString QAccessibleInterface::help( int who ) const

  Returns the current help text of the object if \a who is 0, or the help
  text of the object's subelement with ID \a who.

  The \e help text provides information about the function of an 
  accessible object. Not all objects provide this information.

  \sa name(), description()
*/

/*!
  \fn QString QAccessibleInterface::value( int who ) const

  Returns the current value of the object if \a who is 0, or the value
  of the object's subelement with ID \a who.

  The \e value of an accessible object represents visual information
  contained by the object, e.g. the text in a line edit. Usually, the
  value can be modified by the user.

  Not all objects have a value, e.g. static text labels don't, and some
  objects have a state that already is the value, e.g. toggle buttons.

  \sa name(), state()
*/

/*!
  \fn bool QAccessibleInterface::doDefaultAction( int who )
*/

/*!
  \fn QString QAccessibleInterface::defaultAction( int who ) const

  Returns the current default action of the object if \a who is 0, or the
  value of the object's subelement with ID \a who.

  An accessible object's \e defaultAction describes the object's primary
  method of manipulation, and should be a verb or a short phrase, e.g. 
  "Press" for a button.

  \sa help()
*/

/*!
  \fn QString QAccessibleInterface::accelerator( int who ) const

  Returns the keyboard shortcut for this object if \a who is 0, or
  the keyboard shortcut of the object's subelement with ID \a who.

  A keyboard shortcut is an underlined character in the text of a menu, menu item 
  or control, and is either the character itself, or a combination of this character
  and a modifier key like ALT, CTRL or SHIFT.

  Command controls like tool buttons also have shortcut keys and usually display them
  in their tooltip.

  All objects that have a shortcut should provide this information.

  \sa help()
*/

/*!
  \fn QAccessibleInterface *QAccessibleInterface::hasFocus( int *who ) const
*/

static QPtrDict<QAccessibleInterface> *qAccessibleInterface = 0;

QAccessibleInterface *QAccessible::accessibleInterface( QObject *object )
{
    if ( !object )
	return 0;

    QAccessibleInterface *iface = 0;
    if ( qAccessibleInterface )
	iface = qAccessibleInterface->find( object );
    if ( !iface )
	iface = object->accessibleInterface();
    return iface;
}

/*!
  \class QAccessibleObject qaccessible.h
  \brief The QAccessibleObject class implements the QUnknownInterface.
  \preliminary

  This class is mainly provided for convenience. All further implementations 
  of the QAccessibleInterface should use this class as the base class.
*/

/*!
  Creates a QAccessibleObject.
*/
QAccessibleObject::QAccessibleObject( QObject *object )
: ref( 0 ), object_(object)
{
    if ( !qAccessibleInterface )
	qAccessibleInterface = new QPtrDict<QAccessibleInterface>( 73 );
    qAccessibleInterface->insert( object, this );
}

/*!
  Destroys the QAccessibleObject. 
  
  This will only happen if a call to release() decrements the internal 
  reference counter to zero.
*/
QAccessibleObject::~QAccessibleObject()
{
    if ( qAccessibleInterface ) {
	qAccessibleInterface->remove( object_ );
	if ( !qAccessibleInterface->count() ) {
	    delete qAccessibleInterface;
	    qAccessibleInterface = 0;
	}
    }
}

/*!
  Implements the QUnknownInterface function to return provide an interface for
  IID_QAccessible and IID_QUnknown.
*/
void QAccessibleObject::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QAccessible )
	*iface = (QAccessibleInterface*)this;
    else if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    
    if ( *iface )
	(*iface)->addRef();
    return;
}

/*!
  \reimp
*/
ulong QAccessibleObject::addRef()
{
    return ++ref;
}

/*!
  \reimp
*/
ulong QAccessibleObject::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

/*!
  Returns the QObject for which this QAccessibleInterface implementation provides information.
*/
QObject *QAccessibleObject::object() const
{
    return object_;
}

#endif
