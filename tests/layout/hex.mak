#############################################################################
# Makefile for building hex
# Generated by tmake at 03:04, 1997/07/04
#     Project: hex.pro
#    Template: e:\tmake\lib\win32-msvc\app.t
#############################################################################

####### Compiler, tools and options

CC	=	cl
CFLAGS	=	-nologo -W3 -O1
INCPATH	=	-I$(QTDIR)\include
LINK	=	link
LFLAGS	=	/NOLOGO /SUBSYSTEM:windows
LIBS	=	$(QTDIR)\lib\qt.lib user32.lib gdi32.lib comdlg32.lib wsock32.lib
MOC	=	moc

####### Files

HEADERS =	
SOURCES =	hex.cpp
OBJECTS =	hex.obj
SRCMOC	=	
OBJMOC	=	
TARGET	=	hex.exe

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.obj:
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

clean:
	-del hex.obj
	-del $(TARGET)

####### Compile

hex.obj: hex.cpp

