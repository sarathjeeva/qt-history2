/****************************************************************************
** $Id$
**
** Implementation of platform specific QFontDatabase
**
** Created : 970521
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#include "qt_x11.h"

#include <ctype.h>
#include <stdlib.h>

#define QFONTDATABASE_DEBUG

// ----- begin of generated code -----

struct XlfdEncoding {
    const char *name;
    int id;
};

static const int mib_for_xlfd_encoding[] = {
   0,
    5,
    6,
    7,
    110,
    111,
    8,
    2251,
    2084,
    2088,
    2084,
    10,
    82,
    85,
    -114,
    -113,
    -113,
    57,
    15,
    63,
    63,
    63,
    36,
    -2101,
    -2101,
    -2026,
    -2026,
    2028,
    2259,
    2259,
    -4242,
    0,
    0,
    0,
    0
};

static const char scripts_for_xlfd_encoding[35][50] = {
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0 }

};
enum
  {
    TOTAL_KEYWORDS = 35,
    MIN_WORD_LENGTH = 5,
    MAX_WORD_LENGTH = 15,
    MIN_HASH_VALUE = 5,
    MAX_HASH_VALUE = 133
  };

/* maximum key range = 129, duplicates = 0 */

static inline unsigned int
hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134,   0, 134, 134,   0,   0, 134,   0,   5,
       40,  30,  35,  20,  10,  25,   0,   0, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134,   0,   0,   0,
        0,   0,  35,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0, 134,  10,   0,  15,   0, 134, 134,
        0,   0, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
      134, 134, 134, 134, 134, 134
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

static const struct XlfdEncoding xlfd_encoding[] =
  {
    {"gbk-0", 16},
    {"koi8-u", 9},
    {"*-symbol", 33},
    {"unicode-*", 32},
    {"hkscs-1", 24},
    {"mulelao-1", 30},
    {"koi8-r", 10},
    {"koi8-ru", 8},
    {"tscii-*", 27},
    {"big5-0", 26},
    {"big5*-*", 25},
    {"iso8859-8", 13},
    {"big5hkscs-0", 23},
    {"iso8859-1", 0},
    {"iso8859-6", 12},
    {"iso8859-11", 29},
    {"gb18030-0", 14},
    {"iso8859-5", 6},
    {"iso8859-7", 11},
    {"iso8859-15", 5},
    {"jisx0201*-0", 18},
    {"iso8859-3", 2},
    {"jisx0208.1990-0", 20},
    {"iso8859-4", 3},
    {"iso8859-2", 1},
    {"iso8859-14", 4},
    {"tis620*-0", 28},
    {"iso10646-1", 31},
    {"*-cp1251", 7},
    {"ksc5601.1987-0", 22},
    {"jisx0208.1997-0", 19},
    {"gb18030.2000-0", 15},
    {"jisx0208.1983-0", 21},
    {"*-fontspecific", 34},
    {"gb2312.1980-0", 17}
  };

static const signed char lookup[] =
  {
    -1, -1, -1, -1, -1,  0,  1, -1,  2,  3, -1, -1,  4, -1,
     5, -1,  6,  7, -1, -1, -1, -1,  8, -1, -1, -1,  9, 10,
    -1, 11, -1, 12, -1, -1, 13, -1, -1, -1, -1, 14, 15, -1,
    -1, -1, 16, -1, -1, -1, -1, 17, -1, -1, -1, -1, 18, 19,
    20, -1, -1, 21, 22, -1, -1, -1, 23, -1, -1, -1, -1, 24,
    25, -1, -1, -1, 26, 27, -1, -1, 28, 29, -1, -1, -1, -1,
    -1, 30, -1, -1, -1, 31, 32, -1, -1, -1, -1, -1, -1, -1,
    -1, 33, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, 34
  };

const struct XlfdEncoding *
qt_xlfdEncoding_Id (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = xlfd_encoding[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &xlfd_encoding[index];
            }
        }
    }
  return 0;
}


// ----- end of generated code -----

int qt_mibForXlfd( const char * encoding )
{
    const XlfdEncoding *x = qt_xlfdEncoding_Id( encoding, strlen( encoding ) );
    if ( x )
	return mib_for_xlfd_encoding[x->id];
    return 0;
};

static const char * xlfd_for_id( int id )
{
    for ( int i = 0; i < TOTAL_KEYWORDS; i++ )
	if ( xlfd_encoding[i].id == id )
	    return xlfd_encoding[i].name;
    return 0;
}

static QtFontStyle::Key getStyle( char ** tokens )
{
    QtFontStyle::Key key;

    char slant0 = tolower( (uchar) tokens[QFontPrivate::Slant][0] );

    if ( slant0 == 'r' ) {
        if ( tokens[QFontPrivate::Slant][1]) {
            char slant1 = tolower( (uchar) tokens[QFontPrivate::Slant][1] );

            if ( slant1 == 'o' )
                key.oblique = TRUE;
            else if ( slant1 == 'i' )
		key.italic = TRUE;
        }
    } else if ( slant0 == 'o' )
	key.oblique = TRUE;
    else if ( slant0 == 'i' )
	key.italic = TRUE;

    key.weight = QFontPrivate::getFontWeight( tokens[QFontPrivate::Weight] );

    return key;
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
#endif // QT_NO_XFTFREETYPE


extern bool qt_has_xft; // defined in qfont_x11.cpp


static void loadXlfdEncoding( int encoding_id )
{
    int fontCount;

    // force the X server to give us XLFDs
    QCString xlfd_pattern = "-*-*-*-*-*-*-*-*-*-*-*-*-";
    xlfd_pattern += xlfd_for_id( encoding_id );
    char **fontList = XListFonts( QPaintDevice::x11AppDisplay(),
				  xlfd_pattern.data(),
				  0xffff, &fontCount );

    char *tokens[QFontPrivate::NFontFields];


    for( int i = 0 ; i < fontCount ; i++ ) {
	if ( !QFontPrivate::parseXFontName( fontList[i], tokens ) )
	    continue;

	const char *familyName = tokens[QFontPrivate::Family];
	const char *foundryName = tokens[QFontPrivate::Foundry];
	QtFontStyle::Key styleKey = getStyle( tokens );

	bool smooth_scalable = FALSE;
	bool bitmap_scalable = FALSE;
	if ( QFontPrivate::isScalable(tokens) ) {
	    if ( QFontPrivate::isSmoothlyScalable( tokens ) )
		smooth_scalable = TRUE;
	    else
		bitmap_scalable = TRUE;
	}
	int pixelSize = atoi( tokens[QFontPrivate::PixelSize] );
	bool fixedPitch = QFontPrivate::isFixedPitch( tokens );

	for ( int script = 0; script < QFont::NScripts + 1; script++ ) {
	    if ( !scripts_for_xlfd_encoding[encoding_id][script] )
		continue;

	    QtFontFamily *family = db->scripts[script].family( familyName, TRUE );
	    QtFontFoundry *foundry = family->foundry( foundryName,  TRUE );
	    QtFontStyle *style = foundry->style( styleKey,  TRUE );

	    if ( smooth_scalable ) {
		style->smoothScalable = TRUE;
		if ( style->pixelSizes ) {
		    free( style->pixelSizes );
		    style->pixelSizes = 0;
		    style->count = 0;
		}
	    }
	    if ( !style->smoothScalable ) {
		if ( bitmap_scalable )
		    style->bitmapScalable = TRUE;
		else {
		    style->pixelSize( pixelSize, TRUE );
		}
	    }
	    if ( !fixedPitch )
		family->fixedPitch = FALSE;
	}
    }

    XFreeFontNames( fontList );


}

#ifndef QT_NO_XFTFREETYPE
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

    fonts =
	XftListFonts(QPaintDevice::x11AppDisplay(),
		     QPaintDevice::x11AppScreen(),
		     (const char *)0,
		     XFT_FAMILY, XFT_WEIGHT, XFT_SLANT, XFT_SPACING,
		     (const char *)0);

    for (int i = 0; i < fonts->nfont; i++) {
	if (XftPatternGetString(fonts->fonts[i],
				XFT_FAMILY, 0, &value) != XftResultMatch )
	    continue;
	familyName = value;

	slant_value = XFT_SLANT_ROMAN;
	weight_value = XFT_WEIGHT_MEDIUM;
	spacing_value = XFT_PROPORTIONAL;
	XftPatternGetInteger (fonts->fonts[i], XFT_SLANT, 0, &slant_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_WEIGHT, 0, &weight_value);
	XftPatternGetInteger (fonts->fonts[i], XFT_SPACING, 0, &spacing_value);

	const int script = QFont::Unicode;

	QtFontStyle::Key styleKey;
	styleKey.italic = (slant_value == XFT_SLANT_ITALIC);
	styleKey.oblique = (slant_value == XFT_SLANT_OBLIQUE);
	styleKey.weight = getXftWeight( weight_value );

	QtFontFamily *family = db->scripts[script].family( familyName.lower(), TRUE );
	QtFontFoundry *foundry = family->foundry( QString( QChar(0xfffd) ),  TRUE );
	QtFontStyle *style = foundry->style( styleKey,  TRUE );

	style->smoothScalable = TRUE;
	family->fixedPitch = ( spacing_value < XFT_MONO );
    }

    XftFontSetDestroy (fonts);
}
#endif


#include <qdatetime.h>


void QFontDatabase::createDatabase()
{
    if ( db ) return;
    db = new QFontDatabasePrivate;

    QTime t;
    t.start();
    for ( int i = 0; i < TOTAL_KEYWORDS; ++i ) {
	loadXlfdEncoding( i );
    }
    qDebug("loading took %d ms",  t.elapsed() );

#ifndef QT_NO_XFTFREETYPE
    loadXft();
#endif
    qDebug("xft loading took %d ms",  t.elapsed() );

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for ( int i = 0; i < QFont::NScripts; i++ ) {
	QtFontScript &script = db->scripts[i];
	qDebug("Script %s", QFontDatabase::scriptName( (QFont::Script)i ).latin1() );
	if ( !script.count ) {
	    qDebug("    No fonts found!");
	    continue;
	}
	for ( int f = 0; f < script.count; f++ ) {
	    QtFontFamily *family = script.families[f];
	    qDebug("    %s", family->name.latin1() );

	    for ( int fd = 0; fd < family->count; fd++ ) {
		QtFontFoundry *foundry = family->foundries[fd];
		qDebug("        %s", foundry->name.latin1() );
		for ( int s = 0; s < foundry->count; s++ ) {
		    QtFontStyle *style = foundry->styles[s];
		    qDebug("            style: italic=%d oblique=%d weight=%d",  style->key.italic,
			   style->key.oblique, style->key.weight );
		    if ( style->smoothScalable )
			qDebug("                   smooth scalable" );
		    else if ( style->bitmapScalable )
			qDebug("                   bitmap scalable" );
		    if ( style->pixelSizes )
			qDebug("                   %d pixel sizes",  style->count );
		}
	    }
	}
    }
#endif // QFONTDATABASE_DEBUG

}
