/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "helpdialog.h"
#include "helpwindow.h"
#include "topicchooser.h"
#include "docuparser.h"
#include "mainwindow.h"
#include "config.h"
#include "tabbedbrowser.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qdir.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qlist.h>
#include <qstack.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qurl.h>
#include <qvalidator.h>
#include <qmimefactory.h>

#include <stdlib.h>
#include <limits.h>

static QString stripAmpersand(const QString &str)
{
    QString s(str);
    s = s.replace('&', "");
    return s;
}

static bool verifyDirectory(const QString &str)
{
    QFileInfo dirInfo(str);
    if (!dirInfo.exists())
        return QDir().mkdir(str);
    if (!dirInfo.isDir()) {
        qWarning("'%s' exists but is not a directory", str.latin1());
        return false;
    }
    return true;
}

struct IndexKeyword {
    IndexKeyword(const QString &kw, const QString &l)
        : keyword(kw), link(l) {}
    IndexKeyword() : keyword(QString::null), link(QString::null) {}
    bool operator<(const IndexKeyword &ik) const {
        return keyword.toLower() < ik.keyword.toLower();
    }
    bool operator<=(const IndexKeyword &ik) const {
        return keyword.toLower() <= ik.keyword.toLower();
    }
    bool operator>(const IndexKeyword &ik) const {
        return keyword.toLower() > ik.keyword.toLower();
    }
    Q_DUMMY_COMPARISON_OPERATOR(IndexKeyword)
    QString keyword;
    QString link;
};

QDataStream &operator>>(QDataStream &s, IndexKeyword &ik)
{
    s >> ik.keyword;
    s >> ik.link;
    return s;
}

QDataStream &operator<<(QDataStream &s, const IndexKeyword &ik)
{
    s << ik.keyword;
    s << ik.link;
    return s;
}

QValidator::State SearchValidator::validate(QString &str, int &) const
{
    for (int i = 0; i < (int) str.length(); ++i) {
        QChar c = str[i];
        if (!c.isLetterOrNumber() && c != '\'' && c != '`'
            && c != '\"' && c != ' ' && c != '-' && c != '_'
            && c!= '*')
            return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

HelpNavigationListItem::HelpNavigationListItem(QListBox *ls, const QString &txt)
    : QListBoxText(ls, txt)
{
}

void HelpNavigationListItem::addLink(const QString &link)
{
    int hash = link.indexOf('#');
    if (hash == -1) {
        linkList << link;
        return;
    }

    QString preHash = link.left(hash);
    if (linkList.find(preHash, QString::CaseInsensitive).count() > 0)
        return;
    linkList << link;
}

HelpNavigationContentsItem::HelpNavigationContentsItem(Q3ListView *v, Q3ListViewItem *after)
    : Q3ListViewItem(v, after)
{
}

HelpNavigationContentsItem::HelpNavigationContentsItem(Q3ListViewItem *v, Q3ListViewItem *after)
    : Q3ListViewItem(v, after)
{
}

void HelpNavigationContentsItem::setLink(const QString &lnk)
{
    theLink = lnk;
}

QString HelpNavigationContentsItem::link() const
{
    return theLink;
}



HelpDialog::HelpDialog(QWidget *parent, MainWindow *h)
    : QWidget(parent), lwClosed(false), help(h)
{
    ui.setupUi(this);
}

void HelpDialog::initialize()
{
    connect(ui.tabWidget, SIGNAL(currentChanged(int)),
             this, SLOT(currentTabChanged(int)));
    connect(ui.listContents, SIGNAL(mouseButtonClicked(int, Q3ListViewItem*, const QPoint &, int)),
             this, SLOT(showTopic(int,Q3ListViewItem*, const QPoint &)));
    connect(ui.listContents, SIGNAL(currentChanged(Q3ListViewItem*)),
             this, SLOT(currentContentsChanged(Q3ListViewItem*)));
    connect(ui.listContents, SIGNAL(selectionChanged(Q3ListViewItem*)),
             this, SLOT(currentContentsChanged(Q3ListViewItem*)));
    connect(ui.listContents, SIGNAL(doubleClicked(Q3ListViewItem*)),
             this, SLOT(showTopic(Q3ListViewItem*)));
    connect(ui.listContents, SIGNAL(returnPressed(Q3ListViewItem*)),
             this, SLOT(showTopic(Q3ListViewItem*)));
    connect(ui.listContents, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(showItemMenu(Q3ListViewItem*, const QPoint&)));
    connect(ui.editIndex, SIGNAL(returnPressed()),
             this, SLOT(showTopic()));
    connect(ui.editIndex, SIGNAL(textChanged(const QString&)),
             this, SLOT(searchInIndex(const QString&)));

    connect(ui.listIndex, SIGNAL(selectionChanged(QListBoxItem*)),
             this, SLOT(currentIndexChanged(QListBoxItem*)));
    connect(ui.listIndex, SIGNAL(returnPressed(QListBoxItem*)),
             this, SLOT(showTopic()));
    connect(ui.listIndex, SIGNAL(mouseButtonClicked(int, QListBoxItem*, const QPoint &)),
             this, SLOT(showTopic(int, QListBoxItem *, const QPoint &)));
    connect(ui.listIndex, SIGNAL(currentChanged(QListBoxItem*)),
             this, SLOT(currentIndexChanged(QListBoxItem*)));
    connect(ui.listIndex, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint&)),
             this, SLOT(showItemMenu(QListBoxItem*, const QPoint&)));

    connect(ui.listBookmarks, SIGNAL(mouseButtonClicked(int, Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(showTopic(int, Q3ListViewItem*, const QPoint &)));
    connect(ui.listBookmarks, SIGNAL(returnPressed(Q3ListViewItem*)),
             this, SLOT(showTopic(Q3ListViewItem*)));
    connect(ui.listBookmarks, SIGNAL(selectionChanged(Q3ListViewItem*)),
             this, SLOT(currentBookmarkChanged(Q3ListViewItem*)));
    connect(ui.listBookmarks, SIGNAL(currentChanged(Q3ListViewItem*)),
             this, SLOT(currentBookmarkChanged(Q3ListViewItem*)));
    connect(ui.listBookmarks, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)),
             this, SLOT(showItemMenu(Q3ListViewItem*, const QPoint&)));
    connect(ui.resultBox, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint&)),
             this, SLOT(showItemMenu(QListBoxItem*, const QPoint&)));

    cacheFilesPath = QDir::homeDirPath() + "/.assistant/"; //### Find a better location for the dbs

    ui.editIndex->installEventFilter(this);
    ui.listBookmarks->header()->hide();
    ui.listBookmarks->header()->setStretchEnabled(true);
    ui.listContents->header()->hide();
    ui.listContents->header()->setStretchEnabled(true);
    ui.framePrepare->hide();
    connect(qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()));

    ui.termsEdit->setValidator(new SearchValidator(ui.termsEdit));

    actionOpenCurrentTab = new QAction(this);
    actionOpenCurrentTab->setText(tr("Open Link in Current Tab"));

    actionOpenLinkInNewWindow = new QAction(this);
    actionOpenLinkInNewWindow->setText(tr("Open Link in New Window"));

    actionOpenLinkInNewTab = new QAction(this);
    actionOpenLinkInNewTab->setText(tr("Open Link in New Tab"));

    itemPopup = new QPopupMenu(this);
    itemPopup->addAction(actionOpenCurrentTab);
    itemPopup->addAction(actionOpenLinkInNewWindow);
    itemPopup->addAction(actionOpenLinkInNewTab);

    contentList.clear();

    initDoneMsgShown = false;
    fullTextIndex = 0;
    indexDone = false;
    titleMapDone = false;
    contentsInserted = false;
    bookmarksInserted = false;
    setupTitleMap();
}


void HelpDialog::processEvents()
{
    qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
}


void HelpDialog::lastWinClosed()
{
    lwClosed = true;
}


void HelpDialog::removeOldCacheFiles()
{
    QString dir = cacheFilesPath; // ### remove the last '/' ?
    if (!verifyDirectory(cacheFilesPath)) {
        qWarning("Failed to created assistant directory");
        return;
    }
    QString pname = "." + Config::configuration()->profileName();

    QStringList fileList;
    fileList <<  "indexdb" << "indexdb.dict" << "indexdb.doc" << "contentdb";
    QStringList::iterator it = fileList.begin();
    for (; it != fileList.end(); ++it) {
        if (QFile::exists(cacheFilesPath + *it + pname)) {
            QFile f(cacheFilesPath + *it + pname);
            f.remove();
        }
    }
}

void HelpDialog::timerEvent(QTimerEvent *e)
{
    static int opacity = 255;
    help->setWindowOpacity((opacity-=4)/255.0);
    if (opacity<=0)
        qApp->quit();
}


void HelpDialog::loadIndexFile()
{
    if (indexDone)
        return;

    setCursor(waitCursor);
    indexDone = true;
    ui.labelPrepare->setText(tr("Prepare..."));
    ui.framePrepare->show();
    processEvents();

    QProgressBar *bar = ui.progressPrepare;
    bar->setTotalSteps(100);
    bar->setProgress(0);


    QList<IndexKeyword> lst;
    QFile indexFile(cacheFilesPath + "indexdb." +
                     Config::configuration()->profileName());
    if (!indexFile.open(IO_ReadOnly)) {
        buildKeywordDB();
        processEvents();
        if(lwClosed)
            return;
        if (!indexFile.open(IO_ReadOnly)) {
            QMessageBox::warning(help, tr("Qt Assistant"), tr("Failed to load keyword index file\n"
                                                              "Assistant will not work!"));
#if defined Q_WS_WIN || defined Q_WS_MACX
            startTimer(50);
#endif
            return;
        }
    }

    QDataStream ds(&indexFile);
    Q_UINT32 fileAges;
    ds >> fileAges;
    if (fileAges != getFileAges()) {
        indexFile.close();
        buildKeywordDB();
        if (!indexFile.open(IO_ReadOnly)) {
            QMessageBox::warning(help, tr("Qt Assistant"),
                tr("Cannot open the index file %1").arg(QFileInfo(indexFile).absFilePath()));
            return;
        }
        ds.setDevice(&indexFile);
        ds >> fileAges;
    }
    ds >> lst;
    indexFile.close();

    bar->setProgress(bar->totalSteps());
    processEvents();

    ui.listIndex->clear();
    HelpNavigationListItem *lastItem = 0;
    QString lastKeyword = QString::null;
    QList<IndexKeyword>::ConstIterator it = lst.begin();
    for (; it != lst.end(); ++it) {
        if (lastKeyword.toLower() != (*it).keyword.toLower())
            lastItem = new HelpNavigationListItem(ui.listIndex, (*it).keyword);
        lastItem->addLink((*it).link);
        lastKeyword = (*it).keyword;
    }
    ui.framePrepare->hide();
    showInitDoneMessage();
    setCursor(arrowCursor);
}

Q_UINT32 HelpDialog::getFileAges()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::const_iterator i = addDocuFiles.begin();

    Q_UINT32 fileAges = 0;
    for(; i != addDocuFiles.end(); ++i) {
        QFileInfo fi(*i);
        if (fi.exists())
            fileAges += fi.lastModified().toTime_t();
    }

    return fileAges;
}

void HelpDialog::buildKeywordDB()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::iterator i = addDocuFiles.begin();

    int steps = 0;
    for(; i != addDocuFiles.end(); i++)
        steps += QFileInfo(*i).size();

    ui.labelPrepare->setText(tr("Prepare..."));
    ui.progressPrepare->setTotalSteps(steps);
    ui.progressPrepare->setProgress(0);
    processEvents();

    QList<IndexKeyword> lst;
    Q_UINT32 fileAges = 0;
    for(i = addDocuFiles.begin(); i != addDocuFiles.end(); i++){
        QFile file(*i);
        if (!file.exists()) {
            QMessageBox::warning(this, tr("Warning"),
                tr("Documentation file %1 does not exist!\n"
                    "Skipping file.").arg(QFileInfo(file).absFilePath()));
            continue;
        }
        fileAges += QFileInfo(file).lastModified().toTime_t();
        DocuParser *handler = DocuParser::createParser(*i);
        bool ok = handler->parse(&file);
        file.close();
        if(!ok){
            QString msg = QString("In file %1:\n%2")
                          .arg(QFileInfo(file).absFilePath())
                          .arg(handler->errorProtocol());
            QMessageBox::critical(this, tr("Parse Error"), tr(msg));
            delete handler;
            continue;
        }

        QList<IndexItem*> indLst = handler->getIndexItems();
        int counter = 0;
        foreach (IndexItem *indItem, indLst) {
            QFileInfo fi(indItem->reference);
            lst.append(IndexKeyword(indItem->keyword, fi.absFilePath()));
            if (ui.progressPrepare)
                ui.progressPrepare->setProgress(ui.progressPrepare->progress() +
                                              int(fi.absFilePath().length() * 1.6));

            if(++counter%100 == 0) {
                processEvents();
                if(lwClosed) {
                    return;
                }
            }
        }
        delete handler;
    }
    if (!lst.isEmpty())
        qHeapSort(lst);

    QFile indexout(cacheFilesPath + "indexdb." + Config::configuration()->profileName());
    if (verifyDirectory(cacheFilesPath) && indexout.open(IO_WriteOnly)) {
        QDataStream s(&indexout);
        s << fileAges;
        s << lst;
        indexout.close();
    }
}

void HelpDialog::setupTitleMap()
{
    if (titleMapDone)
        return;
    if (Config::configuration()->docRebuild()) {
        removeOldCacheFiles();
        Config::configuration()->setDocRebuild(false);
        Config::configuration()->saveProfile(Config::configuration()->profile());
    }
    if (contentList.isEmpty())
        getAllContents();

    titleMapDone = true;
    titleMap.clear();
    for(QHash<QString, ContentList>::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
        ContentList lst = it.value();
        foreach (ContentItem item, lst) {
            QFileInfo link(item.reference.simplified());
            titleMap[link.absFilePath()] = item.title.trimmed();
        }
    }
    processEvents();
}

void HelpDialog::getAllContents()
{
    QFile contentFile(cacheFilesPath + "contentdb." + Config::configuration()->profileName());
    contentList.clear();
    if (!contentFile.open(IO_ReadOnly)) {
        buildContentDict();
        return;
    }

    QDataStream ds(&contentFile);
    Q_UINT32 fileAges;
    ds >> fileAges;
    if (fileAges != getFileAges()) {
        contentFile.close();
        buildContentDict();
        return;
    }
    QString key;
    QList<ContentItem> lst;
    while (!ds.atEnd()) {
        ds >> key;
        ds >> lst;
        contentList.insert(key, QList<ContentItem>(lst));
    }
    contentFile.close();
    processEvents();

}

void HelpDialog::buildContentDict()
{
    QStringList docuFiles = Config::configuration()->docFiles();

    Q_UINT32 fileAges = 0;
    for(QStringList::iterator it = docuFiles.begin(); it != docuFiles.end(); it++) {
        QFile file(*it);
        if (!file.exists()) {
            QMessageBox::warning(this, tr("Warning"),
            tr("Documentation file %1 does not exist!\n"
                "Skipping file.").arg(QFileInfo(file).absFilePath()));
            continue;
        }
        fileAges += QFileInfo(file).lastModified().toTime_t();
        DocuParser *handler = DocuParser::createParser(*it);
        if(!handler) {
            QMessageBox::warning(this, tr("Warning"),
            tr("Documentation file %1 is not compatible!\n"
                "Skipping file.").arg(QFileInfo(file).absFilePath()));
            continue;
        }
        bool ok = handler->parse(&file);
        file.close();
        if(ok) {
            contentList.insert(*it, QList<ContentItem>(handler->getContentItems()));
            delete handler;
        } else {
            QString msg = QString("In file %1:\n%2")
                          .arg(QFileInfo(file).absFilePath())
                          .arg(handler->errorProtocol());
            QMessageBox::critical(this, tr("Parse Error"), tr(msg));
            continue;
        }
    }

    QFile contentOut(cacheFilesPath + "contentdb." + Config::configuration()->profileName());
    if (contentOut.open(IO_WriteOnly)) {
        QDataStream s(&contentOut);
        s << fileAges;
        for(QHash<QString, ContentList>::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
            s << it.key();
            s << it.value();
        }
        contentOut.close();
    }
}

void HelpDialog::currentTabChanged(int index)
{
    QString s = ui.tabWidget->tabText(index);
    if (stripAmpersand(s).contains(tr("Index")))
        QTimer::singleShot(0, this, SLOT(loadIndexFile()));
    else if (stripAmpersand(s).contains(tr("Bookmarks")))
        insertBookmarks();
    else if (stripAmpersand(s).contains(tr("Contents")))
        QTimer::singleShot(0, this, SLOT(insertContents()));
    else if (stripAmpersand(s).contains(tr("Search")))
        QTimer::singleShot(0, this, SLOT(setupFullTextIndex()));
}

void HelpDialog::showInitDoneMessage()
{
    if (initDoneMsgShown)
        return;
    initDoneMsgShown = true;
    help->statusBar()->message(tr("Done"), 3000);
}

void HelpDialog::currentIndexChanged(QListBoxItem *)
{
}


void HelpDialog::showTopic(int button, QListBoxItem *item,
                            const QPoint &)
{
    if(button == LeftButton && item)
        showTopic();
}

void HelpDialog::showTopic(int button, Q3ListViewItem *item,
                            const QPoint &)
{
    if(button == LeftButton && item)
        showTopic();
}

void HelpDialog::showTopic(Q3ListViewItem *item)
{
    if(item)
        showTopic();
}

void HelpDialog::showTopic()
{
    QString text = ui.tabWidget->tabText(ui.tabWidget->currentIndex());

    if (stripAmpersand(text).contains(tr("Index")))
        showIndexTopic();
    else if (stripAmpersand(text).contains(tr("Bookmarks")))
        showBookmarkTopic();
    else if (stripAmpersand(text).contains(tr("Contents")))
        showContentsTopic();
}

void HelpDialog::showIndexTopic()
{
    QListBoxItem *i = ui.listIndex->item(ui.listIndex->currentItem());
    if (!i)
        return;

    ui.editIndex->blockSignals(true);
    ui.editIndex->setText(i->text());
    ui.editIndex->blockSignals(false);

    HelpNavigationListItem *item = (HelpNavigationListItem*)i;

    QStringList links = item->links();
    if (links.count() == 1) {
        emit showLink(links.first());
    } else {
        qHeapSort(links);
        QStringList::Iterator it = links.begin();
        QStringList linkList;
        QStringList linkNames;
        for (; it != links.end(); ++it) {
            linkList << *it;
            linkNames << titleOfLink(*it);
        }
        QString link = TopicChooser::getLink(this, linkNames, linkList, i->text());
        if (!link.isEmpty())
            emit showLink(link);
    }
}

void HelpDialog::searchInIndex(const QString &s)
{
    QListBoxItem *i = ui.listIndex->firstItem();
    QString sl = s.toLower();
    while (i) {
        QString t = i->text();
        if (t.length() >= sl.length() &&
             i->text().left(s.length()).toLower() == sl) {
            ui.listIndex->setCurrentItem(i);
            ui.listIndex->setTopItem(ui.listIndex->index(i));
            break;
        }
        i = i->next();
    }
}

QString HelpDialog::titleOfLink(const QString &link)
{
    QString s(link);
    s.remove(s.indexOf('#'), s.length());
    s = titleMap[ s ];
    if (s.isEmpty())
        return link;
    return s;
}

bool HelpDialog::eventFilter(QObject * o, QEvent * e)
{
    if (!o || !e)
        return true;

    if (o == ui.editIndex && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Key_Up) {
            int i = ui.listIndex->currentItem();
            if (--i >= 0) {
                ui.listIndex->setCurrentItem(i);
                ui.editIndex->blockSignals(true);
                ui.editIndex->setText(ui.listIndex->currentText());
                ui.editIndex->blockSignals(false);
            }
            return true;
        } else if (ke->key() == Key_Down) {
            int i = ui.listIndex->currentItem();
            if (++i < int(ui.listIndex->count())) {
                ui.listIndex->setCurrentItem(i);
                ui.editIndex->blockSignals(true);
                ui.editIndex->setText(ui.listIndex->currentText());
                ui.editIndex->blockSignals(false);
            }
            return true;
        } else if (ke->key() == Key_Next || ke->key() == Key_Prior) {
            QApplication::sendEvent(ui.listIndex, e);
            ui.editIndex->blockSignals(true);
            ui.editIndex->setText(ui.listIndex->currentText());
            ui.editIndex->blockSignals(false);
        }
    }

    return QWidget::eventFilter(o, e);
}

void HelpDialog::addBookmark()
{
    if (!bookmarksInserted)
        insertBookmarks();
    QString link = QUrl(help->browsers()->currentBrowser()->context()).resolved(help->browsers()->currentBrowser()->source()).path();
    QString title = help->browsers()->currentBrowser()->documentTitle();
    if (title.isEmpty())
        title = titleOfLink(link);
    HelpNavigationContentsItem *i = new HelpNavigationContentsItem(ui.listBookmarks, 0);
    i->setText(0, title);
    i->setLink(link);
    saveBookmarks();
    help->updateBookmarkMenu();
}

void HelpDialog::on_buttonAdd_clicked()
{
    addBookmark();
}

void HelpDialog::on_buttonRemove_clicked()
{
    if (!ui.listBookmarks->currentItem())
        return;

    delete ui.listBookmarks->currentItem();
    saveBookmarks();
    if (ui.listBookmarks->firstChild()) {
        ui.listBookmarks->setSelected(ui.listBookmarks->firstChild(), true);
    }
    help->updateBookmarkMenu();
}

void HelpDialog::insertBookmarks()
{
    if (bookmarksInserted)
        return;
    bookmarksInserted = true;
    ui.listBookmarks->clear();
    QFile f(cacheFilesPath + "bookmarks." + Config::configuration()->profileName());
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream ts(&f);
    while (!ts.atEnd()) {
        HelpNavigationContentsItem *i = new HelpNavigationContentsItem(ui.listBookmarks, 0);
        i->setText(0, ts.readLine());
        i->setLink(ts.readLine());
    }
    help->updateBookmarkMenu();
    showInitDoneMessage();
}

void HelpDialog::currentBookmarkChanged(Q3ListViewItem *)
{
}

void HelpDialog::showBookmarkTopic()
{
    if (!ui.listBookmarks->currentItem())
        return;

    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)ui.listBookmarks->currentItem();
    QString absPath = "";
    if (QFileInfo(i->link()).isRelative())
        absPath = documentationPath + "/";
    emit showLink(absPath + i->link());
}

void HelpDialog::saveBookmarks()
{
    QFile f(cacheFilesPath + "bookmarks." + Config::configuration()->profileName());
    if (!f.open(IO_WriteOnly))
        return;
    QTextStream ts(&f);
    Q3ListViewItemIterator it(ui.listBookmarks);
    for (; it.current(); ++it) {
        HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)it.current();
        ts << i->text(0) << endl;
        ts << i->link() << endl;
    }
    f.close();
}

void HelpDialog::insertContents()
{
    if (contentsInserted)
        return;

    if (contentList.isEmpty())
        getAllContents();

    contentsInserted = true;
    ui.listContents->clear();
    setCursor(waitCursor);
    if (!titleMapDone)
        setupTitleMap();

    ui.listContents->setSorting(-1);

    for(QHash<QString, ContentList>::Iterator it = contentList.begin(); it != contentList.end(); ++it) {
        HelpNavigationContentsItem *newEntry;

        HelpNavigationContentsItem *contentEntry;
        QStack<HelpNavigationContentsItem*> stack;
        stack.clear();
        int depth = 0;
        bool root = false;

        HelpNavigationContentsItem *lastItem[64];
        for(int j = 0; j < 64; ++j)
            lastItem[j] = 0;

        ContentList lst = it.value();
        for (ContentList::ConstIterator it = lst.begin(); it != lst.end(); ++it) {
            ContentItem item = *it;
            if (item.depth == 0) {
                newEntry = new HelpNavigationContentsItem(ui.listContents, 0);
                newEntry->setPixmap(0, qPixmapFromMimeSource("book.png"));
                newEntry->setText(0, item.title);
                newEntry->setLink(item.reference);
                stack.push(newEntry);
                depth = 1;
                root = true;
            }
            else{
                if((item.depth > depth) && root) {
                    depth = item.depth;
                    stack.push(contentEntry);
                }
                if(item.depth == depth) {
                    contentEntry = new HelpNavigationContentsItem(stack.top(), lastItem[ depth ]);
                    lastItem[ depth ] = contentEntry;
                    contentEntry->setText(0, item.title);
                    contentEntry->setLink(item.reference);
                }
                else if(item.depth < depth) {
                    stack.pop();
                    depth--;
                    item = *(--it);
                }
            }
        }
        processEvents();
    }
    setCursor(arrowCursor);
    showInitDoneMessage();
}

void HelpDialog::currentContentsChanged(Q3ListViewItem *)
{
}

void HelpDialog::showContentsTopic()
{
    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)ui.listContents->currentItem();
    if (!i)
        return;
    emit showLink(i->link());
}

void HelpDialog::toggleContents()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 0) {
        ui.tabWidget->setCurrentPage(0);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleIndex()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 1 || !ui.editIndex->hasFocus()) {
        ui.tabWidget->setCurrentPage(1);
        parentWidget()->show();
        ui.editIndex->setFocus();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleBookmarks()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 2) {
        ui.tabWidget->setCurrentPage(2);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::toggleSearch()
{
    if (!isVisible() || ui.tabWidget->currentIndex() != 3) {
        ui.tabWidget->setCurrentPage(3);
        parentWidget()->show();
    }
    else
        parentWidget()->hide();
}

void HelpDialog::setupFullTextIndex()
{
    if (fullTextIndex)
        return;

    QMap<QString, QString>::ConstIterator it = titleMap.begin();
    QStringList documentList;
    for (; it != titleMap.end(); ++it)
        documentList << it.key();

    QString pname = Config::configuration()->profileName();
    fullTextIndex = new Index(documentList, QDir::homeDirPath()); // ### Is this correct ?
    if (!verifyDirectory(cacheFilesPath)) {
        QMessageBox::warning(help, tr("Qt Assistant"),
                             tr("Failed to save fulltext search index\n"
                                "Assistant will not work!"));
        return;
    }
    fullTextIndex->setDictionaryFile(cacheFilesPath + "indexdb.dict." + pname);
    fullTextIndex->setDocListFile(cacheFilesPath + "indexdb.doc." + pname);
    processEvents();

    connect(fullTextIndex, SIGNAL(indexingProgress(int)),
             this, SLOT(setIndexingProgress(int)));
    QFile f(cacheFilesPath + "indexdb.dict." + pname);
    if (!f.exists()) {
        help->statusBar()->clear();
        setCursor(waitCursor);
        ui.labelPrepare->setText(tr("Indexing files..."));
        ui.progressPrepare->setTotalSteps(100);
        ui.progressPrepare->reset();
        ui.progressPrepare->show();
        ui.framePrepare->show();
        processEvents();
        if (fullTextIndex->makeIndex() == -1)
            return;
        fullTextIndex->writeDict();
        ui.progressPrepare->setProgress(100);
        ui.framePrepare->hide();
        setCursor(arrowCursor);
        showInitDoneMessage();
    } else {
        setCursor(waitCursor);
        help->statusBar()->message(tr("Reading dictionary..."));
        processEvents();
        fullTextIndex->readDict();
        help->statusBar()->message(tr("Done"), 3000);
        setCursor(arrowCursor);
    }
}

void HelpDialog::setIndexingProgress(int prog)
{
    ui.progressPrepare->setProgress(prog);
    processEvents();
}

void HelpDialog::startSearch()
{
    QString str = ui.termsEdit->text();
    str = str.replace("\'", "\"");
    str = str.replace("`", "\"");
    QString buf = str;
    str = str.replace("-", " ");
    str = str.replace(QRegExp("\\s[\\S]?\\s"), " ");
    terms = str.split(QChar(' '));
    QStringList termSeq;
    QStringList seqWords;
    QStringList::iterator it = terms.begin();
    for (; it != terms.end(); ++it) {
        (*it) = (*it).simplified();
        (*it) = (*it).toLower();
        (*it) = (*it).replace("\"", "");
    }
    if (str.contains('\"')) {
        if ((str.count('\"'))%2 == 0) {
            int beg = 0;
            int end = 0;
            QString s;
            beg = str.indexOf('\"', beg);
            while (beg != -1) {
                beg++;
                end = str.indexOf('\"', beg);
                s = str.mid(beg, end - beg);
                s = s.toLower();
                s = s.simplified();
                if (s.contains('*')) {
                    QMessageBox::warning(this, tr("Full Text Search"),
                        tr("Using a wildcard within phrases is not allowed."));
                    return;
                }
                seqWords += s.split(QChar(' '));
                termSeq << s;
                beg = str.indexOf('\"', end + 1);
            }
        } else {
            QMessageBox::warning(this, tr("Full Text Search"),
                tr("The closing quotation mark is missing."));
            return;
        }
    }
    setCursor(waitCursor);
    foundDocs.clear();
    foundDocs = fullTextIndex->query(terms, termSeq, seqWords);
    QString msg(QString("%1 documents found.").arg(foundDocs.count()));
    help->statusBar()->message(tr(msg), 3000);
    ui.resultBox->clear();
    for (it = foundDocs.begin(); it != foundDocs.end(); ++it)
        ui.resultBox->insertItem(fullTextIndex->getDocumentTitle(*it));

    terms.clear();
    bool isPhrase = false;
    QString s = "";
    for (int i = 0; i < (int)buf.length(); ++i) {
        if (buf[i] == '\"') {
            isPhrase = !isPhrase;
            s = s.simplified();
            if (!s.isEmpty())
                terms << s;
            s = "";
        } else if (buf[i] == ' ' && !isPhrase) {
            s = s.simplified();
            if (!s.isEmpty())
                terms << s;
            s = "";
        } else
            s += buf[i];
    }
    if (!s.isEmpty())
        terms << s;

    setCursor(arrowCursor);
}

void HelpDialog::on_helpButton_clicked()
{
    emit showLink(Config::configuration()->assistantDocPath() + "/assistant-5.html");
}

void HelpDialog::on_resultBox_mouseButtonClicked(int button, QListBoxItem *i, const QPoint &)
{
    if(button == LeftButton) {
        showResultPage(i);
    }
}

void HelpDialog::on_resultBox_returnPressed(QListBoxItem *item)
{
    showResultPage(item);
}

void HelpDialog::showResultPage(QListBoxItem *item)
{
    if (item)
        emit showSearchLink(foundDocs[ui.resultBox->index(item)], terms);
}


void HelpDialog::showItemMenu(QListBoxItem *item, const QPoint &pos)
{
    if (!item)
        return;
    QString currentTabText = ui.tabWidget->tabText(ui.tabWidget->currentIndex());

    QAction *action = itemPopup->exec(pos);
    if (action == actionOpenCurrentTab) {
        if (stripAmpersand(currentTabText).contains(tr("Index")))
            showTopic();
        else {
            showResultPage(item);
        }
    } else if (action) {
        HelpWindow *hw = help->browsers()->currentBrowser();
        if (stripAmpersand(currentTabText).contains(tr("Index"))) {
            ui.editIndex->blockSignals(true);
            ui.editIndex->setText(item->text());
            ui.editIndex->blockSignals(false);

            HelpNavigationListItem *hi = (HelpNavigationListItem*)item;

            QStringList links = hi->links();
            if (links.count() == 1) {
                if (action == actionOpenLinkInNewWindow)
                    hw->openLinkInNewWindow(links.first());
                else
                    hw->openLinkInNewPage(links.first());
            } else {
                QStringList::Iterator it = links.begin();
                QStringList linkList;
                QStringList linkNames;
                for (; it != links.end(); ++it) {
                    linkList << *it;
                    linkNames << titleOfLink(*it);
                }
                QString link = TopicChooser::getLink(this, linkNames, linkList, item->text());
                if (!link.isEmpty()) {
                    if (action == actionOpenLinkInNewWindow)
                        hw->openLinkInNewWindow(link);
                    else
                        hw->openLinkInNewPage(link);
                }
            }
        } else {
            QString link = foundDocs[ ui.resultBox->index(item) ];
            if (action == actionOpenLinkInNewWindow)
                hw->openLinkInNewWindow(link);
            else
                hw->openLinkInNewPage(link);
        }
    }
}

void HelpDialog::showItemMenu(Q3ListViewItem *item, const QPoint &pos)
{
    if (!item)
        return;
    QAction *action = itemPopup->exec(pos);
    if (action == actionOpenCurrentTab) {
        if (stripAmpersand(ui.tabWidget->tabText(ui.tabWidget->currentIndex())).contains(tr("Contents")))
            showContentsTopic();
        else
            showBookmarkTopic();
    } else if (action) {
        HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)item;
        if (action == actionOpenLinkInNewWindow)
            help->browsers()->currentBrowser()->openLinkInNewWindow(i->link());
        else
            help->browsers()->currentBrowser()->openLinkInNewPage(i->link());
    }
}

void HelpDialog::on_termsEdit_returnPressed()
{
    startSearch();
}

void HelpDialog::on_searchButton_clicked()
{
    startSearch();
}
