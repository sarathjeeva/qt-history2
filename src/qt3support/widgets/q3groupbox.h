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

#ifndef Q3GROUPBOX_H
#define Q3GROUPBOX_H

#include <QtGui/qgroupbox.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3GroupBoxPrivate;

class Q_COMPAT_EXPORT Q3GroupBox : public QGroupBox
{
    Q_OBJECT
public:
    enum
#if defined(Q_MOC_RUN)
    FrameShape
#else
    DummyFrame
#endif
    {   Box = QFrame::Box, Sunken = QFrame::Sunken, Plain = QFrame::Plain,
        Raised = QFrame::Raised, MShadow=QFrame::Shadow_Mask, NoFrame = QFrame::NoFrame,
        Panel = QFrame::Panel, StyledPanel = QFrame::StyledPanel, HLine = QFrame::HLine,
        VLine = QFrame::VLine,
        WinPanel = QFrame::WinPanel,ToolBarPanel = QFrame::StyledPanel,
        MenuBarPanel = QFrame::StyledPanel, PopupPanel = QFrame::StyledPanel,
        LineEditPanel = QFrame::StyledPanel,TabWidgetPanel = QFrame::StyledPanel,
        GroupBoxPanel = 0x0007,
        MShape = QFrame::Shape_Mask};

    typedef DummyFrame FrameShape;
    Q_ENUMS(FrameShape)

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false)
    Q_PROPERTY(int columns READ columns WRITE setColumns DESIGNABLE false)

    Q_PROPERTY(QRect frameRect READ frameRect WRITE setFrameRect DESIGNABLE false)
    Q_PROPERTY(FrameShape frameShape READ frameShape WRITE setFrameShape)
    Q_PROPERTY(FrameShape frameShadow READ frameShadow WRITE setFrameShadow)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
    Q_PROPERTY(int midLineWidth READ midLineWidth WRITE setMidLineWidth)
    Q_PROPERTY(int margin READ margin WRITE setMargin)

public:
    explicit Q3GroupBox(QWidget* parent=0, const char* name=0);
    explicit Q3GroupBox(const QString &title,
	       QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o,
	       QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o, const QString &title,
	       QWidget* parent=0, const char* name=0);
    ~Q3GroupBox();

    virtual void setColumnLayout(int strips, Qt::Orientation o);

    int columns() const;
    void setColumns(int);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    int insideMargin() const;
    int insideSpacing() const;
    void setInsideMargin(int m);
    void setInsideSpacing(int s);

    void addSpace(int);

    void setFrameRect(QRect);
    QRect frameRect() const;
#ifdef qdoc
    void setFrameShadow(FrameShape);
    FrameShape frameShadow() const;
    void setFrameShape(FrameShape);
    FrameShape frameShape() const;
#else
    void setFrameShadow(DummyFrame);
    DummyFrame frameShadow() const;
    void setFrameShape(DummyFrame);
    DummyFrame frameShape() const;
#endif
    void setFrameStyle(int);
    int frameStyle() const;
    int frameWidth() const;
    void setLineWidth(int);
    int lineWidth() const;
    void setMargin(int margin) { setContentsMargins(margin, margin, margin, margin); }
    int margin() const
    { int margin; int dummy; getContentsMargins(&margin, &dummy, &dummy, &dummy);  return margin; }
    void setMidLineWidth(int);
    int midLineWidth() const;

protected:
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *);
    bool event(QEvent *);

private:
    void skip();
    void init();
    void calculateFrame();
    void insertWid(QWidget*);
    void drawFrame(QPainter *p);

    Q3GroupBoxPrivate * d;

    Q_DISABLE_COPY(Q3GroupBox)
};

QT_END_HEADER

#endif // Q3GROUPBOX_H
