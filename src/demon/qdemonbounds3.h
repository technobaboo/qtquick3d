#ifndef QDEMONBOUNDS3_H
#define QDEMONBOUNDS3_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/QDemonTransform>

#include <QVector3D>
#include <QMatrix3x3>
#include <QMatrix4x4>

#include <limits>
#include <qnumeric.h>

QT_BEGIN_NAMESPACE

typedef QVector3D TNVBounds2BoxPoints[8];

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
    QDemonBounds3();

    /**
    \brief Construct from two bounding points
    */
    Q_ALWAYS_INLINE QDemonBounds3(const QVector3D &m_minimum, const QVector3D &m_maximum);

    /**
    \brief Return empty bounds.
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 empty();

    /**
    \brief returns the AABB containing v0 and v1.
    \param v0 first point included in the AABB.
    \param v1 second point included in the AABB.
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 boundsOfPoints(const QVector3D &v0,
                                                         const QVector3D &v1);

    /**
    \brief returns the AABB from center and extents vectors.
    \param center Center vector
    \param extent Extents vector
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 centerExtents(const QVector3D &center,
                                                        const QVector3D &extent);

    /**
    \brief Construct from center, extent, and (not necessarily orthogonal) basis
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 basisExtent(const QVector3D &center,
                                                      const QMatrix3x3 &basis,
                                                      const QVector3D &extent);

    /**
    \brief Construct from pose and extent
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 poseExtent(const QDemonTransform &pose,
                                                     const QVector3D &extent);

    /**
    \brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).
    \param[in] matrix Transform to apply, can contain scaling as well
    \param[in] bounds The bounds to transform.
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 transform(const QMatrix3x3 &matrix,
                                                    const QDemonBounds3 &bounds);

    /**
    \brief gets the transformed bounds of the passed AABB (resulting in a bigger AABB).
    \param[in] transform Transform to apply, can contain scaling as well
    \param[in] bounds The bounds to transform.
    */
    static  Q_ALWAYS_INLINE QDemonBounds3 transform(const QDemonTransform &transform,
                                                    const QDemonBounds3 &bounds);

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
    Q_ALWAYS_INLINE void include(const QVector3D &v);

    /**
    \brief expands the volume to include b.
    \param b Bounds to perform union with.
    */
    Q_ALWAYS_INLINE void include(const QDemonBounds3 &b);

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
    Q_ALWAYS_INLINE QVector3D getCenter() const;

    /**
    \brief get component of the box's center along a given axis
    */
    Q_ALWAYS_INLINE float getCenter(quint32 axis) const;

    /**
    \brief get component of the box's extents along a given axis
    */
    Q_ALWAYS_INLINE float getExtents(quint32 axis) const;

    /**
    \brief returns the dimensions (width/height/depth) of this axis aligned box.
    */
    Q_ALWAYS_INLINE QVector3D getDimensions() const;

    /**
    \brief returns the extents, which are half of the width/height/depth.
    */
    Q_ALWAYS_INLINE QVector3D getExtents() const;

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

    Q_ALWAYS_INLINE void expand(TNVBounds2BoxPoints &outPoints) const;

    Q_ALWAYS_INLINE void transform(const QMatrix4x4 &inMatrix);

    QVector3D m_minimum;
    QVector3D m_maximum;
};

Q_ALWAYS_INLINE QDemonBounds3::QDemonBounds3(const QVector3D &minimum, const QVector3D &maximum)
    : m_minimum(minimum)
    , m_maximum(maximum)
{
}

Q_ALWAYS_INLINE QDemonBounds3 QDemonBounds3::empty()
{
    const float maxFloat = std::numeric_limits<float>::max();
    return QDemonBounds3(QVector3D(maxFloat, maxFloat, maxFloat), QVector3D(-maxFloat, -maxFloat, -maxFloat));
}

Q_ALWAYS_INLINE QDemonBounds3 QDemonBounds3::centerExtents(const QVector3D &center,
                                                           const QVector3D &extent)
{
    return QDemonBounds3(center - extent, center + extent);
}

Q_ALWAYS_INLINE void QDemonBounds3::setEmpty()
{
    const float maxFloat = std::numeric_limits<float>::max();
    m_minimum = QVector3D(maxFloat, maxFloat, maxFloat);
    m_maximum = QVector3D(-maxFloat, -maxFloat, -maxFloat);
}

Q_ALWAYS_INLINE void QDemonBounds3::setInfinite()
{
    const float maxFloat = std::numeric_limits<float>::max();
    m_minimum = QVector3D(-maxFloat, -maxFloat, -maxFloat);
    m_maximum = QVector3D(maxFloat, maxFloat, maxFloat);
}


Q_ALWAYS_INLINE bool QDemonBounds3::isEmpty() const
{
    Q_ASSERT(isFinite());
    // Consistency condition for (Min, Max) boxes: minimum < maximum
    return m_minimum.x() > m_maximum.x() || m_minimum.y() > m_maximum.y() || m_minimum.z() > m_maximum.z();
}

Q_ALWAYS_INLINE bool QDemonBounds3::intersects(const QDemonBounds3 &b) const
{
    Q_ASSERT(isFinite() && b.isFinite());
    return !(b.m_minimum.x() > m_maximum.x() || m_minimum.x() > b.m_maximum.x() || b.m_minimum.y() > m_maximum.y()
             || m_minimum.y() > b.m_maximum.y() || b.m_minimum.z() > m_maximum.z() || m_minimum.z() > b.m_maximum.z());
}

Q_ALWAYS_INLINE bool QDemonBounds3::intersects1D(const QDemonBounds3 &a, quint32 axis) const
{
    Q_ASSERT(isFinite() && a.isFinite());
    return m_maximum[int(axis)] >= a.m_minimum[axis] && a.m_maximum[axis] >= m_minimum[axis];
}

Q_ALWAYS_INLINE bool QDemonBounds3::contains(const QVector3D &v) const
{
    Q_ASSERT(isFinite());

    return !(v.x() < m_minimum.x() || v.x() > m_maximum.x() || v.y() < m_minimum.y() || v.y() > m_maximum.y()
             || v.z() < m_minimum.z() || v.z() > m_maximum.z());
}

Q_ALWAYS_INLINE bool QDemonBounds3::isInside(const QDemonBounds3 &box) const
{
    Q_ASSERT(isFinite() && box.isFinite());
    if (box.m_minimum.x() > m_minimum.x())
        return false;
    if (box.m_minimum.y() > m_minimum.y())
        return false;
    if (box.m_minimum.z() > m_minimum.z())
        return false;
    if (box.m_maximum.x() < m_maximum.x())
        return false;
    if (box.m_maximum.y() < m_maximum.y())
        return false;
    if (box.m_maximum.z() < m_maximum.z())
        return false;
    return true;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::getCenter() const
{
    Q_ASSERT(isFinite());
    return (m_minimum + m_maximum) * double(0.5);
}

Q_ALWAYS_INLINE float QDemonBounds3::getCenter(quint32 axis) const
{
    Q_ASSERT(isFinite());
    return (m_minimum[int(axis)] + m_maximum[int(axis)]) * 0.5f;
}

Q_ALWAYS_INLINE float QDemonBounds3::getExtents(quint32 axis) const
{
    Q_ASSERT(isFinite());
    return (m_maximum[int(axis)] - m_minimum[int(axis)]) * 0.5f;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::getDimensions() const
{
    Q_ASSERT(isFinite());
    return m_maximum - m_minimum;
}

Q_ALWAYS_INLINE QVector3D QDemonBounds3::getExtents() const
{
    Q_ASSERT(isFinite());
    return getDimensions() * double(0.5);
}

Q_ALWAYS_INLINE void QDemonBounds3::scale(float scale)
{
    Q_ASSERT(isFinite());
    *this = centerExtents(getCenter(), getExtents() * scale);
}

Q_ALWAYS_INLINE void QDemonBounds3::fatten(double distance)
{
    Q_ASSERT(isFinite());
    m_minimum -= QVector3D(float(distance), float(distance), float(distance));
    m_maximum += QVector3D(float(distance), float(distance), float(distance));
}

Q_ALWAYS_INLINE void QDemonBounds3::expand(TNVBounds2BoxPoints &outPoints) const
{
    if (isEmpty()) {
        for (quint32 idx = 0; idx < 8; ++idx)
            outPoints[idx] = QVector3D(0, 0, 0);
    } else {
        // Min corner of box
        outPoints[0] = QVector3D(m_minimum[0], m_minimum[1], m_minimum[2]);
        outPoints[1] = QVector3D(m_maximum[0], m_minimum[1], m_minimum[2]);
        outPoints[2] = QVector3D(m_minimum[0], m_maximum[1], m_minimum[2]);
        outPoints[3] = QVector3D(m_minimum[0], m_minimum[1], m_maximum[2]);

        // Max corner of box
        outPoints[4] = QVector3D(m_maximum[0], m_maximum[1], m_maximum[2]);
        outPoints[5] = QVector3D(m_minimum[0], m_maximum[1], m_maximum[2]);
        outPoints[6] = QVector3D(m_maximum[0], m_minimum[1], m_maximum[2]);
        outPoints[7] = QVector3D(m_maximum[0], m_maximum[1], m_minimum[2]);
    }
}

QT_END_NAMESPACE

#endif // QDEMONBOUNDS3_H
