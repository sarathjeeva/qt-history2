TEMPLATE        = app
CONFIG          += qt warn_on console
CONFIG          -= resource_fork
HEADERS         = ../shared/metatranslator.h \
                  ../shared/proparser.h
SOURCES         = fetchtr.cpp \
                  main.cpp \
                  merge.cpp \
                  numberh.cpp \
                  sametexth.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/proparser.cpp

QT += xml
include( ../../../src/qt_professional.pri )

TARGET          = lupdate
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$bins.path
INSTALLS        += target
