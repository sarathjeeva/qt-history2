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

/*  TRANSLATOR MsgEdit

  This is the right panel of the main window.
*/

#include "msgedit.h"

#include "trwindow.h"
#include "phraselv.h"
#include "simtexth.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3textedit.h>
#include <qpalette.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtextview.h>
#include <qwhatsthis.h>
#include <qvbox.h>
#include <qmainwindow.h>
#include <q3header.h>
#include <qregexp.h>
#include <qdockwindow.h>
#include <qscrollview.h>
#include <qfont.h>
#include <qaccel.h>
#include <private/qrichtext_p.h>

static const int MaxCandidates = 5;

class MED : public Q3TextEdit
{
public:
    MED( QWidget *parent, const char *name = 0 )
        : Q3TextEdit( parent, name ) {}

    int cursorX() const { return textCursor()->x(); }
    int cursorY() const { return textCursor()->paragraph()->rect().y() +
                                 textCursor()->y(); }
};


QString richMeta( const QString& text )
{
    return QString( "<small><font color=blue>(" ) + text +
           QString( ")</font></small>" );
}

QString richText( const QString& text )
{
    const char backTab[] = "\a\b\f\n\r\t";
    const char * const friendlyBackTab[] = {
        QT_TRANSLATE_NOOP( "MessageEditor", "bell" ),
        QT_TRANSLATE_NOOP( "MessageEditor", "backspace" ),
        QT_TRANSLATE_NOOP( "MessageEditor", "new page" ),
        QT_TRANSLATE_NOOP( "MessageEditor", "new line" ),
        QT_TRANSLATE_NOOP( "MessageEditor", "carriage return" ),
        QT_TRANSLATE_NOOP( "MessageEditor", "tab" )
    };
    QString rich;

    for ( int i = 0; i < (int) text.length(); i++ ) {
        int ch = text[i].unicode();

        if ( ch < 0x20 ) {
            const char *p = strchr( backTab, ch );
            if ( p == 0 )
                rich += richMeta( QString::number(ch, 16) );
            else
                rich += richMeta( MessageEditor::tr(friendlyBackTab[p - backTab]) );
        } else if ( ch == '<' ) {
            rich += QString( "&lt;" );
        } else if ( ch == '>' ) {
            rich += QString( "&gt;" );
        } else if ( ch == '&' ) {
            rich += QString( "&amp;" );
        } else if ( ch == ' ' ) {
            if ( i == 0 || i == text.length() - 1 || text[i - 1].isSpace() ||
                 text[i + 1].isSpace() ) {
                rich += richMeta( MessageEditor::tr("sp") );
            } else {
                rich += ' ';
            }
        } else {
            rich += QChar( ch );
        }
        if ( ch == '\n' )
            rich += QString( "<br>" );
    }
    return rich;
}

/*
   ShadowWidget class impl.

   Used to create a shadow like effect for a widget
*/
ShadowWidget::ShadowWidget(QWidget * parent)
    : QWidget(parent), sWidth( 10 ), wMargin( 3 ), childWgt( 0 )
{
}

ShadowWidget::ShadowWidget( QWidget * child, QWidget * parent)
    : QWidget( parent), sWidth( 10 ), wMargin( 3 ), childWgt( 0 )
{
    setWidget( child );
}

void ShadowWidget::setWidget( QWidget * child )
{
    childWgt = child;
    if ( childWgt && childWgt->parent() != this ) {
        childWgt->setParent(this);
        childWgt->move(0,0);
        childWgt->show();
    }
}

void ShadowWidget::resizeEvent( QResizeEvent * )
{
    if( childWgt ) {
        childWgt->move( wMargin, wMargin );
        childWgt->resize( width() - sWidth - wMargin, height() - sWidth -
                          wMargin );
    }
}

void ShadowWidget::paintEvent( QPaintEvent * e )
{
    QPainter p;
    int w = width() - sWidth;
    int h = height() - sWidth;


    if ( !((w > 0) && (h > 0)) )
        return;

    if ( p.begin( this ) ) {
        p.setPen( palette().color(QPalette::Shadow) );

        p.drawPoint( w + 5, 6 );
        p.drawLine( w + 3, 6, w + 5, 8 );
        p.drawLine( w + 1, 6, w + 5, 10 );
        int i;
        for( i=7; i < h; i += 2 )
            p.drawLine( w, i, w + 5, i + 5 );
        for( i = w - i + h; i > 6; i -= 2 )
            p.drawLine( i, h, i + 5, h + 5 );
        for( ; i > 0 ; i -= 2 )
            p.drawLine( 6, h + 6 - i, i + 5, h + 5 );

//        p.eraseRect( w, 0, sWidth, 45 ); // Cheap hack for the page curl..
        p.end();
    }
    QWidget::paintEvent( e );
}

/*
   EditorPage class impl.

   A frame that contains the source text, translated text and any
   source code comments and hints.
*/
EditorPage::EditorPage( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    setLineWidth( 1 );
    setFrameStyle( QFrame::Box | QFrame::Plain );

    // Use white explicitly as the background color for the editor page.
    QPalette p = palette();
    p.setColor( QPalette::Active, QPalette::Base, QColor( Qt::white ) );
    p.setColor( QPalette::Inactive, QPalette::Base, QColor( Qt::white ) );
    p.setColor( QPalette::Disabled, QPalette::Base, QColor( Qt::white ) );
    p.setColor( QPalette::Active, QPalette::Background,
                p.color( QPalette::Active, QPalette::Base ) );
    p.setColor( QPalette::Inactive, QPalette::Background,
                p.color( QPalette::Inactive, QPalette::Base ) );
    p.setColor( QPalette::Disabled, QPalette::Background,
                p.color( QPalette::Disabled, QPalette::Base ) );
    setPalette( p );

    srcTextLbl = new QLabel( tr("Source text"), this);
    transLbl   = new QLabel( tr("Translation"), this);

    QFont fnt = font();
    fnt.setBold( TRUE );
    srcTextLbl->setFont( fnt );
    transLbl->setFont( fnt );

    srcText = new QTextView( this);
    srcText->setFrameStyle( QFrame::NoFrame );
    srcText->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum ) );
    srcText->setResizePolicy( QScrollView::AutoOne );
    srcText->setHScrollBarMode( QScrollView::AlwaysOff );
    srcText->setVScrollBarMode( QScrollView::AlwaysOff );
    p = srcText->palette();
    p.setColor( QPalette::Disabled, QPalette::Base, p.color(QPalette::Active, QPalette::Base));
    srcText->setPalette( p );
    connect( srcText, SIGNAL(textChanged()), SLOT(handleSourceChanges()) );

    cmtText = new QTextView( this, "comment/context view" );
    cmtText->setFrameStyle( QFrame::NoFrame );
    cmtText->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum ) );
    cmtText->setResizePolicy( QScrollView::AutoOne );
    cmtText->setHScrollBarMode( QScrollView::AlwaysOff );
    cmtText->setVScrollBarMode( QScrollView::AlwaysOff );
    p = cmtText->palette();
    p.setColor( QPalette::Active, QPalette::Base, QColor( 236,245,255 ) );
    p.setColor( QPalette::Inactive, QPalette::Base, QColor( 236,245,255 ) );
    cmtText->setPalette( p );
    connect( cmtText, SIGNAL(textChanged()), SLOT(handleCommentChanges()) );

    translationMed = new MED( this, "translation editor" );
    translationMed->setFrameStyle( QFrame::NoFrame );
    translationMed->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                             QSizePolicy::MinimumExpanding ) );
    translationMed->setHScrollBarMode( QScrollView::AlwaysOff );
    translationMed->setVScrollBarMode( QScrollView::AlwaysOff );
    translationMed->setResizePolicy( QScrollView::AutoOne );
    translationMed->setWrapPolicy( QTextView::AtWhiteSpace );
    translationMed->setWordWrap( QTextView::WidgetWidth );
    translationMed->setTextFormat( Qt::PlainText );
    p = translationMed->palette();
    p.setColor( QPalette::Disabled, QPalette::Base, p.color(QPalette::Active, QPalette::Base));
    translationMed->setPalette( p );
    connect( translationMed, SIGNAL(textChanged()),
             SLOT(handleTranslationChanges()) );

    pageCurl = new PageCurl(this);

    // Focus
    setFocusPolicy( Qt::StrongFocus );
    transLbl->setFocusProxy( translationMed );
    srcTextLbl->setFocusProxy( translationMed );
    cmtText->setFocusProxy( translationMed );
    srcText->setFocusProxy( translationMed );
    setFocusProxy( translationMed );

    updateCommentField();
}

/*
   Don't show the comment field if there are no comments.
*/
void EditorPage::updateCommentField()
{
    if( cmtText->text().isEmpty() )
        cmtText->hide();
    else
        cmtText->show();

    layoutWidgets();
}

/*
   Handle the widget layout manually
*/
void EditorPage::layoutWidgets()
 {
    int margin = 6;
    int space  = 2;
    int w = width();

    pageCurl->move( width() - pageCurl->width(), 0 );

    QFontMetrics fm( srcTextLbl->font() );
    srcTextLbl->move( margin, margin );
    srcTextLbl->resize( fm.width( srcTextLbl->text() ), srcTextLbl->height() );

    srcText->move( margin, srcTextLbl->y() + srcTextLbl->height() + space );
    srcText->resize( w - margin*2, srcText->height() );

    cmtText->move( margin, srcText->y() + srcText->height() + space );
    cmtText->resize( w - margin*2, cmtText->height() );

    if( cmtText->isHidden() )
        transLbl->move( margin, srcText->y() + srcText->height() + space );
    else
        transLbl->move( margin, cmtText->y() + cmtText->height() + space );
    transLbl->resize( w - margin*2, transLbl->height() );

    translationMed->move( margin, transLbl->y() + transLbl->height() + space );
    translationMed->resize( w - margin*2, translationMed->height() );

    // Calculate the total height for the editor page - emit a signal
    // if the actual page size is larger/smaller
    int totHeight = margin + srcTextLbl->height() +
                    srcText->height() + space +
                    transLbl->height() + space +
                    translationMed->height() + space +
                    frameWidth()*lineWidth()*2 + space * 3;

    if( !cmtText->isHidden() )
        totHeight += cmtText->height() + space;

     if( height() != totHeight )
         emit pageHeightUpdated( totHeight );
}

void EditorPage::resizeEvent( QResizeEvent * )
{
    handleTranslationChanges();
    handleSourceChanges();
    handleCommentChanges();
    layoutWidgets();
}

void EditorPage::handleTranslationChanges()
{
    calculateFieldHeight( (QTextView *) translationMed );
}

void EditorPage::handleSourceChanges()
{
    calculateFieldHeight( srcText );
}

void EditorPage::handleCommentChanges()
{
    calculateFieldHeight( cmtText );
}

/*
   Check if the translation text field is big enough to show all text
   that has been entered. If it isn't, resize it.
*/
void EditorPage::calculateFieldHeight( QTextView * field )
{
    field->sync(); // make sure the text formatting is done!
    int contentsHeight = field->contentsHeight();

    if( contentsHeight != field->height() ) {
        int oldHeight = field->height();
        if( contentsHeight < 30 )
            contentsHeight = 30;
        field->resize( field->width(), contentsHeight );
        emit pageHeightUpdated( height() + (field->height() - oldHeight) );
    }
}

void EditorPage::fontChange( const QFont & )
{
    QFont fnt = font();

    fnt.setBold( TRUE );
    QFontMetrics fm( fnt );
    srcTextLbl->setFont( fnt );
    srcTextLbl->resize( fm.width( srcTextLbl->text() ), srcTextLbl->height() );
    transLbl->setFont( fnt );
    transLbl->resize( fm.width( transLbl->text() ), transLbl->height() );
    update();
}

/*
   MessageEditor class impl.

   Handle layout of dock windows and the editor page.
*/
MessageEditor::MessageEditor( MetaTranslator * t, QMainWindow *parent )
    : QWidget( parent),
      tor( t )
{
    doGuesses = TRUE;
    v = new QVBoxLayout( this );

    topDockWnd = new QDockWindow(parent, Qt::DockWindowAreaTop);
    topDockWnd->setAllowedAreas(Qt::AllDockWindowAreas);
    topDockWnd->setFeatures(QDockWindow::AllDockWindowFeatures);
    topDockWnd->setWindowTitle(tr("Source text"));

    srcTextList = new Q3ListView(topDockWnd);
    srcTextList->setShowSortIndicator( TRUE );
    srcTextList->setAllColumnsShowFocus( TRUE );
    srcTextList->setSorting( 0 );
    QFontMetrics fm( font() );
    srcTextList->addColumn( tr("Done"), fm.width( tr("Done") ) + 10 );
    srcTextList->addColumn( tr("Source text") );
    srcTextList->addColumn( tr("Translation"), 300 );
    srcTextList->setColumnAlignment( 0, Qt::AlignCenter );
    srcTextList->setColumnWidthMode( 1, Q3ListView::Manual );
    srcTextList->header()->setStretchEnabled( TRUE, 1 );
    srcTextList->setMinimumSize( QSize( 50, 50 ) );
    srcTextList->setHScrollBarMode( QScrollView::AlwaysOff );
    srcTextList->installEventFilter( this );
    topDockWnd->setWidget(srcTextList);

    sv = new QScrollView( this, "scroll view" );
    sv->setHScrollBarMode( QScrollView::AlwaysOff );
    sv->viewport()->setBackgroundRole(QPalette::Background);

    editorPage = new EditorPage( sv, "editor page" );
    connect( editorPage, SIGNAL(pageHeightUpdated(int)),
             SLOT(updatePageHeight(int)) );

    editorPage->translationMed->installEventFilter( this );

    sw = new ShadowWidget( editorPage, sv);
    sw->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                    QSizePolicy::Expanding) );
    sw->setMinimumSize( QSize( 100, 150 ) );
    sv->addChild( sw );

    bottomDockWnd = new QDockWindow(parent, Qt::DockWindowAreaBottom);
    bottomDockWnd->setAllowedAreas(Qt::AllDockWindowAreas);
    bottomDockWnd->setFeatures(QDockWindow::AllDockWindowFeatures);
    bottomDockWnd->setWindowTitle(tr("Phrases"));

    QWidget *w = new QWidget(bottomDockWnd);
    w->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                   QSizePolicy::Minimum ) );
    QHBoxLayout *hl = new QHBoxLayout( w, 6 );
    QVBoxLayout *vl = new QVBoxLayout( 6 );

    phraseLbl = new QLabel( tr("Phrases and guesses:"), w );
    phraseLv = new PhraseLV( w, "phrase list view" );
    phraseLv->setSorting( PhraseLVI::DefinitionText );
    phraseLv->installEventFilter( this );
    hl->addLayout( vl );
    vl->addWidget( phraseLbl );
    vl->addWidget( phraseLv );

    accel = new QAccel( this, "accel" );
    connect( accel, SIGNAL(activated(int)), this, SLOT(guessActivated(int)) );
    for ( int i = 0; i < 9; i++ )
        accel->insertItem( Qt::CTRL + (Qt::Key_1 + i), i + 1 );

    bottomDockWnd->setWidget(w);

    v->addWidget( sv );

    // Signals
    connect( editorPage->pageCurl, SIGNAL(nextPage()),
             SIGNAL(nextUnfinished()) );
    connect( editorPage->pageCurl, SIGNAL(prevPage()),
             SIGNAL(prevUnfinished()) );

    connect( editorPage->translationMed, SIGNAL(textChanged()),
             this, SLOT(emitTranslationChanged()) );
    connect( editorPage->translationMed, SIGNAL(textChanged()),
             this, SLOT(updateButtons()) );
    connect( editorPage->translationMed, SIGNAL(undoAvailable(bool)),
             this, SIGNAL(undoAvailable(bool)) );
    connect( editorPage->translationMed, SIGNAL(redoAvailable(bool)),
             this, SIGNAL(redoAvailable(bool)) );
    connect( editorPage->translationMed, SIGNAL(copyAvailable(bool)),
             this, SIGNAL(cutAvailable(bool)) );
    connect( editorPage->translationMed, SIGNAL(copyAvailable(bool)),
             this, SIGNAL(copyAvailable(bool)) );
    connect( qApp->clipboard(), SIGNAL(dataChanged()),
             this, SLOT(updateCanPaste()) );
    connect( phraseLv, SIGNAL(doubleClicked(Q3ListViewItem *)),
             this, SLOT(insertPhraseInTranslation(Q3ListViewItem *)) );
    connect( phraseLv, SIGNAL(returnPressed(Q3ListViewItem *)),
             this, SLOT(insertPhraseInTranslationAndLeave(Q3ListViewItem *)) );

    // What's this
    this->setWhatsThis(tr("This whole panel allows you to view and edit "
                              "the translation of some source text.") );
    editorPage->srcText->setWhatsThis(tr("This area shows the source text.") );
    editorPage->cmtText->setWhatsThis(tr("This area shows a comment that"
                        " may guide you, and the context in which the text"
                        " occurs.") );
    editorPage->translationMed->setWhatsThis(tr("This is where you can enter or modify"
                        " the translation of some source text.") );
}

void MessageEditor::toggleFinished()
{
    if ( itemFinished )
        itemFinished = FALSE;
    else
        itemFinished = TRUE;
    emit finished( itemFinished );
}

bool MessageEditor::eventFilter( QObject *o, QEvent *e )
{
    static uchar doFocusChange = FALSE;

    // Handle keypresses in the message editor - scroll the view if the current
    // line is hidden.
    if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
        QKeyEvent * ke = (QKeyEvent*)e;
        const int k = ke->key();

        if ( qt_cast<Q3TextEdit*>(o) ) {
            if ( e->type() == QEvent::KeyPress ) {
                // Hardcode the Tab key to do focus changes when pressed
                // inside the editor
                if ( doFocusChange ) {
                    if ( k == Qt::Key_BackTab ) {
                        emit focusSourceList();
                        doFocusChange = FALSE;
                        return TRUE;
                    } else if ( k == Qt::Key_Tab ) {
                        emit focusPhraseList();
                        doFocusChange = FALSE;
                        return TRUE;
                    }
                }
            } else if ( e->type() == QEvent::KeyRelease ) {
                MED * ed = (MED *) o;
                switch( k ) {
                case Qt::Key_Up:
                    if (ed->cursorY() < 10)
                        sv->verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepSub);
                    break;

                case Qt::Key_Down:
                    if (ed->cursorY() >= ed->height() - 20)
                        sv->verticalScrollBar()->triggerAction(QScrollBar::SliderSingleStepAdd);
                    break;

                case Qt::Key_PageUp:
                    if (ed->cursorY() < 10)
                        sv->verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepSub);
                    break;

                case Qt::Key_PageDown:
                    if (ed->cursorY() >= ed->height() - 50)
                        sv->verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepAdd);
                    break;
                default:
                    sv->ensureVisible( sw->margin() + ed->x() + ed->cursorX(),
                                       sw->margin() + ed->y() + ed->cursorY() );
                    break;
                }
            }
            doFocusChange = TRUE;
        } else if ( qt_cast<Q3ListView*>(o) ) {
            // handle the ESC key in the list views
            if ( e->type() == QEvent::KeyRelease && k == Qt::Key_Escape )
                editorPage->translationMed->setFocus();
        }
    }
    return QWidget::eventFilter( o, e );
}

void MessageEditor::updatePageHeight( int height )
{
    sw->resize( sw->width(), height + sw->margin() + sw->shadowWidth() );
}

void MessageEditor::resizeEvent( QResizeEvent * )
{
    sw->resize( sv->viewport()->width(), sw->height() );
}

Q3ListView * MessageEditor::sourceTextList() const
{
    return srcTextList;
}

Q3ListView * MessageEditor::phraseList() const
{
    return phraseLv;
}

void MessageEditor::showNothing()
{
    editorPage->srcText->setText( "" );
    showContext( QString(""), FALSE );
}

void MessageEditor::showContext( const QString& context, bool finished )
{
    setEditionEnabled( FALSE );
    sourceText = QString::null;
    guesses.clear();

    if( context.isEmpty() )
        editorPage->cmtText->setText("");
    else
        editorPage->cmtText->setText( richText(context.simplified()) );
    setTranslation( QString(""), FALSE );
    setFinished( finished );
    phraseLv->clear();
    editorPage->handleSourceChanges();
    editorPage->handleCommentChanges();
    editorPage->handleTranslationChanges();
    editorPage->updateCommentField();
}

void MessageEditor::showMessage( const QString& text,
                                 const QString& comment,
                                 const QString& fullContext,
                                 const QString& translation,
                                 MetaTranslatorMessage::Type type,
                                 const QList<Phrase>& phrases )
{
    bool obsolete = ( type == MetaTranslatorMessage::Obsolete );
    setEditionEnabled( !obsolete );
    sourceText = text;
    guesses.clear();

    editorPage->srcText->setText( QString("<p>") + richText( text ) +
                                  QString("</p>") );

    if ( !fullContext.isEmpty() && !comment.isEmpty() )
        editorPage->cmtText->setText( richText(fullContext.simplified()) +
                                      "\n" + richText(comment.simplified()) );
    else if ( !fullContext.isEmpty() && comment.isEmpty() )
        editorPage->cmtText->setText(richText(fullContext.simplified() ) );
    else if ( fullContext.isEmpty() && !comment.isEmpty() )
        editorPage->cmtText->setText( richText(comment.simplified() ) );
    else
        editorPage->cmtText->setText( "" );

    setTranslation( translation, FALSE );
    setFinished( type != MetaTranslatorMessage::Unfinished );
    QList<Phrase>::ConstIterator p;
    phraseLv->clear();
    for ( p = phrases.begin(); p != phrases.end(); ++p )
         (void) new PhraseLVI( phraseLv, *p );

    if ( doGuesses && !sourceText.isEmpty() ) {
        CandidateList cl = similarTextHeuristicCandidates( tor,
                                                           sourceText.latin1(),
                                                           MaxCandidates );
        int n = 0;
        QList<Candidate>::Iterator it = cl.begin();
        while ( it != cl.end() ) {
            QString def;
            if ( n < 9 )
                def = tr( "Guess (%1)" ).arg( QString(QKeySequence(Qt::CTRL | (Qt::Key_0 + (n + 1)))) );
            else
                def = tr( "Guess" );
            (void) new PhraseLVI( phraseLv,
                                  Phrase((*it).source, (*it).target, def),
                                  n + 1 );
            n++;
            ++it;
        }
    }
    editorPage->handleSourceChanges();
    editorPage->handleCommentChanges();
    editorPage->handleTranslationChanges();
    editorPage->updateCommentField();
}

void MessageEditor::setTranslation( const QString& translation, bool emitt )
{
    // Block signals so that 'textChanged()' is not emitted when
    // for example a new source text item is selected and *not*
    // the actual translation.
    editorPage->translationMed->blockSignals( !emitt );
    editorPage->translationMed->setText( translation );
    editorPage->translationMed->blockSignals( FALSE );
    if ( !emitt )
         updateButtons();
    emit cutAvailable( FALSE );
    emit copyAvailable( FALSE );
}

void MessageEditor::setEditionEnabled( bool enabled )
{
    editorPage->transLbl->setEnabled( enabled );
    editorPage->translationMed->setReadOnly( !enabled );

    phraseLbl->setEnabled( enabled );
    phraseLv->setEnabled( enabled );
    updateCanPaste();
}

void MessageEditor::undo()
{
    editorPage->translationMed->undo();
}

void MessageEditor::redo()
{
    editorPage->translationMed->redo();
}

void MessageEditor::cut()
{
    editorPage->translationMed->cut();
}

void MessageEditor::copy()
{
    editorPage->translationMed->copy();
}

void MessageEditor::paste()
{
    editorPage->translationMed->paste();
}

void MessageEditor::del()
{
    editorPage->translationMed->del();
}

void MessageEditor::selectAll()
{
    editorPage->translationMed->selectAll();
}

void MessageEditor::emitTranslationChanged()
{
    emit translationChanged( editorPage->translationMed->text() );
}

void MessageEditor::guessActivated( int accelKey )
{
    Q3ListViewItem *item = phraseLv->firstChild();
    while ( item != 0 && ((PhraseLVI *) item)->accelKey() != accelKey )
        item = item->nextSibling();
    if ( item != 0 )
        insertPhraseInTranslation( item );
}

void MessageEditor::insertPhraseInTranslation( Q3ListViewItem *item )
{
    editorPage->translationMed->insert(((PhraseLVI *) item)->phrase().target());
    emit translationChanged( editorPage->translationMed->text() );
}

void MessageEditor::insertPhraseInTranslationAndLeave( Q3ListViewItem *item )
{
    editorPage->translationMed->insert(((PhraseLVI *) item)->phrase().target());
    emit translationChanged( editorPage->translationMed->text() );
    editorPage->translationMed->setFocus();
}

void MessageEditor::updateButtons()
{
    bool overwrite = ( !editorPage->translationMed->isReadOnly() &&
             (editorPage->translationMed->text().trimmed().isEmpty() ||
              mayOverwriteTranslation) );
    mayOverwriteTranslation = FALSE;
    emit updateActions( overwrite );
}

void MessageEditor::beginFromSource()
{
    mayOverwriteTranslation = TRUE;
    setTranslation( sourceText, TRUE );
    if ( !editorPage->hasFocus() )
        editorPage->setFocus();
}

void MessageEditor::finishAndNext()
{
    setFinished( TRUE );
    emit nextUnfinished();
    if ( !editorPage->hasFocus() )
        editorPage->setFocus();
}

void MessageEditor::updateCanPaste()
{
    bool oldCanPaste = canPaste;
    canPaste = ( !editorPage->translationMed->isReadOnly() &&
                 !qApp->clipboard()->text().isNull() );
    if ( canPaste != oldCanPaste )
        emit pasteAvailable( canPaste );
}

void MessageEditor::setFinished( bool finished )
{
    if ( !finished != !itemFinished )
        toggleFinished();
}

void MessageEditor::toggleGuessing()
{
    doGuesses = !doGuesses;
    if ( !doGuesses ) {
        phraseLv->clear();
    }
}
