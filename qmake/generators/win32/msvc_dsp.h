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
#ifndef __DSPMAKE_H__
#define __DSPMAKE_H__

#include "winmakefile.h"

class DspMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    bool writeDspParts(QTextStream &);

    bool writeMakefile(QTextStream &);
    QString findTemplate(QString file);
    void init();

public:
    DspMakefileGenerator(QMakeProject *p);
    ~DspMakefileGenerator();

    QString defaultMakefile() const;

protected:
    virtual void processPrlVariable(const QString &, const QStringList &);

};

inline DspMakefileGenerator::~DspMakefileGenerator()
{ }

inline QString DspMakefileGenerator::defaultMakefile() const
{
    return project->first("TARGET") + project->first("DSP_EXTENSION");
}

#endif /* __DSPMAKE_H__ */
