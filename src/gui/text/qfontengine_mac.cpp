/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qapplication_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <qbitmap.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qprintengine_mac_p.h>
#include <qglobal.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qvarlengtharray.h>
#include <qdebug.h>
#include <qendian.h>

#include <ApplicationServices/ApplicationServices.h>

/*****************************************************************************
  QFontEngine debug facilities
 *****************************************************************************/
//#define DEBUG_ADVANCES


#ifndef FixedToQFixed
#define FixedToQFixed(a) QFixed::fromFixed((a) >> 10)
#define QFixedToFixed(x) ((x).value() << 10)
#endif

class QMacFontPath
{
    float x, y;
    QPainterPath *path;
public:
    inline QMacFontPath(float _x, float _y, QPainterPath *_path) : x(_x), y(_y), path(_path) { }
    inline void setPosition(float _x, float _y) { x = _x; y = _y; }
    inline void advance(float _x) { x += _x; }
    static OSStatus lineTo(const Float32Point *, void *);
    static OSStatus cubicTo(const Float32Point *, const Float32Point *,
                            const Float32Point *, void *);
    static OSStatus moveTo(const Float32Point *, void *);
    static OSStatus closePath(void *);
};

OSStatus QMacFontPath::lineTo(const Float32Point *pt, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->lineTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::cubicTo(const Float32Point *cp1, const Float32Point *cp2,
                               const Float32Point *ep, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->cubicTo(p->x + cp1->x, p->y + cp1->y,
                     p->x + cp2->x, p->y + cp2->y,
                     p->x + ep->x, p->y + ep->y);
    return noErr;
}

OSStatus QMacFontPath::moveTo(const Float32Point *pt, void *data)
{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->moveTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::closePath(void *data)
{
    static_cast<QMacFontPath*>(data)->path->closeSubpath();
    return noErr;
}

#include "qscriptengine_p.h"

QFontEngineMacMulti::QFontEngineMacMulti(const ATSFontFamilyRef &atsFamily, const QFontDef &fontDef, bool kerning)
    : QFontEngineMulti(0)
{
    this->fontDef = fontDef;
    this->kerning = kerning;
    familyref = atsFamily;

    OSStatus status;

    status = ATSUCreateTextLayout(&textLayout);
    Q_ASSERT(status == noErr);

    const int maxAttributeCount = 4;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    Fixed size = FixRatio(fontDef.pixelSize, 1);
    tags[attributeCount] = kATSUSizeTag;
    sizes[attributeCount] = sizeof(size);
    values[attributeCount] = &size;
    ++attributeCount;

//    QCFString familyStr;
//    ATSFontFamilyGetName(familyref, kATSOptionFlagsDefault, &familyStr);
//    qDebug() << "family" << (QString)familyStr;

    FMFontFamily family = FMGetFontFamilyFromATSFontFamilyRef(familyref);
    Q_ASSERT(family != kInvalidFontFamily);

    FMFontStyle fntStyle = 0;
    if (fontDef.weight >= QFont::Bold)
        fntStyle |= ::bold;
    if (fontDef.style != QFont::StyleNormal)
        fntStyle |= ::italic;

    FMFontStyle intrinsicStyle;
    FMFont fnt;
    status = FMGetFontFromFontFamilyInstance(family, fntStyle, &fnt, &intrinsicStyle);
    Q_ASSERT(status == noErr);

    tags[attributeCount] = kATSUFontTag;
    // KATSUFontID is typedef'ed to FMFont
    sizes[attributeCount] = sizeof(FMFont);
    values[attributeCount] = &fnt;
    ++attributeCount;

    status = ATSUCreateStyle(&style);
    Q_ASSERT(status == noErr);

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    status = ATSUSetAttributes(style, attributeCount, tags, sizes, values);
    Q_ASSERT(status == noErr);

    QFontEngineMac *fe = new QFontEngineMac(style, fnt, fontDef, this);
    fe->ref.ref();
    engines.append(fe);
}

QFontEngineMacMulti::~QFontEngineMacMulti()
{
    ATSUDisposeTextLayout(textLayout);
    ATSUDisposeStyle(style);

    for (int i = 0; i < engines.count(); ++i) {
        QFontEngineMac *fe = const_cast<QFontEngineMac *>(static_cast<const QFontEngineMac *>(engines.at(i)));
        fe->multiEngine = 0;
        if (!fe->ref.deref())
            delete fe;
    }
    engines.clear();
}

struct QGlyphLayoutInfo
{
    QGlyphLayout *glyphs;
    int *numGlyphs;
    bool callbackCalled;
    int *mappedFonts;
    QTextEngine::ShaperFlags flags;
    QShaperItem *shaperItem;
};

static OSStatus atsuPostLayoutCallback(ATSULayoutOperationSelector selector,
                                 ATSULineRef lineRef,
                                 UInt32 refCon,
                                 void *operationExtraParameter,
                                 ATSULayoutOperationCallbackStatus *callbackStatus)
{
    Q_UNUSED(selector);
    Q_UNUSED(operationExtraParameter);

    QGlyphLayoutInfo *nfo = reinterpret_cast<QGlyphLayoutInfo *>(refCon);
    nfo->callbackCalled = true;

    ATSLayoutRecord *layoutData = 0;
    ItemCount itemCount = 0;

    OSStatus e = noErr;
    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                                   /*iCreate =*/ false,
                                                   (void **) &layoutData,
                                                   &itemCount);
    if (e != noErr)
        return e;

    *nfo->numGlyphs = itemCount - 1;

    Fixed *baselineDeltas = 0;

    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                                   /*iCreate =*/ true,
                                                   (void **) &baselineDeltas,
                                                   &itemCount);
    if (e != noErr)
        return e;

    int nextCharStop = -1;
    int currentClusterGlyph = -1; // first glyph in log cluster
    QShaperItem *item = 0;
    if (nfo->shaperItem) {
        item = nfo->shaperItem;
#if !defined(QT_NO_DEBUG)
        int surrogates = 0;
        const QString &str = *item->string;
        for (int i = item->from; i < item->from + item->length - 1; ++i) {
            surrogates += (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00
                           && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        }
        Q_ASSERT(*nfo->numGlyphs == item->length - surrogates);
#endif
        for (nextCharStop = item->from; nextCharStop < item->from + item->length; ++nextCharStop)
            if (item->charAttributes[nextCharStop].charStop)
                break;
        nextCharStop -= item->from;
    }

    int glyphIdx = 0;
    int glyphIncrement = 1;
    if (nfo->flags & QTextEngine::RightToLeft) {
        glyphIdx  = itemCount - 2;
        glyphIncrement = -1;
    }
    for (int i = 0; i < *nfo->numGlyphs; ++i, glyphIdx += glyphIncrement) {

        int charOffset = layoutData[glyphIdx].originalOffset / sizeof(UniChar);
        const int fontIdx = nfo->mappedFonts[charOffset];

        ATSGlyphRef glyphId = layoutData[glyphIdx].glyphID;

        QFixed yAdvance = FixedToQFixed(baselineDeltas[glyphIdx]);
        QFixed xAdvance = FixedToQFixed(layoutData[glyphIdx + 1].realPos - layoutData[glyphIdx].realPos);

        if (glyphId != 0xffff || i == 0) {
            nfo->glyphs[i].glyph = (glyphId & 0x00ffffff) | (fontIdx << 24);

            nfo->glyphs[i].advance.y = yAdvance;
            nfo->glyphs[i].advance.x = xAdvance;
        } else {
            // ATSUI gives us 0xffff as glyph id at the index in the glyph array for
            // a character position that maps to a ligtature. Such a glyph id does not
            // result in any visual glyph, but it may have an advance, which is why we
            // sum up the glyph advances.
            --i;
            nfo->glyphs[i].advance.y += yAdvance;
            nfo->glyphs[i].advance.x += xAdvance;
            *nfo->numGlyphs -= 1;
        }

        if (item) {
            if (charOffset >= nextCharStop) {
                nfo->glyphs[i].attributes.clusterStart = true;
                currentClusterGlyph = i;

                ++nextCharStop;
                for (; nextCharStop < item->length; ++nextCharStop)
                    if (item->charAttributes[item->from + nextCharStop].charStop)
                        break;
            } else {
                if (currentClusterGlyph == -1)
                    currentClusterGlyph = i;
            }
            item->log_clusters[charOffset] = currentClusterGlyph;

            // surrogate handling
            if (charOffset < item->length - 1) {
                QChar current = item->string->at(item->from + charOffset);
                QChar next = item->string->at(item->from + charOffset + 1);
                if (current.unicode() >= 0xd800 && current.unicode() < 0xdc00
                    && next.unicode() >= 0xdc00 && next.unicode() < 0xe000) {
                    item->log_clusters[charOffset + 1] = currentClusterGlyph;
                }
            }
        }
    }

    /*
    if (item) {
        qDebug() << "resulting logclusters:";
        for (int i = 0; i < item->length; ++i)
            qDebug() << "logClusters[" << i << "] =" << item->log_clusters[i];
        qDebug() << "clusterstarts:";
        for (int i = 0; i < *nfo->numGlyphs; ++i)
            qDebug() << "clusterStart[" << i << "] =" << nfo->glyphs[i].attributes.clusterStart;
    }
    */

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                        (void **) &baselineDeltas);

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                        (void **) &layoutData);

    *callbackStatus = kATSULayoutOperationCallbackStatusHandled;
    return noErr;
}

int QFontEngineMacMulti::fontIndexForFMFont(FMFont font) const
{
    for (int i = 0; i < engines.count(); ++i)
        if (engineAt(i)->fmFont == font)
            return i;

    QFontEngineMacMulti *that = const_cast<QFontEngineMacMulti *>(this);
    QFontEngineMac *fe = new QFontEngineMac(style, font, fontDef, that);
    fe->ref.ref();
    that->engines.append(fe);

    return engines.count() - 1;
}

bool QFontEngineMacMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    return stringToCMap(str, len, glyphs, nglyphs, flags, /*shaperItem=*/0);
}

bool QFontEngineMacMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                                  QShaperItem *shaperItem) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }
    //qDebug() << "stringToCMap" << QString(str, len);

    OSStatus e = noErr;

    e = ATSUSetTextPointerLocation(textLayout, (UniChar *)(str), 0, len, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    QGlyphLayoutInfo nfo;
    nfo.glyphs = glyphs;
    nfo.numGlyphs = nglyphs;
    nfo.callbackCalled = false;
    nfo.flags = flags;
    nfo.shaperItem = shaperItem;

    QVarLengthArray<int> mappedFonts(len);
    for (int i = 0; i < len; ++i)
        mappedFonts[i] = 0;
    nfo.mappedFonts = mappedFonts.data();

    Q_ASSERT(sizeof(void *) <= sizeof(UInt32));
    e = ATSUSetTextLayoutRefCon(textLayout, reinterpret_cast<UInt32>(&nfo));
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextLayoutRefCon %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    {
        const int maxAttributeCount = 3;
        ATSUAttributeTag tags[maxAttributeCount + 1];
        ByteCount sizes[maxAttributeCount + 1];
        ATSUAttributeValuePtr values[maxAttributeCount + 1];
        int attributeCount = 0;

        tags[attributeCount] = kATSULineLayoutOptionsTag;
        ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment
                                       | kATSLineIgnoreFontLeading
                                       | kATSLineNoSpecialJustification // we do kashidas ourselves
                                       | kATSLineDisableAllJustification
                                       ;

        if (!(flags & QTextEngine::DesignMetrics)) {
            layopts |= kATSLineFractDisable | kATSLineUseDeviceMetrics
                       | kATSLineDisableAutoAdjustDisplayPos;
        }

        if (fontDef.styleStrategy & QFont::NoAntialias)
            layopts |= kATSLineNoAntiAliasing;

        if (!kerning)
            layopts |= kATSLineDisableAllKerningAdjustments;

        values[attributeCount] = &layopts;
        sizes[attributeCount] = sizeof(layopts);
        ++attributeCount;

        tags[attributeCount] = kATSULayoutOperationOverrideTag;
        ATSULayoutOperationOverrideSpecifier spec;
        spec.operationSelector = kATSULayoutOperationPostLayoutAdjustment;
        spec.overrideUPP = atsuPostLayoutCallback;
        values[attributeCount] = &spec;
        sizes[attributeCount] = sizeof(spec);
        ++attributeCount;

        Boolean direction;
        if (flags & QTextEngine::RightToLeft)
            direction = kATSURightToLeftBaseDirection;
        else
            direction = kATSULeftToRightBaseDirection;
        tags[attributeCount] = kATSULineDirectionTag;
        values[attributeCount] = &direction;
        sizes[attributeCount] = sizeof(direction);
        ++attributeCount;

        Q_ASSERT(attributeCount < maxAttributeCount + 1);
        e = ATSUSetLayoutControls(textLayout, attributeCount, tags, sizes, values);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUSetLayoutControls %s: %d", e, __FILE__, __LINE__);
            return false;
        }

    }

    e = ATSUSetRunStyle(textLayout, style, 0, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", e, __FILE__, __LINE__);
        return false;
    }

    {
        int pos = 0;
        do {
            FMFont substFont = 0;
            UniCharArrayOffset changedOffset = 0;
            UniCharCount changeCount = 0;

            e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                     &substFont, &changedOffset,
                                     &changeCount);
            if (e == kATSUFontsMatched) {
                int fontIdx = fontIndexForFMFont(substFont);
                for (uint i = 0; i < changeCount; ++i)
                    mappedFonts[changedOffset + i] = fontIdx;
                pos = changedOffset + changeCount;

                ATSUSetRunStyle(textLayout, engineAt(fontIdx)->style, changedOffset, changeCount);

            } else if (e == kATSUFontsNotMatched) {
                pos = changedOffset + changeCount;
            }
        } while (pos < len && e != noErr);
    }

    // trigger the a layout
    {
        Rect rect;
        e = ATSUMeasureTextImage(textLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                                 /*iLocationX =*/ 0, /*iLocationY =*/ 0,
                                 &rect);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage %s: %d", e, __FILE__, __LINE__);
            return false;
        }
    }

    if (!nfo.callbackCalled) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage did not trigger callback %s: %d", e, __FILE__, __LINE__);
            return false;
    }

    ATSUClearLayoutCache(textLayout, kATSUFromTextBeginning);

    return true;
}

void QFontEngineMacMulti::recalcAdvances(int numGlyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    Q_ASSERT(false);
    Q_UNUSED(numGlyphs);
    Q_UNUSED(glyphs);
    Q_UNUSED(flags);
}

void QFontEngineMacMulti::doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const
{
    //Q_ASSERT(false);
}

void QFontEngineMacMulti::loadEngine(int /*at*/)
{
    // should never be called!
    Q_ASSERT(false);
}

bool QFontEngineMacMulti::canRender(const QChar *string, int len)
{
    ATSUSetTextPointerLocation(textLayout, reinterpret_cast<const UniChar *>(string), 0, len, len);
    ATSUSetRunStyle(textLayout, style, 0, len);

    OSStatus e = noErr;
    int pos = 0;
    do {
        FMFont substFont = 0;
        UniCharArrayOffset changedOffset = 0;
        UniCharCount changeCount = 0;

        e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                 &substFont, &changedOffset,
                                 &changeCount);
        if (e == kATSUFontsMatched) {
            pos = changedOffset + changeCount;
        }
    } while (pos < len && e != noErr);

    return e == noErr || e == kATSUFontsMatched;
}

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((quint32)(ch1)) << 24) | \
    (((quint32)(ch2)) << 16) | \
    (((quint32)(ch3)) << 8) | \
    ((quint32)(ch4)) \
   )

QFontEngineMac::QFontEngineMac(ATSUStyle baseStyle, FMFont fmFont, const QFontDef &def, QFontEngineMacMulti *multiEngine)
    : fmFont(fmFont), multiEngine(multiEngine), cmap(0), symbolCMap(false)
{
    fontDef = def;
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    cgFont = CGFontCreateWithPlatformFont(&atsFont);

    const int maxAttributeCount = 4;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    synthesisFlags = 0;

    FMFontStyle fntStyle = 0;
    if (fontDef.weight >= QFont::Bold)
        fntStyle |= ::bold;
    if (fontDef.style != QFont::StyleNormal)
        fntStyle |= ::italic;

    FMFontStyle intrinsicStyle;
    FMFontFamily family;
    OSStatus status = FMGetFontFamilyInstanceFromFont(fmFont, &family, &intrinsicStyle);
    // this can sometimes fail, for example for devanagari. it's not critical though
    if (status == noErr) {
        Boolean atsuBold = false;
        Boolean atsuItalic = false;

        if ((fntStyle & ::italic)
            && (!(intrinsicStyle & ::italic))) {
            synthesisFlags |= SynthesizedItalic;

            atsuItalic = true;
            tags[attributeCount] = kATSUQDItalicTag;
            sizes[attributeCount] = sizeof(atsuItalic);
            values[attributeCount] = &atsuItalic;
            ++attributeCount;
        }
        if ((fntStyle & ::bold)
            && (!(intrinsicStyle & ::bold))) {
            synthesisFlags |= SynthesizedBold;

            atsuBold = true;
            tags[attributeCount] = kATSUQDBoldfaceTag;
            sizes[attributeCount] = sizeof(atsuBold);
            values[attributeCount] = &atsuBold;
            ++attributeCount;
        }
    }

    ATSUCreateAndCopyStyle(baseStyle, &style);

    tags[attributeCount] = kATSUFontTag;
    values[attributeCount] = &fmFont;
    sizes[attributeCount] = sizeof(fmFont);
    ++attributeCount;

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    status = ATSUSetAttributes(style, attributeCount, tags, sizes, values);
    Q_ASSERT(status == noErr);

    int tmpFsType;
    if (ATSFontGetTable(FMGetATSFontRefFromFont(fmFont),
                        MAKE_TAG('O', 'S', '/', '2'),
                        /*offset = */8,
                        /*size = */2, &tmpFsType, 0) == noErr) {
       fsType = qFromBigEndian<quint16>(tmpFsType);
    } else {
        fsType = 0;
    }
}

QFontEngineMac::~QFontEngineMac()
{
    ATSUDisposeStyle(style);
    if (cmap)
        delete [] cmap;
}

static inline quint32 getUInt(unsigned char *p)
{ return qFromBigEndian<quint32>(p); }

static inline quint16 getUShort(unsigned char *p)
{ return qFromBigEndian<quint16>(p); }


static quint32 getGlyphIndex(unsigned char *table, unsigned int unicode)
{
    unsigned short format = getUShort(table);
    if (format == 0) {
        if (unicode < 256)
            return (int) *(table+6+unicode);
    } else if (format == 4) {
        /* some fonts come with invalid cmap tables, where the last segment
           specified end = start = rangeoffset = 0xffff, delta = 0x0001
           Since 0xffff is never a valid Unicode char anyway, we just get rid of the issue
           by returning 0 for 0xffff
        */
        if(unicode >= 0xffff)
            return 0;
        quint16 segCountX2 = getUShort(table + 6);
        unsigned char *ends = table + 14;
        quint16 endIndex = 0;
        int i = 0;
        for (; i < segCountX2/2 && (endIndex = getUShort(ends + 2*i)) < unicode; i++);

        unsigned char *idx = ends + segCountX2 + 2 + 2*i;
        quint16 startIndex = getUShort(idx);

        if (startIndex > unicode)
            return 0;

        idx += segCountX2;
        qint16 idDelta = (qint16)getUShort(idx);
        idx += segCountX2;
        quint16 idRangeoffset_t = (quint16)getUShort(idx);

        quint16 glyphIndex;
        if (idRangeoffset_t) {
            quint16 id = getUShort(idRangeoffset_t + 2*(unicode - startIndex) + idx);
            if (id)
                glyphIndex = (idDelta + id) % 0x10000;
            else
                glyphIndex = 0;
        } else {
            glyphIndex = (idDelta + unicode) % 0x10000;
        }
        return glyphIndex;
    } else if (format == 12) {
        quint32 nGroups = getUInt(table + 12);

        table += 16; // move to start of groups

        int left = 0, right = nGroups - 1;
        while (left <= right) {
            int middle = left + ( ( right - left ) >> 1 );

            quint32 startCharCode = getUInt(table + 12*middle);
            if(unicode < startCharCode)
                right = middle - 1;
            else {
                quint32 endCharCode = getUInt(table + 12*middle + 4);
                if(unicode <= endCharCode)
                    return getUInt(table + 12*middle + 8) + unicode - startCharCode;
                left = middle + 1;
            }
        }
    } else {
        qDebug("QFontEngineMac::cmap table of format %d not implemented", format);
    }

    return 0;
}

static unsigned char *getCMap(ATSFontRef font, bool &symbol)
{
    const quint32 CMAP = MAKE_TAG('c', 'm', 'a', 'p');

    unsigned char header[8];

    // get the CMAP header and the number of encoding tables
    if (ATSFontGetTable(font, CMAP, 0, 4, &header, 0) != noErr)
        return 0;
    {
        unsigned short version = getUShort(header);
        if (version != 0)
            return 0;
    }

    unsigned short numTables = getUShort(header+2);
    unsigned char *maps = new unsigned char[8*numTables];

    // get the encoding table and look for Unicode
    if (ATSFontGetTable(font, CMAP, 4, 8*numTables, maps, 0) != noErr)
        return 0;

    quint32 version = 0;
    unsigned int unicode_table = 0;
    for (int n = 0; n < numTables; n++) {
        const quint16 platformId = getUInt(maps + 8*n);
        if (platformId != 0x0003)
                continue;
        const quint16 platformSpecificId = getUInt(maps + 8*n + 2);
        // accept both symbol and Unicode encodings. prefer unicode.
        if (platformSpecificId == 0x0001
            || platformSpecificId == 0x0000
            || platformSpecificId == 0x000a) {
            if (platformSpecificId > version || !unicode_table) {
                version = platformSpecificId;
                unicode_table = getUInt(maps + 8*n + 4);
            }
        }
    }
    symbol = (version == 0x0000);

    if (!unicode_table) {
        // qDebug("no unicode table found");
        return 0;
    }

    delete [] maps;

    // get the header of the unicode table
    if (ATSFontGetTable(font, CMAP, unicode_table, 8, &header, 0) != noErr)
        return 0;

    unsigned short format = getUShort(header);
    unsigned int length;
    if(format < 8)
        length = getUShort(header+2);
    else
        length = getUInt(header+4);
    unsigned char *unicode_data = new unsigned char[length];

    // get the cmap table itself
    if (ATSFontGetTable(font, CMAP, unicode_table, length, unicode_data, 0) != noErr) {
        delete [] unicode_data;
        return 0;
    }
    return unicode_data;
}

static inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (flags & QTextEngine::GlyphIndicesOnly) {
        if (!cmap) {
            cmap = getCMap(FMGetATSFontRefFromFont(fmFont), symbolCMap);
            if (!cmap)
                return false;
        }
        if (symbolCMap) {
            for (int i = 0; i < len; ++i) {
                unsigned int uc = getChar(str, i, len);
                glyphs->glyph = getGlyphIndex(cmap, uc);
                if(!glyphs->glyph && uc < 0x100)
                    glyphs->glyph = getGlyphIndex(cmap, uc + 0xf000);
                glyphs++;
            }
        } else {
            for (int i = 0; i < len; ++i) {
                unsigned int uc = getChar(str, i, len);
                glyphs->glyph = getGlyphIndex(cmap, uc);
                glyphs++;
            }
        }

        *nglyphs = len;
        return true;
    }
    if (!multiEngine)
        return false;

    const bool kashidaRequest = (len == 1 && *str == QChar(0x640));
    if (kashidaRequest && kashidaGlyph.glyph != 0) {
        *glyphs = kashidaGlyph;
        *nglyphs = 1;
        return true;
    }

    bool result = multiEngine->stringToCMap(str, len, glyphs, nglyphs, flags);

    if (result && kashidaRequest) {
        kashidaGlyph = *glyphs;
    }

    return result;
}

glyph_metrics_t QFontEngineMac::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    QFixed w;
    const QGlyphLayout *end = glyphs + numGlyphs;
    while(end > glyphs)
        w += (--end)->advance.x;
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

glyph_metrics_t QFontEngineMac::boundingBox(glyph_t glyph)
{
    GlyphID atsuGlyph = glyph;

    ATSGlyphScreenMetrics metrics;

    ATSUGlyphGetScreenMetrics(style, 1, &atsuGlyph, 0,
                              /* iForcingAntiAlias =*/ false,
                              /* iAntiAliasSwitch =*/true,
                              &metrics);

    // ### check again

    glyph_metrics_t gm;
    gm.width = int(metrics.width);
    gm.height = int(metrics.height);
    gm.x = QFixed::fromReal(metrics.topLeft.x);
    gm.y = -QFixed::fromReal(metrics.topLeft.y);
    gm.xoff = QFixed::fromReal(metrics.deviceAdvance.x);
    gm.yoff = QFixed::fromReal(metrics.deviceAdvance.y);

    return gm;
}

QFixed QFontEngineMac::ascent() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSUAscentTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

QFixed QFontEngineMac::descent() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSUDescentTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

QFixed QFontEngineMac::leading() const
{
    ATSUTextMeasurement metric;
    ATSUGetAttribute(style, kATSULeadingTag, sizeof(metric), &metric, 0);
    return FixRound(metric);
}

qreal QFontEngineMac::maxCharWidth() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    ATSFontMetrics metrics;
    ATSFontGetHorizontalMetrics(atsFont, kATSOptionFlagsDefault, &metrics);
    return metrics.maxAdvanceWidth * fontDef.pointSize;
}

QFixed QFontEngineMac::xHeight() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    ATSFontMetrics metrics;
    ATSFontGetHorizontalMetrics(atsFont, kATSOptionFlagsDefault, &metrics);
    return QFixed::fromReal(metrics.xHeight * fontDef.pointSize);
}

void QFontEngineMac::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path,
                                           QTextItem::RenderFlags)
{
    if (!numGlyphs)
        return;

    OSStatus e;

    QMacFontPath fontpath(0, 0, path);
    ATSCubicMoveToUPP moveTo = NewATSCubicMoveToUPP(QMacFontPath::moveTo);
    ATSCubicLineToUPP lineTo = NewATSCubicLineToUPP(QMacFontPath::lineTo);
    ATSCubicCurveToUPP cubicTo = NewATSCubicCurveToUPP(QMacFontPath::cubicTo);
    ATSCubicClosePathUPP closePath = NewATSCubicClosePathUPP(QMacFontPath::closePath);

    for (int i = 0; i < numGlyphs; ++i) {
        GlyphID glyph = glyphs[i];

        fontpath.setPosition(positions[i].x.toReal(), positions[i].y.toReal());
        ATSUGlyphGetCubicPaths(style, glyph, moveTo, lineTo,
                               cubicTo, closePath, &fontpath, &e);
    }

    DisposeATSCubicMoveToUPP(moveTo);
    DisposeATSCubicLineToUPP(lineTo);
    DisposeATSCubicCurveToUPP(cubicTo);
    DisposeATSCubicClosePathUPP(closePath);
}

bool QFontEngineMac::canRender(const QChar *string, int len)
{
    Q_ASSERT(false);
    Q_UNUSED(string);
    Q_UNUSED(len);
    return false;
}

void QFontEngineMac::draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight)
{
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix;
    matrix.translate(x, y);
    getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    CGContextSetFontSize(ctx, fontDef.pixelSize);

    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);

    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, -1, 0, -paintDeviceHeight);

    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -tanf(14 * acosf(0) / 90), 1, 0, 0));

    CGContextSetTextMatrix(ctx, cgMatrix);

    CGContextSetTextDrawingMode(ctx, kCGTextFill);


    QVarLengthArray<CGSize> advances(glyphs.size());
    QVarLengthArray<CGGlyph> cgGlyphs(glyphs.size());

    for (int i = 0; i < glyphs.size() - 1; ++i) {
        advances[i].width = (positions[i + 1].x - positions[i].x).toReal();
        advances[i].height = (positions[i + 1].y - positions[i].y).toReal();
        cgGlyphs[i] = glyphs[i];
    }
    advances[glyphs.size() - 1].width = 0;
    advances[glyphs.size() - 1].height = 0;
    cgGlyphs[glyphs.size() - 1] = glyphs[glyphs.size() - 1];

    CGContextSetFont(ctx, cgFont);

    CGContextSetTextPosition(ctx, positions[0].x.toReal(), positions[0].y.toReal());

    CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, positions[0].x.toReal() + 0.5 * lineThickness().toReal(),
                                      positions[0].y.toReal());

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());
    }

    CGContextSetTextMatrix(ctx, oldTextMatrix);
}

QFontEngine::FaceId QFontEngineMac::faceId() const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    FaceId face;

    FSSpec spec;
    if (ATSFontGetFileSpecification(atsFont, &spec) != noErr)
        return FaceId();

    FSRef ref;
    FSpMakeFSRef(&spec, &ref);

    face.filename = QByteArray(128, 0);
    FSRefMakePath(&ref, (UInt8 *)face.filename.data(), face.filename.size());

    return face;
}

QByteArray QFontEngineMac::getSfntTable(uint tag) const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);

    ByteCount length;
    OSStatus status = ATSFontGetTable(atsFont, tag, 0, 0, 0, &length);
    if (status != noErr)
        return QByteArray();
    QByteArray table(length, 0);
    status = ATSFontGetTable(atsFont, tag, 0, table.length(), table.data(), &length);
    if (status != noErr)
        return QByteArray();
    return table;
}

QFontEngine::Properties QFontEngineMac::properties() const
{
    QFontEngine::Properties props = QFontEngine::properties();

    ATSFontRef atsFont = FMGetATSFontRefFromFont(fmFont);
    QCFString psName;
    if (ATSFontGetPostScriptName(atsFont, kATSOptionFlagsDefault, &psName) == noErr)
        props.postscriptName = QString(psName).toUtf8();
    return props;
}

void QFontEngineMac::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    return QFontEngine::getUnscaledGlyph(glyph, path, metrics);
}

