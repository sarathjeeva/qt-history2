/****************************************************************************
**
** Definition of the QAccessibleWidget class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QACCESSIBLEWIDGET_H
#define QACCESSIBLEWIDGET_H

#ifndef QT_H
#include "qaccessibleobject.h"
#endif // QT_H

#if defined(QT_ACCESSIBILITY_SUPPORT)

class QAccessibleWidgetPrivate;

class Q_EXPORT QAccessibleWidget : public QAccessibleObject
{
public:
    QAccessibleWidget(QWidget *o, Role r = Client, QString name = QString());

    int		childCount() const;
    int		indexOfChild(const QAccessibleInterface *child) const;
    int		relationTo(int child, const QAccessibleInterface *other, int otherChild) const;

    int		childAt(int x, int y) const;
    QRect	rect(int child) const;
    int		navigate(Relation rel, int entry, QAccessibleInterface **target) const;

    QString	text(Text t, int child) const;
    Role	role(int child) const;
    int		state(int child) const;

    int		defaultAction(int child) const;
    bool	doAction(int action, int child);

protected:
    ~QAccessibleWidget();
    QWidget *widget() const;
    QObject *parentObject() const;

    void	addControllingSignal(const QString &signal);
    void	setValue(const QString &value);
    void	setDescription(const QString &desc);
    void	setHelp(const QString &help);
    void	setAccelerator(const QString &accel);
    void	setDefaultAction(int defAction, const QString &name);

private:
    QAccessibleWidgetPrivate *d;
};

#endif //QT_ACCESSIBILITY_SUPPORT

#endif //QACCESSIBLEWIDGET_H
