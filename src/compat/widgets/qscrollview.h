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
#ifndef QSCROLLVIEW_H
#define QSCROLLVIEW_H

#include "qframe.h"
#include "qscrollbar.h"

#ifndef QT_NO_SCROLLVIEW

class QScrollViewPrivate;

class Q_COMPAT_EXPORT QScrollView : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScrollView)
    Q_ENUMS(ResizePolicy ScrollBarMode)
    Q_PROPERTY(ResizePolicy resizePolicy READ resizePolicy WRITE setResizePolicy)
    Q_PROPERTY(ScrollBarMode vScrollBarMode READ vScrollBarMode WRITE setVScrollBarMode)
    Q_PROPERTY(ScrollBarMode hScrollBarMode READ hScrollBarMode WRITE setHScrollBarMode)
    Q_PROPERTY(int visibleWidth READ visibleWidth)
    Q_PROPERTY(int visibleHeight READ visibleHeight)
    Q_PROPERTY(int contentsWidth READ contentsWidth)
    Q_PROPERTY(int contentsHeight READ contentsHeight)
    Q_PROPERTY(int contentsX READ contentsX)
    Q_PROPERTY(int contentsY READ contentsY)
#ifndef QT_NO_DRAGANDDROP
    Q_PROPERTY(bool dragAutoScroll READ dragAutoScroll WRITE setDragAutoScroll)
#endif

public:
    QScrollView(QWidget* parent=0, const char* name=0, Qt::WFlags f=0);
    ~QScrollView();

    enum ResizePolicy { Default, Manual, AutoOne, AutoOneFit };
    virtual void setResizePolicy(ResizePolicy);
    ResizePolicy resizePolicy() const;

    void changeEvent(QEvent *);
    void removeChild(QWidget* child);
    virtual void addChild(QWidget* child, int x=0, int y=0);
    virtual void moveChild(QWidget* child, int x, int y);
    int childX(QWidget* child);
    int childY(QWidget* child);
    bool childIsVisible(QWidget* child) { return child->isVisible(); } // obsolete functions
    void showChild(QWidget* child, bool yes=true) {
        if (yes)
            child->show();
        else
            child->hide();
    }

    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };

    ScrollBarMode vScrollBarMode() const;
    virtual void  setVScrollBarMode(ScrollBarMode);

    ScrollBarMode hScrollBarMode() const;
    virtual void  setHScrollBarMode(ScrollBarMode);

    QWidget*     cornerWidget() const;
    virtual void setCornerWidget(QWidget*);

    // ### 4.0: Consider providing a factory function for scrollbars
    //          (e.g. make the two following functions virtual)
    QScrollBar*  horizontalScrollBar() const;
    QScrollBar*  verticalScrollBar() const;
    QWidget*         viewport() const;
    QWidget*         clipper() const;

    int                visibleWidth() const;
    int                visibleHeight() const;

    int                contentsWidth() const;
    int                contentsHeight() const;
    int                contentsX() const;
    int                contentsY() const;

    void        show();

    void        updateContents(int x, int y, int w, int h);
    void        updateContents(const QRect& r);
    void         updateContents();
    void        repaintContents(int x, int y, int w, int h);
    void        repaintContents(const QRect& r);
    void         repaintContents();
    void        contentsToViewport(int x, int y, int& vx, int& vy) const;
    void        viewportToContents(int vx, int vy, int& x, int& y) const;
    QPoint        contentsToViewport(const QPoint&) const;
    QPoint        viewportToContents(const QPoint&) const;
    void        enableClipper(bool y);

    void        setStaticBackground(bool y);
    bool        hasStaticBackground() const;

    QSize        viewportSize(int, int) const;
    QSize        sizeHint() const;
    QSize        minimumSizeHint() const;

    bool        isHorizontalSliderPressed();
    bool        isVerticalSliderPressed();

#ifndef QT_NO_DRAGANDDROP
    virtual void setDragAutoScroll(bool b);
    bool         dragAutoScroll() const;
#endif

signals:
    void        contentsMoving(int x, int y);
    void        horizontalSliderPressed();
    void        horizontalSliderReleased();
    void        verticalSliderPressed();
    void        verticalSliderReleased();

public slots:
    virtual void resizeContents(int w, int h);
    void        scrollBy(int dx, int dy);
    virtual void setContentsPos(int x, int y);
    void        ensureVisible(int x, int y);
    void        ensureVisible(int x, int y, int xmargin, int ymargin);
    void        center(int x, int y);
    void        center(int x, int y, float xmargin, float ymargin);

    void        updateScrollBars(); // ### virtual in 4.0

protected:
    QScrollView(QScrollViewPrivate &, QWidget* parent=0, const char* name=0, Qt::WFlags f=0);
    virtual void drawContents(QPainter*, int cx, int cy, int cw, int ch);
    virtual void drawContentsOffset(QPainter*, int ox, int oy,
                    int cx, int cy, int cw, int ch);


    virtual void contentsMousePressEvent(QMouseEvent*);
    virtual void contentsMouseReleaseEvent(QMouseEvent*);
    virtual void contentsMouseDoubleClickEvent(QMouseEvent*);
    virtual void contentsMouseMoveEvent(QMouseEvent*);
#ifndef QT_NO_DRAGANDDROP
    virtual void contentsDragEnterEvent(QDragEnterEvent *);
    virtual void contentsDragMoveEvent(QDragMoveEvent *);
    virtual void contentsDragLeaveEvent(QDragLeaveEvent *);
    virtual void contentsDropEvent(QDropEvent *);
#endif
#ifndef QT_NO_WHEELEVENT
    virtual void contentsWheelEvent(QWheelEvent *);
#endif
    virtual void contentsContextMenuEvent(QContextMenuEvent *);


    virtual void viewportPaintEvent(QPaintEvent*);
    virtual void viewportResizeEvent(QResizeEvent*);
    virtual void viewportMousePressEvent(QMouseEvent*);
    virtual void viewportMouseReleaseEvent(QMouseEvent*);
    virtual void viewportMouseDoubleClickEvent(QMouseEvent*);
    virtual void viewportMouseMoveEvent(QMouseEvent*);
#ifndef QT_NO_DRAGANDDROP
    virtual void viewportDragEnterEvent(QDragEnterEvent *);
    virtual void viewportDragMoveEvent(QDragMoveEvent *);
    virtual void viewportDragLeaveEvent(QDragLeaveEvent *);
    virtual void viewportDropEvent(QDropEvent *);
#endif
#ifndef QT_NO_WHEELEVENT
    virtual void viewportWheelEvent(QWheelEvent *);
#endif
    virtual void viewportContextMenuEvent(QContextMenuEvent *);

    virtual void setMargins(int left, int top, int right, int bottom);
    int leftMargin() const;
    int topMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

    bool focusNextPrevChild(bool next);

    virtual void setHBarGeometry(QScrollBar& hbar, int x, int y, int w, int h);
    virtual void setVBarGeometry(QScrollBar& vbar, int x, int y, int w, int h);

    void resizeEvent(QResizeEvent*);
    void  mousePressEvent(QMouseEvent *);
    void  mouseReleaseEvent(QMouseEvent *);
    void  mouseDoubleClickEvent(QMouseEvent *);
    void  mouseMoveEvent(QMouseEvent *);
#ifndef QT_NO_WHEELEVENT
    void  wheelEvent(QWheelEvent *);
#endif
    void contextMenuEvent(QContextMenuEvent *);
    bool eventFilter(QObject *, QEvent *e);

    void setCachedSizeHint(const QSize &sh) const;
    QSize cachedSizeHint() const;

private:
    void moveContents(int x, int y);

private slots:
    void hslide(int);
    void vslide(int);
    void hbarIsPressed();
    void hbarIsReleased();
    void vbarIsPressed();
    void vbarIsReleased();
#ifndef QT_NO_DRAGANDDROP
    void doDragAutoScroll();
    void startDragAutoScroll();
    void stopDragAutoScroll();
#endif

private:
    Q_DISABLE_COPY(QScrollView)

    void changeFrameRect(const QRect&);

public:
    void disableSizeHintCaching();

    void repaintContents(int x, int y, int w, int h, bool) {repaintContents(x, y, w, h); }
    void repaintContents(const QRect& r, bool) { repaintContents(r); }
    void repaintContents(bool) { repaintContents(); }
};

#endif // QT_NO_SCROLLVIEW

#endif // QSCROLLVIEW_H
