/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"


CustomEdit::CustomEdit( QWidget *parent, const char *name ) : 
    QLineEdit( parent, name )
{
    connect( this, SIGNAL(textChanged(const QString &)), 
	     this, SLOT(slotChanged(const QString &)) );
}


void CustomEdit::slotChanged( const QString &line )
{
    setUpperLine( line );
}


QString CustomEdit::upperLine() const 
{
    return upper_line;
}


void CustomEdit::setUpperLine( const QString &line )
{
    upper_line = line.upper();
    blockSignals( true );
    setText( upper_line );
    blockSignals( false );
}


FormDialog::FormDialog()
{
    QLabel	*forenameLabel	= new QLabel( "Forename:", this );
    CustomEdit	*forenameEdit	= new CustomEdit( this );
    QLabel	*surnameLabel   = new QLabel( "Surname:", this );
    CustomEdit	*surnameEdit	= new CustomEdit( this );
    QLabel	*salaryLabel	= new QLabel( "Salary:", this );
    QLineEdit	*salaryEdit	= new QLineEdit( this );
    QPushButton *saveButton	= new QPushButton( "&Save", this );
    connect( saveButton, SIGNAL(clicked()), this, SLOT(slotSave()) );

    QGridLayout *grid = new QGridLayout( this );
    grid->addWidget( forenameLabel, 0, 0 );
    grid->addWidget( forenameEdit,  0, 1 );
    grid->addWidget( surnameLabel,  1, 0 );
    grid->addWidget( surnameEdit,   1, 1 );
    grid->addWidget( salaryLabel,   2, 0 );
    grid->addWidget( salaryEdit,    2, 1 );
    grid->addWidget( saveButton,    3, 0 );
    grid->activate();

    staffCursor = new QSqlCursor( "staff" );
    idIndex = staffCursor->index( "id" );
    staffCursor->select( idIndex );
    staffCursor->first();

    propMap = new QSqlPropertyMap;
    propMap->insert( forenameEdit->className(), "upperLine" );

    QSqlRecord *buffer = staffCursor->primeUpdate();
    sqlForm = new QSqlForm( this );
    sqlForm->installPropertyMap( propMap );
    sqlForm->insert( forenameEdit, buffer->field( "forename" ) );
    sqlForm->insert( surnameEdit, buffer->field( "surname" ) );
    sqlForm->insert( salaryEdit, buffer->field( "salary" ) );
    sqlForm->readFields();
}


FormDialog::~FormDialog()
{
    delete staffCursor;
    delete propMap;
    delete sqlForm;
}


void FormDialog::slotSave()
{
    sqlForm->writeFields();
    staffCursor->update();
    staffCursor->select( idIndex );
    staffCursor->first();
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( ! create_connections() ) 
	return 1;

    FormDialog *formDialog = new FormDialog();
    formDialog->show();
    app.setMainWidget( formDialog );

    return app.exec();
}


bool create_connections()
{

    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( "QPSQL6" );
    defaultDB->setDatabaseName( "testdb" );
    defaultDB->setUserName( "db" );
    defaultDB->setPassword( "db" );
    defaultDB->setHostName( "silverfish" );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open sales database: " + 
		  defaultDB->lastError().driverText());
	qWarning( defaultDB->lastError().databaseText() );
	return false;
    }

    QSqlDatabase *oracle = QSqlDatabase::addDatabase( "QPSQL6", "oracle" );
    oracle->setDatabaseName( "pingpong" );
    oracle->setUserName( "db" );
    oracle->setPassword( "db" );
    oracle->setHostName( "silverfish" );
    if ( ! oracle->open() ) {
	qWarning( "Failed to open orders database: " + 
		  oracle->lastError().driverText() );
	qWarning( oracle->lastError().databaseText() );
	return false;
    }

    return true;
}

