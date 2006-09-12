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

#ifndef QEGL_H
#define QEGL_H

#include <EGL/egl.h>


class QImage;
class QRect;

class /*Q_OPENGL_EXPORT*/ QEGL
{
public:
    static NativeWindowType createNativeWindow(const QRect&);
    //static NativeWindowType toNativeWindow(QWidget *);
    static NativePixmapType createNativePixmap(QImage *);
    static void destroyNativePixmap(NativePixmapType);
    // static NativePixmapType toNativePixmap(QPixmap *);
};


#endif
