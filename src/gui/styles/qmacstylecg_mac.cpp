#include <qmacstyle_mac.h>
#include <qmacstylecg_mac.h>

#if QT_MACOSX_VERSION >= 0x1030

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qmenu.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpointer.h>
#include <q3popupmenu.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qrubberband.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qrangecontrol.h>
#include <qt_mac.h>
#include <qtabbar.h>
#include <qaquastyle_mac.h>

#include <private/qtitlebar_p.h>

QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp

const int qt_mac_hitheme_version = 0; //the HITheme version we speak
const int macSpinBoxSep        = 5;    // distance between spinwidget and the lineedit
const ThemeWindowType QtWinType = kThemeUtilityWindow; // Window type we use for QTitleBar.
// Utility to generate correct rectangles for AppManager internals
static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice * =0,
                                            bool off=true, const QRect &rect=QRect())
{
    static HIRect r;
    int offset = 0;
    if (off)
        offset = 1;
    r = CGRectMake(qr.x()+rect.x(), qr.y()+rect.y(),
                   qr.width() - offset - rect.width(), qr.height() - offset - rect.height());
    return &r;
}

static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
                                            bool off=true, const QRect &rect=QRect())
{
    int x, y;
    p->map(qr.x(), qr.y(), &x, &y);
    QRect r(QPoint(x, y), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

static inline QWidget *qt_abuse_painter_for_widget(QPainter *p)
{
    QWidget *w = 0;
    if (p && p->device()->devType() == QInternal::Widget)
        w = static_cast<QWidget *>(p->device());
    return w;
}

static inline const QRect qrectForHIRect(const HIRect &hirect)
{
    return QRect(QPoint(int(hirect.origin.x), int(hirect.origin.y)),
                 QSize(int(hirect.size.width), int(hirect.size.height)));
}

inline static bool qt_mac_is_metal(QWidget *w)
{
    for( ; w; w = w->parentWidget()) {
        if(w->testAttribute(QWidget::WA_MacMetalStyle)) 
            return true;
        if(w->isTopLevel())
            break;
    }
    return false;
}

inline static bool qt_mac_is_metal(QPainter *p)
{
    return qt_mac_is_metal(qt_abuse_painter_for_widget(p));
}

// Utility to get the track info for scroll bars and sliders. It is used a few times and
// its worth not having to initalize all those items everytime. I have no idea why we
// can't do this by value, but it makes the darkening of the slider's thumb thing not work
// if we do. :-/
static inline HIThemeTrackDrawInfo *getTrackDrawInfo(QStyle::ComplexControl control,
                                                    const QAbstractSlider *aslider,
                                                    const QRect &rect = QRect(),
                                                    const QPainter *p = 0)
{
    static HIThemeTrackDrawInfo tdi;
    tdi.version = qt_mac_hitheme_version;
    tdi.reserved = 0;
    QAquaWidgetSize wsize = qt_aqua_size_constrain(aslider);
    if (control == QStyle::CC_Slider) {
        switch (wsize) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            tdi.kind = kThemeMediumSlider;
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                tdi.kind = kThemeMiniSlider;
                break;
            }
#endif
        case QAquaSizeSmall:
            tdi.kind = kThemeSmallSlider;
            break;
        }
    } else if (control == QStyle::CC_ScrollBar) {
        switch (wsize) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            tdi.kind = kThemeMediumScrollBar;
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030 && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                tdi.kind = kThemeMiniScrollBar;
                break;
            }
#endif
        case QAquaSizeSmall:
            tdi.kind = kThemeSmallScrollBar;
            break;
        }
    }
    if (rect.isValid() && p)
        tdi.bounds = *qt_glb_mac_rect(rect, p);
    else
        tdi.bounds = *qt_glb_mac_rect(aslider->rect());
    tdi.min = aslider->minimum();
    tdi.max = aslider->maximum();
    tdi.value = aslider->sliderPosition();
    tdi.attributes = kThemeTrackShowThumb;
    if(control == QStyle::CC_Slider && QSysInfo::MacintoshVersion >= QSysInfo::MV_JAGUAR
       && aslider->hasFocus())
        tdi.attributes |= kThemeTrackHasFocus;
    if(aslider->orientation() == Qt::Horizontal)
        tdi.attributes |= kThemeTrackHorizontal;
    if((control == QStyle::CC_Slider
         && !(aslider->orientation() == Qt::Horizontal) == !aslider->invertedAppearance())
        || (control == QStyle::CC_ScrollBar && aslider->invertedAppearance()))
        tdi.attributes |= kThemeTrackRightToLeft;
    tdi.enableState = aslider->isEnabled() ? kThemeTrackActive : kThemeTrackDisabled;
    // That's about all we can do at this that is the same, time to split it up.
    if (control == QStyle::CC_Slider) {
        const QSlider *slider = static_cast<const QSlider *>(aslider);
        if (slider->tickmarks() == QSlider::NoMarks || slider->tickmarks() == QSlider::Both)
            tdi.trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickmarks() == QSlider::Above)
            tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
    }
    return &tdi;
}

static inline ThemeDrawState getDrawState(QStyle::SFlags flags, const QPalette &pal)
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & QStyle::Style_Down) {
        tds = kThemeStatePressed;
    } else if (qAquaActive(pal)) {
        if (!(flags & QStyle::Style_Enabled))
            tds = kThemeStateUnavailable;
    } else {
        if (flags & QStyle::Style_Enabled)
            tds = kThemeStateInactive;
        else
            tds = kThemeStateUnavailableInactive;
    }
    return tds;
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if (p && p->device()->devType() == QInternal::Widget)
        return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(0);
}

class QMacStyleCGPrivate : public QAquaAnimate
{
public:
    UInt8 progressFrame;
    CFAbsoluteTime defaultButtonStart;
    QMacStyleCGPrivate() : progressFrame(0) { defaultButtonStart = CFAbsoluteTimeGetCurrent(); }

protected:
    friend class QMacStyleCG;
    inline int animateSpeed(Animates) { return 33; }   // Gives us about 30 frames a second.
    inline bool doAnimate(Animates anim) {
        if(anim == AquaProgressBar)
            ++progressFrame;
        return true;
    }
    virtual void doFocus(QWidget *) {
        //not implemented yet
    }
};

//externals
QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp

//HITheme QMacStyle
QMacStyleCG::QMacStyleCG() : QWindowsStyle()
{
    d = new QMacStyleCGPrivate;
}

QMacStyleCG::~QMacStyleCG()
{

}

void QMacStyleCG::polish(QWidget *w)
{
    d->addWidget(w);
    QPixmap px(0, 0, 32);
    if (qt_mac_is_metal(w)) {
        px.resize(200, 200);
        QPainter p(&px);
        HIThemeBackgroundDrawInfo bginfo;
        bginfo.version = qt_mac_hitheme_version;
        bginfo.state = kThemeStateActive;
        bginfo.kind = kThemeBackgroundMetal;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawBackground(&rect, &bginfo, static_cast<CGContextRef>(p.handle()),
                              kHIThemeOrientationNormal);
        p.end();
    }

    if (::qt_cast<Q3PopupMenu *>(w) || ::qt_cast<QMenu*>(w)) {
        px.resize(200, 200);
        QPainter p(&px);
        HIThemeMenuDrawInfo mtinfo;
        mtinfo.version = qt_mac_hitheme_version;
        mtinfo.menuType = kThemeMenuTypePopUp;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(p.handle()),
                                  kHIThemeOrientationNormal);
        p.end();
        w->setWindowOpacity(0.95);
    }

    if (::qt_cast<QRubberBand*>(w))
        w->setWindowOpacity(0.75);

    if (QTitleBar *tb = ::qt_cast<QTitleBar *>(w))
        tb->setAutoRaise(true);

    if (!px.isNull()) {
        QPalette pal = w->palette();
        QBrush background(px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    }
    QWindowsStyle::polish(w);
}

void QMacStyleCG::unPolish(QWidget *w)
{
    d->removeWidget(w);
    if (::qt_cast<Q3PopupMenu *>(w) || ::qt_cast<QMenu*>(w) || qt_mac_is_metal(w)) {
        QPalette pal = w->palette();
        QPixmap tmp;
        QBrush background(tmp);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
        w->setWindowOpacity(1.0);
    }
    if (::qt_cast<QRubberBand*>(w))
        w->setWindowOpacity(1.0);
    QWindowsStyle::unPolish(w);
}

void QMacStyleCG::polish(QApplication *app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QPainter p(&px);

    HIThemeMenuDrawInfo mtinfo;
    mtinfo.version = qt_mac_hitheme_version;
    mtinfo.menuType = kThemeMenuTypeHierarchical;
    HIRect rect = CGRectMake(0, 0, px.width(), px.height());
    HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(p.handle()),
                              kHIThemeOrientationNormal);

    p.end();
    QBrush background(px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
}

void QMacStyleCG::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r,
                                const QPalette &pal, SFlags flags, const QStyleOption &opt) const
{
    ThemeDrawState tds = getDrawState(flags, pal);
    switch (pe) {
    case PE_RubberBandMask:
        p->fillRect(r, color1);
        break;
    case PE_RubberBand:
        p->fillRect(r, pal.highlight());
        break;
    case PE_CheckListController:
        break;
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator: {
        HIThemeButtonDrawInfo info;
        info.version = qt_mac_hitheme_version;
        info.state = tds;
        switch (qt_mac_get_size_for_painter(p)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            info.kind = kThemeRadioButton;
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                info.kind = kThemeMiniRadioButton;
                break;
            }
#endif
        case QAquaSizeSmall:
            info.kind = kThemeSmallRadioButton;
            break;
        }
        info.value = (flags & Style_On) ? kThemeButtonOn : kThemeButtonOff;
        info.adornment = kThemeAdornmentNone;
        const HIRect *mac_r = qt_glb_mac_rect(r, p);
        if (pe == PE_ExclusiveIndicatorMask) {
            QRegion saveRegion = p->clipRegion();
            QCFType<HIShapeRef> shape;
            HIThemeGetButtonShape(mac_r, &info, &shape);
            p->setClipRegion(qt_mac_convert_mac_region(shape));
            p->fillRect(r, color1);
            p->setClipRegion(saveRegion);
        } else {
            HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
                              kHIThemeOrientationNormal, 0);
        }
        break; }
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator: {
        HIThemeButtonDrawInfo info;
        info.version = qt_mac_hitheme_version;
        info.state = tds;
        switch (qt_mac_get_size_for_painter(p)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            info.kind = kThemeCheckBox;
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                info.kind = kThemeMiniCheckBox;
                break;
            }
#endif
        case QAquaSizeSmall:
            info.kind = kThemeSmallCheckBox;
            break;
        }
        if (flags & Style_NoChange)
            info.value = kThemeButtonMixed;
        else if (flags & Style_On)
            info.value = kThemeButtonOn;
        else
            info.value = kThemeButtonOff;
        info.adornment = kThemeAdornmentNone;
        const HIRect *mac_r = qt_glb_mac_rect(r, p);
        if (pe == PE_IndicatorMask) {
            QRegion saveRegion = p->clipRegion();
            QCFType<HIShapeRef> shape;
            HIThemeGetButtonShape(mac_r, &info, &shape);
            p->setClipRegion(qt_mac_convert_mac_region(shape));
            p->fillRect(r, color1);
            p->setClipRegion(saveRegion);
        } else {
            HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
                              kHIThemeOrientationNormal, 0);
        }
        break; }
    case PE_FocusRect:
//        HIThemeDrawFocusRect(qt_glb_mac_rect(r, p), true, static_cast<CGContextRef>(p->handle()),
//                             kHIThemeOrientationNormal);
        break;
    case PE_Splitter: {
        HIThemeSplitterDrawInfo sdi;
        sdi.version = qt_mac_hitheme_version;
        sdi.state = tds;
        sdi.adornment = qt_mac_is_metal(p) ? kHIThemeSplitterAdornmentMetal
                                           : kHIThemeSplitterAdornmentNone;
        HIThemeDrawPaneSplitter(qt_glb_mac_rect(r, p), &sdi, static_cast<CGContextRef>(p->handle()),
                                kHIThemeOrientationNormal);
        break; }
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        HIThemePopupArrowDrawInfo info;
        info.version = qt_mac_hitheme_version;
        info.state = tds;
        switch (pe) {
        case PE_ArrowUp:
            info.orientation = kThemeArrowUp;
            break;
        case PE_ArrowDown:
            info.orientation = kThemeArrowDown;
            break;
        case PE_ArrowRight:
            info.orientation = kThemeArrowRight;
            break;
        case PE_ArrowLeft:
            info.orientation = kThemeArrowLeft;
            break;
        default:     // Stupid compiler _should_ know better.
            break;
        }
        if (r.width() < 8)
            info.size = kThemeArrow5pt;
        else
            info.size = kThemeArrow9pt;
        HIThemeDrawPopupArrow(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
                              kHIThemeOrientationNormal);
        break; }
    case PE_TreeBranch: {
        if (!(flags & Style_Children))
            break;
        HIThemeButtonDrawInfo bi;
        bi.version = qt_mac_hitheme_version;
        bi.state = flags & Style_Enabled ? kThemeStateActive : kThemeStateInactive;
        if (flags & Style_Down)
            bi.state |= kThemeStatePressed;
        bi.kind = kThemeDisclosureButton;
        bi.value = flags & Style_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        bi.adornment = kThemeAdornmentNone;
        HIThemeDrawButton(qt_glb_mac_rect(r, p), &bi, static_cast<CGContextRef>(p->handle()),
                          kHIThemeOrientationNormal, 0);
        break; }
    case PE_TabBarBase: {
        HIThemeTabPaneDrawInfo tpdi;
        tpdi.version = qt_mac_hitheme_version;
        tpdi.state = tds;
        tpdi.direction = kThemeTabNorth;
        if (flags & Style_Bottom)
            tpdi.direction = kThemeTabSouth;
        tpdi.size = kHIThemeTabSizeNormal;
        HIThemeDrawTabPane(qt_glb_mac_rect(r, p), &tpdi, static_cast<CGContextRef>(p->handle()),
                           kHIThemeOrientationNormal);
        break; }
    case PE_SizeGrip: {
        HIThemeGrowBoxDrawInfo gdi;
        gdi.version = qt_mac_hitheme_version;
        gdi.state = tds;
        gdi.kind = kHIThemeGrowBoxKindNormal;
        gdi.direction = kThemeGrowLeft | kThemeGrowDown;
        gdi.size = kHIThemeGrowBoxSizeNormal; // How does one determine the size?
        HIPoint pt = { r.x(), r.y() };
        HIThemeDrawGrowBox(&pt, &gdi, static_cast<CGContextRef>(p->handle()),
                           kHIThemeOrientationNormal);
        break; }
    case PE_HeaderArrow: {
        QWidget *w = qt_abuse_painter_for_widget(p);
        if (w && w->inherits("QTable"))
            drawPrimitive(flags & Style_Up ? PE_ArrowUp : PE_ArrowDown, p, r, pal, flags, opt);
        // ListView header is taken care of.
        break; }
    case PE_HeaderSection: {
        QWidget *w = qt_abuse_painter_for_widget(p);
        HIThemeButtonDrawInfo bdi;
        bdi.version = qt_mac_hitheme_version;
        bdi.state = tds;
        // This will have to go, but it keeps us in parity with the current state of
        // affairs with table header.
        if (w && w->parentWidget()->inherits("QTable")) {
            bdi.kind = kThemeBevelButton;
            if (p->font().bold())
                flags |= Style_Sunken;
            else
                flags &= ~Style_Sunken;
        } else {
            bdi.kind = kThemeListHeaderButton;
        }
        if (flags & Style_Sunken)
            bdi.value = kThemeButtonOn;
        else
            bdi.value = kThemeButtonOff;

        bdi.adornment = kThemeAdornmentNone;

        QRect ir = r;
        if (flags & Style_Off)
            ir.setRight(ir.right() + 50);  // Cheat to hide the down indicator.
        else if (flags & Style_Up)
            bdi.adornment = kThemeAdornmentHeaderButtonSortUp;

        if (flags & Style_HasFocus && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
            bdi.adornment = kThemeAdornmentFocus;
        HIThemeDrawButton(qt_glb_mac_rect(ir, p), &bdi, static_cast<CGContextRef>(p->handle()),
                          kHIThemeOrientationNormal, 0);
        break; }
    case PE_PanelGroupBox: {
        if (opt.isDefault())
            break;
        HIThemeGroupBoxDrawInfo gdi;
        gdi.version = qt_mac_hitheme_version;
        gdi.state = tds;
        QWidget *w = qt_abuse_painter_for_widget(p);
        if (w && ::qt_cast<QGroupBox *>(w->parentWidget()))
            gdi.kind = kHIThemeGroupBoxKindSecondary;
        else
            gdi.kind = kHIThemeGroupBoxKindPrimary;
        HIThemeDrawGroupBox(qt_glb_mac_rect(r, p), &gdi, static_cast<CGContextRef>(p->handle()),
                            kHIThemeOrientationNormal);
        break; }
    case PE_Panel:
    case PE_PanelLineEdit:
        if (flags & Style_Sunken) {
            HIThemeFrameDrawInfo fdi;
            fdi.version = qt_mac_hitheme_version;
            fdi.state = tds;

            SInt32 frame_size;
            if (pe == PE_PanelLineEdit) {
                fdi.kind = kHIThemeFrameTextFieldSquare;
                GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
            } else {
                fdi.kind = kHIThemeFrameListBox;
                GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
            }
            int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth, 0) : opt.lineWidth();
            p->fillRect(r.x(), r.y(), lw, r.height(), pal.background());
            p->fillRect(r.right() - lw + 1, r.y(), lw, r.height(), pal.background());
            p->fillRect(r.x(), r.y(), r.width(), lw, pal.background());
            p->fillRect(r.x(), r.bottom() - lw + 1, r.width(), lw, pal.background());
            const HIRect *rect = qt_glb_mac_rect(r, p, false, QRect(frame_size, frame_size,
                                                                    frame_size * 2, frame_size * 2));
            HIThemeDrawFrame(rect, &fdi, static_cast<CGContextRef>(p->handle()),
                             kHIThemeOrientationNormal);
            break;
        }
        // Fall through!
    default:
        QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
        break;
    }
}

void QMacStyleCG::drawControl(ControlElement element, QPainter *p, const QWidget *widget,
                              const QRect &r, const QPalette &pal, SFlags how,
                              const QStyleOption &opt) const
{
    ThemeDrawState tds = getDrawState(how, pal);
    switch(element) {
    case CE_PushButton: {
        if (!widget)
            break;
        const QPushButton *btn = static_cast<const QPushButton *>(widget);
        if (btn->isFlat() && !(how & (Style_Down | Style_On)))
            return;
        HIThemeButtonDrawInfo info;
        info.version = qt_mac_hitheme_version;
        if (how & (Style_On | Style_Down))
            info.state = kThemeStatePressed;
        else
            info.state = tds;
        info.value = kThemeButtonOff;
        info.adornment = kThemeAdornmentNone;
        if (btn->isFlat())
            info.kind = kThemeMediumBevelButton;
        else
            info.kind = kThemePushButton;

        if (how & Style_ButtonDefault && d->animatable(QAquaAnimate::AquaPushButton, btn)) {
            info.adornment = kThemeAdornmentDefault;
            info.animation.time.start = d->defaultButtonStart;
            info.animation.time.current = CFAbsoluteTimeGetCurrent();
        }
        HIThemeDrawButton(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
                          kHIThemeOrientationNormal, 0);
        break; }
    case CE_PushButtonLabel: {
        // ### This is wrong, we should probably have another couple of rects,
        // the arrow shouldn't be part of the label.
#ifndef QT_NO_PUSHBUTTON
        const QPushButton *button = static_cast<const QPushButton *>(widget);
        QRect ir = r;
        if (button->isMenuButton()) {
            int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
            QRect ar(ir.right() - mbi, ir.height() - 13, mbi, ir.height() - 4);
            drawPrimitive(PE_ArrowDown, p, ar, pal, how, opt);
            ir.setWidth(ir.width() - mbi);
        }
        int tf = AlignVCenter | ShowPrefix | NoAccel;

#ifndef QT_NO_ICONSET
        if (button->iconSet() && ! button->iconSet()->isNull()) {
            QIconSet::Mode mode = button->isEnabled() ? QIconSet::Normal : QIconSet::Disabled;
            if (mode == QIconSet::Normal && button->hasFocus())
                mode = QIconSet::Active;

            QIconSet::State state = QIconSet::Off;
            if (button->isToggleButton() && button->isOn())
                state = QIconSet::On;

            QPixmap pixmap = button->iconSet()->pixmap(QIconSet::Small, mode, state);
            int pixw = pixmap.width();
            int pixh = pixmap.height();

            //Center the icon if there is neither text nor pixmap
            if (button->text().isEmpty() && !button->pixmap())
                p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2,
                              ir.y() + ir.height() / 2 - pixh / 2, pixmap);
            else
                p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);

            ir.moveBy(pixw + 4, 0);
            ir.setWidth(ir.width() - (pixw + 4));
            // left-align text if there is
            if (!button->text().isEmpty())
                tf |= AlignLeft;
            else if (button->pixmap())
                tf |= AlignHCenter;
        } else
#endif //QT_NO_ICONSET
            tf |= AlignHCenter;
        drawItem(p, ir, tf, pal,
                 how & Style_Enabled, button->pixmap(), button->text(), -1,
                 &(pal.buttonText().color()));
#endif
        break; }
    case CE_Q3PopupMenuItem: {
        if (!widget || opt.isDefault())
            break;
        const Q3PopupMenu *popup = static_cast<const Q3PopupMenu *>(widget);
        const Q3MenuItem *mi = opt.menuItem();
        bool disabled = mi ? !mi->isEnabled() : false;
        bool itemSelected = how & Style_Active;
        // Draw the background and then the text and stuff.
        HIRect menuRect = *qt_glb_mac_rect(popup->rect(), popup);
        HIRect itemRect = *qt_glb_mac_rect(r, popup);
        HIThemeMenuItemDrawInfo mdi;
        mdi.version = qt_mac_hitheme_version;
        mdi.itemType = kThemeMenuItemPlain;
        if (mi && mi->iconSet())
            mdi.itemType |= kThemeMenuItemHasIcon;
        if (mi && mi->popup())
            mdi.itemType |= kThemeMenuItemHierarchical | kThemeMenuItemHierBackground;
        else
            mdi.itemType |= kThemeMenuItemPopUpBackground;
        mdi.state = (disabled) ? kThemeMenuDisabled : kThemeMenuActive;
        if (itemSelected)
            mdi.state |= kThemeMenuSelected;
        HIRect contentRect;
        if (mi && mi->isSeparator()) {
            // ### The first argument should be menuRect, but it seems to
            // seriously make things not look very good.
            HIThemeDrawMenuSeparator(&itemRect, &itemRect, &mdi,
                                     static_cast<CGContextRef>(p->handle()),
                                     kHIThemeOrientationNormal);
            break;
        } else {
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi, static_cast<CGContextRef>(p->handle()),
                                kHIThemeOrientationNormal, &contentRect);
        }
        bool checkable = popup->isCheckable();
        bool itemChecked = mi->isChecked();
        bool reverse = QApplication::reverseLayout();
        int maxIW = opt.maxIconWidth();
        if (checkable)
            maxIW += 12;
        QRect qtContentRect = qrectForHIRect(contentRect);
        int xpos = qtContentRect.x() + 2;
        if (reverse)
            xpos = qtContentRect.width() - maxIW - 2;
        // Draw checks
        if (checkable && itemChecked) {
            int mw = maxIW;
            int mh = qtContentRect.height();
            int xp = xpos;
            SFlags myflags = Style_Default;
            if (!disabled)
                myflags |= Style_Enabled;
            if (itemSelected)
                myflags |= Style_On;
            // ### Need to implement PE_CheckMark, commonstyle doesn't look correct for Mac OS X.
            // It should also change color when it is selected.
            drawPrimitive(PE_CheckMark, p, QRect(xp, qtContentRect.y(), mw, mh), pal, myflags);
        }
        if (disabled)
            p->setPen(pal.text());
        else if (itemSelected)
            p->setPen(pal.highlightedText());
        else
            p->setPen(pal.buttonText());
        // All Mac popups appear to have a space for a check.
        xpos += maxIW + 5;
        // Draw text
        if (mi) {
            QString str = mi->text();
            if (!str.isEmpty()) {
                int text_flags = AlignVCenter | NoAccel | DontClip | SingleLine;
                p->drawText(xpos, qtContentRect.y(), qtContentRect.width(), qtContentRect.height(),
                            text_flags, str);
            }
        }
        break; }
    case CE_ProgressBarContents: {
        const QProgressBar *progressbar = static_cast<const QProgressBar *>(widget);
        HIThemeTrackDrawInfo tdi;
        tdi.version = qt_mac_hitheme_version;
        tdi.reserved = 0;
        switch (qt_aqua_size_constrain(progressbar)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            tdi.kind = progressbar->totalSteps() ? kThemeLargeProgressBar
                                                 : kThemeLargeIndeterminateBar;
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                tdi.kind = progressbar->totalSteps() ? kThemeMiniProgressBar
                                                     : kThemeMiniIndeterminateBar;
                break;
            }
#endif
        case QAquaSizeSmall:
            tdi.kind = progressbar->totalSteps() ? kThemeProgressBar
                                                 : kThemeIndeterminateBar;
            break;
        }
        tdi.bounds = *qt_glb_mac_rect(r, p);
        tdi.max = progressbar->totalSteps();
        tdi.min = 0;
        tdi.value = progressbar->progress();
        tdi.attributes = kThemeTrackHorizontal;
        tdi.trackInfo.progress.phase = d->progressFrame;
        if (!qAquaActive(pal))
            tdi.enableState = kThemeTrackInactive;
        else if (!progressbar->isEnabled())
            tdi.enableState = kThemeTrackDisabled;
        else
            tdi.enableState = kThemeTrackActive;
        HIThemeDrawTrack(&tdi, 0, static_cast<CGContextRef>(p->handle()), kHIThemeOrientationNormal);
        break; }
    case CE_ProgressBarLabel:
    case CE_ProgressBarGroove:
        break;
    case CE_TabBarTab: {
        const QTabBar *tabbar = static_cast<const QTabBar *>(widget);
        HIThemeTabDrawInfo tdi;
        tdi.version = qt_mac_hitheme_version;
        ThemeTabStyle tts = kThemeTabNonFront;
        if(how & Style_Selected) {
            if(!qAquaActive(pal))
                tts = kThemeTabFrontUnavailable;
            else if(!(how & Style_Enabled))
                tts = kThemeTabFrontInactive;
            else
                tts = kThemeTabFront;
        } else if(!qAquaActive(pal)) {
            tts = kThemeTabNonFrontUnavailable;
        } else if(!(how & Style_Enabled)) {
            tts = kThemeTabNonFrontInactive;
        } else if((how & Style_Sunken) && (how & Style_MouseOver)) {
            tts = kThemeTabNonFrontPressed;
        }
        tdi.style = tts;
        if (tabbar->shape() == QTabBar::RoundedAbove || tabbar->shape() == QTabBar::TriangularAbove)
            tdi.direction = kThemeTabNorth;
        else
            tdi.direction = kThemeTabSouth;
        tdi.size = kHIThemeTabSizeNormal;
        if (how & Style_HasFocus)
            tdi.adornment = kHIThemeTabAdornmentFocus;
        else
            tdi.adornment = kHIThemeTabAdornmentNone;
        QRect tabrect = r;
        tabrect.setHeight(tabrect.height() + pixelMetric(PM_TabBarBaseOverlap, widget));
        QRegion oldRegion(p->clipRegion());
        p->setClipRect(tabrect);
        HIThemeDrawTab(qt_glb_mac_rect(tabrect, p), &tdi, static_cast<CGContextRef>(p->handle()),
                       kHIThemeOrientationNormal, 0);
        p->setClipRegion(oldRegion);
        // If the tab is not selected, we have to redraw a portion of the pane.
        if (!(how & Style_Selected)) {
            HIThemeTabPaneDrawInfo tpdi;
            tpdi.version = qt_mac_hitheme_version;
            tpdi.state = tds;
            tpdi.direction = tdi.direction;
            tpdi.size = tdi.size;
            // This fudge is used to so we don't draw the edges of the pane in the middle.
            // So draw a little bit more and clip it.
            const int FUDGE = 20;
            QRect panerect(r.x() - FUDGE, r.bottom() - 2, 2 * FUDGE + r.width(),
                           pixelMetric(PM_TabBarBaseHeight, widget));
            if (tdi.direction == kThemeTabSouth)
                panerect.moveBy(0, (-r.height() + 2));
            oldRegion = p->clipRegion();
            p->setClipRect(r.x(), panerect.y(), r.width(), panerect.height());
            HIThemeDrawTabPane(qt_glb_mac_rect(panerect, p), &tpdi,
                               static_cast<CGContextRef>(p->handle()), kHIThemeOrientationNormal);
            p->setClipRegion(oldRegion);
        }
        break; }
    default:
        QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
    }
}

void QMacStyleCG::drawComplexControl(ComplexControl control, QPainter *p, const QWidget *w,
                                     const QRect &r, const QPalette& pal, SFlags flags, SCFlags sub,
                                     SCFlags subActive, const QStyleOption &opt) const
{
    ThemeDrawState tds = getDrawState(flags, pal);
    switch (control) {
    case CC_Slider:
    case CC_ScrollBar: {
        const QAbstractSlider *slider = static_cast<const QAbstractSlider *>(w);
        bool tracking = slider->hasTracking();
        HIThemeTrackDrawInfo tdi = *getTrackDrawInfo(control, slider, r, p);
        if(tds == kThemeStateInactive)
            tdi.enableState = kThemeTrackInactive;
        else if(tds == kThemeStateUnavailable)
            tdi.enableState = kThemeTrackDisabled;
        else
            tdi.enableState = kThemeTrackActive;
        if (control == CC_Slider) {
            if (subActive == SC_SliderGroove)
                tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
            else if (subActive == SC_SliderHandle)
                tdi.trackInfo.slider.pressState = kThemeThumbPressed;
        } else {
            if (subActive == SC_ScrollBarSubLine)
                tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                     | kThemeLeftOutsideArrowPressed;
            else if (subActive == SC_ScrollBarAddLine)
                tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                     | kThemeRightOutsideArrowPressed;
            else if (subActive == SC_ScrollBarAddPage)
                tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
            else if (subActive == SC_ScrollBarSubPage)
                tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
            else if (subActive == SC_ScrollBarSlider)
                tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            tdi.trackInfo.scrollbar.viewsize = slider->pageStep();
        }
        HIRect macRect;
        if (!tracking) {
            // Small optimization, the same as querySubControlMetrics
            QCFType<HIShapeRef> shape;
            HIThemeGetTrackThumbShape(&tdi, &shape);
            HIShapeGetBounds(shape, &macRect);
            tdi.value = slider->value();
        }
        HIThemeDrawTrack(&tdi, tracking ? 0 : &macRect, static_cast<CGContextRef>(p->handle()),
                         kHIThemeOrientationNormal);
        if (sub & SC_SliderTickmarks) {
            int numMarks = slider->maximum() / slider->pageStep();
            if (control == QStyle::CC_Slider) {
                const QSlider *s = static_cast<const QSlider *>(slider);
                if(s->tickInterval())
                    numMarks = s->width() / s->tickInterval();
            }
            if (tdi.trackInfo.slider.thumbDir == kThemeThumbPlain) {
                // They asked for both, so we'll give it to them.
                tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
                HIThemeDrawTrackTickMarks(&tdi, numMarks, static_cast<CGContextRef>(p->handle()),
                                          kHIThemeOrientationNormal);
                tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
                HIThemeDrawTrackTickMarks(&tdi, numMarks, static_cast<CGContextRef>(p->handle()),
                                          kHIThemeOrientationNormal);
            } else {
                HIThemeDrawTrackTickMarks(&tdi, numMarks, static_cast<CGContextRef>(p->handle()),
                                          kHIThemeOrientationNormal);

            }
        }
        break; }
    case CC_SpinWidget: {
        const QSpinWidget *sw = static_cast<const QSpinWidget *>(w);
        if (sub & SC_SpinWidgetFrame)
            drawPrimitive(PE_PanelLineEdit, p,
                          querySubControlMetrics(CC_SpinWidget, sw, SC_SpinWidgetFrame), pal,
                          Style_Sunken);
        if (sub & SC_SpinWidgetUp || sub & SC_SpinWidgetDown) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            switch (qt_aqua_size_constrain(sw)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bdi.kind = kThemeIncDecButton;
                break;
            case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    bdi.kind = kThemeIncDecButtonMini;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bdi.kind = kThemeIncDecButtonSmall;
                break;
            }
            if (!sw->isUpEnabled() && !sw->isDownEnabled())
                tds = kThemeStateUnavailable;
            if (subActive == SC_SpinWidgetDown)
                tds = kThemeStatePressedDown;
            else if (subActive == SC_SpinWidgetUp)
                tds = kThemeStatePressedUp;
            bdi.state = tds;
            bdi.value = kThemeButtonOff;
            if (flags & Style_HasFocus && QMacStyle::focusRectPolicy(sw) != QMacStyle::FocusDisabled)
                bdi.adornment = kThemeAdornmentFocus;
            else
                bdi.adornment = kThemeAdornmentNone;
            QRect updown = sw->upRect() | sw->downRect();
            if (sw->palette().brush(sw->backgroundRole()).pixmap())
                p->drawPixmap(updown, *sw->palette().brush(sw->backgroundRole()).pixmap());
            else
                p->fillRect(updown, sw->palette().color(sw->backgroundRole()));
            HIThemeDrawButton(qt_glb_mac_rect(updown, p), &bdi,
                              static_cast<CGContextRef>(p->handle()), kHIThemeOrientationNormal, 0);
        }
        break; }
    case CC_ComboBox: {
        HIThemeButtonDrawInfo bdi;
        bdi.version = qt_mac_hitheme_version;
        QRect comborect(r);
        bdi.adornment = flags & Style_HasFocus ? kThemeAdornmentFocus : kThemeAdornmentNone;
        bdi.state = subActive & SC_ComboBoxArrow ? (ThemeDrawState)kThemeStatePressed : tds;
        if (static_cast<const QComboBox *>(w)->editable()) {
            bdi.adornment |= kThemeAdornmentArrowDownArrow;
            comborect = querySubControlMetrics(CC_ComboBox, w, SC_ComboBoxArrow, opt);
            switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bdi.kind = kThemeArrowButton;
                break;
            case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    bdi.kind = kThemeArrowButtonMini;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bdi.kind = kThemeArrowButtonSmall;
                break;
            }
            QRect lineeditRect(r);
            lineeditRect.setWidth(r.width() - comborect.width());
            drawPrimitive(PE_PanelLineEdit, p, lineeditRect, pal, flags, opt);
        } else {
            bdi.adornment |= kThemeAdornmentArrowLeftArrow;
            bdi.kind = kThemePopupButton;
        }
        HIThemeDrawButton(qt_glb_mac_rect(comborect, p), &bdi,
                          static_cast<CGContextRef>(p->handle()), kHIThemeOrientationNormal, 0);
        break; }
    case CC_ListView:
        if (sub & SC_ListView)
            QWindowsStyle::drawComplexControl(control, p, w, r, pal, flags, sub, subActive, opt);
        if (sub & (SC_ListViewBranch | SC_ListViewExpand)) {
            if (opt.isDefault())
                break;
            QListViewItem *item = opt.listViewItem()->firstChild();
            int y = r.y();
            int h = r.height();
            int x = r.right() - 10;
            while (item && y < h) {
                if (y + item->height() > 0) {
                    uint myflags = flags;
                    if (item->isExpandable() || item->childCount()) {
                        myflags |= Style_Children;
                        if (item->isOpen())
                            myflags |= Style_Open;
                        QRect mr(x, y + item->height() / 2 - 4, 9, 9);
                        drawPrimitive(PE_TreeBranch, p, mr, pal, myflags, opt);
                    }
                }
                y += item->totalHeight();
                item = item->nextSibling();
            }
        }
        break;
    case CC_TitleBar: {
        const QTitleBar *titlebar = static_cast<const QTitleBar *>(w);
        HIThemeWindowDrawInfo wdi;
        wdi.version = qt_mac_hitheme_version;
        wdi.state = tds;
        wdi.windowType = QtWinType;
        wdi.attributes = kThemeWindowHasTitleText;
        if (titlebar->window()) {
            if (titlebar->window()->isMinimized())
                wdi.attributes |= kThemeWindowIsCollapsed;
            if (titlebar->window()->isWindowModified())
                wdi.attributes |= kThemeWindowHasDirty;
            wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                 | kThemeWindowHasCollapseBox;
        } else if (titlebar->testWFlags(WStyle_SysMenu)) {
            wdi.attributes |= kThemeWindowHasCloseBox;
        }
        wdi.titleHeight = r.height();
        wdi.titleWidth = r.width();
        QCFType<HIShapeRef> titleRegion;
        HIThemeGetWindowShape(qt_glb_mac_rect(r, p), &wdi, kWindowTitleBarRgn,
                              &titleRegion);
        HIRect titleRect;
        HIShapeGetBounds(titleRegion, &titleRect);
        QRect newr = r;
        newr.moveBy(newr.x() - (int)titleRect.origin.x, newr.y() - (int)titleRect.origin.y);
        HIRect finalRect = *qt_glb_mac_rect(newr, p, false);
        HIThemeDrawWindowFrame(&finalRect, &wdi, static_cast<CGContextRef>(p->handle()),
                               kHIThemeOrientationNormal, 0);
        if (sub & SC_TitleBarLabel) {
            int iw = 0;
            if (!!titlebar->windowIcon()) {
                QCFType<HIShapeRef> titleRegion2;
                HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleProxyIconRgn,
                                      &titleRegion2);
                HIShapeGetBounds(titleRegion2, &titleRect);
                if (titleRect.size.width != 1)
                    iw = titlebar->windowIcon().width();
            }
            if (!titlebar->visibleText().isEmpty()) {
                p->save();
                QCFType<HIShapeRef> titleRegion3;
                HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleTextRgn,
                                      &titleRegion3);
                HIShapeGetBounds(titleRegion3, &titleRect);
                p->setClipRect(qrectForHIRect(titleRect));
                QRect br = p->clipRegion().boundingRect();
                int x = br.x(),
                    y = br.y() + (titlebar->height() / 2 - p->fontMetrics().height() / 2);
                if (br.width() <= (p->fontMetrics().width(titlebar->windowTitle()) + iw * 2))
                    x += iw;
                else
                    x += br.width() / 2 - p->fontMetrics().width(titlebar->visibleText()) / 2;
                if (iw)
                    p->drawPixmap(x - iw, y, titlebar->windowIcon());
                p->drawText(x, y + p->fontMetrics().ascent(), titlebar->visibleText());
                p->restore();
            }
        }

        if (sub & (SC_TitleBarCloseButton | SC_TitleBarMaxButton | SC_TitleBarMinButton |
                   SC_TitleBarNormalButton)) {
            HIThemeWindowWidgetDrawInfo wwdi;
            wwdi.version = qt_mac_hitheme_version;
            wwdi.widgetState = tds;
            if (flags & Style_MouseOver)
                wwdi.widgetState = kThemeStateRollover;
            wwdi.windowType = QtWinType;
            wwdi.attributes = wdi.attributes;
            wwdi.windowState = wdi.state;
            wwdi.titleHeight = wdi.titleHeight;
            wwdi.titleWidth = wdi.titleWidth;
            ThemeDrawState savedControlState = wwdi.widgetState;
            uint sc = SC_TitleBarMinButton;
            ThemeTitleBarWidget tbw = kThemeWidgetCollapseBox;
            while (sc <= SC_TitleBarCloseButton) {
                uint tmp = sc;
                wwdi.widgetState = savedControlState;
                wwdi.widgetType = tbw;
                if (sc == SC_TitleBarMinButton)
                    tmp |= SC_TitleBarNormalButton;
                if (qAquaActive(pal) && (subActive & tmp))
                    wwdi.widgetState = kThemeStatePressed;
                if (titlebar->window() && titlebar->window()->isWindowModified()
                    && tbw == kThemeWidgetCloseBox)
                    wwdi.widgetType = kThemeWidgetDirtyCloseBox;
                HIThemeDrawTitleBarWidget(&finalRect, &wwdi, static_cast<CGContextRef>(p->handle()),
                                          kHIThemeOrientationNormal);
                sc = sc << 1;
                tbw = tbw >> 1;
            }
        }
        break; }
    default:
        QWindowsStyle::drawComplexControl(control, p, w, r, pal, flags, sub, subActive, opt);
    }
}

int QMacStyleCG::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret;
    switch(metric) {
    case PM_IndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_IndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxHeight, &ret);
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxHeight, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonHeight, &ret);
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonHeight, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonWidth, &ret);
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonWidth, &ret);
            break;
        }
        break; }
    case PM_MenuButtonIndicator:
        GetThemeMetric(kThemeMetricDisclosureTriangleWidth, &ret);
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    case PM_SliderLength:
        ret = 17;
        break;
    case PM_ScrollBarExtent: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricScrollBarWidth, &ret);
            break;
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030 && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniScrollBarWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallScrollBarWidth, &ret);
            break;
        }
        break; }
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_TabBarTabVSpace:
        ret = 4;
        break;
    case PM_TabBarBaseHeight:
        ret = 8;
        break;
    case PM_TabBarTabOverlap:
        GetThemeMetric(kThemeMetricTabOverlap, &ret);
        break;
    case PM_TabBarBaseOverlap:
        GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
        break;
    case PM_SpinBoxFrameWidth:
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &ret);
        ret += 2;
        break;
    case PM_TitleBarHeight: {
        /*
        if (!widget)
            break;
        const QTitleBar *titlebar = static_cast<const QTitleBar *>(widget);
        HIThemeWindowDrawInfo wdi;
        wdi.version = qt_mac_hitheme_version;
        wdi.state = kThemeStateActive;
        wdi.windowType = QtWinType;
        if (titlebar->window()) {
            if (titlebar->window()->isMinimized())
                wdi.attributes |= kThemeWindowIsCollapsed;
            if (titlebar->window()->isWindowModified())
                wdi.attributes |= kThemeWindowHasDirty;
            wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                | kThemeWindowHasCollapseBox;
        } else if (titlebar->testWFlags(WStyle_SysMenu)) {
            wdi.attributes |= kThemeWindowHasCloseBox;
        }
        wdi.titleHeight = titlebar->height();
        wdi.titleWidth = titlebar->width();
        HIShapeRef region;
        HIThemeGetWindowShape(qt_glb_mac_rect(titlebar->rect()), &wdi, kWindowTitleBarRgn, &region);
        HIRect rect;
        HIShapeGetBounds(region, &rect);
        CFRelease(region);
        ret = int(rect.size.height);
         break;*/ }
    default:
        ret = QWindowsStyle::pixelMetric(metric, widget);
        break;
    }
    return ret;
}


QRect QMacStyleCG::querySubControlMetrics(ComplexControl control, const QWidget *widget,
                                          SubControl sc, const QStyleOption &opt) const
{
    QRect rect;
    switch (control) {
    case CC_Slider: {
        const QSlider *slider = static_cast<const QSlider *>(widget);
        HIThemeTrackDrawInfo tdi = *getTrackDrawInfo(control, slider);
        HIRect macRect;
        switch (sc) {
        case SC_SliderGroove:
            HIThemeGetTrackBounds(&tdi, &macRect);
            rect = qrectForHIRect(macRect);
            break;
        case SC_SliderHandle:
            QCFType<HIShapeRef> shape;
            HIThemeGetTrackThumbShape(&tdi, &shape);
            HIShapeGetBounds(shape, &macRect);
            rect = qrectForHIRect(macRect);
            break;
        }
        break; }
    case CC_ScrollBar: {
        const QScrollBar *scrollbar = static_cast<const QScrollBar *>(widget);
        HIThemeTrackDrawInfo tdi = *getTrackDrawInfo(control, scrollbar);
        HIRect macRect;
        switch (sc) {
        case SC_ScrollBarGroove:
            HIThemeGetTrackDragRect(&tdi, &macRect);
            rect = qrectForHIRect(macRect);
            break;
        case SC_ScrollBarSlider:
            QCFType<HIShapeRef> shape;
            HIThemeGetTrackThumbShape(&tdi, &shape);
            HIShapeGetBounds(shape, &macRect);
            rect = qrectForHIRect(macRect);
            break;
        }
        break; }
    case CC_SpinWidget: {
        const int spinner_w = 10,
                spinner_h = 15;
        int fw = pixelMetric(PM_SpinBoxFrameWidth, widget),
            y = fw,
            x = widget->width() - fw - spinner_w;
        switch(sc) {
            case SC_SpinWidgetUp:
                rect = QRect(x, y + ((widget->height() - fw * 2) / 2 - spinner_h),
                             spinner_w, spinner_h);
                break;
            case SC_SpinWidgetDown:
                rect = QRect(x, y + (widget->height() - fw * 2) / 2, spinner_w, spinner_h);
                break;
            case SC_SpinWidgetButtonField:
                rect = QRect(x, y, spinner_w, widget->height() - fw * 2);
                break;
            case SC_SpinWidgetEditField:
                rect = QRect(fw, fw, widget->width() - spinner_w - fw * 2 - macSpinBoxSep,
                             widget->height() - fw * 2);
                break;
            case SC_SpinWidgetFrame:
                rect = QRect(0, 0, widget->width() - spinner_w - macSpinBoxSep, widget->height());
                break;
            default:
                break;
        }
        break; }
    case CC_TitleBar: {
        /*
        const QTitleBar *titlebar = static_cast<const QTitleBar *>(widget);
        HIThemeWindowDrawInfo wdi;
        wdi.version = qt_mac_hitheme_version;
        wdi.state = kThemeStateActive;
        wdi.windowType = QtWinType;
        if (titlebar->window()) {
            if (titlebar->window()->isMinimized())
                wdi.attributes |= kThemeWindowIsCollapsed;
            if (titlebar->window()->isWindowModified())
                wdi.attributes |= kThemeWindowHasDirty;
            wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                | kThemeWindowHasCollapseBox;
        } else if (titlebar->testWFlags(WStyle_SysMenu)) {
            wdi.attributes |= kThemeWindowHasCloseBox;
        }
        wdi.titleHeight = titlebar->height();
        wdi.titleWidth = titlebar->width();
        WindowRegionCode wrc = kWindowGlobalPortRgn;
        if(sc & SC_TitleBarCloseButton)
            wrc = kWindowCloseBoxRgn;
        else if(sc & SC_TitleBarMinButton)
            wrc = kWindowCollapseBoxRgn;
        else if(sc & SC_TitleBarMaxButton)
            wrc = kWindowZoomBoxRgn;
        else if(sc & SC_TitleBarLabel)
            wrc = kWindowTitleTextRgn;
        else if(sc & SC_TitleBarSysMenu)
            return QRect(0, 0, 10, pixelMetric(PM_TitleBarHeight, 0));
        if (wrc != kWindowGlobalPortRgn) {
            QRect tmpRect = widget->rect();
            HIShapeRef region;
            HIThemeGetWindowShape(qt_glb_mac_rect(tmpRect), &wdi, wrc, &region);
            HIRect titleRect;
            HIShapeGetBounds(region, &titleRect);
            CFRelease(region);
            tmpRect.moveBy(tmpRect.x() - int(titleRect.origin.x),
                           tmpRect.y() - int(titleRect.origin.y));
            HIThemeGetWindowShape(qt_glb_mac_rect(tmpRect), &wdi, wrc, &region);
            HIShapeGetBounds(region, &titleRect);
            CFRelease(region);
            rect = qrectForHIRect(titleRect);
        }
        break; */}
    case CC_ComboBox:
        if (static_cast<const QComboBox *>(widget)->editable()) {
            if (sc == SC_ComboBoxEditField)
                rect = QRect(0, 0, widget->width() - 20, widget->height());
            else if (sc == SC_ComboBoxArrow)
                rect = QRect(widget->width() - 24, 0, 24, widget->height());
            break;
        }
        // Fall through to the default case.
    default:
        rect = QWindowsStyle::querySubControlMetrics(control, widget, sc, opt);
    }
    return rect;
}

QRect QMacStyleCG::subRect(SubRect sr, const QWidget *widget) const
{
    QRect subrect;
    switch (sr) {
        case SR_PushButtonContents: {
            CGRect inRect = CGRectMake(0, 0, widget->width(), widget->height());
            CGRect outRect;
            HIThemeButtonDrawInfo info;
            info.version = qt_mac_hitheme_version;
            info.state = kThemeStateActive;
            info.kind = kThemePushButton;
            info.adornment = kThemeAdornmentNone;
            HIThemeGetButtonContentBounds(&inRect, &info, &outRect);
            subrect = qrectForHIRect(outRect);
            break;
        }
        default:
            subrect = QWindowsStyle::subRect(sr, widget);
    }
    return subrect;
}

QStyle::SubControl QMacStyleCG::querySubControl(ComplexControl control, const QWidget *widget,
                                                const QPoint &pos, const QStyleOption &opt) const
{
    SubControl sc = SC_None;
    switch (control) {
    case CC_Slider: {
        const QSlider *slider = static_cast<const QSlider *>(widget);
        HIThemeTrackDrawInfo tdi = *getTrackDrawInfo(control, slider);
        ControlPartCode part;
        HIPoint pt = {(float)pos.x(), (float)pos.y()};
        if (HIThemeHitTestTrack(&tdi, &pt, &part)) {
            if (part == kControlPageUpPart || part == kControlPageDownPart)
                sc = SC_SliderGroove;
            else // if (part == 129)
                sc = SC_SliderHandle;
        }
        break; }
    case CC_ScrollBar: {
        const QScrollBar *scrollbar = static_cast<const QScrollBar *>(widget);
        HIScrollBarTrackInfo sbi;
        sbi.version = qt_mac_hitheme_version;
        if(!qAquaActive (widget->palette()))
            sbi.enableState = kThemeTrackInactive;
        else if (!widget->isEnabled())
            sbi.enableState = kThemeTrackDisabled;
        else
            sbi.enableState = kThemeTrackActive;
        sbi.viewsize = scrollbar->pageStep();
        HIPoint pt = {(float)pos.x(), (float)pos.y()};
        HIRect macSBRect = *qt_glb_mac_rect(widget->rect());
        ControlPartCode part;
        if (HIThemeHitTestScrollBarArrows(&macSBRect, &sbi, scrollbar->orientation() == Horizontal,
                                          &pt, 0, &part)) {
            if (part == kControlUpButtonPart)
                sc = SC_ScrollBarSubLine;
            else if (part == kControlDownButtonPart)
                sc = SC_ScrollBarAddLine;

        } else {
            HIThemeTrackDrawInfo tdi = *getTrackDrawInfo(control, scrollbar);
            if (HIThemeHitTestTrack(&tdi, &pt, &part)) {
                if (part == kControlPageUpPart)
                    sc = SC_ScrollBarSubPage;
                else if (part == kControlPageDownPart)
                    sc = SC_ScrollBarAddPage;
                else
                    sc = SC_ScrollBarSlider;
            }
        }
        break; }
    default:
        sc = QWindowsStyle::querySubControl(control, widget, pos, opt);
    }
    return sc;
}

int QMacStyleCG::styleHint(StyleHint sh, const QWidget *widget, const QStyleOption &opt,
                           QStyleHintReturn *d) const
{
    SInt32 ret = 0;
    switch (sh) {
    case SH_Widget_ShareActivation:
        ret = true;
        break;
    case SH_UnderlineAccelerator:
        ret = false;
        break;
    case SH_TabBar_PreferNoArrows:
        ret = true;
        break;
    case SH_TabBar_Alignment:
        ret = Qt::AlignHCenter;
        break;
    case SH_TabBar_SelectMouseType:
    case SH_ListViewExpand_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_ComboBox_Popup:
        ret = (!widget || !static_cast<const QComboBox *>(widget)->editable());
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, widget, opt, d);
    }
    return ret;
}

QSize QMacStyleCG::sizeFromContents(ContentsType contents, const QWidget *widget,
                                    const QSize &contentsSize, const QStyleOption &opt) const
{
    QSize sz = contentsSize;
    switch (contents) {
    case CT_SpinBox:
        sz.setWidth(sz.width() + macSpinBoxSep); //leave space between the spinner and the editor
        break;
    case CT_TabWidget:
        sz.setWidth(sz.width() + 15); //leave a little bit of space around the tabs.
        break;
    case CT_TabBarTab: {
        SInt32 tabh = sz.height();
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge: {
            GetThemeMetric(kThemeLargeTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        case QAquaSizeMini:
#if QT_MACOSX_VERSION >= 0x1030
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniTabHeight, &tabh);
                SInt32 overlap;
                GetThemeMetric(kThemeMetricMiniTabFrameOverlap, &overlap);
                tabh += overlap;
                break;
            }
#endif
        case QAquaSizeSmall: {
            GetThemeMetric(kThemeSmallTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricSmallTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        }
        if(sz.height() > tabh)
            sz.setHeight(tabh);
        break; }
    case CT_PushButton:
        sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
        sz = QSize(sz.width() + 16, sz.height()); // I don't know why.
        break;
    default:
        sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
    }
    {
        QSize macsz;
        if(qt_aqua_size_constrain(widget, contents, sz, &macsz) != QAquaSizeUnknown) {
            if(macsz.width() != -1)
                sz.setWidth(macsz.width());
            if(macsz.height() != -1)
                sz.setHeight(macsz.height());
        }
    }
    return sz;
}

QPixmap QMacStyleCG::stylePixmap(StylePixmap sp, const QWidget *widget,
                                 const QStyleOption &opt) const
{
    IconRef icon = 0;
    switch (sp) {
        case SP_MessageBoxQuestion:
        case SP_MessageBoxInformation:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertNoteIcon, &icon);
            break;
        case SP_MessageBoxWarning:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertCautionIcon, &icon);
            break;
        case SP_MessageBoxCritical:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertStopIcon, &icon);
            break;
        default:
            break;
    }
    if (icon) {
        QPixmap ret = qt_mac_convert_iconref(icon, 64, 64);
        ReleaseIconRef(icon);
        return ret;
    }
    return QWindowsStyle::stylePixmap(sp, widget, opt);
}

QPixmap QMacStyleCG::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                                 const QPalette &pal, const QStyleOption &opt) const
{
    return QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
}

#endif
