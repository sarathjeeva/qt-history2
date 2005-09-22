TARGET     = QtOpenGL
QPRO_PWD   = $$PWD
QT         = core gui
DEFINES   += QT_BUILD_OPENGL_LIB
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x63000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2


include(../qbase.pri)

!win32:!embedded:!mac:CONFIG	   += x11
!x11:contains(QT_CONFIG, opengl):CONFIG += opengl


HEADERS += qgl.h \
	   qglcolormap.h \
	   qpaintengine_opengl_p.h \
	   qglpbuffer.h
SOURCES	+= qgl.cpp \
	   qglcolormap.cpp \
	   qpaintengine_opengl.cpp \
	   qglpbuffer.cpp
x11 {
    HEADERS += qglsymbols_x11_p.h
    SOURCES += qgl_x11.cpp \
	       qglpbuffer_x11.cpp \
	       qglsymbols_x11.cpp
    contains(QT_CONFIG, fontconfig):INCLUDEPATH += $$FREETYPE2_INCDIR
}

mac {
    SOURCES += qgl_mac.cpp \
	       qglpbuffer_mac.cpp
    LIBS += -framework Carbon
}
win32 {
    SOURCES += qgl_win.cpp \
	       qglpbuffer_win.cpp
}

!x11:QMAKE_LIBS += $$QMAKE_LIBS_OPENGL
