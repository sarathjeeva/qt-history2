//SDM?? I just don't get why this is platform specific!

/****************************************************************************
** $Id$
**
** The script engine jump table
**
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

const q_scriptEngine scriptEngines[] = {
	// Latin,
    { basic_shape, basic_attributes },
	// Greek,
    { basic_shape, basic_attributes },
	// Cyrillic,
    { basic_shape, basic_attributes },
	// Armenian,
    { basic_shape, basic_attributes },
	// Georgian,
    { basic_shape, basic_attributes },
	// Runic,
    { basic_shape, basic_attributes },
	// Ogham,
    { basic_shape, basic_attributes },
	// SpacingModifiers,
    { basic_shape, basic_attributes },
	// CombiningMarks,
    { basic_shape, basic_attributes },

	// // Middle Eastern Scripts
	// Hebrew,
    { basic_shape, basic_attributes },
	// Arabic,
    { arabic_shape, arabic_attributes },
	// Syriac,
    { basic_shape, basic_attributes },
	// Thaana,
    { basic_shape, basic_attributes },

	// // South and Southeast Asian Scripts
	// Devanagari,
    { basic_shape, basic_attributes },
	// Bengali,
    { basic_shape, basic_attributes },
	// Gurmukhi,
    { basic_shape, basic_attributes },
	// Gujarati,
    { basic_shape, basic_attributes },
	// Oriya,
    { basic_shape, basic_attributes },
	// Tamil,
    { basic_shape, basic_attributes },
	// Telugu,
    { basic_shape, basic_attributes },
	// Kannada,
    { basic_shape, basic_attributes },
	// Malayalam,
    { basic_shape, basic_attributes },
	// Sinhala,
    { basic_shape, basic_attributes },
	// Thai,
    { basic_shape, basic_attributes },
	// Lao,
    { basic_shape, basic_attributes },
	// Tibetan,
    { basic_shape, basic_attributes },
	// Myanmar,
    { basic_shape, basic_attributes },
	// Khmer,
    { basic_shape, basic_attributes },

	// // East Asian Scripts
	// Han,
    { basic_shape, asian_attributes },
	// Hiragana,
    { basic_shape, asian_attributes },
	// Katakana,
    { basic_shape, asian_attributes },
	// Hangul,
    { basic_shape, asian_attributes },
	// Bopomofo,
    { basic_shape, asian_attributes },
	// Yi,
    { basic_shape, asian_attributes },

	// // Additional Scripts
	// Ethiopic,
    { basic_shape, basic_attributes },
	// Cherokee,
    { basic_shape, basic_attributes },
	// CanadianAboriginal,
    { basic_shape, basic_attributes },
	// Mongolian,
    { basic_shape, basic_attributes },

	// // Symbols
	// CurrencySymbols,
    { basic_shape, basic_attributes },
	// LetterlikeSymbols,
    { basic_shape, basic_attributes },
	// NumberForms,
    { basic_shape, basic_attributes },
	// MathematicalOperators,
    { basic_shape, basic_attributes },
	// TechnicalSymbols,
    { basic_shape, basic_attributes },
	// GeometricSymbols,
    { basic_shape, basic_attributes },
	// MiscellaneousSymbols,
    { basic_shape, basic_attributes },
	// EnclosedAndSquare,
    { basic_shape, basic_attributes },
	// Braille,
    { basic_shape, basic_attributes },

	// Unicode,
    { basic_shape, basic_attributes },
    //Tagalog,
    { basic_shape, basic_attributes },
    //Hanunoo,
    { basic_shape, basic_attributes },
    //Buhid,
    { basic_shape, basic_attributes },
    //Tagbanwa,
    { basic_shape, basic_attributes },
    // KatakanaHalfWidth
    { basic_shape, basic_attributes }
};
