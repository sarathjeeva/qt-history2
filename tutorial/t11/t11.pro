TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t11
QTDIR_build:REQUIRES="contains(QT_CONFIG, full-config)"
unix:LIBS += -lm

