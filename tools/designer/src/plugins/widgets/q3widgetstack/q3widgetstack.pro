######################################################################
# Automatically generated by qmake (1.08a) Thu Nov 11 11:56:54 2004
######################################################################

TEMPLATE = lib 

QT += compat

IDEDIR = ../../../..
DESTDIR = ../../../../../../plugins/designer

INCLUDEPATH += \
    ../../../lib/sdk \
    ../../../lib/extension \
    $$IDEDIR/src/uilib
LIBS += -lQtDesigner

# Input
SOURCES += plugin.cpp
CONFIG += qt warn_on qt_no_compat_warning
OBJECTS_DIR=.obj/debug-shared
MOC_DIR=.moc/debug-shared

include(../../../sharedcomponents.pri)
