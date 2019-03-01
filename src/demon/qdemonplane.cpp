#include "qdemonplane.h"

QT_BEGIN_NAMESPACE

namespace  {
float magnitude(const QVector3D &vector) {
    return std::sqrt(vector.x() * vector.x() + vector.y() * vector.y() + vector.z() * vector.z());
}
}

void QDemonPlane::normalize()
{
    float denom = 1.0f / magnitude(n);
    n *= denom;
    d *= denom;
}


QT_END_NAMESPACE
