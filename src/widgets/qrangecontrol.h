/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangecontrol.h#18 $
**
** Definition of QRangeControl class
**
** Created : 940427
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QRANGECONTROL_H
#define QRANGECONTROL_H


class Q_EXPORT QRangeControl
{
public:
    QRangeControl();
    QRangeControl( int minValue, int maxValue,
		   int lineStep, int pageStep, int value );

    int		value()		const;
    virtual void	setValue( int );
    void	addPage();
    void	subtractPage();
    void	addLine();
    void	subtractLine();

    int		minValue()	const;
    int		maxValue()	const;
    virtual void	setRange( int minValue, int maxValue );

    int		lineStep()	const;
    int		pageStep()	const;
    virtual void	setSteps( int line, int page );

protected:
    void	directSetValue( int val );
    int		prevValue()	const;

    virtual void valueChange();
    virtual void rangeChange();
    virtual void stepChange();

private:
    void	adjustValue();

    int		minVal, maxVal;
    int		line, page;
    int		val, prevVal;

private:	// Disabled copy constructor and operator=
    QRangeControl( const QRangeControl & );
    QRangeControl &operator=( const QRangeControl & );
};


inline int QRangeControl::value() const
{ return val; }

inline int QRangeControl::prevValue() const
{ return prevVal; }

inline int QRangeControl::minValue() const
{ return minVal; }

inline int QRangeControl::maxValue() const
{ return maxVal; }

inline int QRangeControl::lineStep() const
{ return line; }

inline int QRangeControl::pageStep() const
{ return page; }


#endif // QRANGECONTROL_H
