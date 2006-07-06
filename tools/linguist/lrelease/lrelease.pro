TEMPLATE        = app
CONFIG          += qt warn_on console
CONFIG          -= app_bundle

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
HEADERS         = ../shared/metatranslator.h \
                  ../shared/translator.h \
                  ../shared/findsourcesvisitor.h \
                  ../shared/proparser.h
SOURCES         = main.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/translator.cpp \
                  ../shared/findsourcesvisitor.cpp \
                  ../shared/proparser.cpp

QT += xml
include( ../../../src/qt_professional.pri )

PROPARSERPATH = ../shared
INCLUDEPATH += $$PROPARSERPATH
# Input
HEADERS += $$PROPARSERPATH/proitems.h \
        $$PROPARSERPATH/proreader.h
SOURCES += $$PROPARSERPATH/proitems.cpp \
        $$PROPARSERPATH/proreader.cpp

TARGET          = lrelease
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
