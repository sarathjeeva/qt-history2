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

#ifndef QVFB_H
#define QVFB_H

#include <QMainWindow>
#include <QStringList>

QT_BEGIN_NAMESPACE

class QVFbAbstractView;
class QVFbRateDialog;
class QPopupMenu;
class QMenuData;
class QAction;
class Config;
class DeviceSkin;
class QVFb;
class QLabel;
class QMenu;
class QScrollArea;

class Zoomer : public QWidget {
    Q_OBJECT
public:
    Zoomer(QVFb* target);

private slots:
    void zoom(int);

private:
    QVFb *qvfb;
    QLabel *label;
};

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    enum DisplayType { QWS, X11 };

    QVFb( int display_id, int w, int h, int d, int r, const QString &skin, DisplayType displayType, QWidget *parent = 0, Qt::WindowFlags wflags = 0 );
    ~QVFb();

    void enableCursor( bool e );

    QSize sizeHint() const;

public slots:
     void popupMenu();

protected slots:
    void saveImage();
    void toggleAnimation();
    void toggleCursor();
    void changeRate();
    void setRate(int);
    void about();

    void configure();
    void skinConfigChosen(int i);
    void chooseSize(const QSize& sz);

    void setZoom1();
    void setZoom2();
    void setZoom3();
    void setZoom4();
    void setZoomHalf();
    void setZoom075();

    void setZoom();

public slots:
    void setZoom(double);

protected:
    template <typename T>
    void createMenu(T *menu);
    QMenu* createFileMenu();
    QMenu* createViewMenu();
    QMenu* createHelpMenu();

private:
    void findSkins(const QString &currentSkin);
    void init( int display_id, int w, int h, int d, int r, const QString& skin );
    DeviceSkin *skin;
    double skinscaleH,skinscaleV;
    QVFbAbstractView *view;
    QVFbAbstractView *secondaryView;
    QVFbRateDialog *rateDlg;
    QMenu *viewMenu;
    QAction *cursorAction;
    Config* config;
    QStringList skinnames;
    QStringList skinfiles;
    int currentSkinIndex;
    Zoomer *zoomer;
    QScrollArea* scroller;
    DisplayType displayType;

    int refreshRate;
private slots:
    void setGamma400(int n);
    void setR400(int n);
    void setG400(int n);
    void setB400(int n);
    void updateGammaLabels();
};

QT_END_NAMESPACE

#endif
