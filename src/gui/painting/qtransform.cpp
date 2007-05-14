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
#include "qtransform.h"

#include "qdatastream.h"
#include "qdebug.h"
#include "qmath_p.h"
#include "qmatrix.h"
#include "qregion.h"
#include "qpainterpath.h"
#include "qvariant.h"

#include <math.h>

#define MAPDOUBLE(x, y, nx, ny) \
{ \
    fx = x; \
    fy = y; \
    nx = affine._m11*fx + affine._m21*fy + affine._dx; \
    ny = affine._m12*fx + affine._m22*fy + affine._dy; \
    if (!isAffine()) { \
        qreal w = m_13*fx + m_23*fy + m_33; \
        w = 1/w; \
        nx *= w; \
        ny *= w; \
    }\
}

#define MAPINT(x, y, nx, ny) \
{ \
    fx = x; \
    fy = y; \
    nx = int(affine._m11*fx + affine._m21*fy + affine._dx); \
    ny = int(affine._m12*fx + affine._m22*fy + affine._dy); \
    if (!isAffine()) { \
        qreal w = m_13*fx + m_23*fy + m_33; \
        w = 1/w; \
        nx = int(nx*w); \
        ny = int(ny*w); \
    }\
}


/*!
    \class QTransform
    \brief The QTransform class specifies 2D transformations of a coordinate system.
    \since 4.3
    \ingroup multimedia

    A transformation specifies how to translate, scale, shear, rotate
    or project the coordinate system, and is typically used when
    rendering graphics.

    QTransform differs from QMatrix in that it is a true 3x3 matrix,
    allowing perpective transformations. QTransform's toAffine()
    method allows casting QTransform to QMatrix. If a perspective
    transformation has been specified on the matrix, then the
    conversion to an affine QMatrix will cause loss of data.

    QTransform is the recommended transformation class in Qt.

    A QTransform object can be built using the setMatrix(), scale(),
    rotate(), translate() and shear() functions.  Alternatively, it
    can be built by applying \l {QTransform#Basic Matrix
    Operations}{basic matrix operations}. The matrix can also be
    defined when constructed, and it can be reset to the identity
    matrix (the default) using the reset() function.

    The QTransform class supports mapping of graphic primitives: A given
    point, line, polygon, region, or painter path can be mapped to the
    coordinate system defined by \e this matrix using the map()
    function. In case of a rectangle, its coordinates can be
    transformed using the mapRect() function. A rectangle can also be
    transformed into a \e polygon (mapped to the coordinate system
    defined by \e this matrix), using the mapToPolygon() function.

    QTransform provides the isIdentity() function which returns true if
    the matrix is the identity matrix, and the isInvertible() function
    which returns true if the matrix is non-singular (i.e. AB = BA =
    I). The inverted() function returns an inverted copy of \e this
    matrix if it is invertible (otherwise it returns the identity
    matrix). In addition, QTransform provides the det() function
    returning the matrix's determinant.

    Finally, the QTransform class supports matrix multiplication, and
    objects of the class can be streamed as well as compared.

    \tableofcontents

    \section1 Rendering Graphics

    When rendering graphics, the matrix defines the transformations
    but the actual transformation is performed by the drawing routines
    in QPainter.

    By default, QPainter operates on the associated device's own
    coordinate system.  The standard coordinate system of a
    QPaintDevice has its origin located at the top-left position. The
    \e x values increase to the right; \e y values increase
    downward. For a complete description, see the \l {The Coordinate
    System}{coordinate system} documentation.

    QPainter has functions to translate, scale, shear and rotate the
    coordinate system without using a QTransform. For example:

    \table 100%
    \row
    \o \inlineimage qmatrix-simpletransformation
    \o
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto SimpleTransformation::paintEvent
    \printuntil }
    \endtable

    Although these functions are very convenient, it can be more
    efficient to build a QTransform and call QPainter::setMatrix() if you
    want to perform more than a single transform operation. For
    example:

    \table 100%
    \row
    \o \inlineimage qmatrix-combinedtransformation.png
    \o
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto CombinedTransformation::paintEvent
    \printuntil }
    \endtable

    \section1 Basic Matrix Operations

    \image qmatrix-representation.png

    A QTransform object contains a 3 x 3 matrix.  The \c dx and \c dy
    elements specify horizontal and vertical translation. The \c m11
    and \c m22 elements specify horizontal and vertical scaling. And
    finally, the \c m21 and \c m12 elements specify horizontal and
    vertical \e shearing.

    QTransform transforms a point in the plane to another point using the
    following formulas:

    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
    \endcode

    The point \e (x, y) is the original point, and \e (x', y') is the
    transformed point. \e (x', y') can be transformed back to \e (x,
    y) by performing the same operation on the inverted() matrix.

    The various matrix elements can be set when constructing the
    matrix, or by using the setMatrix() function later on. They also
    be manipulated using the translate(), rotate(), scale() and
    shear() convenience functions, The currently set values can be
    retrieved using the m11(), m12(), m21(), m22(), dx() and dy()
    functions.

    Translation is the simplest transformation. Setting \c dx and \c
    dy will move the coordinate system \c dx units along the X axis
    and \c dy units along the Y axis.  Scaling can be done by setting
    \c m11 and \c m22. For example, setting \c m11 to 2 and \c m22 to
    1.5 will double the height and increase the width by 50%.  The
    identity matrix has \c m11 and \c m22 set to 1 (all others are set
    to 0) mapping a point to itself. Shearing is controlled by \c m12
    and \c m21. Setting these elements to values different from zero
    will twist the coordinate system. Rotation is achieved by
    carefully setting both the shearing factors and the scaling
    factors.

    Here's the combined transformations example using basic matrix
    operations:

    \table 100%
    \row
    \o \inlineimage qmatrix-combinedtransformation.png
    \o
    \quotefromfile snippets/matrix/matrix.cpp
    \skipto BasicOperations::paintEvent
    \printuntil }
    \endtable

    \sa QPainter, {The Coordinate System}, {demos/affine}{Affine
    Transformations Demo}, {Transformations Example}
*/

/*!
    \enum QTransform::TransformationType

    \value TxNone
    \value TxTranslate
    \value TxScale
    \value TxRotate
    \value TxShear
    \value TxProject
*/

/*!
    Constructs an identity matrix.

    All elements are set to zero except \c m11 and \c m22 (specifying
    the scale) and \c m13 which are set to 1.

    \sa reset()
*/
QTransform::QTransform()
    : m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxNone)
{

}

/*!
    Constructs a matrix with the elements, \a h11, \a h12, \a h13,
    \a h21, \a h22, \a h23, \a h31, \a h32, \a h33.

    \sa setMatrix()
*/
QTransform::QTransform(qreal h11, qreal h12, qreal h13,
                       qreal h21, qreal h22, qreal h23,
                       qreal h31, qreal h32, qreal h33)
    : affine(h11, h12, h21, h22, h31, h32),
      m_13(h13), m_23(h23), m_33(h33)
    , m_type(TxNone)
    , m_dirty(TxProject)
{

}

/*!
    Constructs a matrix with the elements, \a h11, \a h12, \a h21, \a
    h22, \a dx and \a dy.

    \sa setMatrix()
*/
QTransform::QTransform(qreal h11, qreal h12, qreal h21,
                       qreal h22, qreal dx, qreal dy)
    : affine(h11, h12, h21, h22, dx, dy),
      m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxShear)
{

}

/*!
    \fn QTransform::QTransform(const QMatrix &matrix)

    Constructs a matrix that is a copy of the given \a matrix.
    Note that the \c m13, \c m23, and \c m33 elements are set to 0, 0,
    and 1 respectively.
 */
QTransform::QTransform(const QMatrix &mtx)
    : affine(mtx),
      m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxShear)
{

}

/*!
    Returns the adjoint of this matrix.
*/
QTransform QTransform::adjoint() const
{
    qreal h11, h12, h13,
        h21, h22, h23,
        h31, h32, h33;
    h11 = affine._m22*m_33 - m_23*affine._dy;
    h21 = m_23*affine._dx - affine._m21*m_33;
    h31 = affine._m21*affine._dy - affine._m22*affine._dx;
    h12 = m_13*affine._dy - affine._m12*m_33;
    h22 = affine._m11*m_33 - m_13*affine._dx;
    h32 = affine._m12*affine._dx - affine._m11*affine._dy;
    h13 = affine._m12*m_23 - m_13*affine._m22;
    h23 = m_13*affine._m21 - affine._m11*m_23;
    h33 = affine._m11*affine._m22 - affine._m12*affine._m21;
    //### not a huge fan of this simplification but
    //    i'd like to keep m33 as 1.0
    //return QTransform(h11, h12, h13,
    //                  h21, h22, h23,
    //                  h31, h32, h33);
    h33 = 1/h33;
    return QTransform(h11*h33, h12*h33, h13*h33,
                      h21*h33, h22*h33, h23*h33,
                      h31*h33, h32*h33, 1.0);
}

/*!
    Returns the transpose of this matrix.
*/
QTransform QTransform::transposed() const
{
    return QTransform(affine._m11, affine._m21, affine._dx,
                      affine._m12, affine._m22, affine._dy,
                      m_13, m_23, m_33);
}

/*!
    Returns an inverted copy of this matrix.

    If the matrix is singular (not invertible), the returned matrix is
    the identity matrix. If \a invertible is valid (i.e. not 0), its
    value is set to true if the matrix is invertible, otherwise it is
    set to false.

    \sa isInvertible()
*/
QTransform QTransform::inverted(bool *invertible) const
{
    qreal det = determinant();
    if (qFuzzyCompare(det, qreal(0.0))) {
        if (invertible)
            *invertible = false;
        return QTransform();
    }
    if (invertible)
        *invertible = true;
    QTransform adjA = adjoint();
    QTransform invert = adjA / det;
    invert = QTransform(invert.m11()/invert.m33(), invert.m12()/invert.m33(), invert.m13()/invert.m33(),
                        invert.m21()/invert.m33(), invert.m22()/invert.m33(), invert.m23()/invert.m33(),
                        invert.m31()/invert.m33(), invert.m32()/invert.m33(), 1);
    // inverting doesn't change the type
    invert.m_type = m_type;
    invert.m_dirty = m_dirty;
    return invert;
}

/*!
    Moves the coordinate system \a dx along the x axis and \a dy along
    the y axis, and returns a reference to the matrix.

    \sa setMatrix()
*/
QTransform & QTransform::translate(qreal dx, qreal dy)
{
    affine._dx += dx*affine._m11 + dy*affine._m21;
    affine._dy += dy*affine._m22 + dx*affine._m12;
    m_dirty |= TxTranslate;
    return *this;
}

/*!
    Scales the coordinate system by \a sx horizontally and \a sy
    vertically, and returns a reference to the matrix.

    \sa setMatrix()
*/
QTransform & QTransform::scale(qreal sx, qreal sy)
{
    affine._m11 *= sx;
    affine._m12 *= sx;
    affine._m21 *= sy;
    affine._m22 *= sy;
    m_dirty |= TxScale;
    return *this;
}

/*!
    Shears the coordinate system by \a sh horizontally and \a sv
    vertically, and returns a reference to the matrix.

    \sa setMatrix()
*/
QTransform & QTransform::shear(qreal sh, qreal sv)
{
    qreal tm11 = sv*affine._m21;
    qreal tm12 = sv*affine._m22;
    qreal tm21 = sh*affine._m11;
    qreal tm22 = sh*affine._m12;
    affine._m11 += tm11;
    affine._m12 += tm12;
    affine._m21 += tm21;
    affine._m22 += tm22;
    m_dirty |= TxShear;
    return *this;
}

const qreal deg2rad = qreal(0.017453292519943295769);        // pi/180
const qreal inv_dist_to_plane = 1. / 1024.;

/*!
    \fn QTransform &QTransform::rotate(qreal angle, Qt::Axis axis)
    
    Rotates the coordinate system counterclockwise by the given \a angle
    about the specified \a axis and returns a reference to the matrix.

    Note that if you apply a QTransform to a point defined in widget
    coordinates, the direction of the rotation will be clockwise
    because the y-axis points downwards.

    The angle is specified in degrees.

    \sa setMatrix()
*/
QTransform & QTransform::rotate(qreal a, Qt::Axis axis)
{
    qreal sina = 0;
    qreal cosa = 0;
    if (a == 90. || a == -270.)
        sina = 1.;
    else if (a == 270. || a == -90.)
        sina = -1.;
    else if (a == 180.)
        cosa = -1.;
    else{
        qreal b = deg2rad*a;          // convert to radians
        sina = qSin(b);               // fast and convenient
        cosa = qCos(b);
    }

    if (axis == Qt::ZAxis) {
        qreal tm11 = cosa*affine._m11 + sina*affine._m21;
        qreal tm12 = cosa*affine._m12 + sina*affine._m22;
        qreal tm21 = -sina*affine._m11 + cosa*affine._m21;
        qreal tm22 = -sina*affine._m12 + cosa*affine._m22;
        affine._m11 = tm11; affine._m12 = tm12;
        affine._m21 = tm21; affine._m22 = tm22;
        m_dirty |= TxRotate;
    } else {
        QTransform result;
        if (axis == Qt::YAxis) {
            result.affine._m11 = cosa;
            result.m_13 = -sina * inv_dist_to_plane;
        } else {
            result.affine._m22 = cosa;
            result.m_23 = -sina * inv_dist_to_plane;
        }
        m_dirty = TxProject;
        operator*=(result);
    }

    return *this;
}

/*!
    \fn QTransform & QTransform::rotateRadians(qreal angle, Qt::Axis axis)
    
    Rotates the coordinate system counterclockwise by the given \a angle
    about the specified \a axis and returns a reference to the matrix.
    
    Note that if you apply a QTransform to a point defined in widget
    coordinates, the direction of the rotation will be clockwise
    because the y-axis points downwards.

    The angle is specified in radians.

    \sa setMatrix()
*/
QTransform & QTransform::rotateRadians(qreal a, Qt::Axis axis)
{
    qreal sina = qSin(a);
    qreal cosa = qCos(a);

    if (axis == Qt::ZAxis) {
        qreal tm11 = cosa*affine._m11 + sina*affine._m21;
        qreal tm12 = cosa*affine._m12 + sina*affine._m22;
        qreal tm21 = -sina*affine._m11 + cosa*affine._m21;
        qreal tm22 = -sina*affine._m12 + cosa*affine._m22;
        affine._m11 = tm11; affine._m12 = tm12;
        affine._m21 = tm21; affine._m22 = tm22;
        m_dirty |= TxRotate;
    } else {
        QTransform result;
        if (axis == Qt::YAxis) {
            result.affine._m11 = cosa;
            result.m_13 = -sina * inv_dist_to_plane;
        } else {
            result.affine._m22 = cosa;
            result.m_23 = -sina * inv_dist_to_plane;
        }
        m_dirty = TxProject;
        operator*=(result);
    }
    return *this;
}

/*!
    \fn bool QTransform::operator==(const QTransform &matrix) const
    Returns true if this matrix is equal to the given \a matrix,
    otherwise returns false.
*/
bool QTransform::operator==(const QTransform &o) const
{
#define qFZ qFuzzyCompare
    return qFZ(affine._m11, o.affine._m11) &&  qFZ(affine._m12, o.affine._m12) &&  qFZ(m_13, o.m_13)
        && qFZ(affine._m21, o.affine._m21) &&  qFZ(affine._m22, o.affine._m22) &&  qFZ(m_23, o.m_23)
        && qFZ(affine._dx, o.affine._dx) &&  qFZ(affine._dy, o.affine._dy) &&  qFZ(m_33, o.m_33);
#undef qFZ
}

/*!
    \fn bool QTransform::operator!=(const QTransform &matrix) const
    Returns true if this matrix is not equal to the given \a matrix,
    otherwise returns false.
*/
bool QTransform::operator!=(const QTransform &o) const
{
    return !operator==(o);
}

/*!
    \fn QTransform & QTransform::operator*=(const QTransform &matrix)
    \overload

    Returns the result of multiplying this matrix by the given \a
    matrix.
*/
QTransform & QTransform::operator*=(const QTransform &o)
{
    qreal m11 = affine._m11*o.affine._m11 + affine._m12*o.affine._m21 + m_13*o.affine._dx;
    qreal m12 = affine._m11*o.affine._m12 + affine._m12*o.affine._m22 + m_13*o.affine._dy;
    qreal m13 = affine._m11*o.m_13 + affine._m12*o.m_23 + m_13*o.m_33;

    qreal m21 = affine._m21*o.affine._m11 + affine._m22*o.affine._m21 + m_23*o.affine._dx;
    qreal m22 = affine._m21*o.affine._m12 + affine._m22*o.affine._m22 + m_23*o.affine._dy;
    qreal m23 = affine._m21*o.m_13 + affine._m22*o.m_23 + m_23*o.m_33;

    qreal m31 = affine._dx*o.affine._m11 + affine._dy*o.affine._m21 + m_33*o.affine._dx;
    qreal m32 = affine._dx*o.affine._m12 + affine._dy*o.affine._m22 + m_33*o.affine._dy;
    qreal m33 = affine._dx*o.m_13 + affine._dy*o.m_23 + m_33*o.m_33;

    affine._m11 = m11/m33; affine._m12 = m12/m33; m_13 = m13/m33;
    affine._m21 = m21/m33; affine._m22 = m22/m33; m_23 = m23/m33;
    affine._dx = m31/m33; affine._dy = m32/m33; m_33 = 1.0;

    m_dirty = m_dirty | m_type | o.m_dirty | o.m_type;

    return *this;
}

/*!
    \fn QTransform QTransform::operator*(const QTransform &matrix) const
    Returns the result of multiplying this matrix by the given \a
    matrix.

    Note that matrix multiplication is not commutative, i.e. a*b !=
    b*a.
*/
QTransform QTransform::operator*(const QTransform &m) const
{
    QTransform result = *this;
    result *= m;
    return result;
}

/*!
    \fn QTransform & QTransform::operator*=(qreal scalar)
    \overload

    Returns the result of performing an element-wise multiplication of this
    matrix with the given \a scalar.
*/

/*!
    \fn QTransform & QTransform::operator/=(qreal scalar)
    \overload

    Returns the result of performing an element-wise division of this
    matrix by the given \a scalar.
*/

/*!
    \fn QTransform & QTransform::operator+=(qreal scalar)
    \overload

    Returns the matrix obtained by adding the given \a scalar to each
    element of this matrix.
*/

/*!
    \fn QTransform & QTransform::operator-=(qreal scalar)
    \overload

    Returns the matrix obtained by subtracting the given \a scalar from each
    element of this matrix.
*/

/*!
    Assigns the given \a matrix's values to this matrix.
*/
QTransform & QTransform::operator=(const QTransform &matrix)
{
    affine._m11 = matrix.affine._m11;
    affine._m12 = matrix.affine._m12;
    m_13 = matrix.m_13;
    affine._m21 = matrix.affine._m21;
    affine._m22 = matrix.affine._m22;
    m_23 = matrix.m_23;
    affine._dx = matrix.affine._dx;
    affine._dy = matrix.affine._dy;
    m_33 = matrix.m_33;
    m_type = matrix.m_type;
    m_dirty = matrix.m_dirty;

    return *this;
}

/*!
    Resets the matrix to an identity matrix, i.e. all elements are set
    to zero, except \c m11 and \c m22 (specifying the scale) which are
    set to 1.

    \sa QTransform(), isIdentity(), {QTransform#Basic Matrix
    Operations}{Basic Matrix Operations}
*/
void QTransform::reset()
{
    affine._m11 = affine._m22 = m_33 = 1.0;
    affine._m12 = m_13 = affine._m21 = m_23 = affine._dx = affine._dy = 0;
    m_type = TxNone;
    m_dirty = TxNone;
}

#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QTransform &matrix)
    \since 4.3
    \relates QTransform

    Writes the given \a matrix to the given \a stream and returns a
    reference to the stream.

    \sa {Format of the QDataStream Operators}
*/
QDataStream & operator<<(QDataStream &s, const QTransform &m)
{
    s << double(m.m11())
      << double(m.m12())
      << double(m.m13())
      << double(m.m21())
      << double(m.m22())
      << double(m.m23())
      << double(m.m31())
      << double(m.m32())
      << double(m.m33());
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QTransform &matrix)
    \since 4.3
    \relates QTransform

    Reads the given \a matrix from the given \a stream and returns a
    reference to the stream.

    \sa {Format of the QDataStream Operators}
*/
QDataStream & operator>>(QDataStream &s, QTransform &t)
{
     double m11, m12, m13,
         m21, m22, m23,
         m31, m32, m33;

     s >> m11;
     s >> m12;
     s >> m13;
     s >> m21;
     s >> m22;
     s >> m23;
     s >> m31;
     s >> m32;
     s >> m33;
     t.setMatrix(m11, m12, m13,
                 m21, m22, m23,
                 m31, m32, m33);
     return s;
}

#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QTransform &m)
{
    dbg.nospace() << "QTransform("
                  << "11="  << m.m11()
                  << " 12=" << m.m12()
                  << " 13=" << m.m13()
                  << " 21=" << m.m21()
                  << " 22=" << m.m22()
                  << " 23=" << m.m23()
                  << " 31=" << m.m31()
                  << " 32=" << m.m32()
                  << " 33=" << m.m33()
                  << ")";
    return dbg.space();
}
#endif

/*!
    \fn QPoint operator*(const QPoint &point, const QTransform &matrix)
    \relates QTransform

    This is the same as \a{matrix}.map(\a{point}).

    \sa QTransform::map()
*/
QPoint QTransform::map(const QPoint &p) const
{
    qreal fx = p.x();
    qreal fy = p.y();

    qreal x = affine._m11 * fx + affine._m21 * fy + affine._dx;
    qreal y = affine._m12 * fx + affine._m22 * fy + affine._dy;

    if (isAffine()) {
        return QPoint(qRound(x), qRound(y));
    } else {
        qreal w = m_13 * fx + m_23 * fy + m_33;
        return QPoint(qRound(x/w), qRound(y/w));
    }
}


/*!
    \fn QPointF operator*(const QPointF &point, const QTransform &matrix)
    \relates QTransform

    Same as \a{matrix}.map(\a{point}).

    \sa QTransform::map()
*/

/*!
    \overload

    Creates and returns a QPointF object that is a copy of the given point,
    \a p, mapped into the coordinate system defined by this matrix.
*/
QPointF QTransform::map(const QPointF &p) const
{
    qreal fx = p.x();
    qreal fy = p.y();

    qreal x = affine._m11 * fx + affine._m21 * fy + affine._dx;
    qreal y = affine._m12 * fx + affine._m22 * fy + affine._dy;

    if (isAffine()) {
        return QPointF(x, y);
    } else {
        qreal w = m_13 * fx + m_23 * fy + m_33;
        return QPointF(x/w, y/w);
    }
}

/*!
    \fn QPoint QTransform::map(const QPoint &point) const
    \overload

    Creates and returns a QPoint object that is a copy of the given \a
    point, mapped into the coordinate system defined by this
    matrix. Note that the transformed coordinates are rounded to the
    nearest integer.
*/

/*!
    \fn QLineF operator*(const QLineF &line, const QTransform &matrix)
    \relates QTransform

    This is the same as \a{matrix}.map(\a{line}).

    \sa QTransform::map()
*/

/*!
    \fn QLine operator*(const QLine &line, const QTransform &matrix)
    \relates QTransform

    This is the same as \a{matrix}.map(\a{line}).

    \sa QTransform::map()
*/

/*!
    \overload

    Creates and returns a QLineF object that is a copy of the given line,
    \a l, mapped into the coordinate system defined by this matrix.
*/
QLine QTransform::map(const QLine &l) const
{
    return QLine(map(l.p1()), map(l.p2()));
}

/*!
    \overload

    \fn QLineF QTransform::map(const QLineF &line) const
    
    Creates and returns a QLine object that is a copy of the given \a
    line, mapped into the coordinate system defined by this matrix.
    Note that the transformed coordinates are rounded to the nearest
    integer.
*/

QLineF QTransform::map(const QLineF &l) const
{
    return QLineF(map(l.p1()), map(l.p2()));
}

/*!
    \fn QPolygonF operator *(const QPolygonF &polygon, const QTransform &matrix)
    \since 4.3
    \relates QTransform

    This is the same as \a{matrix}.map(\a{polygon}).

    \sa QTransform::map()
*/

/*!
    \fn QPolygon operator*(const QPolygon &polygon, const QTransform &matrix)
    \relates QTransform

    This is the same as \a{matrix}.map(\a{polygon}).

    \sa QTransform::map()
*/

/*!
    \fn QPolygonF QTransform::map(const QPolygonF &polygon) const
    \overload

    Creates and returns a QPolygonF object that is a copy of the given
    \a polygon, mapped into the coordinate system defined by this
    matrix.
*/
QPolygonF QTransform::map(const QPolygonF &a) const
{
    int size = a.size();
    int i;
    QPolygonF p(size);
    const QPointF *da = a.constData();
    QPointF *dp = p.data();

    qreal fx, fy;
    for(i = 0; i < size; ++i) {
        MAPDOUBLE(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
    }
    return p;
}

/*!
    \fn QPolygon QTransform::map(const QPolygon &polygon) const
    \overload

    Creates and returns a QPolygon object that is a copy of the given
    \a polygon, mapped into the coordinate system defined by this
    matrix. Note that the transformed coordinates are rounded to the
    nearest integer.
*/
QPolygon QTransform::map(const QPolygon &a) const
{
    int size = a.size();
    int i;
    QPolygon p(size);
    const QPoint *da = a.constData();
    QPoint *dp = p.data();

    int fx, fy;
    for(i = 0; i < size; ++i) {
        MAPINT(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
    }
    return p;
}

/*!
    \fn QRegion operator*(const QRegion &region, const QTransform &matrix)
    \relates QTransform

    This is the same as \a{matrix}.map(\a{region}).

    \sa QTransform::map()
*/

/*!
    \fn QRegion QTransform::map(const QRegion &region) const
    \overload

    Creates and returns a QRegion object that is a copy of the given
    \a region, mapped into the coordinate system defined by this matrix.

    Calling this method can be rather expensive if rotations or
    shearing are used.
*/
QRegion QTransform::map(const QRegion &r) const
{
    if (isAffine() && !isScaling() && !isRotating()) { // translate or identity
        if (!isTranslating()) // Identity
            return r;
        QRegion copy(r);
        copy.translate(qRound(affine._dx), qRound(affine._dy));
        return copy;
    }

    QPainterPath p;
    p.addRegion(r);
    p = map(p);
    return p.toFillPolygon(QTransform()).toPolygon();
}

/*!
    \fn QPainterPath operator *(const QPainterPath &path, const QTransform &matrix)
    \since 4.3
    \relates QTransform

    This is the same as \a{matrix}.map(\a{path}).

    \sa QTransform::map()
*/

/*!
    \overload

    Creates and returns a QPainterPath object that is a copy of the
    given \a path, mapped into the coordinate system defined by this
    matrix.
*/
QPainterPath QTransform::map(const QPainterPath &path) const
{

    if (path.isEmpty())
        return QPainterPath();

    QPainterPath copy = path;

    // Translate or identity
    if (isAffine() && !isScaling() && !isRotating()) {

        // Translate
        if (isTranslating()) {
            copy.detach();
            for (int i=0; i<path.elementCount(); ++i) {
                QPainterPath::Element &e = copy.d_ptr->elements[i];
                e.x += affine._dx;
                e.y += affine._dy;
            }
        }

    // Full xform
    } else {
        copy.detach();
        qreal fx, fy;
        for (int i=0; i<path.elementCount(); ++i) {
            QPainterPath::Element &e = copy.d_ptr->elements[i];
            MAPDOUBLE(e.x, e.y, e.x, e.y);
        }
    }

    return copy;
}

/*!
    \fn QPolygon QTransform::mapToPolygon(const QRect &rectangle) const

    Creates and returns a QPolygon representation of the given \a
    rectangle, mapped into the coordinate system defined by this
    matrix.

    The rectangle's coordinates are transformed using the following
    formulas:

    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
        if (is not affine) {
            w' = m13*x + m23*y + m33
            x' /= w'
            y' /= w'
        }
    \endcode

    Polygons and rectangles behave slightly differently when
    transformed (due to integer rounding), so
    \c{matrix.map(QPolygon(rectangle))} is not always the same as
    \c{matrix.mapToPolygon(rectangle)}.

    \sa mapRect(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/
QPolygon QTransform::mapToPolygon(const QRect &rect) const
{

    QPolygon a(4);
    qreal x[4], y[4];
    if (isAffine() && !isRotating()) {
        x[0] = affine._m11*rect.x() + affine._dx;
        y[0] = affine._m22*rect.y() + affine._dy;
        qreal w = affine._m11*rect.width();
        qreal h = affine._m22*rect.height();
        if (w < 0) {
            w = -w;
            x[0] -= w;
        }
        if (h < 0) {
            h = -h;
            y[0] -= h;
        }
        x[1] = x[0]+w;
        x[2] = x[1];
        x[3] = x[0];
        y[1] = y[0];
        y[2] = y[0]+h;
        y[3] = y[2];
    } else {
        qreal right = rect.x() + rect.width();
        qreal bottom = rect.y() + rect.height();
        qreal fx, fy;
        MAPDOUBLE(rect.x(), rect.y(), x[0], y[0]);
        MAPDOUBLE(right, rect.y(), x[1], y[1]);
        MAPDOUBLE(right, bottom, x[2], y[2]);
        MAPDOUBLE(rect.x(), bottom, x[3], y[3]);
    }

    // all coordinates are correctly, tranform to a pointarray
    // (rounding to the next integer)
    a.setPoints(4, qRound(x[0]), qRound(y[0]),
                qRound(x[1]), qRound(y[1]),
                qRound(x[2]), qRound(y[2]),
                qRound(x[3]), qRound(y[3]));
    return a;
}

/*!
    Creates a transformation matrix, \a trans, that maps a unit square
    to a four-sided polygon, \a quad. Returns true if the transformation
    is constructed or false if such a transformation does not exist.

    \sa quadToSquare(), quadToQuad()
*/
bool QTransform::squareToQuad(const QPolygonF &quad, QTransform &trans)
{
    if (quad.count() != 4)
        return false;

    qreal dx0 = quad[0].x();
    qreal dx1 = quad[1].x();
    qreal dx2 = quad[2].x();
    qreal dx3 = quad[3].x();

    qreal dy0 = quad[0].y();
    qreal dy1 = quad[1].y();
    qreal dy2 = quad[2].y();
    qreal dy3 = quad[3].y();

    double ax  = dx0 - dx1 + dx2 - dx3;
    double ay  = dy0 - dy1 + dy2 - dy3;

    if (!ax && !ay) { //afine transform
        trans.setMatrix(dx1 - dx0, dy1 - dy0,  0,
                        dx2 - dx1, dy2 - dy1,  0,
                        dx0,       dy0,  1);
    } else {
        double ax1 = dx1 - dx2;
        double ax2 = dx3 - dx2;
        double ay1 = dy1 - dy2;
        double ay2 = dy3 - dy2;

        /*determinants */
        double gtop    =  ax  * ay2 - ax2 * ay;
        double htop    =  ax1 * ay  - ax  * ay1;
        double bottom  =  ax1 * ay2 - ax2 * ay1;

        double a, b, c, d, e, f, g, h;  /*i is always 1*/

        if (!bottom)
            return false;

        g = gtop/bottom;
        h = htop/bottom;

        a = dx1 - dx0 + g * dx1;
        b = dx3 - dx0 + h * dx3;
        c = dx0;
        d = dy1 - dy0 + g * dy1;
        e = dy3 - dy0 + h * dy3;
        f = dy0;

        trans.setMatrix(a, d, g,
                        b, e, h,
                        c, f, 1.0);
    }

    return true;
}

/*!
    \fn bool QTransform::quadToSquare(const QPolygonF &quad, QTransform &trans)
    
    Creates a transformation matrix, \a trans, that maps a four-sided polygon,
    \a quad, to a unit square. Returns true if the transformation is constructed
    or false if such a transformation does not exist. 

    \sa squareToQuad(), quadToQuad()
*/
bool QTransform::quadToSquare(const QPolygonF &quad, QTransform &trans)
{
    if (!squareToQuad(quad, trans))
        return false;

    bool invertible = false;
    trans = trans.inverted(&invertible);

    return invertible;
}

/*!
    Creates a transformation matrix, \a trans, that maps a four-sided
    polygon, \a one, to another four-sided polygon, \a two.
    Returns true if the transformation is possible; otherwise returns
    false.

    This is a convenience method combining quadToSquare() and
    squareToQuad() methods. It allows the input quad to be
    transformed into any other quad.

    \sa squareToQuad(), quadToSquare()
*/
bool QTransform::quadToQuad(const QPolygonF &one,
                            const QPolygonF &two,
                            QTransform &trans)
{
    QTransform stq;
    if (!quadToSquare(one, trans))
        return false;
    if (!squareToQuad(two, stq))
        return false;
    trans *= stq;
    //qDebug()<<"Final = "<<trans;
    return true;
}

/*! 
    Sets the matrix elements to the specified values, \a m11,
    \a m12, \a m13 \a m21, \a m22, \a m23 \a m31, \a m32 and
    \a m33. Note that this function replaces the previous values. 
    QMatrix provides the translate(), rotate(), scale() and shear()
    convenience functions to manipulate the various matrix elements
    based on the currently defined coordinate system. 
    
    \sa QTransform()
*/

void QTransform::setMatrix(qreal m11, qreal m12, qreal m13,
                           qreal m21, qreal m22, qreal m23,
                           qreal m31, qreal m32, qreal m33)
{
    affine._m11 = m11; affine._m12 = m12; m_13 = m13;
    affine._m21 = m21; affine._m22 = m22; m_23 = m23;
    affine._dx = m31; affine._dy = m32; m_33 = m33;
    m_type = TxNone;
    m_dirty = TxProject;
}

QRect QTransform::mapRect(const QRect &rect) const
{
    QRect result;
    if (isAffine() && !isRotating()) {
        int x = qRound(affine._m11*rect.x() + affine._dx);
        int y = qRound(affine._m22*rect.y() + affine._dy);
        int w = qRound(affine._m11*rect.width());
        int h = qRound(affine._m22*rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRect(x, y, w, h);
    } else {
        // see mapToPolygon for explanations of the algorithm.
        qreal x0, y0;
        qreal x, y, fx, fy;
        MAPDOUBLE(rect.left(), rect.top(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.right() + 1, rect.top(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.right() + 1, rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.left(), rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        qreal w = xmax - xmin;
        qreal h = ymax - ymin;
        xmin -= (xmin - x0) / w;
        ymin -= (ymin - y0) / h;
        xmax -= (xmax - x0) / w;
        ymax -= (ymax - y0) / h;
        result = QRect(qRound(xmin), qRound(ymin),
                       qRound(xmax)-qRound(xmin)+1, qRound(ymax)-qRound(ymin)+1);
    }
    return result;
}

/*!
    \fn QRectF QTransform::mapRect(const QRectF &rectangle) const

    Creates and returns a QRectF object that is a copy of the given \a
    rectangle, mapped into the coordinate system defined by this
    matrix.

    The rectangle's coordinates are transformed using the following
    formulas:

    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
        if (is not affine) {
            w' = m13*x + m23*y + m33
            x' /= w'
            y' /= w'
        }
    \endcode

    If rotation or shearing has been specified, this function returns
    the \e bounding rectangle. To retrieve the exact region the given
    \a rectangle maps to, use the mapToPolygon() function instead.

    \sa mapToPolygon(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/
QRectF QTransform::mapRect(const QRectF &rect) const
{
      QRectF result;
    if (isAffine() && !isRotating()) {
        qreal x = affine._m11*rect.x() + affine._dx;
        qreal y = affine._m22*rect.y() + affine._dy;
        qreal w = affine._m11*rect.width();
        qreal h = affine._m22*rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRectF(x, y, w, h);
    } else {
        qreal x0, y0;
        qreal x, y, fx, fy;
        MAPDOUBLE(rect.x(), rect.y(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.x() + rect.width(), rect.y(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x() + rect.width(), rect.y() + rect.height(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x(), rect.y() + rect.height(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        result = QRectF(xmin, ymin, xmax-xmin, ymax - ymin);
    }
    return result;
}

/*!
    \fn QRect QTransform::mapRect(const QRect &rectangle) const
    \overload

    Creates and returns a QRect object that is a copy of the given \a
    rectangle, mapped into the coordinate system defined by this
    matrix. Note that the transformed coordinates are rounded to the
    nearest integer.
*/

/*!
    Maps the given coordinates \a x and \a y into the coordinate
    system defined by this matrix. The resulting values are put in *\a
    tx and *\a ty, respectively.

    The coordinates are transformed using the following formulas:

    \code
        x' = m11*x + m21*y + dx
        y' = m22*y + m12*x + dy
    \endcode

    The point (x, y) is the original point, and (x', y') is the
    transformed point.

    \sa {QTransform#Basic Matrix Operations}{Basic Matrix Operations}
*/
void QTransform::map(qreal x, qreal y, qreal *tx, qreal *ty) const
{
    qreal fx, fy;
    MAPDOUBLE(x, y, *tx, *ty);
}

/*!
    \overload

    Maps the given coordinates \a x and \a y into the coordinate
    system defined by this matrix. The resulting values are put in *\a
    tx and *\a ty, respectively. Note that the transformed coordinates
    are rounded to the nearest integer.
*/
void QTransform::map(int x, int y, int *tx, int *ty) const
{
    int fx, fy;
    MAPINT(x, y, *tx, *ty);
}

/*!
  Returns the QTransform cast to a QMatrix.
 */
const QMatrix &QTransform::toAffine() const
{
    return affine;
}

/*!
  Returns the transformation type of this matrix.
  */
QTransform::TransformationType QTransform::type() const
{
    if (m_dirty != TxNone && m_dirty >= m_type) {
        if (m_dirty > TxShear && (!qFuzzyCompare(m_13, 0) || !qFuzzyCompare(m_23, 0)))
             m_type = TxProject;
        else if (m_dirty > TxScale && (!qFuzzyCompare(affine._m12, 0) || !qFuzzyCompare(affine._m21, 0))) {
            const qreal dot = affine._m11 * affine._m12 + affine._m21 * affine._m22;
            if (qFuzzyCompare(dot, 0))
                m_type = TxRotate;
            else
                m_type = TxShear;
        } else if (m_dirty > TxTranslate && (!qFuzzyCompare(affine._m11, 1) || !qFuzzyCompare(affine._m22, 1) || !qFuzzyCompare(m_33, 1)))
            m_type = TxScale;
        else if (m_dirty > TxNone && (!qFuzzyCompare(affine._dx, 0) || !qFuzzyCompare(affine._dy, 0)))
            m_type = TxTranslate;
        else
            m_type = TxNone;

        m_dirty = TxNone;
    }

    return static_cast<TransformationType>(m_type);
}

/*!

    Returns the transform as a QVariant.
*/
QTransform::operator QVariant() const
{
    return QVariant(QVariant::Transform, this);
}


/*!
    \fn bool QTransform::isInvertible() const

    Returns true if the matrix is invertible, otherwise returns false.

    \sa inverted()
*/

/*!
    \fn qreal QTransform::det() const

    Returns the matrix's determinant.
*/


/*!
    \fn qreal QTransform::m11() const

    Returns the horizontal scaling factor.

    \sa scale(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m12() const

    Returns the vertical shearing factor.

    \sa shear(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m21() const

    Returns the horizontal shearing factor.

    \sa shear(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m22() const

    Returns the vertical scaling factor.

    \sa scale(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::dx() const

    Returns the horizontal translation factor.

    \sa m31(), translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::dy() const

    Returns the vertical translation factor.

    \sa translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/


/*!
    \fn qreal QTransform::m13() const

    Returns the horizontal projection factor.

    \sa translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/


/*!
    \fn qreal QTransform::m23() const

    Returns the vertical projection factor.

    \sa translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m31() const

    Returns the horizontal translation factor.

    \sa dx(), translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m32() const

    Returns the vertical translation factor.

    \sa dy(), translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::m33() const

    Returns the division factor.

    \sa translate(), {QTransform#Basic Matrix Operations}{Basic Matrix
    Operations}
*/

/*!
    \fn qreal QTransform::determinant() const

    Returns the matrix's determinant.
*/

/*!
    \fn bool QTransform::isIdentity() const

    Returns true if the matrix is the identity matrix, otherwise
    returns false.

    \sa reset()
*/

/*!
    \fn bool QTransform::isAffine() const

    Returns true if the matrix represent an affine transformation,
    otherwise returns false.
*/

/*!
    \fn bool QTransform::isScaling() const

    Returns true if the matrix represents a scaling
    transformation, otherwise returns false.

    \sa reset()
*/

/*!
    \fn bool QTransform::isRotating() const

    Returns true if the matrix represents some kind of a
    scaling transformation, otherwise returns false.

    \sa reset()
*/

/*!
    \fn bool QTransform::isTranslating() const

    Returns true if the matrix represents a translating
    transformation, otherwise returns false.

    \sa reset()
*/
