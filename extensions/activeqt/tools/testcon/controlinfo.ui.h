/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/


void ControlInfo::setControl( QAxWidget *activex )
{
    listInfo->clear();

    const QMetaObject *mo = activex->metaObject();
    Q3ListViewItem *item = new Q3ListViewItem(listInfo, "Class Info", QString::number(mo->classInfoCount()));
    int i;
    for (i = mo->classInfoOffset(); i < mo->classInfoCount(); ++i) {
	const QMetaClassInfo info = mo->classInfo(i);
	(void)new Q3ListViewItem(item, info.name(), info.value());
    }
    item = new Q3ListViewItem(listInfo, "Signals", QString::number(mo->signalCount()));
    for (i = mo->signalOffset(); i < mo->signalCount(); ++i) {
	const QMetaMember signal = mo->signal(i);
	(void)new Q3ListViewItem(item, signal.signature());
    }
    item = new Q3ListViewItem(listInfo, "Slots", QString::number(mo->slotCount()));
    for (i = mo->slotOffset(); i < mo->slotCount(); ++i) {
	const QMetaMember slot = mo->slot(i);
	(void)new Q3ListViewItem(item, slot.signature());
    }
    item = new Q3ListViewItem(listInfo, "Properties", QString::number(mo->propertyCount()));    
    for (i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
	const QMetaProperty property = mo->property(i);
	(void)new Q3ListViewItem(item, property.name(), property.typeName());
    }
}
