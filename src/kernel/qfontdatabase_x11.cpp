/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include <qplatformdefs.h>

#include "qt_x11.h"

#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifndef QT_NO_XFTFREETYPE
#include <freetype/freetype.h>
#endif

// #define QFONTDATABASE_DEBUG
// #define FONT_MATCH_DEBUG
#ifdef QFONTDATABASE_DEBUG
#  include <qdatetime.h>
#endif // QFONTDATABASE_DEBUG


// ----- begin of generated code -----

#define make_tag( c1, c2, c3, c4 ) \
( (((unsigned int)c1)<<24) | (((unsigned int)c2)<<16) | \
(((unsigned int)c3)<<8) | ((unsigned int)c4) )

struct XlfdEncoding {
    const char *name;
    int id;
    int mib;
    unsigned int hash1;
    unsigned int hash2;
};

static const XlfdEncoding xlfd_encoding[] = {
    { "iso8859-1", 0, 0, make_tag('i','s','o','8'), make_tag('5','9','-','1') },
    { "iso8859-2", 1, 5, make_tag('i','s','o','8'), make_tag('5','9','-','2') },
    { "iso8859-3", 2, 6, make_tag('i','s','o','8'), make_tag('5','9','-','3') },
    { "iso8859-4", 3, 7, make_tag('i','s','o','8'), make_tag('5','9','-','4') },
    { "iso8859-14", 4, 110, make_tag('i','s','o','8'), make_tag('9','-','1','4') },
    { "iso8859-15", 5, 111, make_tag('i','s','o','8'), make_tag('9','-','1','5') },
    { "iso8859-5", 6, 8, make_tag('i','s','o','8'), make_tag('5','9','-','5') },
    { "*-cp1251", 7, 2251, 0, make_tag('1','2','5','1') },
    { "koi8-ru", 8, 2084, make_tag('k','o','i','8'), make_tag('8','-','r','u') },
    { "koi8-u", 9, 2088, make_tag('k','o','i','8'), make_tag('i','8','-','u') },
    { "koi8-r", 10, 2084, make_tag('k','o','i','8'), make_tag('i','8','-','r') },
    { "iso8859-7", 11, 10, make_tag('i','s','o','8'), make_tag('5','9','-','7') },
    { "iso8859-6", 12, 82, make_tag('i','s','o','8'), make_tag('5','9','-','6') },
    { "iso8859-8", 13, 85, make_tag('i','s','o','8'), make_tag('5','9','-','8') },
    { "gb18030-0", 14, -114, make_tag('g','b','1','8'), make_tag('3','0','-','0') },
    { "gb18030.2000-0", 15, -113, make_tag('g','b','1','8'), make_tag('0','0','-','0') },
    { "gbk-0", 16, -113, make_tag('g','b','k','-'), make_tag('b','k','-','0') },
    { "gb2312.*-0", 17, 57, make_tag('g','b','2','3'), 0 },
    { "jisx0201*-0", 18, 15, make_tag('j','i','s','x'), 0 },
    { "jisx0208*-0", 19, 63, make_tag('j','i','s','x'), 0 },
    { "ksc5601.1987-0", 20, 36, make_tag('k','s','c','5'), make_tag('8','7','-','0') },
    { "big5hkscs-0", 21, -2101, make_tag('b','i','g','5'), make_tag('c','s','-','0') },
    { "hkscs-1", 22, -2101, make_tag('h','k','s','c'), make_tag('c','s','-','1') },
    { "big5*-*", 23, -2026, make_tag('b','i','g','5'), 0 },
    { "tscii-*", 24, 2028, make_tag('t','s','c','i'), 0 },
    { "tis620*-*", 25, 2259, make_tag('t','i','s','6'), 0 },
    { "iso8859-11", 26, 2259, make_tag('i','s','o','8'), make_tag('9','-','1','1') },
    { "mulelao-1", 27, -4242, make_tag('m','u','l','e'), make_tag('a','o','-','1') },
    { "ethiopic-unicode", 28, 0, make_tag('e','t','h','i'), make_tag('c','o','d','e') },
    { "iso10646-1", 29, 0, make_tag('i','s','o','1'), make_tag('4','6','-','1') },
    { "unicode-*", 30, 0, make_tag('u','n','i','c'), 0 },
    { "*-symbol", 31, 0, 0, make_tag('m','b','o','l') },
    { "*-fontspecific", 32, 0, 0, make_tag('i','f','i','c') },
    { 0, 0, 0, 0, 0 }
};

static const char scripts_for_xlfd_encoding[33][58] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 1, 0, 0, 0, 0 }

};

// ----- end of generated code -----


const int numEncodings = sizeof( xlfd_encoding ) / sizeof( XlfdEncoding ) - 1;

int qt_xlfdEncoding_Id( const char *encoding )
{
    // qDebug("looking for encoding id for '%s'", encoding );
    int len = strlen( encoding );
    if ( len < 4 )
	return -1;
    unsigned int hash1 = make_tag( encoding[0], encoding[1], encoding[2], encoding[3] );
    const char *ch = encoding + len - 4;
    unsigned int hash2 = make_tag( ch[0], ch[1], ch[2], ch[3] );

    const XlfdEncoding *enc = xlfd_encoding;
    for ( ; enc->name; ++enc ) {
	if ( (enc->hash1 && enc->hash1 != hash1) ||
	     (enc->hash2 && enc->hash2 != hash2) )
	    continue;
	// hashes match, do a compare if strings match
	// the enc->name can contain '*'s we have to interpret correctly
	const char *n = enc->name;
	const char *e = encoding;
	while ( 1 ) {
 	    // qDebug("bol: *e='%c', *n='%c'", *e,  *n );
	    if ( *e == '\0' ) {
		if ( *n )
		    break;
		// qDebug( "found encoding id %d", enc->id );
		return enc->id;
	    }
	    if ( *e == *n ) {
		++e;
		++n;
		continue;
	    }
	    if ( *n != '*' )
		break;
	    ++n;
 	    // qDebug("skip: *e='%c', *n='%c'", *e,  *n );
	    while ( *e && *e != *n )
		++e;
	}
    }
    // qDebug( "couldn't find encoding %s", encoding );
    return -1;
}

int qt_mibForXlfd( const char * encoding )
{
    int id = qt_xlfdEncoding_Id( encoding );
    if ( id != -1 )
	return xlfd_encoding[id].mib;
    return 0;
};

static const char * xlfd_for_id( int id )
{
    // special case: -1 returns the "*-*" encoding, allowing us to do full
    // database population in a single X server round trip.
    if ( id < 0 || id > (signed) ( numEncodings ) )
	return "*-*";
    return xlfd_encoding[id].name;
}

enum XLFDFieldNames {
    Foundry,
    Family,
    Weight,
    Slant,
    Width,
    AddStyle,
    PixelSize,
    PointSize,
    ResolutionX,
    ResolutionY,
    Spacing,
    AverageWidth,
    CharsetRegistry,
    CharsetEncoding,
    NFontFields
};

// Splits an X font name into fields separated by '-'
static bool parseXFontName( char *fontName, char **tokens )
{
    if ( ! fontName || fontName[0] == '0' || fontName[0] != '-' ) {
	tokens[0] = 0;
	return FALSE;
    }

    int	  i;
    ++fontName;
    for ( i = 0; i < NFontFields && fontName && fontName[0]; ++i ) {
	tokens[i] = fontName;
	for ( ;; ++fontName ) {
	    if ( *fontName == '-' )
		break;
	    if ( ! *fontName ) {
		fontName = 0;
		break;
	    }
	}

	if ( fontName ) *fontName++ = '\0';
    }

    if ( i < NFontFields ) {
	for ( int j = i ; j < NFontFields; ++j )
	    tokens[j] = 0;
	return FALSE;
    }

    return TRUE;
}

static inline bool isZero(char *x)
{
    return (x[0] == '0' && x[1] == 0);
}

static inline bool isScalable( char **tokens )
{
    return (isZero(tokens[PixelSize]) &&
	    isZero(tokens[PointSize]) &&
	    isZero(tokens[AverageWidth]));
}

static inline bool isSmoothlyScalable( char **tokens )
{
    return (isZero(tokens[ResolutionX]) &&
	    isZero(tokens[ResolutionY]));
}

static inline bool isFixedPitch( char **tokens )
{
    return (tokens[Spacing][0] == 'm' ||
	    tokens[Spacing][0] == 'c' ||
	    tokens[Spacing][0] == 'M' ||
	    tokens[Spacing][0] == 'C');
}


static int getFontWeight( const QCString &weightString, bool adjustScore = FALSE )
{
    // Test in decreasing order of commonness
    if ( weightString == "medium" )       return QFont::Normal;
    else if ( weightString == "bold" )    return QFont::Bold;
    else if ( weightString == "demibold") return QFont::DemiBold;
    else if ( weightString == "black" )   return QFont::Black;
    else if ( weightString == "light" )   return QFont::Light;

    QCString s(weightString.lower());

    if ( s.contains("bold") ) {
	if ( adjustScore )
	    return (int) QFont::Bold - 1;  // - 1, not sure that this IS bold
	else
	    return (int) QFont::Bold;
    }

    if ( s.contains("light") ) {
	if ( adjustScore )
	    return (int) QFont::Light - 1; // - 1, not sure that this IS light
	else
	    return (int) QFont::Light;
    }

    if ( s.contains("black") ) {
	if ( adjustScore )
	    return (int) QFont::Black - 1; // - 1, not sure this IS black
	else
	    return (int) QFont::Black;
    }

    if ( adjustScore )
	return (int) QFont::Normal - 2;	   // - 2, we hope it's close to normal

    return (int) QFont::Normal;
}


static QtFontStyle::Key getStyle( char ** tokens )
{
    QtFontStyle::Key key;

    char slant0 = tolower( (uchar) tokens[Slant][0] );

    if ( slant0 == 'r' ) {
        if ( tokens[Slant][1]) {
            char slant1 = tolower( (uchar) tokens[Slant][1] );

            if ( slant1 == 'o' )
                key.oblique = TRUE;
            else if ( slant1 == 'i' )
		key.italic = TRUE;
        }
    } else if ( slant0 == 'o' )
	key.oblique = TRUE;
    else if ( slant0 == 'i' )
	key.italic = TRUE;

    key.weight = getFontWeight( tokens[Weight] );

    if ( qstrcmp( tokens[Width], "normal" ) == 0 ) {
	key.stretch = 100;
    } else if ( qstrcmp( tokens[Width], "semi condensed" ) == 0 ||
		qstrcmp( tokens[Width], "semicondensed" ) == 0 ) {
	key.stretch = 90;
    } else if ( qstrcmp( tokens[Width], "condensed" ) == 0 ) {
	key.stretch = 80;
    } else if ( qstrcmp( tokens[Width], "narrow" ) == 0 ) {
	key.stretch = 60;
    }

    return key;
}


static inline void capitalize ( char *s )
{
    bool space = TRUE;
    while( *s ) {
	if ( space )
	    *s = toupper( *s );
	space = ( *s == ' ' );
	++s;
    }
}


extern bool qt_has_xft; // defined in qfont_x11.cpp

static bool xlfdsFullyLoaded = FALSE;
static unsigned char encodingLoaded[numEncodings];

static void loadXlfds( const char *reqFamily, int encoding_id )
{
    QtFontFamily *fontFamily = reqFamily ? db->family( reqFamily ) : 0;

    // make sure we don't load twice
    if ( (encoding_id == -1 && xlfdsFullyLoaded) || (encoding_id != -1 && encodingLoaded[encoding_id]) )
	return;
    if ( fontFamily && fontFamily->xlfdLoaded )
	return;

    int fontCount;
    // force the X server to give us XLFDs
    QCString xlfd_pattern = "-*-";
    xlfd_pattern += reqFamily ? reqFamily : "*";
    xlfd_pattern += "-*-*-*-*-*-*-*-*-*-*-";
    xlfd_pattern += xlfd_for_id( encoding_id );

    char **fontList = XListFonts( QPaintDevice::x11AppDisplay(),
				  xlfd_pattern.data(),
				  0xffff, &fontCount );
    // qDebug("requesting xlfd='%s', got %d fonts", xlfd_pattern.data(), fontCount );


    char *tokens[NFontFields];

    for( int i = 0 ; i < fontCount ; i++ ) {
	if ( ! parseXFontName( fontList[i], tokens ) ) continue;

	// get the encoding_id for this xlfd.  we need to do this
	// here, since we can pass -1 to this function to do full
	// database population
	*(tokens[CharsetEncoding]-1) = '-';
	int encoding_id = qt_xlfdEncoding_Id( tokens[CharsetRegistry] );
	if ( encoding_id == -1 )
	    continue;

	char *familyName = tokens[Family];
	capitalize( familyName );
	char *foundryName = tokens[Foundry];
	capitalize( foundryName );
	QtFontStyle::Key styleKey = getStyle( tokens );

	bool smooth_scalable = FALSE;
	bool bitmap_scalable = FALSE;
	if ( isScalable(tokens) ) {
	    if ( isSmoothlyScalable( tokens ) )
		smooth_scalable = TRUE;
	    else
		bitmap_scalable = TRUE;
	}
	int pixelSize = atoi( tokens[PixelSize] );
	bool fixedPitch = isFixedPitch( tokens );


	QtFontFamily *family = fontFamily ? fontFamily : db->family( familyName, TRUE );
	family->fontFileIndex = -1;
	QtFontFoundry *foundry = family->foundry( foundryName, TRUE );
	QtFontStyle *style = foundry->style( styleKey, TRUE );

	style->weightName = qstrdup( tokens[Weight] );
	style->setwidthName = qstrdup( tokens[Width] );

	if ( smooth_scalable ) {
	    style->smoothScalable = TRUE;
	    style->bitmapScalable = FALSE;
	    pixelSize = SMOOTH_SCALABLE;
	}
	if ( !style->smoothScalable && bitmap_scalable )
	    style->bitmapScalable = TRUE;
	if ( !fixedPitch )
	    family->fixedPitch = FALSE;

	QtFontSize *size = style->pixelSize( pixelSize, TRUE );
	QtFontEncoding *enc = size->encodingID( encoding_id, TRUE );
	enc->pitch = *tokens[Spacing];
	if ( !enc->pitch ) enc->pitch = '*';

	for ( int script = 0; script < QFont::LastPrivateScript; ++script ) {
	    if ( scripts_for_xlfd_encoding[encoding_id][script] )
		family->scripts[script] = QtFontFamily::Supported;
	    else
		family->scripts[script] |= QtFontFamily::UnSupported_Xlfd;
	}
	if ( encoding_id == -1 )
	    family->xlfdLoaded = TRUE;
    }
    if ( !reqFamily ) {
	// mark encoding as loaded
	if ( encoding_id == -1 )
	    xlfdsFullyLoaded = TRUE;
	else
	    encodingLoaded[encoding_id] = TRUE;
    }

    XFreeFontNames( fontList );
}


#ifndef QT_NO_XFTFREETYPE
static int getXftWeight(int xftweight)
{
    int qtweight = QFont::Black;
    if (xftweight <= (XFT_WEIGHT_LIGHT + XFT_WEIGHT_MEDIUM) / 2)
	qtweight = QFont::Light;
    else if (xftweight <= (XFT_WEIGHT_MEDIUM + XFT_WEIGHT_DEMIBOLD) / 2)
	qtweight = QFont::Normal;
    else if (xftweight <= (XFT_WEIGHT_DEMIBOLD + XFT_WEIGHT_BOLD) / 2)
	qtweight = QFont::DemiBold;
    else if (xftweight <= (XFT_WEIGHT_BOLD + XFT_WEIGHT_BLACK) / 2)
	qtweight = QFont::Bold;

    return qtweight;
}

static void loadXft()
{
    if (!qt_has_xft)
	return;

    XftFontSet  *fonts;

    QString familyName;
    char *value;
    int weight_value;
    int slant_value;
    int spacing_value;
    char *file_value;
    int index_value;

    fonts =
	XftListFonts(QPaintDevice::x11AppDisplay(),
		     QPaintDevice::x11AppScreen(),
		     (const char *)0,
		     XFT_FAMILY, XFT_WEIGHT, XFT_SLANT, XFT_SPACING, XFT_FILE, XFT_INDEX,
		     (const char *)0);

    for (int i = 0; i < fonts->nfont; i++) {
	if (XftPatternGetString(fonts->fonts[i],
				XFT_FAMILY, 0, &value) != XftResultMatch )
	    continue;
	// 	capitalize( value );
	familyName = value;

	slant_value = XFT_SLANT_ROMAN;
	weight_value = XFT_WEIGHT_MEDIUM;
	spacing_value = XFT_PROPORTIONAL;
	XftPatternGetInteger (fonts->fonts[i], XFT_SLANT, 0, &slant_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_WEIGHT, 0, &weight_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_SPACING, 0, &spacing_value);
	XftPatternGetString (fonts->fonts[i], XFT_FILE, 0, &file_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_INDEX, 0, &index_value);

	QtFontFamily *family = db->family( familyName, TRUE );
	family->hasXft = TRUE;

	QCString file = file_value;
	family->fontFilename = file;
	family->fontFileIndex = index_value;

	QtFontStyle::Key styleKey;
	styleKey.italic = (slant_value == XFT_SLANT_ITALIC);
	styleKey.oblique = (slant_value == XFT_SLANT_OBLIQUE);
	styleKey.weight = getXftWeight( weight_value );

	QtFontFoundry *foundry = family->foundry( QString::null,  TRUE );
	QtFontStyle *style = foundry->style( styleKey,  TRUE );

	style->smoothScalable = TRUE;
	family->fixedPitch = ( spacing_value >= XFT_MONO );

	QtFontSize *size = style->pixelSize( SMOOTH_SCALABLE, TRUE );
	QtFontEncoding *enc = size->encodingID( -1, TRUE );
	enc->pitch = ( spacing_value >= XFT_CHARCELL ? 'c' :
		       ( spacing_value >= XFT_MONO ? 'm' : 'p' ) );
    }

    XftFontSetDestroy (fonts);
}

#define MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          ( ( (Q_UINT32)_x1 << 24 ) |     \
            ( (Q_UINT32)_x2 << 16 ) |     \
            ( (Q_UINT32)_x3 <<  8 ) |     \
              (Q_UINT32)_x4         )

#ifdef _POSIX_MAPPED_FILES
static inline Q_UINT32 getUInt(unsigned char *p)
{
    Q_UINT32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline Q_UINT16 getUShort(unsigned char *p)
{
    Q_UINT16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}

static inline void tag_to_string( char *string, Q_UINT32 tag )
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static Q_UINT16 getGlyphIndex( unsigned char *table, Q_UINT16 format, unsigned short unicode )
{
    if ( format == 0 ) {
	if ( unicode < 256 )
	    return (int) *(table+6+unicode);
    } else if ( format == 2 ) {
	qWarning("format 2 encoding table for Unicode, not implemented!");
    } else if ( format == 4 ) {
	Q_UINT16 segCountX2 = getUShort( table + 6 );
	unsigned char *ends = table + 14;
	Q_UINT16 endIndex = 0;
	int i = 0;
	for ( ; i < segCountX2/2 && (endIndex = getUShort( ends + 2*i )) < unicode; i++ );

	unsigned char *idx = ends + segCountX2 + 2 + 2*i;
	Q_UINT16 startIndex = getUShort( idx );

	if ( startIndex > unicode )
	    return 0;

	idx += segCountX2;
	Q_INT16 idDelta = (Q_INT16)getUShort( idx );
	idx += segCountX2;
	Q_UINT16 idRangeoffset_t = (Q_UINT16)getUShort( idx );

	Q_UINT16 glyphIndex;
	if ( idRangeoffset_t ) {
	    Q_UINT16 id = getUShort( idRangeoffset_t + 2*(unicode - startIndex) + idx);
	    if ( id )
		glyphIndex = ( idDelta + id ) % 0x10000;
	    else
		glyphIndex = 0;
	} else {
	    glyphIndex = (idDelta + unicode) % 0x10000;
	}
	return glyphIndex;
    }

    return 0;
}
#endif

static inline void checkXftCoverage( QtFontFamily *family )
{
#ifdef _POSIX_MAPPED_FILES
    QCString ext = family->fontFilename.mid( family->fontFilename.findRev( '.' ) ).lower();
    if ( family->fontFileIndex == 0 && ( ext == ".ttf" || ext == ".otf" ) ) {
	void *map;
	// qDebug("using own ttf code coverage checking of '%s'!", family->name.latin1() );
	int fd = open( family->fontFilename.data(), O_RDONLY );
	size_t pagesize = getpagesize();
	off_t offset = 0;
	size_t length = (8192 / pagesize + 1) * pagesize;

	if ( fd == -1 )
	    goto xftCheck;
	{
	    if ( (map = mmap( 0, length, PROT_READ, MAP_SHARED, fd, offset ) ) == MAP_FAILED )
		goto error;

	    unsigned char *ttf = (unsigned char *)map;
	    Q_UINT32 version = getUInt( ttf );
	    if ( version != 0x00010000 ) {
		// qDebug("file has wrong version %x",  version );
		goto error1;
	    }
	    Q_UINT16 numTables =  getUShort( ttf+4 );

	    unsigned char *table_dir = ttf + 12;
	    Q_UINT32 cmap_offset = 0;
	    Q_UINT32 cmap_length = 0;
	    for ( int n = 0; n < numTables; n++ ) {
		Q_UINT32 tag = getUInt( table_dir + 16*n );
		if ( tag == MAKE_TAG( 'c', 'm', 'a', 'p' ) ) {
		    cmap_offset = getUInt( table_dir + 16*n + 8 );
		    cmap_length = getUInt( table_dir + 16*n + 12 );
		    break;
		}
	    }
	    if ( !cmap_offset ) {
		// qDebug("no cmap found" );
		goto error1;
	    }

	    if ( cmap_offset + cmap_length > length ) {
		munmap( map, length );
		offset = cmap_offset / pagesize * pagesize;
		cmap_offset -= offset;
		length = (cmap_offset + cmap_length);
		if ( (map = mmap( 0, length, PROT_READ, MAP_SHARED, fd, offset ) ) == MAP_FAILED )
		    goto error;
	    }

	    unsigned char *cmap = ((unsigned char *)map) + cmap_offset;

	    version = getUShort( cmap );
	    if ( version != 0 ) {
		// qDebug("wrong cmap version" );
		goto error1;
	    }
	    numTables = getUShort( cmap + 2 );
	    unsigned char *unicode_table = 0;
	    for ( int n = 0; n < numTables; n++ ) {
		Q_UINT32 version = getUInt( cmap + 4 + 8*n );
		// accept both symbol and Unicode encodings. prefer unicode.
		if ( version == 0x00030001 || version == 0x00030000 ) {
		    unicode_table = cmap + getUInt( cmap + 4 + 8*n + 4 );
		    if ( version == 0x00030001 )
			break;
		}
	    }

	    if ( !unicode_table ) {
		// qDebug("no unicode table found" );
		goto error1;
	    }

	    Q_UINT16 format = getUShort( unicode_table );
	    if ( format != 4 )
		goto error1;

	    for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
		QChar ch = sampleCharacter( (QFont::Script)i );
		if ( getGlyphIndex( unicode_table, format, ch.unicode() ) ) {
		    // qDebug("font can render script %d",  i );
		    family->scripts[i] = QtFontFamily::Supported;
		} else {
		    family->scripts[i] |= QtFontFamily::UnSupported_Xft;
		}
	    }
	    family->xftScriptCheck = TRUE;
	}
    error1:
	munmap( map, length );
    error:
	close( fd );
	if ( family->xftScriptCheck )
	    return;
    }
 xftCheck:
#endif

#ifdef QFONTDATABASE_DEBUG
    qDebug("using Freetype for checking of '%s'", family->name.latin1() );
#endif // QFONTDATABASE_DEBUG

    FT_Library ft_lib;
    FT_Error error = FT_Init_FreeType( &ft_lib );
    if ( error ) return;
    FT_Face face;
    error = FT_New_Face( ft_lib, family->fontFilename, family->fontFileIndex, &face );
    if ( error ) return;

    for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
	QChar ch = sampleCharacter( (QFont::Script)i );
	if ( FT_Get_Char_Index ( face, ch.unicode() ) ) {
#ifdef QFONTDATABASE_DEBUG
	    qDebug("font can render char %04x, %04x script %d '%s'",
		   ch.unicode(), FT_Get_Char_Index ( face, ch.unicode() ),
		   i, QFontDatabase::scriptName( (QFont::Script)i ).latin1() );
#endif // QFONTDATABASE_DEBUG
	    family->scripts[i] = QtFontFamily::Supported;
	} else {
	    family->scripts[i] |= QtFontFamily::UnSupported_Xft;
	}
    }
    FT_Done_Face( face );
    FT_Done_FreeType( ft_lib );
    family->xftScriptCheck = TRUE;

}
#endif

static void load( const QString &family = QString::null, int script = -1 )
{
#ifdef QFONTDATABASE_DEBUG
    QTime t;
    t.start();
#endif // QFONTDATABASE_DEBUG

    if ( family.isNull() ) {
	if ( script == -1 )
	    loadXlfds( 0, -1 );
	else {
	    for ( int i = 0; i < numEncodings; i++ ) {
		if ( scripts_for_xlfd_encoding[i][script] )
		    loadXlfds( 0, i );
	    }
	}
    } else {
	QtFontFamily *f = db->family( family, TRUE );
	if ( !f->fullyLoaded ) {

#ifndef QT_NO_XFTFREETYPE
	    // need to check Xft coverage
	    if ( f->hasXft && !f->xftScriptCheck ) {
		checkXftCoverage( f );
	    }
#endif
	    // could reduce this further with some more magic:
	    // would need to remember the encodings loaded for the family.
	    if ( ( script == -1 && !f->xlfdLoaded ) ||
		 ( !f->hasXft && !(f->scripts[script] & QtFontFamily::Supported) &&
		   !(f->scripts[script] & QtFontFamily::UnSupported_Xlfd) ) ) {
		loadXlfds( family, -1 );
		f->fullyLoaded = TRUE;
	    }
	}
    }
#ifdef QFONTDATABASE_DEBUG
    qDebug("QFontDatabase: load( %s, %d) took %d ms", family.latin1(), script, t.elapsed() );
#endif // QFONTDATABASE_DEBUG
}


static void initializeDb()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    memset( encodingLoaded, FALSE, sizeof( encodingLoaded ) );

#ifdef QFONTDATABASE_DEBUG
    QTime t;
    t.start();
#endif // QFONTDATABASE_DEBUG

#ifndef QT_NO_XFTFREETYPE
    loadXft();
#endif

#ifdef QFONTDATABASE_DEBUG
    qDebug("QFontDatabase: loaded Xft: %d ms",  t.elapsed() );
    t.start();
#endif // QFONTDATABASE_DEBUG

#ifndef QT_NO_XFTFREETYPE
    for ( int i = 0; i < db->count; i++ ) {
	checkXftCoverage( db->families[i] );


#ifdef XFT_MATRIX
	for ( int j = 0; j < db->families[i]->count; ++j ) {	// each foundry
	    QtFontFoundry *foundry = db->families[i]->foundries[j];
	    for ( int k = 0; k < foundry->count; ++k ) {
		QtFontStyle *style = foundry->styles[k];
		if ( style->key.italic || style->key.oblique ) continue;

		QtFontSize *size = style->pixelSize( SMOOTH_SCALABLE );
		if ( ! size ) continue; // should not happen
		QtFontEncoding *enc = size->encodingID( -1, TRUE );
		if ( ! enc ) continue; // should not happen either

		QtFontStyle::Key key = style->key;

		// does this style have an italic equivalent?
		key.italic = TRUE;
		QtFontStyle *equiv = foundry->style( key );
		if ( equiv ) continue;

		// does this style have an oblique equivalent?
		key.italic = FALSE;
		key.oblique = TRUE;
		equiv = foundry->style( key );
		if ( equiv ) continue;

		// let's fake one...
		equiv = foundry->style( key, TRUE );
#ifndef QT_XFT2
		// Xft2 seems to do this automatically for us
		equiv->fakeOblique = TRUE;
#endif // !QT_XFT2
		equiv->smoothScalable = TRUE;

		QtFontSize *equiv_size = equiv->pixelSize( SMOOTH_SCALABLE, TRUE );
		QtFontEncoding *equiv_enc = equiv_size->encodingID( -1, TRUE );

		// keep the same pitch
		equiv_enc->pitch = enc->pitch;
	    }
	}
#endif // XFT_MATRIX
    }
#endif

#ifdef QFONTDATABASE_DEBUG
    qDebug("QFontDatabase: xft coverage check: %d ms",  t.elapsed() );
#endif // QFONTDATABASE_DEBUG

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for ( int f = 0; f < db->count; f++ ) {
	QtFontFamily *family = db->families[f];
	qDebug("'%s' %s  hasXft=%s", family->name.latin1(), (family->fixedPitch ? "fixed" : ""),
	       (family->hasXft ? "yes" : "no") );
	for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
	    qDebug( "\t%s: %s", QFontDatabase::scriptName( (QFont::Script)i ).latin1(),
		    ( (family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
		      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
		      "UnSupported" : "Unknown" ) );
	}

	for ( int fd = 0; fd < family->count; fd++ ) {
	    QtFontFoundry *foundry = family->foundries[fd];
	    qDebug("\t\t'%s'", foundry->name.latin1() );
	    for ( int s = 0; s < foundry->count; s++ ) {
		QtFontStyle *style = foundry->styles[s];
		qDebug("\t\t\tstyle: italic=%d oblique=%d weight=%d (%s)\n"
		       "\t\t\tstretch=%d (%s)",
		       style->key.italic, style->key.oblique, style->key.weight,
		       style->weightName, style->key.stretch,
		       style->setwidthName ? style->setwidthName : "nil" );
		if ( style->smoothScalable )
		    qDebug("\t\t\t\tsmooth scalable" );
		else if ( style->bitmapScalable )
		    qDebug("\t\t\t\tbitmap scalable" );
		if ( style->pixelSizes ) {
		    qDebug("\t\t\t\t%d pixel sizes",  style->count );
		    for ( int z = 0; z < style->count; ++z ) {
			QtFontSize *size = style->pixelSizes + z;
			for ( int e = 0; e < size->count; ++e ) {
			    qDebug( "\t\t\t\t  size %5d pitch %c encoding %s",
				    size->pixelSize,
				    size->encodings[e].pitch,
				    xlfd_for_id( size->encodings[e].encoding ) );
			}
		    }
		}
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG
}

void QFontDatabase::createDatabase()
{
    initializeDb();
}


// --------------------------------------------------------------------------------------
// font loader
// --------------------------------------------------------------------------------------
#define MAXFONTSIZE 128

static
QFontEngine *loadEngine( QFont::Script script,
			 const QFontPrivate *fp, const QFontDef &request,
			 QtFontFamily *family, QtFontFoundry *foundry,
			 QtFontStyle *style, QtFontSize *size,
			 QtFontEncoding *encoding )
{
#ifndef QT_NO_XFTFREETYPE
    if ( encoding->encoding == -1 ) {

#  ifdef FONT_MATCH_DEBUG
	qDebug( "    using Xft" );
#  endif // FONT_MATCH_DEBUG

	XftPattern *pattern = XftPatternCreate();
	if ( !pattern ) return 0;

#  ifndef QT_XFT2
	XftPatternAddString (pattern, XFT_ENCODING, "iso10646-1");
#  endif // QT_XFT2

	if ( !foundry->name.isEmpty() )
	    XftPatternAddString( pattern, XFT_FOUNDRY,
				 foundry->name.local8Bit().data() );

	if ( !family->name.isEmpty() )
	    XftPatternAddString( pattern, XFT_FAMILY,
				 family->name.local8Bit().data() );

	const char *stylehint_value = 0;
	switch ( request.styleHint ) {
	case QFont::SansSerif:
	default:
	    stylehint_value = "sans";
	    break;
	case QFont::Serif:
	    stylehint_value = "serif";
	    break;
	case QFont::TypeWriter:
	    stylehint_value = "mono";
	    break;
	}
	XftPatternAddString( pattern, XFT_FAMILY, stylehint_value );

	char pitch_value = ( encoding->pitch == 'c' ? XFT_CHARCELL :
			     ( encoding->pitch == 'm' ? XFT_MONO : XFT_PROPORTIONAL ) );
	XftPatternAddInteger( pattern, XFT_SPACING, pitch_value );

	int weight_value = XFT_WEIGHT_BLACK;
	if ( style->key.weight == 0 )
	    weight_value = XFT_WEIGHT_MEDIUM;
	else if ( style->key.weight < (QFont::Light + QFont::Normal) / 2 )
	    weight_value = XFT_WEIGHT_LIGHT;
	else if ( style->key.weight < (QFont::Normal + QFont::DemiBold) / 2 )
	    weight_value = XFT_WEIGHT_MEDIUM;
	else if ( style->key.weight < (QFont::DemiBold + QFont::Bold) / 2 )
	    weight_value = XFT_WEIGHT_DEMIBOLD;
	else if ( style->key.weight < (QFont::Bold + QFont::Black) / 2 )
	    weight_value = XFT_WEIGHT_BOLD;
	XftPatternAddInteger( pattern, XFT_WEIGHT, weight_value );

	int slant_value = XFT_SLANT_ROMAN;
	if ( style->key.italic )
	    slant_value = XFT_SLANT_ITALIC;
	else if ( style->key.oblique )
	    slant_value = XFT_SLANT_OBLIQUE;
	XftPatternAddInteger( pattern, XFT_SLANT, slant_value );

	/*
	  Xft1 doesn't obey user settings for turning off anti-aliasing using
	  the following:

	  match any size > 6 size < 12 edit antialias = false;

	  ... if we request pixel sizes.  so, work around this limitiation and
	  convert the pixel size to a point size and request that.
	*/
	double size_value = request.pixelSize;
	double scale = 1.;
	if ( size_value > MAXFONTSIZE ) {
	    scale = (double)size_value/(double)MAXFONTSIZE;
	    size_value = MAXFONTSIZE;
	}
	size_value *= 72.0 / QPaintDevice::x11AppDpiY( fp->screen );

	XftPatternAddDouble( pattern, XFT_SIZE, size_value );

#  ifdef XFT_MATRIX
	if ( ( request.stretch > 0 && request.stretch != 100 ) ||
	     ( style->key.oblique && style->fakeOblique ) ) {
	    XftMatrix matrix;
	    XftMatrixInit( &matrix );

	    if ( request.stretch > 0 && request.stretch != 100 )
		XftMatrixScale( &matrix, double( request.stretch ) / 100.0, 1.0 );
	    if ( style->key.oblique && style->fakeOblique )
		XftMatrixShear( &matrix, 0.20, 0.0 );

	    XftPatternAddMatrix( pattern, XFT_MATRIX, &matrix );
	}
#  endif // XFT_MATRIX

	extern bool qt_use_antialiasing; // defined in qfont_x11.cpp
	if ( !qt_use_antialiasing || request.styleStrategy & ( QFont::PreferAntialias |
							       QFont::NoAntialias) ) {
	    Bool requestAA = ( qt_use_antialiasing &&
			       !( request.styleStrategy & QFont::NoAntialias ) );
	    XftPatternAddBool( pattern, XFT_ANTIALIAS, requestAA );
	}

	XftResult res;
	XftPattern *result =
	    XftFontMatch( QPaintDevice::x11AppDisplay(), fp->screen, pattern, &res );
	XftPatternDestroy(pattern);

	// We pass a duplicate to XftFontOpenPattern because either xft font
	// will own the pattern after the call or the pattern will be
	// destroyed.
	XftPattern *dup = XftPatternDuplicate( result );
	XftFont *xftfs = XftFontOpenPattern( QPaintDevice::x11AppDisplay(), dup );

	if ( ! xftfs ) // Xft couldn't find a font?
	    return 0;

	QFontEngine *fe = new QFontEngineXft( xftfs, result, 0 );
	fe->setScale( scale );
	return fe;
    }
#endif // QT_NO_XFTFREETYPE

#ifdef FONT_MATCH_DEBUG
    qDebug( "    using XLFD" );
#endif // FONT_MATCH_DEBUG

    QCString xlfd = "-";
    xlfd += foundry->name.isEmpty() ? "*" : foundry->name.latin1();
    xlfd += "-";
    xlfd += family->name.isEmpty() ? "*" : family->name.latin1();

    xlfd += "-";
    xlfd += style->weightName ? style->weightName : "*";
    xlfd += "-";
    xlfd += ( style->key.italic ? "i" : ( style->key.oblique ? "o" : "r" ) );

    xlfd += "-";
    xlfd += style->setwidthName ? style->setwidthName : "*";
    // ### handle add-style
    xlfd += "-*-";

    int px = size->pixelSize;
    if ( style->smoothScalable )
	px = request.pixelSize;
    else if ( style->bitmapScalable && ( request.styleStrategy & QFont::PreferMatch ) )
	px = request.pixelSize;
    double scale = 1.;
    if ( px > MAXFONTSIZE ) {
	scale = (double)px/(double)MAXFONTSIZE;
	px = MAXFONTSIZE;
    }

    xlfd += QString::number( px ).latin1();

    xlfd += "-*-*-*-";
    // ### handle cell spaced fonts
    xlfd += encoding->pitch;
    xlfd += "-*-";
    xlfd += xlfd_for_id( encoding->encoding );

#ifdef FONT_MATCH_DEBUG
    qDebug( "    xlfd: '%s'", xlfd.data() );
#endif // FONT_MATCH_DEBUG

    XFontStruct *xfs;
    if (! (xfs = XLoadQueryFont(QPaintDevice::x11AppDisplay(), xlfd.data() ) ) )
	return 0;

    switch ( script ) {
    case QFont::Latin:
	// return new QFontEngineLatinXLFD( xfs, xlfd.data(),
	//                                  xlfd_for_id( encoding->encoding ), 0 );
	break;

    case QFont::Han:
	// return new QFontEngineHanXLFD( xfs, xlfd.data(),
	//                                xlfd_for_id( encoding->encoding ), 0 );
	break;

    default: break;
    }

    QFontEngine *fe = new QFontEngineXLFD( xfs, xlfd.data(), xlfd_for_id( encoding->encoding ), 0 );
    fe->setScale( scale );
    return fe;
}
