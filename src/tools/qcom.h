#ifndef QCOM_H
#define QCOM_H

#ifndef QT_H
#include <qstringlist.h>
#include <quuid.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

class QObject;
struct QUInterfaceDescription;
struct QUObject;

#define QRESULT		unsigned long
#define QS_OK		(QRESULT)0x00000000
#define QS_FALSE	(QRESULT)0x00000001

#define QE_NOTIMPL      (QRESULT)0x80000001
#define QE_OUTOFMEMORY  (QRESULT)0x80000002
#define QE_INVALIDARG	(QRESULT)0x80000003
#define QE_NOINTERFACE	(QRESULT)0x80000004
#define QE_NOCOMPONENT	(QRESULT)0x80000005


// {1D8518CD-E8F5-4366-99E8-879FD7E482DE}
#ifndef IID_QUnknown
#define IID_QUnknown QUuid(0x1d8518cd, 0xe8f5, 0x4366, 0x99, 0xe8, 0x87, 0x9f, 0xd7, 0xe4, 0x82, 0xde)
#endif

struct Q_EXPORT QUnknownInterface
{
    virtual QRESULT queryInterface( const QUuid&, QUnknownInterface** ) = 0;
    virtual ulong   addRef() = 0;
    virtual ulong   release() = 0;
};


// {DE56512E-4E9F-4b76-A3C2-D1E2EF42F1AC}//### number is fake
#ifndef IID_QDispatch
#define IID_QDispatch QUuid( 0xde56512e, 0x4e9f, 0x4b76, 0xa3, 0xc2, 0xd1, 0xe2, 0xef, 0x42, 0xf1, 0xac )
#endif

// the dispatch interface that inherits the unknown interface.. It is
// used to explore interfaces during runtime and to do dynamic calls.
struct Q_EXPORT QDispatchInterface : public QUnknownInterface
{
    // returns the interface description of this dispatch interface.
    virtual const QUInterfaceDescription* interfaceDescription() const = 0;

    // returns the event description of this dispatch interface.
    virtual const QUInterfaceDescription* eventsDescription() const = 0;

    // invokes method id with parameters V*. Returns some sort of
    // exception code.
    virtual QRESULT invoke( int id, QUObject* o ) = 0;

    // installs listener as event listener
    virtual void installListener( QDispatchInterface* listener ) = 0;

    // remove listener as event listener
    virtual void removeListener( QDispatchInterface* listener ) = 0;
};

template <class T> class Q_EXPORT QInterfacePtr
{
public:
    QInterfacePtr():iface(0){}

    QInterfacePtr( T* i) {
	if ( (iface = i) )
	    iface->addRef();
    }

    QInterfacePtr(const QInterfacePtr<T> &p) {
	if ( (iface = p.iface) )
	    iface->addRef();
    }

    ~QInterfacePtr() {
	if ( iface )
	    iface->release();
    }

    QInterfacePtr<T> &operator=(const QInterfacePtr<T> &p) {
	if ( iface != p.iface ) {
	    if ( iface )
		iface->release();
	    if ( (iface = p.iface) )
		iface->addRef();
	}
	return *this;
    }

    QInterfacePtr<T> &operator=(T* i) {
	if (iface != i ) {
	    if ( iface )
		iface->release();
	    if ( (iface = i) )
		iface->addRef();
	}
	return *this;
    }

    bool operator==( const QInterfacePtr<T> &p ) const { return iface == p.iface; }

    bool operator!= ( const QInterfacePtr<T>& p ) const {  return !( *this == p ); }

    bool isNull() const { return !iface; }

    T* operator->() const { return iface; }

    T& operator*() const { return *iface; }

    operator T*() const { return iface; }

    QUnknownInterface** operator &() const { return (QUnknownInterface**)&iface; }
    T** operator &() { return &iface; }

private:
    T* iface;
};


//####### WARNING: uuid is fake right now
// {721F033C-D7D0-4462-BD67-1E8C8FA1C741}
#ifndef IID_QObject
#define IID_QObject QUuid( 0x721f033c, 0xd7d0, 0x4462, 0xbd, 0x67, 0x1e, 0x8c, 0x8f, 0xa1, 0xc7, 0x41)
#endif

struct Q_EXPORT QObjectInterface
{
    virtual QObject*   qObject() = 0;
};

// {5F3968A5-F451-45b1-96FB-061AD98F926E}
#ifndef IID_QComponentInformation
#define IID_QComponentInformation QUuid(0x5f3968a5, 0xf451, 0x45b1, 0x96, 0xfb, 0x6, 0x1a, 0xd9, 0x8f, 0x92, 0x6e)
#endif

struct Q_EXPORT QComponentInformationInterface : public QUnknownInterface
{
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString author() const = 0;
    virtual QString version() const = 0;
};

// {6CAA771B-17BB-4988-9E78-BA5CDDAAC31E}
#ifndef IID_QComponentFactory
#define IID_QComponentFactory QUuid( 0x6caa771b, 0x17bb, 0x4988, 0x9e, 0x78, 0xba, 0x5c, 0xdd, 0xaa, 0xc3, 0x1e)
#endif

struct Q_EXPORT QComponentFactoryInterface : public QUnknownInterface
{
    virtual QRESULT createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface** instance, QUnknownInterface *outer ) = 0;
};

// {D16111D4-E1E7-4C47-8599-24483DAE2E07}
#ifndef IID_QLibrary
#define IID_QLibrary QUuid( 0xd16111d4, 0xe1e7, 0x4c47, 0x85, 0x99, 0x24, 0x48, 0x3d, 0xae, 0x2e, 0x07)
#endif

struct Q_EXPORT QLibraryInterface : public QUnknownInterface
{
    virtual bool    init() = 0;
    virtual void    cleanup() = 0;
    virtual bool    canUnload() const = 0;
};

// {3F8FDC44-3015-4f3e-B6D6-E4AAAABDEAAD}
#ifndef IID_QFeatureList
#define IID_QFeatureList QUuid(0x3f8fdc44, 0x3015, 0x4f3e, 0xb6, 0xd6, 0xe4, 0xaa, 0xaa, 0xbd, 0xea, 0xad)
#endif

struct Q_EXPORT QFeatureListInterface : public QUnknownInterface
{
    virtual QStringList	featureList() const = 0;
};

// {B5FEB5DE-E0CD-4E37-B0EB-8A812499A0C1}
#ifndef IID_QComponentServer
#define IID_QComponentServer QUuid( 0xb5feb5de, 0xe0cd, 0x4e37, 0xb0, 0xeb, 0x8a, 0x81, 0x24, 0x99, 0xa0, 0xc1)
#endif

struct Q_EXPORT QComponentServerInterface : public QUnknownInterface
{
    virtual bool    registerComponents( const QString &filepath ) const = 0;
    virtual bool    unregisterComponents() const = 0;
};

// {621F033C-D7D0-4462-BD67-1E8C8FA1C741}
#ifndef IID_QInterfaceList
#define IID_QInterfaceList QUuid( 0x621f033c, 0xd7d0, 0x4462, 0xbd, 0x67, 0x1e, 0x8c, 0x8f, 0xa1, 0xc7, 0x41)
#endif

struct Q_EXPORT QInterfaceListInterface
{
    virtual QUuid   interfaceId( int index ) = 0;
};

class Q_EXPORT QComponentRegistration
{
public:
    static bool registerComponent( const QUuid &cid, const QString &name );
    static bool registerPropertyType( const QUuid &pid, const QString &name );
    static bool registerProperty( const QUuid &cid, const QUuid &pid, const QString &value );

    static bool unregisterComponent( const QUuid &cid );
    static bool unregisterPropertytype( const QUuid &pid );
    static bool unregisterProperty( const QUuid &cid, const QUuid &pid );
};

#ifndef Q_CREATE_INSTANCE
#    define Q_CREATE_INSTANCE( IMPLEMENTATION )		\
	IMPLEMENTATION *i = new IMPLEMENTATION;		\
	QUnknownInterface* iface = 0; 			\
	i->queryInterface( IID_QUnknown, &iface );	\
	return iface;
#endif

#ifndef Q_EXTERN_C
#ifdef __cplusplus
#define Q_EXTERN_C    extern "C"
#else
#define Q_EXTERN_C    extern
#endif
#endif

#if defined(QT_DEBUG)
#define Q_REFCOUNT  ulong addRef() {static bool first=TRUE;if(first){first = FALSE;if (ref) qWarning("RefCounter not initialized: %s", __FILE__);}return ref++;} \
		    ulong release() {if(!--ref){delete this;return 0;}return ref;}
#else
#define Q_REFCOUNT  ulong addRef() {return ref++;} \
		    ulong release() {if(!--ref){delete this;return 0;}return ref;}
#endif

#if defined(QT_THREAD_SUPPORT)
#define QT_THREADED_BUILD 1
#else
#define QT_THREADED_BUILD 0
#endif

#if defined(QT_DEBUG)
#define QT_DEBUG_BUILD 1
#else
#define QT_DEBUG_BUILD 0
#endif

#ifndef Q_EXPORT_INTERFACE
#    ifdef Q_WS_WIN
#	ifdef Q_CC_BOR
#	    define Q_EXPORT_INTERFACE() \
		extern Q_EXPORT QApplication *qApp; \
		extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
		Q_EXTERN_C __declspec(dllexport) int __stdcall ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
		{ \
		    if ( !qApp && theApp ) \
			qt_ucm_initialize( theApp ); \
		    if ( mt ) \
			*mt = QT_THREADED_BUILD; \
		    if ( debug ) \
		        *debug = QT_DEBUG_BUILD; \
		    return QT_VERSION; \
		} \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* __stdcall ucm_instantiate()
#	else
#	    define Q_EXPORT_INTERFACE() \
		extern Q_EXPORT QApplication *qApp; \
		extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
		Q_EXTERN_C __declspec(dllexport) int ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
		{ \
		    if ( !qApp && theApp ) \
			qt_ucm_initialize( theApp ); \
		    if ( mt ) \
			*mt = QT_THREADED_BUILD; \
		    if ( debug ) \
		        *debug = QT_DEBUG_BUILD; \
		    return QT_VERSION; \
		} \
		Q_EXTERN_C __declspec(dllexport) QUnknownInterface* ucm_instantiate()
#	endif
#    else
#	define Q_EXPORT_INTERFACE() \
	    extern Q_EXPORT QApplication *qApp; \
	    extern Q_EXPORT void qt_ucm_initialize( QApplication *theApp ); \
	    Q_EXTERN_C int ucm_initialize( QApplication *theApp, bool *mt, bool *debug ) \
	    { \
		if ( !qApp && theApp ) \
		    qt_ucm_initialize( theApp ); \
		if ( mt ) \
		    *mt = QT_THREADED_BUILD; \
		if ( debug ) \
		    *debug = QT_DEBUG_BUILD; \
		return QT_VERSION; \
	    } \
	    Q_EXTERN_C QUnknownInterface* ucm_instantiate()
#    endif
#    define Q_EXPORT_COMPONENT() Q_EXPORT_INTERFACE()
#endif

#endif //QT_NO_COMPONENT

#endif //QCOM_H
