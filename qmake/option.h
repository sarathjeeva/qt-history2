/****************************************************************************
** $Id: $
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
#ifndef __OPTION_H__
#define __OPTION_H__

#include "project.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>

#define QMAKE_VERSION_MAJOR 1
#define QMAKE_VERSION_MINOR 00
#define QMAKE_VERSION_PATCH a
#define QMAKE_VERSION "1.00a"

void fixEnvVariables(QString &x);
void debug_msg(int level, const char *fmt, ...);
enum QMakeWarn {
    WarnNone    = 0x00,
    WarnParser  = 0x01,
    WarnLogic   = 0x02,
    WarnAll     = 0xFF
};
void warn_msg(QMakeWarn t, const char *fmt, ...);

struct Option
{
    //simply global convenience
    static QString prl_ext;
    static QString ui_ext;
    static QString h_ext;
    static QString moc_ext;
    static QString cpp_ext;
    static QString cc_ext;
    static QString cxx_ext;
    static QString obj_ext;
    static QString lex_ext;
    static QString yacc_ext;
    static QString moc_mod;
    static QString lex_mod;
    static QString yacc_mod;
    static QString dir_sep;
    //both of these must be called..
    static bool parseCommandLine(int argc, char **argv); //parse cmdline
    static bool postProcessProject(QMakeProject *); //parse project

    //and convenience functions    
    static QString fixPathToLocalOS(const QString& in, bool fix_env=TRUE);
    static QString fixPathToTargetOS(const QString& in, bool fix_env=TRUE);

    //global qmake mode, can only be in one mode per invocation!
    enum QMAKE_MODE { QMAKE_GENERATE_NOTHING, QMAKE_GENERATE_PROJECT, QMAKE_GENERATE_MAKEFILE };
    static QMAKE_MODE qmake_mode;

    //all modes
    static QFile output;
    static QString output_dir;
    static int debug_level;
    static int warn_level;
    static QStringList before_user_vars, after_user_vars;
    enum TARG_MODE { TARG_UNIX_MODE, TARG_WIN_MODE, TARG_MACX_MODE, TARG_MAC9_MODE };
    static TARG_MODE target_mode;
    static QString user_template;

    //QMAKE_GENERATE_PROJECT options
    struct projfile {
	static bool do_pwd;
	static QStringList project_dirs;
    };

    //QMAKE_GENERATE_MAKEFILE options
    struct mkfile {
	static QString qmakespec;
	static bool do_cache;
	static bool do_deps;
	static bool do_dep_heuristics;
	static bool do_preprocess;
	static QString cachefile;
	static int cachefile_depth;
	static QStringList project_files;
    };
};


#endif /* __OPTION_H__ */
