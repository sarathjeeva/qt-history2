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

#include "projectgenerator.h"
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <option.h>
#include <qregexp.h>

ProjectGenerator::ProjectGenerator(QMakeProject *p) : MakefileGenerator(p), init_flag(FALSE)
{
}

void
ProjectGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    QMap<QString, QStringList> &v = project->variables();
    v["TEMPLATE"] += Option::user_template.isEmpty() ? QString("app") : Option::user_template;

    //figure out target
    QString o = Option::output.name();
    if(o == "-" || o.isEmpty()) {
	o = "unknown";
    } else {
	int s = o.findRev(Option::dir_sep);
	if(s != -1)
	    o = o.right(o.length() - (s + 1));
	o.replace(QRegExp(".pro$"), "");
	v["TARGET"] += o;
    }

    //the scary stuff
    if(Option::projfile::do_pwd)
	Option::projfile::project_dirs.prepend("*.cpp; *.ui; *.c; *.y; *.l");
    for(QStringList::Iterator pd = Option::projfile::project_dirs.begin(); pd != Option::projfile::project_dirs.end(); pd++)
    {
	QString dir;
	if(QFile::exists((*pd))) {
	    QFileInfo fi((*pd));
	    if(fi.isDir()) {
		dir = (*pd);
	    } else {
		QString file = (*pd);
		int s = file.findRev(Option::dir_sep);
		if(s != -1)
		    dir = file.left(s+1);
		addFile(file);
	    }
	} else { //regexp
	    QString regx = (*pd);
	    int s = regx.findRev(Option::dir_sep);
	    if(s != -1) {
		dir = regx.left(s+1);
		regx = regx.right(regx.length() - (s+1));
	    }
	    QDir d(dir, regx);
	    for(int i = 0; i < d.count(); i++)
		addFile(dir + d[i]);
	}
	if(!dir.isEmpty() && !v["DEPENDPATH"].contains(dir))
	    v["DEPENDPATH"] += dir;
    }

    bool no_qt_files = TRUE;
    QStringList &d = v["DEPENDPATH"], &h = v["HEADERS"];
    QString srcs[] = { "SOURCES", "YACCSOURCES", "LEXSOURCES", "INTERFACES", QString::null };
    for(int i = 0; !srcs[i].isNull(); i++) {
	QStringList &l = v[srcs[i]];
	for(QStringList::Iterator val_it = l.begin(); val_it != l.end(); ++val_it) {
	    if(generateDependancies(d, (*val_it))) {
		QStringList &tmp = depends[(*val_it)];
		if(!tmp.isEmpty()) {
		    for(QStringList::Iterator dep_it = tmp.begin(); dep_it != tmp.end(); ++dep_it) {
			QString file_no_path = (*dep_it).right(
			    (*dep_it).length() - ((*dep_it).findRev(Option::dir_sep)+1));
			if(no_qt_files && file_no_path.find(QRegExp("^q[a-z_0-9].h$")) != -1)
			    no_qt_files = FALSE;
			if((*dep_it).right(Option::h_ext.length()) == Option::h_ext) {
			    if((*dep_it).left(1).lower() == "q") {
				QString qhdr = (*dep_it).lower();
				if(file_no_path == "qthread.h")
				    addConfig("thread");
			    }
			    QString src((*dep_it).left((*dep_it).length() - Option::h_ext.length()) + Option::cpp_ext);
			    if(QFile::exists(src)) {
				if(!l.contains(src))
				    l.append(src);
			    }
			} else if((*dep_it).right(2) == ".l" &&
				  file_no_path.left(Option::lex_mod.length()) == Option::lex_mod) {
			    addConfig("lex_included");
			}
			if(!h.contains((*dep_it))) {
			    if(generateMocList((*dep_it)) && !findMocDestination((*dep_it)).isEmpty())
				h += (*dep_it);
			}
		    }
		}
	    }
	}
    }
    if(h.isEmpty())
	addConfig("moc", FALSE);
    if(no_qt_files)
	addConfig("qt", FALSE);
}


bool
ProjectGenerator::writeMakefile(QTextStream &t)
{
    t << "##############################################################" << endl;
    t << "# Automatically generated by qmake " << QDateTime::currentDateTime().toString() << endl;
    t << "##############################################################" << endl << endl;
#define WRITE_VAR(x) t << getWritableVar(x)
    WRITE_VAR("TEMPLATE");
    WRITE_VAR("TARGET");
    WRITE_VAR("CONFIG");
    WRITE_VAR("CONFIG_REMOVE"); //-= rule
    WRITE_VAR("DEPENDPATH");

    t << endl << "# Input" << endl;
    WRITE_VAR("HEADERS");
    WRITE_VAR("INTERFACES");
    WRITE_VAR("LEXSOURCES");
    WRITE_VAR("YACCSOURCES");
    WRITE_VAR("SOURCES");
#undef WRITE_VAR
    for(QStringList::Iterator it = Option::user_vars.begin(); it != Option::user_vars.end(); ++it)
	t << (*it) << endl;
    return TRUE;
}

bool
ProjectGenerator::addConfig(const QString &cfg, bool add)
{
    QString where = "CONFIG";
    if(!add)
	where = "CONFIG_REMOVE";
    if(!project->variables()[where].contains(cfg)) {
	project->variables()[where] += cfg;
	return TRUE;
    }
    return FALSE;
}


bool
ProjectGenerator::addFile(const QString &file)
{
    QString dir;
    int s = file.findRev(Option::dir_sep);
    if(s != -1)
	dir = file.left(s+1);

    QString where;
    if(file.mid(dir.length(), Option::moc_mod.length()) == Option::moc_mod) {
	//do nothing
    } else if(file.right(Option::cpp_ext.length()) == Option::cpp_ext) {
	if(QFile::exists(file.left(file.length() - Option::cpp_ext.length()) + Option::ui_ext))
	    ; //do nothing
	else
	    where = "SOURCES";
    } else if(file.right(Option::ui_ext.length()) == Option::ui_ext) {
	where = "INTERFACES";
    } else if(file.right(2) == ".c") {
	where = "SOURCES";
    } else if(file.right(2) == ".l") {
	where = "LEXSOURCES";
    } else if(file.right(2) == ".y") {
	where = "YACCSOURCES";
    }
    if(!where.isEmpty() && !project->variables()[where].contains(file)) {
	project->variables()[where] += file;
	return TRUE;
    }
    return FALSE;
}


QString
ProjectGenerator::getWritableVar(const QString &v)
{
    QStringList &vals = project->variables()[v];
    if(vals.isEmpty())
	return "";

    QString ret;
    if(v.right(7) == "_REMOVE")
	ret = v.left(v.length() - 7) + " -= ";
    else
	ret = v + " += ";
    if(vals.count() > 5) {
	QString spaces;
	for(int i = 0; i < ret.length(); i++)
	    spaces += " ";
	ret += vals.join("\\\n" + spaces);
    } else {
        ret += vals.join(" ");
    }
    return ret + "\n";
}
