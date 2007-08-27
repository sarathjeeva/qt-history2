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

#ifndef QATOMIC_S390_H
#define QATOMIC_S390_H

QT_BEGIN_HEADER

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{ return true; }
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{ return false; }

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{ return true; }
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{ return false; }

#define __CS_LOOP(ptr, op_val, op_string, pre, post) ({                 \
	volatile int old_val, new_val;					\
        __asm__ __volatile__(pre                                        \
                             "   l     %0,0(%3)\n"                      \
                             "0: lr    %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   cs    %0,%1,0(%3)\n"			\
                             "   jl    0b\n"				\
                             post                                       \
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	new_val;							\
})

#define __CS_OLD_LOOP(ptr, op_val, op_string, pre, post ) ({            \
	volatile int old_val, new_val;					\
        __asm__ __volatile__(pre                                        \
                             "   l     %0,0(%3)\n"			\
                             "0: lr    %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   cs    %0,%1,0(%3)\n"			\
                             "   jl    0b\n"				\
                             post                                       \
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	old_val;							\
})

#ifdef __s390x__
#define __CSG_OLD_LOOP(ptr, op_val, op_string, pre, post) ({            \
	long old_val, new_val;						\
        __asm__ __volatile__(pre                                        \
                             "   lg    %0,0(%3)\n"                      \
                             "0: lgr   %1,%0\n"				\
                             op_string "  %1,%4\n"			\
                             "   csg   %0,%1,0(%3)\n"			\
                             "   jl    0b\n"				\
                             post                                       \
                             : "=&d" (old_val), "=&d" (new_val),	\
			       "=m" (*ptr)	\
			     : "a" (ptr), "d" (op_val),			\
			       "m" (*ptr)	\
			     : "cc", "memory" );			\
	old_val;							\
})
#endif

inline bool QBasicAtomicInt::ref()
{
    return __CS_LOOP(ptr, 1, "ar", "", "") != 0;
}

inline bool QBasicAtomicInt::deref()
{
    return __CS_LOOP(ptr, 1, "sr", "", "") != 0;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
    int retval;
    __asm__ __volatile__(
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expectedValue) , "d" (newValue),
                           "m" (_q_value) : "cc", "memory" );
    return retval == 0;
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
    int retval;
    __asm__ __volatile__(
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:\n"
                         "  bcr 15,0\n"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expectedValue) , "d" (newValue),
                           "m" (_q_value) : "cc", "memory" );
    return retval == 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    int retval;
    __asm__ __volatile__(
                         "  bcr 15,0\n"
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expectedValue) , "d" (newValue),
                           "m" (_q_value) : "cc", "memory" );
    return retval == 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    return __CS_OLD_LOOP(ptr, newval, "lr", "", "");
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    return __CS_OLD_LOOP(ptr, newval, "lr", "", "bcr 15,0\n");
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    return __CS_OLD_LOOP(ptr, newval, "lr", "bcr 15,0\n", "");
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    return fetchAndStoreAcquire(newValue);
}

#error "QBasicAtomicInt::fetchAndAdd*() not implemented"
/*
inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
}
*/

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    int retval;

#ifndef __s390x__
    __asm__ __volatile__(
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#else
    __asm__ __volatile__(
                         "  lgr   %0,%3\n"
                         "  csg   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#endif

    return retval == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    int retval;

#ifndef __s390x__
    __asm__ __volatile__(
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:\n"
                         "  bcr 15,0\n"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#else
    __asm__ __volatile__(
                         "  lgr   %0,%3\n"
                         "  csg   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:\n"
                         "  bcr 15,0\n"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#endif

    return retval == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    int retval;

#ifndef __s390x__
    __asm__ __volatile__(
                         "  bcr 15,0\n"
                         "  lr   %0,%3\n"
                         "  cs   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#else
    __asm__ __volatile__(
                         "  bcr 15,0\n"
                         "  lgr   %0,%3\n"
                         "  csg   %0,%4,0(%2)\n"
                         "  ipm  %0\n"
                         "  srl  %0,28\n"
                         "0:"
                         : "=&d" (retval), "=m" (_q_value)
                         : "a" (&_q_value), "d" (expected) , "d" (newval),
                           "m" (_q_value) : "cc", "memory" );
#endif

    return retval == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    return testAndSetAcquire(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T* QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
#ifndef __s390x__
    return (T*)__CS_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (int)newValue, "lr",
                             "", "bcr 15,0\n");
#else
    return (T*)__CSG_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (long)newValue, "lgr",
                              "", "bcr 15,0\n");
#endif
}

template <typename T>
Q_INLINE_TEMPLATE T* QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
#ifndef __s390x__
    return (T*)__CS_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (int)newValue, "lr", "", "");
#else
    return (T*)__CSG_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (long)newValue, "lgr", "", "");
#endif
}

template <typename T>
Q_INLINE_TEMPLATE T* QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
#ifndef __s390x__
    return (T*)__CS_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (int)newValue, "lr",
                             "bcr 15,0 \n", "");
#else
    return (T*)__CSG_OLD_LOOP(reinterpret_cast<volatile long*>(ptr), (long)newValue, "lgr",
                              "bcr 15,0\n", "");
#endif
}

template <typename T>
Q_INLINE_TEMPLATE T* QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    return fetchAndStoreAcquire(newValue);
}

#error "QBasicAtomicPointer<T>::fetchAndAdd*() not implemented"
/*
template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
}

template <typename T>
Q_INLINE_TEMPLATE
T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
}
*/

QT_END_HEADER

#endif // QATOMIC_S390_H
