/* Copyright 1996 Daniel Dardailler.

   Permission to use, copy, modify, distribute, and sell this software
   for any purpose is hereby granted without fee, provided that the above
   copyright notice appear in all copies and that both that copyright
   notice and this permission notice appear in supporting documentation,
   and that the name of Daniel Dardailler not be used in advertising or
   publicity pertaining to distribution of the software without specific,
   written prior permission.  Daniel Dardailler makes no representations
   about the suitability of this software for any purpose.  It is
   provided "as is" without express or implied warranty.
************************************************************/

/***********************************************************/
/* Motif Drag&Drop Dynamic Protocol messaging API code */
/* Only requires Xlib layer - not MT safe */
/* Author: Daniel Dardailler, daniel@x.org */
/* Adapted by : Matt Koss, koss@napri.sk */
/***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "qapplication.h"
#include "qwidget.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

static Atom atom_message_type, atom_receiver_info, atom_src_property_type;
static Atom atom_motif_window, atom_target_list ;

static bool in_drop_site = false;
static Window cur_window = 0;
static QWidget *drop_widget = 0L;

static Atom Dnd_transfer_success, Dnd_transfer_failure;

static Atom Dnd_selection;
static Time Dnd_selection_time;

static Atom * src_targets ;
static ushort num_src_targets ;

extern bool qt_motifdnd_active;

// this stuff is copied from qclipboard_x11.cpp

extern bool qt_xclb_wait_for_event( Display *dpy, Window win, int type,
				    XEvent *event, int timeout );
extern bool qt_xclb_read_property( Display *dpy, Window win, Atom property,
				   bool deleteProperty,
				   QByteArray *buffer, int *size, Atom *type,
				   int *format, bool nullterm );

// Motif definitions
#define DndVersion 1
#define DndRevision 0
#define DndIncludeVersion (DndVersion * 10 + DndRevision)

/* The following values are used in the DndData structure */

/* protocol style */
#define DND_DRAG_NONE            0
#define DND_DRAG_DROP_ONLY       1
#define DND_DRAG_DYNAMIC         5

/* message type */
#define DND_TOP_LEVEL_ENTER   0
#define DND_TOP_LEVEL_LEAVE   1
#define DND_DRAG_MOTION       2
#define DND_DROP_SITE_ENTER   3
#define DND_DROP_SITE_LEAVE   4
#define DND_DROP_START        5
#define DND_OPERATION_CHANGED 8

/* operation(s) */
#define DND_NOOP	0L
#define DND_MOVE 	(1L << 0)
#define DND_COPY	(1L << 1)
#define DND_LINK	(1L << 2)

/* status */
#define DND_NO_DROP_SITE        1
#define DND_INVALID_DROP_SITE   2
#define DND_VALID_DROP_SITE	3

/* completion */
#define DND_DROP        0
#define DND_DROP_HELP   1
#define DND_DROP_CANCEL 2

#define BYTE unsigned char
#define CARD32 unsigned int
#define CARD16 unsigned short
#define INT16  signed short

/* Client side structure used in the API */
typedef struct {
    unsigned char       reason;  /* message type: DND_TOP_LEVEL_ENTER, etc */
    Time                time ;
    unsigned char       operation;
    unsigned char       operations;
    unsigned char       status;
    unsigned char       completion;
    short               x ;
    short               y ;
    Window              src_window ;
    Atom                property ;
} DndData ;


typedef struct _DndSrcProp {
    BYTE                byte_order ;
    BYTE                protocol_version ;
    CARD16              target_index ;
    CARD32              selection ;
} DndSrcProp ;

typedef struct _DndReceiverProp {
    BYTE                byte_order ;
    BYTE                protocol_version ;
    BYTE                protocol_style ;
    BYTE                pad1;
    CARD32              proxy_window;
    CARD16              num_drop_sites ;
    CARD16              pad2;
    CARD32              total_size;
} DndReceiverProp ;

/* need to use some union hack since window and property are in
   different order depending on the message ... */
typedef struct _DndTop {
    CARD32		src_window;
    CARD32		property;
} DndTop ;

typedef struct _DndPot {
    INT16		x;
    INT16		y;
    CARD32		property;
    CARD32		src_window;
} DndPot ;

typedef struct _DndMessage {
    BYTE		reason;
    BYTE		byte_order;
    CARD16		flags;
    CARD32		time;
    union {
	DndTop top ;
	DndPot pot ;
    } data ;
} DndMessage ;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_target_lists;
    CARD32	data_size;
    /* then come series of CARD16,CARD32,CARD32,CARD32... */
} DndTargets;


/* protocol version */
#define DND_PROTOCOL_VERSION 0


#define DND_EVENT_TYPE_MASK  ((BYTE)0x80)
#define DND_EVENT_TYPE_SHIFT 7
#define DND_CLEAR_EVENT_TYPE  ((BYTE)0x7F)

/* message_type is data[0] of the client_message
   this return 1 (receiver bit up) or 0 (initiator) */
#define DND_GET_EVENT_TYPE(message_type) \
((char) (((message_type) & DND_EVENT_TYPE_MASK) >> DND_EVENT_TYPE_SHIFT))

/* event_type can be 0 (initiator) or 1 (receiver) */
#define DND_SET_EVENT_TYPE(event_type) \
(((BYTE)(event_type) << DND_EVENT_TYPE_SHIFT) & DND_EVENT_TYPE_MASK)


#define DND_OPERATION_MASK ((CARD16) 0x000F)
#define DND_OPERATION_SHIFT 0
#define DND_STATUS_MASK ((CARD16) 0x00F0)
#define DND_STATUS_SHIFT 4
#define DND_OPERATIONS_MASK ((CARD16) 0x0F00)
#define DND_OPERATIONS_SHIFT 8
#define DND_COMPLETION_MASK ((CARD16) 0xF000)
#define DND_COMPLETION_SHIFT 12

#define DND_GET_OPERATION(flags) \
((unsigned char) \
(((flags) & DND_OPERATION_MASK) >> DND_OPERATION_SHIFT))

#define DND_SET_OPERATION(operation) \
(((CARD16)(operation) << DND_OPERATION_SHIFT)\
& DND_OPERATION_MASK)

#define DND_GET_STATUS(flags) \
((unsigned char) \
(((flags) & DND_STATUS_MASK) >> DND_STATUS_SHIFT))

#define DND_SET_STATUS(status) \
(((CARD16)(status) << DND_STATUS_SHIFT)\
& DND_STATUS_MASK)

#define DND_GET_OPERATIONS(flags) \
((unsigned char) \
(((flags) & DND_OPERATIONS_MASK) >> DND_OPERATIONS_SHIFT))

#define DND_SET_OPERATIONS(operation) \
(((CARD16)(operation) << DND_OPERATIONS_SHIFT)\
& DND_OPERATIONS_MASK)

#define DND_GET_COMPLETION(flags) \
((unsigned char) \
(((flags) & DND_COMPLETION_MASK) >> DND_COMPLETION_SHIFT))

#define DND_SET_COMPLETION(completion) \
(((CARD16)(completion) << DND_COMPLETION_SHIFT)\
& DND_COMPLETION_MASK)


#define SWAP4BYTES(l) {\
struct { unsigned t :32;} bit32;\
char n,	*tp = (char *) &bit32;\
bit32.t = l;\
n = tp[0]; tp[0] = tp[3]; tp[3] = n;\
n = tp[1]; tp[1] = tp[2]; tp[2] = n;\
l = bit32.t;\
}

#define SWAP2BYTES(s) {\
struct { unsigned t :16; } bit16;\
char n, *tp = (char *) &bit16;\
bit16.t = s;\
n = tp[0]; tp[0] = tp[1]; tp[1] = n;\
s = bit16.t;\
}


/** Private extern functions */

static unsigned char DndByteOrder (void);


/***** Targets/Index stuff */

typedef struct {
    int	    num_targets;
    Atom    *targets;
} DndTargetsTableEntryRec, * DndTargetsTableEntry;

typedef struct {
    int	num_entries;
    DndTargetsTableEntry entries;
} DndTargetsTableRec, * DndTargetsTable;


static int _DndTargetsToIndex(Display * display, Atom * targets,
			      int num_targets);

static int _DndIndexToTargets(Display * display,
			      int index,
			      Atom ** targets);

/////////////////////////////////////////////////////////////////

static void InitAtoms(Display * dpy)
{
    if (atom_message_type) return ; /* already Initialized */

    /* Init atoms used in the com */

    atom_message_type =  XInternAtom(dpy, "_MOTIF_DRAG_AND_DROP_MESSAGE", False);
    atom_src_property_type = XInternAtom(dpy, "_MOTIF_DRAG_INITIATOR_INFO", False);
    atom_receiver_info = XInternAtom(dpy, "_MOTIF_DRAG_RECEIVER_INFO", False);
    atom_motif_window = XInternAtom(dpy, "_MOTIF_DRAG_WINDOW", False);
    atom_target_list = XInternAtom(dpy, "_MOTIF_DRAG_TARGETS", False);

    Dnd_transfer_success = XInternAtom( qt_xdisplay(), "XmTRANSFER_SUCCESS", False);
    Dnd_transfer_failure = XInternAtom( qt_xdisplay(), "XmTRANSFER_FAILURE", False);

    char my_dnd_selection_name[30] ;
    sprintf(my_dnd_selection_name, "_MY_DND_SELECTION_%d", getpid());
    Dnd_selection = XInternAtom( qt_xdisplay(), my_dnd_selection_name, False);

}




static unsigned char DndByteOrder (void)
{
    static unsigned char byte_order = 0;

    if (!byte_order) {
	unsigned int endian = 1;
	byte_order = (*((char *)&endian))?'l':'B';
    }
    return byte_order ;
}


/* Position the _MOTIF_DRAG_INITIATOR_INFO property on the source window.
   Called by the source of the drag to indicate the
   supported target list (thru the index scheme) */
static void DndWriteSourceProperty(Display * dpy,
				   Window window, Atom dnd_selection,
				   Atom * targets, unsigned short num_targets)
{
    DndSrcProp src_prop ;

    InitAtoms(dpy);

    src_prop.byte_order = DndByteOrder() ;
    src_prop.protocol_version = DND_PROTOCOL_VERSION;

    int tmp = _DndTargetsToIndex(dpy, targets, num_targets);
    src_prop.target_index = tmp;

    if ( tmp == -1 ) return;

    src_prop.selection = dnd_selection ;

    /* write the buffer to the property */
    XChangeProperty (dpy, window, dnd_selection, atom_src_property_type,
		     8, PropModeReplace, (unsigned char *)&src_prop,
		     sizeof(DndSrcProp));
}

static void
DndReadSourceProperty(Display * dpy,
		      Window window, Atom dnd_selection,
		      Atom ** targets, unsigned short * num_targets)
{
    DndSrcProp * src_prop = NULL;
    Atom type ;
    int format ;
    unsigned long bytesafter, lengthRtn;

    InitAtoms(dpy);

    if ((XGetWindowProperty (dpy, window, dnd_selection, 0L, 100000L,
			     False, atom_src_property_type, &type,
			     &format, &lengthRtn, &bytesafter,
			     (unsigned char **) &src_prop) != Success)
	|| (type == None)) {
	*num_targets = 0;
	return ;
    }

    if (src_prop->byte_order != DndByteOrder()) {
	SWAP2BYTES(src_prop->target_index);
	SWAP4BYTES(src_prop->selection);
    }

    *num_targets = _DndIndexToTargets(dpy, src_prop->target_index, targets);

    XFree((char*)src_prop);
}


/* Position the _MOTIF_DRAG_RECEIVER_INFO property on the dropsite window.
   Called by the receiver of the drop to indicate the
   supported protocol style : dynamic, drop_only or none */
static void DndWriteReceiverProperty(Display * dpy, Window window,
				     unsigned char protocol_style)
{
    DndReceiverProp receiver_prop ;

    InitAtoms(dpy);

    receiver_prop.byte_order = DndByteOrder() ;
    receiver_prop.protocol_version = DND_PROTOCOL_VERSION;
    receiver_prop.protocol_style = protocol_style ;
    receiver_prop.proxy_window =  None ;
    receiver_prop.num_drop_sites = 0 ;
    receiver_prop.total_size = sizeof(DndReceiverProp);

    /* write the buffer to the property */
    XChangeProperty (dpy, window, atom_receiver_info, atom_receiver_info,
		     8, PropModeReplace,
		     (unsigned char *)&receiver_prop,
		     sizeof(DndReceiverProp));
}


/* protocol style equiv (preregister stuff really) */
#define DND_DRAG_DROP_ONLY_EQUIV 3
#define DND_DRAG_DYNAMIC_EQUIV1  2
#define DND_DRAG_DYNAMIC_EQUIV2  4

static void DndReadReceiverProperty(Display * dpy, Window window,
				    unsigned char * protocol_style)
{
    DndReceiverProp *receiver_prop = NULL ;
    Atom type ;
    int format ;
    unsigned long bytesafter, lengthRtn;

    InitAtoms(dpy);

    if ((XGetWindowProperty (dpy, window,
			     atom_receiver_info, /* _MOTIF_DRAG_RECEIVER_INFO */
			     0L, 100000L, False, atom_receiver_info,
			     &type, &format, &lengthRtn, &bytesafter,
			     (unsigned char **) &receiver_prop) != Success)
	|| (type == None)) {
	/* no property => no d&d */
	*protocol_style = DND_DRAG_NONE ;
	return ;
    }

    /* in dynamic we don't really care about byte swapping since
       the pertinent info is all expressed in one byte quantities */
    *protocol_style = receiver_prop->protocol_style;

    /* do the equiv stuff for Motif pre-register */
    if (*protocol_style == DND_DRAG_DROP_ONLY_EQUIV)
	*protocol_style = DND_DRAG_DROP_ONLY ;
    else
	if (*protocol_style == DND_DRAG_DYNAMIC_EQUIV1 ||
	    *protocol_style == DND_DRAG_DYNAMIC_EQUIV2)
	    *protocol_style = DND_DRAG_DYNAMIC ;

    XFree((void *)receiver_prop) ;

}

/* Produce a client message to be sent by the caller */
static void DndFillClientMessage(Display * dpy, Window window,
				 XClientMessageEvent *cm,
				 DndData * dnd_data,
				 char receiver)
{
    DndMessage * dnd_message = (DndMessage*)&cm->data.b[0] ;

    InitAtoms(dpy);

    cm->display = dpy;
    cm->type = ClientMessage;
    cm->serial = LastKnownRequestProcessed(dpy);
    cm->send_event = True;
    cm->window = window;
    cm->format = 8;
    cm->message_type = atom_message_type ;/* _MOTIF_DRAG_AND_DROP_MESSAGE */

    dnd_message->reason = dnd_data->reason | DND_SET_EVENT_TYPE(receiver);

    dnd_message->byte_order = DndByteOrder();

    /* we're filling in flags with more stuff that necessary,
       depending on the reason, but it doesn't matter */
    dnd_message->flags = 0 ;
    dnd_message->flags |= DND_SET_STATUS(dnd_data->status) ;
    dnd_message->flags |= DND_SET_OPERATION(dnd_data->operation) ;
    dnd_message->flags |= DND_SET_OPERATIONS(dnd_data->operations) ;
    dnd_message->flags |= DND_SET_COMPLETION(dnd_data->completion) ;

    dnd_message->time = dnd_data->time ;

    switch(dnd_data->reason) {
    case DND_DROP_SITE_LEAVE: break ;
    case DND_TOP_LEVEL_ENTER:
    case DND_TOP_LEVEL_LEAVE:
	dnd_message->data.top.src_window = dnd_data->src_window ;
	dnd_message->data.top.property = dnd_data->property ;
	break ; /* cannot fall thru since the byte layout is different in
		   both set of messages, see top and pot union stuff */

    case DND_DRAG_MOTION:
    case DND_OPERATION_CHANGED:
    case DND_DROP_SITE_ENTER:
    case DND_DROP_START:
	dnd_message->data.pot.x = dnd_data->x ; /* mouse position */
	dnd_message->data.pot.y = dnd_data->y ;
	dnd_message->data.pot.src_window = dnd_data->src_window ;
	dnd_message->data.pot.property = dnd_data->property ;
	break ;
    default:
	break ;
    }

}

static Bool DndParseClientMessage(XClientMessageEvent *cm, DndData * dnd_data,
				  char * receiver)
{
    DndMessage * dnd_message = (DndMessage*)&cm->data.b[0] ;

    InitAtoms(cm->display);

    if (cm->message_type != atom_message_type) {
	return False ;
    }

    if (dnd_message->byte_order != DndByteOrder()) {
	SWAP2BYTES(dnd_message->flags);
	SWAP4BYTES(dnd_message->time);
    } /* do the rest in the switch */

    dnd_data->reason = dnd_message->reason  ;
    if (DND_GET_EVENT_TYPE(dnd_data->reason))
	*receiver = 1 ;
    else
	*receiver = 0 ;
    dnd_data->reason &= DND_CLEAR_EVENT_TYPE ;

    dnd_data->time = dnd_message->time ;

    /* we're reading in more stuff that necessary. but who cares */
    dnd_data->status = DND_GET_STATUS(dnd_message->flags) ;
    dnd_data->operation = DND_GET_OPERATION(dnd_message->flags) ;
    dnd_data->operations = DND_GET_OPERATIONS(dnd_message->flags) ;
    dnd_data->completion = DND_GET_COMPLETION(dnd_message->flags) ;

    switch(dnd_data->reason) {
    case DND_TOP_LEVEL_ENTER:
    case DND_TOP_LEVEL_LEAVE:
	if (dnd_message->byte_order != DndByteOrder()) {
	    SWAP4BYTES(dnd_message->data.top.src_window);
	    SWAP4BYTES(dnd_message->data.top.property);
	}
	dnd_data->src_window = dnd_message->data.top.src_window ;
	dnd_data->property = dnd_message->data.top.property ;
	break ; /* cannot fall thru, see above comment in write msg */

    case DND_DRAG_MOTION:
    case DND_OPERATION_CHANGED:
    case DND_DROP_SITE_ENTER:
    case DND_DROP_START:
	if (dnd_message->byte_order != DndByteOrder()) {
	    SWAP2BYTES(dnd_message->data.pot.x);
	    SWAP2BYTES(dnd_message->data.pot.y);
	    SWAP4BYTES(dnd_message->data.pot.property);
	    SWAP4BYTES(dnd_message->data.pot.src_window);
	}
	dnd_data->x = dnd_message->data.pot.x ;
	dnd_data->y = dnd_message->data.pot.y ;
	dnd_data->property = dnd_message->data.pot.property ;
	dnd_data->src_window = dnd_message->data.pot.src_window ;
	break ;

    case DND_DROP_SITE_LEAVE:
	break;
    default:
	break ;
    }

    return True ;
}


static Window MotifWindow(Display *display )
{
    Atom            type;
    int             format;
    unsigned long   size;
    unsigned long   bytes_after;
    Window         *property = NULL;
    Window	    motif_window ;

    /* this version does no caching, so it's slow: round trip each time */

    if ((XGetWindowProperty (display, DefaultRootWindow(display),
			     atom_motif_window,
			     0L, 100000L, False, AnyPropertyType,
			     &type, &format, &size, &bytes_after,
			     (unsigned char **) &property) == Success) &&
	(type != None)) {
	motif_window = *property;
    } else {
	XSetWindowAttributes sAttributes;

	/* really, this should be done on a separate connection,
	   with XSetCloseDownMode (RetainPermanent), so that
	   others don't have to recreate it; hopefully, some real
	   Motif application will be around to do it */

	sAttributes.override_redirect = True;
	sAttributes.event_mask = PropertyChangeMask;
	motif_window = XCreateWindow (display,
				      DefaultRootWindow (display),
				      -170, -560, 1, 1, 0, 0,
				      InputOnly, CopyFromParent,
				      (CWOverrideRedirect |CWEventMask),
				      &sAttributes);
	XMapWindow (display, motif_window);
    }

    if (property) {
	XFree ((char *)property);
    }

    return (motif_window);
}


static DndTargetsTable TargetsTable(Display *display)
{
    Atom            type;
    int             format;
    unsigned long   size;
    unsigned long   bytes_after;
    Window motif_window = MotifWindow(display) ;
    DndTargets * target_prop;
    DndTargetsTable targets_table ;
    int i,j ;
    char * target_data ;

    /* this version does no caching, so it's slow: round trip each time */
    /* ideally, register for property notify on this target_list
       atom and update when necessary only */

    if ((XGetWindowProperty (display, motif_window,	
			     atom_target_list, 0L, 100000L,	
			     False, atom_target_list,	
			     &type, &format,	&size, &bytes_after,
			     (unsigned char **) &target_prop) != Success) ||
	type == None) {
	qWarning("QMotifDND: cannot get property on motif window");
	return NULL ;
    }

    if (target_prop->protocol_version != DND_PROTOCOL_VERSION) {
	qWarning("QMotifDND: protocol mismatch");
    }

    if (target_prop->byte_order != DndByteOrder()) {
	/* need to swap num_target_lists and size */
	SWAP2BYTES(target_prop->num_target_lists);
	SWAP4BYTES(target_prop->data_size);
    }

    /* now parse DndTarget prop data in a TargetsTable */

    targets_table = (DndTargetsTable)malloc(sizeof(DndTargetsTableRec));
    targets_table->num_entries = target_prop->num_target_lists ;
    targets_table->entries = (DndTargetsTableEntry)
			     malloc(sizeof(DndTargetsTableEntryRec) * target_prop->num_target_lists);

    target_data = (char*)target_prop + sizeof(*target_prop) ;

    for (i = 0 ; i < targets_table->num_entries; i++) {
	CARD16 num_targets ;
	CARD32 atom ;

	memcpy(&num_targets, target_data, 2);
	target_data += 2;

	/* potential swap needed here */
	if (target_prop->byte_order != DndByteOrder())
	    SWAP2BYTES(num_targets);

	targets_table->entries[i].num_targets = num_targets ;
	targets_table->entries[i].targets = (Atom *)
					    malloc(sizeof(Atom) * targets_table->entries[i].num_targets);


	for (j = 0; j < num_targets; j++) {
	    memcpy(&atom, target_data, 4 );
	    target_data += 4;

	    /* another potential swap needed here */
	    if (target_prop->byte_order != DndByteOrder())
		SWAP4BYTES(atom);

	    targets_table->entries[i].targets[j] = (Atom) atom ;
	}
    }

    if (target_prop) {
	XFree((char *)target_prop);
    }

    return targets_table ;
}


static int AtomCompare(const void *atom1, const void *atom2 )
{
    return (*((Atom *) atom1) - *((Atom *) atom2));
}


static int _DndTargetsToIndex(Display * display,
		   Atom * targets, int num_targets)
{
    int		i, j;
    Atom		*sorted_targets;
    DndTargetsTable	targets_table;
    int index = -1 ;

    InitAtoms(display) ;

    if (!(targets_table = TargetsTable (display))) {
	qWarning("QMotifDND: cannot find the target table");
	return -1 ;
    }

    /* sort the given target list */
    sorted_targets = (Atom *) malloc(sizeof(Atom) * num_targets);
    memcpy (sorted_targets, targets, sizeof(Atom) * num_targets);
    qsort ((void *)sorted_targets, (size_t)num_targets, (size_t)sizeof(Atom),
	   AtomCompare);

    /* look for a match */

    for (i = 0; i < targets_table->num_entries; i++) {
	if (num_targets == targets_table->entries[i].num_targets) {
	    for (j = 0; j < num_targets; j++) {
		if (sorted_targets[j] !=
		    targets_table->entries[i].targets[j]) {
		    break;
		}
	    }
	    if (j == num_targets) {
		index = i ;
		break ;
	    }
	}
    }

    XFree ((char *)sorted_targets);
    /* free the target table and its guts */
    for (i=0 ; i < targets_table->num_entries; i++)
	XFree((char*)targets_table->entries[i].targets);
    XFree((char*)targets_table);

  if (index == -1 )
      qWarning("QMotifDND: TargetsToIndex: non existing target list: unsupported");
    /* to support: need to grab the server, add our target list
       to the property, return index = num_entries++, and ungrab */
    return index ;
}


int _DndIndexToTargets(Display * display,
		       int index,
		       Atom ** targets)
{
    DndTargetsTable	targets_table;
    int i ;

    /* again, slow: no caching here, alloc/free each time */

    InitAtoms(display) ;

    if (!(targets_table = TargetsTable (display)) ||
	(index >= targets_table->num_entries)) {
	return -1;
    }

    /* transfer the correct target list index */
    *targets = (Atom*)malloc(sizeof(Atom)*targets_table->
			     entries[index].num_targets);
    memcpy((char*)*targets,
	   (char*)targets_table->entries[index].targets,
	   sizeof(Atom)*targets_table->entries[index].num_targets);

    /* free the target table and its guts */
    for (i=0 ; i < targets_table->num_entries; i++)
	XFree((char*)targets_table->entries[i].targets);

    int tmp = targets_table->entries[index].num_targets;
    XFree((char*)targets_table);

    return tmp; // targets_table->entries[index].num_targets;
}


QByteArray qt_motifdnd_obtain_data()
{
    QByteArray result;

    if ( XGetSelectionOwner( qt_xdisplay(),
			     Dnd_selection ) == None ) {
	return result; // should never happen?
    }

    QWidget* tw = drop_widget;
    if ( drop_widget->isDesktop() ) {
	tw = new QWidget;
    }

    // convert selection to a string property
    XConvertSelection (qt_xdisplay(), Dnd_selection, XA_STRING,
		       Dnd_selection, tw->winId(), Dnd_selection_time);

    XFlush( qt_xdisplay() );

    XEvent xevent;
    bool got=qt_xclb_wait_for_event( qt_xdisplay(),
				     tw->winId(),
				     SelectionNotify, &xevent, 5000);
    if ( got ) {
	Atom type;

	if ( qt_xclb_read_property( qt_xdisplay(),
				    tw->winId(),
				    Dnd_selection, TRUE,
				    &result, 0, &type, 0, TRUE ) ) {
	}
    }

    //   we have to convert selection in order to indicate success to the initiator
    XConvertSelection (qt_xdisplay(), Dnd_selection, Dnd_transfer_success,
		       Dnd_selection, tw->winId(), Dnd_selection_time);

    // wait again for SelectionNotify event
    qt_xclb_wait_for_event( qt_xdisplay(),
			    tw->winId(),
			    SelectionNotify, &xevent, 5000);

    if ( drop_widget->isDesktop() ) {
	delete tw;
    }

    return result;
}


void qt_motifdnd_enable( QWidget *widget, bool )
{
    DndWriteReceiverProperty( widget->x11Display(), widget->winId(),
			      DND_DRAG_DYNAMIC);
}


void qt_motifdnd_handle_msg( QWidget * /* w */ , const XEvent * xe, bool /* passive */ )
{

    XEvent event = *xe;
    XClientMessageEvent cm ;
    DndData dnd_data ;
    char receiver ;

    if (!(DndParseClientMessage ((XClientMessageEvent*)&event,
				 &dnd_data, &receiver))) {
	return;
    }

    switch ( dnd_data.reason ) {

    case DND_DRAG_MOTION:

	{
	    /* check if in drop site, and depending on the state,
	       send a drop site enter or drop site leave or echo */

	    QPoint p( dnd_data.x, dnd_data.y );
	    QWidget *c = QApplication::widgetAt( p, true );

	    while ( c && !c->acceptDrops() && !c->isTopLevel() ) {
		p = c->mapToParent( p );
		c = c->parentWidget();
	    }

	    QDragMoveEvent me( p );
	    QDropEvent::Action accepted_action = QDropEvent::Copy;
	    me.setAction(accepted_action);

	    if ( c != 0L && c->acceptDrops() ) {

		if ( drop_widget != 0L && drop_widget->acceptDrops() &&
		     drop_widget != c ) {
		    QDragLeaveEvent e;
		    QApplication::sendEvent( drop_widget, &e );
		    QDragEnterEvent de( p );
		    QApplication::sendEvent( c, &de );
		}

		drop_widget = c;
	
		if (!in_drop_site) {
		    in_drop_site = True ;

		    dnd_data.reason = DND_DROP_SITE_ENTER ;
		    dnd_data.time = CurrentTime ;
		    dnd_data.operation = DND_MOVE|DND_COPY;
		    dnd_data.operations = DND_MOVE|DND_COPY;

		    DndFillClientMessage (event.xclient.display,
					  cur_window,
					  &cm, &dnd_data, 0);

		    XSendEvent(event.xbutton.display,
			       cur_window, False, 0,
			       (XEvent *)&cm) ;

		    QDragEnterEvent de( p );
		    QApplication::sendEvent( drop_widget, &de );
		    if ( de.isAccepted() ) {
			me.accept( de.answerRect() );
		    } else {
			me.ignore( de.answerRect() );
		    }

		} else {
		    dnd_data.reason = DND_DRAG_MOTION ;
		    dnd_data.time = CurrentTime ;
		    dnd_data.operation = DND_MOVE|DND_COPY;
		    dnd_data.operations = DND_MOVE|DND_COPY;

		    DndFillClientMessage (event.xclient.display,
					  cur_window,
					  &cm, &dnd_data, 0);

		    XSendEvent(event.xbutton.display,
			       cur_window, False, 0,
			       (XEvent *)&cm) ;

		    QApplication::sendEvent( drop_widget, &me );
		}
	    } else {
		if (in_drop_site) {
		    in_drop_site = False ;

		    dnd_data.reason = DND_DROP_SITE_LEAVE ;
		    dnd_data.time = CurrentTime ;

		    DndFillClientMessage (event.xclient.display,
					  cur_window,
					  &cm, &dnd_data, 0);

		    XSendEvent(event.xbutton.display,
			       cur_window, False, 0,
			       (XEvent *)&cm) ;

		    QDragLeaveEvent e;
		    QApplication::sendEvent( drop_widget, &e );
		}
	    }
	}
	break;

    case DND_TOP_LEVEL_ENTER:

	/* get the size of our drop site for later use */

	cur_window = dnd_data.src_window ;
	qt_motifdnd_active = TRUE;
		
	/* no answer needed, just read source property */
	DndReadSourceProperty (event.xclient.display,
			       cur_window,
			       dnd_data.property,
			       &src_targets, &num_src_targets);
	break;

    case DND_TOP_LEVEL_LEAVE:
	/* no need to do anything */
	break;

    case DND_OPERATION_CHANGED:
	/* need to echo */
	break;

    case DND_DROP_START:
	if (!in_drop_site) {
	    // we have to convert selection in order to indicate failure to the initiator
	    XConvertSelection (qt_xdisplay(), dnd_data.property, Dnd_transfer_failure,
			       dnd_data.property, cur_window, dnd_data.time);
	    QDragLeaveEvent e;
	    QApplication::sendEvent( drop_widget, &e );
	    drop_widget = 0;
	    return;
	}

	/* need to echo and then request a convert */
	dnd_data.reason = DND_DROP_START ;

	DndFillClientMessage (event.xclient.display,
			      drop_widget->winId(),
			      &cm, &dnd_data, 0);

	XSendEvent(event.xbutton.display,
		   cur_window, False, 0,
		   (XEvent *)&cm) ;

	// store selection and its time
	Dnd_selection = dnd_data.property;
	Dnd_selection_time = dnd_data.time;

	QPoint p( dnd_data.x, dnd_data.y );
	QDropEvent de( p );
	de.setAction( QDropEvent::Copy );
	QApplication::sendEvent( drop_widget, &de );

	if (in_drop_site)
	    in_drop_site = False ;

	drop_widget = 0;
	cur_window = 0;
	break;
    }   //  end of switch ( dnd_data.reason )
}
