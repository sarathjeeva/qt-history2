#############################################################################
#!
#! This is the tmake template for building Qt/Win32 applications
#!
#$ $moc_aware = 1;
#$ StdInit();
#$ $project{"TARGET"} || ($project{"TARGET"} = "aout");
#!
# Makefile for building #$ Expand("TARGET")
# Generated by tmake at #$ Now();
#     Project: #$ $text = $project_name;
#    Template: #$ $text = $template_name;
#############################################################################

####### Compiler, tools and options

CC	=	cl
CFLAGS	=	-O1 -nologo
INCPATH	=	#$ ExpandGlue("INCPATH","-I"," -I","");
LINK	=	link
LFLAGS	=	/SUBSYSTEM:windows /NOLOGO
LIBS	=	wsock32.lib user32.lib gdi32.lib comdlg32.lib \
		$(QTDIR)\lib\qt.lib #$ Expand("WIN32LIBS");
MOC	=	moc

####### Files

HEADERS =	#$ ExpandList("HEADERS");
SOURCES =	#$ ExpandList("SOURCES");
OBJECTS =	#$ ExpandList("OBJECTS");
SRCMOC	=	#$ ExpandList("SRCMOC");
OBJMOC	=	#$ ExpandList("OBJMOC");
TARGET	=	#$ ExpandGlue("TARGET","","",".exe");

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

.c.obj:
	$(CC) -c $(CFLAGS) $(INCPATH) -Fo$@ $<

####### Make targets

all: $(TARGET) #$ Expand("ALL_DEPS");

$(TARGET): $(OBJECTS) $(OBJMOC)
	$(LINK) $(LFLAGS) /OUT:$(TARGET) @<<
	    $(OBJECTS) $(OBJMOC) $(LIBS)
<<

moc: $(SRCMOC)

clean:
	#$ ExpandGlue("OBJECTS","-del ","\n\t-del ","");
	#$ ExpandGlue("SRCMOC" ,"-del ","\n\t-del ","");
	#$ ExpandGlue("OBJMOC" ,"-del ","\n\t-del ","");
	#$ ExpandGlue("TARGET" ,"-del ","",".exe");
	#$ ExpandGlue("CLEAN_FILES","-del ","\n\t-del ","");

####### Compile

#$ BuildObj($project{"OBJECTS"},$project{"SOURCES"});
#$ BuildMocObj($project{"OBJMOC"},$project{"SRCMOC"});
#$ BuildMocSrc($project{"HEADERS"});
#$ BuildMocSrc($project{"SOURCES"});
