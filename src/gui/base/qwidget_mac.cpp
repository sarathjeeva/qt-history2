/****************************************************************************
**
** Implementation of QWidget and QWindow classes for mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qt_mac.h"

#include "qaccel.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qdragobject.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine_mac.h"
#include "qpainter.h"
#include "qstack.h"
#include "qstyle.h"
#include "qtextcodec.h"
#include "qtimer.h"

#include <ApplicationServices/ApplicationServices.h>
#include <limits.h>

#include "qwidget_p.h"

#define d d_func()
#define q q_func()

/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS
//#define DEBUG_WINDOW_CREATE

/*****************************************************************************
  QWidget globals
 *****************************************************************************/
static WindowGroupRef qt_mac_stays_on_top_group = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
int mac_window_count = 0;

/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_set_paintevent_clipping(QPaintDevice*, const QRegion&); //qpaintengine_mac.cpp
extern void qt_clear_paintevent_clipping(QPaintDevice *); //qpaintengine_mac.cpp
extern QSize qt_naturalWidgetSize(QWidget *); //qwidget.cpp
extern QString cfstring2qstring(CFStringRef); //qglobal.cpp
extern QSize qt_initial_size(QWidget *w); //qwidget.cpp
extern void qt_mac_clip_cg_handle(CGContextRef, const QRegion &, const QPoint &, bool); //qpaintdevice_mac.cpp
extern CFStringRef qstring2cfstring(const QString &); //qglobal.cpp
extern void qt_mac_unicode_reset_input(QWidget *); //qapplication_mac.cpp
extern void qt_mac_unicode_init(QWidget *); //qapplication_mac.cpp
extern void qt_mac_unicode_cleanup(QWidget *); //qapplication_mac.cpp
extern void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
extern bool qt_event_remove_activate(); //qapplication_mac.cpp
extern void qt_mac_event_release(QWidget *w); //qapplication_mac.cpp
extern void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
extern IconRef qt_mac_create_iconref(const QPixmap &); //qpixmap_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const Point *); //qcursor_mac.cpp
extern bool qt_nograb();
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/
QPoint posInWindow(QWidget *w)
{
    QPoint ret(0, 0);
    if(w->isTopLevel())
	return ret;
    if(QWidget *par = w->parentWidget())
	ret = posInWindow(par) + w->pos();
    return ret;
}

static void qt_mac_release_stays_on_top_group() //cleanup function
{
    ReleaseWindowGroup(qt_mac_stays_on_top_group);
    if(GetWindowGroupRetainCount(qt_mac_stays_on_top_group) == 1) { //only the global pointer exists
	ReleaseWindowGroup(qt_mac_stays_on_top_group);
	qt_mac_stays_on_top_group = 0;
    }
}

//find a QWidget from a WindowPtr
QWidget *qt_mac_find_window(WindowPtr window) 
{
    //There is a better way to do it, but at the moment I just want to see if my hacks work.. FIXME ###
    HIViewRef hiview = 0;
    if(HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &hiview) == noErr) {
	QWidgetList list = QApplication::topLevelWidgets();
	for (int i = 0; i < list.size(); ++i) {
	    QWidget *w = list.at(i);
	    if(hiview == HIViewGetSuperview((HIViewRef)w->winId()))
		return w;
	}
    }
    return 0;
}

inline static void qt_mac_set_fullscreen_mode(bool b)
{
#if 0
    if(b)
	SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
    else
	SetSystemUIMode(kUIModeNormal, 0);
#else
    extern bool qt_mac_app_fullscreen;
    qt_mac_app_fullscreen = b;
    if(b)
	HideMenuBar();
    else
	ShowMenuBar();
#endif
}

//find a WindowPtr from a QWidget/HIView
WindowPtr qt_mac_window_for(HIViewRef hiview)
{
#if QT_MACOSX_VERSION >= 0x1030
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) 
	return HIViewGetWindow(hiview);
#endif
    return GetControlOwner(hiview);

}

/* Use this function instead of ReleaseWindowGroup, this will be sure to release the
   stays on top window group (created with qt_mac_get_stays_on_top_group below) */
static void qt_mac_release_window_group(WindowGroupRef group)
{
    if(group == qt_mac_stays_on_top_group)
	qt_mac_release_stays_on_top_group();
    else
	ReleaseWindowGroup(group);
}
#define ReleaseWindowGroup(x) Are you sure you wanted to do that? (you wanted qt_mac_release_window_group)

/* We create one static stays on top window group so that all stays on top (aka popups) will
   fall into the same group and be able to be raise()'d with releation to one another (from
   within the same window group). */
static WindowGroupRef qt_mac_get_stays_on_top_group()
{
    if(!qt_mac_stays_on_top_group) {
	CreateWindowGroup(kWindowActivationScopeNone, &qt_mac_stays_on_top_group);
	SetWindowGroupLevel(qt_mac_stays_on_top_group, kCGMaximumWindowLevel);
	SetWindowGroupParent(qt_mac_stays_on_top_group, GetWindowGroupOfClass(kAllWindowClasses));
    }
    RetainWindowGroup(qt_mac_stays_on_top_group);
    return qt_mac_stays_on_top_group;
}


void qt_mac_update_metal_style(QWidget *w)
{
    if(w->isTopLevel()) {
	if(w->testAttribute(QWidget::WA_MacMetalStyle))
	    ChangeWindowAttributes((WindowRef)w->handle(), kWindowMetalAttribute, 0);
	else
	    ChangeWindowAttributes((WindowRef)w->handle(), 0, kWindowMetalAttribute);
    }
}

static OSStatus qt_mac_create_window(WindowClass wclass, WindowAttributes wattr,
				     Rect *geo, WindowPtr *w)
{
    OSStatus ret;
    if(geo->right == geo->left)
	geo->right++;
    if(geo->bottom == geo->top)
	geo->bottom++;
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
	Rect null_rect; SetRect(&null_rect, 0, 0, 0, 0);
	ret = CreateNewWindow(wclass, wattr, &null_rect, w);
	if(ret == noErr) {
	    ret = SetWindowBounds(*w, kWindowContentRgn, geo);
	    if(ret != noErr)
		qWarning("%s:%d This error shouldn't really ever happen!!!", __FILE__, __LINE__);
	}
    } else {
	ret = CreateNewWindow(wclass, wattr, geo, w);
    }
    return ret;
}

// window events
static EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowGetRegion },
    { kEventClassMouse, kEventMouseDown }
};
static EventHandlerUPP mac_win_eventUPP = 0;
static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = 0;
}
static const EventHandlerUPP make_win_eventUPP()
{
    if(mac_win_eventUPP)
	return mac_win_eventUPP;
    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_window_event);
}
QMAC_PASCAL OSStatus QWidgetPrivate::qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassWindow:
	if(ekind == kEventWindowGetRegion) {
	    WindowRef window;
	    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
			      sizeof(window), 0, &window);
	    CallNextEventHandler(er, event);
	    WindowRegionCode wcode;
	    GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, 0,
			      sizeof(wcode), 0, &wcode);
	    RgnHandle rgn;
	    GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL,
			      sizeof(rgn), NULL, &rgn);
	    if(QWidgetPrivate::qt_widget_rgn(qt_mac_find_window(window), wcode, rgn, false))
		SetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, sizeof(rgn), &rgn);
	} else {
	    handled_event = false;
	}
	break;
    case kEventClassMouse:
	if(ekind == kEventMouseDown) {
	    UInt32 count;
	    GetEventParameter(event, kEventParamClickCount, typeUInt32, 0,
			      sizeof(count), 0, &count);
	    if(count == 1) {
		Point where;
		GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0,
				  sizeof(where), 0, &where);
		bool ok;
		if(qApp->do_mouse_down(&where, &ok) || ok)
		    handled_event = false;
	    }
	} else {
	    handled_event = false;
	}
	break;
    default:
	handled_event = false;
    }
    if(!handled_event) //let the event go through
	return eventNotHandledErr;
    return noErr; //we eat the event
}

// widget events
static HIObjectClassRef widget_class = NULL;
static CFStringRef kObjectQWidget = CFSTR("com.trolltech.qt.widget");
static EventTypeSpec widget_events[] = {
    { kEventClassHIObject, kEventHIObjectConstruct },
    { kEventClassHIObject, kEventHIObjectDestruct },

    { kEventClassControl, kEventControlDraw },
    { kEventClassControl, kEventControlInitialize },
    { kEventClassControl, kEventControlGetPartRegion },
    { kEventClassControl, kEventControlDragEnter },
    { kEventClassControl, kEventControlDragWithin },
    { kEventClassControl, kEventControlDragLeave },
    { kEventClassControl, kEventControlDragReceive },
};
static EventHandlerUPP mac_widget_eventUPP = 0;
static void cleanup_widget_eventUPP()
{
    DisposeEventHandlerUPP(mac_widget_eventUPP);
    mac_widget_eventUPP = 0;
}
static const EventHandlerUPP make_widget_eventUPP()
{
    if(mac_widget_eventUPP)
	return mac_widget_eventUPP;
    qAddPostRoutine(cleanup_widget_eventUPP);
    return mac_widget_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_widget_event);
}
QMAC_PASCAL OSStatus QWidgetPrivate::qt_widget_event(EventHandlerCallRef, EventRef event, void *)
{
    bool handled_event = true;
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    switch(eclass) {
    case kEventClassHIObject:
	if(ekind == kEventHIObjectConstruct) {
	    HIViewRef view;
            if(GetEventParameter(event, kEventParamHIObjectInstance, typeHIObjectRef, 
				 NULL, sizeof(view), NULL, &view) == noErr) {
#if QT_MACOSX_VERSION >= 0x1030
		if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) 
		    HIViewChangeFeatures(view, kHIViewAllowsSubviews, 0);
#endif
	    }
	} else if(ekind == kEventHIObjectDestruct) {
	    //nothing to really do.. or is there?
	} else {
	    handled_event = false;
	}
	break;
    case kEventClassControl: {
	QWidget *widget = 0;
	HIViewRef hiview = 0;
	if(GetEventParameter(event, kEventParamDirectObject, typeControlRef, 
			     NULL, sizeof(hiview), NULL, &hiview) == noErr) 
	    widget = QWidget::find((WId)hiview);
	if(ekind == kEventControlDraw) {
	    if(widget) {
#if 0
		qDebug("asked to draw %p [%s::%s] %p", hiview, widget->className(), widget->objectName(),
		       (HIViewRef)(widget->parentWidget() ? widget->parentWidget()->winId() : (WId)-1));
#endif
		//update clip
		widget->d->clp_serial++;
		RgnHandle rgn;
		GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn);
		QRegion qrgn(qt_mac_convert_mac_region(rgn));
		widget->d->clp = qrgn;
		if(!widget->isTopLevel()) {
		    QPoint pt(posInWindow(widget));
		    widget->d->clp.translate(pt.x(), pt.y());
		}

		//update cg context
		CGContextRef cgref;
		GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(cgref), NULL, &cgref);
		widget->cg_hd = cgref;

		//update qd port
		GrafPtr qdref = 0;
		if(GetEventParameter(event, kEventParamGrafPort, typeGrafPtr, NULL, sizeof(qdref), NULL, &qdref) != noErr) 
		    GetGWorld(&qdref, 0); //just use the global port..
		if(qdref)
		    widget->hd = GetWindowFromPort(qdref);

		if((widget->data->widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible) {  //process the actual paint event.
		    if(widget->testWState(Qt::WState_InPaintEvent))
			qWarning("QWidget::repaint: recursive repaint detected.");
		    
		    widget->setWState(Qt::WState_InPaintEvent);
		    qt_set_paintevent_clipping(widget, qrgn);

		    if(!widget->testAttribute(QWidget::WA_NoBackground) && 
		       !widget->d->isBackgroundInherited()) {
			QBrush bg = widget->palette().brush(widget->d->bg_role);
			QRect rr = qrgn.boundingRect();
			bool was_unclipped = widget->testWFlags(Qt::WPaintUnclipped);
			widget->clearWFlags(Qt::WPaintUnclipped);
			QPainter p(widget);
			if(was_unclipped)
			    widget->setWFlags(Qt::WPaintUnclipped);
			p.setClipRegion(qrgn);
			if(bg.pixmap())
			    p.drawTiledPixmap(rr, *bg.pixmap(), QPoint(rr.x()+(bg.pixmap()->width()),
								       rr.y()+(bg.pixmap()->height())));
			else 
			    p.fillRect(rr, bg.color());
		    }
		    QPaintEvent e(qrgn);
		    QApplication::sendSpontaneousEvent(widget, &e);
		    qt_clear_paintevent_clipping(widget);
		    widget->clearWState(WState_InPaintEvent);
		}
		SetPort(qdref); //restore the state..

		//remove the old pointers (this is just a hack of course..) --Sam
		widget->d->clp_serial++;
		widget->d->clp = QRegion();
		widget->cg_hd = 0;
		widget->hd = 0;
	    }
	} else if(ekind == kEventControlInitialize) {
	    UInt32 features = kControlSupportsDragAndDrop;
	    if(QSysInfo::MacintoshVersion < QSysInfo::MV_PANTHER) 
		features |= (kControlSupportsEmbedding|kControlSupportsGetRegion);
	    SetEventParameter(event, kEventParamControlFeatures, typeUInt32, sizeof(features), &features);
	} else if(ekind == kEventControlGetPartRegion) {
	    handled_event = false;
	    if(widget && !widget->isTopLevel()) {
		ControlPartCode part;
		GetEventParameter(event, kEventParamControlPart, typeControlPartCode, 0, sizeof(part), 0, &part);
		if(part == kControlStructureMetaPart) {
		    RgnHandle rgn;
		    GetEventParameter(event, kEventParamControlRegion, typeQDRgnHandle, NULL, sizeof(rgn), NULL, &rgn);
		    SetRectRgn(rgn, 0, 0, widget->width(), widget->height());
		    if(QWidgetPrivate::qt_widget_rgn(widget, kWindowStructureRgn, rgn, false))
			handled_event = true;
		}
	    }
	} else if(ekind == kEventControlDragEnter || ekind == kEventControlDragWithin ||
		  ekind == kEventControlDragLeave || ekind == kEventControlDragReceive) {
	    handled_event = false;
	    if(widget) {
		//these are really handled in qdnd_mac.cpp just to modularize the code a little..
		DragRef drag;
		GetEventParameter(event, kEventParamDragRef, typeDragRef, NULL, sizeof(drag), NULL, &drag);
		if(widget->d->qt_mac_dnd_event(ekind, drag)) 
		    handled_event = true;
#if QT_MACOSX_VERSION >= 0x1030
		if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
		    if(ekind == kEventControlDragEnter) 
			SetEventParameter(event, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof(handled_event), &handled_event);
		}
#endif
	    }
	}
	break; }
    default:
	handled_event = false;
	break;
    }
    if(!handled_event) //let the event go through
	return eventNotHandledErr;
    return noErr; //we eat the event
}
static HIViewRef qt_mac_create_widget(HIViewRef parent)
{
    if(!widget_class) {
	if(HIObjectRegisterSubclass(kObjectQWidget, kHIViewClassID, 0, make_widget_eventUPP(), 
				    GetEventTypeCount(widget_events), widget_events, NULL, &widget_class) != noErr)
	    qWarning("That cannot happen!!! %d", __LINE__);
    }
    HIViewRef ret = 0;
    if(widget_class) {
	if(HIObjectCreate(kObjectQWidget, 0, (HIObjectRef*)&ret) != noErr)
	    qWarning("That cannot happen!!! %d", __LINE__);
	if(ret) 
	    HIViewAddSubview(parent, ret);
    }
    return ret;
}

bool qt_mac_is_macdrawer(QWidget *w)
{
#if 1
    if(w && w->isTopLevel() && w->parentWidget() && w->testWFlags(Qt::WMacDrawer))
	return true;
#else
    Q_UNUSED(w);
#endif
    return false;
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::Dock where) //users of Qt/Mac can use this..
{
    if(!qt_mac_is_macdrawer(w))
	return false;
    OptionBits bits;
    if(where == Qt::DockTop)
	bits = kWindowEdgeTop;
    else if(where == Qt::DockLeft)
	bits = kWindowEdgeLeft;
    else if(where == Qt::DockRight)
	bits = kWindowEdgeRight;
    else if(where == Qt::DockBottom)
	bits = kWindowEdgeBottom;
    else
	return false;
    SetDrawerPreferredEdge(qt_mac_window_for((HIViewRef)w->winId()), bits);
    return true;
}

bool qt_mac_is_macsheet(QWidget *w, bool ignore_exclusion=false)
{
#if 0
    if(w && w->isTopLevel() && w->testWFlags(Qt::WStyle_DialogBorder) &&
       (ignore_exclusion || !w->testWFlags(Qt::WMacNoSheet)) &&
       w->parentWidget() && !w->parentWidget()->topLevelWidget()->isDesktop() &&
       w->parentWidget()->topLevelWidget()->isVisible()
       && ::qt_cast<QMacStyle *>(&w->style()))
	return true;
#else
    Q_UNUSED(w);
    Q_UNUSED(ignore_exclusion);
#endif
    return false;
}



/*****************************************************************************
  QWidgetPrivate member functions
 *****************************************************************************/
bool QWidgetPrivate::qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isTopLevel())
	return false;
    
    w->d->createTLExtra();
    w->d->extraData()->topextra->resizer += up;
    {
	WindowClass wclass;
	GetWindowClass(qt_mac_window_for((HIViewRef)w->winId()), &wclass);
	if(!(GetAvailableWindowAttributes(wclass) & kWindowResizableAttribute))
	    return TRUE;
    }
    bool remove_grip = (w->d->extraData()->topextra->resizer ||
			(w->d->extraData()->maxw && w->d->extraData()->maxh &&
			 w->d->extraData()->maxw == w->d->extraData()->minw && w->d->extraData()->maxh == w->d->extraData()->minh));
    
    WindowAttributes attr;
    GetWindowAttributes((WindowRef)w->handle(), &attr);
    if(remove_grip) {
	if(attr & kWindowResizableAttribute) {
	    ChangeWindowAttributes((WindowRef)w->handle(), kWindowNoAttributes,
                                   kWindowResizableAttribute);
	    ReshapeCustomWindow(qt_mac_window_for((HIViewRef)w->winId()));
	}
    } else if(!(attr & kWindowResizableAttribute)) {
	ChangeWindowAttributes((WindowRef)w->handle(), kWindowResizableAttribute,
			       kWindowNoAttributes);
	ReshapeCustomWindow(qt_mac_window_for((HIViewRef)w->winId()));
    }
    return true;
}

static QList<QWidget *> qt_root_win_widgets;
static WindowPtr qt_root_win = 0;
void QWidgetPrivate::qt_clean_root_win() 
{
    if(!qt_root_win)
	return;
    if(HIViewRef root_hiview = HIViewGetRoot(qt_root_win)) {
	for(int i = 0; i < qt_root_win_widgets.count(); i++) {
	    QWidget *w = qt_root_win_widgets.at(i);
	    if((HIViewRef)w->winId() == root_hiview) 
		w->setWinId(0); //at least now we'll just crash
	}
    }
    DisposeWindow(qt_root_win);
    qt_root_win = NULL;
}

bool QWidgetPrivate::qt_create_root_win() {
    if(qt_root_win)
	return false;
    Rect r;
    int w = 0, h = 0;
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	w = QMAX(w, (*g)->gdRect.right);
	h = QMAX(h, (*g)->gdRect.bottom);
    }
    SetRect(&r, 0, 0, w, h);
    qt_mac_create_window(kOverlayWindowClass, kWindowNoAttributes, &r, &qt_root_win);
    if(!qt_root_win)
	return false;
    qAddPostRoutine(qt_clean_root_win);
    return true;
}

bool QWidgetPrivate::qt_recreate_root_win() {
    if(!qt_root_win) //sanity check
	return false;
    //store old
    WindowPtr old_root_win = qt_root_win;
    HIViewRef old_root_hiview = HIViewGetRoot(qt_root_win);
    //recreate
    qt_root_win = NULL;
    qt_create_root_win();
    if(HIViewRef root_hiview = HIViewGetRoot(qt_root_win)) {
	for(int i = 0; i < qt_root_win_widgets.count(); i++) { //reset points
	    QWidget *w = qt_root_win_widgets.at(i);
	    if((HIViewRef)w->winId() == old_root_hiview) 
		w->setWinId((WId)root_hiview);
	}
    }
    //cleanup old window
    old_root_hiview = 0;
    DisposeWindow(old_root_win);
    return true;
}

bool QWidgetPrivate::qt_widget_rgn(QWidget *widget, short wcode, RgnHandle rgn, bool force = false)
{
    switch(wcode) {
    case kWindowStructureRgn: {
	bool ret = false;
	if(widget) {
	    if(widget->d->extra && !widget->d->extra->mask.isEmpty()) {
		QRegion rin = qt_mac_convert_mac_region(rgn);
		if(!rin.isEmpty()) {
		    QPoint rin_tl = rin.boundingRect().topLeft(); //in offset
		    rin.translate(-rin_tl.x(), -rin_tl.y()); //bring into same space as below
		    QRegion mask = widget->d->extra->mask;
		    if(widget->isTopLevel() && 
		       (!widget->testWFlags(Qt::WStyle_Customize) || !widget->testWFlags(Qt::WStyle_NoBorder))) {
			QRegion title;
			{
			    RgnHandle rgn = qt_mac_get_rgn();
			    GetWindowRegion(qt_mac_window_for((HIViewRef)widget->winId()), kWindowTitleBarRgn, rgn);
			    title = qt_mac_convert_mac_region(rgn);
			    qt_mac_dispose_rgn(rgn);
			}
			QRect br = title.boundingRect();
			mask.translate(0, br.height()); //put the mask 'under' the titlebar..
			title.translate(-br.x(), -br.y());
			mask += title;
		    }
                            
		    QRegion cr = rin & mask;
		    cr.translate(rin_tl.x(), rin_tl.y()); //translate back to incoming space
		    CopyRgn(cr.handle(true), rgn);
		}
		ret = true;
	    } else if(force) {
		QRegion cr(widget->geometry());
		CopyRgn(cr.handle(true), rgn);
		ret = true;
	    }
	}
	return ret; }
    default: break;
    }
    return false;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    d->window_event = 0;
    WId destroyid = 0;
    setWState(WState_Created);                        // set created flag

    if(!parentWidget() || parentWidget()->isDesktop())
	setWFlags(WType_TopLevel);            // top-level widget

    bool topLevel = testWFlags(WType_TopLevel);
    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop = testWFlags(WType_Desktop);

    //position
    QRect dskr;
    if(desktop) {
	int w = 0, h = 0;
	for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	    w = qMax(w, (*g)->gdRect.right);
	    h = qMax(h, (*g)->gdRect.bottom);
	}
	dskr = QRect(0, 0, w, h);
	setWState(WState_Visible);
    } else {
	if(QDesktopWidget *dsk = QApplication::desktop()) {
	    int deskn = dsk->primaryScreen();
	    if(parentWidget() && !parentWidget()->isDesktop())
		deskn = dsk->screenNumber(parentWidget());
	    dskr = dsk->screenGeometry(deskn);
	}
	clearWState(WState_Visible);
    }
    if(desktop)                             // desktop widget
	data->crect.setRect(0, 0, dskr.width(), dskr.height());
    else if(topLevel)                    // calc pos/size from screen
	data->crect.setRect(dskr.width()/4, 3*dskr.height()/10, dskr.width()/2, 4*dskr.height()/10);
    else                                    // child widget
	data->crect.setRect(0, 0, 100, 30);

    if(desktop)
	dialog = popup = false;                  // force these flags off
    if(!window)                              // always initialize
	initializeWindow=true;
    if(dialog || popup || desktop) {          // these are top-level, too
	topLevel = true;
	setWFlags(WType_TopLevel);
	if(popup)
	    setWFlags(WStyle_Tool|WStyle_StaysOnTop); // a popup is a tool window
    }
    if(topLevel && parentWidget()) { // if our parent has WStyle_StaysOnTop, so must we
	QWidget *ptl = parentWidget()->topLevelWidget();
	if(ptl && ptl->testWFlags(WStyle_StaysOnTop))
	    setWFlags(WStyle_StaysOnTop);
    }
    if(dialog && !testWFlags(WShowModal) && parentWidget() && parentWidget()->testWFlags(WShowModal))
	setWFlags(WShowModal);
    if(!testWFlags(WStyle_Customize) && !(desktop || popup) && !testWFlags(WShowModal))
	setWFlags(WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu);

    hd = 0;
    cg_hd = 0;
    if(window) {				// override the old window
	data->fstrut_dirty = true; // we'll re calculate this later
	if(destroyOldWindow)
	    destroyid = winId();
	HIViewRef hiview = (HIViewRef)window;
	HIRect bounds;
	HIViewGetFrame(hiview, &bounds);
	if(HIViewIsVisible(hiview))
	    setWState(WState_Visible);
	else
	    clearWState(WState_Visible);
	data->crect.setRect((int)bounds.origin.x, (int)bounds.origin.y, (int)bounds.size.width, (int)bounds.size.height);
	setWinId((WId)hiview);
    } else if(desktop) {			// desktop widget
	if(!qt_root_win)
            QWidgetPrivate::qt_create_root_win();
	qt_root_win_widgets.append(this);
	if(HIViewRef hiview = HIViewGetRoot(qt_root_win)) {
	    CFRetain((HIViewRef)hiview);
	    setWinId((WId)hiview);
	}
    } else if(isTopLevel()) {
	Rect r;
	SetRect(&r, data->crect.left(), data->crect.top(), data->crect.left(), data->crect.top());
	WindowClass wclass = kSheetWindowClass;
	if(popup || testWFlags(WStyle_Splash) == WStyle_Splash)
	    wclass = kModalWindowClass;
	else if(testWFlags(WShowModal))
	    wclass = kMovableModalWindowClass;
	else if(qt_mac_is_macdrawer(this))
	    wclass = kDrawerWindowClass;
	else if(testWFlags(WStyle_Tool) && qstrcmp(objectName(), "toolTipTip") == 0) // Tool tips
	    wclass = kHelpWindowClass;
	else if(testWFlags(WStyle_Tool)
                || (dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop()))
	    wclass = kFloatingWindowClass;
	else if(dialog)
	    wclass = kToolbarWindowClass;
	else
	    wclass = kDocumentWindowClass;

	WindowGroupRef grp = 0;
	WindowAttributes wattr = kWindowCompositingAttribute;
	if(testWFlags(WStyle_Customize)) {
	    if(qt_mac_is_macsheet(this)) {
		grp = GetWindowGroupOfClass(kMovableModalWindowClass);
		wclass = kSheetWindowClass;
	    } else {
		grp = GetWindowGroupOfClass(wclass);
                // Shift things around a bit to get the correct window class based on the presence
                // (or lack) of the border.
		if(testWFlags(WStyle_NoBorder)) {
		    if(wclass == kDocumentWindowClass)
			wclass = kPlainWindowClass;
		    else if(wclass == kFloatingWindowClass)
			wclass = kToolbarWindowClass;
		} else {
		    if(wclass != kModalWindowClass)
			wattr |= kWindowResizableAttribute;
                    if(wclass == kToolbarWindowClass) {
                        if(!parentWidget() || parentWidget()->isDesktop())
                            wclass = kDocumentWindowClass;
                        else
                            wclass = kFloatingWindowClass;
                    }
		}
		// Only add extra decorations (well, buttons) for widgets that can have them
		// and have an actual border we can put them on.
                if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass
                    && wclass != kSheetWindowClass && wclass != kPlainWindowClass
                    && !testWFlags(WStyle_NoBorder) && wclass != kDrawerWindowClass) {
		    if(testWFlags(WStyle_Maximize))
			wattr |= kWindowFullZoomAttribute;
		    if(testWFlags(WStyle_Minimize))
			wattr |= kWindowCollapseBoxAttribute;
		    if(testWFlags(WStyle_Title) || testWFlags(WStyle_SysMenu))
		       wattr |= kWindowCloseBoxAttribute;
		}
	    }
	}
	if(testWFlags(Qt::WMacMetal))
	    wattr |= kWindowMetalAttribute;
	if(testWFlags(WStyle_Tool))
	    wattr |= kWindowHideOnSuspendAttribute;
	wattr |= kWindowLiveResizeAttribute;
	if(testWFlags(WStyle_Tool) && testWFlags(WStyle_Splash) != WStyle_Splash)
	    wattr |= kWindowHideOnSuspendAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
	struct {
	    UInt32 tag;
	    const char *name;
	} known_attribs[] = {
	    ADD_DEBUG_WINDOW_NAME(kWindowCompositingAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowMetalAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowLiveResizeAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCloseBoxAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
	    { 0, 0 }
	}, known_classes[] = {
	    ADD_DEBUG_WINDOW_NAME(kPlainWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kDrawerWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kUtilityWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kSheetWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kFloatingWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kDocumentWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kMovableModalWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kModalWindowClass),
	    { 0, 0 }
	};
#undef ADD_DEBUG_WINDOW_NAME
	qDebug("Qt: internal: ************* Creating new window (%s::%s)", className(), objectName());
	bool found_class = false;
	for(int i = 0; known_classes[i].name; i++) {
	    if(wclass == known_classes[i].tag) {
		found_class = true;
		qDebug("Qt: internal: ** Class: %s", known_classes[i].name);
		break;
	    }
	}
	if(!found_class)
	    qDebug("Qt: internal: !! Class: Unknown! (%d)", (int)wclass);
	if(wattr) {
	    WindowAttributes tmp_wattr = wattr;
	    qDebug("Qt: internal: ** Attributes:");
	    for(int i = 0; tmp_wattr && known_attribs[i].name; i++) {
		if((tmp_wattr & known_attribs[i].tag) == known_attribs[i].tag) {
		    tmp_wattr ^= known_attribs[i].tag;
		    qDebug("Qt: internal: * %s %s", known_attribs[i].name,
			   (GetAvailableWindowAttributes(wclass) & known_attribs[i].tag) ? "" : "(*)");
		}
	    }
	    if(tmp_wattr)
		qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)tmp_wattr);
	}
#endif

	/* Just to be extra careful we will change to the kUtilityWindowClass if the
	   requested attributes cannot be used */
	if((GetAvailableWindowAttributes(wclass) & wattr) != wattr) {
	    WindowClass tmp_class = wclass;
	    if(wclass == kToolbarWindowClass || wclass == kUtilityWindowClass)
		wclass = kFloatingWindowClass;
	    if(tmp_class != wclass) {
		if(!grp)
		    grp = GetWindowGroupOfClass(wclass);
		wclass = tmp_class;
	    }
	}

	WindowRef window = 0;
	if(OSStatus ret = qt_mac_create_window(wclass, wattr, &r, &window))
	    qDebug("Qt: internal: %s:%d If you reach this error please contact Trolltech and include the\n"
		   "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, ret);
	if(!desktop) { //setup an event callback handler on the window
	    SetAutomaticControlDragTrackingEnabledForWindow(window, true);
	    InstallWindowEventHandler(window, make_win_eventUPP(), GetEventTypeCount(window_events),
				      window_events, static_cast<void *>(qApp), &d->window_event);
	}
	if(qt_mac_is_macdrawer(this))
	    SetDrawerParent(window, qt_mac_window_for((HIViewRef)parentWidget()->winId()));
	if(dialog && !parentWidget() && !testWFlags(WShowModal))
	    grp = GetWindowGroupOfClass(kDocumentWindowClass);
	if(testWFlags(WStyle_StaysOnTop)) {
	    d->createTLExtra();
	    if(d->topData()->group)
		qt_mac_release_window_group(d->topData()->group);
	    d->topData()->group = qt_mac_get_stays_on_top_group();
	    SetWindowGroup(window, d->topData()->group);
	} else if(grp) {
	    SetWindowGroup(window, grp);
	}
#ifdef DEBUG_WINDOW_CREATE
	if(WindowGroupRef grpf = GetWindowGroup(window)) {
	    CFStringRef cfname;
	    CopyWindowGroupName(grpf, &cfname);
	    SInt32 lvl;
	    GetWindowGroupLevel(grpf, &lvl);
	    const char *from = "Default";
	    if(d->topData() && grpf == d->topData()->group)
		from = "Created";
	    else if(grpf == grp)
		from = "Copied";
	    qDebug("Qt: internal: With window group '%s' [%p] @ %d: %s",
		   cfstring2qstring(cfname).latin1(), grpf, (int)lvl, from);
	} else {
	    qDebug("Qt: internal: No window group!!!");
	}
#endif
	if(d->extra && !d->extra->mask.isEmpty())
	   ReshapeCustomWindow(window);
	if(testWFlags(WType_Popup) || testWFlags(WStyle_Tool))
	    SetWindowModality(window, kWindowModalityNone, NULL);
	if(qt_mac_is_macsheet(this))
	    setWindowOpacity(0.70);
	else if(qt_mac_is_macdrawer(this))
	    SetDrawerOffsets(window, 0.0, 25.0);
	data->fstrut_dirty = true; // when we create a toplevel widget, the frame strut should be dirty
	HIViewRef window_hiview = 0;
	OSStatus err = HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &window_hiview);
	if(err == errUnknownControl)
	    window_hiview = HIViewGetRoot(window);
	else if(err != noErr)
	    qWarning("That cannot happen! %d [%ld]", __LINE__, err);
	if(HIViewRef hiview = qt_mac_create_widget(window_hiview)) {
	    Rect win_rect;
	    GetWindowBounds(qt_mac_window_for(window_hiview), kWindowContentRgn, &win_rect);
	    HIRect bounds = CGRectMake(0, 0, win_rect.right-win_rect.left, win_rect.bottom-win_rect.top);
	    HIViewSetFrame(hiview, &bounds);
	    HIViewSetVisible(hiview, true);
	    setWinId((WId)hiview);
	}
    } else {
	data->fstrut_dirty = false; // non-toplevel widgets don't have a frame, so no need to update the strut
	if(HIViewRef hiview = qt_mac_create_widget((HIViewRef)parentWidget()->winId())) {
	    HIRect bounds = CGRectMake(data->crect.x(), data->crect.y(), data->crect.width(), data->crect.height());
	    HIViewSetFrame(hiview, &bounds);
	    setWinId((WId)hiview);
	}
    }

    d->macDropEnabled = false;
    if(HIViewRef destroy_hiview = (HIViewRef)destroyid) {
	if(isTopLevel())
	    DisposeWindow(qt_mac_window_for(destroy_hiview));
	CFRelease(destroy_hiview);
    }
    qt_mac_unicode_init(this);
}

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    deactivateWidgetCleanup();
    qt_mac_event_release(this);
    qt_mac_unicode_cleanup(this);
    if(isDesktop() && destroyWindow) 
	qt_root_win_widgets.remove(this);
    if(testWState(WState_Created)) {
        clearWState(WState_Created);
	QObjectList chldrn = children();
	for(int i = 0; i < chldrn.size(); i++) {  // destroy all widget children
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType())
		((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
        }
	if(mac_mouse_grabber == this)
	    releaseMouse();
	if(mac_keyboard_grabber == this)
	    releaseKeyboard();
	if(acceptDrops())
	    setAcceptDrops(false);

        if(testWFlags(WShowModal))          // just be sure we leave modal
            qt_leave_modal(this);
        else if(testWFlags(WType_Popup))
            qApp->closePopup(this);
	if(destroyWindow) {
	    if(d->window_event) 
		RemoveEventHandler(d->window_event);
	    HIViewRef hiview = (HIViewRef)winId();
	    if(isTopLevel())
		DisposeWindow(qt_mac_window_for(hiview));
	    if(hiview)
		CFRelease(hiview);
	}
    }
    setWinId(0);
}

void QWidget::reparent_helper(QWidget *parent, WFlags f, const QPoint &p, bool showIt)
{
    QCursor oldcurs;
    bool setcurs=testAttribute(WA_SetCursor);
    if(setcurs) {
	oldcurs = cursor();
	unsetCursor();
    }
    
    EventHandlerRef old_window_event = 0;
    HIViewRef old_id = 0;
    if(!isDesktop()) {
	old_id = (HIViewRef)winId();
	old_window_event = d->window_event;
    }
    QWidget* oldtlw = topLevelWidget();
    reparentFocusWidgets(parent);		// fix focus chains

    //recreate and seutp flags
    setWinId(0);
    QObject::setParent_helper(parent);
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt = windowTitle();
    data->widget_flags = f;
    clearWState(WState_Created | WState_Visible | WState_Hidden | WState_ExplicitShowHide);
    create();
    if(isTopLevel() || (!parent || parent->isVisible()))
	setWState(WState_Hidden);
    if(dropable)
	setAcceptDrops(false);

    //reparent children
    QObjectList chlist = children();
    for (int i = 0; i < chlist.size(); ++i) { 
	QObject *obj = chlist.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    if(!w->isTopLevel()) 
		HIViewAddSubview((HIViewRef)winId(), (HIViewRef)w->winId());
	}
    }

    //get new hd, now move
    setGeometry(p.x(), p.y(), s.width(), s.height());

    //reset flags and show (if neccesary)
    setEnabled(enable);
    setFocusPolicy(fp);
    setAcceptDrops(dropable);
    if(!capt.isNull()) {
	d->topData()->caption = QString::null;
	setWindowTitle(capt);
    }
    if(showIt)
	show();
    if(setcurs)
	setCursor(oldcurs);
    reparentFocusWidgets(oldtlw);

    //cleanup
    if(old_window_event) 
	RemoveEventHandler(old_window_event);
    if(old_id) { //don't need old window anymore
	if(oldtlw == this)
	    DisposeWindow(qt_mac_window_for(old_id));
	HIViewRemoveFromSuperview(old_id);
	CFRelease(old_id);
    }
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    HIPoint hi_pos = CGPointMake(pos.x(), pos.y());
    HIViewConvertPoint(&hi_pos, (HIViewRef)winId(), 0);
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for((HIViewRef)winId()), kWindowStructureRgn, &win_rect);
    return QPoint((int)hi_pos.x+win_rect.left, (int)hi_pos.y+win_rect.top);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Rect win_rect;
    GetWindowBounds(qt_mac_window_for((HIViewRef)winId()), kWindowStructureRgn, &win_rect);
    HIPoint hi_pos = CGPointMake(pos.x()-win_rect.left, pos.y()-win_rect.top);
    HIViewConvertPoint(&hi_pos, 0, (HIViewRef)winId());
    return QPoint((int)hi_pos.x, (int)hi_pos.y);
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height, bool text, QFont *)
{
    if(!width)
	width = 1;
    if(!height)
	height = 1;
    if(text && QRect(x, y, width, height) != microFocusHint()) {
	d->createExtra();
	d->extraData()->micro_focus_hint.setRect(x, y, width, height);
    }
}

void QWidgetPrivate::setFont_syshelper(QFont *)
{
}

void QWidgetPrivate::updateSystemBackground()
{
}


void QWidget::setCursor(const QCursor &cursor)
{
    d->createExtra();
    delete d->extraData()->curs;
    d->extraData()->curs = new QCursor(cursor);
    setAttribute(WA_SetCursor);

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor *n = &cursor;
	if(QApplication::overrideCursor())
	    n = QApplication::overrideCursor();
	qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::unsetCursor()
{
    if(!isTopLevel()) {
	if(QWExtra *extra = d->extraData()) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	setAttribute(WA_SetCursor, false);
    }

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor *n = 0;
	if(QApplication::overrideCursor()) {
	    n = QApplication::overrideCursor();
	} else {
	    for(QWidget *p = this; p; p = p->parentWidget()) {
		QWExtra *extra = p->d->extraData();
		if(extra && extra->curs) {
		    n = extra->curs;
		    break;
		}
	    }
	}
	const QCursor def(Qt::ArrowCursor);
	if(!n) n = &def; //I give up..
	qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::setWindowModified(bool mod)
{
    setAttribute(WA_WindowModified, mod);
    if(isTopLevel())
	SetWindowModified(qt_mac_window_for((HIViewRef)winId()), mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(WA_WindowModified);
}

void QWidget::setWindowTitle(const QString &cap)
{
    if(d->topData() && d->topData()->caption == cap)
	return; // for less flicker
    d->createTLExtra();
    d->topData()->caption = cap;
    if(isTopLevel()) {
	CFStringRef str = CFStringCreateWithCharacters(0, (UniChar *)cap.unicode(), cap.length());
	SetWindowTitleWithCFString(qt_mac_window_for((HIViewRef)winId()), str);
    }
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowIcon(const QPixmap &pixmap)
{
    if(d->topData()) {
	delete d->topData()->icon;
	d->topData()->icon = 0;
    } else {
	d->createTLExtra();
    }
    if(!pixmap.isNull())
	d->topData()->icon = new QPixmap(pixmap);
    if(isTopLevel()) {
	if(qApp && qApp->mainWidget() == this) {
	    if(pixmap.isNull()) {
		RestoreApplicationDockTileImage();
	    } else {
		QImage i = pixmap.convertToImage().convertDepth(32).smoothScale(40, 40);
		for(int y = 0; y < i.height(); y++) {
		    uchar *l = i.scanLine(y);
		    for(int x = 0; x < i.width(); x+=4)
			*(l+x) = 255;
		}
		CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
		CGDataProviderRef dp = CGDataProviderCreateWithData(0, i.bits(), i.numBytes(), 0);
		CGImageRef ir = CGImageCreate(i.width(), i.height(), 8, 32, i.bytesPerLine(),
					      cs, kCGImageAlphaNoneSkipFirst, dp,
					      0, 0, kCGRenderingIntentDefault);
		//cleanup
		SetApplicationDockTileImage(ir);
		CGImageRelease(ir);
		CGColorSpaceRelease(cs);
		CGDataProviderRelease(dp);
	    }
	}
	SetWindowProxyIcon(qt_mac_window_for((HIViewRef)winId()), qt_mac_create_iconref(pixmap));
    }
    QEvent e(QEvent::WindowIconChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowIconText(const QString &iconText)
{
    d->createTLExtra();
    d->topData()->iconText = iconText;
    if(isTopLevel()) {
	CFStringRef str = 0;
	if(!iconText.isNull())
	    CFStringCreateWithCharacters(0, (UniChar *)iconText.unicode(), iconText.length());
	SetWindowAlternateTitle(qt_mac_window_for((HIViewRef)winId()), str);
    }
    QEvent e(QEvent::IconTextChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::grabMouse()
{
    if(isVisible() && !qt_nograb()) {
	if(mac_mouse_grabber)
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::grabMouse(const QCursor &)
{
    if(isVisible() && !qt_nograb()) {
	if(mac_mouse_grabber)
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::releaseMouse()
{
    if(!qt_nograb() && mac_mouse_grabber == this)
	mac_mouse_grabber = 0;
}

void QWidget::grabKeyboard()
{
    if(!qt_nograb()) {
	if(mac_keyboard_grabber)
	    mac_keyboard_grabber->releaseKeyboard();
	mac_keyboard_grabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if(!qt_nograb() && mac_keyboard_grabber == this)
	mac_keyboard_grabber = 0;
}

QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if(!tlw->isVisible() || !tlw->isTopLevel() || tlw->isDesktop())
	return;
    qt_event_remove_activate();
    qt_mac_set_fullscreen_mode(tlw->windowState() & WindowFullScreen);
    WindowPtr window = qt_mac_window_for((HIViewRef)winId());
    if(tlw->isPopup() || tlw->testWFlags(WStyle_Tool) || qt_mac_is_macdrawer(tlw)) {
	ActivateWindow(window, true);
    } else {
	if(IsWindowActive(window)) {
	    ActivateWindow(window, true);
	    qApp->setActiveWindow(tlw);
	} else if(!isMinimized()){
	    SelectWindow(window);
	}
    }
    SetUserFocusWindow(window);
}

void QWidget::update()
{
    update(0, 0, width(), height());
}

void QWidget::update(int x, int y, int w, int h)
{
    if(!testWState(WState_BlockUpdates) && isVisible()) {
	if(w < 0)
	    w = data->crect.width()  - x;
	if(h < 0)
	    h = data->crect.height() - y;
	if(w && h) {
	    QRegion rgn(x, y, w, h);
	    HIViewSetNeedsDisplayInRegion((HIViewRef)winId(), rgn.handle(true), true);
	}
    }
}

void QWidget::update(const QRegion &rgn)
{
    if(!testWState(WState_BlockUpdates) && isVisible()) 
	HIViewSetNeedsDisplayInRegion((HIViewRef)winId(), rgn.handle(true), true);
}

void QWidget::repaint(const QRegion &rgn)
{
    HIViewRef hiview = (HIViewRef)winId();
    HIViewSetNeedsDisplayInRegion(hiview, rgn.handle(true), true);
#if 0
    qDebug("---> %d %d [%p %p] (%d %d)", HIViewGetNeedsDisplay((HIViewRef)winId()), 
	   parentWidget() ? HIViewGetNeedsDisplay((HIViewRef)parentWidget()->winId()) : -1,
	   (void*)winId(), parentWidget() ? (void*)parentWidget()->winId() : (void*)1,
	   HIViewIsDrawingEnabled((HIViewRef)winId()), parentWidget() ? HIViewIsDrawingEnabled((HIViewRef)parentWidget()->winId()) : -1);
    HIViewRender(hiview);
    qDebug("<--- %d %d [%p %p] (%d %d)", HIViewGetNeedsDisplay((HIViewRef)winId()), 
	   parentWidget() ? HIViewGetNeedsDisplay((HIViewRef)parentWidget()->winId()) : -1,
	   (void*)winId(), parentWidget() ? (void*)parentWidget()->winId() : (void*)1,
	   HIViewIsDrawingEnabled((HIViewRef)winId()), parentWidget() ? HIViewIsDrawingEnabled((HIViewRef)parentWidget()->winId()) : -1);
#else
# warning "I'm leaving repaint() as pretty stupid for now because HIViews seems to be broken, more on it later.."
#endif
}

void QWidget::showWindow()
{
    if(isDesktop()) //desktop is always visible
	return;

    if(isTopLevel()) {
	d->createTLExtra();
	QDesktopWidget *dsk = QApplication::desktop();
	if(!d->topData()->is_moved && dsk) {
	    int movex = x(), movey = y();
	    QRect r = frameGeometry();
	    QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
		if(r.bottom() > avail.bottom())
		    movey = avail.bottom() - r.height();
		if(r.right() > avail.right())
		    movex = avail.right() - r.width();
		move(qMax(avail.left(), movex), qMax(avail.top(), movey));
	}
    }
    data->fstrut_dirty = true;
    if(isTopLevel()) {
	WindowPtr window = qt_mac_window_for((HIViewRef)winId());
	SizeWindow(window, width(), height(), true);
	if(qt_mac_is_macsheet(this)) {
	    qt_event_request_showsheet(this);
	} else if(qt_mac_is_macdrawer(this)) {
	    OpenDrawer(window, kWindowEdgeDefault, true);
	} else {
	    ShowHide(window, true);	//now actually show it
        }
	if(windowState() & WindowMinimized) //show in collapsed state
	    CollapseWindow(window, true);
#ifndef QMAC_NO_FAKECURSOR
	if(qstrcmp(name(), "fake_cursor") != 0)
#endif
	    qt_event_request_activate(this);
    } else if(!parentWidget() || parentWidget()->isVisible()) {
	HIViewSetVisible((HIViewRef)winId(), true);
    }
}

void QWidget::hideWindow()
{
    if(isDesktop()) //you can't hide the desktop!
	return;

    if(isTopLevel()) {
	WindowPtr window = qt_mac_window_for((HIViewRef)winId());
	if(qt_mac_is_macsheet(this))
	    HideSheetWindow(window);
	else if(qt_mac_is_macdrawer(this))
	    CloseDrawer(window, true);
	else
	    ShowHide(window, false); //now we hide
	SizeWindow(window, 0, 0, true);
	if(isActiveWindow()) {
	    QWidget *w = 0;
	    if(parentWidget())
		w = parentWidget()->topLevelWidget();
	    if(!w || !w->isVisible()) {
		for(WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true);
		    wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
		    if((w = QWidget::find((WId)wp)))
			break;
		}
	    }
	    if(w && w->isVisible())
		qt_event_request_activate(w);
	}
    } else if(!parentWidget() || parentWidget()->isVisible()) { //strange!! ###
	HIViewSetVisible((HIViewRef)winId(), false);
    }
    deactivateWidgetCleanup();
    qt_mac_event_release(this);
}

void QWidget::setWindowState(uint newstate)
{
    uint oldstate = windowState();

    bool needShow = FALSE;
    if(isTopLevel()) {
	WindowPtr window = qt_mac_window_for((HIViewRef)winId());
	if((oldstate & WindowMinimized) != (newstate & WindowMinimized))
	    CollapseWindow(window, (newstate & WindowMinimized) ? true : false);

	if((oldstate & WindowFullScreen) != (newstate & WindowFullScreen)) {
	    if(newstate & WindowFullScreen) {
		if(QTLWExtra *tlextra = d->topData()) {
		    if(tlextra->normalGeometry.width() < 0) {
			if(testAttribute(WA_Resized))
			    tlextra->normalGeometry = geometry();
			else
			    tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
		    }
		    tlextra->savedFlags = getWFlags();
		}
		setParent(0, WType_TopLevel | WStyle_Customize | WStyle_NoBorder |
			  (getWFlags() & 0xffff0000)); 			  // preserve some widget flags
		setGeometry(qApp->desktop()->screenGeometry(qApp->desktop()->screenNumber(this)));
		qt_mac_set_fullscreen_mode(true);
	    } else {
		setParent(0, d->topData()->savedFlags);
		setGeometry(d->topData()->normalGeometry);
		qt_mac_set_fullscreen_mode(false);
	    }
	}

	if((oldstate & WindowMaximized) != (newstate & WindowMaximized)) {
	    if(newstate & WindowMaximized) {
		Rect bounds;
		data->fstrut_dirty = TRUE;
		QDesktopWidget *dsk = QApplication::desktop();
		QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
		SetRect(&bounds, avail.x(), avail.y(), avail.x() + avail.width(), avail.y() + avail.height());
		if(QWExtra *extra = d->extraData()) {
		    if(bounds.right - bounds.left > extra->maxw)
			bounds.right = bounds.left + extra->maxw;
		    if(bounds.bottom - bounds.top > extra->maxh)
			bounds.bottom = bounds.top + extra->maxh;
		}
		if(QTLWExtra *tlextra = d->topData()) {
		    if(tlextra->normalGeometry.width() < 0) {
			if(testAttribute(WA_Resized))
			    tlextra->normalGeometry = geometry();
			else
			    tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
		    }
		    if(data->fstrut_dirty)
			updateFrameStrut();
		    bounds.left += tlextra->fleft;
		    bounds.top += tlextra->ftop;
		    bounds.right -= tlextra->fright;
		    bounds.bottom -= tlextra->fbottom;
		}
		QRect orect(geometry().x(), geometry().y(), width(), height()),
		    nrect(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top);
		if(orect.size() != nrect.size()) { // no real point..
		    Rect oldr;
		    if(QTLWExtra *tlextra = d->topData()) 
			SetRect(&oldr, tlextra->normalGeometry.left(), tlextra->normalGeometry.top(), 
				tlextra->normalGeometry.right()+1, tlextra->normalGeometry.bottom()+1);
		    else
			SetRect(&oldr, orect.x(), orect.y(), orect.right(), orect.bottom());
		    SetWindowUserState(window, &oldr);

		    SetWindowStandardState(window, &bounds);
		    ZoomWindow(window, inZoomOut, false);

		    data->crect = nrect;
		    if(isVisible()) {
			//issue a resize
			QResizeEvent qre(size(), orect.size());
			QApplication::sendEvent(this, &qre);
			//issue a move
			QMoveEvent qme(pos(), orect.topLeft());
			QApplication::sendEvent(this, &qme);
		    }
		}
	    } else {
		ZoomWindow(window, inZoomIn, false);
	    }
	}
    }

    data->widget_state &= ~(WState_Minimized | WState_Maximized | WState_FullScreen);
    if(newstate & WindowMinimized)
	data->widget_state |= WState_Minimized;
    if(newstate & WindowMaximized)
	data->widget_state |= WState_Maximized;
    if(newstate & WindowFullScreen)
	data->widget_state |= WState_FullScreen;

    if(needShow)
	show();

    if(newstate & WindowActive)
	setActiveWindow();

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::raise()
{
    if(isDesktop())
	return;
    if(isTopLevel()) {
	//raise this window
	BringToFront(qt_mac_window_for((HIViewRef)winId()));
	//we get to be the active process now
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
	SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
    } else if(QWidget *p = parentWidget()) {
	if(p->d->children.indexOf(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.append(this);
	}
	HIViewSetZOrder((HIViewRef)winId(), kHIViewZOrderAbove, 0);
    }
}

void QWidget::lower()
{
    if(isDesktop())
	return;

    if(isTopLevel()) {
	SendBehind(qt_mac_window_for((HIViewRef)winId()), 0);
    } else if(QWidget *p = parentWidget()) {
	if(p->d->children.indexOf(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.insert(0, this);
	}
	HIViewSetZOrder((HIViewRef)winId(), kHIViewZOrderBelow, 0);
    }
}

void QWidget::stackUnder(QWidget *w)
{
    if(!w || isTopLevel() || isDesktop())
	return;

    QWidget *p = parentWidget();
    if(!p || p != w->parentWidget())
	return;
    int loc = p->d->children.indexOf(w);
    if(loc >= 0 && p->d->children.indexOf(this) >= 0) {
	p->d->children.remove(this);
	p->d->children.insert(loc, this);
    }
    HIViewSetZOrder((HIViewRef)winId(), kHIViewZOrderBelow, (HIViewRef)w->winId());
}

void QWidget::setGeometry_helper(int x, int y, int w, int h, bool isMove)
{
    if(isTopLevel() && isMove) {
	d->createTLExtra();
	d->topData()->is_moved = 1;
    }
    if(isDesktop())
	return;
    if(QWExtra *extra = d->extraData()) {	// any size restrictions?
	if(isTopLevel()) {
	    WindowPtr window = qt_mac_window_for((HIViewRef)winId());
            QWidgetPrivate::qt_mac_update_sizer(this);
	    if(testWFlags(WStyle_Maximize)) {
		if(extra->maxw && extra->maxh && extra->maxw == extra->minw
			&& extra->maxh == extra->minh)
		    ChangeWindowAttributes(window, kWindowNoAttributes, kWindowFullZoomAttribute);
		else
		    ChangeWindowAttributes(window, kWindowFullZoomAttribute, kWindowNoAttributes);
	    }
	}
	w = qMin(w,extra->maxw);
	h = qMin(h,extra->maxh);
	w = qMax(w,extra->minw);
	h = qMax(h,extra->minh);

	// Deal with size increment
	if(QTLWExtra *top = d->topData()) {
	    if(top->incw) {
		w = w/top->incw;
		w *= top->incw;
	    }
	    if(top->inch) {
		h = h/top->inch;
		h *= top->inch;
	    }
	}
    }
    if(w < 1)                                // invalid size
	w = 1;
    if(h < 1)
	h = 1;

    QPoint oldp = pos();
    QSize  olds = size();
    const bool isResize = (olds != QSize(w, h));
    if(!isTopLevel() && !isResize && QPoint(x, y) == oldp)
	return;
    const bool visible = isVisible();
    data->crect = QRect(x, y, w, h);
    //update the widget also..
    HIRect bounds = CGRectMake(x, y, w, h);
    if(isTopLevel()) //nice try..
	bounds.origin.x = bounds.origin.y = 0;
    HIViewSetFrame((HIViewRef)winId(), &bounds);

    if(isTopLevel()) {
	if(isResize && isMaximized())
	    clearWState(WState_Maximized);
	WindowPtr window = qt_mac_window_for((HIViewRef)winId());
	if(isMove)
	    MoveWindow(window, x, y, false);
	if(isResize)
	    SizeWindow(window, w, h, true);
    }

    if(isMove || isResize) {
	if(!visible) {
	    if(isMove && pos() != oldp)
		setAttribute(WA_PendingMoveEvent, true);
	    if(isResize)
		setAttribute(WA_PendingResizeEvent, true);
	} else {
	    if(isResize) { //send the resize event..
		QResizeEvent e(size(), olds);
		QApplication::sendEvent(this, &e);
	    }
	    if(isMove && pos() != oldp) { //send the move event..
		QMoveEvent e(pos(), oldp);
		QApplication::sendEvent(this, &e);
	    }
	}
    }
}

void QWidget::setMinimumSize(int minw, int minh)
{
    if(minw < 0 || minh < 0)
	qWarning("Qt: QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if(d->extraData()->minw == minw && d->extraData()->minh == minh)
	return;
    d->extraData()->minw = minw;
    d->extraData()->minh = minh;
    if(minw > width() || minh > height()) {
	bool resized = testAttribute(WA_Resized);
	resize(qMax(minw,width()), qMax(minh,height()));
	setAttribute(WA_Resized, resized); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if(maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 objectName("unnamed"), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX);
	maxw = qMin(maxw, QWIDGETSIZE_MAX);
	maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if(maxw < 0 || maxh < 0) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		 objectName("unnamed"), className(), maxw, maxh);
	maxw = qMax(maxw, 0);
	maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if(d->extraData()->maxw == maxw && d->extraData()->maxh == maxh)
	return;
    d->extraData()->maxw = maxw;
    d->extraData()->maxh = maxh;
    if(maxw < width() || maxh < height()) {
	bool resized = testAttribute(WA_Resized);
	resize(qMin(maxw,width()), qMin(maxh,height()));
	setAttribute(WA_Resized, resized); //not a user resize
    }
    updateGeometry();
}


void QWidget::setSizeIncrement(int w, int h)
{
    d->createTLExtra();
    d->topData()->incw = w;
    d->topData()->inch = h;
}

void QWidget::setBaseSize(int w, int h)
{
    d->createTLExtra();
    d->topData()->basew = w;
    d->topData()->baseh = h;
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    bool valid_rect = r.isValid();
    if(testWState(WState_BlockUpdates) &&  (valid_rect || !children()))
	return;

    if(!valid_rect) {	// scroll children
	QPoint pd(dx, dy);
	QWidgetList moved;
	QObjectList chldrn = children();
	for(int i = 0; i < chldrn.size(); i++) {  //first move all children
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget*)obj;
		if(!w->isTopLevel()) {
		    w->data->crect = QRect(w->pos() + pd, w->size());
		    HIRect bounds = CGRectMake(data->crect.x(), data->crect.y(), data->crect.width(), data->crect.height());
		    HIViewSetFrame((HIViewRef)winId(), &bounds);
		    moved.append(w);
		}
	    }
	}
        //now send move events (do not do this in the above loop, breaks QAquaFocusWidget)
	for(int i = 0; i < moved.size(); i++) {
	    QWidget *w = moved.at(i);
	    QMoveEvent e(w->pos(), w->pos() - pd);
	    QApplication::sendEvent(w, &e);
	}
    }
    HIRect scrollrect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    HIViewScrollRect((HIViewRef)winId(), valid_rect ? &scrollrect : 0, dx, dy);
}

int QWidget::metric(int m) const
{
    switch(m) {
    case QPaintDeviceMetrics::PdmHeightMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmHeight)*100)/288;
    case QPaintDeviceMetrics::PdmWidthMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmWidth)*100)/288;
    case QPaintDeviceMetrics::PdmHeight:
    case QPaintDeviceMetrics::PdmWidth: {
	HIRect rect;
	HIViewGetFrame((HIViewRef)winId(), &rect);
	if(m == QPaintDeviceMetrics::PdmWidth)
	    return (int)rect.size.width;
	return (int)rect.size.height; }
    case QPaintDeviceMetrics::PdmDepth:// FIXME : this is a lie in most cases
	return 16;
    case QPaintDeviceMetrics::PdmDpiX: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
	return 80;
    case QPaintDeviceMetrics::PdmDpiY: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiY:
	return 80;
    default: //leave this so the compiler complains when new ones are added
	qWarning("Qt: QWidget::metric unhandled parameter %d",m);
	return QPaintDevice::metric(m);// XXX
    }
    return 0;
}

void QWidgetPrivate::createSysExtra()
{
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->group = 0;
    extra->topextra->is_moved = 0;
    extra->topextra->resizer = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group)
	qt_mac_release_window_group(extra->topextra->group);
}

bool QWidget::acceptDrops() const
{
    return d->macDropEnabled;
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this; //mutable
    if(!data->fstrut_dirty) {
	that->data->fstrut_dirty = isVisible();
	return;
    }
    that->data->fstrut_dirty = false;
    QTLWExtra *top = that->d->topData();
    top->fleft = top->fright = top->ftop = top->fbottom = 0;
    if(!isDesktop() && isTopLevel()) {
	WindowPtr window = qt_mac_window_for((HIViewRef)winId());
	Rect window_r, content_r;
	//get bounding rects
	RgnHandle rgn = qt_mac_get_rgn();
	GetWindowRegion(window, kWindowStructureRgn, rgn);
	GetRegionBounds(rgn, &window_r);
	GetWindowRegion(window, kWindowContentRgn, rgn);
	GetRegionBounds(rgn, &content_r);
	qt_mac_dispose_rgn(rgn);
	//put into qt structure
	top->fleft = content_r.left - window_r.left;
	top->ftop = content_r.top - window_r.top;
	top->fright = window_r.right - content_r.right;
	top->fbottom = window_r.bottom - window_r.bottom;
    }
}

void QWidget::setAcceptDrops(bool on)
{
    if(on == d->macDropEnabled)
	return;
    d->macDropEnabled = on;
    SetControlDragTrackingEnabled((HIViewRef)winId(), on);
}

void QWidget::setMask(const QRegion &region)
{
    d->createExtra();
    if(region.isEmpty() && d->extraData()->mask.isEmpty())
	return;

    d->extra->mask = region;
    if(isTopLevel())
	ReshapeCustomWindow(qt_mac_window_for((HIViewRef)winId()));
    else
	HIViewReshapeStructure((HIViewRef)winId());
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}

void QWidget::clearMask()
{
    setMask(QRegion());
}

void QWidget::resetInputContext()
{
    qt_mac_unicode_reset_input(this);
}

void QWidget::setWindowOpacity(double level)
{
    if(!isTopLevel())
	return;

    level = qMin(qMax(level, 0), 1.0);
    QMacSavedPortInfo::setWindowAlpha(this, level);
    d->topData()->opacity = (uchar)(level * 255);
}

double QWidget::windowOpacity() const
{
    return isTopLevel() ? ((QWidget*)this)->d->topData()->opacity / 255.0 : 0.0;
}

QPaintEngine *QWidget::engine() const
{
    if(!d->paintEngine) {
#if defined( USE_CORE_GRAPHICS )
	const_cast<QWidget *>(this)->d->paintEngine = new QCoreGraphicsPaintEngine(const_cast<QWidget *>(this));
#else
	const_cast<QWidget *>(this)->d->paintEngine = new QQuickDrawPaintEngine(const_cast<QWidget *>(this));
#endif
    }
    return d->paintEngine;
}

