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

#include <qplatformdefs.h>
#include "qfile.h"
#include <qlist.h>
#include <qfileengine.h>
#include <qfileinfo.h>
#include <qtemporaryfile.h>
#include <private/qiodevice_p.h>
#include <private/qfile_p.h>
#if defined(QT_BUILD_CORE_LIB)
# include "qcoreapplication.h"
#endif

#include <errno.h>

#define d d_func()
#define q q_func()

static const int read_cache_size = 4096;

//************* QFilePrivate
QFile::EncoderFn QFilePrivate::encoder = QFilePrivate::locale_encode;
QFile::DecoderFn QFilePrivate::decoder = QFilePrivate::locale_decode;

QFilePrivate::QFilePrivate() :
#ifndef QT_NO_FILE_BUFFER
    buffer(read_cache_size),
#endif
    fileEngine(0), isOpen(false), error(QFile::NoError)
{
}

QFilePrivate::~QFilePrivate()
{
    delete fileEngine;
    fileEngine = 0;
}

bool
QFilePrivate::openExternalFile(int flags, int fd)
{
    Q_ASSERT(!fileEngine || !q->isOpen());
    delete fileEngine;
    QFSFileEngine *fe = new QFSFileEngine;
    fileEngine = fe;
    return fe->open(flags, fd);
}

void 
QFilePrivate::setError(QFile::Error err)
{
    error = err;
    d->errorString.clear();
}

void 
QFilePrivate::setError(QFile::Error err, const QString &errStr)
{
    error = err;
    d->errorString = errStr;
}

void 
QFilePrivate::setError(QFile::Error err, int errNum)
{
    error = err;
    extern QString qt_errorstr(int errorCode); //qglobal.cpp
    errorString = qt_errorstr(errNum);
}

//************* QFile

/*!
    \class QFile
    \reentrant
    \brief The QFile class is an I/O device that operates on files.

    \ingroup io
    \mainclass

    QFile is an I/O device for reading and writing binary and text
    files. A QFile may be used by itself or, more conveniently, with a
    QDataStream or QTextStream.

    The file name is usually passed in the constructor, but it can be
    changed with setFileName(). You can check for a file's existence with
    exists(), and remove a file with remove().

    The file is opened with open(), closed with close(), and flushed
    with flush(). Data is usually read and written using QDataStream
    or QTextStream, but you can read with read() and readLine(),
    and write with write(). QFile also supports getch(), ungetch(),
    and putch().

    The size of the file is returned by size(). You can get the
    current file position or move to a new file position using the
    at() functions. If you've reached the end of the file, atEnd()
    returns true. The file handle is returned by handle().

    The following example uses QTextStream to read a text file
    line by line, printing each line with a line number:
    \code
    QStringList lines;
    QFile file("file.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line;
        int i = 1;
        while (!stream.atEnd()) {
            line = stream.readLine(); // line of text excluding '\n'
            printf("%3d: %s\n", i++, line.latin1());
            lines += line;
        }
        file.close();
    }
    \endcode

    Writing text is just as easy. The following example shows how to
    write the data we read in the previous example to a file:
    \code
    QFile file("file.txt");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        QStringList::ConstIterator i = lines.constBegin();
        for (; i != lines.constEnd(); ++i)
            stream << *i << "\n";
        file.close();
    }
    \endcode

    The QFileInfo class holds detailed information about a file, such
    as access permissions, file dates and file types.

    The QDir class manages directories and lists of file names.

    When you use QFile, QFileInfo, and QDir to access the file system
    with Qt, you can use Unicode file names. On Unix, these file names
    are converted to an 8-bit encoding. If you want to do your own
    file I/O on Unix, you should convert file names using the
    encodeName() and decodeName() functions to convert the file name
    into the local encoding.

    The conversion scheme can be changed using setEncodingFunction().
    This might be useful if you wish to give the user an option to
    store file names in UTF-8, for example, but be aware that such file
    names would probably then be unrecognizable when seen by other
    programs.

    On Windows NT/2000, Unicode file names are supported
    directly in the file system and this function should be avoided.
    On Windows 95, non-Latin1 locales are not supported.

    \sa QDataStream, QTextStream
*/

/*!
    \enum QFile::Permission

    This enum is used by the permission() function to report the
    permissions and ownership of a file. The values may be OR-ed
    together to test multiple permissions and ownership values.

    \value ReadOwner The file is readable by the owner of the file.
    \value WriteOwner The file is writable by the owner of the file.
    \value ExeOwner The file is executable by the owner of the file.
    \value ReadUser The file is readable by the user.
    \value WriteUser The file is writable by the user.
    \value ExeUser The file is executable by the user.
    \value ReadGroup The file is readable by the group.
    \value WriteGroup The file is writable by the group.
    \value ExeGroup The file is executable by the group.
    \value ReadOther The file is readable by anyone.
    \value WriteOther The file is writable by anyone.
    \value ExeOther The file is executable by anyone.

    \warning The semantics of \c ReadUser, \c WriteUser and \c ExeUser are
    unfortunately not platform independent: on Unix, the rights of the owner of
    the file are returned and on Windows the rights of the current user are
    returned. This behavior might change in a future Qt version. If you want to
    find the rights of the owner of the file, you should use the flags \c
    ReadOwner, \c WriteOwner and \c ExeOwner. If you want to find out the
    rights of the current user, you should use isReadable(), isWritable() and
    isExecutable().
*/

/*!
    Constructs a QFile with no name.
*/
QFile::QFile()
    : QIODevice(*new QFilePrivate)
{
    d_ptr = static_cast<QFilePrivate *>(QIODevice::d_ptr);
    setFlags(QIODevice::Direct);
    unsetError();
}

#ifndef QT_NO_QFILE_QOBJECT
/*!
    Constructs a QFile with no name.

    The \a parent is passed to the QObject constructor.
*/
QFile::QFile(QObject *parent) : QObject(parent), QIODevice(*new QFilePrivate)
{
    d_ptr = static_cast<QFilePrivate *>(QIODevice::d_ptr);
    setFlags(QIODevice::Direct);
    unsetError();
}
#endif

/*!
    Constructs a QFile with a file name \a name.

    \sa setFileName()
*/
QFile::QFile(const QString &name)
    : QIODevice(*new QFilePrivate)
{
    d_ptr = static_cast<QFilePrivate *>(QIODevice::d_ptr);
    d->fileName = name;
    setFlags(QIODevice::Direct);
    unsetError();
}

/*!
   \internal
*/
QFile::QFile(QFilePrivate &dd)
    : QIODevice(dd)
{
    d_ptr = static_cast<QFilePrivate *>(QIODevice::d_ptr);
    setFlags(QIODevice::Direct);
    unsetError();
}

/*!
    Destroys the file object, closing it if necessary.
*/
QFile::~QFile()
{
    close();
}

/*!
    Returns the name set by setFileName().

    \sa setFileName(), QFileInfo::fileName()
*/
QString
QFile::fileName() const
{
    return fileEngine()->fileName(QFileEngine::DefaultName);
}

/*!
    Sets the \a name of the file. The name can have no path, a
    relative path, or an absolute absolute path.

    Do not call this function if the file has already been opened.

    If the file name has no path or a relative path, the path used
    will be the application's current directory path
    \e{at the time of the open()} call.

    Example:
    \code
        QFile file;
        QDir::setCurrent("/tmp");
        file.setFileName("readme.txt");
        QDir::setCurrent("/home");
        file.open(QIODevice::ReadOnly);      // opens "/home/readme.txt" under Unix
    \endcode

    Note that the directory separator "/" works for all operating
    systems supported by Qt.

    \sa fileName(), QFileInfo, QDir
*/
void
QFile::setFileName(const QString &name)
{
    if (isOpen()) {
        qWarning("QFile::setFileName: file is already opened");
        close();
    }
    if(d->fileEngine) { //get a new file engine later
        delete d->fileEngine;
        d->fileEngine = 0;
    }
    d->fileName = name;
}

/*!
    \fn QString QFile::decodeName(const char *localFileName)

    \overload

    Returns the Unicode version of the given \a localFileName. See
    encodeName() for details.
*/

/*!
    By default, this function converts \a fileName to the local 8-bit
    encoding determined by the user's locale. This is sufficient for
    file names that the user chooses. File names hard-coded into the
    application should only use 7-bit ASCII filename characters.

    \sa decodeName() setEncodingFunction()
*/

QByteArray
QFile::encodeName(const QString &fileName)
{
    return (*QFilePrivate::encoder)(fileName);
}

/*!
    \enum QFile::EncoderFn

    This is used by QFile::setEncodingFunction() to specify how Unicode
    file names are converted to the appropriate local encoding.
*/


/*!
    This does the reverse of QFile::encodeName() using \a localFileName.

    \sa setDecodingFunction()
*/

QString
QFile::decodeName(const QByteArray &localFileName)
{
    return (*QFilePrivate::decoder)(localFileName);
}

/*!
    \fn void QFile::setEncodingFunction(EncoderFn function)

    \nonreentrant

    Sets the \a function for encoding Unicode file names. The
    default encodes in the locale-specific 8-bit encoding.

    \sa encodeName()
*/

void
QFile::setEncodingFunction(EncoderFn f)
{
    QFilePrivate::encoder = f;
}

/*!
    \enum QFile::DecoderFn

    This is used by QFile::setDecodingFunction() to specify how file names
    are converted from the local encoding to Unicode.
*/

/*!
    \fn void QFile::setDecodingFunction(DecoderFn function)

    \nonreentrant

    Sets the \a function for decoding 8-bit file names. The
    default uses the locale-specific 8-bit encoding.

    \sa encodeName(), decodeName()
*/

void
QFile::setDecodingFunction(DecoderFn f)
{
    QFilePrivate::decoder = f;
}

/*!
    \overload

    Returns true if the file specified by fileName() exists; otherwise
    returns false.

    \sa fileName() setFileName()
*/

bool
QFile::exists() const
{
    return (fileEngine()->fileFlags(QFileEngine::FlagsMask) & QFileEngine::ExistsFlag);
}

/*!
    Returns true if the file specified by \a fileName exists; otherwise
    returns false.
*/

bool
QFile::exists(const QString &fileName)
{
    return QFileInfo(fileName).exists();
}

/*!
    \overload

    Returns the name a symlink (or shortcut on Windows) points to, or
    a an empty string if the object isn't a symbolic link.

    This name may not represent an existing file; it is only a string.
    QFie::exists() returns true if the symlink points to an
    existing file.

    \sa fileName() setFileName()
*/

QString
QFile::readLink() const
{
    return fileEngine()->fileName(QFileEngine::LinkName);
}

/*!
    Returns the name a symlink (or shortcut on Windows) points to, or
    a an empty string if the object isn't a symbolic link.

    This name may not represent an existing file; it is only a string.
    QFie::exists() returns true if the symlink points to an
    existing file.
*/

QString
QFile::readLink(const QString &fileName)
{
    return QFileInfo(fileName).readLink();
}

/*!
    Removes the file specified by fileName(). Returns true if successful;
    otherwise returns false.

    The file is closed before it is removed.

    \sa setFileName()
*/

bool
QFile::remove()
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::remove: Empty or null file name");
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        if(fileEngine()->remove()) {
            unsetError();
            return true;
        }
        d->setError(QFile::RemoveError, errno);
    }
    return false;
}

/*!
    \overload

    Removes the file specified by the \a fileName given.

    Returns true if successful; otherwise returns false.

    \sa remove()
*/

bool
QFile::remove(const QString &fileName)
{
    return QFile(fileName).remove();
}

/*!
    Renames the file currently specified by fileName() to \a newName.
    Returns true if successful; otherwise returns false.

    The file is closed before it is renamed.

    \sa setFileName()
*/

bool
QFile::rename(const QString &newName)
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::rename: Empty or null file name");
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        if(fileEngine()->rename(newName)) {
            unsetError();
            return true;
        } else {
            QFile in(fileName());
            QFile out(newName);
            if (in.open(QIODevice::ReadOnly)) {
                if(out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    bool error = false;
                    char block[1024];
                    while(!atEnd()) {
                        Q_LONG read = in.read(block, 1024);
                        if(read == -1)
                            break;
                        if(read != out.write(block, read)) {
                            d->setError(QFile::CopyError, QLatin1String("Failure to write block"));
                            error = true;
                            break;
                        }
                    }
                    if(!error)
                        in.remove();
                    return !error;
                 }
            }
        }
        d->setError(QFile::RenameError, errno);
    }
    return false;
}

/*!
    \overload

    Renames the file \a oldName to \a newName. Returns true if
    successful; otherwise returns false.

    \sa rename()
*/

bool
QFile::rename(const QString &oldName, const QString &newName)
{
    return QFile(oldName).rename(newName);
}

/*!
    Creates a link from the file currently specified by fileName() to
    \a newName. What a link is depends on the underlying filesystem
    (be it a shortcut on Windows or a symbolic link on Unix). Returns
    true if successful; otherwise returns false.

    \sa setFileName()
*/

bool
QFile::link(const QString &newName)
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::link: Empty or null file name");
        return false;
    }
    if(fileEngine()->link(newName)) {
        unsetError();
        return true;
    }
    d->setError(QFile::RenameError, errno);
    return false;
}

/*!
    \overload

    Creates a link from \a oldName to \a newName. What a link is
    depends on the underlying filesystem (be it a shortcut on Windows
    or a symbolic link on Unix). Returns true if successful; otherwise
    returns false.

    \sa link()
*/

bool
QFile::link(const QString &oldName, const QString &newName)
{
    return QFile(oldName).link(newName);
}

/*!
    Copies the file currently specified by fileName() to \a newName.
    Returns true if successful; otherwise returns false.

    The file is closed before it is copied.

    \sa setFileName()
*/

bool
QFile::copy(const QString &newName)
{
    if (d->fileName.isEmpty()) {
        qWarning("QFile::copy: Empty or null file name");
        return false;
    }
    close();
    if(error() == QFile::NoError) {
        bool error = false;
        if(!open(QFile::ReadOnly)) {
            error = true;
            QString errorMessage = QLatin1String("Cannot open %1 for input");
            d->setError(QFile::CopyError, errorMessage.arg(d->fileName));
        } else {
            QTemporaryFile out;
            if(!out.open()) {
                close();
                error = true;
                d->setError(QFile::CopyError, QLatin1String("Cannot open for output"));
            } else {
                char block[1024];
                while(!atEnd()) {
                    Q_LONG in = read(block, 1024);
                    if(in == -1)
                        break;
                    if(in != out.write(block, in)) {
                        d->setError(QFile::CopyError, QLatin1String("Failure to write block"));
                        error = true;
                        break;
                    }
                }
                if(!error && !QFile::rename(out.fileName(), newName)) {
                    error = true;
                    QString errorMessage = QLatin1String("Cannot create %1 for output");
                    d->setError(QFile::CopyError, errorMessage.arg(newName));
                }
            }
        }
        if(!error) {
            QFile::setPermissions(newName, permissions());
            unsetError();
            return true;
        }
    }
    return false;
}

/*!
    \overload

    Copies the file \a fileName to \a newName. Returns true if successful;
    otherwise returns false.

    \sa rename()
*/

bool
QFile::copy(const QString &fileName, const QString &newName)
{
    return QFile(fileName).copy(newName);
}

/*!
    \overload

    Opens the existing file handle \a fh in the given \a mode.
    Returns true if successful; otherwise returns false.

    Example:
    \code
        #include <stdio.h>

        void printError(const char* msg)
        {
            QFile file;
            file.open(QIODevice::WriteOnly, stderr);
            file.write(msg, qstrlen(msg));        // write to stderr
            file.close();
        }
    \endcode

    When a QFile is opened using this function, close() does not actually
    close the file, but only flushes it.

    \warning If \a fh is \c stdin, \c stdout, or \c stderr, you may not be
    able to seek(). See QIODevice::isSequentialAccess() for more
    information.

    \sa close()
*/

bool
QFile::open(int mode, FILE *fh)
{
    return open(mode, QT_FILENO(fh));
}

/*!
    \reimp
*/

bool
QFile::open(int mode)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if(mode & Append) //append implies write
        mode |= WriteOnly;
    setFlags(QIODevice::Direct);
    unsetError();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QIODevice::open: File access not specified");
        return false;
    }
    if(fileEngine()->open(flags())) {
        d->isOpen = true;
        if(fileEngine()->isSequential())
            setType(Sequential);
        return true;
    }
    QFile::Error err = fileEngine()->error();
    if(err == QFile::UnspecifiedError)
        err = QFile::OpenError;
    d->setError(err, fileEngine()->errorString());
    return false;
}

/*!
    \overload

    Opens the existing file descripter \a fd in the given \a mode.
    Returns true if successful; otherwise returns false.

    When a QFile is opened using this function, close() does not
    actually close the file.

    The QFile that is opened using this function is automatically set
    to be in raw mode; this means that the file input/output functions
    are slow. If you run into performance issues, you should try to
    use one of the other open functions.

    \warning If \a fd is 0 (stdin), 1 (stdout), or 2 (stderr),
    you may not be able to seek(). size() is set to \c LLONG_MAX (in
    \c limits.h).

    \sa close()
*/

bool
QFile::open(int mode, int fd)
{
    if (isOpen()) {
        qWarning("QFile::open: File already open");
        return false;
    }
    if(mode & (Append|WriteOnly)) //append implies write
        mode |= WriteOnly;
    setFlags(QIODevice::Direct);
    unsetError();
    setMode(mode);
    if (!(isReadable() || isWritable())) {
        qWarning("QFile::open: File access not specified");
        return false;
    }
    if(d->openExternalFile(flags(), fd)) {
        d->isOpen = true;
        setMode(mode | QIODevice::Raw);
        if(fileEngine()->isSequential())
            setType(QIODevice::Sequential);
        return true;
    }
    return false;
}

/*!
    \reimp
*/

int
QFile::ungetch(int character)
{
    if (!isOpen()) {                                // file not open
        qWarning("QFile::ungetch: File not open");
        return EOF;
    }
    if (!isReadable()) {                        // reading not permitted
        qWarning("QFile::ungetch: Read operation not permitted");
        return EOF;
    }
    if (character == EOF)                        // cannot unget EOF
        return character;
#ifndef QT_NO_FILE_BUFFER
    d->buffer.push(character);
#else
    qWarning("must implement...");
    character = -1;
#endif
    return character;
}

/*!
    \reimp
*/

Q_LONGLONG
QFile::readLine(char *data, Q_LONGLONG maxSize)
{
    if (maxSize <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::readLine: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::readLine: Read operation not permitted");
        return -1;
    }

#ifndef QT_NO_FILE_BUFFER
    bool foundEnd = false;
    Q_LONGLONG ret = 0;
    //from buffer
    while(!foundEnd && !d->buffer.isEmpty() && ret < maxSize) {
        uint buffered = qMin(maxSize, (Q_LONGLONG)d->buffer.used()), len = 0;
        char *buffer = d->buffer.take(buffered, &buffered);
        for( ; len < buffered && len < maxSize-ret; len++) {
            if(*(buffer+len) == '\n') {
                foundEnd = true;
                len++;
                break;
            }
        }

        memcpy(data+ret, buffer, len);
        ret += len;
        d->buffer.free(len);
    }
    //from the device
    while(!foundEnd && ret < maxSize) {
        char *buffer = d->buffer.alloc(read_cache_size);
        Q_LONGLONG got = fileEngine()->read(buffer, read_cache_size);
        if(got == -1) {
            d->buffer.truncate(read_cache_size); //we read nothing!
            if(ret == 0)
                ret = -1;
            break;
        } else if(got < read_cache_size) {
            d->buffer.truncate(read_cache_size - got);
        }
        uint len = 0;
        for( ; len < got && len < maxSize-ret; len++) {
            if(*(buffer+len) == '\n') {
                foundEnd = true;
                len++;
                break;
            }
        }

        memcpy(data+ret, buffer, len);
        ret += len;
        d->buffer.free(len);
    }
    if(ret > 0 && ret != maxSize) 
        *(data + ret) = '\0';
    return ret;
#else
    return QIODevice::readLine(data, maxSize);
#endif
}

/*!
    Reads a line of text.

    Reads bytes from the file into the \a str until end-of-line or
    \a maxSize bytes have been read, whichever occurs first.
    Returns the number of bytes read, or -1 if there was an error
    (e.g. end of file). Any terminating newline is not stripped.

    This function is only efficient for buffered files. Avoid using
    readLine() for files that have been opened with the
    QIODevice::Raw flag.

    Note that the string is read as plain Latin-1 bytes, not Unicode.

    \sa read(), QTextStream::readLine()
*/

Q_LONGLONG
QFile::readLine(QString &str, Q_LONGLONG maxSize)
{
    QByteArray ba;
    ba.resize(maxSize);
    Q_LONGLONG l = readLine(ba.data(), maxSize);
    if (l > 0)
        str = QString::fromLatin1(ba);
    return l;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(). On systems that use file
  descriptors for sockets (i.e. Unix systems, but not Windows) the handle
  can be used with QSocketNotifier as well.

  If the file is not open, or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int
QFile::handle() const
{
    if (!isOpen())
        return -1;
    QFileEngine *engine = fileEngine();
    if(engine->type() == QFileEngine::File)
        return static_cast<QFSFileEngine*>(engine)->handle();
    return -1;
}

/*!
    \fn QString QFile::name() const

    Use fileName() instead.
*/

/*!
    \fn void QFile::setName(const QString &name)

    Use setFileName() instead.
*/

/*!
    Sets the file size (in bytes) \a sz. Returns true if the file if the
    resize succeeds; false otherwise. If \a sz is larger than the file
    currently is the new bytes will be set to 0, if \a sz is smaller the
    file is simply truncated.

    \sa QFile::size(), setFileName()
*/

bool
QFile::resize(QIODevice::Offset sz)
{
    if(fileEngine()->setSize(sz)) {
        unsetError();
        return true;
    }
    d->setError(QFile::ResizeError, errno);
    return false;
}

/*!
    \overload

    Sets \a fileName to size (in bytes) \a sz. Returns true if the file if
    the resize succeeds; false otherwise. If \a sz is larger than \a
    fileName currently is the new bytes will be set to 0, if \a sz is
    smaller the file is simply truncated.

    \sa resize()
*/

bool
QFile::resize(const QString &fileName, QIODevice::Offset sz)
{
    return QFile(fileName).resize(sz);
}

/*!
    Returns the complete OR-ed together combination of
    QFile::Permission for the file.

    \sa QFile::setPermissions, QFile::Permission, setFileName()
*/

QFile::Permissions
QFile::permissions() const
{
    QFileEngine::FileFlags perms = fileEngine()->fileFlags(QFileEngine::PermsMask) & QFileEngine::PermsMask;
    return QFile::Permissions((int)perms); //ewww
}

/*!
    \overload

    Returns the complete OR-ed together combination of
    QFile::Permission for \a fileName.

    \sa permissions(), QFile::Permission
*/

QFile::Permissions
QFile::permissions(const QString &fileName)
{
    return QFile(fileName).permissions();
}

/*!
    Sets the permissions for the file to \a permissions. 

    \sa permissions(), QFile::Permission, setFileName()
*/

bool
QFile::setPermissions(QFile::Permissions permissions)
{
    if(fileEngine()->chmod(permissions)) {
        unsetError();
        return true;
    }
    d->setError(QFile::PermissionsError, errno);
    return false;
}

/*!
    \overload

    Sets the permissions for \a fileName file to \a permissions. 

    \sa setPermissions(), QFile::Permission
*/

bool
QFile::setPermissions(const QString &fileName, QFile::Permissions permissions)
{
    return QFile(fileName).setPermissions(permissions);
}

/*!
  \reimp
*/

void
QFile::flush()
{
    fileEngine()->flush();
}

/*!
  \reimp
*/

void
QFile::close()
{
    if(!isOpen())
        return;

    d->isOpen = false;
    unsetError();
#ifndef QT_NO_FILE_BUFFER
    d->buffer.clear();
#endif
    if(!fileEngine()->close()) 
        d->setError(fileEngine()->error(), fileEngine()->errorString());
    else 
        setFlags(QIODevice::Direct);
}

/*! \reimp
*/
bool 
QFile::isOpen() const
{
    return d->isOpen;
}

/*!
  \reimp
*/

Q_LONGLONG QFile::size() const
{
    return fileEngine()->size();
}

/*!
  \reimp
*/

Q_LONGLONG QFile::at() const
{
    if (!isOpen())
        return 0;
#ifndef QT_NO_FILE_BUFFER
    return fileEngine()->at() - d->buffer.used();
#else
    return fileEngine()->at();
#endif
}

/*!
  \reimp
*/

bool QFile::seek(Q_LONGLONG off)
{
    if (!isOpen()) {
        qWarning("QFile::seek: IODevice is not open");
        return false;
    }
    if(!fileEngine()->seek(off)) {
        QFile::Error err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::PositionError;
        d->setError(err, fileEngine()->errorString());
        return false;
    }
#ifndef QT_NO_FILE_BUFFER
    d->buffer.clear();
#endif
    unsetError();
    return true;
}

/*!
  \reimp
*/

Q_LONGLONG QFile::read(char *data, Q_LONGLONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {
        qWarning("QFile::read: File not open");
        return -1;
    }
    if (!isReadable()) {
        qWarning("QFile::read: Read operation not permitted");
        return -1;
    }
    unsetError();

    Q_LONGLONG ret = 0;
#ifndef QT_NO_FILE_BUFFER
    //from buffer
    while(ret != len && !d->buffer.isEmpty()) {
        uint buffered = qMin(len, (Q_LONGLONG)d->buffer.used());
        char *buffer = d->buffer.take(buffered, &buffered);
        memcpy(data+ret, buffer, buffered);
        d->buffer.free(buffered);
        ret += buffered;
    }
    //from the device
    if(ret < len) {
        if(len > read_cache_size) {
            Q_LONGLONG read = fileEngine()->read(data+ret, len-ret);
            if(read != -1)
                ret += read;
        } else {
            char *buffer = d->buffer.alloc(read_cache_size);
            Q_LONGLONG got = fileEngine()->read(buffer, read_cache_size);
            if(got != -1) {
                if(got < read_cache_size)
                    d->buffer.truncate(read_cache_size - got);

                const Q_LONGLONG need = qMin(len-ret, got);
                memcpy(data+ret, buffer, need);
                d->buffer.free(need);
                ret += need;
            } else {
                if(!ret)
                    ret = -1;
                d->buffer.truncate(read_cache_size);
            }
        }
    }
#else
    Q_LONGLONG read = fileEngine()->read(data+ret, len-ret);
    if(read != -1)
        ret += read;
#endif
    if(ret < 0) {
        QFile::Error err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::ReadError;
        d->setError(err, fileEngine()->errorString());
    }
    return ret;
}

/*!
  \reimp
*/

Q_LONGLONG 
QFile::write(const char *data, Q_LONGLONG len)
{
    if (len <= 0) // nothing to do
        return 0;
    Q_CHECK_PTR(data);
    if (!isOpen()) {                                // file not open
        qWarning("QFile::write: File not open");
        return -1;
    }
    if (!isWritable()) {                        // writing not permitted
        qWarning("QFile::write: Write operation not permitted");
        return -1;
    }
    unsetError();

#ifndef QT_NO_FILE_BUFFER
    if(!d->buffer.isEmpty())
        seek(at());
#endif
    Q_LONGLONG ret = fileEngine()->write(data, len);
    if(ret < 0) {
        QFile::Error err = fileEngine()->error();
        if(err == QFile::UnspecifiedError)
            err = QFile::WriteError;
        d->setError(err, fileEngine()->errorString());
    }
    return ret;
}

/*!
  Returns the QIOEngine for this QFile object.
*/

QFileEngine
*QFile::fileEngine() const
{
    if(!d->fileEngine)
        d->fileEngine = QFileEngine::createFileEngine(d->fileName);
    return d->fileEngine;
}

/*!
    Returns the file error status.

    \keyword QFile::NoError
    \keyword QFile::ReadError
    \keyword QFile::WriteError
    \keyword QFile::FatalError
    \keyword QFile::OpenError
    \keyword QFile::ConnectError
    \keyword QFile::AbortError
    \keyword QFile::TimeOutError
    \keyword QFile::UnspecifiedError

    The I/O device status returns an error code. For example, if open()
    returns false, or a read/write operation returns -1, this function can
    be called to find out the reason why the operation failed.

    The status codes are:
    \table
    \header \i Status code \i Meaning
    \row \i \c QFile::NoError \i The operation was successful.
    \row \i \c QFile::ReadError \i Could not read from the device.
    \row \i \c QFile::WriteError \i Could not write to the device.
    \row \i \c QFile::FatalError \i A fatal unrecoverable error occurred.
    \row \i \c QFile::OpenError \i Could not open the device.
    \row \i \c QFile::ConnectError \i Could not connect to the device.
    \row \i \c QFile::AbortError \i The operation was unexpectedly aborted.
    \row \i \c QFile::TimeOutError \i The operation timed out.
    \row \i \c QFile::UnspecifiedError \i An unspecified error happened on close.
    \endtable

    \sa unsetError()
*/

QFile::Error
QFile::error() const
{
    return d->error;
}

/*!
    Sets the file's error to \c QFile::NoError.

    \sa error()
*/
void 
QFile::unsetError()
{
    d->setError(QFile::NoError);
}

/*!
    Returns a human-readable description of an error that occurred on
    the device. The error described by the string corresponds to
    changes of QFile::error(). If the status is reset, the error
    string is also reset.

    \code
        QFile file("address.dat");
        if (!file.open(QIODevice::ReadOnly) {
            QMessageBox::critical(this, tr("Error"),
                    tr("Could not open file for reading: %1")
                    .arg(file.errorString()));
            return;
        }
    \endcode

    \sa unsetError()
*/

QString 
QFile::errorString() const
{
    if (d->errorString.isEmpty()) {
        const char *str = 0;
        switch (d->error) {
        case NoError:
        case UnspecifiedError:
            str = QT_TRANSLATE_NOOP("QFile", "Unknown error");
            break;
        case ReadError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not read from the file");
            break;
        case WriteError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not write to the file");
            break;
        case FatalError:
            str = QT_TRANSLATE_NOOP("QFile", "Fatal error");
            break;
        case ResourceError:
            str = QT_TRANSLATE_NOOP("QFile", "Resource error");
            break;
        case OpenError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not open the file");
            break;
#ifdef QT_COMPAT
        case ConnectError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not connect to host");
            break;
#endif
        case AbortError:
            str = QT_TRANSLATE_NOOP("QFile", "Aborted");
            break;
        case TimeOutError:
            str = QT_TRANSLATE_NOOP("QFile", "Timeout");
            break;
        case RemoveError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not remove file");
            break;
        case RenameError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not rename file");
            break;
        case PositionError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not position in file");
            break;
        case PermissionsError:
            str = QT_TRANSLATE_NOOP("QFile", "Failure to set Permissions");
            break;
        case CopyError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not copy file");
            break;
        case ResizeError:
            str = QT_TRANSLATE_NOOP("QFile", "Could not resize file");
            break;
        }
#if defined(QT_BUILD_CORE_LIB)
        QString ret = QCoreApplication::translate("QFile", str);
#ifdef QT_COMPAT
        if(ret == str)
            ret = QCoreApplication::translate("QIODevice", str);
#endif
        return ret;
#else
        return QString::fromLatin1(str);
#endif
    }
    return d->errorString;
}
