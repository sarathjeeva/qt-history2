#include <qdatetime.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qpainter.h>

#include "canvas.h"

#include <stdlib.h>

FigureEditor::FigureEditor(
	QCanvas& c, QWidget* parent,
	const char* name, WFlags f) :
    QCanvasView(&c,parent,name,f)
{
}

void FigureEditor::contentsMousePressEvent(QMouseEvent* e)
{
    QCanvasItemList l=canvas()->collisions(e->pos());
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
	moving = *it;
	moving_start = e->pos();
	return;
    }
    moving = 0;
}

void FigureEditor::contentsMouseMoveEvent(QMouseEvent* e)
{
    if ( moving ) {
	moving->moveBy(e->pos().x() - moving_start.x(),
		       e->pos().y() - moving_start.y());
	moving_start = e->pos();

	// You can call for extra updates whenevever you want.
	canvas()->update();
    }
}



SpaceShip::SpaceShip()
{
    static QCanvasPixmapSequence spaceship("sprites/spaceship%02d.png");
    setSequence(&spaceship);
    setAnimated(TRUE);
}

const int spaceship_rtti = 1234;

int SpaceShip::rtti() const
{
    return spaceship_rtti;
}

void SpaceShip::initSpeed()
{
    const double speed = 4.0;
    double d = drand48();
    setVelocity( d*speed*2-speed, (1-d)*speed*2-speed );
}

void SpaceShip::advance(int stage)
{
    switch ( stage ) {
      case 0: {
	double vx = xVelocity();
	double vy = yVelocity();

	if ( vx == 0.0 && vy == 0.0 ) {
	    // stopped last turn
	    initSpeed();
	    vx = xVelocity();
	    vy = yVelocity();
	}

	double nx = x() + vx;
	double ny = y() + vy;

	if ( nx < 0 || nx >= canvas()->width() )
	    vx = -vy;
	if ( ny < 0 || ny >= canvas()->height() )
	    vy = -vy;

	for (int bounce=0; bounce<4; bounce++) {
	    QCanvasItemList l=collisions(FALSE);
	    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
		QCanvasItem *hit = *it;
		if ( hit->rtti()==spaceship_rtti && hit->collidesWith(this) ) {
		    switch ( bounce ) {
		      case 0:
			vx = -vx;
			break;
		      case 1:
			vy = -vy;
			vx = -vx;
			break;
		      case 2:
			vx = -vx;
			break;
		      case 3:
			// Stop for this turn
			vx = 0;
			vy = 0;
			break;
		    }
		    setVelocity(vx,vy);
		    break;
		}
	    }
	}

	setVelocity(vx,vy);
      } break;
      case 1:
	QCanvasItem::advance(stage);
        break;
    }
}


Main::Main() :
    canvas(500,500)
{
    editor = new FigureEditor(canvas,this);
    QMenuBar* menu = menuBar();

    QPopupMenu* file = new QPopupMenu;
    file->insertItem("Quit", qApp, SLOT(quit()), CTRL+Key_Q);
    menu->insertItem("&File", file);

    QPopupMenu* edit = new QPopupMenu;
    edit->insertItem("Add &Circle", this, SLOT(addCircle()), CTRL+Key_C);
    edit->insertItem("Add &Hexagon", this, SLOT(addHexagon()), CTRL+Key_H);
    edit->insertItem("Add &Rectangle", this, SLOT(addRectangle()), CTRL+Key_R);
    edit->insertItem("Add &Sprite", this, SLOT(addSprite()), CTRL+Key_S);
    menu->insertItem("&Edit", edit);

    QPopupMenu* options = new QPopupMenu;
    menu->insertItem("&Options",options);

    statusBar();

    setCentralWidget(editor);

    canvas.setAdvancePeriod(30);

    for (int test=0; test<10; test++) {
	addCircle();
	addHexagon();
	addRectangle();
	addSprite();
    }
}

void Main::addSprite()
{
    QCanvasItem* i = new SpaceShip;
    do {
	i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
    } while (!i->collisions(TRUE).isEmpty());
}

void Main::addCircle()
{
    QCanvasPolygonalItem* i = new QCanvasEllipse(50,50);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
}

void Main::addHexagon()
{
    QCanvasPolygon* i = new QCanvasPolygon;
    QPointArray pa(6);
    pa[0] = QPoint(20,0);
    pa[1] = QPoint(10,-17);
    pa[2] = QPoint(-10,-17);
    pa[3] = QPoint(-20,0);
    pa[4] = QPoint(-10,17);
    pa[5] = QPoint(10,17);
    i->setPoints(pa);
    i->setBrush( QColor(lrand48()%32*8,lrand48()%32*8,lrand48()%32*8) );
    i->move(lrand48()%canvas.width(),lrand48()%canvas.height());
}

void Main::addRectangle()
{
    QCanvasPolygonalItem *i = new QCanvasRectangle( lrand48()%canvas.width(),lrand48()%canvas.height(),
			    50,50);
    int z = lrand48()%256;
    //i->setBrush( QColor(lrand48()%32*8,z,lrand48()%32*8) );
    i->setBrush( QColor(z,z,z) );
    i->setZ(z);
}

int main(int argc, char** argv)
{
    QApplication app(argc,argv);

    Main m;
    //m.resize(500,500);
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}


