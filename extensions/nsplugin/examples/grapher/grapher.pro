TEMPLATE	=	app
CONFIG		=	qt dll release
HEADERS		=   
SOURCES		=	grapher.cpp
unix:LIBS      /=	s/-lqt/-lqnp -lqt/
win32:LIBS	=	$(QTDIR)/lib/qnp.lib
DEF_FILE	=	grapher.def
RC_FILE		=	grapher.rc
TARGET		=	grapher
win32:TARGET	=	npgrapher
