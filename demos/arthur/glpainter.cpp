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

#include <qgl.h>
#include <qpainter.h>
#include <qlayout.h>

#include "glpainter.h"

class GLWidget : public QGLWidget
{
public:
    GLWidget(QWidget *parent);

protected:
    void initializeGL();
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

private:
    DemoWidget *dw;
    int step;
    int rot;
    GLuint cubeList;
    GLuint cubeTextureId;
};

extern void drawPrimitives(DemoWidget *dw, QPainter *p, int count, double distance, int step);
extern void drawShadedCube(DemoWidget *dw, QPainter *p, int iterations, int spread, int step);

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
    dw = qt_cast<DemoWidget *>(parent);
    step = 0;
    rot = 0;
    Q_ASSERT(dw);
}

void GLWidget::initializeGL()
{
    // cubeList
    QImage tex;
    tex.load("cubelogo.png");

    tex = QGLWidget::convertToGLFormat(tex);  // flipped 32bit RGBA

    glGenTextures(1, &cubeTextureId);
    glBindTexture(GL_TEXTURE_2D, cubeTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());

    cubeList = glGenLists(1);
    glNewList(cubeList, GL_COMPILE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glColor4ub(255, 255, 255, 127);
    glBegin( GL_QUADS );
    {
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );

	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
    }
    glEnd();

    glEndList();
}

void GLWidget::timerEvent(QTimerEvent *)
{
    update();
    ++step;
    ++rot;
}

void GLWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setBrush(QBrush(QPoint(0,0), Qt::white,
 		      QPoint(width(), height()), Qt::black));
    p.drawRect(0, 0, width(), height());
    p.translate(width()/2, height()/2);
    p.rotate(++rot % 360);
    p.shear(dw->xfunc(rot*0.5), dw->yfunc(rot*0.5));
    p.translate(-width()/2, -height()/2);
    dw->fillBackground(&p);
    drawShadedCube(dw, &p, 2, 5, step);
    p.resetXForm();

    drawPrimitives(dw, &p, 150, 0.3, step);
    p.setFont(QFont("helvetica", 14, QFont::Bold, true));
    p.setPen(Qt::white);
    p.drawText(75, height() - 75/3, "Arthur & OpenGL - together in harmony");


    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(0, 0, 75, 75);
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.1, 1.1, -1.1, 1.1, 0.1, 10);
    glTranslatef(0, 0, -1.2);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(rot % 360, 1, 0, 0);
    glRotatef(rot % 360, 0, 1, 0);
    glRotatef(rot % 360, 0, 0, 1);
    glTranslatef(-0.5, -0.5, -0.5);

    glBindTexture(GL_TEXTURE_2D, cubeTextureId);
    glCallList(cubeList);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

GLPainter::GLPainter(QWidget *parent)
    : DemoWidget(parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setMargin(0);
    glwidget = new GLWidget(this);
    layout->addWidget(glwidget);
}

void GLPainter::startAnimation()
{
    animTimer.start(50, glwidget);
}

void GLPainter::stopAnimation()
{
    animTimer.stop();
}
