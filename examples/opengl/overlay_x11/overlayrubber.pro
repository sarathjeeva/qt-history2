TEMPLATE	= app
TARGET		= overlayrubber

CONFIG		+= qt warn_on release
QT         += opengl
DEPENDPATH	= ../include

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, opengl)" "contains(QT_CONFIG, full-config)"

HEADERS		= gearwidget.h \
		  rubberbandwidget.h
SOURCES		= gearwidget.cpp \
		  main.cpp \
		  rubberbandwidget.cpp
