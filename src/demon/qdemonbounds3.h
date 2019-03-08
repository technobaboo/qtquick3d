#ifndef QDEMONBOUNDS3_H
#define QDEMONBOUNDS3_H

#include <QtDemon/qtdemonglobal.h>

#include <QVector3D>
#include <QMatrix3x3>
#include <QMatrix4x4>

#include <limits>
#include <qnumeric.h>

QT_BEGIN_NAMESPACE

typedef QVector3D QDemonBounds2BoxPoints[8];

/**
\brief Class representing 3D range or axis aligned bounding box.

Stored as minimum and maximum extent corners. Alternate representation
would be center and dimensions.
May be empty or nonempty. If not empty, minimum <= maximum has to hold.
*/
class Q_DEMON_EXPORT QDemonBounds3
{
public:
    /**
    \brief Default constructor, not performing any initialization for performance reason.
    \remark Use empty() function below to construct empty bounds.
    */
    QDemonBounds3() = default;

    /**
    \brief Construct from two bounding points
    */
    Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QDemonBounds3(const QVector3D &minimum, const QVector3D &maximum);

    /**
    \brief Return empty bounds.
    */
    static Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QDemonBounds3 empty();

    /**
    \brief returns the AABB containing v0 and v1.
    \param v0 first point included in the AABB.
    \param v1 second point included in the AABB.
    */
    static Q_ALWAYS_INLINE QDemonBounds3 boundsOfPoints(const QVector3D &v0, const QVector3D &v1);

    /**
    \brief returns the AABB from center and extents vectors.
    \param center Center vector
    \param extent Extents vector
    */
    static Q_ALWAYS_INLINE QDemonBounds3 centerExtents(const QVector3D &center, const QVector3D &extent);

    /**
    \brief Construct from center, extent, and (not necessarily orthogonal) basis
    */
    static Q_ALWAYS_INLINE QDemonBounds3 basisExtent(const QVector3D &center, const QMatrix3x3 &basis, const QVector3D &extent);

    /**
    \brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).
    \param[in] matrix Transform to apply, can contain scaling as well
    \param[in] bounds The bounds to transform.
    */
    static Q_ALWAYS_INLINE QDemonBounds3 transform(const QMatrix3x3 &matrix, const QDemonBounds3 &bounds);

    /**
    \brief Sets empty to true
    */
    Q_ALWAYS_INLINE void setEmpty();

    /**
    \brief Sets infinite bounds
    */
    Q_ALWAYS_INLINE void setInfinite();

    /**
    \brief expands the volume to include v
    \param v Point to expand to.
    */
    void include(const QVector3D &v);

    /**
    \brief expands the volume to include b.
    \param b Bounds to perform union with.
    */
    void include(const QDemonBounds3 &b);

    Q_ALWAYS_INLINE bool isEmpty() const;

    /**
    \brief indicates whether the intersection of this and b is empty or not.
    \param b Bounds to test for intersection.
    */
    Q_ALWAYS_INLINE bool intersects(const QDemonBounds3 &b) const;

    /**
     \brief computes the 1D-intersection between two AABBs, on a given axis.
     \param	a		the other AABB
     \param	axis	the axis (0, 1, 2)
     */
    Q_ALWAYS_INLINE bool intersects1D(const QDemonBounds3 &a, quint32 axis) const;

    /**
    \brief indicates if these bounds contain v.
    \param v Point to test against bounds.
    */
    Q_ALWAYS_INLINE bool contains(const QVector3D &v) const;

    /**
     \brief	checks a box is inside another box.
     \param	box		the other AABB
     */
    Q_ALWAYS_INLINE bool isInside(const QDemonBounds3 &box) const;

    /**
    \brief returns the center of this axis aligned box.
    */
    Q_ALWAYS_INLINE QVector3D center() const;

    /**
    \brief get component of the box's center along a given axis
    */
    Q_ALWAYS_INLINE float center(quint32 axis) const;

    /**
    \brief get component of the box's extents along a given axis
    */
    Q_ALWAYS_INLINE float extents(quint32 axis) const;

    /**
    \brief returns the dimensions (width/height/depth) of this axis aligned box.
    */
    Q_ALWAYS_INLINE QVector3D dimensions() const;

    /**
    \brief returns the extents, which are half of the width/height/depth.
    */
    Q_ALWAYS_INLINE QVector3D extents() const;

    /**
    \brief scales the AABB.
    \param scale Factor to scale AABB by.
    */
    Q_ALWAYS_INLINE void scale(float scale);

    /**
    fattens the AABB in all 3 dimensions by the given distance.
    */
    Q_ALWAYS_INLINE void fatten(double distance);

    /**
    checks that the AABB values are not NaN
    */

    bool isFinite() const;

    Q_ALWAYS_INLINE void expand(QDemonBounds2BoxPoints &outPoints) const;

    void transform(const QMatrix4x4 &inMatrix);

    QVector3D minimum;
    QVector3D maximum;
};

Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QDemonBounds3::QDemonBounds3(const QVector3D &_minimum, const QVector3D &_maximum)
    : minimum(_minimum), maximum(_maximum)
{
}

Q_DECL_CONSTEXPR Q_ALWAYS_INLINE QDemonBounds3 QDemonBounds3::empty()
{
    return QDemonBounds3(QVector3D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
                         QVector3D(-std::numeric_limits<float>::max(),
                                   -std::numeric_limits<float>::max(),
                                   -std::numeric_limits<float>::max()));
}

Q_ALWAYS_INLINE QDemonBounds3 QDemonBounds3::centerExtents(const QVector3D &center, const QVector3D &extent)
{
    return QDemonBounds3(center - extent, center + extent);
}

Q_ALWAYS_INLINE void QDemonBounds3::setEmpty()
{
    const float maxFloat = std::numeric_limits<float>::max();
    minimum = QVector3D(maxFloat, maxFloat, maxFloat);
    maximum = QVector3D(-maxFloat, -maxFloat, -maxFloat);
}

Q_ALWAYS_INLINE void QDemonBounds3::setInfinite()
{
    const float maxFloat = std::numeric_limits<float>::max();
    minimum = QVector3D(-maxFloat, -maxFloat, -maxFloat);
    maximum = QVector3D(maxFloat, maxFloat, maxFloat);
}

Q_ALWAYS_INLINE bool QDemonBounds3::isEmpty() const
{
    Q_ASSERT(isFinite());
    // Consistency condition for (Min, Max) boxes: minimum < maximum
    return minimum.x() > maximum.x() || minimum.y() > maximum.y() || minimum.z() > maximum.z();
}

Q_ALWAYS_INLINE bool QDemonBounds3::intersects(const QDemonBounds3 &b) const
{
    Q_ASSERT(isFinite() && b.isFinite());
    return !(b.minimum.x() > maximum.x() || minimum.x() > b.maximum.x() || b.minimum.y() > maximum.y()
             || minimum.y() > b.maximum.y() || b.minimum.z() > maximum.z() || minimum.z() > b.maximum.z());
}

Q_ALWAYS_INLINE bool QDemonBounds3::intersects1D(const QDemonBounds3 &a, quint32 axis) const
{
    Q_ASSERT(isFinite() && a.isFinite());
    return maximum[int(axis)] >= a.minimum[axis] && a.maximum[axis] >= minimum[axis];
}

Q_ALWAYS_INLINE bool QDemonBounds3::contains(const QVector3D &v) const
{
    Q_ASSERT(isFinite());

    return !(v.x() < minimum.x() || v.x() > maximum.x() || v.y() < minimum.y() || v.y() > maximum.y()
             || v.z() < minimum.z() || v.z() > maximum.z());
}

Q_ALWAYS_INLINE bool QDemonBounds3::isInside(const QDemonBounds3 &box) const
{
    Q_ASSERT(isFinite() && box.isFinite());
    if (box.minimum.x() > minimum.x())
        return false;
    if (box.minimum.y() > minimum.y())
        return false;
    if (box.minimum.z() > minimum.z())
        return false;
    if (box.maximum.x() < maximum.x())
        return false;
    if (box.maximum.y() < maximum.y())
        return false;
    if (box.maximum.z() < maximum.z())
        return false;
    return true;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::center() const
{
    Q_ASSERT(isFinite());
    return (minimum + maximum) * double(0.5);
}

Q_ALWAYS_INLINE float QDemonBounds3::center(quint32 axis) const
{
    Q_ASSERT(isFinite());
    return (minimum[int(axis)] + maximum[int(axis)]) * 0.5f;
}

Q_ALWAYS_INLINE float QDemonBounds3::extents(quint32 axis) const
{
    Q_ASSERT(isFinite());
    return (maximum[int(axis)] - minimum[int(axis)]) * 0.5f;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::dimensions() const
{
    Q_ASSERT(isFinite());
    return maximum - minimum;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::extents() const
{
    Q_ASSERT(isFinite());
    return dimensions() * double(0.5);
}

Q_ALWAYS_INLINE void QDemonBounds3::scale(float scale)
{
    Q_ASSERT(isFinite());
    *this = centerExtents(center(), extents() * scale);
}

Q_ALWAYS_INLINE void QDemonBounds3::fatten(double distance)
{
    Q_ASSERT(isFinite());
    minimum -= QVector3D(float(distance), float(distance), float(distance));
    maximum += QVector3D(float(distance), float(distance), float(distance));
}

Q_ALWAYS_INLINE void QDemonBounds3::expand(QDemonBounds2BoxPoints &outPoints) const
{
    if (isEmpty()) {
        for (quint32 idx = 0; idx < 8; ++idx)
            outPoints[idx] = QVector3D(0, 0, 0);
    } else {
        // Min corner of box
        outPoints[0] = QVector3D(minimum[0], minimum[1], minimum[2]);
        outPoints[1] = QVector3D(maximum[0], minimum[1], minimum[2]);
        outPoints[2] = QVector3D(minimum[0], maximum[1], minimum[2]);
        outPoints[3] = QVector3D(minimum[0], minimum[1], maximum[2]);

        // Max corner of box
        outPoints[4] = QVector3D(maximum[0], maximum[1], maximum[2]);
        outPoints[5] = QVector3D(minimum[0], maximum[1], maximum[2]);
        outPoints[6] = QVector3D(maximum[0], minimum[1], maximum[2]);
        outPoints[7] = QVector3D(maximum[0], maximum[1], minimum[2]);
    }
}

QT_END_NAMESPACE

#endif // QDEMONBOUNDS3_H
