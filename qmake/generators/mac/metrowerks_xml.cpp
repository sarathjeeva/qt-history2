/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
**
** Definition of ________ class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
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

#include "option.h"
#include "metrowerks_xml.h"
#include <time.h>
#include <qdir.h>
#include <qregexp.h>
#include <stdlib.h>
#ifdef Q_OS_MAC
#include "Files.h"
#endif

MetrowerksMakefileGenerator::MetrowerksMakefileGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
{

}

bool
MetrowerksMakefileGenerator::writeMakefile(QTextStream &t)
{
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	/* for now just dump, I need to generated an empty xml or something.. */
	fprintf(stderr, "Project file not generated because all requirements not met:\n\t%s\n",
		var("QMAKE_FAILED_REQUIREMENTS").latin1());
	return TRUE;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	return writeMakeParts(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeHeader(t);
	qDebug("Not supported!");
	return TRUE;
    }
    return FALSE;
}

bool
MetrowerksMakefileGenerator::writeMakeParts(QTextStream &t)
{
    QString xmlfile;
    if ( !project->variables()["XML_TEMPLATE"].isEmpty() ) {
	xmlfile = project->first("XML_TEMPLATE");
    } else {
	xmlfile = project->first("MWERKS_XML_TEMPLATE");
    }
    xmlfile = findTemplate(xmlfile);

    QFile file(xmlfile);
    if(!file.open(IO_ReadOnly )) {
	fprintf(stderr, "Cannot open XML file: %s\n", xmlfile.latin1());
	return FALSE;
    }
    QTextStream xml(&file);

    int rep;
    QString line;
    while ( !xml.eof() ) {
	line = xml.readLine();
	while((rep = line.find(QRegExp("\\$\\$[a-zA-Z0-9_-]*"))) != -1) {
	    QString torep = line.mid(rep, line.find(QRegExp("[^\\$a-zA-Z0-9_-]"), rep) - rep);
	    QString variable = torep.right(torep.length()-2);

	    t << line.left(rep); //output the left side
	    line = line.right(line.length() - (rep + torep.length())); //now past the variable
	    if(variable == "CODEWARRIOR_HEADERS" || variable == "CODEWARRIOR_SOURCES") {
		QString arg=variable.right(variable.length() - variable.findRev('_') - 1);
		if(!project->variables()[arg].isEmpty()) {
		    QStringList &list = project->variables()[arg];
		    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			t << "\t\t\t\t<FILE>" << endl
			  << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			  << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			  << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			  << "\t\t\t\t\t<FILEKIND>Text</FILEKIND>" << endl
			  << "\t\t\t\t\t<FILEFLAGS></FILEFLAGS>" << endl
			  << "\t\t\t\t</FILE>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_SOURCES_LINKORDER" || variable == "CODEWARRIOR_HEADERS_LINKORDER") {
		QString arg=variable.mid(variable.find('_')+1, variable.length()-variable.findRev('_')-3);
		if(!project->variables()[arg].isEmpty()) {
		    QStringList &list = project->variables()[arg];
		    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			t << "\t\t\t\t<FILEREF>" << endl
			  << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			  << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			  << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			  << "\t\t\t\t</FILEREF>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_HEADERS_GROUP" || variable == "CODEWARRIOR_SOURCES_GROUP") {
		QString arg=variable.mid(variable.find('_')+1, variable.length()-variable.findRev('_')+1);
		if(!project->variables()[arg].isEmpty()) {
		    QStringList &list = project->variables()[arg];
		    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			t << "\t\t\t\t<FILEREF>" << endl
			  << "\t\t\t\t\t<TARGETNAME>" << var("TARGET") << "</TARGETNAME>" << endl
			  << "\t\t\t\t\t<PATHTYPE>Name</PATHTYPE>" << endl
			  << "\t\t\t\t\t<PATH>" << (*it) << "</PATH>" << endl
			  << "\t\t\t\t\t<PATHFORMAT>MacOS</PATHFORMAT>" << endl
			  << "\t\t\t\t</FILEREF>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_DEPENDPATH" || variable == "CODEWARRIOR_INCLUDEPATH") {
		QString arg=variable.mid(variable.find('_')+1, variable.length()-variable.findRev('_'));
		QStringList list = project->variables()[arg];
		if(arg == "INCLUDEPATH") {
		    list << Option::mkfile::qmakespec;
		    list << Option::output_dir;
		}
		if(!list.isEmpty()) {
		    QString volume;
#ifdef Q_OS_MAC
		    uchar foo[512];
		    HVolumeParam pb;
		    memset(&pb, '\0', sizeof(pb));
		    pb.ioVRefNum = 0;
		    pb.ioNamePtr = foo;
		    if(PBHGetVInfoSync((HParmBlkPtr)&pb) == noErr) {
			int len = foo[0];
			memcpy(foo,foo+1, len);
			foo[len] = '\0';
			volume = (char *)foo;
		    }
#endif

		    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
			QString p = (*it);
			if(p.right(1) != "/")
			    p += "/";
			if(QDir::isRelativePath(p))
			    p.prepend(Option::output_dir + '/');
			p = QDir::cleanDirPath(p) + ":";
			p.replace(QRegExp("/"), ":");
			if(!volume.isEmpty())
			    p.prepend(volume); //FIXME

			t << "\t\t\t\t\t<SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>SearchPath</NAME>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>Path</NAME>"
			  << "<VALUE>" << p << "</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>PathFormat</NAME><VALUE>MacOS</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t\t<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t</SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t\t<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>" << endl
			  << "\t\t\t\t\t</SETTING>" << endl;
		    }
		}
	    } else if(variable == "CODEWARRIOR_WARNING") {
		t << (int)project->isActiveConfig("warn_on");
	    } else {
		t << var(variable);
	    }
	}
	t << line << endl;
    }
    t << endl;
    file.close();

    if(mocAware()) { //generate the $$TARGET.mocs file
	QString mocs = Option::output_dir + Option::dir_sep + project->variables()["TARGET"].first() + ".mocs";
	QFile mocfile(mocs);
	if(!mocfile.open(IO_WriteOnly)) {
	    fprintf(stderr, "Cannot open MOCS file: %s\n", mocs.latin1());
	} else {
	    QTextStream mocs(&mocfile);
	    QStringList &list = project->variables()["SRCMOC"];
	    for(QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
		QString src = findMocSource((*it));
		if(src.findRev('/') != -1)
		    src = src.right(src.length() - src.findRev('/') - 1);
		mocs << src << endl;
	    }
	    mocfile.close();
	}
    }
    return TRUE;
}



void
MetrowerksMakefileGenerator::init()
{
    if(init_flag)
	return;
    QStringList::Iterator it;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app" ) {
	project->variables()["MWERKS_XML_TEMPLATE"].append("mwerksapp.xml");
    } else if(project->first("TEMPLATE") == "lib") {
	qDebug("Lib not supported yet");
	exit(666);
    }
    
    QStringList &configs = project->variables()["CONFIG"];
    if(project->isActiveConfig("qt"))
	if(configs.findIndex("moc")) configs.append("moc");
    if ( project->isActiveConfig("moc") ) {
	setMocAware(TRUE);
    }

    MakefileGenerator::init();

    //let metrowerks find the files
    QString paths[] = { QString("SOURCES"),QString("HEADERS"),QString::null };
    for(int y = 0; paths[y] != QString::null; y++) {
	QStringList &l = project->variables()[paths[y]];
	for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
	    int s = (*val_it).findRev('/');
	    if(s != -1) {
		QString dir = (*val_it).left(s);
		(*val_it) = (*val_it).right((*val_it).length() - s - 1);
		if(project->variables()["DEPENDPATH"].findIndex(dir) == -1 &&
		   project->variables()["INCLUDEPATH"].findIndex(dir) == -1)
		    project->variables()["INCLUDEPATH"].append(dir);
	    }
	}
    }
}


QString
MetrowerksMakefileGenerator::findTemplate(QString file)
{
    QString ret;
    if(!QFile::exists(ret = file) && !QFile::exists((ret = (QString(getenv("HOME")) + "/.tmake/" + file))) &&
	!QFile::exists((ret = QString(getenv("QTDIR")) + "/mkspecs/mac-mwerks/" + file)))
	return "";
    return ret;
}

