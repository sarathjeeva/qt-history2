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

#ifndef QWSMANAGER_QWS_H
#define QWSMANAGER_QWS_H

#include "qpixmap.h"
#include "qobject.h"
#include "qdecoration_qws.h"
#include "qevent.h"

#ifndef QT_NO_QWS_MANAGER

class QAction;
class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSButton;
class QWSManagerPrivate;

class QWSManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWSManager)
public:
    QWSManager(QWidget *);
    ~QWSManager();

    static QDecoration *newDefaultDecoration();

    QRegion region();
    QWidget *widget();
    void maximize();

    static QWidget *grabbedMouse();

protected slots:
    void menuTriggered(QAction *item);
    void styleMenuTriggered(QAction *item);

protected:
    void handleMove();
    virtual QDecoration::Region pointInRegion(const QPoint &);

    virtual bool event(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *) {}
    virtual void paintEvent(QPaintEvent *);

    void menu(const QPoint &);
    void close();
    void minimize();
    void toggleMaximize();

private:
    void setMouseOver(QWSButton *, bool);
    void setClicked(QWSButton *, bool);
    void setOn(QWSButton *, bool);
    void repaintButton(QWSButton *);


};

class QWSButton
{
public:
    QWSButton(QWSManager *m, QDecoration::Region t, bool tb = false);

    enum State { MouseOver = 0x01, Clicked = 0x02, On = 0x04 };
    int state() { return flags; }
    QDecoration::Region type() { return typ; }
    bool setMouseOver(bool);
    bool setClicked(bool);
    bool setOn(bool);

private:
    int  flags;
    bool toggle;
    QDecoration::Region typ;
    QWSManager *manager;
};

#include "qdecorationdefault_qws.h"

#endif // QT_NO_QWS_MANAGER

#endif // QWSMANAGER_QWS_H
