TEMPLATE = subdirs
CONFIG += ordered

REQUIRES = !CONFIG(static,shared|static)
contains(QT_CONFIG, qt3support): SUBDIRS += widgets
win32:SUBDIRS += activeqt
# contains(QT_CONFIG, opengl): SUBDIRS += tools/view3d

