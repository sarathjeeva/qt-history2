/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qwizard.cpp#17 $
**
** Implementation of something useful.
**
** Created : 979899
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#include "qwizard.h"

#include "qlayout.h"
#include "qpushbutton.h"
#include "qlabel.h"
#include "qwidgetstack.h"
#include "qapplication.h"
#include "qvector.h"
#include "qpainter.h"


/*! \class QWizard qwizard.h

  \brief The QWizard class provides a framework for easily writing wizards.

  A wizard is a dialog that consists of a sequential number of steps,
  each consisting of a single page.  QWizard provides a title for each
  page, and "Next", "Back", "Finish" and "Help" buttons, as
  appropriate.

*/


class QWizardPrivate
{
public:
    struct Page {
	Page( QWidget * widget, const QString & title ):
	    w( widget ), t( title ), back( 0 ),
	    backEnabled( TRUE ), nextEnabled( TRUE ), finishEnabled( FALSE ),
	    helpEnabled( TRUE ),
	    isLast( FALSE ), appropriate( TRUE )
	{}
	QWidget * w;
	QString t;
	QWidget * back;
	bool backEnabled;
	bool nextEnabled;
	bool finishEnabled;
	bool helpEnabled;
	bool isLast;
	bool appropriate;
    };

    QVBoxLayout * v;
    Page * current;
    QWidgetStack * ws;
    QVector<Page> pages;
    QLabel * title;
    QPushButton * backButton;
    QPushButton * nextButton;
    QPushButton * finishButton;
    QPushButton * helpButton;

    QFrame * hbar1, * hbar2;

    Page * page( const QWidget * w )
    {
	if ( !w )
	    return 0;
	int i = pages.size();
	while( --i >= 0 && pages[i] && pages[i]->w != w )
	    ;
	return i >= 0 ? pages[i] : 0;
    }

};


/*!  Constructs an empty wizard dialog. */

QWizard::QWizard( QWidget *parent, const char *name, bool modal,
		  WFlags f )
    : QDialog( parent, name, modal, f )
{
    d = new QWizardPrivate();
    d->current = 0; // not quite true, but...
    d->ws = new QWidgetStack( this );
    d->title = 0;
    d->backButton = new QPushButton( this, "back" );
    d->nextButton = new QPushButton( this, "next" );
    d->finishButton = new QPushButton( this, "finish" );
    d->helpButton = new QPushButton( this, "help" );

    d->ws->installEventFilter( this );

    d->v = 0;
    d->hbar1 = 0;
    d->hbar2 = 0;

    d->helpButton->setText( tr( "Help" ) );
    d->nextButton->setText( tr( "Next>>" ) );
    d->finishButton->setText( tr( "Finish" ) );
    d->backButton->setText( tr( "<<Back" ) );

    d->nextButton->setDefault( TRUE );

    connect( d->backButton, SIGNAL(clicked()),
	     this, SLOT(back()) );
    connect( d->nextButton, SIGNAL(clicked()),
	     this, SLOT(next()) );
    connect( d->finishButton, SIGNAL(clicked()),
	     this, SLOT(finish()) );
    connect( d->helpButton, SIGNAL(clicked()),
	     this, SLOT(help()) );
}


/*! Destroys the object and frees any allocated resources, including,
of course, all pages and controllers.
*/

QWizard::~QWizard()
{
    delete d;
}


/*!  \reimp  */

void QWizard::show()
{
    if ( d->current )
	showPage( d->current->w );
    else if ( count() > 0 )
	showPage( d->pages[0]->w );
    else
	showPage( 0 );

    QDialog::show();
}


/*! \reimp */

void QWizard::setFont( const QFont & font )
{
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
    QDialog::setFont( font );
}


/*!  Adds \a page to the end of the wizard, titled \a title.  If \a
controller is 0 (the default) there will be no wizard controller for
\a page, otherwise \a controller is the controller for \a page.
*/

void QWizard::addPage( QWidget * page, const QString & title )
{
    if ( !page )
	return;
    if ( d->page( page ) ) {
#if defined(CHECK_STATE)
	debug( "already added %s/%s to %s/%s",
	       page->className(), page->name(),
	       className(), name() );
#endif	
	return;
    }
    int i = d->pages.size();
    QWizardPrivate::Page * p = new QWizardPrivate::Page( page, title );
    p->backEnabled = ( i > 0 );

    d->ws->addWidget( page, i );
    d->pages.resize( i+1 );
    d->pages.insert( i, p );
}


/*!  Makes \a w be the displayed page. */

void QWizard::showPage( QWidget * w )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( p ) {
	setBackEnabled( p->back != 0 );
	setNextEnabled( TRUE );
	d->ws->raiseWidget( w );
	d->current = p;
    }

    layOut();
    updateButtons();
}


/*!  Returns the number of pages in the wizard. */

int QWizard::count() const
{
    return d->pages.count();
}


/*!

*/

void QWizard::back()
{
    if ( d->current && d->current->back )
	showPage( d->current->back );
}


/*!

*/

void QWizard::next()
{
    int i = 0;
    while( i < (int)d->pages.size() && d->pages[i] &&
	   d->current && d->pages[i]->w != d->current->w )
	i++;
    i++;
    while( i <= (int)d->pages.size()-1 && 
	   ( !d->pages[i] || !appropriate( d->pages[i]->w ) ) )
	i++;
    // if we fell of the end of the world, step back
    while ( i > 0 && (i >= (int)d->pages.size() || !d->pages[i] ) )
	i--;
    if ( d->pages.at( i ) ) {
	d->pages[i]->back = d->current ? d->current->w : 0;
	showPage( d->pages[i]->w );
    }
}


/*!

*/

void QWizard::finish()
{

}

/*!
  \fn void QWizard::helpClicked()
  This signal is emitted when the user clicks on the help button.
*/

/*!  This slot either makes the wizard help you, if it can.  The only
way it knows is to emit the helpClicked() signal, and perhaps the
QWizardPage::helpClicked() signal too, if a QWizardPage is currently
being displayed.
*/

void QWizard::help()
{
    QWidget * page = d->ws->visibleWidget();
    if ( !page )
	return;

#if 0
    if ( page->inherits( "QWizardPage" ) )
	emit ((QWizardPage *)page)->helpClicked();
#endif
    emit helpClicked();
}


void QWizard::setBackEnabled( bool enable )
{
    d->backButton->setEnabled( enable );
}


void QWizard::setNextEnabled( bool enable )
{
    d->nextButton->setEnabled( enable );
}


void QWizard::setHelpEnabled( bool enable )
{
    d->helpButton->setEnabled( enable );
}


/*!

*/

void QWizard::setFinish( QWidget * w, bool isLast )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( !p )
	return;

    p->isLast = isLast;
    updateButtons();
}


/*!

*/

void QWizard::setBackEnabled( QWidget * w, bool enable )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( !p )
	return;

    p->backEnabled = enable;
    updateButtons();
}


/*!

*/

void QWizard::setNextEnabled( QWidget * w, bool enable )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( !p )
	return;

    p->nextEnabled = enable;
    updateButtons();
}


/*!

*/

void QWizard::setFinishEnabled( QWidget * w, bool enable )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( !p )
	return;

    p->finishEnabled = enable;
    updateButtons();
}


/*!

*/

void QWizard::setHelpEnabled( QWidget * w, bool enable )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( !p )
	return;

    p->helpEnabled = enable;
    updateButtons();
}


/*!  This virtual function returns TRUE if \a w is appropriate for
display in the current context of the wizard, and FALSE if QWizard
should go on.

It is called when the Next button is clicked.

\warning The last page of a wizard will be displayed if nothing else wants
to, and the Next button was enabled when the user clicked.

The default implementation returns whatever was set using
setApproprate().  The ultimate default is TRUE.
*/

bool QWizard::appropriate( QWidget * w ) const
{
    QWizardPrivate::Page * p = d->page( w );
    return p ? p->appropriate : TRUE;
}


/*!

*/

void QWizard::setApproprate( QWidget * w, bool enable )
{
    QWizardPrivate::Page * p = d->page( w );
    if ( p )
	p->appropriate = enable;
}


void QWizard::updateButtons()
{
    if ( !d->current )
	return;

    d->backButton->setEnabled( d->current->backEnabled &&
			       d->current->back != 0 );
    d->nextButton->setEnabled( d->current->nextEnabled );
    d->finishButton->setEnabled( d->current->finishEnabled );
    d->helpButton->setEnabled( d->current->helpEnabled );

    if ( ( d->current->finishEnabled && !d->finishButton->isVisible() ) ||
	 ( d->current->backEnabled && !d->backButton->isVisible() ) ||
	 ( d->current->nextEnabled && !d->nextButton->isVisible() ) ||
	 ( d->current->helpEnabled && !d->helpButton->isVisible() ) )
	layOut();
}


/*!  Returns a pointer to the page currently being displayed by the
wizard.  The wizard does its best to make sure that this value is
never 0, but if you try hard enough it can be.
*/

QWidget * QWizard::currentPage() const
{
    return d->ws->visibleWidget();
}


/*!  Returns the title of \a page.
*/

QString QWizard::title( QWidget * page ) const
{
    QWizardPrivate::Page * p = d->page( page );
    return p ? p->t : QString::null;
}


/*!

*/

QPushButton * QWizard::backButton() const
{
    return d->backButton;
}


/*!

*/

QPushButton * QWizard::nextButton() const
{
    return d->nextButton;
}


/*!

*/

QPushButton * QWizard::finishButton() const
{
    return d->finishButton;
}


/*!

*/

QPushButton * QWizard::helpButton() const
{
    return d->helpButton;
}


/*!  This virtual function is responsible for adding the bottom
divider and buttons below it.

\a layout is the vertical layout of the entire wizard.
*/

void QWizard::layOutButtonRow( QHBoxLayout * layout )
{
    bool hasHelp = FALSE;
    bool hasEarlyFinish = FALSE;

    int i = d->pages.size() - 2;
    while ( !hasEarlyFinish && i >= 0 ) {
	if ( d->pages[i] && d->pages[i]->finishEnabled )
	    hasEarlyFinish = TRUE;
	i--;
    }
    i = 0;
    while ( !hasHelp && i < (int)d->pages.size() ) {
	if ( d->pages[i] && d->pages[i]->helpEnabled )
	    hasHelp = TRUE;
	i++;
    }

    QBoxLayout * h = new QBoxLayout( QBoxLayout::LeftToRight );
    layout->addLayout( h );
    h->addStretch( 42 );

    h->addWidget( d->backButton );

    h->addSpacing( 6 );

    if ( hasEarlyFinish ) {
	d->nextButton->show();
	d->finishButton->show();
	h->addWidget( d->nextButton );
	h->addSpacing( 12 );
	h->addWidget( d->finishButton );
    } else if ( d->current->finishEnabled ||
		d->current == d->pages[d->pages.size()-1] ) {
	d->nextButton->hide();
	d->finishButton->show();
	h->addWidget( d->finishButton );
    } else {
	d->nextButton->show();
	d->finishButton->hide();
	h->addWidget( d->nextButton );
    }

    if ( hasHelp ) {
	h->addSpacing( 12 );
	h->addWidget( d->helpButton );
    }
}


/*!  This virtual function is responsible for laying out the title row
and adding the vertical divider between the title and the wizard page.
\a layout is the vertical layout for the wizard, \a title is the title
for this page, and this function is called every time \a title
changes.
*/

void QWizard::layOutTitleRow( QHBoxLayout * layout, const QString & title )
{
    if ( !d->title )
	d->title = new QLabel( this );
    d->title->setText( title );
    layout->addWidget( d->title );
    d->title->repaint();
}


/*!

*/

void QWizard::layOut()
{
    delete d->v;
    d->v = new QVBoxLayout( this, 6, 0, "top-level layout" );

    QHBoxLayout * l;
    l = new QHBoxLayout( 6 );
    d->v->addLayout( l, 0 );
    layOutTitleRow( l, d->current ? d->current->t : QString::null );

    if ( ! d->hbar1 ) {
	d->hbar1 = new QFrame( this, "<hr>", 0, TRUE );
	d->hbar1->setFrameStyle( QFrame::Sunken + QFrame::HLine );
	d->hbar1->setFixedHeight( 12 );
    }

    d->v->addWidget( d->hbar1 );

    d->v->addWidget( d->ws, 10 );

    if ( ! d->hbar2 ) {
	d->hbar2 = new QFrame( this, "<hr>", 0, TRUE );
	d->hbar2->setFrameStyle( QFrame::Sunken + QFrame::HLine );
	d->hbar2->setFixedHeight( 12 );
    }
    d->v->addWidget( d->hbar2 );

    l = new QHBoxLayout( 6 );
    d->v->addLayout( l );
    layOutButtonRow( l );
    d->v->activate();
}


/*! \reimp */

bool QWizard::eventFilter( QObject * o, QEvent * e )
{
    if ( o == d->ws && e && e->type() == QEvent::ChildRemoved ) {
	QChildEvent * c = (QChildEvent*)e;
	if ( c->child() && c->child()->isWidgetType() )
	    removePage( (QWidget *)c->child() );
    }
    return QDialog::eventFilter( o, e );
}


/*!  Removes \a page from this wizard.  If \a page is currently being
  displayed, QWizard will display something else.
*/

void QWizard::removePage( QWidget * page )
{
    if ( !page )
	return;

    int i = d->pages.size();
    while( --i >= 0 && d->pages[i] && d->pages[i]->w != page )
	;
    QWizardPrivate::Page * p = d->pages[i];
    d->pages.remove( i );
    delete p;
    if ( i+1 == (int) d->pages.size() )
	d->pages.resize( i );
    d->ws->removeWidget( page );
}
