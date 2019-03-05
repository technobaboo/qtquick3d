#include "qdemontransform.h"

#include <QtDemon/qdemonutils.h>

#include <qmath.h>
#include <qnumeric.h>

QT_BEGIN_NAMESPACE

QDemonTransform::QDemonTransform() : q(0, 0, 0, 1), p(0, 0, 0) {}

QDemonTransform::QDemonTransform(const QQuaternion &orientation) : q(orientation), p(0, 0, 0)
{
    Q_ASSERT(quant::isSane(orientation));
}

QDemonTransform::QDemonTransform(const QVector3D &p0, const QQuaternion &q0) : q(q0), p(p0)
{
    Q_ASSERT(quant::isSane(q0));
}

QDemonTransform QDemonTransform::getInverse() const
{
    Q_ASSERT(isFinite());
    return QDemonTransform(quant::inverseRotated(q, -p), q.conjugated());
}

QVector3D QDemonTransform::transform(const QVector3D &input) const
{
    Q_ASSERT(isFinite());
    return quant::rotated(q, input) + p;
}

QVector3D QDemonTransform::transformInv(const QVector3D &input) const
{
    Q_ASSERT(isFinite());
    return quant::inverseRotated(q, input - p);
}

QVector3D QDemonTransform::rotate(const QVector3D &input) const
{
    Q_ASSERT(isFinite());
    return quant::rotated(q, input);
}

QVector3D QDemonTransform::rotateInv(const QVector3D &input) const
{
    Q_ASSERT(isFinite());
    return quant::inverseRotated(q, input);
}

QDemonTransform QDemonTransform::transform(const QDemonTransform &src) const
{
    Q_ASSERT(src.isSane());
    Q_ASSERT(isSane());
    // src = [srct, srcr] -> [r*srct + t, r*srcr]
    return QDemonTransform(quant::rotated(q, src.p) + p, q * src.q);
}

bool QDemonTransform::isValid() const
{
    return vec3::isFinite(p) && quant::isFinite(q) && quant::isUnit(q);
}

bool QDemonTransform::isSane() const
{
    return isFinite() && quant::isSane(q);
}

bool QDemonTransform::isFinite() const
{
    return vec3::isFinite(p) && quant::isFinite(q);
}

QDemonTransform QDemonTransform::transformInv(const QDemonTransform &src) const
{
    Q_ASSERT(src.isSane());
    Q_ASSERT(isFinite());
    // src = [srct, srcr] -> [r^-1*(srct-t), r^-1*srcr]
    QQuaternion qinv = q.conjugated();
    return QDemonTransform(quant::rotated(qinv, src.p - p), qinv * src.q);
}

QDemonTransform QDemonTransform::createIdentity()
{
    return QDemonTransform(QVector3D(0, 0, 0));
}

QDemonPlane QDemonTransform::transform(const QDemonPlane &plane) const
{
    QVector3D transformedNormal = rotate(plane.n);
    return QDemonPlane(transformedNormal, plane.d - QVector3D::dotProduct(p, transformedNormal));
}

QDemonPlane QDemonTransform::inverseTransform(const QDemonPlane &plane) const
{
    QVector3D transformedNormal = rotateInv(plane.n);
    return QDemonPlane(transformedNormal, plane.d + QVector3D::dotProduct(p, plane.n));
}

QT_END_NAMESPACE
