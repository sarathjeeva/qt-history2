# Additional Qt project file for qtmain lib on Windows
TEMPLATE = lib
TARGET	 = qtmain
DESTDIR	 = $$QMAKE_LIBDIR_QT

CONFIG	+= staticlib warn_on
CONFIG	-= qt

win32 {
	SOURCES		= qtmain_win.cpp
	CONFIG		+= png zlib
	CONFIG		-= jpeg
	INCLUDEPATH	+= tmp $$QMAKE_INCDIR_QT/QtCore
}

!win32:error("$$_FILE_ is intended only for Windows!")
