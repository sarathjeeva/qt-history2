/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "menumanager.h"
#include "colors.h"
#include "menucontent.h"
#include "examplecontent.h"

MenuManager *MenuManager::pInstance = 0;

MenuManager * MenuManager::instance()
{
    if (!MenuManager::pInstance)
        MenuManager::pInstance = new MenuManager();
    return MenuManager::pInstance;
}

MenuManager::MenuManager()
{
    this->ticker = 0;
    this->assistant = new QAssistantClient(QLibraryInfo::location(QLibraryInfo::BinariesPath), this);
    this->score = new Score();
    this->currentMenu = "[no menu visible]";
    this->currentMenuButtons = "[no menu buttons visible]";
    this->currentInfo = "[no info visible]";
    this->currentMenuCode = -1;
    this->readXmlDocument();
}

MenuManager::~MenuManager()
{
    delete this->score;
    delete this->contentsDoc;
}

void MenuManager::readXmlDocument()
{
    this->contentsDoc = new QDomDocument();
    QString errorStr;
    int errorLine;
    int errorColumn;
    
    QFile file(":/xml/examples.xml");
    bool statusOK = this->contentsDoc->setContent(&file, true, &errorStr, &errorLine, &errorColumn);
    if (!statusOK){
        QMessageBox::critical(0,
                              QObject::tr("DOM Parser"),
                              QObject::tr("Could not read or find the contents document. Error at line %1, column %2:\n%3")
                              .arg(errorLine).arg(errorColumn).arg(errorStr)
                              );
        exit(-1);
    }
}

void MenuManager::itemSelected(int userCode, const QString &menuName)
{
    this->currentMenuCode = userCode;  
    switch (userCode){
    case LAUNCH:
        this->launchExample(this->currentInfo);
        break;
    case DOCUMENTATION:
        this->assistant->showPage(info[this->currentInfo]["docfile"]);
        break;
    case QUIT:
        this->window->loop = false;
        QCoreApplication::quit();
        break;
    case FULLSCREEN:
        this->window->toggleFullscreen();
        break;
    case ROOT:
        // out:
        this->score->queueMovie(this->currentMenu + " -out", Score::FROM_START, Score::LOCK_ITEMS);
        this->score->queueMovie(this->currentMenuButtons + " -out", Score::FROM_START, Score::LOCK_ITEMS);
        this->score->queueMovie(this->currentInfo + " -out");
        this->score->queueMovie(this->currentInfo + " -buttons -out", Score::NEW_ANIMATION_ONLY);
        // book-keeping:
        this->currentMenu = menuName + " -menu1";
        this->currentMenuButtons = menuName + " -buttons";
        this->currentInfo = menuName + " -info";
        // in:
        this->score->queueMovie(this->currentMenu, Score::FROM_START, Score::UNLOCK_ITEMS);
        this->score->queueMovie(this->currentMenuButtons, Score::FROM_START, Score::UNLOCK_ITEMS);
        this->score->queueMovie(this->currentInfo);
        if (!Colors::noTicker){
            this->ticker->useGuideQt();
            this->score->queueMovie("ticker", Score::NEW_ANIMATION_ONLY);
            this->window->switchTimerOnOff(true);
        }
        break;
    case MENU1:
        // out:
        this->score->queueMovie(this->currentMenu + " -out", Score::FROM_START, Score::LOCK_ITEMS);
        this->score->queueMovie(this->currentMenuButtons + " -out", Score::FROM_START, Score::LOCK_ITEMS);
        this->score->queueMovie(this->currentInfo + " -out");
        // book-keeping:
        this->currentMenu = menuName + " -menu1";
        this->currentMenuButtons = menuName + " -buttons";
        this->currentInfo = menuName + " -info";
        // in:
        this->score->queueMovie(this->currentMenu, Score::FROM_START, Score::UNLOCK_ITEMS);
        this->score->queueMovie(this->currentMenuButtons, Score::FROM_START, Score::UNLOCK_ITEMS);
        this->score->queueMovie(this->currentInfo);         
        if (!Colors::noTicker)
            this->ticker->useGuideTt();
        break;
    case MENU2:
        // out:
        this->score->queueMovie(this->currentInfo + " -out", Score::NEW_ANIMATION_ONLY);
        this->score->queueMovie(this->currentInfo + " -buttons -out", Score::NEW_ANIMATION_ONLY);
        // book-keeping:
        this->currentInfo = menuName;
        // in / shake:
        this->score->queueMovie(this->currentMenu + " -shake");
        this->score->queueMovie(this->currentInfo, Score::NEW_ANIMATION_ONLY);
        this->score->queueMovie(this->currentInfo + " -buttons", Score::NEW_ANIMATION_ONLY);
        if (!Colors::noTicker){
            this->score->queueMovie("ticker -out", Score::NEW_ANIMATION_ONLY);        
            this->window->switchTimerOnOff(false);
        }
        break;
    case MORE:
        this->score->queueMovie(this->currentMenu + " -out", Score::FROM_START, Score::LOCK_ITEMS);
        this->currentMenu = menuName;
        this->score->queueMovie(this->currentMenu, Score::FROM_START, Score::UNLOCK_ITEMS);
        break;
    }
    
    if (this->score->hasQueuedMovies()){
        this->score->playQue();
        // Playing new movies might include
        // loading etc. So ignore the FPS
        // at this point
        this->window->fpsHistory.clear();
    }
}

void MenuManager::launchExample(const QString &name)
{
    QString executable = this->info[name]["executable"];
    
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(exampleFinished()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(exampleError(QProcess::ProcessError)));
    
#ifdef Q_OS_WIN
    //make sure it finds the dlls on windows
    QString curpath = QString::fromLocal8Bit(qgetenv("PATH").constData());
    QString newpath = QString("PATH=%1;%2").arg(QLibraryInfo::location(QLibraryInfo::BinariesPath), curpath);
    process->setEnvironment(QStringList(newpath));
#endif
        
    if (info[name]["changedirectory"] != "false"){
        QDir dir(info[name]["executable"]);
        dir.cdUp();
        process->setWorkingDirectory(dir.absolutePath());
        if (Colors::verbose)
            qDebug() << "Setting working directory:" << dir.absolutePath();
    }

    if (Colors::verbose)
        qDebug() << "Launching:" << executable;    
    process->start(executable);
}

void MenuManager::exampleFinished()
{
}

void MenuManager::exampleError(QProcess::ProcessError error)
{
    if (error != QProcess::Crashed)
        QMessageBox::critical(0, tr("Failed to launch the example"),
                          tr("Could not launch the example. Ensure that it has been build."),
                          QMessageBox::Cancel);
}

void MenuManager::init(MainWindow *window)
{
    this->window = window;
    
    // Create ticker:
    this->createTicker();
    
    // Create first level menu:
    QDomElement rootElement = this->contentsDoc->documentElement();
    this->createRootMenu(rootElement);
    
    // Create second level menus:
    QDomNode level2MenuNode = rootElement.firstChild();
    while (!level2MenuNode.isNull()){
        QDomElement level2MenuElement = level2MenuNode.toElement();
        this->createSubMenu(level2MenuElement);
        
        // create leaf menu and example info:
        QDomNode exampleNode = level2MenuElement.firstChild();
        while (!exampleNode.isNull()){
            QDomElement exampleElement = exampleNode.toElement();
            this->readInfoAboutExample(exampleElement);
            this->createLeafMenu(exampleElement);
            exampleNode = exampleNode.nextSibling();
        }
        
        level2MenuNode = level2MenuNode.nextSibling();
    }
}

void MenuManager::readInfoAboutExample(const QDomElement &example)
{
    QString name = example.attribute("name");
    if (this->info.contains(name))
        qWarning() << "WARNING: MenuManager::readInfoAboutExample: Demo/example with name"
                    << name << "appears twize in the xml-file!";
        
    this->info[name]["filename"] = example.attribute("filename");
    this->info[name]["changedirectory"] = example.attribute("changedirectory");
    this->info[name]["executable"] = this->resolveExecutable(example);
    this->info[name]["docfile"] = this->resolveDocFile(example);
    this->info[name]["imgfile"] = this->resolveImgFile(example);
}

QString MenuManager::resolveExecutable(const QDomElement &example)
{
    QDomElement parent = example.parentNode().toElement();
    QDir dir;
    if (parent.tagName() == "demos")
        dir = QDir(QLibraryInfo::location(QLibraryInfo::DemosPath));
    else
        dir = QDir(QLibraryInfo::location(QLibraryInfo::ExamplesPath));
    
    QString fileName = example.attribute("filename");
    dir.cd(parent.attribute("dirname"));
    dir.cd(fileName);
    
    QFile unixFile(dir.path() + "/" + fileName);
    if (unixFile.exists()) return unixFile.fileName();
    QFile winR(dir.path() + "\\release\\" + fileName + ".exe");
    if (winR.exists()) return winR.fileName();
    QFile winD(dir.path() + "\\debug\\" + fileName + ".exe");
    if (winD.exists()) return winD.fileName();
    QFile mac(dir.path() + "/" + fileName + ".app");
    if (mac.exists()) return mac.fileName();
    
    return "QtDemo: Executable not found!";
}

QString MenuManager::resolveDocFile(const QDomElement &example)
{
    QDomElement parent = example.parentNode().toElement();
    QString docRootPath = QLibraryInfo::location(QLibraryInfo::DocumentationPath);
    if (parent.tagName() == "demos")
        return docRootPath + "/html/demos-" + example.attribute("filename") + ".html";
    else
        return docRootPath + "/html/" + parent.attribute("dirname") + "-" + example.attribute("filename") + ".html";
}

QString MenuManager::resolveImgFile(const QDomElement &example)
{
    // Scan the html document and look for an image:
    QDomDocument domDoc;
    QFile docFile(this->info[example.attribute("name")]["docfile"]);
    domDoc.setContent(&docFile);
    QDomNodeList images = domDoc.elementsByTagName("img");
    QDir docRootDir = QDir(QLibraryInfo::location(QLibraryInfo::DocumentationPath));
    docRootDir.cd("html");
    QStringList imageFiles;
    
    for (int i = 0; i< int(images.length()); ++i) {
        QDomElement imageElement = images.item(i).toElement();
        QString imagePath = imageElement.attribute("src");
        if (!imagePath.contains("-logo"))
            imageFiles.append(docRootDir.absoluteFilePath(imagePath));
    }
    
    if (imageFiles.size() > 0)
        return imageFiles[0];
    else
        return QLatin1String("No image found in document: ") + 
            this->info[example.attribute("name")]["docfile"];
}

void MenuManager::createRootMenu(const QDomElement &el)
{
    QString name = el.attribute("name");
    createMenu(el, MENU1);
    createInfo(new MenuContentItem(el, this->window->scene, 0), name + " -info");

    Movie *menuButtonsIn = this->score->insertMovie(name + " -buttons");
    Movie *menuButtonsOut = this->score->insertMovie(name + " -buttons -out");
    createLowLeftButton(QLatin1String("Quit"), QUIT, menuButtonsIn, menuButtonsOut, 0);
    createLowRightButton("Toggle fullscreen", FULLSCREEN, menuButtonsIn, menuButtonsOut, 0);
}

void MenuManager::createSubMenu(const QDomElement &el)
{
    QString name = el.attribute("name");
    createMenu(el, MENU2);
    createInfo(new MenuContentItem(el, this->window->scene, 0), name + " -info");

    Movie *menuButtonsIn = this->score->insertMovie(name + " -buttons");
    Movie *menuButtonsOut = this->score->insertMovie(name + " -buttons -out");
    createLowLeftButton(QLatin1String("Main menu"), ROOT, menuButtonsIn, menuButtonsOut, 0, QLatin1String("Qt Examples and Demos"));
}

void MenuManager::createLeafMenu(const QDomElement &el)
{    
    QString name = el.attribute("name");
    createInfo(new ExampleContent(name, this->window->scene, 0), name);
    
    Movie *infoButtonsIn = this->score->insertMovie(name + " -buttons");
    Movie *infoButtonsOut = this->score->insertMovie(name + " -buttons -out");
    createLowRightLeafButton("Documentation", 600, DOCUMENTATION, infoButtonsIn, infoButtonsOut, 0);    
    if (el.attribute("executable") != "false")
        createLowRightLeafButton("Launch", 405, LAUNCH, infoButtonsIn, infoButtonsOut, 0);    
}

void MenuManager::createMenu(const QDomElement &category, BUTTON_TYPE type)
{
    qreal sw = this->window->scene->sceneRect().width();
    int xOffset = 15;
    int yOffset = 10;
    int maxExamples = 20;
    int menuIndex = 1;
    QString name = category.attribute("name");
    QDomNode currentNode = category.firstChild();
        
    while (!currentNode.isNull()){
        QString moreString = QLatin1String(" -menu") + QString::number(menuIndex);
        Movie *movieIn = this->score->insertMovie(name + moreString);
        Movie *movieOut = this->score->insertMovie(name + moreString + " -out");
//        Movie *movieNext = this->score->insertMovie(name + moreString + " -next");
//        Movie *moviePrev = this->score->insertMovie(name + moreString + " -prev");
        Movie *movieShake = this->score->insertMovie(name + moreString + " -shake");

        int i = 0;
        while (!currentNode.isNull() && i <= maxExamples){
            TextButton *item;
            if (i == maxExamples){
                ++menuIndex;
                item = new TextButton("More...", TextButton::LEFT, MORE, this->window->scene, 0, TextButton::PANEL);
                item->setMenuString(name + QLatin1String(" -menu") + QString::number(menuIndex));
            } else {
                QString label = currentNode.toElement().attribute("name");
                item = new TextButton(label, TextButton::LEFT, type, this->window->scene, 0);
                currentNode = currentNode.nextSibling();
            }
                
            item->setRecursiveVisible(false);
            item->setZValue(10);
            qreal ih = item->sceneBoundingRect().height();
            qreal iw = item->sceneBoundingRect().width();
            qreal ihp = ih + 3;
            
            // create in-animation:
            DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
            anim->setDuration(float(1000 + (i * 20)) * Colors::animSpeedButtons);
            anim->setStartPos(QPointF(xOffset, -ih));
            anim->setPosAt(0.20, QPointF(xOffset, -ih));
            anim->setPosAt(0.50, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (10 * float(i / 4.0f))));
            anim->setPosAt(0.60, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            anim->setPosAt(0.70, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (5 * float(i / 4.0f))));
            anim->setPosAt(0.80, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            anim->setPosAt(0.90, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY + (2 * float(i / 4.0f))));
            anim->setPosAt(1.00, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            movieIn->append(anim);
            
            // create out-animation:
            anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
            anim->hideOnFinished = true;
            anim->setDuration((700 + (30 * i)) * Colors::animSpeedButtons);
            anim->setStartPos(QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            anim->setPosAt(0.60, QPointF(xOffset, 600 - ih - ih));
            anim->setPosAt(0.65, QPointF(xOffset + 20, 600 - ih));
            anim->setPosAt(1.00, QPointF(sw + iw, 600 - ih));
            movieOut->append(anim);
        
            // create shake-animation:
            anim = new DemoItemAnimation(item);
            anim->setDuration(700 * Colors::animSpeedButtons);
            anim->setStartPos(QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            anim->setPosAt(0.55, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY - i*2.0));
            anim->setPosAt(0.70, QPointF(xOffset - 10, (i * ihp) + yOffset + Colors::contentStartY - i*1.5));
            anim->setPosAt(0.80, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY - i*1.0));
            anim->setPosAt(0.90, QPointF(xOffset - 2, (i * ihp) + yOffset + Colors::contentStartY - i*0.5));
            anim->setPosAt(1.00, QPointF(xOffset, (i * ihp) + yOffset + Colors::contentStartY));
            movieShake->append(anim);

            i++;
        }
    }
}


void MenuManager::createLowLeftButton(const QString &label, BUTTON_TYPE type,
    Movie *movieIn, Movie *movieOut, Movie */*movieShake*/, const QString &menuString)
{
    TextButton *button = new TextButton(label, TextButton::RIGHT, type, this->window->scene, 0, TextButton::PANEL);
    if (!menuString.isNull())
        button->setMenuString(menuString);
    button->setRecursiveVisible(false);
    button->setZValue(10);

    qreal iw = button->sceneBoundingRect().width();
    int xOffset = 15;
    
    // create in-animation:
    DemoItemAnimation *buttonIn = new DemoItemAnimation(button, DemoItemAnimation::ANIM_IN);
    buttonIn->setDuration(1800 * Colors::animSpeedButtons);
    buttonIn->setStartPos(QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    buttonIn->setPosAt(0.5, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 35));
    buttonIn->setPosAt(0.7, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    buttonIn->setPosAt(1.0, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    movieIn->append(buttonIn);
    
    // create out-animation:
    DemoItemAnimation *buttonOut = new DemoItemAnimation(button, DemoItemAnimation::ANIM_OUT);
    buttonOut->hideOnFinished = true;
    buttonOut->setDuration(400 * Colors::animSpeedButtons);
    buttonOut->setStartPos(QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    buttonOut->setPosAt(1.0, QPointF(-iw, Colors::contentStartY + Colors::contentHeight - 26));
    movieOut->append(buttonOut);
}

void MenuManager::createLowRightButton(const QString &label, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie */*movieShake*/)
{
    TextButton *item = new TextButton(label, TextButton::RIGHT, type, this->window->scene, 0, TextButton::PANEL);
    item->setRecursiveVisible(false);
    item->setZValue(10);

    qreal sw = this->window->scene->sceneRect().width();
    int xOffset = 70;
    
    // create in-animation:
    DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    anim->setDuration(1800 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.5, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.7, QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(1.0, QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 26));
    movieIn->append(anim);
    
    // create out-animation:
    anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    anim->hideOnFinished = true;
    anim->setDuration(400 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(xOffset + 535, Colors::contentStartY + Colors::contentHeight - 26));
    anim->setPosAt(1.0, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 26));
    movieOut->append(anim);
}

void MenuManager::createLowRightLeafButton(const QString &label, int xOffset, BUTTON_TYPE type, Movie *movieIn, Movie *movieOut, Movie */*movieShake*/)
{
    TextButton *item = new TextButton(label, TextButton::RIGHT, type, this->window->scene, 0, TextButton::PANEL);
    item->setRecursiveVisible(false);
    item->setZValue(10);

    qreal sw = this->window->scene->sceneRect().width();
    qreal sh = this->window->scene->sceneRect().height();
    
    // create in-animation:
    DemoItemAnimation *anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    anim->setDuration(1050 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.10, QPointF(sw, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.30, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.35, QPointF(xOffset + 30, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.40, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.45, QPointF(xOffset + 5, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(0.50, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 35));
    anim->setPosAt(1.00, QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    movieIn->append(anim);
    
    // create out-animation:
    anim = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    anim->hideOnFinished = true;
    anim->setDuration(300 * Colors::animSpeedButtons);
    anim->setStartPos(QPointF(xOffset, Colors::contentStartY + Colors::contentHeight - 26));
    anim->setPosAt(1.0, QPointF(xOffset, sh));
    movieOut->append(anim);
}

void MenuManager::createInfo(DemoItem *item, const QString &name)
{
    Movie *movie_in = new Movie();
    Movie *movie_out = new Movie();
    this->score->insertMovie(name, movie_in);
    this->score->insertMovie(name + " -out", movie_out);
    
    item->setZValue(8);
    item->setRecursiveVisible(false);
    
    float xOffset = 230.0f;
    DemoItemAnimation *infoIn = new DemoItemAnimation(item, DemoItemAnimation::ANIM_IN);
    infoIn->timeline->setCurveShape(QTimeLine::LinearCurve);
    infoIn->setDuration(650);
    infoIn->setStartPos(QPointF(this->window->scene->sceneRect().width(), Colors::contentStartY));
    infoIn->setPosAt(0.60, QPointF(xOffset, Colors::contentStartY));
    infoIn->setPosAt(0.70, QPointF(xOffset + 20, Colors::contentStartY));
    infoIn->setPosAt(0.80, QPointF(xOffset, Colors::contentStartY));
    infoIn->setPosAt(0.90, QPointF(xOffset + 7, Colors::contentStartY));
    infoIn->setPosAt(1.00, QPointF(xOffset, Colors::contentStartY));
    movie_in->append(infoIn);
    
    DemoItemAnimation *infoOut = new DemoItemAnimation(item, DemoItemAnimation::ANIM_OUT);
    infoOut->timeline->setCurveShape(QTimeLine::EaseInCurve);
    infoOut->setDuration(300);
    infoOut->hideOnFinished = true;
    infoOut->setStartPos(QPointF(xOffset, Colors::contentStartY));
    infoOut->setPosAt(1.0, QPointF(-600, Colors::contentStartY));
    movie_out->append(infoOut);
}

void MenuManager::createTicker()
{
    if (!Colors::noTicker){
        Movie *movie_in = new Movie();
        Movie *movie_out = new Movie();
        Movie *movie_activate = new Movie();
        Movie *movie_deactivate = new Movie();
        this->score->insertMovie("ticker", movie_in);
        this->score->insertMovie("ticker -out", movie_out);
        this->score->insertMovie("ticker -activate", movie_activate);
        this->score->insertMovie("ticker -deactivate", movie_deactivate);
        
        this->ticker = new ItemCircleAnimation(this->window->scene, 0);
        this->ticker->setZValue(50);
        this->ticker->hide();
        
        // Move ticker in:
        int qtendpos = 485;
        int qtPosY = 120;
        DemoItemAnimation *qtIn = new DemoItemAnimation(this->ticker, DemoItemAnimation::ANIM_IN);
        qtIn->setDuration(500);
        qtIn->startDelay = 2000;
        qtIn->setStartPos(QPointF(this->window->scene->sceneRect().width(), Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.60, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.70, QPointF(qtendpos + 30, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.80, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(0.90, QPointF(qtendpos + 5, Colors::contentStartY + qtPosY));
        qtIn->setPosAt(1.00, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        movie_in->append(qtIn);
        
        // Move ticker out:
        DemoItemAnimation *qtOut = new DemoItemAnimation(this->ticker, DemoItemAnimation::ANIM_OUT);
        qtOut->hideOnFinished = true;
        qtOut->setDuration(500);
        qtOut->setStartPos(QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtOut->setPosAt(1.00, QPointF(this->window->scene->sceneRect().width() + 700, Colors::contentStartY + qtPosY));
        movie_out->append(qtOut);

        // Move ticker in on activate:
        DemoItemAnimation *qtActivate = new DemoItemAnimation(this->ticker);
        qtActivate->setDuration(400);
        qtActivate->setStartPos(QPointF(this->window->scene->sceneRect().width(), Colors::contentStartY + qtPosY));
        qtActivate->setPosAt(0.60, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtActivate->setPosAt(0.70, QPointF(qtendpos + 30, Colors::contentStartY + qtPosY));
        qtActivate->setPosAt(0.80, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtActivate->setPosAt(0.90, QPointF(qtendpos + 5, Colors::contentStartY + qtPosY));
        qtActivate->setPosAt(1.00, QPointF(qtendpos, Colors::contentStartY + qtPosY));
        movie_activate->append(qtActivate);

        // Move ticker out on deactivate:
        DemoItemAnimation *qtDeactivate = new DemoItemAnimation(this->ticker);
        qtDeactivate->hideOnFinished = true;
        qtDeactivate->setDuration(400);
        qtDeactivate->setStartPos(QPointF(qtendpos, Colors::contentStartY + qtPosY));
        qtDeactivate->setPosAt(1.00, QPointF(qtendpos, 800));
        movie_deactivate->append(qtDeactivate);
    }
}
