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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include "qglobal.h"

// Documentation is in qabstractlayout.cpp.

class Q_GUI_EXPORT QSizePolicy
{
private:
    enum { HSize = 6, HMask = 0x3f, VMask = HMask << HSize,
           MayGrow = 1, ExpMask = 2, MayShrink = 4 };
public:
    enum SizeTypeFlag { Fixed = 0,
                    Minimum = MayGrow,
                    Maximum = MayShrink,
                    Preferred = MayGrow | MayShrink,
                    MinimumExpanding = MayGrow | ExpMask,
                    Expanding = MayGrow | MayShrink | ExpMask,
                    Ignored = ExpMask /* magic value */ };

    Q_DECLARE_FLAGS(SizeType, SizeTypeFlag);

    enum ExpandData { NoDirection = 0,
                      Horizontally = 1,
                      Vertically = 2,
#ifdef QT_COMPAT
                      Horizontal = Horizontally,
                      Vertical = Vertically,
#endif
                      BothDirections = Horizontally | Vertically };

    QSizePolicy() : data(0) { }

    QSizePolicy(SizeType hor, SizeType ver, bool hfw = false)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) { }
    QSizePolicy(SizeType hor, SizeType ver, uchar hors, uchar vers, bool hfw = false);

    SizeType horData() const { return static_cast<SizeTypeFlag>(data & HMask); }
    SizeType verData() const { return static_cast<SizeTypeFlag>((data & VMask) >> HSize); }

    bool mayShrinkHorizontally() const { return horData() & MayShrink || horData() == Ignored; }
    bool mayShrinkVertically() const { return verData() & MayShrink || verData() == Ignored; }
    bool mayGrowHorizontally() const { return horData() & MayGrow || horData() == Ignored; }
    bool mayGrowVertically() const { return verData() & MayGrow || verData() == Ignored; }

    ExpandData expanding() const
    {
        return static_cast<ExpandData>(int(verData() & ExpMask ? Vertically : 0)
                                       | int(horData() & ExpMask ? Horizontally : 0));
    }

    void setHorData(SizeType d) { data = (data & ~HMask) | d; }
    void setVerData(SizeType d) { data = (data & ~(HMask << HSize)) | (d << HSize); }

    void setHeightForWidth(bool b) { data = b ? (data | (1 << 2*HSize)) : (data & ~(1 << 2*HSize));  }
    bool hasHeightForWidth() const { return data & (1 << 2*HSize); }

    bool operator==(const QSizePolicy& s) const { return data == s.data; }
    bool operator!=(const QSizePolicy& s) const { return data != s.data; }


    uint horStretch() const { return data >> 24; }
    uint verStretch() const { return (data >> 16) & 0xff; }
    void setHorStretch(uchar sf) { data = (data&0x00ffffff) | (uint(sf)<<24); }
    void setVerStretch(uchar sf) { data = (data&0xff00ffff) | (uint(sf)<<16); }
    inline void transpose();

private:
    QSizePolicy(int i) : data(i) { }

    Q_UINT32 data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSizePolicy::SizeType);

inline QSizePolicy::QSizePolicy(SizeType hor, SizeType ver, uchar hors, uchar vers, bool hfw)
    : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) {
    setHorStretch(hors);
    setVerStretch(vers);
}

inline void QSizePolicy::transpose() {
    *this = QSizePolicy(verData(), horData(), verStretch(), horStretch(),
                         hasHeightForWidth());
}

#endif // QSIZEPOLICY_H
