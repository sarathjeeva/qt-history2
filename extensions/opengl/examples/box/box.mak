#############################################################################
# Makefile for building box
# Generated by tmake at 13:28, 1997/09/29
#     Project: box
#    Template: app
#############################################################################

####### Compiler, tools and options

CC	=	cl
CFLAGS	=	-nologo -O1
INCPATH	=	-I"$(QTDIR)\include"
LINK	=	link
LFLAGS	=	/NOLOGO /SUBSYSTEM:windows
LIBS	=	$(QTDIR)\lib\qgl.lib $(QTDIR)\lib\qt.lib opengl32.lib user32.lib gdi32.lib comdlg32.lib wsock32.lib
MOC	=	moc

####### Files

HEADERS =	glbox.h \
		globjwin.h
SOURCES =	glbox.cpp \
		globjwin.cpp \
		main.cpp
OBJECTS =	glbox.obj \
		globjwin.obj \
		main.obj
SRCMOC	=	moc_glbox.cpp \
		moc_globjwin.cpp
OBJMOC	=	moc_glbox.obj \
		moc_globjwin.obj
TARGET	=	box.exe

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .c

.cpp.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.cxx.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.cc.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.c.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

####### Build rules

all: $(TARGET) 

$(TARGET): $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) /OUT:$(TARGET) @<<
	    $(OBJECTS) $(OBJMOC) $(LIBS)
<<

moc: $(SRCMOC)

tmake: box.mak

box.mak: box.pro
	tmake box.pro -nodepend -o box.mak

clean:
	-del glbox.obj
	-del globjwin.obj
	-del main.obj
	-del moc_glbox.cpp
	-del moc_globjwin.cpp
	-del moc_glbox.obj
	-del moc_globjwin.obj
	-del $(TARGET)

####### Compile

glbox.obj: glbox.cpp

globjwin.obj: globjwin.cpp

main.obj: main.cpp

moc_glbox.obj: moc_glbox.cpp \
		glbox.h

moc_globjwin.obj: moc_globjwin.cpp \
		globjwin.h

moc_glbox.cpp: glbox.h
	$(MOC) glbox.h -o moc_glbox.cpp

moc_globjwin.cpp: globjwin.h
	$(MOC) globjwin.h -o moc_globjwin.cpp

