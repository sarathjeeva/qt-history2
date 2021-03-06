/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_DECLARE_CLASS(QComboBox)
QT_DECLARE_CLASS(QLabel)
QT_DECLARE_CLASS(QSpinBox)
class RenderArea;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private slots:
    void fillRuleChanged();
    void fillGradientChanged();
    void penColorChanged();

private:
    void populateWithColors(QComboBox *comboBox);
    QVariant currentItemData(QComboBox *comboBox);

    enum { NumRenderAreas = 9 };

    RenderArea *renderAreas[NumRenderAreas];
    QLabel *fillRuleLabel;
    QLabel *fillGradientLabel;
    QLabel *fillToLabel;
    QLabel *penWidthLabel;
    QLabel *penColorLabel;
    QLabel *rotationAngleLabel;
    QComboBox *fillRuleComboBox;
    QComboBox *fillColor1ComboBox;
    QComboBox *fillColor2ComboBox;
    QSpinBox *penWidthSpinBox;
    QComboBox *penColorComboBox;
    QSpinBox *rotationAngleSpinBox;
};

#endif
