/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwsproperty.h#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSPROPERTY_H
#define QWSPROPERTY_H

#include "qwscommand.h"

#include <qcstring.h>

/*********************************************************************
 *
 * Class: QWSSetPropertyCommand
 *
 *********************************************************************/

class QWSSetPropertyCommand : public QWSCommand
{
public:
    enum Mode {
	PropReplace,
	PropPrepend,
	PropAppend
    };
	
    QWSSetPropertyCommand( QWSServer *s, QWSClient *c );
    virtual ~QWSSetPropertyCommand();

    virtual void readData();
    virtual void execute();

private:
    int winId, property, mode;
    QByteArray data;

};

#endif
