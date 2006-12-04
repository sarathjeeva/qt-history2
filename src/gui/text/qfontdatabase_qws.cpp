/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdir.h"
#include "qscreen_qws.h" //so we can check for rotation
#include "qlibraryinfo.h"
#include "qabstractfileengine.h"
#if !defined(QT_NO_FREETYPE)
#include "qfontengine_ft_p.h"
#endif
#include "qfontengine_qpf_p.h"

// for mmap
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

static void addFont(QFontDatabasePrivate *db, const char *family, int weight, bool italic, int pixelSize, const char *file, bool antialiased)
{
    QString familyname = QString::fromUtf8(family);
    QString foundryname = QLatin1String("");
    QtFontStyle::Key styleKey;
    styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
    styleKey.weight = weight;
    styleKey.stretch = 100;
    QtFontFamily *f = db->family(familyname, true);
    //### get lang info from freetype
    for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws)
        f->writingSystems[ws] = QtFontFamily::Supported;
    QtFontFoundry *foundry = f->foundry(foundryname, true);
    QtFontStyle *style = foundry->style(styleKey,  true);
    style->smoothScalable = (pixelSize == 0);
    style->antialiased = antialiased;
    QtFontSize *size = style->pixelSize(pixelSize?pixelSize:SMOOTH_SCALABLE, true);
    size->fileName = file;
}


/*!
    \internal
*/
static void initializeDb()
{
    QFontDatabasePrivate *db = privateDb();
    if (!db || db->count)
        return;

    // Load in font definition file
    QString fn;
#ifndef QT_NO_SETTINGS
    fn = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
#else
    fn += QLatin1String("/lib");
#endif
    fn += QLatin1String("/fonts/fontdir");
    FILE* fontdef=fopen(fn.toLocal8Bit().constData(),"r");
    if(!fontdef) {
        qFatal("QFontDatabase: Cannot find font definition file %s - is Qt installed correctly?",
               fn.toLocal8Bit().constData());
    }
    char buf[200]="";
    char name[200]="";
    char render[200]="";
    char file[200]="";
    char isitalic[10]="";
    char flags[10]="";
    do {
        fgets(buf,200,fontdef);
        if (buf[0] != '#') {
            int weight=50;
            int size=0;
            sscanf(buf,"%s %s %s %s %d %d %s",name,file,render,isitalic,&weight,&size,flags);
            QString filename;
            if (file[0] != '/') {
#ifndef QT_NO_SETTINGS
                filename = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
                filename += QLatin1String("/fonts/");
#else
                filename += QLatin1String("/lib/fonts/");
#endif
            }
            filename += file;
            bool italic = isitalic[0] == 'y';
            bool smooth = QByteArray(flags).contains('s');
            if (file[0] && QFile::exists(filename))
                addFont(db, name, weight, italic, size/10, file, smooth);
        }
    } while (!feof(fontdef));
    fclose(fontdef);


    QString fontpath;
#ifndef QT_NO_SETTINGS
    fontpath = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    fontpath += QLatin1String("/fonts");
#else
    fontpath += QLatin1String("/lib/fonts");
#endif
    QDir dir(fontpath,"*.qpf");
    for (int i=0; i<int(dir.count()); i++) {
        int u0 = dir[i].indexOf('_');
        int u1 = dir[i].indexOf('_',u0+1);
        int u2 = dir[i].indexOf('_',u1+1);
        int u3 = dir[i].indexOf('.',u1+1);
        if (u2 < 0) u2 = u3;

#if 0
        /*
          Skip fonts for other screen orientations. Such fonts may be
          installed even on a production device. Different orientations
          could have different fonts.
        */
        //### This code could be prettier
        QString rotation;
        if (u2 != u3)
            rotation = dir[i].mid(u2+1,u3-u2-1);

        QString screenr;
        if (qt_screen->isTransformed()) {
            screenr = "t";
            QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
            QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
            screenr += QString::number(a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()));
        }

        if (rotation != screenr)
            continue;
#endif

        QString familyname = dir[i].left(u0);
        int pixelSize = dir[i].mid(u0+1,u1-u0-1).toInt()/10;
        bool italic = dir[i].mid(u2-1,1) == "i";
        int weight = dir[i].mid(u1+1,u2-u1-1-(italic?1:0)).toInt();
        QtFontFamily *f = db->family(familyname, true);
        for (int ws = 1; ws < QFontDatabase::WritingSystemsCount; ++ws) {
            if (!requiresOpenType(ws))
                f->writingSystems[ws] = QtFontFamily::Supported;
        }
        QtFontFoundry *foundry = f->foundry("qt", true);
        QtFontStyle::Key styleKey;
        styleKey.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
        styleKey.weight = weight;
        styleKey.stretch = 100;
        QtFontStyle *style = foundry->style(styleKey,  true);
        style->smoothScalable = false;
        style->pixelSize(pixelSize, true);
    }
    dir = QDir(fontpath, "*.qpf2");
    for (int i = 0; i < int(dir.count()); ++i) {
        const QString file = dir.absoluteFilePath(dir[i]);
        int f = ::open(QFile::encodeName(file), O_RDONLY);
        if (f < 0)
            continue;
        struct stat st;
        if (!fstat(f, &st)) {
            const uchar *data = (const uchar *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, f, 0);
            if (data && data != (const uchar *)MAP_FAILED) {
                if (QFontEngineQPF::verifyHeader(data, st.st_size)) {
                    QString fontName = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_FontName).toString();
                    int pixelSize = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_PixelSize).toInt();
                    QVariant weight = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_Weight);
                    QVariant style = QFontEngineQPF::extractHeaderField(data, QFontEngineQPF::Tag_Style);

                    if (!fontName.isEmpty() && pixelSize) {
                        QtFontFamily *f = db->family(fontName, true);
                        QtFontFoundry *foundry = f->foundry("prerendered", true);
                        QtFontStyle::Key styleKey;
                        styleKey.style = static_cast<QFont::Style>(style.toInt());
                        if (weight.type() != QVariant::Int)
                            styleKey.weight = 50;
                        else
                            styleKey.weight = weight.toInt();
                        styleKey.stretch = 100; // ####
                        QtFontStyle *style = foundry->style(styleKey, true);
                        style->smoothScalable = false;
                        QtFontSize *p = style->pixelSize(pixelSize, true);
                        p->fileName = QFile::encodeName(file);
                    }
                } else {
                    qDebug() << "header verification of QPF2 font" << file << "failed. maybe it is corrupt?";
                }
                munmap((void *)data, st.st_size);
            }
        }
        ::close(f);
    }

#ifdef QFONTDATABASE_DEBUG
    // print the database
    for (int f = 0; f < db->count; f++) {
        QtFontFamily *family = db->families[f];
        FD_DEBUG("'%s' %s", qPrintable(family->name), (family->fixedPitch ? "fixed" : ""));
#if 0
        for (int i = 0; i < QFont::LastPrivateScript; ++i) {
            FD_DEBUG("\t%s: %s", qPrintable(QFontDatabase::scriptName((QFont::Script) i)),
                     ((family->scripts[i] & QtFontFamily::Supported) ? "Supported" :
                      (family->scripts[i] & QtFontFamily::UnSupported) == QtFontFamily::UnSupported ?
                      "UnSupported" : "Unknown"));
        }
#endif

        for (int fd = 0; fd < family->count; fd++) {
            QtFontFoundry *foundry = family->foundries[fd];
            FD_DEBUG("\t\t'%s'", qPrintable(foundry->name));
            for (int s = 0; s < foundry->count; s++) {
                QtFontStyle *style = foundry->styles[s];
                FD_DEBUG("\t\t\tstyle: style=%d weight=%d\n"
                         "\t\t\tstretch=%d",
                         style->key.style, style->key.weight,
                         style->key.stretch);
                if (style->smoothScalable)
                    FD_DEBUG("\t\t\t\tsmooth scalable");
                else if (style->bitmapScalable)
                    FD_DEBUG("\t\t\t\tbitmap scalable");
                if (style->pixelSizes) {
                    FD_DEBUG("\t\t\t\t%d pixel sizes",  style->count);
                    for (int z = 0; z < style->count; ++z) {
                        QtFontSize *size = style->pixelSizes + z;
                        FD_DEBUG("\t\t\t\t  size %5d",
                                  size->pixelSize);
                    }
                }
            }
        }
    }
#endif // QFONTDATABASE_DEBUG


}

static inline void load(const QString & = QString(), int = -1)
{
}

#ifndef QT_NO_FREETYPE

#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
#define X_SIZE(face,i) ((face)->available_sizes[i].x_ppem)
#define Y_SIZE(face,i) ((face)->available_sizes[i].y_ppem)
#else
#define X_SIZE(face,i) ((face)->available_sizes[i].width << 6)
#define Y_SIZE(face,i) ((face)->available_sizes[i].height << 6)
#endif

#endif // QT_NO_FREETYPE

static
QFontEngine *loadEngine(int script, const QFontPrivate *fp,
                         const QFontDef &request,
                         QtFontFamily *family, QtFontFoundry *foundry,
                         QtFontStyle *style, QtFontSize *size)
{
    Q_UNUSED(script);
    Q_UNUSED(fp);
#ifdef QT_NO_FREETYPE
    Q_UNUSED(foundry);
#endif
#ifdef QT_NO_QWS_QPF
    Q_UNUSED(family);
#endif
    Q_ASSERT(size);

    int pixelSize = size->pixelSize;
    if (!pixelSize || style->smoothScalable && pixelSize == SMOOTH_SCALABLE)
        pixelSize = request.pixelSize;

#ifndef QT_NO_QPF2
    if (foundry->name == QLatin1String("prerendered")) {
        int f = ::open(QFile::encodeName(size->fileName), O_RDONLY);
        if (f >= 0) {
            struct stat st;
            if (!fstat(f, &st)) {
                const uchar *data = (const uchar *)mmap(0, st.st_size, PROT_READ, MAP_SHARED, f, 0);
                if (data && data != (const uchar *)MAP_FAILED
                    && QFontEngineQPF::verifyHeader(data, st.st_size)) {
                    ::close(f);
                    QFontEngineQPF *fe = new QFontEngineQPF(request, data, st.st_size);
                    if (fe->isValid())
                        return fe;
                    delete fe; // will implicitly munmap
                }
            }
            ::close(f);
        }
    } else
#endif
#ifndef QT_NO_FREETYPE
    if ( foundry->name != QLatin1String("qt") ) { ///#### is this the best way????
        QString file;
#ifndef QT_NO_SETTINGS
        file = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
        file += QLatin1String("/fonts/");
#else
        file += QLatin1String("/lib/fonts/");
#endif
        file += size->fileName;

        if (QFile::exists(file)) {
            QFontEngine::FaceId faceId;
            faceId.filename = file.toLocal8Bit();
            faceId.index = 0;

            QFontDef def = request;
            def.pixelSize = pixelSize;

            QFontEngineFT *fe = new QFontEngineFT(def);
            fe->init(faceId, style->antialiased);
            return fe;
        }
    } else
#endif // QT_NO_FREETYPE
    {
#ifndef QT_NO_QWS_QPF
        QString fn;
#ifndef QT_NO_SETTINGS
	fn = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
        fn += QLatin1String("/fonts/");
#else
        fn += QLatin1String("/lib/fonts/");
#endif
        fn += family->name.toLower()
              + "_" + QString::number(pixelSize*10)
              + "_" + QString::number(style->key.weight)
              + (style->key.style == QFont::StyleItalic ? "i.qpf" : ".qpf");
        //###rotation ###

        QFontEngine *fe = new QFontEngineQPF1(request, fn);
        return fe;
#endif // QT_NO_QWS_QPF
    }
    return new QFontEngineBox(pixelSize);
}

static void registerFont(QFontDatabasePrivate::ApplicationFont *fnt)
{
    // #######
    fnt->families.clear();
}

bool QFontDatabase::removeApplicationFont(int handle)
{
    Q_UNUSED(handle);
    // #######
    return false;
}

bool QFontDatabase::removeAllApplicationFonts()
{
    // #######
    return false;
}

