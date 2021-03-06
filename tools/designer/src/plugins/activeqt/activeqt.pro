TARGET      = $$qtLibraryTarget(qaxwidget)
TEMPLATE    = lib
QTDIR_build:DESTDIR 	= $$QT_BUILD_TREE/plugins/designer

CONFIG     += qt warn_on qaxcontainer plugin designer debug_and_release
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

INCLUDEPATH += $$QT_SOURCE_TREE/src/activeqt/shared/ \
               $$QT_BUILD_TREE/src/activeqt/container \
               ../../lib/uilib

# Input
SOURCES += plugin.cpp \
    activeqt_extrainfo.cpp \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.cpp

HEADERS += activeqt_extrainfo.h \
    $$QT_SOURCE_TREE/src/activeqt/shared/qaxtypes.h

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
