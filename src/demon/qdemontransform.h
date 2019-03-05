#ifndef QDEMONTRANSFORM_H
#define QDEMONTRANSFORM_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/QDemonPlane>
#include <QtGui/QQuaternion>
#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

class Q_DEMON_EXPORT QDemonTransform
{
public:
    QQuaternion q;
    QVector3D p;

    Q_ALWAYS_INLINE QDemonTransform();

    Q_ALWAYS_INLINE explicit QDemonTransform(const QVector3D &position) : q(0, 0, 0, 1), p(position) {}

    Q_ALWAYS_INLINE explicit QDemonTransform(const QQuaternion &orientation);

    Q_ALWAYS_INLINE QDemonTransform(const QVector3D &p0, const QQuaternion &q0);

    Q_ALWAYS_INLINE explicit QDemonTransform(const QMatrix4x4 &m); // defined in QMatrix4x4.h

    Q_ALWAYS_INLINE QDemonTransform operator*(const QDemonTransform &x) const
    {
        Q_ASSERT(x.isSane());
        return transform(x);
    }

    Q_ALWAYS_INLINE QDemonTransform getInverse() const;

    QVector3D transform(const QVector3D &input) const;

    Q_ALWAYS_INLINE QVector3D transformInv(const QVector3D &input) const;

    Q_ALWAYS_INLINE QVector3D rotate(const QVector3D &input) const;

    Q_ALWAYS_INLINE QVector3D rotateInv(const QVector3D &input) const;

    //! Transform transform to parent (returns compound transform: first src, then *this)
    QDemonTransform transform(const QDemonTransform &src) const;

    /**
    \brief returns true if finite and q is a unit quaternion
    */

    bool isValid() const;

    /**
    \brief returns true if finite and quat magnitude is reasonably close to unit to allow for some
    accumulation of error vs isValid
    */

    bool isSane() const;

    /**
    \brief returns true if all elems are finite (not NAN or INF, etc.)
    */
    Q_ALWAYS_INLINE bool isFinite() const;

    //! Transform transform from parent (returns compound transform: first src, then this->inverse)
    Q_ALWAYS_INLINE QDemonTransform transformInv(const QDemonTransform &src) const;

    static Q_ALWAYS_INLINE QDemonTransform createIdentity();

    /**
    \brief transform plane
    */

    Q_ALWAYS_INLINE QDemonPlane transform(const QDemonPlane &plane) const;

    /**
    \brief inverse-transform plane
    */

    Q_ALWAYS_INLINE QDemonPlane inverseTransform(const QDemonPlane &plane) const;
};

QT_END_NAMESPACE

#endif // QDEMONTRANSFORM_H
