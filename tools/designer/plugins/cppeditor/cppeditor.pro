TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin
HEADERS		= cppeditor.h  \
		  syntaxhighliter_cpp.h \
		  indent_cpp.h \
		  cppcompletion.h \
		  editorinterfaceimpl.h \
		  languageinterfaceimpl.h

SOURCES		= cppeditor.cpp \
		  syntaxhighliter_cpp.cpp \
		  indent_cpp.cpp \
		  cppcompletion.cpp \
		  editorinterfaceimpl.cpp \
		  languageinterfaceimpl.cpp \
		  common.cpp
		
TARGET		= cppeditor
DESTDIR		= $(QTDIR)/plugins
VERSION		= 1.0.0
unix:LIBS	+= -leditor
win32:LIBS	+= $(QTDIR)/lib/editor.lib
INCLUDEPATH	+= $(QTDIR)/src/kernel $(QTDIR)/tools/designer/interfaces $(QTDIR)/tools/designer/editor
