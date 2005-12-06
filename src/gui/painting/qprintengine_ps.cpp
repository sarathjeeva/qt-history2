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

#include "qplatformdefs.h"

#include <private/qprintengine_ps_p.h>
#include <private/qpainter_p.h>
#include <private/qfontengine_p.h>
#include <private/qpaintengine_p.h>
#include <private/qpdf_p.h>

// <X11/Xlib.h> redefines Status -> int
#if defined(Status)
# undef Status
#endif

#ifndef QT_NO_PRINTER

#include "qprinter.h"
#include "qpainter.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qdatetime.h"
#include "qstring.h"
#include "qbytearray.h"
#include "qhash.h"
#include "qbuffer.h"
#include "qfile.h"
#include "qsettings.h"
#include "qmap.h"
#include "qbitmap.h"
#include "qregion.h"
#include "qimagewriter.h"
#include <private/qunicodetables_p.h>
#include <private/qpainterpath_p.h>
#include <qdebug.h>
#include <private/qdrawhelper_p.h>

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#if defined (Q_WS_X11) || defined (Q_WS_QWS)
#include <qtextlayout.h>
#endif

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <private/qt_x11_p.h>
#endif

static bool qt_gen_epsf = false;

void qt_generate_epsf(bool b)
{
    qt_gen_epsf = b;
}

static const char *const ps_header =
"/BD{bind def}bind def/d2{dup dup}BD/ED{exch def}BD/D0{0 ED}BD/F{setfont}BD\n"
"/RL{rlineto}BD/NP{newpath}BD/CM{currentmatrix}BD/SM{setmatrix}BD/TR\n"
"{translate}BD/SD{setdash}BD/SC{aload pop setrgbcolor}BD/CR{currentfile read\n"
"pop}BD/i{index}BD/scs{setcolorspace}BD/DB{dict dup begin}BD/DE{end def}BD/ie\n"
"{ifelse}BD/gs{gsave}BD/gr{grestore}BD/w{setlinewidth}BD/d{setdash}BD/J\n"
"{setlinecap}BD/j{setlinejoin}BD/scn{3 array astore/BCol exch def}BD/SCN{3\n"
"array astore/PCol exch def}BD/cm{6 array astore concat}BD/m{moveto}BD/l\n"
"{lineto}BD/c{curveto}BD/h{closepath}BD/W{clip}BD/W*{eoclip}BD/n{}BD/q{gsave\n"
"10 dict begin}BD/Q{end grestore}BD/re{4 2 roll m dup 0 exch RL exch 0 RL 0\n"
"exch neg RL h}BD/S{gs PCol SC stroke gr}BD/BT{gsave 10 dict begin/_m matrix\n"
"CM def}BD/ET{end grestore}BD/Tf{/_fs ED findfont[_fs 0 0 _fs 0 0]makefont F}\n"
"BD/Tm{6 array astore concat}BD/Td{translate}BD/Tj{0 0 m show}BD/BSt 0 def\n"
"/LWi 0 def/WFi false def/BCol[1 1 1]def/PCol[0 0 0]def/BkCol[1 1 1]def/BDArr\n"
"[0.94 0.88 0.63 0.50 0.37 0.12 0.06]def/defM matrix def/level3{\n"
"/languagelevel where{pop languagelevel 3 ge}{false}ie}BD/QCIgray D0/QCIcolor\n"
"D0/QCIindex D0/QCI{/colorimage where{pop false 3 colorimage}{exec/QCIcolor\n"
"ED/QCIgray QCIcolor length 3 idiv string def 0 1 QCIcolor length 3 idiv 1\n"
"sub{/QCIindex ED/_x QCIindex 3 mul def QCIgray QCIindex QCIcolor _x get 0.30\n"
"mul QCIcolor _x 1 add get 0.59 mul QCIcolor _x 2 add get 0.11 mul add add\n"
"cvi put}for QCIgray image}ie}BD/di{gs TR 1 i 1 eq{pop pop false 3 1 roll\n"
"BCol SC imagemask}{dup false ne{level3}{false}ie{/_ma ED 8 eq{/_dc[0 1]def\n"
"/DeviceGray}{/_dc[0 1 0 1 0 1]def/DeviceRGB}ie scs/_im ED/_mt ED/_h ED/_w ED\n"
"<</ImageType 3/DataDict <</ImageType 1/Width _w/Height _h/ImageMatrix _mt\n"
"/DataSource _im/BitsPerComponent 8/Decode _dc >>/MaskDict <</ImageType 1\n"
"/Width _w/Height _h/ImageMatrix _mt/DataSource _ma/BitsPerComponent 1/Decode\n"
"[0 1]>>/InterleaveType 3 >> image}{pop 8 4 1 roll 8 eq{image}{QCI}ie}ie}ie\n"
"gr}BD/BF{gs BSt 1 eq{BCol SC WFi{fill}{eofill}ie}if BSt 2 ge BSt 8 le and{\n"
"BDArr BSt 2 sub get/_sc ED BCol{1. exch sub _sc mul 1. exch sub}forall 3\n"
"array astore SC WFi{fill}{eofill}ie}if BSt 9 ge BSt 14 le and{WFi{W}{W*}ie\n"
"pathbbox 3 i 3 i TR 4 2 roll 3 2 roll exch sub/_h ED sub/_w ED BCol SC 0.3 w\n"
"NP BSt 9 eq BSt 11 eq or{0 4 _h{dup 0 exch m _w exch l}for}if BSt 10 eq BSt\n"
"11 eq or{0 4 _w{dup 0 m _h l}for}if BSt 12 eq BSt 14 eq or{_w _h gt{0 6 _w\n"
"_h add{dup 0 m _h sub _h l}for}{0 6 _w _h add{dup 0 exch m _w sub _w exch l}\n"
"for}ie}if BSt 13 eq BSt 14 eq or{_w _h gt{0 6 _w _h add{dup _h m _h sub 0 l}\n"
"for}{0 6 _w _h add{dup _w exch m _w sub 0 exch l}for}ie}if S}if BSt 15 eq{}\n"
"if BSt 24 eq{}if gr}BD/f{/WFi true def BF}BD/f*{/WFi false def BF}BD/B{/WFi\n"
"true def BF S}BD/B*{/WFi false def BF S}BD/BC{/BkCol ED}BD/BR{/BCol ED/BSt\n"
"ED}BD/NB{0[0 0 0]BR}BD/PE{setlinejoin setlinecap/PCol ED/LWi ED/PSt ED PCol\n"
"SC}BD/P1{1 0 3 2 roll 0 0 PE}BD/ST{defM SM concat}BD/MF{true exch true exch{\n"
"exch pop exch pop dup 0 get dup findfont dup/FontName get 3 -1 roll eq{exit}\n"
"if}forall exch dup 1 get/fxscale ED 2 get/fslant ED exch/fencoding ED[\n"
"fxscale 0 fslant 1 0 0]makefont fencoding false eq{}{dup maxlength dict\n"
"begin{1 i/FID ne{def}{pop pop}ifelse}forall/Encoding fencoding def\n"
"currentdict end}ie definefont pop}BD/MFEmb{findfont dup length dict begin{1\n"
"i/FID ne{def}{pop pop}ifelse}forall/Encoding ED currentdict end definefont\n"
"pop}BD/QI{/C save def pageinit q}BD/QP{Q C restore showpage}BD/SPD{\n"
"/setpagedevice where{<< 3 1 roll >> setpagedevice}{pop pop}ie}BD\n";


#define MM(n) int((n * 720 + 127) / 254)
#define IN(n) int(n * 72)

struct PaperSize {
    int width, height;
};

static const PaperSize paperSizes[QPrinter::NPageSize] =
{
    {  MM(210), MM(297) },      // A4
    {  MM(176), MM(250) },      // B5
    {  IN(8.5), IN(11) },       // Letter
    {  IN(8.5), IN(14) },       // Legal
    {  IN(7.5), IN(10) },       // Executive
    {  MM(841), MM(1189) },     // A0
    {  MM(594), MM(841) },      // A1
    {  MM(420), MM(594) },      // A2
    {  MM(297), MM(420) },      // A3
    {  MM(148), MM(210) },      // A5
    {  MM(105), MM(148) },      // A6
    {  MM(74), MM(105)},        // A7
    {  MM(52), MM(74) },        // A8
    {  MM(37), MM(52) },        // A9
    {  MM(1000), MM(1414) },    // B0
    {  MM(707), MM(1000) },     // B1
    {  MM(31), MM(44) },        // B10
    {  MM(500), MM(707) },      // B2
    {  MM(353), MM(500) },      // B3
    {  MM(250), MM(353) },      // B4
    {  MM(125), MM(176) },      // B6
    {  MM(88), MM(125) },       // B7
    {  MM(62), MM(88) },        // B8
    {  MM(44), MM(62) },        // B9
    {  MM(162),    MM(229) },   // C5E
    {  IN(4.125),  IN(9.5) },   // Comm10E
    {  MM(110),    MM(220) },   // DLE
    {  IN(8.5),    IN(13) },    // Folio
    {  IN(17),     IN(11) },    // Ledger
    {  IN(11),     IN(17) }     // Tabloid
};


#if 0
// ---------------------------------------------------------------------
// postscript font substitution dictionary. We assume every postscript printer has at least
// Helvetica, Times, Courier and Symbol

#if defined (Q_WS_WIN) && defined (Q_CC_MSVC)
#  pragma warning(disable: 4305)
#endif

struct psfont {
    const char *psname;
    qreal slant;
    qreal xscale;
};

static const psfont Arial[] = {
    { "Arial", 0, 84.04 },
    { "Arial-Italic", 0, 84.04 },
    { "Arial-Bold", 0, 88.65 },
    { "Arial-BoldItalic", 0, 88.65 }
};

static const psfont AvantGarde[] = {
    { "AvantGarde-Book", 0, 87.43 },
    { "AvantGarde-BookOblique", 0, 88.09 },
    { "AvantGarde-Demi", 0, 88.09 },
    { "AvantGarde-DemiOblique", 0, 87.43 },
};

static const psfont Bookman [] = {
    { "Bookman-Light", 0, 93.78 },
    { "Bookman-LightItalic", 0, 91.42 },
    { "Bookman-Demi", 0, 99.86 },
    { "Bookman-DemiItalic", 0, 101.54 }
};

static const psfont Charter [] = {
    { "CharterBT-Roman", 0, 84.04 },
    { "CharterBT-Italic", 0.0, 81.92 },
    { "CharterBT-Bold", 0, 88.99 },
    { "CharterBT-BoldItalic", 0.0, 88.20 }
};

static const psfont Courier [] = {
    { "Courier", 0, 100. },
    { "Courier-Oblique", 0, 100. },
    { "Courier-Bold", 0, 100. },
    { "Courier-BoldOblique", 0, 100. }
};

static const psfont Garamond [] = {
    { "Garamond-Antiqua", 0, 78.13 },
    { "Garamond-Kursiv", 0, 78.13 },
    { "Garamond-Halbfett", 0, 78.13 },
    { "Garamond-KursivHalbfett", 0, 78.13 }
};

static const psfont GillSans [] = { // estimated value for xstretch
    { "GillSans", 0, 82 },
    { "GillSans-Italic", 0, 82 },
    { "GillSans-Bold", 0, 82 },
    { "GillSans-BoldItalic", 0, 82 }
};

static const psfont Helvetica [] = {
    { "Helvetica", 0, 84.04 },
    { "Helvetica-Oblique", 0, 84.04 },
    { "Helvetica-Bold", 0, 88.65 },
    { "Helvetica-BoldOblique", 0, 88.65 }
};

static const psfont Letter [] = {
    { "LetterGothic", 0, 83.32 },
    { "LetterGothic-Italic", 0, 83.32 },
    { "LetterGothic-Bold", 0, 83.32 },
    { "LetterGothic-Bold", 0.2, 83.32 }
};

static const psfont LucidaSans [] = {
    { "LucidaSans", 0, 94.36 },
    { "LucidaSans-Oblique", 0, 94.36 },
    { "LucidaSans-Demi", 0, 98.10 },
    { "LucidaSans-DemiOblique", 0, 98.08 }
};

static const psfont LucidaSansTT [] = {
    { "LucidaSans-Typewriter", 0, 100.50 },
    { "LucidaSans-TypewriterOblique", 0, 100.50 },
    { "LucidaSans-TypewriterBold", 0, 100.50 },
    { "LucidaSans-TypewriterBoldOblique", 0, 100.50 }
};

static const psfont LucidaBright [] = {
    { "LucidaBright", 0, 93.45 },
    { "LucidaBright-Italic", 0, 91.98 },
    { "LucidaBright-Demi", 0, 96.22 },
    { "LucidaBright-DemiItalic", 0, 96.98 }
};

static const psfont Palatino [] = {
    { "Palatino-Roman", 0, 82.45 },
    { "Palatino-Italic", 0, 76.56 },
    { "Palatino-Bold", 0, 83.49 },
    { "Palatino-BoldItalic", 0, 81.51 }
};

static const psfont Symbol [] = {
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 },
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 }
};

static const psfont Tahoma [] = {
    { "Tahoma", 0, 83.45 },
    { "Tahoma", 0.2, 83.45 },
    { "Tahoma-Bold", 0, 95.59 },
    { "Tahoma-Bold", 0.2, 95.59 }
};

static const psfont Times [] = {
    { "Times-Roman", 0, 82.45 },
    { "Times-Italic", 0, 82.45 },
    { "Times-Bold", 0, 82.45 },
    { "Times-BoldItalic", 0, 82.45 }
};

static const psfont Verdana [] = {
    { "Verdana", 0, 96.06 },
    { "Verdana-Italic", 0, 96.06 },
    { "Verdana-Bold", 0, 107.12 },
    { "Verdana-BoldItalic", 0, 107.10 }
};

static const psfont Utopia [] = {
    { "Utopia-Regular", 0, 84.70 },
    { "Utopia-Regular", 0.2, 84.70 },
    { "Utopia-Bold", 0, 88.01 },
    { "Utopia-Bold", 0.2, 88.01 }
};

static const psfont * const SansSerifReplacements[] = {
    Helvetica, 0
        };
static const psfont * const SerifReplacements[] = {
    Times, 0
        };
static const psfont * const FixedReplacements[] = {
    Courier, 0
        };
static const psfont * const TahomaReplacements[] = {
    Verdana, AvantGarde, Helvetica, 0
        };
static const psfont * const VerdanaReplacements[] = {
    Tahoma, AvantGarde, Helvetica, 0
        };

static const struct {
    const char * input; // spaces are stripped in here, and everything lowercase
    const psfont * ps;
    const psfont *const * replacements;
} postscriptFonts [] = {
    { "arial", Arial, SansSerifReplacements },
    { "arialmt", Arial, SansSerifReplacements },
    { "arialunicodems", Arial, SansSerifReplacements },
    { "avantgarde", AvantGarde, SansSerifReplacements },
    { "bookman", Bookman, SerifReplacements },
    { "charter", Charter, SansSerifReplacements },
    { "bitstreamcharter", Charter, SansSerifReplacements },
    { "bitstreamcyberbit", Times, SerifReplacements },
    { "courier", Courier, 0 },
    { "couriernew", Courier, 0 },
    { "fixed", Courier, 0 },
    { "garamond", Garamond, SerifReplacements },
    { "gillsans", GillSans, SansSerifReplacements },
    { "helvetica", Helvetica, 0 },
    { "letter", Letter, FixedReplacements },
    { "lucida", LucidaSans, SansSerifReplacements },
    { "lucidasans", LucidaSans, SansSerifReplacements },
    { "lucidabright", LucidaBright, SerifReplacements },
    { "lucidasanstypewriter", LucidaSansTT, FixedReplacements },
    { "luciduxsans", LucidaSans, SansSerifReplacements },
    { "luciduxserif", LucidaBright, SerifReplacements },
    { "luciduxmono", LucidaSansTT, FixedReplacements },
    { "palatino", Palatino, SerifReplacements },
    { "symbol", Symbol, 0 },
    { "tahoma", Tahoma, TahomaReplacements },
    { "terminal", Courier, 0 },
    { "times", Times, 0 },
    { "timesnewroman", Times, 0 },
    { "verdana", Verdana, VerdanaReplacements },
    { "utopia", Utopia, SerifReplacements },
    { 0, 0, 0 }
};
#endif

// ------------------------------End of static data ----------------------------------

// make sure DSC comments are not longer than 255 chars per line.
static QByteArray wrapDSC(const QByteArray &str)
{
    QByteArray dsc = str.simplified();
    const int wrapAt = 254;
    QByteArray wrapped;
    if (dsc.length() < wrapAt)
        wrapped = dsc;
    else {
        wrapped = dsc.left(wrapAt);
        QByteArray tmp = dsc.mid(wrapAt);
        while (tmp.length() > wrapAt-3) {
            wrapped += "\n%%+" + tmp.left(wrapAt-3);
            tmp = tmp.mid(wrapAt-3);
        }
        wrapped += "\n%%+" + tmp;
    }
    return wrapped + "\n";
}

// ----------------------------- Internal class declarations -----------------------------

class QPSPrintEnginePrivate : public QPaintEnginePrivate {
public:
    QPSPrintEnginePrivate(QPrinter::PrinterMode m);
    ~QPSPrintEnginePrivate();

    void emitHeader(bool finished);
    void emitPages();
    void drawImage(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask);
    void flushPage(bool last = false);
    QRect paperRect() const;
    QRect pageRect() const;
    QRegion getClip();

    void drawTextItem(QPSPrintEngine *q, const QPointF &p, const QTextItemInt &ti);

    inline int requestObject() { return ++object; }
    int object;

    int         pageCount;
    bool        epsf;
    QByteArray     fontsUsed;

    // the device the output is in the end streamed to.
    QIODevice *outDevice;
    int fd;

    // stores the descriptions of the n first pages.
    QByteArray buffer;
    // buffer for the current page. Needed because we might have page fonts.
    QByteArray pageBuffer;
    QPdf::ByteStream *currentPage;

    QHash<QFontEngine::FaceId, QPdf::Font *> fonts;
    bool firstPage;

    QRect boundingBox;

    Qt::BGMode backgroundMode;
    QBrush backgroundBrush;
    QPointF brushOrigin;
    QBrush brush;
    QPen pen;
    QList<QPainterPath> clips;
    bool clipEnabled;
    bool allClipped;
    bool hasPen;
    bool hasBrush;

    QPdf::Stroker stroker;

    QStringList fontpath;

    bool        collate;
    int         copies;
    QString printerName;
    QString outputFileName;
    QString selectionOption;
    QString printProgram;
    QString title;
    QString creator;
    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    int resolution;
    QPrinter::ColorMode colorMode;
    bool fullPage;
    QPrinter::PaperSource paperSource;
    QPrinter::PrinterState printerState;
    bool embedFonts;
};


#if 0
#if defined(Q_WS_X11) && defined(QT_NO_FONTCONFIG)
static QStringList fontPath()
{
    // append qsettings fontpath
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    QStringList fontpath;

    int npaths;
    char** font_path;
    font_path = XGetFontPath(X11->display, &npaths);
    bool xfsconfig_read = false;
    for (int i=0; i<npaths; i++) {
        // If we're using xfs, append font paths from /etc/X11/fs/config
        // can't hurt, and chances are we'll get all fonts that way.
        if (((font_path[i])[0] != '/') && !xfsconfig_read) {
            // We're using xfs -> read its config
            bool finished = false;
            QFile f("/etc/X11/fs/config");
            if (!f.exists())
                f.setFileName("/usr/X11R6/lib/X11/fs/config");
            if (!f.exists())
                f.setFileName("/usr/X11/lib/X11/fs/config");
            if (f.exists()) {
                f.open(QIODevice::ReadOnly);
                while (f.error()==QFile::NoError && !finished) {
                    QString fs = f.readLine(1024);
                    fs=fs.trimmed();
                    if (fs.left(9)=="catalogue" && fs.contains('=')) {
                        fs = fs.mid(fs.indexOf('=') + 1).trimmed();
                        bool end = false;
                        while (f.error()==QFile::NoError && !end) {
                            if (fs[int(fs.length())-1] == ',')
                                fs = fs.left(fs.length()-1);
                            else
                                end = true;
                            if (fs[0] != '#' && !fs.contains(":unscaled"))
                                fontpath += fs;
                            fs = f.readLine(1024);
                            fs=fs.trimmed();
                            if (fs.isEmpty())
                                end = true;
                        }
                        finished = true;
                    }
                }
                f.close();
            }
            xfsconfig_read = true;
        } else if (!strstr(font_path[i], ":unscaled")) {
            // Fonts paths marked :unscaled are always bitmapped fonts
            // -> we can as well ignore them now and save time
            fontpath += font_path[i];
        }
    }
    XFreeFontPath(font_path);

    // append qsettings fontpath
    QStringList fp = settings.value(QLatin1String("fontPath")).toStringList();
    if (!fp.isEmpty())
        fontpath += fp;

    return fontpath;
}

static QString fontFile(const QStringList &fontpath, const QByteArray &xname)
{
    QByteArray searchname = xname.toLower();
    QString fontfilename;
    for (QStringList::ConstIterator it = fontpath.constBegin(); it != fontpath.constEnd(); ++it) {
        if ((*it).left(1) != "/")
            continue; // not a path name, a font server
        QString fontmapname;
        int num = 0;
        // search font.dir and font.scale for the right file
        while (num < 2) {
            if (num == 0)
                fontmapname = (*it) + "/fonts.scale";
            else
                fontmapname = (*it) + "/fonts.dir";
            ++num;
            //qWarning(fontmapname);
            QFile fontmap(fontmapname);
            if (!fontmap.open(QIODevice::ReadOnly))
                continue;
            while (!fontmap.atEnd()) {
                QByteArray mapping = fontmap.readLine();
                // fold to lower (since X folds to lowercase)
                //qWarning(xfontname);
                //qWarning(mapping);
                if (!mapping.toLower().contains(searchname))
                    continue;
                int index = mapping.indexOf(' ');
                QString ffn = mapping.mid(0,index);
                // remove the most common bitmap formats
                if(ffn.contains(".pcf") || ffn.contains(".bdf") || ffn.contains(".spd") || ffn.contains(".phont"))
                    continue;
                fontfilename = (*it) + QString("/") + ffn;
                if (QFile::exists(fontfilename)) {
                    // ############ check if scalable
                    goto end;
                }
                fontfilename = QString();
            }
            fontmap.close();
        }
    }
end:
    return fontfilename;
}
#endif
#endif

QPSPrintEnginePrivate::QPSPrintEnginePrivate(QPrinter::PrinterMode m)
    : object(0), outDevice(0), fd(-1), currentPage(0),
      collate(false), copies(1), orientation(QPrinter::Portrait),
      pageSize(QPrinter::A4), pageOrder(QPrinter::FirstPageFirst), colorMode(QPrinter::Color),
      fullPage(false), printerState(QPrinter::Idle)
{
    backgroundMode = Qt::TransparentMode;

    firstPage = true;
    resolution = 72;
    if (m == QPrinter::HighResolution)
        resolution = 1200;
#ifdef Q_WS_X11
    else if (m == QPrinter::ScreenResolution)
        resolution = QX11Info::appDpiY();
#endif

#ifndef QT_NO_SETTINGS
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));
    embedFonts = settings.value(QLatin1String("embedFonts"), true).toBool();
#else
    embedFonts = true;
#endif
#if defined(Q_WS_X11) && defined(QT_NO_FONTCONFIG)
    if (!X11->use_xrender)
        fontpath = ::fontPath();
#endif
}

QPSPrintEnginePrivate::~QPSPrintEnginePrivate()
{
    qDeleteAll(fonts);
    delete currentPage;
}

void QPSPrintEnginePrivate::drawTextItem(QPSPrintEngine *q, const QPointF &p, const QTextItemInt &ti)
{
    QFontEngine *fe = ti.fontEngine;

    QFontEngine::FaceId face_id = fe->faceId();
    if (face_id.filename.isEmpty()) {
        *currentPage << "Q\n";
        q->QPaintEngine::drawTextItem(p, ti);
        *currentPage << "q\n";
        return;
    }

    QPdf::Font *font = fonts.value(face_id, 0);
    if (!font)
        font = new QPdf::Font(fe, requestObject());
    fonts.insert(face_id, font);

//     if (!currentPage->fonts.contains(font->object_id))
//         currentPage->fonts.append(font->object_id);

    qreal size;
#ifdef Q_WS_WIN
    size = ti.fontEngine->tm.w.tmHeight;
#else
    size = ti.fontEngine->fontDef.pixelSize;
#endif

    QVarLengthArray<glyph_t> glyphs;
    QVarLengthArray<QFixedPoint> positions;
    QMatrix m;
    m.translate(p.x(), p.y());
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, m, ti.flags,
                                     glyphs, positions);
    int synthesized = ti.fontEngine->synthesized();
    qreal stretch = synthesized & QFontEngine::SynthesizedStretch ? ti.fontEngine->fontDef.stretch/100. : 1.;

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut|QTextItem::Overline)) {
        *currentPage << "NP ";
        qreal lw = fe->lineThickness().toReal();
        if (ti.flags & (QTextItem::Underline))
            *currentPage << p.x() << (p.y() + fe->underlinePosition().toReal())
                           << ti.width.toReal() << lw << "re ";
        if (ti.flags & (QTextItem::StrikeOut))
            *currentPage  << p.x() << (p.y() - fe->ascent().toReal()/3.)
                            << ti.width.toReal() << lw << "re ";
        if (ti.flags & (QTextItem::Overline))
            *currentPage  << p.x() << (p.y() - fe->ascent().toReal())
                            << ti.width.toReal() << lw << "re ";
        *currentPage << "f\n";
    }
    *currentPage << "BT\n"
                 << "/F" << font->object_id << size << "Tf "
                 << stretch << (synthesized & QFontEngine::SynthesizedItalic
                                ? "0 .3 -1 0 0 Tm\n"
                                : "0 0 -1 0 0 Tm\n");


    qreal last_x = 0.;
    qreal last_y = 0.;
    for (int i = 0; i < glyphs.size(); ++i) {
        qreal x = positions[i].x.toReal();
        qreal y = positions[i].y.toReal();
        if (synthesized & QFontEngine::SynthesizedItalic)
            x += .3*y;
        x /= stretch;
        char buf[5];
        int g = font->addGlyph(glyphs[i]);
        *currentPage << x - last_x << last_y - y << "Td <"
                     << QPdf::toHex((ushort)g, buf) << "> Tj\n";
        last_x = x;
        last_y = y;
    }
    if (synthesized & QFontEngine::SynthesizedBold) {
        *currentPage << stretch << (synthesized & QFontEngine::SynthesizedItalic
                            ? "0 .3 -1 0 0 Tm\n"
                            : "0 0 -1 0 0 Tm\n");
        *currentPage << "/Span << /ActualText <> >> BDC\n";
        last_x = 0.5*fe->lineThickness().toReal();
        last_y = 0.;
        for (int i = 0; i < glyphs.size(); ++i) {
            qreal x = positions[i].x.toReal();
            qreal y = positions[i].y.toReal();
            if (synthesized & QFontEngine::SynthesizedItalic)
                x += .3*y;
            x /= stretch;
            char buf[5];
            int g = font->addGlyph(glyphs[i]);
            *currentPage << x - last_x << last_y - y << "Td <"
                        << QPdf::toHex((ushort)g, buf) << "> Tj\n";
            last_x = x;
            last_y = y;
        }
        *currentPage << "EMC\n";
    }

    *currentPage << "ET\n";
}


static void ps_r7(QPdf::ByteStream& stream, const char * s, int l)
{
    int i = 0;
    uchar line[80];
    int col = 0;

    while(i < l) {
        line[col++] = s[i++];
        if (i < l - 1 && col >= 76) {
            line[col++] = '\n';
            line[col++] = '\0';
            stream << (const char *)line;
            col = 0;
        }
    }
    if (col > 0) {
        while((col&3) != 0)
            line[col++] = '%'; // use a comment as padding
        line[col++] = '\n';
        line[col++] = '\0';
        stream << (const char *)line;
    }
}

static QByteArray runlengthEncode(const QByteArray &input)
{
    if (!input.length())
        return input;

    const char *data = input.constData();

    QByteArray out;
    int start = 0;
    char last = *data;

    enum State {
        Undef,
        Equal,
        Diff
    };
    State state = Undef;

    int i = 1;
    int written = 0;
    while (1) {
        bool flush = (i == input.size());
        if (!flush) {
            switch(state) {
            case Undef:
                state = (last == data[i]) ? Equal : Diff;
                break;
            case Equal:
                if (data[i] != last)
                    flush = true;
                break;
            case Diff:
                if (data[i] == last) {
                    --i;
                    flush = true;
                }
            }
        }
        if (flush || i - start == 128) {
            int size = i - start;
            if (state == Equal) {
                out.append((char)(uchar)(257-size));
                out.append(last);
                written += size;
            } else {
                out.append((char)(uchar)size-1);
                while (start < i)
                    out.append(data[start++]);
                written += size;
            }
            state = Undef;
            start = i;
            if (i == input.size())
                break;
        }
        last = data[i];
        ++i;
    };
    out.append((char)(uchar)128);
    return out;
}

enum format {
    Raw,
    Runlength,
    DCT
};
static const char *filters[3] const = {
    " ",
    "/RunLengthDecode filter ",
    "/DCTDecode filter "
};

static QByteArray compress(const QImage &img, bool gray, int *format)
{
    // we can't use premultiplied here
    QImage image = img;

    if (image.format() == QImage::Format_ARGB32_Premultiplied)
        image = image.convertToFormat(QImage::Format_ARGB32);

    QByteArray pixelData;
    int depth = image.depth();

    if (depth != 1 && !gray && QImageWriter::supportedImageFormats().contains("jpeg")) {
        QBuffer buffer(&pixelData);
        QImageWriter writer(&buffer, "jpeg");
        writer.setQuality(94);
        writer.write(img);
        *format = DCT;
    } else {
        int width = image.width();
        int height = image.height();
        int size = width*height;

        if (depth == 1)
            size = (width+7)/8*height;
        else if (!gray)
            size = size*3;

        pixelData.resize(size);
        uchar *pixel = (uchar *)pixelData.data();
        int i = 0;
        if (depth == 1) {
            QImage::Format format = image.format();
            memset(pixel, 0xff, size);
            for(int y=0; y < height; y++) {
                const uchar * s = image.scanLine(y);
                for(int x=0; x < width; x++) {
                    // need to copy bit for bit...
                    bool b = (format == QImage::Format_MonoLSB) ?
                             (*(s + (x >> 3)) >> (x & 7)) & 1 :
                             (*(s + (x >> 3)) << (x & 7)) & 0x80 ;
                    if (b)
                        pixel[i >> 3] ^= (0x80 >> (i & 7));
                    i++;
                }
                // we need to align to 8 bit here
                i = (i+7) & 0xffffff8;
            }
        } else if (depth == 8) {
            for(int y=0; y < height; y++) {
                const uchar * s = image.scanLine(y);
                for(int x=0; x < width; x++) {
                    QRgb rgb = image.color(s[x]);
                    if (gray) {
                        pixel[i] = (unsigned char) qGray(rgb);
                        i++;
                    } else {
                        pixel[i] = (unsigned char) qRed(rgb);
                        pixel[i+1] = (unsigned char) qGreen(rgb);
                        pixel[i+2] = (unsigned char) qBlue(rgb);
                        i += 3;
                    }
                }
            }
        } else {
            for(int y=0; y < height; y++) {
                QRgb * s = (QRgb*)(image.scanLine(y));
                for(int x=0; x < width; x++) {
                    QRgb rgb = (*s++);
                    if (gray) {
                        pixel[i] = (unsigned char) qGray(rgb);
                        i++;
                    } else {
                        pixel[i] = (unsigned char) qRed(rgb);
                        pixel[i+1] = (unsigned char) qGreen(rgb);
                        pixel[i+2] = (unsigned char) qBlue(rgb);
                        i += 3;
                    }
                }
            }
        }
        *format = Raw;
        if (depth == 1) {
            pixelData = runlengthEncode(pixelData);
            *format = Runlength;
        }
    }
    QByteArray outarr = QPdf::ascii85Encode(pixelData);
    return outarr;
}


void QPSPrintEnginePrivate::drawImage(qreal x, qreal y, qreal w, qreal h,
                                      const QImage &img, const QImage &mask)
{
    if (!w || !h || img.isNull()) return;

    int width  = img.width();
    int height = img.height();
    qreal scaleX = width/w;
    qreal scaleY = height/h;

    bool gray = (colorMode == QPrinter::GrayScale) ||
                img.allGray();
    int splitSize = 21830 * (gray ? 3 : 1);
    if (width * height > splitSize) { // 65535/3, tolerance for broken printers
        int images, subheight;
        images = (width * height + splitSize - 1) / splitSize;
        subheight = (height + images-1) / images;
        while (subheight * width > splitSize) {
            images++;
            subheight = (height + images-1) / images;
        }
        int suby = 0;
        while(suby < height) {
            drawImage(x, y + suby/scaleY, w, qMin(subheight, height-suby)/scaleY,
                      img.copy(0, suby, width, qMin(subheight, height-suby)),
                      mask.isNull() ? mask : mask.copy(0, suby, width, qMin(subheight, height-suby)));
            suby += subheight;
        }
    } else {
        QByteArray out;
        int size = 0;
        const char *bits;

        if (!mask.isNull()) {
            int format;
            out = ::compress(mask, true, &format);
            size = (width+7)/8*height;
            *currentPage << "/mask currentfile/ASCII85Decode filter"
                         << filters[format]
                         << size << " string readstring\n";
            ps_r7(*currentPage, out, out.size());
            *currentPage << " pop def\n";
        }
        if (img.depth() == 1) {
            size = (width+7)/8*height;
            bits = "1 ";
        } else if (gray) {
            size = width*height;
            bits = "8 ";
        } else {
            size = width*height*3;
            bits = "24 ";
        }

        int format;
        out = ::compress(img, gray, &format);
        *currentPage << "/sl currentfile/ASCII85Decode filter"
                     << filters[format]
                     << size << " string readstring\n";
        ps_r7(*currentPage, out, out.size());
        *currentPage << " pop def\n";
        *currentPage << width << ' ' << height << "[" << scaleX << " 0 0 " << scaleY << " 0 0]sl "
                    << bits << (!mask.isNull() ? "mask " : "false ")
                    << x << ' ' << y << " di\n";
    }
}

void QPSPrintEnginePrivate::emitHeader(bool finished)
{
    QPSPrintEngine *q = static_cast<QPSPrintEngine *>(q_ptr);
    QPrinter *printer = static_cast<QPrinter*>(pdev);

    if (creator.isEmpty())
        creator = QLatin1String("Qt " QT_VERSION_STR);
    outDevice = new QFile();
    static_cast<QFile *>(outDevice)->open(fd, QIODevice::WriteOnly);

    QByteArray header;
    QPdf::ByteStream s(&header);
    s << "%!PS-Adobe-1.0";

    qreal scale = 72. / ((qreal) q->metric(QPaintDevice::PdmDpiY));
    QRect pageRect = this->pageRect();
    QRect paperRect = this->paperRect();
    uint mtop = pageRect.top() - paperRect.top();
    uint mleft = pageRect.left() - paperRect.left();
    uint mbottom = paperRect.bottom() - pageRect.bottom();
    uint mright = paperRect.right() - pageRect.right();
    int width = pageRect.width();
    int height = pageRect.height();
    if (finished && pageCount == 1 && copies == 1 &&
        ((fullPage && qt_gen_epsf) || (outputFileName.endsWith(".eps")))
       ) {
        if (!boundingBox.isValid())
            boundingBox.setRect(0, 0, width, height);
        if (orientation == QPrinter::Landscape) {
            if (!fullPage)
                boundingBox.translate(-mleft, -mtop);
            s << " EPSF-3.0\n%%BoundingBox: "
              << (int)(printer->height() - boundingBox.bottom())*scale // llx
              << (int)(printer->width() - boundingBox.right())*scale - 1 // lly
              << (int)(printer->height() - boundingBox.top())*scale + 1 // urx
              << (int)(printer->width() - boundingBox.left())*scale; // ury
        } else {
            if (!fullPage)
                boundingBox.translate(mleft, -mtop);
            s << " EPSF-3.0\n%%BoundingBox: "
              << (int)(boundingBox.left())*scale
              << (int)(printer->height() - boundingBox.bottom())*scale - 1
              << (int)(boundingBox.right())*scale + 1
              << (int)(printer->height() - boundingBox.top())*scale;
        }
    } else {
        int w = width + (fullPage ? 0 : mleft + mright);
        int h = height + (fullPage ? 0 : mtop + mbottom);
        w = (int)(w*scale);
        h = (int)(h*scale);
        // set a bounding box according to the DSC
        if (orientation == QPrinter::Landscape)
            s << "\n%%BoundingBox: 0 0 " << h << w;
        else
            s << "\n%%BoundingBox: 0 0 " << w << h;
    }
    s << "\n" << wrapDSC("%%Creator: " + creator.toUtf8());
    if (!title.isEmpty())
        s << wrapDSC("%%Title: " + title.toUtf8());
#ifndef QT_NO_DATESTRING
    s << "%%CreationDate: " << QDateTime::currentDateTime().toString().toUtf8();
#endif
    s << "\n%%Orientation: ";
    if (orientation == QPrinter::Landscape)
        s << "Landscape";
    else
        s << "Portrait";

    s << "\n%%Pages: (atend)"
        "\n%%DocumentFonts: (atend)"
        "\n%%EndComments\n"

        "%%BeginProlog\n"
        "% Prolog copyright 1994-2003 Trolltech. You may copy this prolog in any way\n"
        "% that is directly related to this document. For other use of this prolog,\n"
        "% see your licensing agreement for Qt.\n"
      << ps_header << "\n";


    s << "/pageinit {\n";
    if (!fullPage) {
        if (orientation == QPrinter::Portrait)
            s << mleft*scale << mbottom*scale << "translate\n";
        else
            s << mtop*scale << mleft*scale << "translate\n";
    }
    if (orientation == QPrinter::Portrait) {
        s << "% " << printer->widthMM() << "*" << printer->heightMM()
          << "mm (portrait)\n0 " << height*scale
          << "translate " << scale << "-" << scale << "scale/defM matrix CM def } def\n";
    } else {
        s << "% " << printer->heightMM() << "*" << printer->widthMM()
          << " mm (landscape)\n 90 rotate " << scale << "-" << scale << "scale/defM matrix CM def } def\n";
    }
    s << "%%EndProlog\n";


    s << "%%BeginSetup\n";
    if (copies > 1) {
        s << "/#copies " << copies << " def\n";
        s << "/NumCopies " << copies << " SPD\n";
        s << "/Collate " << (collate ? "true" : "false") << " SPD\n";
    }
    s << "%%EndSetup\n";

    outDevice->write(header);

}


void QPSPrintEnginePrivate::emitPages()
{
    // ############# fix fonts for huge documents
    for (QHash<QFontEngine::FaceId, QPdf::Font *>::Iterator it = fonts.begin(); it != fonts.end(); ++it)
        outDevice->write((*it)->toType1());

    outDevice->write(buffer);

    buffer = QByteArray();
    qDeleteAll(fonts);
    fonts.clear();
}


#ifdef Q_WS_QWS
const int max_in_memory_size = 50000000;
#else
const int max_in_memory_size = 2000000;
#endif

void QPSPrintEnginePrivate::flushPage(bool last)
{
    if (!last && pageBuffer.isEmpty())
        return;
    QPdf::ByteStream s(&buffer);
    s << "%%Page: "
      << pageCount << pageCount << "\n"
      << "QI\n"
      << pageBuffer
      << "\nQP\n";
    pageBuffer = QByteArray();
    if (last) { // ############## || buffer.size() > max_in_memory_size) {
//        qDebug("emiting header at page %d", pageCount);
        if (!outDevice)
            emitHeader(last);
        emitPages();
    }
    pageCount++;
}

// ================ PSPrinter class ========================

QPSPrintEngine::QPSPrintEngine(QPrinter::PrinterMode m)
    : QPaintEngine(*(new QPSPrintEnginePrivate(m)),
                   PrimitiveTransform
                   | PatternTransform
                   | PixmapTransform
                   | PainterPaths
                   | PatternBrush
       )
{
}


static void ignoreSigPipe(bool b)
{
    static struct sigaction *users_sigpipe_handler = 0;

    if (b) {
        if (users_sigpipe_handler != 0)
            return; // already ignoring sigpipe

        users_sigpipe_handler = new struct sigaction;
        struct sigaction tmp_sigpipe_handler;
        tmp_sigpipe_handler.sa_handler = SIG_IGN;
        sigemptyset(&tmp_sigpipe_handler.sa_mask);
        tmp_sigpipe_handler.sa_flags = 0;

        if (sigaction(SIGPIPE, &tmp_sigpipe_handler, users_sigpipe_handler) == -1) {
            delete users_sigpipe_handler;
            users_sigpipe_handler = 0;
        }
    }
    else {
        if (users_sigpipe_handler == 0)
            return; // not ignoring sigpipe

        if (sigaction(SIGPIPE, users_sigpipe_handler, 0) == -1)
            qWarning("QPSPrintEngine: could not restore SIGPIPE handler");

        delete users_sigpipe_handler;
        users_sigpipe_handler = 0;
    }
}

QPSPrintEngine::~QPSPrintEngine()
{
    Q_D(QPSPrintEngine);
    if (d->fd >= 0)
        ::close(d->fd);
}

static const char * const psToStr[QPrinter::NPageSize+1] =
{
    "A4", "B5", "Letter", "Legal", "Executive",
    "A0", "A1", "A2", "A3", "A5", "A6", "A7", "A8", "A9", "B0", "B1",
    "B10", "B2", "B3", "B4", "B6", "B7", "B8", "B9", "C5E", "Comm10E",
    "DLE", "Folio", "Ledger", "Tabloid", 0
};

static void closeAllOpenFds()
{
    // hack time... getting the maximum number of open
    // files, if possible.  if not we assume it's the
    // larger of 256 and the fd we got
    int i;
#if defined(_SC_OPEN_MAX)
    i = (int)sysconf(_SC_OPEN_MAX);
#elif defined(_POSIX_OPEN_MAX)
    i = (int)_POSIX_OPEN_MAX;
#elif defined(OPEN_MAX)
    i = (int)OPEN_MAX;
#else
    i = 256;
#endif
    while(--i > 0)
	::close(i);
}

bool QPSPrintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QPSPrintEngine);

    if (d->fd >= 0)
        return true;

    d->pdev = pdev;
    if (!d->outputFileName.isEmpty()) {
        d->fd = QT_OPEN(d->outputFileName.toLocal8Bit().constData(), O_CREAT | O_NOCTTY | O_TRUNC | O_WRONLY,
#if defined(Q_OS_WIN)
            _S_IREAD | _S_IWRITE
#else
            0666
#endif
           );
    } else {
        QString pr;
        if (!d->printerName.isEmpty())
            pr = d->printerName;
        int fds[2];
        if (pipe(fds) != 0) {
            qWarning("QPSPrinter: could not open pipe to print");
            return false;
        }

        pid_t pid = fork();
        if (pid == 0) {       // child process
            // if possible, exit quickly, so the actual lp/lpr
            // becomes a child of init, and ::waitpid() is
            // guaranteed not to wait.
            if (fork() > 0) {
                closeAllOpenFds();

                // try to replace this process with "true" - this prevents
                // global destructors from being called (that could possibly
                // do wrong things to the parent process)
                (void)execlp("true", "true", (char *)0);
                (void)execl("/bin/true", "true", (char *)0);
                (void)execl("/usr/bin/true", "true", (char *)0);
                ::exit(0);
            }
            dup2(fds[0], 0);

            closeAllOpenFds();

            if (!d->printProgram.isEmpty()) {
                if (!d->selectionOption.isEmpty())
                    pr.prepend(d->selectionOption);
                else
                    pr.prepend(QLatin1String("-P"));
                (void)execlp(d->printProgram.toLocal8Bit().data(), d->printProgram.toLocal8Bit().data(),
                              pr.toLocal8Bit().data(), (char *)0);
            } else {
                // if no print program has been specified, be smart
                // about the option string too.
                QList<QByteArray> lprhack;
                QList<QByteArray> lphack;
                QByteArray media;
                if (!pr.isEmpty() || !d->selectionOption.isEmpty()) {
                    if (!d->selectionOption.isEmpty()) {
                        QStringList list = d->selectionOption.split(QChar(' '));
                        for (int i = 0; i < list.size(); ++i)
                            lprhack.append(list.at(i).toLocal8Bit());
                        lphack = lprhack;
                    } else {
                        lprhack.append("-P");
                        lphack.append("-d");
                    }
                    lprhack.append(pr.toLocal8Bit());
                    lphack.append(pr.toLocal8Bit());
                }
                char ** lpargs = new char *[lphack.size()+6];
                lpargs[0] = "lp";
                int i;
                for (i = 0; i < lphack.size(); ++i)
                    lpargs[i+1] = (char *)lphack.at(i).constData();
#ifndef Q_OS_OSF
                if (psToStr[d->pageSize]) {
                    lpargs[++i] = "-o";
                    lpargs[++i] = (char *)psToStr[d->pageSize];
                    lpargs[++i] = "-o";
                    media = "media=";
                    media += psToStr[d->pageSize];
                    lpargs[++i] = (char *)media.constData();
                }
#endif
                lpargs[++i] = 0;
                char **lprargs = new char *[lprhack.size()+2];
                lprargs[0] = "lpr";
                for (int i = 0; i < lprhack.size(); ++i)
                    lprargs[i+1] = (char *)lprhack[i].constData();
                lprargs[lprhack.size() + 1] = 0;
                (void)execvp("lp", lpargs);
                (void)execvp("lpr", lprargs);
                (void)execv("/bin/lp", lpargs);
                (void)execv("/bin/lpr", lprargs);
                (void)execv("/usr/bin/lp", lpargs);
                (void)execv("/usr/bin/lpr", lprargs);
            }
            // if we couldn't exec anything, close the fd,
            // wait for a second so the parent process (the
            // child of the GUI process) has exited.  then
            // exit.
            ::close(0);
            (void)::sleep(1);
            ::exit(0);
        }
        // parent process
        ::close(fds[0]);
        d->fd = fds[1];
    }
    if (d->fd < 0)
        return false;

    d->pageCount = 1;                // initialize state

    d->pen = QPen(Qt::black);
    d->brush = Qt::NoBrush;
    d->hasPen = true;
    d->hasBrush = false;
    d->clipEnabled = false;
    d->allClipped = false;
    d->boundingBox = QRect();
    d->fontsUsed = "";

    setActive(true);

    newPage();

    return true;
}

bool QPSPrintEngine::end()
{
    Q_D(QPSPrintEngine);

    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(true);
    d->flushPage(true);
    QByteArray trailer;
    QPdf::ByteStream s(&trailer);
    s << "%%Trailer\n";
    s << "%%Pages: " << d->pageCount - 1 << "\n" <<
        wrapDSC("%%DocumentFonts: " + d->fontsUsed);
    s << "%%EOF\n";
    d->outDevice->write(trailer);
    ignoreSigPipe(false);

    if (d->outDevice)
        d->outDevice->close();
    if (d->fd >= 0)
        ::close(d->fd);
    d->fd = -1;
    delete d->outDevice;
    d->outDevice = 0;

    qDeleteAll(d->fonts);
    d->fonts.clear();
    d->firstPage = true;

    setActive(false);
    d->pdev = 0;

    return true;
}


void QPSPrintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QPSPrintEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform)
        d->stroker.matrix = state.matrix();

    if (flags & DirtyPen) {
        d->pen = state.pen();
        d->hasPen = (d->pen != Qt::NoPen);
        d->stroker.setPen(d->pen);
    }
    if (flags & DirtyBrush) {
        d->brush = state.brush();
        d->hasBrush = (d->brush != Qt::NoBrush);
    }
    if (flags & DirtyBrushOrigin) {
        d->brushOrigin = state.brushOrigin();
        flags |= DirtyBrush;
    }

    if (flags & DirtyBackground)
        d->backgroundBrush = state.backgroundBrush();
    if (flags & DirtyBackgroundMode)
        d->backgroundMode = state.backgroundMode();

    bool ce = d->clipEnabled;
    if (flags & DirtyClipEnabled)
        d->clipEnabled = state.isClipEnabled();
    if (flags & DirtyClipPath)
        updateClipPath(state.clipPath(), state.clipOperation());
    if (flags & DirtyClipRegion) {
        QPainterPath path;
        QVector<QRect> rects = state.clipRegion().rects();
        for (int i = 0; i < rects.size(); ++i)
            path.addRect(rects.at(i));
        updateClipPath(path, state.clipOperation());
        flags |= DirtyClipPath;
    }

    if (ce != d->clipEnabled)
        flags |= DirtyClipPath;
    else if (!d->clipEnabled)
        flags &= ~DirtyClipPath;

    if (flags & DirtyClipPath) {
        *d->currentPage << "Q q\n";
        flags |= DirtyPen|DirtyBrush;
    }

    if (flags & DirtyClipPath) {
        d->allClipped = false;
        if (d->clipEnabled && !d->clips.isEmpty()) {
            for (int i = 0; i < d->clips.size(); ++i) {
                if (d->clips.at(i).isEmpty()) {
                    d->allClipped = true;
                    break;
                }
            }
            if (!d->allClipped) {
                for (int i = 0; i < d->clips.size(); ++i) {
                    *d->currentPage << QPdf::generatePath(d->clips.at(i), QMatrix(), QPdf::ClipPath);
                }
            }
        }
    }

    if (flags & DirtyBrush)
        setBrush();
}

void QPSPrintEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_D(QPSPrintEngine);
    QPainterPath path = d->stroker.matrix.map(p);
    //qDebug() << "updateClipPath: " << matrix << p.boundingRect() << path.boundingRect();

    if (op == Qt::NoClip) {
        d->clipEnabled = false;
    } else if (op == Qt::ReplaceClip) {
        d->clips.clear();
        d->clips.append(path);
    } else if (op == Qt::IntersectClip) {
        d->clips.append(path);
    } else { // UniteClip
        // ask the painter for the current clipping path. that's the easiest solution
        path = painter()->clipPath();
        path = d->stroker.matrix.map(path);
        d->clips.clear();
        d->clips.append(path);
    }
}

void QPSPrintEngine::setPen()
{
    Q_D(QPSPrintEngine);
    QBrush b = d->pen.brush();
    Q_ASSERT(b.style() == Qt::SolidPattern && b.isOpaque());

    QColor rgba = b.color();
    *d->currentPage << rgba.redF()
                   << rgba.greenF()
                   << rgba.blueF()
                   << "SCN\n";

    *d->currentPage << d->pen.widthF() << "w ";

    int pdfCapStyle = 0;
    switch(d->pen.capStyle()) {
    case Qt::FlatCap:
        pdfCapStyle = 0;
        break;
    case Qt::SquareCap:
        pdfCapStyle = 2;
        break;
    case Qt::RoundCap:
        pdfCapStyle = 1;
        break;
    default:
        break;
    }
    *d->currentPage << pdfCapStyle << "J ";

    int pdfJoinStyle = 0;
    switch(d->pen.joinStyle()) {
    case Qt::MiterJoin:
        pdfJoinStyle = 0;
        break;
    case Qt::BevelJoin:
        pdfJoinStyle = 2;
        break;
    case Qt::RoundJoin:
        pdfJoinStyle = 1;
        break;
    default:
        break;
    }
    *d->currentPage << pdfJoinStyle << "j ";

    *d->currentPage << QPdf::generateDashes(d->pen) << " 0 d\n";
}

void QPSPrintEngine::setBrush()
{
    Q_D(QPSPrintEngine);
#if 0
    bool specifyColor;
    int gStateObject = 0;
    int patternObject = d->addBrushPattern(brush, d->stroker.matrix, brushOrigin, &specifyColor, &gStateObject);

    *d->currentPage << (patternObject ? "/PCSp cs " : "/CSp cs ");
    if (specifyColor) {
        QColor rgba = brush.color();
        *d->currentPage << rgba.redF()
                        << rgba.greenF()
                        << rgba.blueF();
    }
    if (patternObject)
        *d->currentPage << "/Pat" << patternObject;
    *d->currentPage << "scn\n";
#endif
    QColor rgba = d->brush.color();
    *d->currentPage << rgba.redF()
                    << rgba.greenF()
                    << rgba.blueF()
                    << "scn\n";
    *d->currentPage << "/BSt " << d->brush.style() << "def\n";
}



void QPSPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (!lines || !lineCount)
        return;

    QPainterPath p;
    for (int i=0; i!=lineCount;++i) {
        p.moveTo(lines[i].p1());
        p.lineTo(lines[i].p2());
    }
    drawPath(p);
}

void QPSPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    if (!points || !pointCount)
        return;
    Q_D(QPSPrintEngine);

    QPainterPath p;

    bool hb = d->hasBrush;

    switch(mode) {
    case OddEvenMode:
        p.setFillRule(Qt::OddEvenFill);
        break;
    case ConvexMode:
    case WindingMode:
        p.setFillRule(Qt::WindingFill);
        break;
    case PolylineMode:
        d->hasBrush = false;
        break;
    default:
        break;
    }

    p.moveTo(points[0]);
    for (int i = 1; i < pointCount; ++i)
        p.lineTo(points[i]);

    if (mode != PolylineMode)
        p.closeSubpath();
    drawPath(p);

    d->hasBrush = hb;
}

void QPSPrintEngine::drawImageInternal(const QRectF &r, QImage image, bool bitmap)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    if (bitmap && image.depth() != 1)
        bitmap = false;
    QImage mask;
    if (!bitmap) {
        if (image.format() == QImage::Format_Mono || image.format() == QImage::Format_MonoLSB)
            image = image.convertToFormat(QImage::Format_Indexed8);
        if (image.hasAlphaChannel()) {
            // get better alpha dithering
            int xscale = image.width();
            xscale *= xscale <= 800 ? 4 : (xscale <= 1600 ? 2 : 1);
            int yscale = image.height();
            yscale *= yscale <= 800 ? 4 : (yscale <= 1600 ? 2 : 1);
            image = image.scaled(xscale, yscale);
            mask = image.createAlphaMask(Qt::OrderedAlphaDither);
        }
    }
    *d->currentPage << "q\n" << QPdf::generateMatrix(d->stroker.matrix);
    QBrush b = d->brush;
    if (image.depth() == 1) {
        if (d->backgroundMode == Qt::OpaqueMode) {
            // draw background
            d->brush = d->backgroundBrush;
            setBrush();
            *d->currentPage << r.x() << r.y() << r.width() << r.height() << "re f\n";
        }
        // set current pen as brush
        d->brush = d->pen.brush();
        setBrush();
    }
    d->drawImage(r.x(), r.y(), r.width(), r.height(), image, mask);
    *d->currentPage << "Q\n";
    d->brush = b;
}


void QPSPrintEngine::drawImage(const QRectF &r, const QImage &img, const QRectF &sr,
                               Qt::ImageConversionFlags)
{
    QImage image = img.copy(sr.toRect());
    drawImageInternal(r, image, false);
}

void QPSPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    QImage img = pm.copy(sr.toRect()).toImage();
    drawImageInternal(r, img, true);
}

void QPSPrintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QPSPrintEngine);
    if (!d->hasPen || (d->clipEnabled && d->allClipped))
        return;

    *d->currentPage << "q " << QPdf::generateMatrix(d->stroker.matrix);

    bool hp = d->hasPen;
    d->hasPen = false;
    QBrush b = d->brush;
    d->brush = d->pen.brush();
    setBrush();

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    if (ti.fontEngine->type() == QFontEngine::Multi) {
        QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
        QGlyphLayout *glyphs = ti.glyphs;
        int which = glyphs[0].glyph >> 24;

        qreal x = p.x();
        qreal y = p.y();

        int start = 0;
        int end, i;
        for (end = 0; end < ti.num_glyphs; ++end) {
            const int e = glyphs[end].glyph >> 24;
            if (e == which)
                continue;

            // set the high byte to zero
            for (i = start; i < end; ++i)
                glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

            // draw the text
            QTextItemInt ti2 = ti;
            ti2.glyphs = ti.glyphs + start;
            ti2.num_glyphs = end - start;
            ti2.fontEngine = multi->engine(which);
            ti2.f = ti.f;
            d->drawTextItem(this, QPointF(x, y), ti2);

            QFixed xadd;
            // reset the high byte for all glyphs and advance to the next sub-string
            const int hi = which << 24;
            for (i = start; i < end; ++i) {
                glyphs[i].glyph = hi | glyphs[i].glyph;
                xadd += glyphs[i].advance.x;
            }
            x += xadd.toReal();

            // change engine
            start = end;
            which = e;
        }

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        d->drawTextItem(this, QPointF(x,y), ti2);

        // reset the high byte for all glyphs
        const int hi = which << 24;
        for (i = start; i < end; ++i)
            glyphs[i].glyph = hi | glyphs[i].glyph;
    } else {
        d->drawTextItem(this, p, ti);
    }
    d->hasPen = hp;
    d->brush = b;
    *d->currentPage << "Q\n";
}

void QPSPrintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    // ### Optimise implementation!
    qreal yPos = r.y();
    qreal yOff = p.y();
    while(yPos < r.y() + r.height()) {
        qreal drawH = pixmap.height() - yOff;    // Cropping first row
        if (yPos + drawH > r.y() + r.height())        // Cropping last row
            drawH = r.y() + r.height() - yPos;
        qreal xPos = r.x();
        qreal xOff = p.x();
        while(xPos < r.x() + r.width()) {
            qreal drawW = pixmap.width() - xOff; // Cropping first column
            if (xPos + drawW > r.x() + r.width())    // Cropping last column
                drawW = r.x() + r.width() - xPos;
            // ########
            painter()->drawPixmap(QPointF(xPos, yPos).toPoint(), pixmap,
                                   QRectF(xOff, yOff, drawW, drawH).toRect());
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }

}

void QPSPrintEngine::drawPath(const QPainterPath &p)
{
    Q_D(QPSPrintEngine);
    if (d->clipEnabled && d->allClipped)
        return;
    QBrush penBrush = d->pen.brush();
    if (d->hasPen && penBrush == Qt::SolidPattern && penBrush.isOpaque()) {
        // draw strokes natively in this case for better output
        *d->currentPage << "q\n";
        setPen();
        *d->currentPage << QPdf::generateMatrix(d->stroker.matrix);
        *d->currentPage << "NP " << QPdf::generatePath(p, QMatrix(), d->hasBrush ? QPdf::FillAndStrokePath : QPdf::StrokePath);
        *d->currentPage << "Q\n";
    } else {
        if (d->hasBrush) {
            *d->currentPage << "NP " << QPdf::generatePath(p, d->stroker.matrix, QPdf::FillPath);
        }
        if (d->hasPen) {
            *d->currentPage << "q\n";
            QBrush b = d->brush;
            d->brush = d->pen.brush();
            setBrush();
            *d->currentPage << "NP ";
            d->stroker.strokePath(p);
            *d->currentPage << "Q\n";
            d->brush = b;
        }
    }
}


bool QPSPrintEngine::newPage()
{
    Q_D(QPSPrintEngine);
    // we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
    // if lp/lpr dies
    ignoreSigPipe(true);
    if (!d->firstPage)
        d->flushPage();
    d->firstPage = false;
    ignoreSigPipe(false);

    d->pageBuffer = QByteArray();
    delete d->currentPage;
    d->currentPage = new QPdf::ByteStream(&d->pageBuffer);
    d->stroker.stream = d->currentPage;

    return true;
}

bool QPSPrintEngine::abort()
{
    // ### abort!?!
    return false;
}

QRect QPSPrintEnginePrivate::paperRect() const
{
    PaperSize s = paperSizes[pageSize];
    int w = qRound(s.width*resolution/72.);
    int h = qRound(s.height*resolution/72.);
    if (orientation == QPrinter::Portrait)
        return QRect(0, 0, w, h);
    else
        return QRect(0, 0, h, w);
}

QRect QPSPrintEnginePrivate::pageRect() const
{
    QRect r = paperRect();
    if (fullPage)
        return r;
    // would be nice to get better margins than this.
    return QRect(resolution/3, resolution/3, r.width()-2*resolution/3, r.height()-2*resolution/3);
}

int  QPSPrintEngine::metric(QPaintDevice::PaintDeviceMetric metricType) const
{
    Q_D(const QPSPrintEngine);
    int val;
    QRect r = d->fullPage ? d->paperRect() : d->pageRect();
    switch (metricType) {
    case QPaintDevice::PdmWidth:
        val = r.width();
        break;
    case QPaintDevice::PdmHeight:
        val = r.height();
        break;
    case QPaintDevice::PdmDpiX:
        val = d->resolution;
        break;
    case QPaintDevice::PdmDpiY:
        val = d->resolution;
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY:
        val = 1200;
        break;
    case QPaintDevice::PdmWidthMM:
        val = qRound(r.width()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmHeightMM:
        val = qRound(r.height()*25.4/d->resolution);
        break;
    case QPaintDevice::PdmNumColors:
        val = INT_MAX;
        break;
    case QPaintDevice::PdmDepth:
        val = 32;
        break;
    default:
        qWarning("QPrinter::metric: Invalid metric command");
        return 0;
    }
    return val;
}

QPrinter::PrinterState QPSPrintEngine::printerState() const
{
    Q_D(const QPSPrintEngine);
    return d->printerState;
}

void QPSPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QPSPrintEngine);
    switch (key) {
    case PPK_CollateCopies:
        d->collate = value.toBool();
        break;
    case PPK_ColorMode:
        d->colorMode = QPrinter::ColorMode(value.toInt());
        break;
    case PPK_Creator:
        d->creator = value.toString();
        break;
    case PPK_DocumentName:
        d->title = value.toString();
        break;
    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;
    case PPK_NumberOfCopies:
        d->copies = value.toInt();
        break;
    case PPK_Orientation:
        d->orientation = QPrinter::Orientation(value.toInt());
        break;
    case PPK_OutputFileName:
        d->outputFileName = value.toString();
        break;
    case PPK_PageOrder:
        d->pageOrder = QPrinter::PageOrder(value.toInt());
        break;
    case PPK_PageSize:
        d->pageSize = QPrinter::PageSize(value.toInt());
        break;
    case PPK_PaperSource:
        d->paperSource = QPrinter::PaperSource(value.toInt());
        break;
    case PPK_PrinterName:
        d->printerName = value.toString();
        break;
    case PPK_PrinterProgram:
        d->printProgram = value.toString();
        break;
    case PPK_Resolution:
        d->resolution = value.toInt();
        break;
    case PPK_SelectionOption:
        d->selectionOption = value.toString();
        break;
    case PPK_FontEmbedding:
        d->embedFonts = value.toBool();
        break;
    default:
        break;
    }
}

QVariant QPSPrintEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QPSPrintEngine);
    QVariant ret;
    switch (key) {
    case PPK_CollateCopies:
        ret = d->collate;
        break;
    case PPK_ColorMode:
        ret = d->colorMode;
        break;
    case PPK_Creator:
        ret = d->creator;
        break;
    case PPK_DocumentName:
        ret = d->title;
        break;
    case PPK_FullPage:
        ret = d->fullPage;
        break;
    case PPK_NumberOfCopies:
        ret = d->copies;
        break;
    case PPK_Orientation:
        ret = d->orientation;
        break;
    case PPK_OutputFileName:
        ret = d->outputFileName;
        break;
    case PPK_PageOrder:
        ret = d->pageOrder;
        break;
    case PPK_PageSize:
        ret = d->pageSize;
        break;
    case PPK_PaperSource:
        ret = d->paperSource;
        break;
    case PPK_PrinterName:
        ret = d->printerName;
        break;
    case PPK_PrinterProgram:
        ret = d->printProgram;
        break;
    case PPK_Resolution:
        ret = d->resolution;
        break;
    case PPK_SupportedResolutions:
        ret = QList<QVariant>() << 72;
        break;
    case PPK_PaperRect:
        ret = d->paperRect();
        break;
    case PPK_PageRect:
        ret = d->pageRect();
        break;
    case PPK_SelectionOption:
        ret = d->selectionOption;
        break;
    case PPK_FontEmbedding:
        ret = d->embedFonts;
        break;
    default:
        break;
    }
    return ret;
}

#endif // QT_NO_PRINTER
