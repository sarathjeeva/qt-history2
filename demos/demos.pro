TEMPLATE    = subdirs
SUBDIRS     = \
	shared \
	deform \
	gradients \
	pathstroke \
	affine \
	composition \
        books \
        interview \
        mainwindow \
        spreadsheet \
        textedit 

!contains(QT_EDITION, Console):!cross_compile:SUBDIRS += arthurplugin

CONFIG += ordered

!cross_compile:SUBDIRS += sqlbrowser

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_DEMOS]
INSTALLS += sources
