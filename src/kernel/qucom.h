#ifndef QUTYPES_H
#define QUTYPES_H
#include <ucom.h>
#include <qstring.h>

// {B1D3BE80-2F2F-44F7-AB11-E8A0CEC84B82}
extern const UUid TID_UType_QString;

struct UType_QString : public UType
{
    const UUid *uuid() const;
    const char *desc() const;

    void set( UObject *, const QString & );
    QString &get( UObject * o ) { return *(QString*)o->payload.ptr; }

    bool convertFrom( UObject *, UType * );
    bool convertTo( UObject *, UType * );

    void clear( UObject * );
};
extern UType_QString * pUType_QString;



#endif //QUTYPES_H

