TEMPLATE	= lib
OBJECTS_DIR	= .
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
SOURCES		= qwidgetfactory.cpp \
		  ../shared/widgetdatabase.cpp \
		  ../shared/domtool.cpp \
		  ../integration/kdevelop/kdewidgets.cpp \
		  ../designer/config.cpp \
		  ../designer/database.cpp \
		  ../designer/pixmapchooser.cpp
HEADERS		= qwidgetfactory.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h \
		  ../designer/config.h \
		  ../designer/database.h \
		  ../designer/pixmapchooser.h

TARGET		= qresource
INCLUDEPATH	= ../shared ../util ../../../src/3rdparty/zlib/
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
