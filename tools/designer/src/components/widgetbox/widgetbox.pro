TEMPLATE = lib
DESTDIR=../../../lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../sdk \
    ../../extension \
    ../../uilib \
    ../../shared \
    ../formeditor

LIBS += \
    -L../../../lib \
    -lQtDesigner \
    -lshared \
    -lformeditor \
    -luilib \
    -lpropertyeditor \

DEFINES += QT_WIDGETBOX_LIBRARY

SOURCES += widgetbox.cpp
HEADERS += widgetbox.h \
        widgetbox_global.h

RESOURCES += widgetbox.qrc

# DEFINES -= QT_COMPAT_WARNINGS
# DEFINES += QT_COMPAT
