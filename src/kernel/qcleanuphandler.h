#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#ifndef QT_H
#include <qlist.h>
#include <qguardedptr.h>
#endif // QT_H

template<class Type>
class Q_EXPORT QGuardedCleanupHandler
{
public:
    ~QGuardedCleanupHandler() { clear(); }

    void add( Type* object )
    {
	cleanupObjects.insert( 0, new QGuardedPtr<Type>(object) );
    }

    void remove( Type *object )
    {
	QListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( (Type *)guard == object ) {
		cleanupObjects.removeRef( guard );
		delete guard;
		break;
	    }
	}
    }

    bool isEmpty() const
    {
	QListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( (Type*)*guard )
		return FALSE;
	}
	return TRUE;
    }

    void clear() {
	QListIterator<QGuardedPtr<Type> > it( cleanupObjects );
	it.toLast();
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    --it;
	    cleanupObjects.removeRef( guard );
	    delete (Type*)*guard;
	    delete guard;
	}
    }

private:
    QList<QGuardedPtr<Type> > cleanupObjects;
};

template<class Type>
class Q_EXPORT QCleanupHandler
{
public:
    ~QCleanupHandler() { clear(); }

    void add( Type* object )
    {
	if ( object )
	    cleanupObjects.insert( 0, object );
    }

    void remove( Type *object )
    {
	if ( object )
	    cleanupObjects.removeRef( object );
    }

    bool isEmpty() const
    {
	return cleanupObjects.isEmpty();
    }

    void clear()
    {
	QListIterator<Type> it( cleanupObjects );
	it.toLast();
	while ( it.current() ) {
	    Type* object = it.current();
	    --it;
	    cleanupObjects.removeRef( object );
	    delete object;
	}
    }

private:
    QList<Type> cleanupObjects;
};

#endif //QCLEANUPHANDLER_H
