/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfontdatabase.h#11 $
**
** Definition of the QFontDatabase class
**
** Created : 981126
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#include "qwindowdefs.h"

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qfont.h"
#include "qlist.h"
#include "qvaluelist.h"
#endif // QT_H

class QStringList;
class QFontStylePrivate;

class QFontDatabasePrivate;

class Q_EXPORT QFontDatabase
{
public:
    QFontDatabase();

    QStringList families( bool onlyForLocale = TRUE ) const;
    QValueList<int> pointSizes( const QString &family,
				const QString &style = QString::null,
				const QString &charSet = QString::null );
    QStringList styles( const QString &family,
			const QString &charSet = QString::null ) const;
    QStringList charSets( const QString &familyName,
			  bool onlyForLocale = TRUE ) const;

    QFont font( const QString familyName, const QString &style,
		int pointSize, const QString charSetName = QString::null );

    bool  isBitmapScalable( const QString &family,
			    const QString &style   = QString::null,
			    const QString &charSet = QString::null ) const;
    bool  isSmoothlyScalable( const QString &family,
			      const QString &style   = QString::null,
			      const QString &charSet = QString::null ) const;
    bool  isScalable( const QString &family,
		      const QString &style   = QString::null,
		      const QString &charSet = QString::null ) const;

    QValueList<int> smoothSizes( const QString &family,
				 const QString &style,
				 const QString &charSet = QString::null );

    static QValueList<int> standardSizes();

    bool italic( const QString &family,
		 const QString &style,
		 const QString &charSet = QString::null ) const;

    bool bold( const QString &family,
	       const QString &style,
	       const QString &charSet = QString::null ) const;

    int weight( const QString &family,
		const QString &style,
		const QString &charSet = QString::null ) const;


#if 0
    QValueList<QFont::CharSet> charSets( const QString &familyName ) const;
    bool  supportsCharSet( const QString &familyName,
			   const QString &charSet ) const;
    bool  supportsCharSet( const QString &familyName,
			   QFont::CharSet charSet ) const;
#endif

    QString styleString( const QFont &);

    static QString verboseCharSetName( const QString & charSetName );
    static QString charSetSample( const QString & charSetName );

private:
    static void createDatabase();

    QFontDatabasePrivate *d;

};

#endif // QFONTDATABASE_H
