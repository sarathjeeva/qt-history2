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

#include "project.h"

#include "unixmake.h"
#include "borland_bmake.h"
#include "msvc_nmake.h"
#include "msvc_dsp.h"

#include <stdio.h>
#include <ctype.h>


extern int line_count;
extern "C" void yyerror(const char *foo)
{ 
    printf("%d: %s\n", line_count, foo);
}

int
main(int argc, char **argv)
{
    QMakeProject proj;
    for(int x = 1; x < argc; x++) {
	/* read in the project */
	if(!proj.read(argv[x], NULL)) {
	    printf("Error processing project file: %s\n", argv[x]);
	    continue;
	}

	/* now generate a makefile */
	DspMakefileGenerator mkfile(&proj);
	mkfile.write(NULL);

#ifdef QMAKE_DEBUG
	QMap<QString, QStringList> &vars = proj.variables();
	for( QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it)
	    printf("%s === %s\n", it.key().latin1(), it.data().join(" ").latin1());
#endif

    }
    return 0;
}
