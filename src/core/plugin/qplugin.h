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

#ifndef QPLUGIN_H
#define QPLUGIN_H

#include <qobject.h>
#include <qpointer.h>

#ifndef Q_EXTERN_C
#  ifdef __cplusplus
#    define Q_EXTERN_C extern "C"
#  else
#    define Q_EXTERN_C extern
#  endif
#endif

// NOTE: if you change pattern, you MUST change the pattern in
// qpluginloader.cpp as well.  changing the pattern will break all
// backwards compatibility as well (no old plugins will be loaded).
#define Q_PLUGIN_VERIFICATION_DATA \
  static const char *qt_plugin_verification_data = \
    "pattern=""QT_PLUGIN_VERIFICATION_DATA""\n" \
    "version="QT_VERSION_STR"\n" \
    "buildkey="QT_BUILD_KEY"\0";

#define Q_PLUGIN_INSTANCE(IMPLEMENTATION) \
        { \
            static QPointer<IMPLEMENTATION> _instance; \
            if (!_instance) \
                _instance = new IMPLEMENTATION; \
            return _instance; \
        }

#ifdef Q_WS_WIN
#  ifdef Q_CC_BOR
#    define Q_EXPORT_PLUGIN(PLUGIN) \
                Q_PLUGIN_VERIFICATION_DATA \
                Q_EXTERN_C __declspec(dllexport) \
                const char * __stdcall qt_plugin_query_verification_data() \
                { return qt_plugin_verification_data; } \
                Q_EXTERN_C __declspec(dllexport) QObject * __stdcall qt_plugin_instance() \
                Q_PLUGIN_INSTANCE(PLUGIN)
#  else
#    define Q_EXPORT_PLUGIN(PLUGIN) \
                Q_PLUGIN_VERIFICATION_DATA \
                Q_EXTERN_C __declspec(dllexport) \
                const char *qt_plugin_query_verification_data() \
                { return qt_plugin_verification_data; } \
                Q_EXTERN_C __declspec(dllexport) QObject *qt_plugin_instance() \
                Q_PLUGIN_INSTANCE(PLUGIN)
#  endif
#else
#  define Q_EXPORT_PLUGIN(PLUGIN) \
            Q_PLUGIN_VERIFICATION_DATA \
            Q_EXTERN_C \
            const char *qt_plugin_query_verification_data() \
            { return qt_plugin_verification_data; } \
            Q_EXTERN_C QObject *qt_plugin_instance() \
            Q_PLUGIN_INSTANCE(PLUGIN)
#endif


#endif // Q_PLUGIN_H

