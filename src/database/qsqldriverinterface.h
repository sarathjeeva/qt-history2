#ifndef QSQLDRIVERINTERFACE_H
#define QSQLDRIVERINTERFACE_H

#ifndef QT_H
#include "qplugininterface.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlDriverInterface : public QPlugInInterface
{
public:
    QCString queryPlugInInterface() const { return "QSqlDriverInterface"; }

    virtual QSqlDriver* create( const QString& name ) = 0;
};

#endif // QT_NO_SQL

#endif // QSQLDRIVERINTERFACE_H
