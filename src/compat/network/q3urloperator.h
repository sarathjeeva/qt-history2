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

#ifndef Q3URLOPERATOR_H
#define Q3URLOPERATOR_H

#ifndef QT_H
#include "qobject.h"
#include "q3url.h"
#include "q3ptrlist.h"
#include "q3networkprotocol.h"
#include "qstringlist.h" // QString->QStringList conversion
#endif // QT_H

#ifndef QT_NO_NETWORKPROTOCOL

class QUrlInfo;
class Q3UrlOperatorPrivate;
class Q3NetworkProtocol;

class Q_COMPAT_EXPORT Q3UrlOperator : public QObject, public Q3Url
{
    Q_OBJECT
    friend class Q3NetworkProtocol;

public:
    Q3UrlOperator();
    Q3UrlOperator( const QString &urL );
    Q3UrlOperator( const Q3UrlOperator& url );
    Q3UrlOperator( const Q3UrlOperator& url, const QString& relUrl, bool checkSlash = false );
    virtual ~Q3UrlOperator();

    virtual void setPath( const QString& path );
    virtual bool cdUp();

    virtual const Q3NetworkOperation *listChildren();
    virtual const Q3NetworkOperation *mkdir( const QString &dirname );
    virtual const Q3NetworkOperation *remove( const QString &filename );
    virtual const Q3NetworkOperation *rename( const QString &oldname, const QString &newname );
    virtual const Q3NetworkOperation *get( const QString &location = QString::null );
    virtual const Q3NetworkOperation *put( const QByteArray &data, const QString &location = QString::null  );
    virtual Q3PtrList<Q3NetworkOperation> copy( const QString &from, const QString &to, bool move = false, bool toPath = true );
    virtual void copy( const QStringList &files, const QString &dest, bool move = false );
    virtual bool isDir( bool *ok = 0 );

    virtual void setNameFilter( const QString &nameFilter );
    QString nameFilter() const;

    virtual QUrlInfo info( const QString &entry ) const;

    Q3UrlOperator& operator=( const Q3UrlOperator &url );
    Q3UrlOperator& operator=( const QString &url );

    virtual void stop();

signals:
    void newChildren( const Q3ValueList<QUrlInfo> &, Q3NetworkOperation *res );
    void finished( Q3NetworkOperation *res );
    void start( Q3NetworkOperation *res );
    void createdDirectory( const QUrlInfo &, Q3NetworkOperation *res );
    void removed( Q3NetworkOperation *res );
    void itemChanged( Q3NetworkOperation *res );
    void data( const QByteArray &, Q3NetworkOperation *res );
    void dataTransferProgress( int bytesDone, int bytesTotal, Q3NetworkOperation *res );
    void startedNextCopy( const Q3PtrList<Q3NetworkOperation> &lst );
    void connectionStateChanged( int state, const QString &data );

protected:
    void reset();
    bool parse( const QString& url );
    virtual bool checkValid();
    virtual void clearEntries();
    void getNetworkProtocol();
    void deleteNetworkProtocol();

private slots:
    const Q3NetworkOperation *startOperation( Q3NetworkOperation *op );
    void copyGotData( const QByteArray &data, Q3NetworkOperation *op );
    void continueCopy( Q3NetworkOperation *op );
    void finishedCopy();
    void addEntry( const Q3ValueList<QUrlInfo> &i );
    void slotItemChanged( Q3NetworkOperation *op );

private:
    void deleteOperation( Q3NetworkOperation *op );

    Q3UrlOperatorPrivate *d;
};

#endif // QT_NO_NETWORKPROTOCOL

#endif // Q3URLOPERATOR_H
