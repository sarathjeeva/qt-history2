/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "paletteeditorimpl.h"
#include "paletteeditoradvancedimpl.h"
#include "previewframe.h"
#include "styledbutton.h"
#include "mainwindow.h"
#include "formwindow.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qapplication.h>
#include <qpushbutton.h>

PaletteEditor::PaletteEditor( FormWindow *fw, QWidget * parent, const char * name, bool modal, WFlags f )
    : PaletteEditorBase( parent, name, modal, f ), formWindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );

    editPalette =  QApplication::palette();
    setPreviewPalette( editPalette );

    buttonMainColor->setColor( editPalette.active().color( QPalette::Button ) );
    buttonMainColor2->setColor( editPalette.active().color( QPalette::Background ) );
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::onTune()
{
    bool ok;
    QPalette pal = PaletteEditorAdvanced::getPalette( &ok, editPalette, backgroundMode, this, "tune_palette", formWindow);
    if (!ok) return;

    editPalette = pal;
    setPreviewPalette( editPalette );
}

void PaletteEditor::onChooseMainColor()
{
    buildPalette();
}

void PaletteEditor::onChoose2ndMainColor()
{
    buildPalette();
}

void PaletteEditor::paletteSelected(int)
{
    setPreviewPalette(editPalette);
}

QPalette::ColorRole PaletteEditor::centralFromItem( int item )
{
    switch( item )
	{
	case 0:
	    return QPalette::Background;
	case 1:
	    return QPalette::Foreground;
	case 2:
	    return QPalette::Button;
	case 3:
	    return QPalette::Base;
	case 4:
	    return QPalette::Text;
	case 5:
	    return QPalette::BrightText;
	case 6:
	    return QPalette::ButtonText;
	case 7:
	    return QPalette::Highlight;
	case 8:
	    return QPalette::HighlightedText;
	default:
	    return QPalette::NColorRoles;
	}
}

QPalette::ColorRole PaletteEditor::effectFromItem( int item )
{
    switch( item )
	{
	case 0:
	    return QPalette::Light;
	case 1:
	    return QPalette::Midlight;
	case 2:
	    return QPalette::Mid;
	case 3:
	    return QPalette::Dark;
	case 4:
	    return QPalette::Shadow;
	default:
	    return QPalette::NColorRoles;
	}
}

void PaletteEditor::buildPalette()
{
    int i;
    QPalette cg;
    QColor btn = buttonMainColor->color();
    QColor back = buttonMainColor2->color();
    QPalette automake( btn, back );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), automake.active().color( centralFromItem(i) ) );

    editPalette.setActive( cg );
    buildActiveEffect();

    cg = editPalette.inactive();

    QPalette temp( editPalette.active().color( QPalette::Button ),
		   editPalette.active().color( QPalette::Background ) );

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.inactive().color( centralFromItem(i) ) );

    editPalette.setInactive( cg );
    buildInactiveEffect();

    cg = editPalette.disabled();

    for (i = 0; i<9; i++)
	cg.setColor( centralFromItem(i), temp.disabled().color( centralFromItem(i) ) );

    editPalette.setDisabled( cg );
    buildDisabledEffect();

    updateStyledButtons();
}

void PaletteEditor::buildActiveEffect()
{
    QPalette cg = editPalette.active();
    QColor btn = cg.color( QPalette::Button );

    QPalette temp( btn, btn );

    for (int i = 0; i<5; i++)
	cg.setColor( effectFromItem(i), temp.active().color( effectFromItem(i) ) );

    editPalette.setActive( cg );
    setPreviewPalette( editPalette );

    updateStyledButtons();
}

void PaletteEditor::buildInactive()
{
    editPalette.setInactive( editPalette.active() );
    buildInactiveEffect();
}

void PaletteEditor::buildInactiveEffect()
{
    QPalette cg = editPalette.inactive();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QPalette::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QPalette::Light, light );
    cg.setColor( QPalette::Midlight, midlight );
    cg.setColor( QPalette::Mid, mid );
    cg.setColor( QPalette::Dark, dark );
    cg.setColor( QPalette::Shadow, shadow );

    editPalette.setInactive( cg );
    setPreviewPalette( editPalette );
    updateStyledButtons();
}

void PaletteEditor::buildDisabled()
{
    QPalette cg = editPalette.active();
    cg.setColor( QPalette::ButtonText, darkGray );
    cg.setColor( QPalette::Foreground, darkGray );
    editPalette.setDisabled( cg );

    buildDisabledEffect();
}

void PaletteEditor::buildDisabledEffect()
{
    QPalette cg = editPalette.disabled();

    QColor light, midlight, mid, dark, shadow;
    QColor btn = cg.color( QPalette::Button );

    light = btn.light(150);
    midlight = btn.light(115);
    mid = btn.dark(150);
    dark = btn.dark();
    shadow = black;

    cg.setColor( QPalette::Light, light );
    cg.setColor( QPalette::Midlight, midlight );
    cg.setColor( QPalette::Mid, mid );
    cg.setColor( QPalette::Dark, dark );
    cg.setColor( QPalette::Shadow, shadow );

    editPalette.setDisabled( cg );
    setPreviewPalette( editPalette );
    updateStyledButtons();
}

void PaletteEditor::setPreviewPalette( const QPalette& pal )
{
    QPalette cg;

    switch (paletteCombo->currentItem()) {
    case 0:
    default:
	cg = pal.active();
	break;
    case 1:
	cg = pal.inactive();
	break;
    case 2:
	cg = pal.disabled();
	break;
    }
    previewPalette.setActive( cg );
    previewPalette.setInactive( cg );
    previewPalette.setDisabled( cg );

    previewFrame->setPreviewPalette(previewPalette);
}

void PaletteEditor::updateStyledButtons()
{
    buttonMainColor->setColor( editPalette.active().color( QPalette::Button ));
    buttonMainColor2->setColor( editPalette.active().color( QPalette::Background ));
}

void PaletteEditor::setPal( const QPalette& pal )
{
    editPalette = pal;
    setPreviewPalette( pal );
    updateStyledButtons();
}

QPalette PaletteEditor::pal() const
{
    return editPalette;
}

QPalette PaletteEditor::getPalette( bool *ok, const QPalette &init, BackgroundMode mode,
				    QWidget* parent, const char* name, FormWindow *fw )
{
    PaletteEditor* dlg = new PaletteEditor( fw, parent, name, TRUE );
    dlg->setupBackgroundMode( mode );

    if ( init != QPalette() )
        dlg->setPal( init );
    int resultCode = dlg->exec();

    QPalette result = init;
    if ( resultCode == QDialog::Accepted ) {
	if ( ok )
	    *ok = TRUE;
	result = dlg->pal();
    } else {
	if ( ok )
	    *ok = FALSE;
    }
    delete dlg;
    return result;
}
