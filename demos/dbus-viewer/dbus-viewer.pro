TEMPLATE        = app
TARGET          = dbus-viewer

CONFIG          += qt warn_on

HEADERS         = qdbusviewer.h qdbusmodel.h
SOURCES         = qdbusviewer.cpp \
                  qdbusmodel.cpp \
                  main.cpp

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

CONFIG += qdbus
QT += xml

# install
target.path = $$[QT_INSTALL_DEMOS]/dbus-viewer
sources.files = $$SOURCES $$HEADERS *.pro *.html *.doc images
sources.path = $$[QT_INSTALL_DEMOS]/dbus-viewer
INSTALLS += target sources

