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

#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include "qstring.h"
#include "qnamespace.h"
#include "qrect.h"
#include "qvector.h"
#include "qcolor.h"
#include "qobject.h"

class QTextEngine;
class QFont;
class QRect;
class QRegion;
class QTextFormat;
class QPalette;
class QPainter;

class Q_GUI_EXPORT QTextInlineObject
{
public:
    QTextInlineObject(int i, QTextEngine *e) : itm(i), eng(e) {}
    inline QTextInlineObject() : itm(0), eng(0) {}
    inline bool isValid() const { return eng; }

    QRect rect() const;
    float width() const;
    float ascent() const;
    float descent() const;
    float height() const;

    bool isRightToLeft() const;

    void setWidth(float w);
    void setAscent(float a);
    void setDescent(float d);

    int at() const;

    QTextEngine *engine() const { return eng; }
    int item() const { return itm; }

    int formatIndex() const;
    QTextFormat format() const;

private:
    friend class QTextLayout;
    int itm;
    QTextEngine *eng;
};

class QPaintDevice;
class QTextFormat;
class QTextLine;
class QTextBlock;

class Q_GUI_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout();
    QTextLayout(const QString& string);
    QTextLayout(const QString& string, const QFont &font, QPaintDevice *paintdevice);
    QTextLayout(const QString& string, const QFont& fnt);
    QTextLayout(const QTextBlock &b);
    ~QTextLayout();

    void setText(const QString& string, const QFont& fnt);

    // ######### go away
    void setText(const QString& string);
    QString text() const;

    enum LineBreakStrategy {
        AtWordBoundaries,
        AtCharBoundaries
    };

    void setTextFlags(int textFlags);

    enum PaletteFlags {
        None,
        UseTextColor
    };
    void setPalette(const QPalette &, PaletteFlags f = None);

    void useDesignMetrics(bool);
    bool usesDesignMetrics() const;

    enum LayoutMode {
        MultiLine = 0,
        NoBidi = 1,
        SingleLine = 2
    };

    void clearLines();
    QTextLine createLine();

    int numLines() const;
    QTextLine lineAt(int i) const;
    QTextLine findLine(int pos) const;

    // ### go away!
    void beginLayout(LayoutMode m = MultiLine, int textFlags = 0);


    enum CursorMode {
        SkipCharacters,
        SkipWords
    };
    bool validCursorPosition(int pos) const;
    int nextCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
    int previousCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;

    enum SelectionType {
        NoSelection = 0,
        Highlight = -1,
        ImText = -2,
        ImSelection = -3
    };
    class Selection {
        int f;
        int l;
        int t;
    public:
        Selection() : f(-1), l(0), t(NoSelection) {}
        //Selection(int f, int l, int formatIndex) : from(f), length(l), selectionType(formatIndex) {}
        Selection(int from, int length, SelectionType type) : f(from), l(length), t(type) {}
        inline int from() const { return f; }
        inline int length() const { return l; }
        inline int type() const { return t; }
        inline void setRange(int from, int length) { f = from; l = length; }
        inline void setType(SelectionType type) { t = type; }
    };

    enum { NoCursor = -1 };

    inline void draw(QPainter *p, const QPointF &pos, int cursorPos, const QVector<Selection> &selections) const {
        draw(p, pos, cursorPos, selections.constData(), selections.size());
    }
    void draw(QPainter *p, const QPointF &pos, int cursorPos = NoCursor,
              const Selection *selections = 0, int nSelections = 0, const QRect &cr = QRect()) const;

    QPointF position() const;
    void setPosition(const QPointF &p);

    QRectF boundingRect() const;
    QRectF rect() const;

    QTextEngine *engine() const { return d; }

    int minimumWidth() const;
    int maximumWidth() const;

    void setDirection(QChar::Direction);
private:
    QTextLayout(QTextEngine *e) : d(e) {}
    /* disable copy and assignment */
    QTextLayout(const QTextLayout &) {}
    void operator = (const QTextLayout &) {}

    friend class QPainter;
    friend class QPSPrinter;
    QTextEngine *d;
};

class Q_GUI_EXPORT QTextLine
{
public:
    inline QTextLine() : i(0), eng(0) {}
    inline bool isValid() const { return eng; }

    QRect rect() const;
    float x() const;
    float y() const;
    float width() const;
    float ascent() const;
    float descent() const;
    float height() const;
    float textWidth() const;

    enum Edge {
        Leading,
        Trailing
    };
    enum CursorPosition {
        CursorBetweenCharacters,
        CursorOnCharacter
    };

    /* cursorPos gets set to the valid position */
    float cursorToX(int *cursorPos, Edge edge = Leading) const;
    inline float cursorToX(int cursorPos, Edge edge = Leading) const { return cursorToX(&cursorPos, edge); }
    int xToCursor(float x, CursorPosition = CursorBetweenCharacters) const;

    void layout(float width);
    void layoutFixedColumnWidth(int numColumns);
    void setPosition(const QPointF &pos);

    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int line() const { return i; }

    void draw(QPainter *p, const QPointF &point, int selection = -1) const;

private:
    QTextLine(int line, QTextEngine *e) : i(line), eng(e) {}
    void layout_helper(int numGlyphs);
    friend class QTextLayout;
    int i;
    QTextEngine *eng;
};

#endif
