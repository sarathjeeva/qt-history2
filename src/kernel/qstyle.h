/****************************************************************************
** $Id: $
**
** Definition of QStyle class
**
** Created : 980616
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qobject.h"
#endif // QT_H


#ifndef QT_NO_STYLE

class QPopupMenu;
class QStylePrivate;

class Q_EXPORT QStyle: public QObject
{
    Q_OBJECT
    GUIStyle gs;

private:
    QStyle(GUIStyle);
    QStyle();
    friend class QCommonStyle;

public:
    virtual ~QStyle();

    // New QStyle API - most of these should probably be pure virtual

    virtual void polish( QWidget * );
    virtual void unPolish( QWidget * );

    virtual void polish( QApplication * );
    virtual void unPolish( QApplication * );

    virtual void polish( QPalette & );

    virtual void polishPopupMenu( QPopupMenu* ) = 0;

    virtual QRect itemRect( QPainter *p, const QRect &r,
			    int flags, bool enabled,
			    const QPixmap *pixmap,
			    const QString &text, int len = -1 ) const;

    virtual void drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString &text,
			   int len = -1, const QColor *penColor = 0 ) const;


    enum PrimitiveElement {
	PE_ButtonCommand,
	PE_ButtonBevel,
	PE_ButtonTool,
	PE_ButtonDropDown,

	PE_FocusRect,

	PE_ArrowUp,
	PE_ArrowDown,
	PE_ArrowRight,
	PE_ArrowLeft,

	PE_SpinWidgetUp,
	PE_SpinWidgetDown,
	PE_SpinWidgetPlus,
	PE_SpinWidgetMinus,

	PE_Indicator,
	PE_IndicatorMask,
	PE_ExclusiveIndicator,
	PE_ExclusiveIndicatorMask,

	PE_DockWindowHandle,
	PE_DockWindowSeparator,
	PE_DockWindowResizeHandle,

	PE_Splitter,

	PE_Panel,
	PE_PanelPopup,
	PE_PanelMenuBar,
	PE_PanelDockWindow,

	PE_TabBarBase,

	PE_HeaderSection,
	PE_HeaderArrow,
	PE_StatusBarSection,

	PE_GroupBoxFrame,

	PE_Separator,

	PE_SizeGrip,

	PE_CheckMark,

	PE_ScrollBarAddLine,
	PE_ScrollBarSubLine,
	PE_ScrollBarAddPage,
	PE_ScrollBarSubPage,
	PE_ScrollBarSlider,
	PE_ScrollBarFirst,
	PE_ScrollBarLast,

	PE_ProgressBarChunk
    };

    enum PrimitiveElementFlags {
	PStyle_Default = 		0x00000000,
	PStyle_Enabled = 		0x00000001,
	PStyle_Raised =			0x00000002,
	PStyle_Sunken = 		0x00000004,
	PStyle_Off =			0x00000008,
	PStyle_NoChange =		0x00000010,
	PStyle_On =			0x00000020,
	PStyle_Down =			0x00000040,
	PStyle_Horizontal =		0x00000080,
	PStyle_Vertical =		0x00000100,
	PStyle_HasFocus =		0x00000200,
	PStyle_Top =			0x00000400,
	PStyle_Bottom =			0x00000800,
	PStyle_FocusAtBorder =		0x00001000,
	PStyle_AutoRaise =		0x00002000,
	PStyle_MouseOver =		0x00004000,
	PStyle_Up =                     0x00008000

	/*
	  PStyle_FocusHighlight=	0x00000001,
	*/
    };
    typedef uint PFlags;

    virtual void drawPrimitive( PrimitiveElement op,
				QPainter *p,
				const QRect &r,
				const QColorGroup &cg,
				PFlags flags = PStyle_Default,
				void **data = 0 ) const = 0;


    enum ControlElement {
	CE_PushButton,
	CE_PushButtonLabel,

	CE_CheckBox,
	CE_CheckBoxLabel,

	CE_RadioButton,
	CE_RadioButtonLabel,

	CE_TabBarTab,
	CE_TabBarLabel,

	CE_ProgressBarGroove,
	CE_ProgressBarContents,
	CE_ProgressBarLabel,

	CE_PopupMenuItem,
	CE_MenuBarItem
    };

    enum ControlElementFlags{
	CStyle_Default = 		0x00000000,
	CStyle_Selected =	 	0x00000001,
	CStyle_HasFocus =		0x00000002,
	CStyle_Active =			0x00000004
    };
    typedef uint CFlags;

    virtual void drawControl( ControlElement element,
			      QPainter *p,
			      const QWidget *widget,
			      const QRect &r,
			      const QColorGroup &cg,
			      CFlags how = CStyle_Default,
			      void **data = 0 ) const = 0;
    virtual void drawControlMask( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  void **data = 0 ) const = 0;

    enum SubRect {
	SR_PushButtonContents,
	SR_PushButtonFocusRect,

	SR_CheckBoxIndicator,
	SR_CheckBoxContents,
	SR_CheckBoxFocusRect,

	SR_RadioButtonIndicator,
	SR_RadioButtonContents,
	SR_RadioButtonFocusRect,

	SR_ComboBoxFocusRect,

	SR_SliderFocusRect,

	SR_DockWindowHandleRect,

	SR_ProgressBarGroove,
	SR_ProgressBarContents,
	SR_ProgressBarLabel

	/*
	  SR_DefaultFrameContents,
	  SR_PopupFrameContents,
	  SR_MenuFrameContents,
	*/
    };

    virtual QRect subRect( SubRect r, const QWidget *widget ) const = 0;


    enum ComplexControl{
	CC_SpinWidget,
	CC_ComboBox,
	CC_ScrollBar,
	CC_Slider,
	CC_ToolButton,
	CC_TitleBar,
	CC_ListView
    };

    enum SubControl {
	SC_None =			0x00000000,

	SC_ScrollBarAddLine =		0x00000001,
	SC_ScrollBarSubLine =		0x00000002,
	SC_ScrollBarAddPage =		0x00000004,
	SC_ScrollBarSubPage =		0x00000008,
	SC_ScrollBarFirst =		0x00000010,
	SC_ScrollBarLast =		0x00000020,
	SC_ScrollBarSlider =		0x00000040,
	SC_ScrollBarGroove =		0x00000080,

	SC_SpinWidgetUp =		0x00000001,
	SC_SpinWidgetDown =		0x00000002,
	SC_SpinWidgetFrame =		0x00000004,
	SC_SpinWidgetEditField =	0x00000008,
	SC_SpinWidgetButtonField =	0x00000010,

	SC_ComboBoxEditField =		0x00000001,
	SC_ComboBoxArrow =		0x00000002,

	SC_SliderGroove =		0x00000001,
	SC_SliderHandle = 		0x00000002,
	SC_SliderTickmarks = 		0x00000004,

	SC_ToolButton =			0x00000001,
	SC_ToolButtonMenu =		0x00000002,

	SC_TitleBarSysMenu =		0x00000001,
	SC_TitleBarMinButton =		0x00000002,
	SC_TitleBarMaxButton =		0x00000004,
	SC_TitleBarCloseButton =	0x00000008,
	SC_TitleBarLabel =		0x00000010,
	SC_TitleBarNormalButton =	0x00000020,
	SC_TitleBarShadeButton =	0x00000040,
	SC_TitleBarUnshadeButton =	0x00000080,

	SC_ListView =			0x00000001,
	SC_ListViewBranch =		0x00000002,
	SC_ListViewExpand =		0x00000004,

	SC_All =			0xffffffff
    };
    typedef uint SCFlags;


    virtual void drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     CFlags flags = CStyle_Default,
				     SCFlags sub = SC_All,
				     SCFlags subActive = SC_None,
				     void **data = 0 ) const = 0;
    virtual void drawComplexControlMask( ComplexControl control,
					 QPainter *p,
					 const QWidget *widget,
					 const QRect &r,
					 void **data = 0 ) const = 0;

    virtual QRect querySubControlMetrics( ComplexControl control,
					  const QWidget *widget,
					  SubControl sc,
					  void **data = 0 ) const = 0;
    virtual SubControl querySubControl( ComplexControl control,
					const QWidget *widget,
					const QPoint &pos,
					void **data = 0 ) const = 0;


    enum PixelMetric {
	PM_ButtonMargin,
	PM_ButtonDefaultIndicator,
	PM_MenuButtonIndicator,
	PM_ButtonShiftHorizontal,
	PM_ButtonShiftVertical,

	PM_DefaultFrameWidth,
	PM_SpinBoxFrameWidth,

	PM_MaximumDragDistance,

	PM_ScrollBarExtent,

	PM_SliderThickness,	       	// total slider thickness
	PM_SliderControlThickness,    	// thickness of the business part
	PM_SliderLength,		// total length of slider
	PM_SliderTickmarkOffset,	// 
	PM_SliderSpaceAvailable,	// available space for slider to move

	PM_DockWindowSeparatorExtent,
	PM_DockWindowHandleExtent,
	PM_DockWindowFrameWidth,

	PM_MenuBarFrameWidth,

	PM_TabBarTabOverlap,
	PM_TabBarTabHSpace,
	PM_TabBarTabVSpace,
	PM_TabBarBaseHeight,
	PM_TabBarBaseOverlap,

	PM_ProgressBarChunkWidth,

	PM_SplitterWidth,

	PM_IndicatorWidth,
	PM_IndicatorHeight,
	PM_ExclusiveIndicatorWidth,
	PM_ExclusiveIndicatorHeight

	/*
	  PM_PopupFrameWidth,
	  PM_MenuFrameWidth,

	  PM_ComboBoxAdditionalWidth,
	  PM_ComboBoxAdditionalHeight,

	  PM_SpinWidgetAdditionalWidth,
	  PM_SpinWidgetAdditionalHeight,
	*/
    };

    virtual int pixelMetric( PixelMetric metric,
			     const QWidget *widget = 0 ) const = 0;


    enum ContentsType {
	CT_PushButton,
	CT_CheckBox,
	CT_RadioButton,
	CT_ToolButton,
	CT_ComboBox,
	CT_Splitter,
	CT_DockWindow,
	CT_ProgressBar,
	CT_PopupMenuItem
    };

    virtual QSize sizeFromContents( ContentsType contents,
				    const QWidget *widget,
				    const QSize &contentsSize,
				    void **data = 0 ) const = 0;

    enum StyleHint  {
	SH_ScrollBar_BackgroundMode,
	SH_ScrollBar_MiddleClickAbsolutePosition,
	SH_ScrollBar_ScrollWhenPointerLeavesControl,
	SH_TabBar_Alignment,
	SH_Header_Arrow_Alignment
    };

    virtual int styleHint( StyleHint stylehint,
			   const QWidget *widget = 0,
			   void ***returnData = 0 ) const = 0;


    enum StylePixmap {
	SP_TitleBarMinButton,
	SP_TitleBarMaxButton,
	SP_TitleBarCloseButton,
	SP_TitleBarNormalButton,
	SP_TitleBarShadeButton,
	SP_TitleBarUnshadeButton,
	SP_DockWindowCloseButton
    };

    virtual QPixmap stylePixmap( StylePixmap stylepixmap,
				 const QWidget *widget = 0,
				 void **data = 0 ) const = 0;


    static QRect visualRect( const QRect &logical, const QWidget *w );

    static QRect visualRect( const QRect &logical, const QRect &bounding );




    // Old 2.x QStyle API

#ifndef QT_NO_COMPAT
    operator GUIStyle() const { return gs; }
    bool operator==(GUIStyle s) const { return gs==s; }
    bool operator!=(GUIStyle s) const { return gs!=s; }
#endif

    GUIStyle guiStyle() const { return gs; }


private:
    QStylePrivate * d;

#if defined(Q_DISABLE_COPY)
    QStyle( const QStyle & );
    QStyle& operator=( const QStyle & );
#endif
};

#endif // QT_NO_STYLE
#endif // QSTYLE_H
