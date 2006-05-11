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

#include <qdebug.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <qglframebufferobject.h>
#include <qlibrary.h>
#include <qimage.h>

// #define DEPTH_BUFFER

#ifndef GL_EXT_framebuffer_object
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                    0x0506
#define GL_MAX_RENDERBUFFER_SIZE_EXT                            0x84E8
#define GL_FRAMEBUFFER_BINDING_EXT                              0x8CA6
#define GL_RENDERBUFFER_BINDING_EXT                             0x8CA7
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT               0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT               0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT             0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT     0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT        0x8CD4
#define GL_FRAMEBUFFER_COMPLETE_EXT                             0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT                0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT        0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT      0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT                0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT                   0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT               0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT               0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT                          0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS_EXT                            0x8CDF
#define GL_COLOR_ATTACHMENT0_EXT                                0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT                                0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT                                0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT                                0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT                                0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT                                0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT                                0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT                                0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT                                0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT                                0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT                               0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT                               0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT                               0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT                               0x8CED
#define GL_COLOR_ATTACHMENT14_EXT                               0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT                               0x8CEF
#define GL_DEPTH_ATTACHMENT_EXT                                 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT                               0x8D20
#define GL_FRAMEBUFFER_EXT                                      0x8D40
#define GL_RENDERBUFFER_EXT                                     0x8D41
#define GL_RENDERBUFFER_WIDTH_EXT                               0x8D42
#define GL_RENDERBUFFER_HEIGHT_EXT                              0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                     0x8D44
#define GL_STENCIL_INDEX_EXT                                    0x8D45
#define GL_STENCIL_INDEX1_EXT                                   0x8D46
#define GL_STENCIL_INDEX4_EXT                                   0x8D47
#define GL_STENCIL_INDEX8_EXT                                   0x8D48
#define GL_STENCIL_INDEX16_EXT                                  0x8D49
#define GL_RENDERBUFFER_RED_SIZE_EXT                            0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_EXT                          0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_EXT                           0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_EXT                          0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT                          0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_EXT                        0x8D55
#endif

// ### hm. should be part of the GL 1.2 spec..
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812F
#endif

#ifndef Q_WS_MAC
#ifndef APIENTRYP
#ifdef APIENTRY
#define APIENTRYP APIENTRY *
#else
#define APIENTRYP *
#endif
#endif

typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);

PFNGLISRENDERBUFFEREXTPROC qt_glIsRenderbufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC qt_glBindRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC qt_glDeleteRenderbuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC qt_glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC qt_glRenderbufferStorageEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC qt_glGetRenderbufferParameterivEXT;
PFNGLISFRAMEBUFFEREXTPROC qt_glIsFramebufferEXT;
PFNGLBINDFRAMEBUFFEREXTPROC qt_glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC qt_glDeleteFramebuffersEXT;
PFNGLGENFRAMEBUFFERSEXTPROC qt_glGenFramebuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC qt_glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC qt_glFramebufferTexture1DEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC qt_glFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC qt_glFramebufferTexture3DEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC qt_glFramebufferRenderbufferEXT;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC qt_glGetFramebufferAttachmentParameterivEXT;
PFNGLGENERATEMIPMAPEXTPROC qt_glGenerateMipmapEXT;

#define glIsRenderbufferEXT qt_glIsRenderbufferEXT
#define glBindRenderbufferEXT qt_glBindRenderbufferEXT
#define glDeleteRenderbuffersEXT qt_glDeleteRenderbuffersEXT
#define glGenRenderbuffersEXT qt_glGenRenderbuffersEXT
#define glRenderbufferStorageEXT qt_glRenderbufferStorageEXT
#define glGetRenderbufferParameterivEXT qt_glGetRenderbufferParameterivEXT
#define glIsFramebufferEXT qt_glIsFramebufferEXT
#define glBindFramebufferEXT qt_glBindFramebufferEXT
#define glDeleteFramebuffersEXT qt_glDeleteFramebuffersEXT
#define glGenFramebuffersEXT qt_glGenFramebuffersEXT
#define glCheckFramebufferStatusEXT qt_glCheckFramebufferStatusEXT
#define glFramebufferTexture1DEXT qt_glFramebufferTexture1DEXT
#define glFramebufferTexture2DEXT qt_glFramebufferTexture2DEXT
#define glFramebufferTexture3DEXT qt_glFramebufferTexture3DEXT
#define glFramebufferRenderbufferEXT qt_glFramebufferRenderbufferEXT
#define glGetFramebufferAttachmentParameterivEXT qt_glGetFramebufferAttachmentParameterivEXT
#define glGenerateMipmapEXT qt_glGenerateMipmapEXT

#if defined(Q_WS_X11)
static bool qt_resolve_framebufferobject_extensions()
{
    static bool resolved = false;
    if (resolved && qt_glIsRenderbufferEXT)
        return true;
    else if (resolved)
        return false;

    QLibrary lib("GL");

    qt_glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) lib.resolve("glIsRenderbufferEXT");
    qt_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) lib.resolve("glBindRenderbufferEXT");
    qt_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) lib.resolve("glDeleteRenderbuffersEXT");
    qt_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) lib.resolve("glGenRenderbuffersEXT");
    qt_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) lib.resolve("glRenderbufferStorageEXT");
    qt_glGetRenderbufferParameterivEXT =
        (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) lib.resolve("glGetRenderbufferParameterivEXT");
    qt_glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) lib.resolve("glIsFramebufferEXT");
    qt_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) lib.resolve("glBindFramebufferEXT");
    qt_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) lib.resolve("glDeleteFramebuffersEXT");
    qt_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) lib.resolve("glGenFramebuffersEXT");
    qt_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) lib.resolve("glCheckFramebufferStatusEXT");
    qt_glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) lib.resolve("glFramebufferTexture1DEXT");
    qt_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) lib.resolve("glFramebufferTexture2DEXT");
    qt_glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) lib.resolve("glFramebufferTexture3DEXT");
    qt_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) lib.resolve("glFramebufferRenderbufferEXT");
    qt_glGetFramebufferAttachmentParameterivEXT =
        (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) lib.resolve("glGetFramebufferAttachmentParameterivEXT");
    qt_glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) lib.resolve("glGenerateMipmapEXT");
    resolved = true;
    return qt_glIsRenderbufferEXT;
}
#elif defined(Q_WS_WIN)
static bool qt_resolve_framebufferobject_extensions()
{
    static bool resolved = false;
    if (resolved && qt_glIsRenderbufferEXT)
        return true;
    else if (resolved)
        return false;

    if (wglGetCurrentContext() == 0) {
	qWarning("QGLFramebufferObject: Unable to resolve framebuffer object extensions -"
		 " make sure there is a current context when creating the framebuffer object.");
	return false;
    }

    qt_glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) wglGetProcAddress("glIsRenderbufferEXT");
    qt_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
    qt_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
    qt_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) wglGetProcAddress("glGenRenderbuffersEXT");
    qt_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) wglGetProcAddress("glRenderbufferStorageEXT");
    qt_glGetRenderbufferParameterivEXT =
        (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) wglGetProcAddress("glGetRenderbufferParameterivEXT");
    qt_glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) wglGetProcAddress("glIsFramebufferEXT");
    qt_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");
    qt_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
    qt_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
    qt_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
    qt_glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) wglGetProcAddress("glFramebufferTexture1DEXT");
    qt_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
    qt_glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) wglGetProcAddress("glFramebufferTexture3DEXT");
    qt_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
    qt_glGetFramebufferAttachmentParameterivEXT =
        (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
    qt_glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) wglGetProcAddress("glGenerateMipmapEXT");
    resolved = true;
    return qt_glIsRenderbufferEXT;
}
#endif
#else
static bool qt_resolve_framebufferobject_extensions()
{
    return true; // assume these are always available on Mac OS X
}
#endif

#define QT_CHECK_GLERROR()                                \
{                                                         \
    GLenum err = glGetError();                            \
    if (err != GL_NO_ERROR) {                             \
        qDebug("[%s line %d] GL Error: %s",               \
               __FILE__, __LINE__, gluErrorString(err));  \
    }                                                     \
}

class QGLFramebufferObjectPrivate
{
public:
    QGLFramebufferObjectPrivate() : valid(false) {}
    ~QGLFramebufferObjectPrivate() {}

    void init(const QSize& sz, GLenum texture_target);
    bool checkFramebufferStatus() const;
    GLuint texture;
    GLuint fbo;
    GLuint depth_buffer;
    GLenum target;
    QSize size;
    uint valid : 1;
};

bool QGLFramebufferObjectPrivate::checkFramebufferStatus() const
{
#ifdef GL_FRAMEBUFFER_EXT
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
    case GL_NO_ERROR:
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        return true;
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        qDebug("QGLFramebufferObject: Unsupported framebuffer format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, duplicate attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same dimensions.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, attached images must have same format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing draw buffer.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        qDebug("QGLFramebufferObject: Framebuffer incomplete, missing read buffer.");
        break;
    default:
        qDebug() <<"QGLFramebufferObject: An undefined error has occured: "<< status;
        break;
    }
#endif
    return false;
}

void QGLFramebufferObjectPrivate::init(const QSize &sz, GLenum texture_target)
{
    // ### this stuff needs to be exported to work under windows - fix this
    if (//(QGLExtensions::glExtensions & QGLExtensions::FramebufferObject)
        //&&
	!qt_resolve_framebufferobject_extensions())
        return;

    size = sz;
    target = texture_target;
    // texture dimensions

    glGetError(); // reset error state
    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    QT_CHECK_GLERROR();
    // init texture
    glGenTextures(1, &texture);
    glBindTexture(target, texture);
    glTexImage2D(target, 0, GL_RGBA8, size.width(), size.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              target, texture, 0);

    QT_CHECK_GLERROR();
    valid = checkFramebufferStatus();

#ifdef DEPTH_BUFFER
    // depth buffer
    glGenRenderbuffersEXT(1, &depth_buffer);
    Q_ASSERT(!glIsRenderbufferEXT(depth_buffer));
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_buffer);
    Q_ASSERT(glIsRenderbufferEXT(depth_buffer));
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, size.width(), size.height());
    int i = 0;
    glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &i);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                 GL_RENDERBUFFER_EXT, depth_buffer);

#endif
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    checkFramebufferStatus();
    QT_CHECK_GLERROR();
}



/*!
    \class QGLFramebufferObject
    \brief The QGLFramebufferObject class encapsulates an OpenGL framebuffer object.
    \since 4.2

    \ingroup multimedia

    The QGLFramebufferObject provides a rendering surface that can be
    painted on with a QPainter, native GL calls, or a mix of the
    two. This surface can be bound and used as a regular texture in
    your own GL drawing code.  By default, the QGLFramebufferObject
    class generates a 2D GL texture (using the GL_TEXTURE_2D target),
    which is used as the rendering surface.

    It is important to have a GL context set as the current context
    when creating a QGLFramebufferObject.

    OpenGL framebuffer objects are similar in use to pbuffers
    (\l{QGLPixelBuffer}{QGLPixelBuffer}), but there are a number of
    advantages with using framebuffer objects instead of
    pbuffers:

    \list 1
    \o A framebuffer object does not require a separate rendering
    context, so no context switching will occur when switching
    rendering targets. There is an overhead involved in switching
    rendering targets, but in general it is cheaper than a context
    switch to a pbuffer.

    \o Another major benefit is that rendering to dynamic textures
    works on all platforms. No need to do explicit copy calls from a
    buffer into a texture, as was necessary under X11 if you wanted
    dynamically updated textures.

    \o The OpenGL framebuffer extension is a pure GL extension with no
    system dependant WGL, AGL or GLX parts. This makes using
    framebuffer objects more portable.
    \endlist

    \sa {opengl/framebufferobject}{Framebuffer Object example}
*/


/*! \fn QGLFramebufferObject::QGLFramebufferObject(const QSize &size, GLenum target)

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the size \a size. The texture is bound to the
    GL_COLOR_ATTACHMENT0 target in the framebuffer object.

    The \a target parameter is used to specify the GL texture
    target. The default target is GL_TEXTURE_2D. Keep in mind that
    GL_TEXTURE_2D textures must have a power of 2 width and height
    (i.e. 256x512).

    It is important that you have a current GL context set when
    creating the QGLFramebufferObject, otherwise the initalization
    might fail.

    \sa size(), texture()
*/
QGLFramebufferObject::QGLFramebufferObject(const QSize &sz, GLenum texture_target)
    : d_ptr(new QGLFramebufferObjectPrivate)
{
    Q_D(QGLFramebufferObject);
    d->init(sz, texture_target);
}


/*! \overload */
QGLFramebufferObject::QGLFramebufferObject(int width, int height, GLenum texture_target)
    : d_ptr(new QGLFramebufferObjectPrivate)
{
    Q_D(QGLFramebufferObject);
    d->init(QSize(width, height), texture_target);
}

/*!
    \fn QGLFramebufferObject::~QGLFramebufferObject()

    Destroys the framebuffer object and frees any allocated resources.
*/
QGLFramebufferObject::~QGLFramebufferObject()
{
    Q_D(QGLFramebufferObject);

    if (isValid()) {
	glDeleteTextures(1, &d->texture);
#ifdef DEPTH_BUFFER
	glDeleteRenderbuffersEXT(1, &d->depth_buffer);
#endif
	glDeleteFramebuffersEXT(1, &d->fbo);
    }
    delete d_ptr;
}

/*!
    \fn bool QGLFramebufferObject::isValid() const

    Returns true if the framebuffer object is valid.

    The framebuffer can become invalid if the init process fails, the
    user attaches an invalid buffer to the framebuffer object, or a
    non-power of 2 width/height is specified as the texture size if
    the texture target is GL_TEXTURE_2D.
*/
bool QGLFramebufferObject::isValid() const
{
    Q_D(const QGLFramebufferObject);
    return d->valid;
}

/*!
    \fn bool QGLFramebufferObject::bind()

    Switches rendering from the default, windowing system provided
    framebuffer to this framebuffer object.
    Returns true upon success, false otherwise.
*/
bool QGLFramebufferObject::bind()
{
    if (!isValid())
	return false;
    Q_D(QGLFramebufferObject);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d->fbo);
    return d->checkFramebufferStatus();
}

/*!
    \fn bool QGLFramebufferObject::release()

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.
*/
bool QGLFramebufferObject::release()
{
    if (!isValid())
	return false;
    Q_D(QGLFramebufferObject);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    return d->checkFramebufferStatus();
}

/*!
    \fn GLuint QGLFramebufferObject::texture() const

    Returns the texture id for this framebuffer object. This texture
    id can be bound as a normal texture in your own GL code.
*/
GLuint QGLFramebufferObject::texture() const
{
    Q_D(const QGLFramebufferObject);
    return d->texture;
}

/*!
    \fn QSize QGLFramebufferObject::size() const

    Returns the size of the texture attached to this framebuffer
    object.
*/
QSize QGLFramebufferObject::size() const
{
    Q_D(const QGLFramebufferObject);
    return d->size;
}

/*!
    \fn QImage QGLFramebufferObject::toImage() const

    Returns the contents of this framebuffer object as a QImage.
*/
QImage QGLFramebufferObject::toImage() const
{
    return QImage();
}

Q_GLOBAL_STATIC(QOpenGLPaintEngine, qt_buffer_paintengine)

/*! \reimp */
QPaintEngine *QGLFramebufferObject::paintEngine() const
{
    return qt_buffer_paintengine();
}

/*!
    \fn bool QGLFramebufferObject::hasOpenGLFramebufferObjects()

    Returns true if the OpenGL \c{GL_EXT_framebuffer_object} extension
    is present on this system; otherwise returns false.
*/
bool QGLFramebufferObject::hasOpenGLFramebufferObjects()
{
    return (//(QGLExtensions::glExtensions & QGLExtensions::FramebufferObject)
            //&&
        qt_resolve_framebufferobject_extensions());
}

extern int qt_defaultDpi();

/*! \reimp */
int QGLFramebufferObject::metric(PaintDeviceMetric metric) const
{
    Q_D(const QGLFramebufferObject);

    float dpmx = qt_defaultDpi()*100./2.54;
    float dpmy = qt_defaultDpi()*100./2.54;
    int w = d->size.width();
    int h = d->size.height();
    switch (metric) {
    case PdmWidth:
        return w;

    case PdmHeight:
        return h;

    case PdmWidthMM:
        return qRound(w * 1000 / dpmx);

    case PdmHeightMM:
        return qRound(h * 1000 / dpmy);

    case PdmNumColors:
        return 0;

    case PdmDepth:
        return 32;//d->depth;

    case PdmDpiX:
        return (int)(dpmx * 0.0254);

    case PdmDpiY:
        return (int)(dpmy * 0.0254);

    case PdmPhysicalDpiX:
        return (int)(dpmx * 0.0254);

    case PdmPhysicalDpiY:
        return (int)(dpmy * 0.0254);

    default:
        qWarning("QGLFramebufferObject::metric(), Unhandled metric type: %d.\n", metric);
        break;
    }
    return 0;
}

/*!
    \fn GLuint QGLFramebufferObject::handle() const

    Returns the framebuffer object handle for this framebuffer
    object. This handle can be used to attach new images or buffers to
    this framebuffer. The user is responsible for cleaningup and
    destroying up these objects.
*/
GLuint QGLFramebufferObject::handle() const
{
    Q_D(const QGLFramebufferObject);
    return d->fbo;
}

/*! \fn int QGLFramebufferObject::devType() const

    \reimp
*/
