TEMPLATE    	= lib
CONFIG      	+= qt x11 release staticlib
unix:HEADERS	= qnp.h
win32:HEADERS	= ../../../include/qnp.h
win32:LIBS	+= $(QTDIR)\lib\qtmain.lib
SOURCES		= qnp.cpp
MOC_DIR		= .
TARGET		= qnp
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.3
