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

#ifndef QT_MAC_P_H
#define QT_MAC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qglobal.h"
#include "QtCore/qvariant.h"
#include "private/qcore_mac_p.h"
#include <ApplicationServices/ApplicationServices.h>

#include "QtGui/qpainter.h"
#include "QtGui/qwidget.h"
#include "private/qwidget_p.h"

/* Event masks */
// internal Qt types
const UInt32 kEventClassQt = 'cute'; // Event class for our own Carbon events.
enum {
    //AE types
    typeAEClipboardChanged = 1,
    //types
    typeQWidget = 1,  /* QWidget *  */
    typeMacTimerInfo = 2, /* MacTimerInfo * */
    typeQEventDispatcherMac = 3, /* QEventDispatcherMac * */
    //params
    kEventParamMacTimer = 'qtim',     /* typeMacTimerInfo */
    kEventParamQWidget = 'qwid',   /* typeQWidget */
    kEventParamQEventDispatcherMac = 'qevd', /* typeQEventDispatcherMac */
    //events
    kEventQtRequestSelect = 12,
    kEventQtRequestContext = 13,
    kEventQtRequestMenubarUpdate = 14,
    kEventQtRequestTimer = 15,
    kEventQtRequestShowSheet = 17,
    kEventQtRequestActivate = 18,
    kEventQtRequestSocketAct = 19,
    kEventQtRequestWindowChange = 20
};

class QMacBlockingFunction //implemented in qeventdispatcher_mac.cpp
{
private:
    class Object;
    static Object *block;
public:
    inline QMacBlockingFunction()  { addRef(); }
    inline ~QMacBlockingFunction() { subRef(); }
    static void addRef();
    static void subRef();
    static bool blocking() { return block != 0; }
};

class QMacCocoaAutoReleasePool
{
private:
    void *pool;
public:
    QMacCocoaAutoReleasePool();
    ~QMacCocoaAutoReleasePool();

    inline void *handle() const { return pool; }
};

class Q_GUI_EXPORT QMacWindowChangeEvent
{
private:
    static QList<QMacWindowChangeEvent*> *change_events;
public:
    QMacWindowChangeEvent() {
        if(!change_events)
            change_events = new QList<QMacWindowChangeEvent*>;
        change_events->append(this);
    }
    virtual ~QMacWindowChangeEvent() {
        change_events->removeAll(this);
        if(change_events->isEmpty()) {
            delete change_events;
            change_events = 0;
        }
    }
    static inline void exec(bool flush) {
        if(change_events) {
            bool send_window_changed = !flush;
            if(!send_window_changed) {
                extern bool qt_event_remove_window_change(); //qapplication_mac.cpp
                send_window_changed = qt_event_remove_window_change();
            }
            for(int i = 0; i < change_events->count(); i++) {
                if(send_window_changed)
                    change_events->at(i)->windowChanged();
                if(flush)
                    change_events->at(i)->flushWindowChanged();
            }
        }
    }
protected:
    virtual void windowChanged() = 0;
    virtual void flushWindowChanged() = 0;
};

class QMacCGContext
{
    CGContextRef context;
public:
    QMacCGContext(QPainter *p); //qpaintengine_mac.cpp
    inline QMacCGContext() { context = 0; }
    inline QMacCGContext(const QPaintDevice *pdev) {
        extern CGContextRef qt_mac_cg_context(const QPaintDevice *);
        context = qt_mac_cg_context(pdev);
    }
    inline QMacCGContext(CGContextRef cg, bool takeOwnership=false) {
        context = cg;
        if(!takeOwnership)
            CGContextRetain(context);
    }
    inline QMacCGContext(const QMacCGContext &copy) : context(0) { *this = copy; }
    inline ~QMacCGContext() {
        if(context)
            CGContextRelease(context);
    }
    inline bool isNull() const { return context; }
    inline operator CGContextRef() { return context; }
    inline QMacCGContext &operator=(const QMacCGContext &copy) {
        if(context)
            CGContextRelease(context);
        context = copy.context;
        CGContextRetain(context);
        return *this;
    }
    inline QMacCGContext &operator=(CGContextRef cg) {
        if(context)
            CGContextRelease(context);
        context = cg;
        CGContextRetain(context); //we do not take ownership
        return *this;
    }
};

class QMacPasteboardMime;
class QMimeData;

class QMacPasteboard
{
    struct Promise {
        Promise(QMacPasteboardMime *c, QString m, QVariant d) : convertor(c), mime(m), data(d) { }
        QMacPasteboardMime *convertor;
        QString mime;
        QVariant data;
    };
    QList<Promise> promises;

    PasteboardRef paste;
    uchar mime_type;
    mutable QPointer<QMimeData> mime;
    mutable bool mac_mime_source;
    static OSStatus promiseKeeper(PasteboardRef, PasteboardItemID, CFStringRef, void *);
public:
    QMacPasteboard(PasteboardRef p, uchar mime_type=0);
    QMacPasteboard(uchar mime_type);
    QMacPasteboard(CFStringRef name=0, uchar mime_type=0);
    ~QMacPasteboard();

    bool hasFlavor(QString flavor) const;
    bool hasOSType(int c_flavor) const;

    PasteboardRef pasteBoard() const;
    QMimeData *mimeData() const;
    void setMimeData(QMimeData *mime);

    QStringList formats() const;
    bool hasFormat(const QString &format) const;
    QVariant retrieveData(const QString &format, QVariant::Type) const;

    void clear();
    bool sync() const;
};

#ifdef __LP64__
# define QT_MAC_NO_QUICKDRAW
#endif

extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp
#ifndef QT_MAC_NO_QUICKDRAW
#include "qpaintdevice.h"
extern WindowPtr qt_mac_window_for(const QWidget*); //qwidget_mac.cpp
class QMacSavedPortInfo
{
    RgnHandle clip;
    GWorldPtr world;
    GDHandle handle;
    PenState pen; //go pennstate
    RGBColor back, fore;
    bool valid_gworld;
    void init();

public:
    inline QMacSavedPortInfo() { init(); }
    inline QMacSavedPortInfo(QPaintDevice *pd) { init(); setPaintDevice(pd); }
    inline QMacSavedPortInfo(QWidget *w, bool set_clip = false)
        { init(); setPaintDevice(w, set_clip); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRect &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    inline QMacSavedPortInfo(QPaintDevice *pd, const QRegion &r)
        { init(); setPaintDevice(pd); setClipRegion(r); }
    ~QMacSavedPortInfo();
    static inline bool setClipRegion(const QRect &r);
    static inline bool setClipRegion(const QRegion &r);
    static inline bool setClipRegion(QWidget *w)
        { return setClipRegion(w->d_func()->clippedRegion()); }
    static inline bool setPaintDevice(QPaintDevice *);
    static inline bool setPaintDevice(QWidget *, bool set_clip=false, bool with_child=true);
};

inline bool
QMacSavedPortInfo::setClipRegion(const QRect &rect)
{
    Rect r;
    SetRect(&r, rect.x(), rect.y(), rect.right()+1, rect.bottom()+1);
    ClipRect(&r);
    return true;
}

inline bool
QMacSavedPortInfo::setClipRegion(const QRegion &r)
{
    if(r.isEmpty())
        return setClipRegion(QRect());
    RgnHandle rgn = r.handle();
    if(!rgn)
        return setClipRegion(r.boundingRect());
    SetClip(rgn);
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QWidget *w, bool set_clip, bool with_child)
{
    if (!w)
        return false;
    if(!setPaintDevice((QPaintDevice *)w))
        return false;
    if(set_clip)
        return setClipRegion(w->d_func()->clippedRegion(with_child));
    return true;
}

inline bool
QMacSavedPortInfo::setPaintDevice(QPaintDevice *pd)
{
    if(!pd)
        return false;
    bool ret = true;
    extern GrafPtr qt_mac_qd_context(const QPaintDevice *); // qpaintdevice_mac.cpp
    if(pd->devType() == QInternal::Widget)
        SetPortWindowPort(qt_mac_window_for(static_cast<QWidget*>(pd)));
    else if(pd->devType() == QInternal::Pixmap || pd->devType() == QInternal::Printer)
        SetGWorld((GrafPtr)qt_mac_qd_context(pd), 0); //set the gworld
    return ret;
}


inline void
QMacSavedPortInfo::init()
{
    GetBackColor(&back);
    GetForeColor(&fore);
    GetGWorld(&world, &handle);
    valid_gworld = true;
    clip = NewRgn();
    GetClip(clip);
    GetPenState(&pen);
}

inline QMacSavedPortInfo::~QMacSavedPortInfo()
{
    bool set_state = false;
    if(valid_gworld) {
        set_state = IsValidPort(world);
        if(set_state)
            SetGWorld(world,handle); //always do this one first
    } else {
        setPaintDevice(qt_mac_safe_pdev);
    }
    if(set_state) {
        SetClip(clip);
        SetPenState(&pen);
        RGBForeColor(&fore);
        RGBBackColor(&back);
    }
    DisposeRgn(clip);
}
#else
class QMacSavedPortInfo
{
public:
    inline QMacSavedPortInfo() { }
    inline QMacSavedPortInfo(QPaintDevice *) { }
    inline QMacSavedPortInfo(QWidget *, bool = false) { }
    inline QMacSavedPortInfo(QPaintDevice *, const QRect &) { }
    inline QMacSavedPortInfo(QPaintDevice *, const QRegion &) { }
    ~QMacSavedPortInfo() { }
    static inline bool setClipRegion(const QRect &) { }
    static inline bool setClipRegion(const QRegion &) { }
    static inline bool setClipRegion(QWidget *) { }
    static inline bool setPaintDevice(QPaintDevice *) { }
    static inline bool setPaintDevice(QWidget *, bool =false, bool =true) { }
};
#endif

#ifdef check
# undef check
#endif

#endif // QT_MAC_P_H
