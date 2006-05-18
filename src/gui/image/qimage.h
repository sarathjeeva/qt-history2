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

#ifndef QIMAGE_H
#define QIMAGE_H

#include <QtGui/qpaintdevice.h>
#include <QtGui/qrgb.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qrect.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QIODevice;
class QStringList;
class QMatrix;
class QVariant;
template <class T> class QList;
template <class T> class QVector;

struct QImageData;
class QImageDataMisc; // internal
#ifndef QT_NO_IMAGE_TEXT
class Q_GUI_EXPORT QImageTextKeyLang {
public:
    QImageTextKeyLang(const char* k, const char* l) : key(k), lang(l) { }
    QImageTextKeyLang() { }

    QByteArray key;
    QByteArray lang;

    bool operator< (const QImageTextKeyLang& other) const
        { return key < other.key || key==other.key && lang < other.lang; }
    bool operator== (const QImageTextKeyLang& other) const
        { return key==other.key && lang==other.lang; }
    inline bool operator!= (const QImageTextKeyLang &other) const
        { return !operator==(other); }
};
#endif //QT_NO_IMAGE_TEXT


class Q_GUI_EXPORT QImage : public QPaintDevice
{
public:
    enum InvertMode { InvertRgb, InvertRgba };
    enum Format {
        Format_Invalid,
        Format_Mono,
        Format_MonoLSB,
        Format_Indexed8,
        Format_RGB32,
        Format_ARGB32,
        Format_ARGB32_Premultiplied,
        Format_RGB16,
#if 0
        // reserved for future use
        Format_RGB15,
        Format_Grayscale16,
        Format_Grayscale8,
        Format_Grayscale4,
        Format_Grayscale4LSB,
        Format_Grayscale2,
        Format_Grayscale2LSB
#endif
#ifndef qdoc
        NImageFormats
#endif
    };

    QImage();
    QImage(const QSize &size, Format format);
    QImage(int width, int height, Format format);
    QImage(uchar *data, int width, int height, Format format);

#ifndef QT_NO_IMAGEFORMAT_XPM
    explicit QImage(const char * const xpm[]);
#endif
    explicit QImage(const QString &fileName, const char *format = 0);
#ifndef QT_NO_CAST_FROM_ASCII
    explicit QImage(const char *fileName, const char *format = 0);
#endif

    QImage(const QImage &);
    ~QImage();

    QImage &operator=(const QImage &);
    bool isNull() const;

    int devType() const;

    bool operator==(const QImage &) const;
    bool operator!=(const QImage &) const;
    operator QVariant() const;
    void detach();
    bool isDetached() const;

    QImage copy(const QRect &rect = QRect()) const;
    inline QImage copy(int x, int y, int w, int h) const
        { return copy(QRect(x, y, w, h)); }

    Format format() const;

    QImage convertToFormat(Format f, Qt::ImageConversionFlags flags = Qt::AutoColor) const Q_REQUIRED_RESULT;
    QImage convertToFormat(Format f, const QVector<QRgb> &colorTable, Qt::ImageConversionFlags flags = Qt::AutoColor) const Q_REQUIRED_RESULT;

    int width() const;
    int height() const;
    QSize size() const;
    QRect rect() const;

    int depth() const;
    int numColors() const;

    QRgb color(int i) const;
    void setColor(int i, QRgb c);
    void setNumColors(int);

    bool allGray() const;
    bool isGrayscale() const;

    uchar *bits();
    const uchar *bits() const;
    int numBytes() const;

    uchar *scanLine(int);
    const uchar *scanLine(int) const;
    int bytesPerLine() const;

    bool valid(int x, int y) const;
    int pixelIndex(int x, int y) const;
    QRgb pixel(int x, int y) const;
    void setPixel(int x, int y, uint index_or_rgb);

    QVector<QRgb> colorTable() const;
    void setColorTable(const QVector<QRgb> colors);

    void fill(uint pixel);

    bool hasAlphaChannel() const;
    void setAlphaChannel(const QImage &alphaChannel);
    QImage alphaChannel() const;
    QImage createAlphaMask(Qt::ImageConversionFlags flags = Qt::AutoColor) const;
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
    QImage createHeuristicMask(bool clipTight = true) const;
#endif

    inline QImage scaled(int w, int h, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                        Qt::TransformationMode mode = Qt::FastTransformation) const
        { return scaled(QSize(w, h), aspectMode, mode); }
    QImage scaled(const QSize &s, Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio,
                 Qt::TransformationMode mode = Qt::FastTransformation) const;
    QImage scaledToWidth(int w, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QImage scaledToHeight(int h, Qt::TransformationMode mode = Qt::FastTransformation) const;
    QImage transformed(const QMatrix &matrix, Qt::TransformationMode mode = Qt::FastTransformation) const;
    static QMatrix trueMatrix(const QMatrix &, int w, int h);
    QImage mirrored(bool horizontally = false, bool vertically = true) const;
    QImage rgbSwapped() const;
    void invertPixels(InvertMode = InvertRgb);


    bool load(QIODevice *device, const char* format);
    bool load(const QString &fileName, const char* format=0);
    bool loadFromData(const uchar *buf, int len, const char *format = 0);
    inline bool loadFromData(const QByteArray &data, const char* aformat=0)
        { return loadFromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), aformat); }

    bool save(const QString &fileName, const char* format, int quality=-1) const;
    bool save(QIODevice *device, const char* format, int quality=-1) const;

    static QImage fromData(const uchar *data, int size, const char *format = 0);
    inline static QImage fromData(const QByteArray &data, const char *format = 0)
        { return fromData(reinterpret_cast<const uchar *>(data.constData()), data.size(), format); }

    int serialNumber() const;

    QPaintEngine *paintEngine() const;

    // Auxiliary data
    int dotsPerMeterX() const;
    int dotsPerMeterY() const;
    void setDotsPerMeterX(int);
    void setDotsPerMeterY(int);
    QPoint offset() const;
    void setOffset(const QPoint&);
#ifndef QT_NO_IMAGE_TEXT
    QStringList textKeys() const;
    QString text(const QString &key = QString()) const;
    void setText(const QString &key, const QString &value);

    // The following functions are obsolete as of 4.1
    QString text(const char* key, const char* lang=0) const;
    QList<QImageTextKeyLang> textList() const;
    QStringList textLanguages() const;
    QString text(const QImageTextKeyLang&) const;
    void setText(const char* key, const char* lang, const QString&);
#endif

#ifdef QT3_SUPPORT
    enum Endian { BigEndian, LittleEndian, IgnoreEndian };
    QT3_SUPPORT_CONSTRUCTOR QImage(int width, int height, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    QT3_SUPPORT_CONSTRUCTOR QImage(const QSize&, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    QT3_SUPPORT_CONSTRUCTOR QImage(uchar *data, int w, int h, int depth, const QRgb *colortable, int numColors, Endian bitOrder);
#ifdef Q_WS_QWS
    QT3_SUPPORT_CONSTRUCTOR QImage(uchar *data, int w, int h, int depth, int pbl, const QRgb *colortable, int numColors, Endian bitOrder);
#endif
    inline QT3_SUPPORT Endian bitOrder() const {
        Format f = format();
        return f == Format_Mono ? BigEndian : (f == Format_MonoLSB ? LittleEndian : IgnoreEndian);
    }
    QT3_SUPPORT QImage convertDepth(int, Qt::ImageConversionFlags flags = Qt::AutoColor) const;
    QT3_SUPPORT QImage convertDepthWithPalette(int, QRgb* p, int pc, Qt::ImageConversionFlags flags = Qt::AutoColor) const;
    QT3_SUPPORT QImage convertBitOrder(Endian) const;
    QT3_SUPPORT bool hasAlphaBuffer() const;
    QT3_SUPPORT void setAlphaBuffer(bool);
    QT3_SUPPORT uchar **jumpTable();
    QT3_SUPPORT const uchar * const *jumpTable() const;
    inline QT3_SUPPORT void reset() { *this = QImage(); }
    static inline QT3_SUPPORT Endian systemByteOrder()
        { return QSysInfo::ByteOrder == QSysInfo::BigEndian ? BigEndian : LittleEndian; }
    inline QT3_SUPPORT QImage swapRGB() const { return rgbSwapped(); }
    inline QT3_SUPPORT QImage mirror(bool horizontally = false, bool vertically = true) const
        { return mirrored(horizontally, vertically); }
    QT3_SUPPORT bool create(const QSize&, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    QT3_SUPPORT bool create(int width, int height, int depth, int numColors=0, Endian bitOrder=IgnoreEndian);
    inline QT3_SUPPORT QImage xForm(const QMatrix &matrix) const { return transformed(matrix); }
    inline QT3_SUPPORT QImage smoothScale(int w, int h, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio) const
        { return scaled(QSize(w, h), mode, Qt::SmoothTransformation); }
    inline QImage QT3_SUPPORT smoothScale(const QSize &s, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio) const
        { return scaled(s, mode, Qt::SmoothTransformation); }
    inline QT3_SUPPORT QImage scaleWidth(int w) const { return scaledToWidth(w); }
    inline QT3_SUPPORT QImage scaleHeight(int h) const { return scaledToHeight(h); }
    inline QT3_SUPPORT void invertPixels(bool invertAlpha) { invertAlpha ? invertPixels(InvertRgba) : invertPixels(InvertRgb); }
    inline QT3_SUPPORT QImage copy(int x, int y, int w, int h, Qt::ImageConversionFlags) const
        { return copy(QRect(x, y, w, h)); }
    inline QT3_SUPPORT QImage copy(const QRect &rect, Qt::ImageConversionFlags) const
        { return copy(rect); }
    static QT3_SUPPORT Endian systemBitOrder();
    inline QT3_SUPPORT_CONSTRUCTOR QImage(const QByteArray &data)
        { d = 0; *this = QImage::fromData(data); }
#endif

protected:
    virtual int metric(PaintDeviceMetric metric) const;

private:
#if defined(Q_WS_QWS) && !defined(QT3_SUPPORT)
public:
    enum Endian { BigEndian, LittleEndian, IgnoreEndian };
private:
    QImage(uchar *data, int w, int h, int depth, int pbl, const QRgb *colortable, int numColors, Endian bitOrder);
#endif

    QImageData *d;

    friend class QPixmap;
    friend Q_GUI_EXPORT qint64 qt_image_id(const QImage &image);
    friend const QVector<QRgb> *qt_image_colortable(const QImage &image);
};

Q_DECLARE_SHARED(QImage)
Q_DECLARE_TYPEINFO(QImage, Q_MOVABLE_TYPE);

// QImage stream functions

#if !defined(QT_NO_DATASTREAM)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QImage &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QImage &);
#endif

#ifdef QT3_SUPPORT
Q_GUI_EXPORT QT3_SUPPORT void bitBlt(QImage* dst, int dx, int dy, const QImage* src,
                                     int sx=0, int sy=0, int sw=-1, int sh=-1, Qt::ImageConversionFlags flags = Qt::AutoColor);
#endif

QT_END_HEADER

#endif // QIMAGE_H
