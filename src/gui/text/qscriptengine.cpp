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

#include "qscriptengine_p.h"

#include "qstring.h"
#include "qrect.h"
#include "qfont.h"
#include <private/qunicodetables_p.h>
#include "qtextengine_p.h"
#include "qfontengine_p.h"
#include <stdlib.h>
#include <qvarlengtharray.h>
#ifdef QT_OPENTYPE
#include "qopentype_p.h"
#endif

#undef None
#undef Pre
#undef Above
#undef Below

// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Basic processing
//
// --------------------------------------------------------------------------------------------------------------------------------------------

static inline void positionCluster(QShaperItem *item, int gfrom,  int glast )
{
    int nmarks = glast - gfrom;
    if ( nmarks <= 0 ) {
	qWarning( "positionCluster: no marks to position!" );
	return;
    }

    QGlyphLayout *glyphs = item->glyphs;
    QFontEngine *f = item->font;

    glyph_metrics_t baseInfo = f->boundingBox( glyphs[gfrom].glyph );

    if ( item->script == QFont::Hebrew )
	// we need to attach below the baseline, because of the hebrew iud.
	baseInfo.height = qMax( baseInfo.height, -baseInfo.y );

    QRect baseRect( baseInfo.x.value(), baseInfo.y.value(), baseInfo.width.value(), baseInfo.height.value() );

//     qDebug("---> positionCluster: cluster from %d to %d", gfrom, glast );
//     qDebug( "baseInfo: %d/%d (%d/%d) off=%d/%d", baseInfo.x, baseInfo.y, baseInfo.width, baseInfo.height, baseInfo.xoff, baseInfo.yoff );

    int size = f->ascent().value()/10;
    int offsetBase = (size - 4) / 4 + qMin( size, 4 ) + 1;
//     qDebug("offset = %d", offsetBase );

    bool rightToLeft = item->flags & QTextEngine::RightToLeft;

    int i;
    unsigned char lastCmb = 0;
    QRect attachmentRect;

    for( i = 1; i <= nmarks; i++ ) {
	glyph_t mark = glyphs[gfrom+i].glyph;
	QPoint p;
	glyph_metrics_t markInfo = f->boundingBox( mark );
	QRect markRect( markInfo.x.value(), markInfo.y.value(), markInfo.width.value(), markInfo.height.value() );

	int offset = offsetBase;
	unsigned char cmb = glyphs[gfrom+i].attributes.combiningClass;

	// ### maybe the whole position determination should move down to heuristicSetGlyphAttributes. Would save some
	// bits  in the glyphAttributes structure.
	if ( cmb < 200 ) {
	    // fixed position classes. We approximate by mapping to one of the others.
	    // currently I added only the ones for arabic, hebrew, lao and thai.

	    // for Lao and Thai marks with class 0, see below ( heuristicSetGlyphAttributes )

	    // add a bit more offset to arabic, a bit hacky
	    if ( cmb >= 27 && cmb <= 36 && offset < 3 )
		offset +=1;
 	    // below
	    if ( (cmb >= 10 && cmb <= 18) ||
		 cmb == 20 || cmb == 22 ||
		 cmb == 29 || cmb == 32 )
		cmb = QChar::Combining_Below;
	    // above
	    else if ( cmb == 23 || cmb == 27 || cmb == 28 ||
		      cmb == 30 || cmb == 31 || (cmb >= 33 && cmb <= 36 ) )
		cmb = QChar::Combining_Above;
	    //below-right
	    else if ( cmb == 9 || cmb == 103 || cmb == 118 )
		cmb = QChar::Combining_BelowRight;
	    // above-right
	    else if ( cmb == 24 || cmb == 107 || cmb == 122 )
		cmb = QChar::Combining_AboveRight;
	    else if ( cmb == 25 )
		cmb = QChar::Combining_AboveLeft;
	    // fixed:
	    //  19 21

	}

	// combining marks of different class don't interact. Reset the rectangle.
	if ( cmb != lastCmb ) {
	    //qDebug( "resetting rect" );
	    attachmentRect = baseRect;
	}

	switch( cmb ) {
	case QChar::Combining_DoubleBelow:
		// ### wrong in rtl context!
	case QChar::Combining_BelowLeft:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowLeftAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    break;
	case QChar::Combining_Below:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowAttached:
	    p += attachmentRect.bottomLeft() - markRect.topLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_BelowRight:
	    p += QPoint( 0, offset );
	case QChar::Combining_BelowRightAttached:
	    p += attachmentRect.bottomRight() - markRect.topRight();
	    break;
	    case QChar::Combining_Left:
	    p += QPoint( -offset, 0 );
	case QChar::Combining_LeftAttached:
	    break;
	    case QChar::Combining_Right:
	    p += QPoint( offset, 0 );
	case QChar::Combining_RightAttached:
	    break;
	case QChar::Combining_DoubleAbove:
	    // ### wrong in RTL context!
	case QChar::Combining_AboveLeft:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveLeftAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    break;
	    case QChar::Combining_Above:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveAttached:
	    p += attachmentRect.topLeft() - markRect.bottomLeft();
	    p += QPoint( (attachmentRect.width() - markRect.width())/2 , 0 );
	    break;
	    case QChar::Combining_AboveRight:
	    p += QPoint( 0, -offset );
	case QChar::Combining_AboveRightAttached:
	    p += attachmentRect.topRight() - markRect.bottomRight();
	    break;

	case QChar::Combining_IotaSubscript:
	    default:
		break;
	}
// 	qDebug( "char=%x combiningClass = %d offset=%d/%d", mark, cmb, p.x(), p.y() );
	markRect.moveBy( p.x(), p.y() );
	attachmentRect |= markRect;
	lastCmb = cmb;
	if ( rightToLeft ) {
	    glyphs[gfrom+i].offset.x = Q26Dot6(p.x(), F26Dot6);
	    glyphs[gfrom+i].offset.y = Q26Dot6(p.y(), F26Dot6) - baseInfo.yoff;
	} else {
	    glyphs[gfrom+i].offset.x = Q26Dot6(p.x(), F26Dot6) - baseInfo.xoff;
	    glyphs[gfrom+i].offset.y = Q26Dot6(p.y(), F26Dot6) - baseInfo.yoff;
	}
    }
}


void q_heuristicPosition(QShaperItem *item)
{
    QGlyphLayout *glyphs = item->glyphs;

    int cEnd = -1;
    int i = item->num_glyphs;
    while ( i-- ) {
	if ( cEnd == -1 && glyphs[i].attributes.mark ) {
	    cEnd = i;
	} else if ( cEnd != -1 && !glyphs[i].attributes.mark ) {
	    positionCluster(item, i, cEnd);
	    cEnd = -1;
	}
    }
}



// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars and glyphs
// and no reordering.
// also computes logClusters heuristically
static void heuristicSetGlyphAttributes(QShaperItem *item)
{
    // ### zeroWidth and justification are missing here!!!!!

    Q_ASSERT(item->num_glyphs == item->length);

//     qDebug("QScriptEngine::heuristicSetGlyphAttributes, num_glyphs=%d", num_glyphs);
    QGlyphLayout *glyphs = item->glyphs;
    unsigned short *logClusters = item->log_clusters;

    // honour the logClusters array if it exists.
    const QChar *uc = item->string->unicode() + item->from;

    for ( int i = 0; i < item->num_glyphs; i++ )
	logClusters[i] = i;

    // first char in a run is never (treated as) a mark
    int cStart = 0;
    glyphs[0].attributes.mark = FALSE;
    glyphs[0].attributes.clusterStart = TRUE;

    int pos = 1;
    QChar::Category lastCat = ::category(uc[0]);
    while (pos < item->length) {
	QChar::Category cat = ::category(uc[pos]);
	if (cat != QChar::Mark_NonSpacing) {
	    glyphs[pos].attributes.mark = FALSE;
	    glyphs[pos].attributes.clusterStart = TRUE;
	    glyphs[pos].attributes.combiningClass = 0;
	    cStart = pos;
	} else {
	    int cmb = combiningClass( uc[pos] );

	    if ( cmb == 0 ) {
		// Fix 0 combining classes
		if ( uc[pos].row() == 0x0e ) {
		    // thai or lao
		    unsigned char col = uc[pos].cell();
		    if ( col == 0x31 ||
			 col == 0x34 ||
			 col == 0x35 ||
			 col == 0x36 ||
			 col == 0x37 ||
			 col == 0x47 ||
			 col == 0x4c ||
			 col == 0x4d ||
			 col == 0x4e ) {
			cmb = QChar::Combining_AboveRight;
		    } else if ( col == 0xb1 ||
				col == 0xb4 ||
				col == 0xb5 ||
				col == 0xb6 ||
				col == 0xb7 ||
				col == 0xbb ||
				col == 0xcc ||
				col == 0xcd ) {
			cmb = QChar::Combining_Above;
		    } else if ( col == 0xbc ) {
			cmb = QChar::Combining_Below;
		    }
		}
	    }

	    glyphs[pos].attributes.mark = TRUE;
	    glyphs[pos].attributes.clusterStart = FALSE;
	    glyphs[pos].attributes.combiningClass = cmb;
	    // 		qDebug("found a mark at position %d", pos );
	    logClusters[pos] = cStart;
	    glyphs[pos].advance.x = 0;
	    glyphs[pos].advance.y = 0;
	}

	if (lastCat == QChar::Separator_Space)
	    glyphs[pos-1].attributes.justification = QGlyphLayout::Space;
	else if (cat != QChar::Mark_NonSpacing)
	    glyphs[pos-1].attributes.justification = QGlyphLayout::Character;
	else
	    glyphs[pos-1].attributes.justification = QGlyphLayout::NoJustification;

	lastCat = cat;
	pos++;
    }
    if (lastCat == QChar::Separator_Space)
	glyphs[pos-1].attributes.justification = QGlyphLayout::Space;
    else
	glyphs[pos-1].attributes.justification = QGlyphLayout::Character;
}

static bool basic_shape(QShaperItem *item)
{
    if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
	return false;

    heuristicSetGlyphAttributes(item);
    if (!(item->flags & QTextEngine::WidthOnly))
	q_heuristicPosition(item);
    return true;
}

static void basic_attributes( int /*script*/, const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    attributes += from;

    QCharAttributes *a = attributes;

    for ( int i = 0; i < len; i++ ) {
	QChar::Category cat = ::category( *uc );
	a->whiteSpace = (cat == QChar::Separator_Space) && (uc->unicode() != 0xa0);
	a->softBreak = FALSE;
	a->charStop = (cat != QChar::Mark_NonSpacing);
	a->wordStop = FALSE;
	a->invalid = FALSE;
	++uc;
	++a;
    }
}



// --------------------------------------------------------------------------------------------------------------------------------------------
//
// Middle eastern languages
//
// --------------------------------------------------------------------------------------------------------------------------------------------


// these groups correspond to the groups defined in the Unicode standard.
// Some of these groups are equal whith regards to both joining and line breaking behaviour,
// and thus have the same enum value
//
// I'm not sure the mapping of syriac to arabic enums is correct with regards to justification, but as
// I couldn't find any better document I'll hope for the best.
enum ArabicGroup {
    // NonJoining
    ArabicNone,
    ArabicSpace,
    // Transparent
    Transparent,
    // Causing
    Center,
    Kashida,

    // Arabic
    // Dual
    Beh,
    Noon,
    Meem = Noon,
    Heh = Noon,
    KnottedHeh = Noon,
    HehGoal = Noon,
    SwashKaf = Noon,
    Yeh,
    Hah,
    Seen,
    Sad = Seen,
    Tah,
    Kaf = Tah,
    Gaf = Tah,
    Lam = Tah,
    Ain,
    Feh = Ain,
    Qaf = Ain,
    // Right
    Alef,
    Waw,
    Dal,
    TehMarbuta = Dal,
    Reh,
    HamzaOnHehGoal,
    YehWithTail = HamzaOnHehGoal,
    YehBarre = HamzaOnHehGoal,

    // Syriac
    // Dual
    Beth = Beh,
    Gamal = Ain,
    Heth = Noon,
    Teth = Hah,
    Yudh = Noon,
    Kaph = Noon,
    Lamadh = Lam,
    Mim = Noon,
    Nun = Noon,
    Semakh = Noon,
    FinalSemakh = Noon,
    SyriacE = Ain,
    Pe = Ain,
    ReversedPe = Hah,
    Qaph = Noon,
    Shin = Noon,
    Fe = Ain,

    // Right
    Alaph = Alef,
    Dalath = Dal,
    He = Dal,
    SyriacWaw = Waw,
    Zain = Alef,
    YudhHe = Waw,
    Sadhe = HamzaOnHehGoal,
    Taw = Dal,

    // Compiler bug? Otherwise ArabicGroupsEnd would be equal to Dal + 1.
    Dummy = HamzaOnHehGoal,
    ArabicGroupsEnd
};

static const unsigned char arabic_group[0x150] = {
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,

    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,

    ArabicNone, ArabicNone, Alef, Alef,
    Waw, Alef, Yeh, Alef,
    Beh, TehMarbuta, Beh, Beh,
    Hah, Hah, Hah, Dal,

    Dal, Reh, Reh, Seen,
    Seen, Sad, Sad, Tah,
    Tah, Ain, Ain, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,

    // 0x640
    Kashida, Feh, Qaf, Kaf,
    Lam, Meem, Noon, Heh,
    Waw, Yeh, Yeh, Transparent,
    Transparent, Transparent, Transparent, Transparent,

    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,
    Transparent, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,

    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, Beh, Qaf,

    Transparent, Alef, Alef, Alef,
    ArabicNone, Alef, Waw, Waw,
    Yeh, Beh, Beh, Beh,
    Beh, Beh, Beh, Beh,

    // 0x680
    Beh, Hah, Hah, Hah,
    Hah, Hah, Hah, Hah,
    Dal, Dal, Dal, Dal,
    Dal, Dal, Dal, Dal,

    Dal, Reh, Reh, Reh,
    Reh, Reh, Reh, Reh,
    Reh, Reh, Seen, Seen,
    Seen, Sad, Sad, Tah,

    Ain, Feh, Feh, Feh,
    Feh, Feh, Feh, Qaf,
    Qaf, Gaf, SwashKaf, Gaf,
    Kaf, Kaf, Kaf, Gaf,

    Gaf, Gaf, Gaf, Gaf,
    Gaf, Lam, Lam, Lam,
    Lam, Noon, Noon, Noon,
    Noon, Noon, KnottedHeh, Hah,

    // 0x6c0
    TehMarbuta, HehGoal, HamzaOnHehGoal, HamzaOnHehGoal,
    Waw, Waw, Waw, Waw,
    Waw, Waw, Waw, Waw,
    Yeh, YehWithTail, Yeh, Waw,

    Yeh, Yeh, YehBarre, YehBarre,
    ArabicNone, TehMarbuta, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,
    Transparent, ArabicNone, ArabicNone, Transparent,

    Transparent, Transparent, Transparent, Transparent,
    Transparent, ArabicNone, ArabicNone, Transparent,
    Transparent, ArabicNone, Transparent, Transparent,
    Transparent, Transparent, Dal, Reh,

    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, Seen, Sad,
    Ain, ArabicNone, ArabicNone, KnottedHeh,

    // 0x700
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,
    ArabicNone, ArabicNone, ArabicNone, ArabicNone,

    Alaph, Transparent, Beth, Gamal,
    Gamal, Dalath, Dalath, He,
    SyriacWaw, Zain, Heth, Teth,
    Teth, Yudh, YudhHe, Kaph,

    Lamadh, Mim, Nun, Semakh,
    FinalSemakh, SyriacE, Pe, ReversedPe,
    Sadhe, Qaph, Dalath, Shin,
    Taw, Beth, Gamal, Dalath,

    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,

    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, Transparent,
    Transparent, Transparent, Transparent, ArabicNone,
    ArabicNone, Zain, Kaph, Fe,
};

static inline ArabicGroup arabicGroup(unsigned short uc)
{
    if (uc >= 0x0600 && uc < 0x750)
	return (ArabicGroup) arabic_group[uc-0x600];
    else if (uc == 0x200d)
	return Center;
    else if (::category(uc) == QChar::Separator_Space)
	return ArabicSpace;
    else
	return ArabicNone;
}


/*
   Arabic shaping obeys a number of rules according to the joining classes (see Unicode book, section on
   arabic).

   Each unicode char has a joining class (right, dual (left&right), center (joincausing) or transparent).
   transparent joining is not encoded in QChar::joining(), but applies to all combining marks and format marks.

   Right join-causing: dual + center
   Left join-causing: dual + right + center

   Rules are as follows (for a string already in visual order, as we have it here):

   R1 Transparent characters do not affect joining behaviour.
   R2 A right joining character, that has a right join-causing char on the right will get form XRight
   (R3 A left joining character, that has a left join-causing char on the left will get form XLeft)
   Note: the above rule is meaningless, as there are no pure left joining characters defined in Unicode
   R4 A dual joining character, that has a left join-causing char on the left and a right join-causing char on
	     the right will get form XMedial
   R5  A dual joining character, that has a right join causing char on the right, and no left join causing char on the left
	 will get form XRight
   R6 A dual joining character, that has a  left join causing char on the left, and no right join causing char on the right
	 will get form XLeft
   R7 Otherwise the character will get form XIsolated

   Additionally we have to do the minimal ligature support for lam-alef ligatures:

   L1 Transparent characters do not affect ligature behaviour.
   L2 Any sequence of Alef(XRight) + Lam(XMedial) will form the ligature Alef.Lam(XLeft)
   L3 Any sequence of Alef(XRight) + Lam(XLeft) will form the ligature Alef.Lam(XIsolated)

   The state table below handles rules R1-R7.
*/

enum Shape {
    XFinal,
    XMedial,
    XInitial,
    XIsolated,
    // intermediate state
    XCausing
};


enum Joining {
    JNone,
    JCausing,
    JDual,
    JRight,
    JTransparent
};


static const Joining joining_for_group[ArabicGroupsEnd] = {
    // NonJoining
    JNone, // ArabicNone
    JNone, // ArabicSpace
    // Transparent
    JTransparent, // Transparent
    // Causing
    JCausing, // Center
    JCausing, // Kashida
    // Dual
    JDual, // Beh
    JDual, // Noon
    JDual, // Yeh
    JDual, // Hah
    JDual, // Seen
    JDual, // Tah
    JDual, // Ain
    // Right
    JRight, // Alef
    JRight, // Waw
    JRight, // Dal
    JRight, // Reh
    JRight  // HamzaOnHehGoal
};


struct JoiningPair {
    Shape form1;
    Shape form2;
};

static const JoiningPair joining_table[5][4] =
// None, Causing, Dual, Right
{
    { { XFinal, XIsolated }, { XFinal, XCausing }, { XFinal, XInitial }, { XFinal, XIsolated } }, // XFinal
    { { XFinal, XIsolated }, { XMedial, XCausing }, { XMedial, XMedial }, { XMedial, XFinal } }, // XMedial
    { { XIsolated, XIsolated }, { XInitial, XCausing }, { XInitial, XMedial }, { XInitial, XFinal } }, // XInitial
    { { XIsolated, XIsolated }, { XIsolated, XCausing }, { XIsolated, XInitial }, { XIsolated, XIsolated } }, // XIsolated
    { { XIsolated, XIsolated }, { XIsolated, XCausing }, { XIsolated, XMedial }, { XIsolated, XFinal } }, // XCausing
};


/*
According to http://www.microsoft.com/middleeast/Arabicdev/IE6/KBase.asp

1. Find the priority of the connecting opportunities in each word
2. Add expansion at the highest priority connection opportunity
3. If more than one connection opportunity have the same highest value,
   use the opportunity closest to the end of the word.

Following is a chart that provides the priority for connection
opportunities and where expansion occurs. The character group names
are those in table 6.6 of the UNICODE 2.0 book.


PrioritY	Glyph                   Condition                                       Kashida Location

Arabic_Kashida	User inserted Kashida   The user entered a Kashida in a position.       After the user
		(Shift+j or Shift+�)    Thus, it is the highest priority to insert an   inserted kashida
					automatic kashida.

Arabic_Seen	Seen, Sad               Connecting to the next character.               After the character.
					(Initial or medial form).

Arabic_HaaDal	Teh Marbutah, Haa, Dal  Connecting to previous character.               Before the final form
											of these characters.

Arabic_Alef     Alef, Tah, Lam,         Connecting to previous character.               Before the final form
		Kaf and Gaf                                                             of these characters.

Arabic_BaRa     Reh, Yeh                Connected to medial Beh                         Before preceding medial Baa

Arabic_Waw	Waw, Ain, Qaf, Feh      Connecting to previous character.               Before the final form of
											these characters.

Arabic_Normal   Other connecting        Connecting to previous character.               Before the final form
		characters                                                              of these characters.



This seems to imply that we have at most one kashida point per arabic word.

*/

struct ArabicProperties {
    unsigned char shape;
    unsigned char justification;
};
Q_DECLARE_TYPEINFO(ArabicProperties, Q_PRIMITIVE_TYPE);


static void getArabicProperties(const unsigned short *chars, int len, ArabicProperties *properties)
{
//     qDebug("arabicSyriacOpenTypeShape: properties:");
    int lastPos = 0;
    int lastGroup = ArabicNone;

    ArabicGroup group = arabicGroup(chars[0]);
    Joining j = joining_for_group[group];
    Shape shape = joining_table[XIsolated][j].form2;
    properties[0].justification = QGlyphLayout::NoJustification;

    for (int i = 1; i < len; ++i) {
	// #### fix handling for spaces and punktuation
	properties[i].justification = QGlyphLayout::NoJustification;

	group = arabicGroup(chars[i]);
	j = joining_for_group[group];

	if (j == JTransparent) {
	    properties[i].shape = XIsolated;
	    continue;
	}

	properties[lastPos].shape = joining_table[shape][j].form1;
	shape = joining_table[shape][j].form2;

	switch(lastGroup) {
	case Seen:
	    if (properties[lastPos].shape == XInitial || properties[lastPos].shape == XMedial)
		properties[i-1].justification = QGlyphLayout::Arabic_Seen;
	    break;
	case Hah:
	    if (properties[lastPos].shape == XFinal)
		properties[lastPos-1].justification = QGlyphLayout::Arabic_HaaDal;
	    break;
	case Alef:
	    if (properties[lastPos].shape == XFinal)
		properties[lastPos-1].justification = QGlyphLayout::Arabic_Alef;
	    break;
	case Ain:
	    if (properties[lastPos].shape == XFinal)
		properties[lastPos-1].justification = QGlyphLayout::Arabic_Waw;
	    break;
	case Noon:
	    if (properties[lastPos].shape == XFinal)
		properties[lastPos-1].justification = QGlyphLayout::Arabic_Normal;
	    break;
	case ArabicNone:
	    break;

	default:
	    Q_ASSERT(false);
	}

	lastGroup = ArabicNone;

	switch(group) {
	case ArabicNone:
	case Transparent:
	// ### Center should probably be treated as transparent when it comes to justification.
	case Center:
	    break;
	case ArabicSpace:
	    properties[i].justification = QGlyphLayout::Arabic_Space;

	case Kashida:
	    properties[i].justification = QGlyphLayout::Arabic_Kashida;
	    break;
	case Seen:
	    lastGroup = Seen;
	    break;

	case Hah:
	case Dal:
	    lastGroup = Hah;
	    break;

	case Alef:
	case Tah:
	    lastGroup = Alef;
	    break;

	case Yeh:
	case Reh:
	    if (properties[lastPos].shape == XMedial && arabicGroup(chars[lastPos]) == Beh)
		properties[lastPos-1].justification = QGlyphLayout::Arabic_BaRa;
	    break;

	case Ain:
	case Waw:
	    lastGroup = Ain;
	    break;

	case Noon:
	case Beh:
	case HamzaOnHehGoal:
	    lastGroup = Noon;
	    break;
	case ArabicGroupsEnd:
	    Q_ASSERT(false);
	}

	lastPos = i;
    }
    properties[lastPos].shape = joining_table[shape][JNone].form1;


//     for (int i = 0; i < len; ++i)
// 	qDebug("arabic properties(%d): uc=%x shape=%d, justification=%d", i, chars[i], properties[i].shape, properties[i].justification);
}






// The unicode to unicode shaping codec.
// does only presentation forms B at the moment, but that should be enough for
// simple display
static const ushort arabicUnicodeMapping[256][2] = {
    // base of shaped forms, and number-1 of them ( 0 for non shaping,
    // 1 for right binding and 3 for dual binding

    // These are just the glyphs available in Unicode,
    // some characters are in R class, but have no glyphs in Unicode.

    { 0x0600, 0 }, // 0x0600
    { 0x0601, 0 }, // 0x0601
    { 0x0602, 0 }, // 0x0602
    { 0x0603, 0 }, // 0x0603
    { 0x0604, 0 }, // 0x0604
    { 0x0605, 0 }, // 0x0605
    { 0x0606, 0 }, // 0x0606
    { 0x0607, 0 }, // 0x0607
    { 0x0608, 0 }, // 0x0608
    { 0x0609, 0 }, // 0x0609
    { 0x060A, 0 }, // 0x060A
    { 0x060B, 0 }, // 0x060B
    { 0x060C, 0 }, // 0x060C
    { 0x060D, 0 }, // 0x060D
    { 0x060E, 0 }, // 0x060E
    { 0x060F, 0 }, // 0x060F

    { 0x0610, 0 }, // 0x0610
    { 0x0611, 0 }, // 0x0611
    { 0x0612, 0 }, // 0x0612
    { 0x0613, 0 }, // 0x0613
    { 0x0614, 0 }, // 0x0614
    { 0x0615, 0 }, // 0x0615
    { 0x0616, 0 }, // 0x0616
    { 0x0617, 0 }, // 0x0617
    { 0x0618, 0 }, // 0x0618
    { 0x0619, 0 }, // 0x0619
    { 0x061A, 0 }, // 0x061A
    { 0x061B, 0 }, // 0x061B
    { 0x061C, 0 }, // 0x061C
    { 0x061D, 0 }, // 0x061D
    { 0x061E, 0 }, // 0x061E
    { 0x061F, 0 }, // 0x061F

    { 0x0620, 0 }, // 0x0620
    { 0xFE80, 0 }, // 0x0621            HAMZA
    { 0xFE81, 1 }, // 0x0622    R       ALEF WITH MADDA ABOVE
    { 0xFE83, 1 }, // 0x0623    R       ALEF WITH HAMZA ABOVE
    { 0xFE85, 1 }, // 0x0624    R       WAW WITH HAMZA ABOVE
    { 0xFE87, 1 }, // 0x0625    R       ALEF WITH HAMZA BELOW
    { 0xFE89, 3 }, // 0x0626    D       YEH WITH HAMZA ABOVE
    { 0xFE8D, 1 }, // 0x0627    R       ALEF
    { 0xFE8F, 3 }, // 0x0628    D       BEH
    { 0xFE93, 1 }, // 0x0629    R       TEH MARBUTA
    { 0xFE95, 3 }, // 0x062A    D       TEH
    { 0xFE99, 3 }, // 0x062B    D       THEH
    { 0xFE9D, 3 }, // 0x062C    D       JEEM
    { 0xFEA1, 3 }, // 0x062D    D       HAH
    { 0xFEA5, 3 }, // 0x062E    D       KHAH
    { 0xFEA9, 1 }, // 0x062F    R       DAL

    { 0xFEAB, 1 }, // 0x0630    R       THAL
    { 0xFEAD, 1 }, // 0x0631    R       REH
    { 0xFEAF, 1 }, // 0x0632    R       ZAIN
    { 0xFEB1, 3 }, // 0x0633    D       SEEN
    { 0xFEB5, 3 }, // 0x0634    D       SHEEN
    { 0xFEB9, 3 }, // 0x0635    D       SAD
    { 0xFEBD, 3 }, // 0x0636    D       DAD
    { 0xFEC1, 3 }, // 0x0637    D       TAH
    { 0xFEC5, 3 }, // 0x0638    D       ZAH
    { 0xFEC9, 3 }, // 0x0639    D       AIN
    { 0xFECD, 3 }, // 0x063A    D       GHAIN
    { 0x063B, 0 }, // 0x063B
    { 0x063C, 0 }, // 0x063C
    { 0x063D, 0 }, // 0x063D
    { 0x063E, 0 }, // 0x063E
    { 0x063F, 0 }, // 0x063F

    { 0x0640, 0 }, // 0x0640    C       TATWEEL // ### Join Causing, only one glyph
    { 0xFED1, 3 }, // 0x0641    D       FEH
    { 0xFED5, 3 }, // 0x0642    D       QAF
    { 0xFED9, 3 }, // 0x0643    D       KAF
    { 0xFEDD, 3 }, // 0x0644    D       LAM
    { 0xFEE1, 3 }, // 0x0645    D       MEEM
    { 0xFEE5, 3 }, // 0x0646    D       NOON
    { 0xFEE9, 3 }, // 0x0647    D       HEH
    { 0xFEED, 1 }, // 0x0648    R       WAW
    { 0x0649, 0 }, // 0x0649            ALEF MAKSURA // ### Dual, glyphs not consecutive, handle in code.
    { 0xFEF1, 3 }, // 0x064A    D       YEH
    { 0x064B, 0 }, // 0x064B
    { 0x064C, 0 }, // 0x064C
    { 0x064D, 0 }, // 0x064D
    { 0x064E, 0 }, // 0x064E
    { 0x064F, 0 }, // 0x064F

    { 0x0650, 0 }, // 0x0650
    { 0x0651, 0 }, // 0x0651
    { 0x0652, 0 }, // 0x0652
    { 0x0653, 0 }, // 0x0653
    { 0x0654, 0 }, // 0x0654
    { 0x0655, 0 }, // 0x0655
    { 0x0656, 0 }, // 0x0656
    { 0x0657, 0 }, // 0x0657
    { 0x0658, 0 }, // 0x0658
    { 0x0659, 0 }, // 0x0659
    { 0x065A, 0 }, // 0x065A
    { 0x065B, 0 }, // 0x065B
    { 0x065C, 0 }, // 0x065C
    { 0x065D, 0 }, // 0x065D
    { 0x065E, 0 }, // 0x065E
    { 0x065F, 0 }, // 0x065F

    { 0x0660, 0 }, // 0x0660
    { 0x0661, 0 }, // 0x0661
    { 0x0662, 0 }, // 0x0662
    { 0x0663, 0 }, // 0x0663
    { 0x0664, 0 }, // 0x0664
    { 0x0665, 0 }, // 0x0665
    { 0x0666, 0 }, // 0x0666
    { 0x0667, 0 }, // 0x0667
    { 0x0668, 0 }, // 0x0668
    { 0x0669, 0 }, // 0x0669
    { 0x066A, 0 }, // 0x066A
    { 0x066B, 0 }, // 0x066B
    { 0x066C, 0 }, // 0x066C
    { 0x066D, 0 }, // 0x066D
    { 0x066E, 0 }, // 0x066E
    { 0x066F, 0 }, // 0x066F

    { 0x0670, 0 }, // 0x0670
    { 0xFB50, 1 }, // 0x0671    R       ALEF WASLA
    { 0x0672, 0 }, // 0x0672
    { 0x0673, 0 }, // 0x0673
    { 0x0674, 0 }, // 0x0674
    { 0x0675, 0 }, // 0x0675
    { 0x0676, 0 }, // 0x0676
    { 0x0677, 0 }, // 0x0677
    { 0x0678, 0 }, // 0x0678
    { 0xFB66, 3 }, // 0x0679    D       TTEH
    { 0xFB5E, 3 }, // 0x067A    D       TTEHEH
    { 0xFB52, 3 }, // 0x067B    D       BEEH
    { 0x067C, 0 }, // 0x067C
    { 0x067D, 0 }, // 0x067D
    { 0xFB56, 3 }, // 0x067E    D       PEH
    { 0xFB62, 3 }, // 0x067F    D       TEHEH

    { 0xFB5A, 3 }, // 0x0680    D       BEHEH
    { 0x0681, 0 }, // 0x0681
    { 0x0682, 0 }, // 0x0682
    { 0xFB76, 3 }, // 0x0683    D       NYEH
    { 0xFB72, 3 }, // 0x0684    D       DYEH
    { 0x0685, 0 }, // 0x0685
    { 0xFB7A, 3 }, // 0x0686    D       TCHEH
    { 0xFB7E, 3 }, // 0x0687    D       TCHEHEH
    { 0xFB88, 1 }, // 0x0688    R       DDAL
    { 0x0689, 0 }, // 0x0689
    { 0x068A, 0 }, // 0x068A
    { 0x068B, 0 }, // 0x068B
    { 0xFB84, 1 }, // 0x068C    R       DAHAL
    { 0xFB82, 1 }, // 0x068D    R       DDAHAL
    { 0xFB86, 1 }, // 0x068E    R       DUL
    { 0x068F, 0 }, // 0x068F

    { 0x0690, 0 }, // 0x0690
    { 0xFB8C, 1 }, // 0x0691    R       RREH
    { 0x0692, 0 }, // 0x0692
    { 0x0693, 0 }, // 0x0693
    { 0x0694, 0 }, // 0x0694
    { 0x0695, 0 }, // 0x0695
    { 0x0696, 0 }, // 0x0696
    { 0x0697, 0 }, // 0x0697
    { 0xFB8A, 1 }, // 0x0698    R       JEH
    { 0x0699, 0 }, // 0x0699
    { 0x069A, 0 }, // 0x069A
    { 0x069B, 0 }, // 0x069B
    { 0x069C, 0 }, // 0x069C
    { 0x069D, 0 }, // 0x069D
    { 0x069E, 0 }, // 0x069E
    { 0x069F, 0 }, // 0x069F

    { 0x06A0, 0 }, // 0x06A0
    { 0x06A1, 0 }, // 0x06A1
    { 0x06A2, 0 }, // 0x06A2
    { 0x06A3, 0 }, // 0x06A3
    { 0xFB6A, 3 }, // 0x06A4    D       VEH
    { 0x06A5, 0 }, // 0x06A5
    { 0xFB6E, 3 }, // 0x06A6    D       PEHEH
    { 0x06A7, 0 }, // 0x06A7
    { 0x06A8, 0 }, // 0x06A8
    { 0xFB8E, 3 }, // 0x06A9    D       KEHEH
    { 0x06AA, 0 }, // 0x06AA
    { 0x06AB, 0 }, // 0x06AB
    { 0x06AC, 0 }, // 0x06AC
    { 0xFBD3, 3 }, // 0x06AD    D       NG
    { 0x06AE, 0 }, // 0x06AE
    { 0xFB92, 3 }, // 0x06AF    D       GAF

    { 0x06B0, 0 }, // 0x06B0
    { 0xFB9A, 3 }, // 0x06B1    D       NGOEH
    { 0x06B2, 0 }, // 0x06B2
    { 0xFB96, 3 }, // 0x06B3    D       GUEH
    { 0x06B4, 0 }, // 0x06B4
    { 0x06B5, 0 }, // 0x06B5
    { 0x06B6, 0 }, // 0x06B6
    { 0x06B7, 0 }, // 0x06B7
    { 0x06B8, 0 }, // 0x06B8
    { 0x06B9, 0 }, // 0x06B9
    { 0x06BA, 0 }, // 0x06BA
    { 0xFBA0, 3 }, // 0x06BB    D       RNOON
    { 0x06BC, 0 }, // 0x06BC
    { 0x06BD, 0 }, // 0x06BD
    { 0xFBAA, 3 }, // 0x06BE    D       HEH DOACHASHMEE
    { 0x06BF, 0 }, // 0x06BF

    { 0xFBA4, 1 }, // 0x06C0    R       HEH WITH YEH ABOVE
    { 0xFBA6, 3 }, // 0x06C1    D       HEH GOAL
    { 0x06C2, 0 }, // 0x06C2
    { 0x06C3, 0 }, // 0x06C3
    { 0x06C4, 0 }, // 0x06C4
    { 0xFBE0, 1 }, // 0x06C5    R       KIRGHIZ OE
    { 0xFBD9, 1 }, // 0x06C6    R       OE
    { 0xFBD7, 1 }, // 0x06C7    R       U
    { 0xFBDB, 1 }, // 0x06C8    R       YU
    { 0xFBE2, 1 }, // 0x06C9    R       KIRGHIZ YU
    { 0x06CA, 0 }, // 0x06CA
    { 0xFBDE, 1 }, // 0x06CB    R       VE
    { 0xFBFC, 3 }, // 0x06CC    D       FARSI YEH
    { 0x06CD, 0 }, // 0x06CD
    { 0x06CE, 0 }, // 0x06CE
    { 0x06CF, 0 }, // 0x06CF

    { 0xFBE4, 3 }, // 0x06D0    D       E
    { 0x06D1, 0 }, // 0x06D1
    { 0xFBAE, 1 }, // 0x06D2    R       YEH BARREE
    { 0xFBB0, 1 }, // 0x06D3    R       YEH BARREE WITH HAMZA ABOVE
    { 0x06D4, 0 }, // 0x06D4
    { 0x06D5, 0 }, // 0x06D5
    { 0x06D6, 0 }, // 0x06D6
    { 0x06D7, 0 }, // 0x06D7
    { 0x06D8, 0 }, // 0x06D8
    { 0x06D9, 0 }, // 0x06D9
    { 0x06DA, 0 }, // 0x06DA
    { 0x06DB, 0 }, // 0x06DB
    { 0x06DC, 0 }, // 0x06DC
    { 0x06DD, 0 }, // 0x06DD
    { 0x06DE, 0 }, // 0x06DE
    { 0x06DF, 0 }, // 0x06DF

    { 0x06E0, 0 }, // 0x06E0
    { 0x06E1, 0 }, // 0x06E1
    { 0x06E2, 0 }, // 0x06E2
    { 0x06E3, 0 }, // 0x06E3
    { 0x06E4, 0 }, // 0x06E4
    { 0x06E5, 0 }, // 0x06E5
    { 0x06E6, 0 }, // 0x06E6
    { 0x06E7, 0 }, // 0x06E7
    { 0x06E8, 0 }, // 0x06E8
    { 0x06E9, 0 }, // 0x06E9
    { 0x06EA, 0 }, // 0x06EA
    { 0x06EB, 0 }, // 0x06EB
    { 0x06EC, 0 }, // 0x06EC
    { 0x06ED, 0 }, // 0x06ED
    { 0x06EE, 0 }, // 0x06EE
    { 0x06EF, 0 }, // 0x06EF

    { 0x06F0, 0 }, // 0x06F0
    { 0x06F1, 0 }, // 0x06F1
    { 0x06F2, 0 }, // 0x06F2
    { 0x06F3, 0 }, // 0x06F3
    { 0x06F4, 0 }, // 0x06F4
    { 0x06F5, 0 }, // 0x06F5
    { 0x06F6, 0 }, // 0x06F6
    { 0x06F7, 0 }, // 0x06F7
    { 0x06F8, 0 }, // 0x06F8
    { 0x06F9, 0 }, // 0x06F9
    { 0x06FA, 0 }, // 0x06FA
    { 0x06FB, 0 }, // 0x06FB
    { 0x06FC, 0 }, // 0x06FC
    { 0x06FD, 0 }, // 0x06FD
    { 0x06FE, 0 }, // 0x06FE
    { 0x06FF, 0 }  // 0x06FF
};

// the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, this table does
static const ushort alefMaksura[4] = {0xFEEF, 0xFEF0, 0xFBE8, 0xFBE9};

// this is a bit tricky. Alef always binds to the right, so the second parameter descibing the shape
// of the lam can be either initial of medial. So initial maps to the isolated form of the ligature,
// medial to the final form
static const ushort arabicUnicodeLamAlefMapping[6][4] = {
    { 0xfffd, 0xfffd, 0xfef5, 0xfef6 }, // 0x622        R       Alef with Madda above
    { 0xfffd, 0xfffd, 0xfef7, 0xfef8 }, // 0x623        R       Alef with Hamza above
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x624        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfef9, 0xfefa }, // 0x625        R       Alef with Hamza below
    { 0xfffd, 0xfffd, 0xfffd, 0xfffd }, // 0x626        // Just to fill the table ;-)
    { 0xfffd, 0xfffd, 0xfefb, 0xfefc }  // 0x627        R       Alef
};

static inline int getShape( uchar cell, int shape )
{
    // the arabicUnicodeMapping does not work for U+0649 ALEF MAKSURA, handle this here
    uint ch = ( cell != 0x49 )
	      ? (shape ? arabicUnicodeMapping[cell][0] + shape : 0x600+cell)
	      : alefMaksura[shape] ;
    return ch;
}


/*
  Two small helper functions for arabic shaping.
*/
static inline const QChar prevChar( const QString *str, int pos )
{
    //qDebug("leftChar: pos=%d", pos);
    pos--;
    const QChar *ch = str->unicode() + pos;
    while( pos > -1 ) {
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
	    return *ch;
	pos--;
	ch--;
    }
    return QChar::replacement;
}

static inline const QChar nextChar( const QString *str, int pos)
{
    pos++;
    int len = str->length();
    const QChar *ch = str->unicode() + pos;
    while( pos < len ) {
	//qDebug("rightChar: %d isLetter=%d, joining=%d", pos, ch.isLetter(), ch.joining());
	if( ::category( *ch ) != QChar::Mark_NonSpacing )
	    return *ch;
	// assume it's a transparent char, this might not be 100% correct
	pos++;
	ch++;
    }
    return QChar::replacement;
}


static void shapedString(const QString *uc, int from, int len, QChar *shapeBuffer, int *shapedLength,
			 bool reverse, QGlyphLayout *glyphs, unsigned short *logClusters )
{
    Q_ASSERT(uc->length() >= from + len);

    if( len == 0 ) {
	*shapedLength = 0;
	return;
    }

    QVarLengthArray<ArabicProperties> properties(len);
    getArabicProperties((const unsigned short *)(uc->unicode()+from), len, properties);

    const QChar *ch = uc->unicode() + from;
    QChar *data = shapeBuffer;
    int clusterStart = 0;

    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	int gpos = data - shapeBuffer;

	if ( r != 0x06 ) {
	    if ( r == 0x20 ) {
		uchar c = ch->cell();
		if (c == 0x0c || c == 0x0d)
		    // remove ZWJ and ZWNJ
		    goto skip;
	    }
	    if ( reverse )
		*data = mirroredChar( *ch );
	    else
		*data = *ch;
	} else {
	    uchar c = ch->cell();
	    int pos = i + from;
	    int shape = properties[i].shape;
//  	    qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, getShape(c, shape));
	    // take care of lam-alef ligatures (lam right of alef)
	    ushort map;
	    switch ( c ) {
		case 0x44: { // lam
		    const QChar pch = nextChar( uc, pos );
		    if ( pch.row() == 0x06 ) {
			switch ( pch.cell() ) {
			    case 0x22:
			    case 0x23:
			    case 0x25:
			    case 0x27:
// 				qDebug(" lam of lam-alef ligature");
				map = arabicUnicodeLamAlefMapping[pch.cell() - 0x22][shape];
				goto next;
			    default:
				break;
			}
		    }
		    break;
		}
		case 0x22: // alef with madda
		case 0x23: // alef with hamza above
		case 0x25: // alef with hamza below
		case 0x27: // alef
		    if ( prevChar( uc, pos ).unicode() == 0x0644 ) {
			// have a lam alef ligature
			//qDebug(" alef of lam-alef ligature");
			goto skip;
		    }
		default:
		    break;
	    }
	    map = getShape( c, shape );
	next:
	    *data = map;
	}
	// ##### Fixme
	//glyphs[gpos].attributes.zeroWidth = zeroWidth;
	if ( ::category( *ch ) == QChar::Mark_NonSpacing ) {
	    glyphs[gpos].attributes.mark = TRUE;
// 	    qDebug("glyph %d (char %d) is mark!", gpos, i );
	} else {
	    glyphs[gpos].attributes.mark = FALSE;
	    clusterStart = data - shapeBuffer;
	}
	glyphs[gpos].attributes.clusterStart = !glyphs[gpos].attributes.mark;
	glyphs[gpos].attributes.combiningClass = combiningClass( *ch );
	glyphs[gpos].attributes.justification = properties[i].justification;
	data++;
    skip:
	ch++;
	logClusters[i] = clusterStart;
    }
    *shapedLength = data - shapeBuffer;
}

#if defined(QT_OPENTYPE)

static bool arabicSyriacOpenTypeShape(QOpenType *openType, QShaperItem *item)
{
    int nglyphs = item->num_glyphs;
    if (!item->font->stringToCMap(item->string->unicode()+item->from, item->length, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
	return false;

    heuristicSetGlyphAttributes(item);

    QGlyphLayout *glyphs = item->glyphs;
    unsigned short *logClusters = item->log_clusters;
    const unsigned short *uc = (const unsigned short *)item->string->unicode() + item->from;

    QVarLengthArray<ArabicProperties> properties(item->num_glyphs);
    getArabicProperties(uc, item->length, properties);

    QVarLengthArray<bool> apply(item->num_glyphs);


    // Hack to remove ZWJ and ZWNJ from rendered output.
    int j = 0;
    for ( int i = 0; i < item->num_glyphs; i++ ) {
 	if (uc[i] == 0x200c || uc[i] == 0x200d)
 	    continue;
 	glyphs[j] = glyphs[i];
 	properties[j] = properties[i];
 	glyphs[j].attributes.justification = properties[i].justification;
 	logClusters[i] = logClusters[j];
 	++j;
    }
    item->num_glyphs = j;

    openType->init(item);

    // call features in the order defined by http://www.microsoft.com/typography/otfntdev/arabicot/shaping.htm
    openType->applyGSUBFeature(FT_MAKE_TAG( 'c', 'c', 'm', 'p' ));

    if (item->script == QFont::Arabic) {
	const struct {
	    int tag;
	    int shape;
	} features[] = {
	    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), XIsolated },
	    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), XFinal },
	    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), XMedial },
	    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), XInitial }
	};
	for (int j = 0; j < 4; ++j) {
	    for ( int i = 0; i < item->num_glyphs; i++ )
		apply[i] = (properties[i].shape == features[j].shape);
	    openType->applyGSUBFeature(features[j].tag, apply);
	}
    } else {
	const struct {
	    int tag;
	    int shape;
	} features[] = {
	    { FT_MAKE_TAG( 'i', 's', 'o', 'l' ), XIsolated },
	    { FT_MAKE_TAG( 'f', 'i', 'n', 'a' ), XFinal },
	    { FT_MAKE_TAG( 'f', 'i', 'n', '2' ), XFinal },
	    { FT_MAKE_TAG( 'f', 'i', 'n', '3' ), XFinal },
	    { FT_MAKE_TAG( 'm', 'e', 'd', 'i' ), XMedial },
	    { FT_MAKE_TAG( 'm', 'e', 'd', '2' ), XMedial },
	    { FT_MAKE_TAG( 'i', 'n', 'i', 't' ), XInitial }
	};
	for (int j = 0; j < 7; ++j) {
	    for ( int i = 0; i < item->num_glyphs; i++ )
		apply[i] = (properties[i].shape == features[j].shape);
	    openType->applyGSUBFeature(features[j].tag, apply);
	}
    }
    const int commonFeatures[] = {
	// these features get applied to all glyphs and both scripts
	FT_MAKE_TAG( 'r', 'l', 'i', 'g' ),
	FT_MAKE_TAG( 'c', 'a', 'l', 't' ),
	FT_MAKE_TAG( 'l', 'i', 'g', 'a' ),
	FT_MAKE_TAG( 'd', 'l', 'i', 'g' )
    };
    for (int j = 0; j < 4; ++j)
	openType->applyGSUBFeature(commonFeatures[j]);

    if (item->script == QFont::Arabic) {
	const int features[] = {
	    FT_MAKE_TAG( 'c', 's', 'w', 'h' ),
	    // mset is used in old Win95 fonts that don't have a 'mark' positioning table.
	    FT_MAKE_TAG( 'm', 's', 'e', 't' )
	};
	for (int j = 0; j < 2; ++j)
	    openType->applyGSUBFeature(features[j]);
    }

    openType->applyGPOSFeatures();

    // reset num_glyphs to what is available.
    item->num_glyphs = nglyphs;
    return openType->appendTo(item);
}

#endif

static void arabic_attributes( int /*script*/, const QString &text, int from, int len, QCharAttributes *attributes )
{
    const QChar *uc = text.unicode() + from;
    attributes += from;
    for ( int i = 0; i < len; i++ ) {
	QChar::Category cat = ::category( *uc );
	attributes->whiteSpace = (cat == QChar::Separator_Space) && (uc->unicode() != 0xa0);
	attributes->softBreak = FALSE;
	attributes->charStop = (cat != QChar::Mark_NonSpacing);
	attributes->wordStop = FALSE;
	attributes->invalid = FALSE;
	++uc;
	++attributes;
    }
}


// #### stil missing: identify invalid character combinations
static bool arabic_shape(QShaperItem *item)
{
    Q_ASSERT(item->script == QFont::Arabic);

#ifdef QT_OPENTYPE
    QOpenType *openType = item->font->openType();

    if ( openType && openType->supportsScript( QFont::Arabic ) )
	return arabicSyriacOpenTypeShape(openType, item);
#endif

    QVarLengthArray<ushort> shapedChars(item->length);

    int slen;
    shapedString( item->string, item->from, item->length, (QChar *)(unsigned short*)shapedChars, &slen,
		  item->flags & QTextEngine::RightToLeft,
		  item->glyphs, item->log_clusters );

    if (!item->font->stringToCMap( (QChar *)(unsigned short*)shapedChars, slen, item->glyphs, &item->num_glyphs, QFlag(item->flags)))
	return false;

    for (int i = 0; i < slen; ++i)
	if (item->glyphs[i].attributes.mark) {
	    item->glyphs[i].advance.x = 0;
	    item->glyphs[i].advance.y = 0;
	}
    q_heuristicPosition(item);
    return true;
}

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
# include "qscriptengine_unix.cpp"
#elif defined( Q_WS_WIN )
# include "qscriptengine_win.cpp"
#elif defined( Q_WS_MAC )
# include "qscriptengine_mac.cpp"
#endif
