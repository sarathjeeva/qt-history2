/**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Linguist.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SIMTEXTH_H
#define SIMTEXTH_H

#include <qstring.h>
#include <qvaluelist.h>

struct Candidate {
    QString source;
    QString target;

    Candidate() { }
    Candidate( const QString& source0, const QString& target0 )
	: source( source0 ), target( target0 ) { }
};

inline bool operator==( const Candidate& c, const Candidate& d ) {
    return c.target == d.target && c.source == d.source;
}
inline bool operator!=( const Candidate& c, const Candidate& d ) {
    return !operator==( c, d );
}

typedef QValueList<Candidate> CandidateList;

CandidateList similarTextHeuristicCandidates( const MetaTranslator *tor,
					      const char *text,
					      int maxCandidates );

#endif
