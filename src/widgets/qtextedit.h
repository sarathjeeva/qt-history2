/****************************************************************************
**
** Definition of the QTextEdit class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstylesheet.h"
#include "qmap.h"
#endif // QT_H

#ifndef QT_NO_TEXTEDIT
// uncomment below to enable optimization mode - also uncomment the
// optimDoAutoScroll() private slot since moc ignores #ifdefs..
#define QT_TEXTEDIT_OPTIMIZATION

class QPainter;
class Q3TextDocument;
class Q3TextCursor;
class QKeyEvent;
class QResizeEvent;
class QMouseEvent;
class QTimer;
class Q3TextString;
class QTextCommand;
class Q3TextParagraph;
class Q3TextFormat;
class QFont;
class QColor;
class QTextEdit;
class QTextBrowser;
class Q3TextString;
struct QUndoRedoInfoPrivate;
class QPopupMenu;
class QTextEditPrivate;
class QSyntaxHighlighter;

#ifdef QT_TEXTEDIT_OPTIMIZATION
class QTextEditOptimPrivate
{
public:
    // Note: no left-tag has any value for leftTag or parent, and
    // no right-tag has any formatting flags set.
    enum TagType { Color = 0, Format = 1 };
    struct Tag {
	TagType type:2;
	bool bold:1;
	bool italic:1;
	bool underline:1;
	int line;
	int index;
	Tag * leftTag; // ptr to left-tag in a left-right tag pair
	Tag * parent;  // ptr to parent left-tag in a nested tag
	Tag * prev;
	Tag * next;
	QString tag;
    };
    QTextEditOptimPrivate()
    {
	len = numLines = maxLineWidth = 0;
	selStart.line = selStart.index = -1;
	selEnd.line = selEnd.index = -1;
	search.line = search.index = 0;
	tags = lastTag = 0;
    }
    void clearTags()
    {
	Tag * itr = tags;
	while ( tags ) {
	    itr  = tags;
	    tags = tags->next;
	    delete itr;
	}
	tags = lastTag = 0;
	tagIndex.clear();
    }
    ~QTextEditOptimPrivate()
    {
	clearTags();
    }
    int len;
    int numLines;
    int maxLineWidth;
    struct Selection {
	int line;
	int index;
    };
    Selection selStart, selEnd, search;
    Tag * tags, * lastTag;
    QMap<int, QString> lines;
    QMap<int, Tag *> tagIndex;
};
#endif

class Q_GUI_EXPORT QTextEdit : public QScrollView
{
    friend class QTextBrowser;
    friend class QSyntaxHighlighter;

    Q_OBJECT
    Q_ENUMS( WordWrap WrapPolicy )
    Q_FLAGS( AutoFormattingFlags )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QBrush paper READ paper WRITE setPaper )
    Q_PROPERTY( bool linkUnderline READ linkUnderline WRITE setLinkUnderline )
    Q_PROPERTY( QString documentTitle READ documentTitle )
    Q_PROPERTY( int length READ length )
    Q_PROPERTY( WordWrap wordWrap READ wordWrap WRITE setWordWrap )
    Q_PROPERTY( int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth )
    Q_PROPERTY( WrapPolicy wrapPolicy READ wrapPolicy WRITE setWrapPolicy )
    Q_PROPERTY( bool hasSelectedText READ hasSelectedText )
    Q_PROPERTY( QString selectedText READ selectedText )
    Q_PROPERTY( int undoDepth READ undoDepth WRITE setUndoDepth )
    Q_PROPERTY( bool overwriteMode READ isOverwriteMode WRITE setOverwriteMode )
    Q_PROPERTY( bool modified READ isModified WRITE setModified DESIGNABLE false )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled )
    Q_PROPERTY( int tabStopWidth READ tabStopWidth WRITE setTabStopWidth )
    Q_PROPERTY( bool tabChangesFocus READ tabChangesFocus WRITE setTabChangesFocus )
    Q_PROPERTY( AutoFormattingFlags autoFormatting READ autoFormatting WRITE setAutoFormatting )

public:
    enum WordWrap {
	NoWrap,
	WidgetWidth,
	FixedPixelWidth,
	FixedColumnWidth
    };

    enum WrapPolicy {
	AtWordBoundary,
	AtWhiteSpace = AtWordBoundary, // AtWhiteSpace is deprecated
	Anywhere,
	AtWordOrDocumentBoundary
    };

    enum AutoFormattingFlags {
	AutoNone = 0,
	AutoBulletList = 0x00000001,
	AutoAll = 0xffffffff
    };

    Q_DECLARE_FLAGS(AutoFormatting, AutoFormattingFlags);

    enum KeyboardAction {
	ActionBackspace,
	ActionDelete,
	ActionReturn,
	ActionKill,
	ActionWordBackspace,
	ActionWordDelete
    };

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
	MovePgUp,
	MovePgDown
    };

    enum VerticalAlignment {
	AlignNormal,
	AlignSuperScript,
	AlignSubScript
    };

    enum TextInsertionFlags {
	RedoIndentation = 0x0001,
	CheckNewLines = 0x0002,
	RemoveSelected = 0x0004
    };

    QTextEdit( const QString& text, const QString& context = QString::null,
	       QWidget* parent=0, const char* name=0);
    QTextEdit( QWidget* parent=0, const char* name=0 );
    virtual ~QTextEdit();
    void setPalette( const QPalette & );

    QString text() const;
    QString text( int para ) const;
    TextFormat textFormat() const;
    QString context() const;
    QString documentTitle() const;

    void getSelection( int *paraFrom, int *indexFrom,
		    int *paraTo, int *indexTo, int selNum = 0 ) const;
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
		       int *para = 0, int *index = 0 );

    int paragraphs() const;
    int lines() const;
    int linesOfParagraph( int para ) const;
    int lineOfChar( int para, int chr );
    int length() const;
    QRect paragraphRect( int para ) const;
    int paragraphAt( const QPoint &pos ) const;
    int charAt( const QPoint &pos, int *para ) const;
    int paragraphLength( int para ) const;

    QStyleSheet* styleSheet() const;
#ifndef QT_NO_MIME
    QMimeSourceFactory* mimeSourceFactory() const;
#endif
    QBrush paper() const;
    bool linkUnderline() const;

    int heightForWidth( int w ) const;

    bool hasSelectedText() const;
    QString selectedText() const;
    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    WordWrap wordWrap() const;
    int wrapColumnOrWidth() const;
    WrapPolicy wrapPolicy() const;

    int tabStopWidth() const;

    QString anchorAt( const QPoint& pos, AnchorAttribute a = AnchorHref );

    QSize sizeHint() const;

    bool isReadOnly() const { return readonly; }

    void getCursorPosition( int *parag, int *index ) const;

    bool isModified() const;
    bool italic() const;
    bool bold() const;
    bool underline() const;
    QString family() const;
    int pointSize() const;
    QColor color() const;
    QFont font() const;
    QFont currentFont() const;
    int alignment() const;
    int undoDepth() const;

    // do not use, will go away
    virtual bool getFormat( int para, int index, QFont *font, QColor *color, VerticalAlignment *verticalAlignment );
    // do not use, will go away
    virtual bool getParagraphFormat( int para, QFont *font, QColor *color,
				     VerticalAlignment *verticalAlignment, int *alignment,
				     QStyleSheetItem::DisplayMode *displayMode,
				     QStyleSheetItem::ListStyle *listStyle,
				     int *listDepth );


    bool isOverwriteMode() const { return overWrite; }
    QColor paragraphBackgroundColor( int para ) const;

    bool isUndoRedoEnabled() const;
    bool eventFilter( QObject *o, QEvent *e );
    bool tabChangesFocus() const;

    void setAutoFormatting( AutoFormatting );
    AutoFormatting autoFormatting() const;
    QSyntaxHighlighter *syntaxHighlighter() const;

public slots:
    void setEnabled( bool );
#ifndef QT_NO_MIME
    virtual void setMimeSourceFactory( QMimeSourceFactory* factory );
#endif
    virtual void setStyleSheet( QStyleSheet* styleSheet );
    virtual void scrollToAnchor( const QString& name );
    virtual void setPaper( const QBrush& pap );
    virtual void setLinkUnderline( bool );

    virtual void setWordWrap( WordWrap mode );
    virtual void setWrapColumnOrWidth( int );
    virtual void setWrapPolicy( WrapPolicy policy );

    virtual void copy();
    virtual void append( const QString& text );

    void setText( const QString &txt ) { setText( txt, QString::null ); }
    virtual void setText( const QString &txt, const QString &context );
    virtual void setTextFormat( TextFormat f );

    virtual void selectAll( bool select = TRUE );
    virtual void setTabStopWidth( int ts );
    virtual void zoomIn( int range );
    virtual void zoomIn() { zoomIn( 1 ); }
    virtual void zoomOut( int range );
    virtual void zoomOut() { zoomOut( 1 ); }
    virtual void zoomTo( int size );

    virtual void sync();
    virtual void setReadOnly( bool b );

    virtual void undo();
    virtual void redo();
    virtual void cut();
    virtual void paste();
#ifndef QT_NO_CLIPBOARD
    virtual void pasteSubType( const QByteArray &subtype );
#endif
    virtual void clear();
    virtual void del();
    virtual void indent();
    virtual void setItalic( bool b );
    virtual void setBold( bool b );
    virtual void setUnderline( bool b );
    virtual void setFamily( const QString &f );
    virtual void setPointSize( int s );
    virtual void setColor( const QColor &c );
    virtual void setFont( const QFont &f );
    virtual void setVerticalAlignment( VerticalAlignment a );
    virtual void setAlignment( int a );

    // do not use, will go away
    virtual void setParagType( QStyleSheetItem::DisplayMode dm, QStyleSheetItem::ListStyle listStyle );

    virtual void setCursorPosition( int parag, int index );
    virtual void setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum = 0 );
    virtual void setSelectionAttributes( int selNum, const QColor &back, bool invertText );
    virtual void setModified( bool m );
    virtual void resetFormat();
    virtual void setUndoDepth( int d );
    virtual void setFormat( Q3TextFormat *f, int flags );
    virtual void ensureCursorVisible();
    virtual void placeCursor( const QPoint &pos, Q3TextCursor *c = 0 );
    virtual void moveCursor( CursorAction action, bool select );
    virtual void doKeyboardAction( KeyboardAction action );
    virtual void removeSelectedText( int selNum = 0 );
    virtual void removeSelection( int selNum = 0 );
    virtual void setCurrentFont( const QFont &f );
    virtual void setOverwriteMode( bool b ) { overWrite = b; }

    virtual void scrollToBottom();

    virtual void insert( const QString &text, uint insertionFlags = CheckNewLines | RemoveSelected );

    // obsolete
    virtual void insert( const QString &text, bool, bool = TRUE, bool = TRUE );

    virtual void insertAt( const QString &text, int para, int index );
    virtual void removeParagraph( int para );
    virtual void insertParagraph( const QString &text, int para );

    virtual void setParagraphBackgroundColor( int para, const QColor &bg );
    virtual void clearParagraphBackground( int para );

    virtual void setUndoRedoEnabled( bool b );
    virtual void setTabChangesFocus( bool b );

#ifdef QT_TEXTEDIT_OPTIMIZATION
    void polishEvent(QEvent*);
    void setMaxLogLines( int numLines );
    int maxLogLines() const;
#endif

signals:
    void textChanged();
    void selectionChanged();
    void copyAvailable( bool );
    void undoAvailable( bool yes );
    void redoAvailable( bool yes );
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int a );
    void currentVerticalAlignmentChanged( VerticalAlignment a );
    void cursorPositionChanged( Q3TextCursor *c );
    void cursorPositionChanged( int para, int pos );
    void returnPressed();
    void modificationChanged( bool m );
    void clicked( int parag, int index );
    void doubleClicked( int parag, int index );

protected:
    void repaintChanged();
    void updateStyles();
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    bool event( QEvent *e );
    void changeEvent( QEvent * );
    void keyPressEvent( QKeyEvent *e );
    void resizeEvent( QResizeEvent *e );
    void viewportResizeEvent( QResizeEvent* );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
#ifndef QT_NO_WHEELEVENT
    void contentsWheelEvent( QWheelEvent *e );
#endif
    void imStartEvent( QIMEvent * );
    void imComposeEvent( QIMEvent * );
    void imEndEvent( QIMEvent * );
#ifndef QT_NO_DRAGANDDROP
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );
    void contentsDragLeaveEvent( QDragLeaveEvent *e );
    void contentsDropEvent( QDropEvent *e );
#endif
    void contentsContextMenuEvent( QContextMenuEvent *e );
    bool focusNextPrevChild( bool next );
    Q3TextDocument *document() const;
    Q3TextCursor *textCursor() const;
    void setDocument( Q3TextDocument *doc );
    virtual QPopupMenu *createPopupMenu( const QPoint& pos );
    virtual QPopupMenu *createPopupMenu();
    void drawCursor( bool visible );

protected slots:
    virtual void doChangeInterval();
    virtual void sliderReleased();

private slots:
    void formatMore();
    void doResize();
    void autoScrollTimerDone();
    void blinkCursor();
    void setModified();
    void startDrag();
    void documentWidthChanged( int w );
    void clipboardChanged();

private:
    struct Q_GUI_EXPORT UndoRedoInfo {
	enum Type { Invalid, Insert, Delete, Backspace, Return, RemoveSelected, Format, Style };

	UndoRedoInfo( Q3TextDocument *dc );
	~UndoRedoInfo();
	void clear();
	bool valid() const;

	QUndoRedoInfoPrivate *d;
	int id;
	int index;
	int eid;
	int eindex;
	Q3TextFormat *format;
	int flags;
	Type type;
	Q3TextDocument *doc;
	QByteArray styleInformation;
    };

private:
    void updateCursor( const QPoint & pos );
    void handleMouseMove( const QPoint& pos );
    void drawContents( QPainter * );
    virtual bool linksEnabled() const { return FALSE; }
    void init();
    void checkUndoRedoInfo( UndoRedoInfo::Type t );
    void updateCurrentFormat();
    bool handleReadOnlyKeyEvent( QKeyEvent *e );
    void makeParagVisible( Q3TextParagraph *p );
    void normalCopy();
    void copyToClipboard();
#ifndef QT_NO_MIME
    QByteArray pickSpecial(QMimeSource* ms, bool always_ask, const QPoint&);
    QTextDrag *dragObject( QWidget *parent = 0 ) const;
#endif
#ifndef QT_NO_MIMECLIPBOARD
    void pasteSpecial(const QPoint&);
#endif
    void setFontInternal( const QFont &f );

    virtual void emitHighlighted( const QString & ) {}
    virtual void emitLinkClicked( const QString & ) {}

    void readFormats( Q3TextCursor &c1, Q3TextCursor &c2, Q3TextString &text, bool fillStyles = FALSE );
    void clearUndoRedo();
    void paintDocument( bool drawAll, QPainter *p, int cx = -1, int cy = -1, int cw = -1, int ch = -1 );
    void moveCursor( CursorAction action );
    void ensureFormatted( Q3TextParagraph *p );
    void placeCursor( const QPoint &pos, Q3TextCursor *c, bool link );
    void updateMicroFocusHint();

#ifdef QT_TEXTEDIT_OPTIMIZATION
    bool checkOptimMode();
    QString optimText() const;
    void optimSetText( const QString &str );
    void optimAppend( const QString &str );
    void optimInsert( const QString &str, int line, int index );
    void optimDrawContents( QPainter * p, int cx, int cy, int cw, int ch );
    void optimMousePressEvent( QMouseEvent * e );
    void optimMouseReleaseEvent( QMouseEvent * e );
    void optimMouseMoveEvent( QMouseEvent * e );
    int  optimCharIndex( const QString &str, int mx ) const;
    void optimSelectAll();
    void optimRemoveSelection();
    void optimSetSelection( int startLine, int startIdx, int endLine,
			    int endIdx );
    bool optimHasSelection() const;
    QString optimSelectedText() const;
    bool optimFind( const QString & str, bool, bool, bool, int *, int * );
    void optimParseTags( QString * str, int lineNo = -1, int indexOffset = 0 );
    QTextEditOptimPrivate::Tag * optimPreviousLeftTag( int line );
    void optimSetTextFormat( Q3TextDocument *, Q3TextCursor *, Q3TextFormat * f,
			     int, int, QTextEditOptimPrivate::Tag * t );
    QTextEditOptimPrivate::Tag * optimAppendTag( int index, const QString & tag );
    QTextEditOptimPrivate::Tag * optimInsertTag( int line, int index, const QString & tag );
    void optimCheckLimit( const QString& str );

private slots:
    void optimDoAutoScroll();
#endif // QT_TEXTEDIT_OPTIMIZATION

private:
#ifndef QT_NO_CLIPBOARD
    void pasteSubType( const QByteArray &subtype, QMimeSource *m );
#endif

private:
    Q3TextDocument *doc;
    Q3TextCursor *cursor;
    QTimer *formatTimer, *scrollTimer, *changeIntervalTimer, *blinkTimer, *dragStartTimer;
    Q3TextParagraph *lastFormatted;
    int interval;
    UndoRedoInfo undoRedoInfo;
    Q3TextFormat *currentFormat;
    int currentAlignment;
    QPoint oldMousePos, mousePos;
    QPoint dragStartPos;
    QString onLink;
    WordWrap wrapMode;
    WrapPolicy wPolicy;
    int wrapWidth;
    QString pressedLink;
    QTextEditPrivate *d;
    bool inDoubleClick : 1;
    bool mousePressed : 1;
    bool cursorVisible : 1;
    bool blinkCursorVisible : 1;
    bool readOnly : 1;
    bool modified : 1;
    bool mightStartDrag : 1;
    bool inDnD : 1;
    bool readonly : 1;
    bool undoEnabled : 1;
    bool overWrite : 1;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextEdit( const QTextEdit & );
    QTextEdit &operator=( const QTextEdit & );
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextEdit::AutoFormatting)

inline Q3TextDocument *QTextEdit::document() const
{
    return doc;
}

inline Q3TextCursor *QTextEdit::textCursor() const
{
    return cursor;
}

inline void QTextEdit::setCurrentFont( const QFont &f )
{
    QTextEdit::setFontInternal( f );
}

#endif //QT_NO_TEXTEDIT
#endif //QTEXTVIEW_H
