/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "private/qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS

extern int qt_UnicodeToBig5hkscs(uint wc, uchar *r);


int QFontBig5hkscsCodec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


int QFontBig5hkscsCodec::heuristicNameMatch(const char* hint) const
{
    //qDebug("QFontBig5hkscsCodec::heuristicNameMatch(const char* hint = \"%s\")", hint);
    return ( qstricmp(hint, "big5hkscs-0") == 0 ||
	     qstricmp(hint, "hkscs-1") == 0 )
	? 13 : 0;
}


QFontBig5hkscsCodec::QFontBig5hkscsCodec()
{
    //qDebug("QFontBig5hkscsCodec::QFontBig5hkscsCodec()");
}


const char* QFontBig5hkscsCodec::name() const
{
    //qDebug("QFontBig5hkscsCodec::name() = \"big5hkscs-0\"");
    return "big5hkscs-0";
}


int QFontBig5hkscsCodec::mibEnum() const
{
    //qDebug("QFontBig5hkscsCodec::mibEnum() = -2101");
    return -2101;
}


QString QFontBig5hkscsCodec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}

unsigned short
QFontBig5hkscsCodec::characterFromUnicode(const QString &str, int pos) const
{
    uchar c[2];
    if (qt_UnicodeToBig5hkscs((str.unicode() + pos)->unicode(), c) == 2)
        return (c[0] << 8) + c[1];
    return 0;
}

QByteArray QFontBig5hkscsCodec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    //qDebug("QFontBig5hkscsCodec::fromUnicode(const QString& uc, int& lenInOut = %d)", lenInOut);
    QByteArray result;
    result.resize(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	uchar c[2];

#if 0
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
#endif
	if ( qt_UnicodeToBig5hkscs( ch.unicode(), c ) == 2) {
	    *rdata++ = c[0];
	    *rdata++ = c[1];
	} else {
	    //white square
	    *rdata++ = 0xa1;
	    *rdata++ = 0xbc;
	}
    }
    lenInOut *=2;
    return result;
}


/*! \internal */
void QFontBig5hkscsCodec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    uchar c[2];
    while (length--) {
	if ( in->row() == 0x00 && in->cell() < 0x80 ) {
	    // ASCII
	    *out = in->cell();
	} else if ( qt_UnicodeToBig5hkscs( in->unicode(), c ) == 2 ) {
	    // Big5-HKSCS
	    *out = (c[0] << 8) | c[1];
	} else {
	    // Unknown char
	    *out = 0;
	}

	++in;
	++out;
    }
}

bool QFontBig5hkscsCodec::canEncode( QChar ch ) const
{
    //qDebug("QFontBig5hkscsCodec::canEncode( QChar ch = %02X%02X )", ch.row(), ch.cell());
    uchar c[2];
    return ( qt_UnicodeToBig5hkscs( ch.unicode(), c ) == 2 );
}

#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
