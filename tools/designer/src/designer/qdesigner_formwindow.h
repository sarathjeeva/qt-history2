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

#ifndef QDESIGNER_FORMWINDOW_H
#define QDESIGNER_FORMWINDOW_H

#include <QtCore/QPointer>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QDesignerWorkbench;
class QDesignerFormWindowInterface;

class QDesignerFormWindow: public QWidget
{
    Q_OBJECT
public:
    QDesignerFormWindow(QDesignerFormWindowInterface *formWindow, QDesignerWorkbench *workbench,
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void firstShow();

    virtual ~QDesignerFormWindow();

    QAction *action() const;
    QDesignerWorkbench *workbench() const;
    QDesignerFormWindowInterface *editor() const;

    QRect geometryHint() const;

public slots:
    void updateChanged();

private slots:
    void updateWindowTitle(const QString &fileName);
    void geometryChanged();
    void widgetActivated(QWidget *widget);

signals:
    void minimizationStateChanged(QDesignerFormWindowInterface *formWindow, bool minimized);
    void triggerAction();

protected:
    virtual void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent *ev);
    virtual void resizeEvent(QResizeEvent* rev);

private:
    int getNumberOfUntitledWindows() const;
    QPointer<QDesignerFormWindowInterface> m_editor;
    QPointer<QDesignerWorkbench> m_workbench;
    QAction *m_action;
    bool m_initialized;
    bool m_windowTitleInitialized;
};

QT_END_NAMESPACE

#endif // QDESIGNER_FORMWINDOW_H
