/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#include <QtGui/qabstractscrollarea.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qtextoption.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>

#ifndef QT_NO_TEXTEDIT

#ifdef QT3_SUPPORT
#include <QtGui/qtextobject.h>
#include <QtGui/qtextlayout.h>
#endif

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QStyleSheet;
class QTextDocument;
class QMenu;
class QTextEditPrivate;
class QMimeData;

class Q_GUI_EXPORT QTextEdit : public QAbstractScrollArea
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTextEdit)
    Q_FLAGS(AutoFormatting)
    Q_ENUMS(LineWrapMode)
    Q_PROPERTY(AutoFormatting autoFormatting READ autoFormatting WRITE setAutoFormatting)
    Q_PROPERTY(bool tabChangesFocus READ tabChangesFocus WRITE setTabChangesFocus)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle)
    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
    Q_PROPERTY(LineWrapMode lineWrapMode READ lineWrapMode WRITE setLineWrapMode)
    QDOC_PROPERTY(QTextOption::WrapMode wordWrapMode READ wordWrapMode WRITE setWordWrapMode)
    Q_PROPERTY(int lineWrapColumnOrWidth READ lineWrapColumnOrWidth WRITE setLineWrapColumnOrWidth)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(QString html READ toHtml WRITE setHtml NOTIFY textChanged USER true)
    Q_PROPERTY(bool overwriteMode READ overwriteMode WRITE setOverwriteMode)
    Q_PROPERTY(int tabStopWidth READ tabStopWidth WRITE setTabStopWidth)
    Q_PROPERTY(bool acceptRichText READ acceptRichText WRITE setAcceptRichText)
    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth)
public:
    enum LineWrapMode {
        NoWrap,
        WidgetWidth,
        FixedPixelWidth,
        FixedColumnWidth
    };

    enum AutoFormattingFlag {
        AutoNone = 0,
        AutoBulletList = 0x00000001,
        AutoAll = 0xffffffff
    };

    Q_DECLARE_FLAGS(AutoFormatting, AutoFormattingFlag)

    enum CursorAction {
        MoveBackward,
        MoveForward,
        MoveWordBackward,
        MoveWordForward,
        MoveUp,
        MoveDown,
        MoveLineStart,
        MoveLineEnd,
        MoveHome,
        MoveEnd,
        MovePageUp,
        MovePageDown
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        ,
        MovePgUp = MovePageUp,
        MovePgDown = MovePageDown
#endif
    };

    explicit QTextEdit(QWidget *parent = 0);
    explicit QTextEdit(const QString &text, QWidget *parent = 0);
    virtual ~QTextEdit();

    void setDocument(QTextDocument *document);
    QTextDocument *document() const;

    void setTextCursor(const QTextCursor &cursor);
    QTextCursor textCursor() const;

    bool isReadOnly() const;
    void setReadOnly(bool ro);

    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    Qt::TextInteractionFlags textInteractionFlags() const;

    qreal fontPointSize() const;
    QString fontFamily() const;
    int fontWeight() const;
    bool fontUnderline() const;
    bool fontItalic() const;
    QColor textColor() const;
    QFont currentFont() const;
    Qt::Alignment alignment() const;

    void mergeCurrentCharFormat(const QTextCharFormat &modifier);

    void setCurrentCharFormat(const QTextCharFormat &format);
    QTextCharFormat currentCharFormat() const;

    AutoFormatting autoFormatting() const;
    void setAutoFormatting(AutoFormatting features);

    bool tabChangesFocus() const;
    void setTabChangesFocus(bool b);

    inline void setDocumentTitle(const QString &title)
    { document()->setMetaInformation(QTextDocument::DocumentTitle, title); }
    inline QString documentTitle() const
    { return document()->metaInformation(QTextDocument::DocumentTitle); }

    inline bool isUndoRedoEnabled() const
    { return document()->isUndoRedoEnabled(); }
    inline void setUndoRedoEnabled(bool enable)
    { document()->setUndoRedoEnabled(enable); }

    LineWrapMode lineWrapMode() const;
    void setLineWrapMode(LineWrapMode mode);

    int lineWrapColumnOrWidth() const;
    void setLineWrapColumnOrWidth(int w);

    QTextOption::WrapMode wordWrapMode() const;
    void setWordWrapMode(QTextOption::WrapMode policy);

    bool find(const QString &exp, QTextDocument::FindFlags options = 0);

    inline QString toPlainText() const
    { return document()->toPlainText(); }
    inline QString toHtml() const
    { return document()->toHtml(); }

    void ensureCursorVisible();

    virtual QVariant loadResource(int type, const QUrl &name);
#ifndef QT_NO_CONTEXTMENU
    QMenu *createStandardContextMenu();
#endif

    QTextCursor cursorForPosition(const QPoint &pos) const;
    QRect cursorRect(const QTextCursor &cursor) const;
    QRect cursorRect() const;

    QString anchorAt(const QPoint& pos) const;

    bool overwriteMode() const;
    void setOverwriteMode(bool overwrite);

    int tabStopWidth() const;
    void setTabStopWidth(int width);

    int cursorWidth() const;
    void setCursorWidth(int width);

    bool acceptRichText() const;
    void setAcceptRichText(bool accept);

    struct ExtraSelection
    {
        QTextCursor cursor;
        QTextCharFormat format;
    };
    void setExtraSelections(const QList<ExtraSelection> &selections);
    QList<ExtraSelection> extraSelections() const;

public Q_SLOTS:
    void setFontPointSize(qreal s);
    void setFontFamily(const QString &fontFamily);
    void setFontWeight(int w);
    void setFontUnderline(bool b);
    void setFontItalic(bool b);
    void setTextColor(const QColor &c);
    void setCurrentFont(const QFont &f);
    void setAlignment(Qt::Alignment a);

    void setPlainText(const QString &text);
    void setHtml(const QString &text);

#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif

    void undo();
    void redo();

    void clear();
    void selectAll();

    void insertPlainText(const QString &text);
    void insertHtml(const QString &text);

    void append(const QString &text);

    void scrollToAnchor(const QString &name);

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

    void moveCursor(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

Q_SIGNALS:
    void textChanged();
    void undoAvailable(bool b);
    void redoAvailable(bool b);
    void currentCharFormatChanged(const QTextCharFormat &format);
    void copyAvailable(bool b);
    void selectionChanged();
    void cursorPositionChanged();

protected:
    virtual bool event(QEvent *e);
    virtual void timerEvent(QTimerEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual bool focusNextPrevChild(bool next);
    virtual void contextMenuEvent(QContextMenuEvent *e);
#ifndef QT_NO_DRAGANDDROP
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragLeaveEvent(QDragLeaveEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dropEvent(QDropEvent *e);
#endif
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void showEvent(QShowEvent *);
    virtual void changeEvent(QEvent *e);
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent *e);
#endif

    virtual QMimeData *createMimeDataFromSelection() const;
    virtual bool canInsertFromMimeData(const QMimeData *source) const;
    virtual void insertFromMimeData(const QMimeData *source);

    virtual void inputMethodEvent(QInputMethodEvent *);
    QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    QTextEdit(QTextEditPrivate &dd, QWidget *parent);

    virtual void scrollContentsBy(int dx, int dy);

#ifdef QT3_SUPPORT
Q_SIGNALS:
    QT_MOC_COMPAT void currentFontChanged(const QFont &f);
    QT_MOC_COMPAT void currentColorChanged(const QColor &c);

public:
    QT3_SUPPORT_CONSTRUCTOR QTextEdit(QWidget *parent, const char *name);
    inline QT3_SUPPORT bool find(const QString &exp, bool cs, bool wo)
    {
        QTextDocument::FindFlags flags = 0;
        if (cs)
            flags |= QTextDocument::FindCaseSensitively;
        if (wo)
            flags |= QTextDocument::FindWholeWords;
        return find(exp, flags);
    }

    inline QT3_SUPPORT void sync() {}

    QT3_SUPPORT void moveCursor(CursorAction action, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    QT3_SUPPORT void moveCursor(CursorAction action, bool select);

    enum KeyboardAction {
        ActionBackspace,
        ActionDelete,
        ActionReturn,
        ActionKill,
        ActionWordBackspace,
        ActionWordDelete
    };

    QT3_SUPPORT void doKeyboardAction(KeyboardAction action);

    QT3_SUPPORT void setText(const QString &text);
    QT3_SUPPORT QString text() const;
    QT3_SUPPORT void setTextFormat(Qt::TextFormat);
    QT3_SUPPORT Qt::TextFormat textFormat() const;

    inline QT3_SUPPORT void setBold(bool b) { setFontWeight(b ? QFont::Bold : QFont::Normal); }
    inline QT3_SUPPORT void setUnderline(bool b) { setFontUnderline(b); }
    inline QT3_SUPPORT void setItalic(bool i) { setFontItalic(i); }
    inline QT3_SUPPORT void setFamily(const QString &family) { setFontFamily(family); }
    inline QT3_SUPPORT void setPointSize(int size) { setFontPointSize(size); }

    inline QT3_SUPPORT bool italic() const { return fontItalic(); }
    inline QT3_SUPPORT bool bold() const { return fontWeight() >= QFont::Bold; }
    inline QT3_SUPPORT bool underline() const { return fontUnderline(); }
    inline QT3_SUPPORT QString family() const { return fontFamily(); }
    inline QT3_SUPPORT int pointSize() const { return (int)(fontPointSize()+0.5); }

    inline QT3_SUPPORT bool hasSelectedText() const
    { return textCursor().hasSelection(); }
    inline QT3_SUPPORT QString selectedText() const
    { return textCursor().selectedText(); }

    inline QT3_SUPPORT bool isUndoAvailable() const
    { return document()->isUndoAvailable(); }
    inline QT3_SUPPORT bool isRedoAvailable() const
    { return document()->isRedoAvailable(); }

    inline QT3_SUPPORT void insert(const QString &text)
    { insertPlainText(text); }

    inline QT3_SUPPORT bool isModified() const
    { return document()->isModified(); }

    inline QT3_SUPPORT QColor color() const
    { return textColor(); }

public Q_SLOTS:
    inline QT_MOC_COMPAT void setModified(bool m = true)
    { document()->setModified(m); }
public:
    inline QT3_SUPPORT void undo() const
    { document()->undo(); }
    inline QT3_SUPPORT void redo() const
    { document()->redo(); }

public Q_SLOTS:
    inline QT_MOC_COMPAT void setColor(const QColor &c)
    { setTextColor(c); }

#endif

private:
    Q_DISABLE_COPY(QTextEdit)
    Q_PRIVATE_SLOT(d_func(), void _q_repaintContents(const QRectF &r))
    Q_PRIVATE_SLOT(d_func(), void _q_currentCharFormatChanged(const QTextCharFormat &))
    Q_PRIVATE_SLOT(d_func(), void _q_adjustScrollbars())
    Q_PRIVATE_SLOT(d_func(), void _q_ensureVisible(const QRectF &))
    friend class QTextEditControl;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextEdit::AutoFormatting)

#endif // QT_NO_TEXTEDIT

QT_END_HEADER

#endif // QTEXTEDIT_H
