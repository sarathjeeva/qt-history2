#ifndef FORMEDITOR_GLOBAL_H
#define FORMEDITOR_GLOBAL_H

#include <qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_FORMEDITOR_LIBRARY
# define QT_FORMEDITOR_EXPORT
#else
# define QT_FORMEDITOR_EXPORT
#endif
#else
#define QT_FORMEDITOR_EXPORT
#endif

#endif // FORMEDITOR_GLOBAL_H

