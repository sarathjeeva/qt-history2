TEMPLATE = app
CONFIG += console
CONFIG -= resource_fork
build_all:CONFIG += release

QT += xml compat

DESTDIR = ../../../bin

include(../uic/uic.pri)

INCLUDEPATH += .

HEADERS += ui3reader.h \
           parser.h \
           domtool.h \
           widgetinfo.h

SOURCES += main.cpp \
           ui3reader.cpp \
           parser.cpp \
           domtool.cpp \
           object.cpp \
           subclassing.cpp \
           form.cpp \
           converter.cpp \
           widgetinfo.cpp \
           embed.cpp

DEFINES -= QT_COMPAT_WARNINGS
DEFINES += QT_COMPAT

target.path=$$bins.path
INSTALLS += target
