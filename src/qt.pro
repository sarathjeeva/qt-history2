TEMPLATE	= lib
CONFIG		= qt warn_on release

# Comment out the next line if you don't want use png/zlib in 3rdparty
CONFIG		+= png zlib

# Uncomment the next line if you want to use the standard png/zlib libs
# unix:LIBS	+= -lpng -lz

#DEFINES += QT_NO_ASCII_CAST
#DEFINES += QT_NO_CAST_ASCII

unix:CONFIG	+= x11inc
unix:TMAKE_CXXFLAGS   += -DQT_FATAL_ASSERT

win32:TMAKE_CFLAGS     += -DUNICODE
win32:TMAKE_CXXFLAGS   += -DUNICODE
#win32:TMAKE_CFLAGS     += -MD
#win32:TMAKE_CXXFLAGS   += -MD

PNG_INCLUDEPATH		= 3rdparty/libpng
ZLIB_INCLUDEPATH	= 3rdparty/zlib
png:INCLUDEPATH	       += $$PNG_INCLUDEPATH
zlib:INCLUDEPATH       += $$ZLIB_INCLUDEPATH
win32:INCLUDEPATH      += tmp
win32-borland:INCLUDEPATH += kernel

win32:MOC_DIR	= tmp
win32:OBJECTS_DIR = tmp

win32:DIALOGS_H	= ../include
win32:KERNEL_H	= ../include
win32:TOOLS_H	= ../include
win32:WIDGETS_H	= ../include

unix:DIALOGS_H	= dialogs
unix:KERNEL_H	= kernel
unix:TOOLS_H	= tools
unix:WIDGETS_H	= widgets

win32:DEPENDPATH = ../include
unix:DEPENDPATH	= $$DIALOGS_H:$$KERNEL_H:$$TOOLS_H:$$WIDGETS_H

HEADERS		= $$DIALOGS_H/qcolordialog.h \
		  $$DIALOGS_H/qfiledialog.h \
		  $$DIALOGS_H/qfontdialog.h \
		  $$DIALOGS_H/qmessagebox.h \
		  $$DIALOGS_H/qprogressdialog.h \
		  $$DIALOGS_H/qtabdialog.h \
		  $$DIALOGS_H/qwizard.h \
		  $$KERNEL_H/qabstractlayout.h \
		  $$KERNEL_H/qaccel.h \
		  $$KERNEL_H/qapplication.h \
		  $$KERNEL_H/qasyncimageio.h \
		  $$KERNEL_H/qasyncio.h \
		  $$KERNEL_H/qbitmap.h \
		  $$KERNEL_H/qbrush.h \
		  $$KERNEL_H/qclipboard.h \
		  $$KERNEL_H/qcolor.h \
		  $$KERNEL_H/qconnection.h \
		  $$KERNEL_H/qcursor.h \
		  $$KERNEL_H/qdialog.h \
		  $$KERNEL_H/qdragobject.h \
		  $$KERNEL_H/qdrawutil.h \
		  $$KERNEL_H/qdropsite.h \
		  $$KERNEL_H/qevent.h \
		  $$KERNEL_H/qfocusdata.h \
		  $$KERNEL_H/qfont.h \
		  $$KERNEL_H/qfontdata.h \
		  $$KERNEL_H/qfontinfo.h \
		  $$KERNEL_H/qfontmetrics.h \
		  $$KERNEL_H/qgif.h \
		  $$KERNEL_H/qiconset.h \
		  $$KERNEL_H/qimage.h \
		  $$KERNEL_H/qkeycode.h \
		  $$KERNEL_H/qlayout.h \
		  $$KERNEL_H/qlayoutengine.h \
		  $$KERNEL_H/qtranslator.h \
		  $$KERNEL_H/qmetaobject.h \
		  $$KERNEL_H/qmime.h \
		  $$KERNEL_H/qmovie.h \
		  $$KERNEL_H/qnamespace.h \
		  $$KERNEL_H/qobject.h \
		  $$KERNEL_H/qobjectdefs.h \
		  $$KERNEL_H/qobjectdict.h \
		  $$KERNEL_H/qobjectlist.h \
		  $$KERNEL_H/qpaintdevice.h \
		  $$KERNEL_H/qpaintdevicedefs.h \
		  $$KERNEL_H/qpainter.h \
		  $$KERNEL_H/qpalette.h \
		  $$KERNEL_H/qpaintdevicemetrics.h \
		  $$KERNEL_H/qpen.h \
		  $$KERNEL_H/qpicture.h \
		  $$KERNEL_H/qpixmap.h \
		  $$KERNEL_H/qpixmapcache.h \
		  $$KERNEL_H/qpngio.h \
		  $$KERNEL_H/qpointarray.h \
		  $$KERNEL_H/qpoint.h \
		  $$KERNEL_H/qprinter.h \
		  $$KERNEL_H/qrect.h \
		  $$KERNEL_H/qregion.h \
		  $$KERNEL_H/qsemimodal.h \
		  $$KERNEL_H/qsessionmanager.h \
		  $$KERNEL_H/qsignal.h \
		  $$KERNEL_H/qsignalmapper.h \
		  $$KERNEL_H/qsignalslotimp.h \
		  $$KERNEL_H/qsimplerichtext.h \
		  $$KERNEL_H/qsize.h \
		  $$KERNEL_H/qsizegrip.h \
		  $$KERNEL_H/qsizepolicy.h \
		  $$KERNEL_H/qsocketnotifier.h \
		  $$KERNEL_H/qstyle.h \
		  $$KERNEL_H/qstylesheet.h \
		  $$KERNEL_H/qtimer.h \
		  $$KERNEL_H/qwidget.h \
		  $$KERNEL_H/qwidgetintdict.h \
		  $$KERNEL_H/qwidgetlist.h \
		  $$KERNEL_H/qwindowdefs.h \
		  $$KERNEL_H/qwmatrix.h \
		  $$TOOLS_H/qarray.h \
		  $$TOOLS_H/qasciicache.h \
		  $$TOOLS_H/qasciidict.h \
		  $$TOOLS_H/qbitarray.h \
		  $$TOOLS_H/qbuffer.h \
		  $$TOOLS_H/qcache.h \
		  $$TOOLS_H/qcollection.h \
		  $$TOOLS_H/qcstring.h \
		  $$TOOLS_H/qdatastream.h \
		  $$TOOLS_H/qdatetime.h \
		  $$TOOLS_H/qdict.h \
		  $$TOOLS_H/qdir.h \
		  $$TOOLS_H/qeucjpcodec.h \
		  $$TOOLS_H/qeuckrcodec.h \
		  $$TOOLS_H/qfile.h \
		  $$TOOLS_H/qfiledefs.h \
		  $$TOOLS_H/qfileinfo.h \
		  $$TOOLS_H/qgarray.h \
		  $$TOOLS_H/qgcache.h \
		  $$TOOLS_H/qgdict.h \
		  $$TOOLS_H/qgeneric.h \
		  $$TOOLS_H/qglist.h \
		  $$TOOLS_H/qglobal.h \
		  $$TOOLS_H/qgvector.h \
		  $$TOOLS_H/qintcache.h \
		  $$TOOLS_H/qintdict.h \
		  $$TOOLS_H/qiodevice.h \
		  $$TOOLS_H/qjiscodec.h \
		  $$TOOLS_H/qjpunicode.h \
		  $$TOOLS_H/qlist.h \
		  $$TOOLS_H/qmap.h \
		  $$TOOLS_H/qptrdict.h \
		  $$TOOLS_H/qqueue.h \
		  $$TOOLS_H/qregexp.h \
		  $$TOOLS_H/qrtlcodec.h \
		  $$TOOLS_H/qshared.h \
		  $$TOOLS_H/qsjiscodec.h \
		  $$TOOLS_H/qsortedlist.h \
		  $$TOOLS_H/qstack.h \
		  $$TOOLS_H/qstring.h \
		  $$TOOLS_H/qstringlist.h \
		  $$TOOLS_H/qstrlist.h \
		  $$TOOLS_H/qstrvec.h \
		  $$TOOLS_H/qtextcodec.h \
		  $$TOOLS_H/qtextstream.h \
		  $$TOOLS_H/qutfcodec.h \
		  $$TOOLS_H/qvector.h \
		  $$TOOLS_H/qvaluelist.h \
		  $$WIDGETS_H/qbuttongroup.h \
		  $$WIDGETS_H/qbutton.h \
		  $$WIDGETS_H/qcheckbox.h \
		  $$WIDGETS_H/qcdestyle.h \
		  $$WIDGETS_H/qcombobox.h \
		  $$WIDGETS_H/qcommonstyle.h \
		  $$WIDGETS_H/qframe.h \
		  $$WIDGETS_H/qgroupbox.h \
		  $$WIDGETS_H/qgrid.h \
		  $$WIDGETS_H/qhbuttongroup.h \
		  $$WIDGETS_H/qheader.h \
		  $$WIDGETS_H/qhgroupbox.h \
		  $$WIDGETS_H/qhbox.h \
		  $$WIDGETS_H/qlabel.h \
		  $$WIDGETS_H/qlcdnumber.h \
		  $$WIDGETS_H/qlineedit.h \
		  $$WIDGETS_H/qlistbox.h \
		  $$WIDGETS_H/qlistview.h \
		  $$WIDGETS_H/qmainwindow.h \
		  $$WIDGETS_H/qmenubar.h \
		  $$WIDGETS_H/qmenudata.h \
		  $$WIDGETS_H/qmotifstyle.h \
		  $$WIDGETS_H/qmultilineedit.h \
		  $$WIDGETS_H/qplatinumstyle.h \
		  $$WIDGETS_H/qpopupmenu.h \
		  $$WIDGETS_H/qprogressbar.h \
		  $$WIDGETS_H/qpushbutton.h \
		  $$WIDGETS_H/qradiobutton.h \
		  $$WIDGETS_H/qrangecontrol.h \
		  $$WIDGETS_H/qscrollbar.h \
		  $$WIDGETS_H/qscrollview.h \
		  $$WIDGETS_H/qslider.h \
		  $$WIDGETS_H/qspinbox.h \
		  $$WIDGETS_H/qsplitter.h \
		  $$WIDGETS_H/qstatusbar.h \
		  $$WIDGETS_H/qtabbar.h \
		  $$WIDGETS_H/qtabwidget.h \
		  $$WIDGETS_H/qtableview.h \
		  $$WIDGETS_H/qtextbrowser.h \
		  $$WIDGETS_H/qtextview.h \
		  $$WIDGETS_H/qtoolbar.h \
		  $$WIDGETS_H/qtoolbutton.h \
		  $$WIDGETS_H/qtooltip.h \
		  $$WIDGETS_H/qvalidator.h \
		  $$WIDGETS_H/qvbox.h \
		  $$WIDGETS_H/qvbuttongroup.h \
		  $$WIDGETS_H/qvgroupbox.h \
		  $$WIDGETS_H/qwellarray.h \
		  $$WIDGETS_H/qwhatsthis.h \
		  $$WIDGETS_H/qwidgetstack.h \
		  $$WIDGETS_H/qwindowsstyle.h

# DO NOT sort directories - dependency order = tools,kernel,widgets,dialogs

win32:SOURCES	= kernel/qapplication_win.cpp \
		  kernel/qclipboard_win.cpp \
		  kernel/qcolor_win.cpp \
		  kernel/qcursor_win.cpp \
		  kernel/qdnd_win.cpp \
		  kernel/qfont_win.cpp \
		  kernel/qmime_win.cpp \
		  kernel/qpixmap_win.cpp \
		  kernel/qprinter_win.cpp \
		  kernel/qpaintdevice_win.cpp \
		  kernel/qpainter_win.cpp \
		  kernel/qregion_win.cpp \
		  kernel/qwidget_win.cpp \
		  dialogs/qfiledialog_win.cpp

win32:SOURCES  += kernel/qole_win.c

unix:SOURCES    = kernel/qapplication_x11.cpp \
		  kernel/qclipboard_x11.cpp \
		  kernel/qcolor_x11.cpp \
		  kernel/qcursor_x11.cpp \
		  kernel/qdnd_x11.cpp \
		  kernel/qfont_x11.cpp \
		  kernel/qpixmap_x11.cpp \
		  kernel/qprinter_x11.cpp \
		  kernel/qpaintdevice_x11.cpp \
		  kernel/qpainter_x11.cpp \
		  kernel/qregion_x11.cpp \
		  kernel/qwidget_x11.cpp

unix:SOURCES   += dialogs/qprintdialog.cpp \
		  kernel/qpsprinter.cpp \
		  kernel/qnpsupport.cpp \
		  kernel/qwidgetcreate_x11.cpp

SOURCES	       += tools/qbitarray.cpp \
		  tools/qbuffer.cpp \
		  tools/qcollection.cpp \
		  tools/qcstring.cpp \
		  tools/qdatastream.cpp \
		  tools/qdatetime.cpp \
		  tools/qdir.cpp \
		  tools/qeucjpcodec.cpp \
		  tools/qeuckrcodec.cpp \
		  tools/qfile.cpp \
		  tools/qfileinfo.cpp \
		  tools/qgarray.cpp \
		  tools/qgcache.cpp \
		  tools/qgdict.cpp \
		  tools/qglist.cpp \
		  tools/qglobal.cpp \
		  tools/qgvector.cpp \
		  tools/qiodevice.cpp \
		  tools/qjiscodec.cpp \
		  tools/qjpunicode.cpp \
		  tools/qmap.cpp \
		  tools/qregexp.cpp \
		  tools/qrtlcodec.cpp \
		  tools/qsjiscodec.cpp \
		  tools/qstring.cpp \
		  tools/qstringlist.cpp \
		  tools/qtextcodec.cpp \
		  tools/qtextstream.cpp \
		  tools/qutfcodec.cpp \
		  kernel/qabstractlayout.cpp \
		  kernel/qaccel.cpp \
		  kernel/qapplication.cpp \
		  kernel/qasyncimageio.cpp \
		  kernel/qasyncio.cpp \
		  kernel/qbitmap.cpp \
		  kernel/qclipboard.cpp \
		  kernel/qcolor.cpp \
		  kernel/qconnection.cpp \
		  kernel/qcursor.cpp \
		  kernel/qdialog.cpp \
		  kernel/qdragobject.cpp \
		  kernel/qdrawutil.cpp \
		  kernel/qdropsite.cpp \
		  kernel/qevent.cpp \
		  kernel/qfocusdata.cpp \
		  kernel/qfont.cpp \
		  kernel/qfontdatabase.cpp \
		  kernel/qiconset.cpp \
		  kernel/qimage.cpp \
		  kernel/qlayout.cpp \
		  kernel/qlayoutengine.cpp \
		  kernel/qtranslator.cpp \
		  kernel/qmetaobject.cpp \
		  kernel/qmime.cpp \
		  kernel/qmovie.cpp \
		  kernel/qobject.cpp \
		  kernel/qpainter.cpp \
		  kernel/qpalette.cpp \
		  kernel/qpaintdevicemetrics.cpp \
		  kernel/qpicture.cpp \
		  kernel/qpixmap.cpp \
		  kernel/qpixmapcache.cpp \
		  kernel/qpngio.cpp \
		  kernel/qpointarray.cpp \
		  kernel/qpoint.cpp \
		  kernel/qprinter.cpp \
		  kernel/qrect.cpp \
		  kernel/qregion.cpp \
		  kernel/qrichtext.cpp \
		  kernel/qsemimodal.cpp \
		  kernel/qsignal.cpp \
		  kernel/qsignalmapper.cpp \
		  kernel/qsimplerichtext.cpp \
		  kernel/qsize.cpp \
		  kernel/qsizegrip.cpp \
		  kernel/qstyle.cpp \
		  kernel/qsocketnotifier.cpp \
		  kernel/qstylesheet.cpp \
		  kernel/qtimer.cpp \
		  kernel/qwidget.cpp \
		  kernel/qwmatrix.cpp \
		  widgets/qbuttongroup.cpp \
		  widgets/qbutton.cpp \
		  widgets/qcdestyle.cpp \
		  widgets/qcheckbox.cpp \
		  widgets/qcombobox.cpp \
		  widgets/qcommonstyle.cpp \
		  widgets/qframe.cpp \
		  widgets/qgroupbox.cpp \
		  widgets/qgrid.cpp \
		  widgets/qhbuttongroup.cpp \
		  widgets/qheader.cpp \
		  widgets/qhgroupbox.cpp \
		  widgets/qhbox.cpp \
		  widgets/qlabel.cpp \
		  widgets/qlcdnumber.cpp \
		  widgets/qlineedit.cpp \
		  widgets/qlistbox.cpp \
		  widgets/qlistview.cpp \
		  widgets/qmainwindow.cpp \
		  widgets/qmenubar.cpp \
		  widgets/qmenudata.cpp \
		  widgets/qmotifstyle.cpp \
		  widgets/qmultilineedit.cpp \
		  widgets/qplatinumstyle.cpp \
		  widgets/qpopupmenu.cpp \
		  widgets/qprogressbar.cpp \
		  widgets/qpushbutton.cpp \
		  widgets/qradiobutton.cpp \
		  widgets/qrangecontrol.cpp \
		  widgets/qscrollbar.cpp \
		  widgets/qscrollview.cpp \
		  widgets/qslider.cpp \
		  widgets/qspinbox.cpp \
		  widgets/qsplitter.cpp \
		  widgets/qstatusbar.cpp \
		  widgets/qtabbar.cpp \
		  widgets/qtabwidget.cpp \
		  widgets/qtableview.cpp \
		  widgets/qtextbrowser.cpp \
		  widgets/qtextview.cpp \
		  widgets/qtoolbar.cpp \
		  widgets/qtoolbutton.cpp \
		  widgets/qtooltip.cpp \
		  widgets/qvalidator.cpp \
		  widgets/qvbox.cpp \
		  widgets/qvbuttongroup.cpp \
		  widgets/qvgroupbox.cpp \
		  widgets/qwellarray.cpp \
		  widgets/qwhatsthis.cpp \
		  widgets/qwidgetstack.cpp \
		  widgets/qwindowsstyle.cpp \
		  dialogs/qcolordialog.cpp \
		  dialogs/qfiledialog.cpp \
		  dialogs/qfontdialog.cpp \
		  dialogs/qmessagebox.cpp \
		  dialogs/qprogressdialog.cpp \
		  dialogs/qtabdialog.cpp \
		  dialogs/qwizard.cpp

unix:HEADERS   += $$DIALOGS_H/qprintdialog.h \
		  $$KERNEL_H/qpsprinter.h \
		  $$KERNEL_H/qfontdatabase.h

PNG_SOURCES	= 3rdparty/libpng/png.c \
		  3rdparty/libpng/pngerror.c \
		  3rdparty/libpng/pngget.c \
		  3rdparty/libpng/pngmem.c \
		  3rdparty/libpng/pngpread.c \
		  3rdparty/libpng/pngread.c \
		  3rdparty/libpng/pngrio.c \
		  3rdparty/libpng/pngrtran.c \
		  3rdparty/libpng/pngrutil.c \
		  3rdparty/libpng/pngset.c \
		  3rdparty/libpng/pngtrans.c \
		  3rdparty/libpng/pngwio.c \
		  3rdparty/libpng/pngwrite.c \
		  3rdparty/libpng/pngwtran.c \
		  3rdparty/libpng/pngwutil.c

ZLIB_SOURCES	= 3rdparty/zlib/adler32.c \
		  3rdparty/zlib/compress.c \
		  3rdparty/zlib/crc32.c \
		  3rdparty/zlib/deflate.c \
		  3rdparty/zlib/gzio.c \
		  3rdparty/zlib/infblock.c \
		  3rdparty/zlib/infcodes.c \
		  3rdparty/zlib/inffast.c \
		  3rdparty/zlib/inflate.c \
		  3rdparty/zlib/inftrees.c \
		  3rdparty/zlib/infutil.c \
		  3rdparty/zlib/trees.c \
		  3rdparty/zlib/uncompr.c \
		  3rdparty/zlib/zutil.c

png:SOURCES    += $$PNG_SOURCES
zlib:SOURCES   += $$ZLIB_SOURCES

TARGET		= qt
VERSION		= 2.1.0
DESTDIR		= ../lib
DLLDESTDIR	= ../bin
