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

#include <qmessagebox.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qevent.h>

// Standard Qt widgets

#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <q3whatsthis.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qprogressbar.h>
#include <qsplitter.h>
#include <q3listview.h>
#include <q3header.h>
#include <qtextbrowser.h>
#include <qfiledialog.h>
#include <qaccel.h>
#include <qmetaobject.h>
#include <qpainter.h>
#include <qcstring.h>

#include "widgets.h"


// Some sample widgets

#include "../aclock/aclock.h"
#include "../dclock/dclock.h"


#define MOVIEFILENAME "trolltech.gif"

#include "../application/fileopen.xpm"
#include "../application/filesave.xpm"
#include "../application/fileprint.xpm"


using namespace Qt;

class MyWhatsThis : public Q3WhatsThis
{
public:
    MyWhatsThis( QListBox* lb)
	: Q3WhatsThis( lb ) { listbox = lb; };
    ~MyWhatsThis(){};


    QString text( const QPoint & p) {
	QListBoxItem* i = listbox->itemAt( p );
	if ( i && i->pixmap() ) {
	    return "Isn't that a <em>wonderful</em> pixmap? <br>" \
		"Imagine, you could even decorate a" \
		" <b>red</b> pushbutton with it! :-)";
	}
	return "This is a QListBox.";
    }

private:
    QListBox* listbox;
};

//
// Construct the WidgetView with children
//

WidgetView::WidgetView( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    QColor col;

    // Set the window caption/title
    setWindowTitle( "Qt Example - Widgets Demo Application" );

    // create a toolbar
    QToolBar *tools = new QToolBar( this, "toolbar" );

    // put something in it
    QPixmap openIcon( fileopen );
    QToolButton * toolb = new QToolButton( openIcon, "toolbutton 1",
					   QString::null, this, SLOT(open()),
					   tools, "open file" );
    QWhatsThis::add( toolb, "This is a <b>QToolButton</b>. It lives in a "
		     "QToolBar. This particular button doesn't do anything "
		     "useful." );
    tools->addWidget(toolb);

    QPixmap saveIcon( filesave );
    toolb = new QToolButton( saveIcon, "toolbutton 2", QString::null,
			     this, SLOT(dummy()),
			     tools, "save file" );
    QWhatsThis::add( toolb, "This is also a <b>QToolButton</b>." );
    tools->addWidget(toolb);

    QPixmap  printIcon( fileprint );
    toolb = new QToolButton( printIcon, "toolbutton 3", QString::null,
			     this, SLOT(dummy()),
			     tools, "print file" );
    QWhatsThis::add( toolb, "This is the third <b>QToolButton</b>.");
    tools->addWidget(toolb);

    toolb = QWhatsThis::whatsThisButton( tools );
    QWhatsThis::add( toolb, "This is a <b>What's This</b> button "
		     "It enables the user to ask for help "
		     "about widgets on the screen.");
    tools->addWidget(toolb);

    // Install an application-global event filter to catch control+leftbutton
    qApp->installEventFilter( this );

    //make a central widget to contain the other widgets
    central = new QWidget( this );
    setCentralWidget( central );

    // Create a layout to position the widgets
    QHBoxLayout *topLayout = new QHBoxLayout( central, 10 );

    // Create a grid layout to hold most of the widgets
    QGridLayout *grid = new QGridLayout( 0, 3 ); //3 wide and autodetect number of rows
    topLayout->addLayout( grid, 1 );

    // Create an easter egg
    QToolTip::add( menuBar(), QRect( 0, 0, 2, 2 ), "easter egg" );

    QPopupMenu* popup;
    popup = new QPopupMenu( this );
    menuBar()->insertItem( "&File", popup );
    int id;
    id = popup->insertItem( "&New" );
    popup->setItemEnabled( id, FALSE );
    id = popup->insertItem( openIcon, "&Open...", this, SLOT( open() ) );

    popup->insertSeparator();
    popup->insertItem( "Quit", qApp, SLOT(quit()), CTRL+Key_Q );

    textStylePopup = popup = new QPopupMenu( this );
    menuBar()->insertItem( "&Edit", popup );

    plainStyleID = id = popup->insertItem( "&Plain" );
    popup->setAccel( CTRL+Key_T, id );

    connect( textStylePopup, SIGNAL(activated(int)),
	     this, SLOT(popupSelected(int)) );
#if 1
    // Create an analog and a digital clock
    AnalogClock  *aclock = new AnalogClock( central );
    aclock->setAutoMask( TRUE );
    DigitalClock *dclock = new DigitalClock( central );
    dclock->setMaximumWidth(200);
    grid->addWidget( aclock, 0, 2 );
    grid->addWidget( dclock, 1, 2 );

    // Give the dclock widget a blue palette
    col.setRgb( 0xaa, 0xbe, 0xff );
    dclock->setPalette( QPalette( col ) );

    // make tool tips for both of them
    QToolTip::add( aclock, "custom widget: analog clock" );
    QToolTip::add( dclock, "custom widget: digital clock" );
#endif
    // Create a push button.
    QPushButton *pb;
    pb = new QPushButton( "&Push button 1", central, "button1" );
    grid->addWidget( pb, 0, 0, AlignVCenter );
    connect( pb, SIGNAL(clicked()), SLOT(button1Clicked()) );
    QToolTip::add( pb, "push button 1" );
    QWhatsThis::add( pb, "This is a <b>QPushButton</b>.<br>"
		     "Click it and watch...<br>"
		     "The wonders of modern technology.");

    QPixmap pm;
    bool pix = pm.load("qt.png");
    if ( !pix ) {
	QMessageBox::information( 0, "Qt Widgets Example",
				  "Could not load the file \"qt.png\", which\n"
				  "contains an icon used...\n\n"
				  "The text \"line 42\" will be substituted.",
				  QMessageBox::Ok + QMessageBox::Default );
    }

#if 1
    // Create a label containing a QMovie
    movie = QMovie( MOVIEFILENAME );
    movielabel = new QLabel( central, "label0" );
    movie.connectStatus(this, SLOT(movieStatus(int)));
    movie.connectUpdate(this, SLOT(movieUpdate(const QRect&)));
    movielabel->setFrameStyle( QFrame::Box | QFrame::Plain );
    movielabel->setMovie( movie );
    movielabel->setFixedSize( 128+movielabel->frameWidth()*2,
			      64+movielabel->frameWidth()*2 );
    grid->addWidget( movielabel, 0, 1, AlignCenter );
    QToolTip::add( movielabel, "movie" );
    QWhatsThis::add( movielabel, "This is a <b>QLabel</b> "
		     "that contains a QMovie." );
#endif
    // Create a group of check boxes
    bg = new Q3ButtonGroup( central );
    bg->setTitle( "Check Boxes" );
    grid->addWidget( bg, 1, 0 );

    // Create a layout for the check boxes
    QVBoxLayout *vbox = new QVBoxLayout(bg, 10);

    vbox->addSpacing( bg->fontMetrics().height() );

    cb[0] = new QCheckBox( bg );
    cb[0]->setText( "&Read" );
    vbox->addWidget( cb[0] );
    cb[1] = new QCheckBox( bg );
    cb[1]->setText( "&Write" );
    vbox->addWidget( cb[1] );
    cb[2] = new QCheckBox( bg );
    cb[2]->setText( "&Execute" );
    vbox->addWidget( cb[2] );

    connect( bg, SIGNAL(clicked(int)), SLOT(checkBoxClicked(int)) );

    QToolTip::add( cb[0], "check box 1" );
    QToolTip::add( cb[1], "check box 2" );
    QToolTip::add( cb[2], "check box 3" );

    // Create a group of radio buttons
    QRadioButton *rb;
    bg = new Q3ButtonGroup( central, "radioGroup" );
    bg->setTitle( "Radio buttons" );

    grid->addWidget( bg, 1, 1 );

    // Create a layout for the radio buttons
    vbox = new QVBoxLayout(bg, 10);

    vbox->addSpacing( bg->fontMetrics().height() );
    rb = new QRadioButton( bg );
    rb->setText( "&AM" );
    rb->setChecked( TRUE );
    vbox->addWidget(rb);
    QToolTip::add( rb, "radio button 1" );
    rb = new QRadioButton( bg );
    rb->setText( "F&M" );
    vbox->addWidget(rb);
    QToolTip::add( rb, "radio button 2" );
    rb = new QRadioButton( bg );
    rb->setText( "&Short Wave" );
    vbox->addWidget(rb);

    connect( bg, SIGNAL(clicked(int)), SLOT(radioButtonClicked(int)) );
    QToolTip::add( rb, "radio button 3" );

    // Create a list box
    QListBox *lb = new QListBox( central, "listBox" );
    for ( int i=0; i<100; i++ ) {		// fill list box
	QString str;
	str.sprintf( "line %d", i );
	if ( i == 42 && pix )
	    lb->insertItem( pm );
	else
	    lb->insertItem( str );
    }
    grid->addWidget( lb, 2, 0, 3, 1);
    connect( lb, SIGNAL(selected(int)), SLOT(listBoxItemSelected(int)) );
    QToolTip::add( lb, "list box" );
    (void)new MyWhatsThis( lb );

    vbox = new QVBoxLayout(8);
    grid->addLayout( vbox, 2, 1 );

    // Create a slider
    QSlider *sb = new QSlider( Qt::Horizontal, central );
    sb->setMinimum( 0 );
    sb->setMaximumWidth( 300 );
    sb->setPageStep( 30 );
    sb->setSliderPosition( 100 );
    sb->setTickmarks( QSlider::Below );
    sb->setTickInterval( 10 );
    sb->setFocusPolicy( Qt::TabFocus );
    vbox->addWidget( sb );

    connect( sb, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)) );
    QToolTip::add( sb, "slider" );
    QWhatsThis::add( sb, "This is a <b>QSlider</b>. "
		     "The tick marks are optional."
		     " This slider controls the speed of the movie." );
    // Create a combo box
    QComboBox *combo = new QComboBox( FALSE, central, "comboBox" );
    combo->insertItem( "darkBlue" );
    combo->insertItem( "darkRed" );
    combo->insertItem( "darkGreen" );
    combo->insertItem( "blue" );
    combo->insertItem( "red" );
    vbox->addWidget( combo );
    connect( combo, SIGNAL(activated(int)),
	     this, SLOT(comboBoxItemActivated(int)) );
    QToolTip::add( combo, "read-only combo box" );

    // Create an editable combo box
    QComboBox *edCombo = new QComboBox( TRUE, central, "edComboBox" );
    // QListBox *edComboLst = new QListBox(this);
    //edCombo->setListBox(edComboLst);
    edCombo->insertItem( "Permutable" );
    edCombo->insertItem( "Malleable" );
    edCombo->insertItem( "Adaptable" );
    edCombo->insertItem( "Alterable" );
    edCombo->insertItem( "Inconstant" );
    vbox->addWidget( edCombo );
    connect( edCombo, SIGNAL(activated(const QString&)),
	     this, SLOT(edComboBoxItemActivated(const QString&)) );
    QToolTip::add( edCombo, "editable combo box" );

    edCombo->setAutoCompletion( TRUE );

    vbox = new QVBoxLayout(8);
    grid->addLayout( vbox, 2, 2 );

    // Create a spin box
    QSpinBox *spin = new QSpinBox( 0, 10, 1, central, "spin" );
    spin->setSuffix(" mm");
    spin->setSpecialValueText( "Auto" );
    connect( spin, SIGNAL( valueChanged(const QString&) ),
	     SLOT( spinBoxValueChanged(const QString&) ) );
    QToolTip::add( spin, "spin box" );
    QWhatsThis::add( spin, "This is a <b>QSpinBox</b>. "
		     "You can chose values in a given range "
		     "either by using the arrow buttons "
		     "or by typing them in." );
    vbox->addWidget( spin );

    vbox->addStretch( 1 );

    // Create a tabwidget that switches between multi line edits
    tabs = new QTabWidget( central );
    //tabs->setTabPosition( QTabWidget::Bottom );
    tabs->setMargin( 4 );
    grid->addWidget( tabs, 3, 1, 1, 2 );
    QMultiLineEdit *mle = new QMultiLineEdit( tabs, "multiLineEdit" );
    edit = mle;
    mle->setWordWrap( QMultiLineEdit::WidgetWidth );
    mle->setText("This is a QMultiLineEdit widget, "
	         "useful for small multi-line "
		 "input fields.");
    QToolTip::add( mle, "multi line editor" );

    tabs->addTab( mle, "F&irst");

    mle = new QMultiLineEdit( tabs, "multiLineEdit" );
    QString mleText = "This is another QMultiLineEdit widget.";
#if 1
    mleText += "\n";
    mleText += "Japanese: ";
    mleText += QChar((ushort)0x6a38); // Kanji
    mleText += "\n";
    mleText += "Russian: ";
    mleText += QChar((ushort)0x042e); // Cyrillic
    mleText += "\n";
    mleText += "Norwegian: ";
    mleText += QChar((ushort)0x00d8); // Norwegian
    mleText += "\n";
    mleText += "Unicode (black square): ";
    mleText += QChar((ushort)0x25A0); // BLACK SQUARE
    mleText += "\n";
#endif
    mle->setText( mleText );
    QToolTip::add( mle, "second multi line editor" );
    tabs->addTab( mle, "Se&cond");


    // Create a single line edit
    QLineEdit *le = new QLineEdit( central, "lineEdit" );


    grid->addWidget( le, 4, 1, 1, 2 );
    connect( le, SIGNAL(textChanged(const QString&)),
	     SLOT(lineEditTextChanged(const QString&)) );
    QToolTip::add( le, "single line editor" );
    QWhatsThis::add( le, "This is a <b>QLineEdit</b>, you can enter a "
		     "single line of text in it. "
		      "It also it accepts text drops." );

    grid->setRowStretch(0,0);
    grid->setRowStretch(1,0);
    grid->setRowStretch(2,0);
    grid->setRowStretch(3,1);
    grid->setRowStretch(4,0);

    grid->setColStretch(0,1);
    grid->setColStretch(1,1);
    grid->setColStretch(2,1);


    QSplitter *split = new QSplitter( Vertical, central, "splitter" );
    split->setOpaqueResize( TRUE );
    topLayout->addWidget( split, 1 );
    Q3ListView *lv = new MyListView( split );
    connect(lv, SIGNAL(selectionChanged() ),
	    this, SLOT( selectionChanged() ) );
    connect(lv, SIGNAL(selectionChanged(Q3ListViewItem*) ),
	    this, SLOT( selectionChanged(Q3ListViewItem*) ) );
    connect(lv, SIGNAL(clicked(Q3ListViewItem*) ),
	    this, SLOT( clicked(Q3ListViewItem*) ) );
    connect(lv, SIGNAL(mySelectionChanged(Q3ListViewItem*) ),
	    this, SLOT( mySelectionChanged(Q3ListViewItem*) ) );
    lv->addColumn( "One" );
    lv->addColumn( "Two" );
    lv->setAllColumnsShowFocus( TRUE );

    Q3ListViewItem *lvi=  new Q3ListViewItem( lv, "Text", "Text" );
    lvi=  new Q3ListViewItem( lv, "Text", "Other Text" );
    lvi=  new Q3ListViewItem( lv, "Text", "More Text" );
    lvi=  new Q3ListViewItem( lv, "Text", "Extra Text" );
    lvi->setOpen(TRUE);
    (void)new Q3ListViewItem( lvi, "SubText", "Additional Text" );
    lvi=  new Q3ListViewItem( lvi, "SubText", "Side Text" );
    lvi=  new Q3ListViewItem( lvi, "SubSubText", "Complimentary Text" );

    QToolTip::add( lv, "list view" );
    QWhatsThis::add( lv, "This is a <b>Q3ListView</b>, you can display lists "
		     "(or outline lists) of multiple-column data in it." );

    lv = new Q3ListView( split );
    lv->addColumn( "Choices" );
    (void) new QCheckListItem( lv, "Onion", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Artichoke", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Pepper", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Habaneros", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Pineapple", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Ham", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Pepperoni", QCheckListItem::CheckBox );
    (void) new QCheckListItem( lv, "Garlic", QCheckListItem::CheckBox );


    QCheckListItem *lit = new QCheckListItem( lv, "Cheese" );
    lit->setOpen( TRUE );
    (void) new QCheckListItem( lit, "Cheddar", QCheckListItem::RadioButton );
    (void) new QCheckListItem( lit, "Mozarella", QCheckListItem::RadioButton );
    (void) new QCheckListItem( lit, "Jarlsberg", QCheckListItem::RadioButton );

    QToolTip::add( lv, "list view" );
    QWhatsThis::add( lv, "This is also a <b>Q3ListView</b>, with "
		     "interactive items." );

    QTextBrowser *browser =  new QTextBrowser( split );
    browser->setText( "<h1>QTextBrowser</h1>"
		   "<p>Qt supports formatted rich text, such "
		   "as the heading above, <em>emphasized</em> and "
		   "<b>bold</b> text, via an XML subset.</p> "
		   "<p><a href=\"nogo://some.where.com\">Hypertext navigation</a> and style sheets are supported.</p>", "" );
    browser->setFont(QFont("Charter",11));
    browser->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    connect( browser, SIGNAL(linkClicked(const QString&)), browser, SLOT(setText(const QString&)) );

    // Create an label and a message in the status bar
    // The message is updated when buttons are clicked etc.
    msg = new QLabel( statusBar(), "message" );
    msg->setAlignment( AlignCenter );
    QFont boldfont; boldfont.setWeight(QFont::Bold);
    msg->setFont( boldfont );
    statusBar()->addWidget( msg, 4 );
    QToolTip::add( msg, "Message area" );

    prog = new QProgressBar( statusBar(), "progress" );
    prog->setTotalSteps( 100 );
    progress = 64;
    prog->setProgress( progress );
    statusBar()->addWidget( prog , 1 );
    QWhatsThis::add( prog, "This is a <b>QProgressBar</b> "
		     "You can use it to show that a lengthy "
		     " process is progressing. "
		     "In this program, nothing much seems to happen." );
    statusBar()->message( "Welcome to Qt", 2000 );
}

void WidgetView::setStatus(const QString& text)
{
    msg->setText(text);
}

void WidgetView::button1Clicked()
{
    msg->setText( "The push button was clicked" );
    prog->setProgress( ++progress );
}


void WidgetView::movieUpdate( const QRect& )
{
    // Uncomment this to test animated icons on your window manager
    //setIcon( movie.framePixmap() );
}

void WidgetView::movieStatus( int s )
{
    switch ( s ) {
      case QMovie::SourceEmpty:
      case QMovie::UnrecognizedFormat:
	{
	    QPixmap pm("tt-logo.png");
	    movielabel->setPixmap(pm);
	    movielabel->setFixedSize(pm.size());
	}
      break;
      default:
	if ( movielabel->movie() )	 	// for flicker-free animation:
	    movielabel->setAttribute(WA_NoSystemBackground, true);
    }
}


void WidgetView::popupSelected( int selectedId )
{
    if ( selectedId == plainStyleID ) {
	for ( int i = 0; i < int(textStylePopup->count()); i++ ) {
	    int id = textStylePopup->idAt( i );
	    textStylePopup->setItemChecked( id, FALSE);
	}
    } else {
	textStylePopup->setItemChecked( selectedId, TRUE );
    }
}

void WidgetView::checkBoxClicked( int id )
{
    QString str;
    str = tr("Check box %1 clicked : ").arg(id);
    QString chk = "---";
    if ( cb[0]->isChecked() )
	chk[0] = 'r';
    if ( cb[1]->isChecked() )
	chk[1] = 'w';
    if ( cb[2]->isChecked() )
	chk[2] = 'x';
    str += chk;
    msg->setText( str );
}


void WidgetView::edComboBoxItemActivated( const QString& text)
{
    QString str = tr("Editable Combo Box set to ");
    str += text;
    msg->setText( str );
}


void WidgetView::radioButtonClicked( int id )
{
    msg->setText( tr("Radio button #%1 clicked").arg(id) );
}


void WidgetView::listBoxItemSelected( int index )
{
    msg->setText( tr("List box item %1 selected").arg(index) );
}


void WidgetView::sliderValueChanged( int value )
{
    msg->setText( tr("Movie set to %1% of normal speed").arg(value) );
    movie.setSpeed( value );
}


void WidgetView::comboBoxItemActivated( int index )
{
    msg->setText( tr("Combo box item %1 activated").arg(index) );
    QPalette p(QApplication::palette());
    switch ( index ) {
    default:
    case 0:
	p.setColor(QPalette::Highlight, darkBlue);
	break;
    case 1:
	p.setColor(QPalette::Highlight, darkRed);
	break;
    case 2:
	p.setColor(QPalette::Highlight, darkGreen);
	break;
    case 3:
	p.setColor(QPalette::Highlight, blue);
	break;
    case 4:
	p.setColor(QPalette::Highlight, red);
	break;
    }
    QApplication::setPalette(p);
}



void WidgetView::lineEditTextChanged( const QString& newText )
{
    QString str( "Line edit text: ");
    str += newText;
    if ( newText.length() == 1 ) {
	QString u;
	u.sprintf(" (U%02x%02x)", newText[0].row(), newText[0].cell() );
	str += u;
    }
    msg->setText( str );
}


void WidgetView::spinBoxValueChanged( const QString& valueText )
{
    QString str( "Spin box value: " );
    str += valueText;
    msg->setText( str );
}

//
// All application events are passed through this event filter.
// We're using it to display some information about a clicked
// widget (right mouse button + CTRL).
//

bool WidgetView::eventFilter( QObject *obj, QEvent *event )
{
    static bool identify_now = TRUE;
    if ( event->type() == QEvent::MouseButtonPress && identify_now ) {
	QMouseEvent *e = (QMouseEvent*)event;
	if ( e->button() == Qt::RightButton &&
	     (e->state() & Qt::ControlButton) != 0 ){
	    QString str = "The clicked widget is a\n";
	    str += obj->className();
	    str += "\nThe widget's name is\n";
	    if ( obj->objectName().isEmpty() )
		str += "<no name>";
	    else
		str += obj->objectName();
	    identify_now = FALSE;		// don't do it in message box
	    QMessageBox::information( (QWidget*)obj, "Identify Widget", str );
	    identify_now = TRUE;		// allow it again
	}
    }
    return QMainWindow::eventFilter( obj, event ); // don't eat event
}


void WidgetView::open()
{
    //###### QFileDialog::getOpenFileName( QString::null, "Textfiles (*.txt)", this );
}


void WidgetView::dummy()
{
    QMessageBox::information( this, "Sorry",
			      "This function is not implemented" );
}

void WidgetView::selectionChanged()
{
    //qDebug("selectionChanged");
}
void WidgetView::selectionChanged( Q3ListViewItem* /*item*/)
{
    //qDebug("selectionChanged %p", item );
}

void WidgetView::clicked( Q3ListViewItem* /*item*/ )
{
    //qDebug("clicked %p", item );
}

void WidgetView::mySelectionChanged( Q3ListViewItem* /*item*/ )
{
    //qDebug("mySelectionChanged %p", item );
}
