/****************************************************************************
**
** Implementation of Mac flavor <-> clipboard converters.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmime.h"

#ifndef QT_NO_MIME

#include "qstrlist.h"
#include "qimage.h"
#include "qdatastream.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qapplication_p.h"
#include "qtextcodec.h"
#include "qregexp.h"
#include "qsettings.h"
#include "qmap.h"
#include "qt_mac.h"

static QPtrList<QMacMime> mimes;

//functions
OSErr FSpLocationFromFullPath(short, const void *, FSSpec *); //qsound_mac.cpp

/*!
  \class QMacMime
  \brief The QMacMime class maps open-standard MIME to Mac flavors.
  \ingroup io
  \ingroup draganddrop
  \ingroup misc

  Qt's drag-and-drop and clipboard facilities use the MIME standard.
  On X11, this maps trivially to the Xdnd protocol, but on Mac
  although some applications use MIME types to describe clipboard
  formats, others use arbitrary non-standardized naming conventions,
  or unnamed built-in Mac formats.

  By instantiating subclasses of QMacMime that provide conversions
  between Mac flavors and MIME formats, you can convert proprietary
  clipboard formats to MIME formats.

  Qt has predefined support for the following Mac flavors:
  \list
    \i kScrapFlavorTypeUnicode - converted to "text/plain;charset=ISO-10646-UCS-2"
	    and supported by QTextDrag.
    \i kScrapFlavorTypeText - converted to "text/plain;charset=system" or "text/plain"
	    and supported by QTextDrag.
    \i kScrapFlavorTypePicture - converted to "image/*", where * is
		a \link QImage::outputFormats() Qt image format\endlink,
	    and supported by QImageDrag.
    \i kDragFlavorTypeHFS - converted to "text/uri-list",
	    and supported by QUriDrag.
  \endlist

  You can check if a MIME type is convertible using canConvert() and
  can perform conversions with convertToMime() and convertFromMime().
*/

/*! \enum QMacMime::QMacMimeType
    \internal
*/

/*!
  Constructs a new conversion object of type \a t, adding it to the
  globally accessed list of available convertors.
*/
QMacMime::QMacMime(char t) : type(t)
{
    mimes.append(this);
}

/*!
  Destroys a conversion object, removing it from the global
  list of available convertors.
*/
QMacMime::~QMacMime()
{
    mimes.remove(this);
}

ScrapFlavorType qt_mac_mime_type = 'CUTE';
class QMacMimeAnyMime : public QMacMime {
private:
    QMap<QString, int> mime_registry;
    int registerMimeType(const char *mime);
    bool loadMimeRegistry();

public:
    QMacMimeAnyMime() : QMacMime(MIME_QT_CONVERTOR|MIME_ALL) { }
    int		countFlavors();
    const char* convertorName();
    int		flavor(int index);
    int		flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool	canConvert(const char* mime, int flav);
    QByteArray	           convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

bool QMacMimeAnyMime::loadMimeRegistry()
{
    QSettings mime_settings;
    mime_settings.setPath("MimeRegistry", "qt");
    mime_registry.clear();
    QStringList entries = mime_settings.subkeyList("/mimetypes/keys");
    for(QStringList::ConstIterator it=entries.begin(); it != entries.end(); ++it) {
	bool ok = FALSE;
	int mac_t = mime_settings.readNumEntry(QString("/mimetypes/keys/") + (*it) + "/mac_type", 0, &ok);
	if(!ok) {
	    qWarning("That shouldn't happen!! %s", (*it).latin1());
	    continue;
	}
	QString qt_t = mime_settings.readEntry(QString("/mimetypes/keys/") + (*it) + "/qt_type", "", &ok);
	if(!ok) {
	    qWarning("That shouldn't happen!! %s", (*it).latin1());
	    continue;
	}
	mime_registry.insert(qt_t, mac_t);
    }
    return TRUE;
}

int QMacMimeAnyMime::registerMimeType(const char *mime)
{
    if(!mime_registry.contains(mime)) {
	if(!loadMimeRegistry()) {
	    qWarning("That shouldn't happen!");
	    return 0;
	}
	if(!mime_registry.contains(mime)) {
	    QSettings mime_settings;
	    mime_settings.setPath("MimeRegistry", "qt");
	    mime_settings.beginGroup("/mimetypes/");
	    int ser = mime_settings.readNumEntry("serial_reg", 'QT00');
	    int ret = ser;
	    mime_settings.writeEntry(QString("keys/mime_type") + QString::number(ret - 'QT00') + "/mac_type", ret);
	    mime_settings.writeEntry(QString("keys/mime_type") + QString::number(ret - 'QT00') + "/qt_type", mime);
	    mime_settings.writeEntry("serial_reg", ++ser);
	    return ret;
	}
    }
    return mime_registry[mime];
}

int QMacMimeAnyMime::countFlavors()
{
    loadMimeRegistry();
    return mime_registry.count();
}

const char* QMacMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

int QMacMimeAnyMime::flavor(int index)
{
    loadMimeRegistry();
    int i = 0;
    for(QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it, ++i) {
	if(i == index)
	    return it.data();
    }
    return 0;
}

int QMacMimeAnyMime::flavorFor(const char* mime)
{
    return registerMimeType(mime);
}

const char* QMacMimeAnyMime::mimeFor(int flav)
{
    loadMimeRegistry();
    for(QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it) {
	if(it.data() == flav)
	    return it.key();
    }
    return NULL;
}

bool QMacMimeAnyMime::canConvert(const char* mime, int flav)
{
    loadMimeRegistry();
    if(mime_registry.contains(mime) && mime_registry[mime] == flav)
	return TRUE;
    return FALSE;
}

QByteArray QMacMimeAnyMime::convertToMime(QList<QByteArray> data, const char* , int)
{
    if(data.count() > 1)
	qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeAnyMime::convertFromMime(QByteArray data, const char* , int)
{
    QList<QByteArray> ret;
    ret.append(data);
    return ret;
}


class QMacMimeText : public QMacMime {
public:
    QMacMimeText() : QMacMime(MIME_ALL) { }
    int		countFlavors();
    const char* convertorName();
    int		flavor(int index);
    int		flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool	canConvert(const char* mime, int flav);
    QByteArray	           convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeText::countFlavors()
{
    return 2;
}

const char* QMacMimeText::convertorName()
{
    return "Text";
}

int QMacMimeText::flavor(int index)
{
    if(index == 0)
	return kScrapFlavorTypeUnicode;
    return kScrapFlavorTypeText;
}

int QMacMimeText::flavorFor(const char* mime)
{
    if(!qstricmp(mime, "text/plain"))
	return kScrapFlavorTypeText;
    QByteArray m(mime);
    int i = m.find("charset=");
    if(i >= 0) {
	QByteArray cs(m.data()+i+8);
	i = cs.find(";");
	if(i>=0)
	    cs = cs.left(i);
	if(cs == "system")
	    return kScrapFlavorTypeText;
	else if(cs == "ISO-10646-UCS-2" || cs == "utf16")
	    return kScrapFlavorTypeUnicode;
    }
    return 0;
}

const char* QMacMimeText::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypeText)
	return "text/plain";
    else if(flav == kScrapFlavorTypeUnicode)
	return "text/plain;charset=ISO-10646-UCS-2";
    return NULL;
}

bool QMacMimeText::canConvert(const char* mime, int flav)
{
    return flav && flavorFor(mime) == flav;
}

QByteArray QMacMimeText::convertToMime(QList<QByteArray> data, const char*, int)
{
    if(data.count() > 1)
	qWarning("QMacMimeText: cannot handle multiple member data");
    return data.first();
}

QList<QByteArray> QMacMimeText::convertFromMime(QByteArray data, const char*, int)
{
    QList<QByteArray> ret;
    ret.append(data);
    return ret;
}


class QMacMimeImage : public QMacMime {
public:
    QMacMimeImage() : QMacMime(MIME_ALL) { }
    int		countFlavors();
    const char* convertorName();
    int		flavor(int index);
    int		flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool	canConvert(const char* mime, int flav);
    QByteArray	           convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeImage::countFlavors()
{
    return 1;
}

const char* QMacMimeImage::convertorName()
{
    return "Image";
}

int QMacMimeImage::flavor(int)
{
    return kScrapFlavorTypePicture;
}

int QMacMimeImage::flavorFor(const char* mime)
{
    if(!qstrnicmp(mime,"image/",5)) {
	QStrList ofmts = QImage::outputFormats();
	for(const char* fmt=ofmts.first(); fmt; fmt=ofmts.next()) {
	    if(!qstricmp(fmt,mime+6))
		return kScrapFlavorTypePicture;
	}
    }
    return 0;
}

const char* QMacMimeImage::mimeFor(int flav)
{
    if(flav == kScrapFlavorTypePicture)
	return "image/png";
    return 0;
}

bool QMacMimeImage::canConvert(const char* mime, int flav)
{
    if(flav == kScrapFlavorTypePicture && !qstrnicmp(mime,"image/",5)) {
	QStrList ofmts = QImage::outputFormats();
	for(const char* fmt=ofmts.first(); fmt; fmt=ofmts.next()) {
	    if(!qstricmp(fmt,mime+6))
		return TRUE;
	}
    }
    return FALSE;
}

QByteArray QMacMimeImage::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(data.count() > 1)
	qWarning("QMacMimeAnyMime: cannot handle multiple member data");
    QByteArray ret;
    if(qstrnicmp(mime,"image/",6) || flav != kScrapFlavorTypePicture)
	return ret;
    QByteArray &a = data.first();
    PicHandle pic = (PicHandle)NewHandle(a.size());
    memcpy(*pic, a.data(), a.size());
    PictInfo pinfo;
    if(GetPictInfo(pic, &pinfo, 0, 0, 0, 0) == noErr) {
	QPixmap px(pinfo.sourceRect.right - pinfo.sourceRect.left,
		   pinfo.sourceRect.bottom - pinfo.sourceRect.top, 32);
	{
	    Rect r; SetRect(&r, 0, 0, px.width(), px.height());
	    QMacSavedPortInfo pi(&px);
	    DrawPicture(pic, &r);
	}
	QByteArray ofmt = mime+6;
	QBuffer iod(ret);
	iod.open(IO_WriteOnly);
	QImage img = px.convertToImage();
	QImageIO iio(&iod, ofmt.upper());
	iio.setImage(img);
	if(iio.write()) 
	    iod.close();
    }
    DisposeHandle((Handle)pic);
    return ret;
}

QList<QByteArray> QMacMimeImage::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstrnicmp(mime,"image/",6) || flav != kScrapFlavorTypePicture)
	return ret;
    QPixmap px;
    QByteArray ar;
    {
	QImage img;
	img.loadFromData((unsigned char*)data.data(),data.size());
	if (img.isNull())
	    return ret;
	px = img;
    }
    Rect r; SetRect(&r, 0, 0, px.width(), px.height());    
    PicHandle pic = OpenPicture(&r);
    {
	GWorldPtr world;
	GDHandle handle;
	GetGWorld(&world, &handle);
	CopyBits(GetPortBitMapForCopyBits((GWorldPtr)px.handle()), 
		 GetPortBitMapForCopyBits((GWorldPtr)world), &r, &r, srcCopy, 0);
    }
    ClosePicture();
    int size = GetHandleSize((Handle)pic);
    HLock((Handle)pic);
    ar.setRawData((char *)*pic, size);
    HUnlock((Handle)pic);
    ret.append(ar);
    return ret;
}


class QMacMimeFileUri : public QMacMime {
public:
    QMacMimeFileUri() : QMacMime(MIME_DND) { }
    int		countFlavors();
    const char* convertorName();
    int		flavor(int index);
    int		flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool	canConvert(const char* mime, int flav);
    QByteArray	           convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeFileUri::countFlavors()
{
    return 1;
}

const char* QMacMimeFileUri::convertorName()
{
    return "FileURL";
}

int QMacMimeFileUri::flavor(int)
{
    return typeFileURL;
}

int QMacMimeFileUri::flavorFor(const char* mime)
{
    if(qstricmp(mime,"text/uri-list"))
	return 0;
    return (int)typeFileURL;
}

const char* QMacMimeFileUri::mimeFor(int flav)
{
    if(flav == typeFileURL)
	return "text/uri-list";
    return NULL;
}

bool QMacMimeFileUri::canConvert(const char* mime, int flav)
{
    if(!qstricmp(mime,"text/uri-list"))
	return flav == typeFileURL;
    return FALSE;
}

QByteArray QMacMimeFileUri::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(qstricmp(mime,"text/uri-list") || flav != typeFileURL)
	return QByteArray();
    int done = 0;
    QByteArray ret;
    for(QList<QByteArray>::Iterator it = data.begin(); it != data.end(); ++it) {
	QByteArray tmp_str(QString::fromUtf8((*it).data(), (*it).size()));
	if(tmp_str.left(17) == "file://localhost/") //mac encodes a differently
	    tmp_str = "file:///" + tmp_str.mid(17);
	int l = tmp_str.length();
	ret.resize(ret.size()+(l+2));
	memcpy(ret.data()+done,tmp_str.data(),l);
	memcpy(ret.data()+l+done,"\r\n",2);
	done += l + 2;
    }
    return ret;
}

QList<QByteArray> QMacMimeFileUri::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstricmp(mime,"text/uri-list") || flav != typeFileURL)
	return ret;
    int len = 0;
    char *buffer = (char *)malloc(data.size());
    for(int i = 0; i < data.size(); i++) {
	if(data[i] == '\r' && i < data.size()-1 && data[i+1] == '\n')
	    break;
	buffer[len++] = data[i];
    }
    if(!qstrncmp(buffer, "file:///", 8)) { //Mac likes localhost to be in it!
	if(len + 9 > data.size())
	    buffer = (char *)realloc(buffer, len + 9);
	qstrncpy(buffer + 7, buffer + 9 + 7, len - 7);
	qstrncpy(buffer + 7, "localhost", 9);
	len += 9;
    }
    free(buffer);
    ret.append(data);
    return ret;
}

class QMacMimeHFSUri : public QMacMime {
public:
    QMacMimeHFSUri() : QMacMime(MIME_DND) { }
    int		countFlavors();
    const char* convertorName();
    int		flavor(int index);
    int		flavorFor(const char* mime);
    const char* mimeFor(int flav);
    bool	canConvert(const char* mime, int flav);
    QByteArray	           convertToMime(QList<QByteArray> data, const char* , int);
    QList<QByteArray> convertFromMime(QByteArray data, const char* , int);
};

int QMacMimeHFSUri::countFlavors()
{
    return 1;
}

const char* QMacMimeHFSUri::convertorName()
{
    return "HFSUri";
}

int QMacMimeHFSUri::flavor(int)
{
    return kDragFlavorTypeHFS;
}

int QMacMimeHFSUri::flavorFor(const char* mime)
{
    if(qstricmp(mime,"text/uri-list"))
	return 0;
    return (int)kDragFlavorTypeHFS;
}

const char* QMacMimeHFSUri::mimeFor(int flav)
{
    if(flav == kDragFlavorTypeHFS)
	return "text/uri-list";
    return NULL;
}

bool QMacMimeHFSUri::canConvert(const char* mime, int flav)
{
    if(!qstricmp(mime,"text/uri-list"))
	return flav == kDragFlavorTypeHFS;
    return FALSE;
}

QByteArray QMacMimeHFSUri::convertToMime(QList<QByteArray> data, const char* mime, int flav)
{
    if(qstricmp(mime,"text/uri-list") || flav != kDragFlavorTypeHFS)
	return QByteArray();
    int done = 0;
    QByteArray ret;
    char *buffer = (char *)malloc(1024);
    for(QList<QByteArray>::Iterator it = data.begin(); it != data.end(); ++it) {
	FSRef fsref;
	HFSFlavor *hfs = (HFSFlavor *)(*it).data();
	FSpMakeFSRef(&hfs->fileSpec, &fsref);
	FSRefMakePath(&fsref, (UInt8 *)buffer, 1024);
	QByteArray s = QUriDrag::localFileToUri(QString::fromUtf8((const char *)buffer));
	//now encode them to be handled by quridrag
	int l = qstrlen(s);
	ret.resize(ret.size()+(l+2));
	memcpy(ret.data()+done,s,l);
	memcpy(ret.data()+l+done,"\r\n",2);
	done += l + 2;
    }
    free(buffer);
    return ret;
}

QList<QByteArray> QMacMimeHFSUri::convertFromMime(QByteArray data, const char* mime, int flav)
{
    QList<QByteArray> ret;
    if(qstricmp(mime,"text/uri-list") || flav != kDragFlavorTypeHFS)
	return ret;
    HFSFlavor hfs;
    hfs.fileType = 'TEXT';
    hfs.fileCreator = 'CUTE';
    hfs.fdFlags = 0;
    FSpLocationFromFullPath(data.size(), data.data(), &hfs.fileSpec);
    ret.append(data);
    return ret;
}

static void cleanup_mimes()
{
    QMacMime* wm;
    while((wm = mimes.first()))
	delete wm;
}

/*!
  \internal

  This is an internal function.
*/
void QMacMime::initialize()
{
    if(mimes.isEmpty()) {
	qAddPostRoutine(cleanup_mimes);
	new QMacMimeImage;
	new QMacMimeText;
	new QMacMimeFileUri;
	new QMacMimeHFSUri;
	new QMacMimeAnyMime;
    }
}

/*!
  Returns the most-recently created QMacMime of type \a t that can convert
  between the \a mime and \a flav formats.  Returns 0 if no such convertor
  exists.
*/
QMacMime*
QMacMime::convertor(QMacMimeType t, const char *mime, int flav)
{
    // return nothing for illegal requests
    if(!flav)
	return 0;

    QMacMime* wm;
    for(wm = mimes.first(); wm; wm = mimes.next()) {
	if((wm->type & t) && wm->canConvert(mime,flav)) {
	    return wm;
	}
    }
    return 0;
}

/*!
  Returns a MIME type of type \a t for \a flav, or 0 if none exists.
*/
const char* QMacMime::flavorToMime(QMacMimeType t, int flav)
{
    const char* m=0;
    for(QMacMime *wm = mimes.first(); wm && !m; wm = mimes.next()) {
	if(wm->type & t)
	    m = wm->mimeFor(flav);
    }
    return m;
}

/*!
  Returns a list of all currently defined QMacMime objects of type \a t.
*/
QPtrList<QMacMime> QMacMime::all(QMacMimeType t)
{
    QPtrList<QMacMime> ret;
    for(QMacMime *wm = mimes.first(); wm; wm = mimes.next()) {
	if(wm->type & t)
	    ret.append(wm);
    }
    return ret;
}

/*!
  \fn const char* QMacMime::convertorName()

  Returns a name for the convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::countFlavors()

  Returns the number of Mac flavors supported by this convertor.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::flavor(int index)

  Returns the Mac flavor supported by this convertor that is
  ordinarily at position \a index. This means that flavor(0) returns
  the first Mac flavor supported, and flavor(countFlavors()-1) returns
  the last. If \a index is out of range the return value is undefined.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn bool QMacMime::canConvert(const char* mime, int flav)

  Returns TRUE if the convertor can convert (both ways) between
  \a mime and \a flav; otherwise returns FALSE.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn const char* QMacMime::mimeFor(int flav)

  Returns the MIME type used for Mac flavor \a flav, or 0 if this
  convertor does not support \a flav.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn int QMacMime::flavorFor(const char* mime)

  Returns the Mac flavor used for MIME type \a mime, or 0 if this
  convertor does not support \a mime.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QByteArray QMacMime::convertToMime(QList<QByteArray> data, const char* mime, int flav)

  Returns \a data converted from Mac flavor \a flav to MIME type \a
    mime.

  Note that Mac flavors must all be self-terminating.  The input \a
  data may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

/*!
  \fn QList<QByteArray> QMacMime::convertFromMime(QByteArray data, const char* mime, int flav)

  Returns \a data converted from MIME type \a mime
    to Mac flavor \a flav.

  Note that Mac flavors must all be self-terminating.  The return
  value may contain trailing data.

  All subclasses must reimplement this pure virtual function.
*/

#endif // QT_NO_MIME
