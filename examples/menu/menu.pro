TEMPLATE	= app
TARGET		= menu

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= menu.h
SOURCES		= menu.cpp
