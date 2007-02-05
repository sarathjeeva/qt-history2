
INCLUDEPATH += $$PWD

DEFINES += \
    QT_DESIGNER

# Input
HEADERS += \
    $$PWD/ui4_p.h \
    $$PWD/abstractformbuilder.h \
    $$PWD/formbuilder.h \
    $$PWD/container.h \
    $$PWD/customwidget.h \
    $$PWD/properties_p.h

SOURCES += \
    $$PWD/abstractformbuilder.cpp \
    $$PWD/formbuilder.cpp \
    $$PWD/ui4.cpp \
    $$PWD/properties.cpp
