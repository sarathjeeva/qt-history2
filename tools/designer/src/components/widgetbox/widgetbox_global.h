#ifndef WIDGETBOX_GLOBAL_H
#define WIDGETBOX_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_WIDGETBOX_LIBRARY
# define QT_WIDGETBOX_EXPORT
#else
# define QT_WIDGETBOX_EXPORT
#endif
#else
#define QT_WIDGETBOX_EXPORT
#endif

#endif // WIDGETBOX_GLOBAL_H

