#ifndef QTEXTLISTMANAGER_P_H
#define QTEXTLISTMANAGER_P_H

#ifndef QT_H
#include <qobject.h>
#include <qtextformat.h>
#include <qmap.h>
#include "qtextpiecetable_p.h"
#include "qtextlist_p.h"
#endif

class QTextList;

class QTextListManager : public QObject
{
    Q_OBJECT
public:
    QTextListManager(QTextPieceTable *table);
    ~QTextListManager();

    QTextList *list(int listIdx) const;

    QTextPieceTable *pieceTable() { return table; }

    QVector<QTextPieceTable::BlockIterator> blocksForObject(int listIdx) const;

private slots:
    void blockChanged(int blockPosition, QText::ChangeOperation);
    void formatChanged(int position, int length);
    void listDestroyed(QObject *obj);

private:
    void removeListEntry(int listIdx, const QTextPieceTable::BlockIterator &blockIt);
    void addListEntry(int listIdx, const QTextPieceTable::BlockIterator &blockIt);

    // map from list index to list of blocks
    typedef QMap<int, QTextList *> ListMap;
    ListMap lists;

    QTextPieceTable *table;

#if defined(Q_DISABLE_COPY)
    QTextListManager(const QTextListManager &);
    QTextListManager &operator=(const QTextListManager &);
#endif
};

#endif // QTEXTLISTMANAGER_P_H
