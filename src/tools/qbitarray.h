#ifndef QBITARRAY_H
#define QBITARRAY_H

#ifndef QT_H
#include "qbytearray.h"
#endif // QT_H

class QBitRef;
class Q_CORE_EXPORT QBitArray
{
    friend Q_CORE_EXPORT QDataStream &operator<<( QDataStream &, const QBitArray & );
    friend Q_CORE_EXPORT QDataStream &operator>>( QDataStream &, QBitArray & );
    QByteArray d;
public:
    inline QBitArray(){};
    QBitArray(int size, bool val = false);
    inline QBitArray &operator=(const QBitArray &other) { d = other.d; return *this; }


    inline int size() const { return (d.size() << 3) - *d.constData(); }
    inline int count() const { return (d.size() << 3) - *d.constData(); }

    inline bool isEmpty() const { return d.isEmpty(); }

    void resize(int size);

    inline void  detach() { d.detach(); }
    inline bool isDetached() const { return d.isDetached(); }

    bool testBit(int i) const;
    void setBit(int i);
    void setBit(int i, bool val);
    void clearBit(int i);
    bool toggleBit(int i);

    bool at(int i) const;
    QBitRef operator[](int i);
    bool operator[](int i) const;
    QBitRef operator[](uint i);
    bool operator[](uint i) const;

    QBitArray& operator&=(const QBitArray &);
    QBitArray& operator|=(const QBitArray &);
    QBitArray& operator^=(const QBitArray &);
    QBitArray  operator~() const;

    inline bool operator==(const QBitArray& a) const { return d == a.d; }
    inline bool operator!=(const QBitArray& a) const { return d != a.d; }

    inline bool isNull() { return d.isNull(); }

    inline bool fill(bool val, int size = -1)
	{ *this=QBitArray((size < 0 ? this->size() : size), val); return true; }

    inline bool ensure_constructed()
    { return d.ensure_constructed(); }
};

QBitArray operator&(const QBitArray &, const QBitArray &);
QBitArray operator|(const QBitArray &, const QBitArray &);
QBitArray operator^(const QBitArray &, const QBitArray &);

inline bool QBitArray::testBit(int i) const
{ Q_ASSERT(i >= 0); if (i >= size()) return false;
 return (*((uchar*)d.constData()+1+(i>>3)) & (1 << (i & 7))) != 0; }

inline void QBitArray::setBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 *((uchar*)d.data()+1+(i>>3)) |= (1 << (i & 7)); }

inline void QBitArray::clearBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 *((uchar*)d.data()+1+(i>>3)) &= ~(1 << (i & 7)); }

inline void QBitArray::setBit(int i, bool val)
{ if (val) setBit(i); else clearBit(i); }

inline bool QBitArray::toggleBit(int i)
{ Q_ASSERT(i >= 0); if (i >= size()) resize(i+1);
 uchar b = 1<< (i&7); uchar* p = (uchar*)d.data()+1+(i>>3);
 uchar c = *p&b; *p^=b; return c!=0; }

inline bool QBitArray::operator[](int i) const { return testBit(i); }
inline bool QBitArray::operator[](uint i) const { return testBit(i); }
inline bool QBitArray::at(int i) const { return testBit(i); }

class Q_CORE_EXPORT QBitRef
{
private:
    QBitArray& a;
    int i;
    inline QBitRef(QBitArray& array, int idx) : a(array), i(idx) {}
    friend class QBitArray;
public:
    inline operator bool() const { return a.testBit(i); }
    QBitRef& operator=(const QBitRef& val) { a.setBit(i, val); return *this; }
    QBitRef& operator=(bool val) { a.setBit(i, val); return *this; }
};

inline QBitRef QBitArray::operator[](int i)
{ Q_ASSERT(i >= 0); return QBitRef(*this, i); }
inline QBitRef QBitArray::operator[](uint i)
{ return QBitRef(*this, i); }


#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<( QDataStream &, const QBitArray & );
Q_CORE_EXPORT QDataStream &operator>>( QDataStream &, QBitArray & );
#endif

Q_DECLARE_TYPEINFO(QBitArray, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QBitArray);

#endif // QBITARRAY_H
