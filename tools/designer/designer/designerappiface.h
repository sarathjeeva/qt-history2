 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include "../interfaces/designerinterface.h"
#include "project.h"

class FormWindow;
class MainWindow;
class Project;
class OutputWindow;

class DesignerInterfaceImpl : public DesignerInterface
{
public:
    DesignerInterfaceImpl( MainWindow *mw );

    DesignerProject *currentProject() const;
    QList<DesignerProject> projectList() const;
    void showStatusMessage( const QString & ) const;
    DesignerDock *createDock() const;
    DesignerOutputDock *outputDock() const;


    QUnknownInterface *queryInterface( const QUuid &uuid );
    ulong addRef();
    ulong release();

private:
    ulong ref;
    MainWindow *mainWindow;

};

class DesignerProjectImpl: public DesignerProject
{
public:
    DesignerProjectImpl( Project *pr );

    DesignerFormWindow *currentForm() const;
    QList<DesignerFormWindow> formList() const;
    void addForm( DesignerFormWindow * );
    void removeForm( DesignerFormWindow * );
    QString fileName() const;
    void setFileName( const QString & );
    QString projectName() const;
    void setProjectName( const QString & );
    QString databaseFile() const;
    void setDatabaseFile( const QString & );
    void setupDatabases() const;
    QList<DesignerDatabase> databaseConnections() const;
    void addDatabase( DesignerDatabase * );
    void removeDatabase( DesignerDatabase * );
    void save() const;

private:
    Project *project;

};

class DesignerDatabaseImpl: public DesignerDatabase
{
public:
    DesignerDatabaseImpl( Project::DatabaseConnection *d );

    QString name() const;
    void setName( const QString & );
    QString driver() const;
    void setDriver( const QString & );
    QString database() const;
    void setDatabase( const QString & );
    QString userName() const;
    void setUserName( const QString & );
    QString password() const;
    void setPassword( const QString & );
    QString hostName() const;
    void setHostName( const QString & );
    QStringList tables() const;
    void setTables( const QStringList & );
    QMap<QString, QStringList> fields() const;
    void setFields( const QMap<QString, QStringList> & );
    void open() const;
    void close() const;
private:
    Project::DatabaseConnection *db;

};

class DesignerFormWindowImpl: public DesignerFormWindow
{
public:
    DesignerFormWindowImpl( FormWindow *fw );

    QString fileName() const;
    void setFileName( const QString & );
    void save() const;
    void insertWidget( QWidget * );
    QWidget *create( const char *className, QWidget *parent, const char *name );
    void removeWidget( QWidget * );
    QWidgetList widgets() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void adjustSize();
    void editConnections();
    void checkAccels();
    void layoutH();
    void layoutV();
    void layoutHSplit();
    void layoutVSplit();
    void layoutG();
    void layoutHContainer();
    void layoutVContainer();
    void layoutGContainer();
    void breakLayout();
    void selectWidget( QWidget * );
    void selectAll();
    void clearSelection();
    bool isWidgetSelected( QWidget * ) const;
    QWidgetList selectedWidgets() const;
    QWidget *currentWidget() const;
    void setCurrentWidget( QWidget * );
    QList<QAction> actionList() const;
    void addAction( QAction * );
    void removeAction( QAction * );
    void preview() const;
    void addConnection( QObject *sender, const char *signal, QObject *receiver, const char *slot );
    void setProperty( QObject *o, const char *property, const QVariant &value );
    QVariant property( QObject *o, const char *property ) const;
    void setPropertyChanged( QObject *o, const char *property, bool changed );
    bool isPropertyChanged( QObject *o, const char *property ) const;
    void setColumnFields( QObject *o, const QMap<QString, QString> & );

private:
    FormWindow *formWindow;

};

class DesignerDockImpl: public DesignerDock
{
public:
    DesignerDockImpl();

    QDockWindow *dockWindow() const;
};

class DesignerOutputDockImpl: public DesignerOutputDock
{
public:
    DesignerOutputDockImpl( OutputWindow *ow );

    QWidget *addView( const QString &pageName );
    void appendDebug( const QString & );
    void clearDebug();
    void appendError( const QString &, int );
    void clearError();

private:
    OutputWindow *outWin;

};

#endif
