/****************************************************************************
** $Id$
**
** Implementation of MingwMakefileGenerator class.
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of qmake.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "mingw_make.h"
#include "option.h"
#include <qregexp.h>
#include <qdir.h>
#include <stdlib.h>
#include <time.h>


MingwMakefileGenerator::MingwMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{
    Option::obj_ext = ".o";
}

bool
MingwMakefileGenerator::findLibraries() // todo - pascal
{
    return TRUE;
}

bool
MingwMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	t << "all clean:" << "\n\t"
	  << "@echo \"Some of the required modules ("
	  << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
	  << "@echo \"Skipped.\"" << endl << endl;
	writeMakeQmake(t);
	return TRUE;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	writeMingwParts(t);
	return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeSubDirs(t);
	return TRUE;
    }
    return FALSE;
}

void
MingwMakefileGenerator::writeMingwParts(QTextStream &t)
{
    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC		=	" << var("QMAKE_CC") << endl;
    t << "CXX		=	" << var("QMAKE_CXX") << endl;
    t << "LEX		= " << var("QMAKE_LEX") << endl;
    t << "YACC		= " << var("QMAKE_YACC") << endl;
    t << "CFLAGS	=	" << var("QMAKE_CFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      <<  varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS	=	" << var("QMAKE_CXXFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS	=" << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS	=" << var("QMAKE_YACCFLAGS") << endl;

    t << "INCPATH	=	";
    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
	QString inc = (*incit);
	inc.replace(QRegExp("\\\\$"), "\\\\");
	inc.replace(QRegExp("\""), "");
	t << " -I" << "\"" << inc << "\"";
    }
    t << " -I" << "\"" << specdir()  << "\"" << endl;
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "LINK	=	" << var("QMAKE_LINK") << endl;
	t << "LFLAGS	=	" << var("QMAKE_LFLAGS") << endl;
	t << "LIBS	=	";
	if ( !project->variables()["QMAKE_LIBDIR"].isEmpty() )
	    t << varGlue("QMAKE_LIBDIR","-L\"","\" -L\"","\"") << " ";
	t << var("QMAKE_LIBS").replace(QRegExp("(\\slib|^lib)")," -l") << endl;
    }
    else {
	t << "LIB	=	" << var("QMAKE_LIB") << endl;
    }
    t << "MOC		=	" << (project->isEmpty("QMAKE_MOC") ? QString("moc") :
			      Option::fixPathToTargetOS(var("QMAKE_MOC"), FALSE)) << endl;
    t << "UIC		=	" << (project->isEmpty("QMAKE_UIC") ? QString("uic") :
			      Option::fixPathToTargetOS(var("QMAKE_UIC"), FALSE)) << endl;
    t << "QMAKE		=	" << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") :
			      Option::fixPathToTargetOS(var("QMAKE_QMAKE"), FALSE)) << endl;
    t << "IDC		=	" << (project->isEmpty("QMAKE_IDC") ? QString("idc") :
			      Option::fixPathToTargetOS(var("QMAKE_IDC"), FALSE)) << endl;
    t << "IDL		=	" << (project->isEmpty("QMAKE_IDL") ? QString("midl") :
			      Option::fixPathToTargetOS(var("QMAKE_IDL"), FALSE)) << endl;
    t << "ZIP		=	" << var("QMAKE_ZIP") << endl;
    t << "DEF_FILE      =	" << varList("DEF_FILE") << endl;
    t << "COPY_FILE	=       " << var("QMAKE_COPY") << endl;
    t << "COPY_DIR	=       " << var("QMAKE_COPY") << endl;
    t << "DEL_FILE	=       " << var("QMAKE_DEL_FILE") << endl;
    t << "DEL_DIR	=       " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE		=       " << var("QMAKE_MOVE") << endl;
    t << "CHK_DIR_EXISTS =	" << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR		=	" << var("QMAKE_MKDIR") << endl;
    t << endl;

    t << "####### Output directory" << endl << endl;
    if (! project->variables()["OBJECTS_DIR"].isEmpty())
	t << "OBJECTS_DIR = " << var("OBJECTS_DIR").replace(QRegExp("\\\\$"),"") << endl;
    else
	t << "OBJECTS_DIR = . " << endl;
    if (! project->variables()["MOC_DIR"].isEmpty())
	t << "MOC_DIR = " << var("MOC_DIR").replace(QRegExp("\\\\$"),"") << endl;
    else
	t << "MOC_DIR = . " << endl;
    t << endl;

    t << "####### Files" << endl << endl;
    t << "HEADERS =	" << varList("HEADERS") << endl;
    t << "SOURCES =	" << varList("SOURCES") << endl;
//    t << "OBJECTS =	" << varList("OBJECTS").replace(QRegExp("\\.obj"),".o") << endl;
    t << "OBJECTS =	" << varList("OBJECTS") << endl;
    t << "FORMS =	" << varList("FORMS") << endl;
    t << "UICDECLS =	" << varList("UICDECLS") << endl;
    t << "UICIMPLS =	" << varList("UICIMPLS") << endl;
    t << "SRCMOC	=	" << varList("SRCMOC") << endl;
    t << "OBJMOC	=	" << varList("OBJMOC") << endl;
//    t << "OBJMOC	=	" << varList("OBJMOC").replace(QRegExp("\\.obj"),".o") << endl;
    QString extraCompilerDeps;
    if(!project->isEmpty("QMAKE_EXTRA_WIN_COMPILERS")) {
	t << "OBJCOMP = " << varList("OBJCOMP") << endl;
	extraCompilerDeps += " $(OBJCOMP) ";
	
	QStringList &comps = project->variables()["QMAKE_EXTRA_WIN_COMPILERS"];
	for(QStringList::Iterator compit = comps.begin(); compit != comps.end(); ++compit) {
	    QStringList &vars = project->variables()[(*compit) + ".variables"];
	    for(QStringList::Iterator varit = vars.begin(); varit != vars.end(); ++varit) {
		QStringList vals = project->variables()[(*varit)];
		if(!vals.isEmpty())
		    t << "QMAKE_COMP_" << (*varit) << " = " << valList(vals) << endl;
	    }
	}
    }

    t << "DIST	=	" << varList("DISTFILES") << endl;
    t << "TARGET	=	";
    if( !project->variables()[ "DESTDIR" ].isEmpty() )
	t << varGlue("TARGET",project->first("DESTDIR"),"",project->first("TARGET_EXT"));
    else
	t << project->variables()[ "TARGET" ].value(0) << project->variables()[ "TARGET_EXT" ].value(0);
    t << endl;
    t << endl;

    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .cpp .cxx .cc .C .c" << endl << endl;
    t << ".cpp.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cxx.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".cc.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".C.o:\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
    t << ".c.o:\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;

    t << "####### Build rules" << endl << endl;
    t << "all: " << "$(OBJECTS_DIR) " << "$(MOC_DIR) " << varGlue("ALL_DEPS",""," "," ") << "$(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(UICDECLS) $(OBJECTS) $(OBJMOC) "
      << extraCompilerDeps << var("POST_TARGETDEPS");
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "\n\t" << "$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)";
    } else {
	t << "\n\t" << "$(LIB) $(TARGET) $(OBJECTS) $(OBJMOC)";
    }

    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
	QStringList dlldirs = project->variables()["DLLDESTDIR"];
	for ( QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir ) {
	    t << "\n\t" << "$(COPY_FILE) $(TARGET) " << *dlldir;
	}
    }
    QString targetfilename = project->variables()["TARGET"].value(0);
    if(project->isActiveConfig("activeqt")) {
	QString version = project->variables()["VERSION"].value(0);
	if ( version.isEmpty() )
	    version = "1.0";

	if ( project->isActiveConfig("dll")) {
	    t << "\n\t" << ("-$(IDC) $(TARGET) /idl " + var("OBJECTS_DIR") + targetfilename + ".idl -version " + version);
	    t << "\n\t" << ("-$(IDL) /nologo " + var("OBJECTS_DIR") + targetfilename + ".idl /tlb " + var("OBJECTS_DIR") + targetfilename + ".tlb");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /tlb " + var("OBJECTS_DIR") + targetfilename + ".tlb");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /regserver" );
	} else {
	    t << "\n\t" << ("-$(TARGET) -dumpidl " + var("OBJECTS_DIR") + targetfilename + ".idl -version " + version);
	    t << "\n\t" << ("-$(IDL) /nologo " + var("OBJECTS_DIR") + targetfilename + ".idl /tlb " + var("OBJECTS_DIR") + targetfilename + ".tlb");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /tlb " + var("OBJECTS_DIR") + targetfilename + ".tlb");
	    t << "\n\t" << "-$(TARGET) -regserver";
	}
    }
    t << endl << endl;

    if(!project->variables()["RC_FILE"].isEmpty()) {
	t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
	  << var("QMAKE_RC") << " -i " << var("RC_FILE") << " -o " << var("RC_FILE").replace(QRegExp("\\.rc"),".o") << " --include-dir=" << QFileInfo(var("RC_FILE")).dirPath() << endl << endl;
    }
	project->variables()["RES_FILE"].value(0).replace(QRegExp("\\.rc"),".o");

    t << "mocables: $(SRCMOC)" << endl << endl;

    t << "$(OBJECTS_DIR):" << "\n\t"
      << "@if not exist $(OBJECTS_DIR) $(MKDIR) $(OBJECTS_DIR)" << endl << endl;

    t << "$(MOC_DIR):" << "\n\t"
      << "@if not exist $(MOC_DIR) $(MKDIR) $(MOC_DIR)" << endl << endl;

    writeMakeQmake(t);

    t << "dist:" << "\n\t"
      << "$(ZIP) " << var("PROJECT") << ".zip "
      << var("PROJECT") << ".pro $(SOURCES) $(HEADERS) $(DIST) $(FORMS)" << endl << endl;

    t << "clean:"
      << varGlue("OBJECTS","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","").replace(QRegExp("\\.obj"),".o")
      << varGlue("SRCMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("OBJMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","").replace(QRegExp("\\.obj"),".o")
      << varGlue("UICDECLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("UICIMPLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << "\n\t-$(DEL_FILE) $(TARGET)"
      << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("CLEAN_FILES","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","");
    if ( project->isActiveConfig("activeqt")) {
	t << ("\n\t-$(DEL_FILE) " + var("OBJECTS_DIR") + targetfilename + ".idl");
	t << ("\n\t-$(DEL_FILE) " + var("OBJECTS_DIR") + targetfilename + ".tlb");
    }
    if(!project->isEmpty("IMAGES"))
	t << varGlue("QMAKE_IMAGE_COLLECTION", "\n\t-$(DEL_FILE) ", "\n\t-$(DEL_FILE) ", "");

    // user defined targets
    QStringList::Iterator it;
    QStringList &qut = project->variables()["QMAKE_EXTRA_WIN_TARGETS"];

    for(it = qut.begin(); it != qut.end(); ++it) {
	QString targ = var((*it) + ".target"),
		 cmd = var((*it) + ".commands"), deps;
	if(targ.isEmpty())
	    targ = (*it);
	QStringList &deplist = project->variables()[(*it) + ".depends"];
	for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
	    QString dep = var((*dep_it) + ".target");
	    if(dep.isEmpty())
		dep = (*dep_it);
	    deps += " " + dep;
	}
	t << "\n\n" << targ << ":" << deps << "\n\t"
	  << cmd;
    }

    t << endl << endl;
   
    QStringList &quc = project->variables()["QMAKE_EXTRA_WIN_COMPILERS"];
    for(it = quc.begin(); it != quc.end(); ++it) {
	QString tmp_out = project->variables()[(*it) + ".output"].first();
	QString tmp_cmd = project->variables()[(*it) + ".commands"].join(" ");
	QString tmp_dep = project->variables()[(*it) + ".depends"].join(" ");
	QStringList &vars = project->variables()[(*it) + ".variables"];
	if(tmp_out.isEmpty() || tmp_cmd.isEmpty())
	    continue;
	QStringList &tmp = project->variables()[(*it) + ".input"];
	for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
	    QStringList &inputs = project->variables()[(*it2)];
	    for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
		QFileInfo fi(Option::fixPathToLocalOS((*input)));
		QString in = Option::fixPathToTargetOS((*input), FALSE),
		       out = tmp_out, cmd = tmp_cmd, deps;
		out.replace("${QMAKE_FILE_BASE}", fi.baseName());
		out.replace("${QMAKE_FILE_NAME}", fi.fileName());
		cmd.replace("${QMAKE_FILE_BASE}", fi.baseName());
		cmd.replace("${QMAKE_FILE_OUT}", out);
		cmd.replace("${QMAKE_FILE_NAME}", fi.fileName());
		for(QStringList::Iterator it3 = vars.begin(); it3 != vars.end(); ++it3)
		    cmd.replace("$(" + (*it3) + ")", "$(QMAKE_COMP_" + (*it3)+")");
		if(!tmp_dep.isEmpty()) {
		    char buff[256];
		    QString dep_cmd = tmp_dep;
		    dep_cmd.replace("${QMAKE_FILE_NAME}", fi.fileName());
		    if(FILE *proc = QT_POPEN(dep_cmd.latin1(), "r")) {
			while(!feof(proc)) {
			    int read_in = fread(buff, 1, 255, proc);
			    if(!read_in)
				break;
			    int l = 0;
			    for(int i = 0; i < read_in; i++) {
				if(buff[i] == '\n' || buff[i] == ' ') {
				    deps += " " + QString::fromLatin1(buff+l, (i - l) + 1);
				    l = i;
				}
			    }
			}
			fclose(proc);
		    }
		}
		t << out << ": " << in << deps << "\n\t"
		  << cmd << endl << endl;
	    }
	}
    }
    t << endl;
}


void
MingwMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["QMAKE"].isEmpty())
	    project->variables()["QMAKE"].append("qmake");
	return;
    }

    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    // LIBS defined in Profile comes first for gcc
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];
    
    QString targetfilename = project->variables()["TARGET"].first();
    QStringList &configs = project->variables()["CONFIG"];
    
    if (project->isActiveConfig("qt") && project->isActiveConfig("shared"))
	project->variables()["DEFINES"].append("QT_DLL");
    
    if (project->isActiveConfig("qt_dll"))
	if (configs.indexOf("qt") == -1)
	    configs.append("qt");
    
    if ( project->isActiveConfig("qt") ) {
	if ( project->isActiveConfig( "plugin" ) ) {
	    project->variables()["CONFIG"].append("dll");
	    if(project->isActiveConfig("qt"))
		project->variables()["DEFINES"].append("QT_PLUGIN");
	}
	if ( (project->variables()["DEFINES"].indexOf("QT_NODLL") == -1) &&
         ((project->variables()["DEFINES"].indexOf("QT_MAKEDLL") != -1 ||
           project->variables()["DEFINES"].indexOf("QT_DLL") != -1) ||
          (getenv("QT_DLL") && !getenv("QT_NODLL"))) ) {
	    project->variables()["QMAKE_QT_DLL"].append("1");
	    if ( project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() )
		project->variables()["CONFIG"].append("dll");
	}
	if ( project->isActiveConfig("thread") )
	    project->variables()[project->isActiveConfig("target_qt") ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_THREAD_SUPPORT");
	if ( project->isActiveConfig("accessibility" ) )
	    project->variables()[project->isActiveConfig("target_qt") ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_ACCESSIBILITY_SUPPORT");
	if ( project->isActiveConfig("tablet") )
	    project->variables()[project->isActiveConfig("target_qt") ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_TABLET_SUPPORT");
    }
    
    if ( project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["QMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }
    
    if ( project->isActiveConfig("warn_off") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_OFF"];
    } else if ( project->isActiveConfig("warn_on") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_ON"];
    }
    
    if ( project->isActiveConfig("debug") ) {
        if ( project->isActiveConfig("thread") ) {
	    // use the DLL RT even here
	    if ( project->variables()["DEFINES"].contains("QT_DLL") ) {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DLLDBG"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DLLDBG"];
	    } else {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DBG"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DBG"];
	    }
	}
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_DEBUG"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_DEBUG"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_DEBUG"];
    } else {
	if ( project->isActiveConfig("thread") ) {
	    if ( project->variables()["DEFINES"].contains("QT_DLL") ) {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DLL"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DLL"];
	    } else {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT"];
	    }
	}
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RELEASE"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RELEASE"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_RELEASE"];
    }
    
    if ( !project->variables()["QMAKE_INCDIR"].isEmpty())
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    
    if ( project->isActiveConfig("qt") || project->isActiveConfig("opengl") )
	project->variables()["CONFIG"].append("windows");
    
    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["INCLUDEPATH"] +=	project->variables()["QMAKE_INCDIR_QT"];
	project->variables()["QMAKE_LIBDIR"] += project->variables()["QMAKE_LIBDIR_QT"];
	if ( !project->isActiveConfig("debug") )
	    project->variables()[project->isActiveConfig("target_qt") ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_NO_DEBUG");
	if ( project->isActiveConfig("target_qt") && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty()) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_QT_DLL"];
	    }
	} else {

	    if(project->isActiveConfig("thread"))
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    else
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("libqt" QTDLL_POSTFIX "%d.a", hver);
		    QStringList &libs = project->variables()["QMAKE_LIBS"];
// @@@HGTODO maybe we must change the replace regexp if we understand what's going on
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt\\.lib"), ver);
		}
	    }
	    if ( project->isActiveConfig( "activeqt" ) ) {
		project->variables().remove("QMAKE_LIBS_QT_ENTRY");
		project->variables()["QMAKE_LIBS_QT_ENTRY"] = "-lqaxserver";
		if ( project->isActiveConfig( "dll" ) ) {
		   project->variables()["QMAKE_LIBS"]  += project->variables()["QMAKE_LIBS_QT_ENTRY"];
		}
	    }
	    if ( !project->isActiveConfig("dll") && !project->isActiveConfig("plugin") ) {
		project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
	    }

            // QMAKE_LIBS_QT_ENTRY should be first on the link line as it needs qt
            project->variables()["QMAKE_LIBS"].remove(project->variables()["QMAKE_LIBS_QT_ENTRY"].first());
	    project->variables()["QMAKE_LIBS"].prepend(project->variables()["QMAKE_LIBS_QT_ENTRY"].first());
	    
	    if (project->isActiveConfig( "activeqt" ) && project->variables()["QMAKE_LIBS"].contains("-lqaxserver")) { // ordering
		project->variables()["QMAKE_LIBS"].remove("-lqaxserver");
		project->variables()["QMAKE_LIBS"].prepend("-lqaxserver");
	    }
	}
    }
    
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_OPENGL"];
    }
    
    if ( project->isActiveConfig("dll") ) {
	project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CXXFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_LFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["QMAKE_LFLAGS_WINDOWS_DLL"];
	if ( !project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(
		QStringList::split('.',project->first("VERSION")).join("") + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CFLAGS_CONSOLE"];
	project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CXXFLAGS_CONSOLE"];
	project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_LFLAGS_CONSOLE"];
	project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["QMAKE_LFLAGS_WINDOWS"];
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".a");
	    project->variables()["QMAKE_LFLAGS"].append("-static");
	    if(project->variables()["TARGET"].value(0).left(3) != "lib")
		project->variables()["TARGET"].value(0).prepend("lib");
	}
    }
    
    if ( project->isActiveConfig("windows") ) {
	if ( project->isActiveConfig("console") ) {
	    project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_CONSOLE"];
	} else {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"];
	}
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_WINDOWS"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_CONSOLE"];
    }
    
    if ( project->isActiveConfig("exceptions") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_EXCEPTIONS_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_EXCEPTIONS_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_EXCEPTIONS_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_EXCEPTIONS_OFF"];
    }
    
    if ( project->isActiveConfig("rtti") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_OFF"];
    }

    if ( project->isActiveConfig("moc") )
	setMocAware(TRUE);
    
    // add -L libs to libdir
    QStringList &libs = project->variables()["QMAKE_LIBS"];
    for ( QStringList::Iterator libit = libs.begin(); libit != libs.end();  ) {
	if ( (*libit).startsWith( "-L" ) ) {
	    project->variables()["QMAKE_LIBDIR"] += (*libit).mid(2);
	    libit = libs.erase( libit );
	} else {
	    ++libit;
	}
    }

    project->variables()["QMAKE_FILETAGS"] += QStringList::split(' ',
	"HEADERS SOURCES DEF_FILE RC_FILE TARGET QMAKE_LIBS DESTDIR DLLDESTDIR INCLUDEPATH");
    QStringList &l = project->variables()["QMAKE_FILETAGS"];
    QStringList::Iterator it;
    for(it = l.begin(); it != l.end(); ++it) {
	QStringList &gdmf = project->variables()[(*it)];
	for(QStringList::Iterator inner = gdmf.begin(); inner != gdmf.end(); ++inner)
	    (*inner) = Option::fixPathToTargetOS((*inner), FALSE);
    }

    if ( project->isActiveConfig("dll") ) {
		QString destDir = "";
		if (!project->first("DESTDIR").isEmpty())
			destDir = project->first("DESTDIR") + "\\";
		project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,--out-implib,") +
					  destDir + "lib" + project->first("TARGET") + ".a");
    }
    
    if ( !project->variables()["DEF_FILE"].isEmpty() )
	project->variables()["QMAKE_LFLAGS"].append(QString("-Wl,") + project->first("DEF_FILE"));
//    if(!project->isActiveConfig("incremental"))
//	project->variables()["QMAKE_LFLAGS"].append(QString("/incremental:no"));

#if 0
    if ( !project->variables()["VERSION"].isEmpty() ) {
	QString version = project->variables()["VERSION"][0];
	int firstDot = version.find( "." );
	QString major = version.left( firstDot );
	QString minor = version.right( version.length() - firstDot - 1 );
	minor.replace( ".", "" );
	project->variables()["QMAKE_LFLAGS"].append( "/VERSION:" + major + "." + minor );
    }
#endif
    
    if ( !project->variables()["RC_FILE"].isEmpty()) {
	if ( !project->variables()["RES_FILE"].isEmpty()) {
	    fprintf(stderr, "Both .rc and .res file specified.\n");
	    fprintf(stderr, "Please specify one of them, not both.");
	    exit(666);
	}
	project->variables()["RES_FILE"] = project->variables()["RC_FILE"];
        project->variables()["RES_FILE"].first().replace(".rc",".o");
	project->variables()["POST_TARGETDEPS"] += project->variables()["RES_FILE"];
	project->variables()["CLEAN_FILES"] += project->variables()["RES_FILE"];
    }
    
    if ( !project->variables()["RES_FILE"].isEmpty())
	project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];

    MakefileGenerator::init();
    
    if ( !project->variables()["VERSION"].isEmpty()) {
	QStringList l = QStringList::split('.', project->first("VERSION"));
	project->variables()["VER_MAJ"].append(l[0]);
	project->variables()["VER_MIN"].append(l[1]);
    }
    
    if(project->isActiveConfig("dll")) {
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") +"lib" + project->first("TARGET") + ".a");
    }
    
    QStringList &quc = project->variables()["QMAKE_EXTRA_WIN_COMPILERS"];
    for(it = quc.begin(); it != quc.end(); ++it) {
	QString tmp_out = project->variables()[(*it) + ".output"].first();
	if(tmp_out.isEmpty())
	    continue;
	QStringList &tmp = project->variables()[(*it) + ".input"];
	for(QStringList::Iterator it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
	    QStringList &inputs = project->variables()[(*it2)];
	    for(QStringList::Iterator input = inputs.begin(); input != inputs.end(); ++input) {
		QFileInfo fi(Option::fixPathToLocalOS((*input)));
		QString in = Option::fixPathToTargetOS((*input), FALSE),
		       out = tmp_out;
		out.replace("${QMAKE_FILE_BASE}", fi.baseName());
		out.replace("${QMAKE_FILE_NAME}", fi.fileName());
		if(project->variables()[(*it) + ".CONFIG"].indexOf("no_link") == -1)
		    project->variables()["OBJCOMP"] += out;
	    }
	}
    }
}

void
MingwMakefileGenerator::writeSubDirs(QTextStream &t)
{
    QString qs ;
    QTextStream ts (&qs, IO_WriteOnly) ;
    Win32MakefileGenerator::writeSubDirs( ts ) ;
    QRegExp rx("(\\n\\tcd [^\\n\\t]+)(\\n\\t.+)\\n\\t@cd ..") ;
    rx.setMinimal(TRUE);
    int pos = 0 ;
    while ( -1 != (pos = rx.search( qs, pos)))
    {
	QString qsMatch = rx.cap(2);
	qsMatch.replace("\n\t"," && \\\n\t");
	qs.replace(pos+rx.cap(1).length(), rx.cap(2).length(), qsMatch );
	pos += (rx.cap(1).length()+qsMatch.length());
    }
    t << qs ;
}
