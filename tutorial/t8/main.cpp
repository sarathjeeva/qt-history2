/****************************************************************
**
** Qt tutorial 8
**
****************************************************************/

#include <qapp.h>
#include <qpushbt.h>
#include <qscrbar.h>
#include <qlcdnum.h>
#include <qfont.h>

#include "lcdrange.h"
#include "cannon.h"


class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent=0, const char *name=0 );
protected:
    void resizeEvent( QResizeEvent * );
private:
    QPushButton *quit;
    LCDRange    *angle;
    CannonField *cannonField;
};


MyWidget::MyWidget( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    setMinimumSize( 500, 355 );

    quit = new QPushButton( "Quit", this, "quit" );
    quit->setGeometry( 10, 10, 75, 30 );
    quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    angle  = new LCDRange( this, "angle" );
    angle->setRange( 5, 70 );
    angle->setGeometry( 10, quit->y() + quit->height() + 10, 75, 130 );

    cannonField = new CannonField( this, "cannonField" );
    cannonField->move( angle->x() + angle->width() + 10, angle->y() );
    cannonField->setBackgroundColor( QColor( 250, 250, 200) );

    connect( angle,SIGNAL(valueChanged(int)), cannonField,SLOT(setAngle(int)));
    connect( cannonField,SIGNAL(angleChanged(int)), angle,SLOT(setValue(int)));

    angle->setValue( 60 );
}

void MyWidget::resizeEvent( QResizeEvent * )
{
    cannonField->resize( width()  - cannonField->x() - 10,
			 height() - cannonField->y() - 10 );
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    MyWidget w;
    w.setGeometry( 100, 100, 500, 355 );
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
