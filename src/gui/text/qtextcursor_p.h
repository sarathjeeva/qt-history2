#ifndef QTEXTCURSOR_P_H
#define QTEXTCURSOR_P_H

#include "qtextcursor.h"
#include "qtextdocument.h"
#include "qtextpiecetable_p.h"
#include <private/qtextformat_p.h>

class QTextCursorPrivate : public QSharedObject
{
public:
    QTextCursorPrivate(const QTextPieceTable *table);
    QTextCursorPrivate(const QTextCursorPrivate &rhs);
    ~QTextCursorPrivate();

    void adjustPosition(int positionOfChange, int charsAddedOrRemoved, UndoCommand::Operation op);

    void remove();
    void setPosition(int newPosition);
    void setX();
    bool canDelete(int pos) const;

    void insertDirect(int strPos, int strLength, int format);
    void insertBlock(const QTextBlockFormat &format);
    bool moveTo(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

    QTextPieceTable::BlockIterator block() const
    { return pieceTable->blocksFind(position-1); }
    QTextBlockFormat blockFormat() const
    { return block().blockFormat(); }

    void adjustCursor(int dir);

    int x;
    int position;
    int anchor;
    int adjusted_anchor;
    QTextPieceTablePointer pieceTable;
};

#endif // QTEXTCURSOR_P_H
