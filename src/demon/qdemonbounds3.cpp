#include "qdemonbounds3.h"
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

void QDemonBounds3::include(const QVector3D &v)
{
    minimum = vec3::minimum(minimum, v);
    maximum = vec3::maximum(maximum, v);
}

void QDemonBounds3::include(const QDemonBounds3 &b)
{
    minimum = vec3::minimum(minimum, b.minimum);
    maximum = vec3::maximum(maximum, b.maximum);
}

bool QDemonBounds3::isFinite() const
{
    return vec3::isFinite(minimum) && vec3::isFinite(maximum);
}

QDemonBounds3 QDemonBounds3::boundsOfPoints(const QVector3D &v0, const QVector3D &v1)
{
    return QDemonBounds3(vec3::minimum(v0, v1), vec3::maximum(v0, v1));
}

QDemonBounds3 QDemonBounds3::basisExtent(const QVector3D &center, const QMatrix3x3 &basis, const QVector3D &extent)
{
    // extended basis vectors
    QVector3D c0 = QVector3D(basis(0, 0), basis(1, 0), basis(2, 0)) * extent.x();
    QVector3D c1 = QVector3D(basis(0, 1), basis(1, 1), basis(2, 1)) * extent.y();
    QVector3D c2 = QVector3D(basis(0, 2), basis(1, 2), basis(2, 2)) * extent.z();

    QVector3D w;
    // find combination of base vectors that produces max. distance for each component = sum of
    // abs()
    w.setX(qAbs(c0.x()) + qAbs(c1.x()) + qAbs(c2.x()));
    w.setY(qAbs(c0.y()) + qAbs(c1.y()) + qAbs(c2.y()));
    w.setZ(qAbs(c0.z()) + qAbs(c1.z()) + qAbs(c2.z()));

    return QDemonBounds3(center - w, center + w);
}

QDemonBounds3 QDemonBounds3::transform(const QMatrix3x3 &matrix, const QDemonBounds3 &bounds)
{
    return bounds.isEmpty()
            ? bounds
            : QDemonBounds3::basisExtent(mat33::transform(matrix, bounds.center()), matrix, bounds.extents());
}

void QDemonBounds3::transform(const QMatrix4x4 &inMatrix)
{
    if (!isEmpty()) {
        QDemonBounds2BoxPoints thePoints;
        expand(thePoints);
        setEmpty();
        for (quint32 idx = 0; idx < 8; ++idx)
            include(inMatrix * thePoints[idx]);
    }
}

QT_END_NAMESPACE
