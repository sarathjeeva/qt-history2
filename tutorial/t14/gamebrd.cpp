/****************************************************************
**
** Implementation of GameBoard class, Qt tutorial 14
**
****************************************************************/

#include "gamebrd.h"

#include <qfont.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qaccel.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qlayout.h>

#include "lcdrange.h"
#include "cannon.h"

GameBoard::GameBoard( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 500, 355 );

    QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    LCDRange *angle  = new LCDRange( "ANGLE", this, "angle" );
    angle->setRange( 5, 70 );

    LCDRange *force  = new LCDRange( "FORCE", this, "force" );
    force->setRange( 10, 50 );

    QFrame *frame = new QFrame( this, "cannonFrame" );
    frame->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );

    cannonField = new CannonField( frame, "cannonField" );
    cannonField->setBackgroundColor( QColor( 250, 250, 200) );
    //### to use frameWidth is a hack, sort of
    QHBoxLayout *frameLayout = new QHBoxLayout( frame, frame->frameWidth() );
    frameLayout->addWidget( cannonField );

    connect( angle,SIGNAL(valueChanged(int)), cannonField,SLOT(setAngle(int)));
    connect( cannonField,SIGNAL(angleChanged(int)), angle,SLOT(setValue(int)));

    connect( force,SIGNAL(valueChanged(int)), cannonField,SLOT(setForce(int)));
    connect( cannonField,SIGNAL(forceChanged(int)), force,SLOT(setValue(int)));

    connect( cannonField, SIGNAL(hit()),SLOT(hit()) );
    connect( cannonField, SIGNAL(missed()),SLOT(missed()) );

    angle->setValue( 60 );
    force->setValue( 25 );

    QPushButton *shoot = new QPushButton( "Shoot", this, "shoot" );
    shoot->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( shoot, SIGNAL(clicked()), SLOT(fire()) );

    QPushButton *restart = new QPushButton( "New Game", this, "newgame" );
    restart->setFont( QFont( "Times", 18, QFont::Bold ) );

    connect( restart, SIGNAL(clicked()), SLOT(newGame()) );

    hits  	       = new QLCDNumber( 2, this, "hits" );
    shotsLeft 	       = new QLCDNumber( 2, this, "shotsleft" );
    QLabel *hitsL      = new QLabel( "HITS", this, "hitsLabel" );
    QLabel *shotsLeftL = new QLabel( "SHOTS LEFT", this, "shotsleftLabel" );

    QAccel *accel = new QAccel( this );
    accel->connectItem( accel->insertItem( Key_Space), this, SLOT(fire()) );
    accel->connectItem( accel->insertItem( Key_Q), qApp, SLOT(quit()) );

    QGridLayout *grid = new QGridLayout( this, 2, 2, 10 );
    grid->addWidget( quit, 0, 0 );
    grid->addWidget( frame, 1, 1 );
    grid->setColStretch( 1, 10 );

    QVBoxLayout *leftBox = new QVBoxLayout;
    grid->addLayout( leftBox, 1, 0 );
    leftBox->addWidget( angle );
    leftBox->addWidget( force );

    QHBoxLayout *topBox = new QHBoxLayout;
    grid->addLayout( topBox, 0, 1 );
    topBox->addWidget( shoot );
    topBox->addWidget( hits );
    topBox->addWidget( hitsL );
    topBox->addWidget( shotsLeft );
    topBox->addWidget( shotsLeftL );
    topBox->addStretch( 1 );
    topBox->addWidget( restart );


    newGame();
}

void GameBoard::fire()
{
    if ( cannonField->gameOver() || cannonField->isShooting() )
	return;
    shotsLeft->display( shotsLeft->intValue() - 1 );
    cannonField->shoot();
}

void GameBoard::hit()
{
    hits->display( hits->intValue() + 1 );
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
    else
	cannonField->newTarget();
}

void GameBoard::missed()
{
    if ( shotsLeft->intValue() == 0 )
	cannonField->setGameOver();
}

void GameBoard::newGame()
{
    shotsLeft->display( 15 );
    hits->display( 0 );
    cannonField->restartGame();
    cannonField->newTarget();
}
