/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "tetrixboard.h"

#include <QtGui>

Q_DECLARE_METATYPE(QPainter*)

TetrixBoard::TetrixBoard(QWidget *parent)
    : QFrame(parent)
{
    timer = new QTimer(this);
    qMetaTypeId<QPainter*>();
}

void TetrixBoard::setNextPieceLabel(QWidget *label)
{
    nextPieceLabel = qobject_cast<QLabel*>(label);
}

QObject *TetrixBoard::getTimer()
{
    return timer;
}

QSize TetrixBoard::minimumSizeHint() const
{
    return QSize(BoardWidth * 5 + frameWidth() * 2,
                 BoardHeight * 5 + frameWidth() * 2);
}

void TetrixBoard::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    QRect rect = contentsRect();
    int boardTop = rect.bottom() - BoardHeight*squareHeight();
    painter.translate(rect.left(), boardTop);

    emit paintRequested(&painter);
}

void TetrixBoard::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event->key());
}

void TetrixBoard::showNextPiece(int width, int height)
{
    if (!nextPieceLabel)
        return;

    QPixmap pixmap(width * squareWidth(), height * squareHeight());
    QPainter painter(&pixmap);
    painter.fillRect(pixmap.rect(), nextPieceLabel->palette().background());

    emit paintNextPieceRequested(&painter);

    nextPieceLabel->setPixmap(pixmap);
}

void TetrixBoard::drawPauseScreen(QPainter *painter)
{
    painter->drawText(contentsRect(), Qt::AlignCenter, tr("Pause"));
}

void TetrixBoard::drawSquare(QPainter *painter, int x, int y, int shape)
{
    static const QRgb colorTable[8] = {
        0x000000, 0xCC6666, 0x66CC66, 0x6666CC,
        0xCCCC66, 0xCC66CC, 0x66CCCC, 0xDAAA00
    };

    x = x*squareWidth();
    y = y*squareHeight();

    QColor color = colorTable[shape];
    painter->fillRect(x + 1, y + 1, squareWidth() - 2, squareHeight() - 2,
                      color);

    painter->setPen(color.light());
    painter->drawLine(x, y + squareHeight() - 1, x, y);
    painter->drawLine(x, y, x + squareWidth() - 1, y);

    painter->setPen(color.dark());
    painter->drawLine(x + 1, y + squareHeight() - 1,
                      x + squareWidth() - 1, y + squareHeight() - 1);
    painter->drawLine(x + squareWidth() - 1, y + squareHeight() - 1,
                      x + squareWidth() - 1, y + 1);
}
