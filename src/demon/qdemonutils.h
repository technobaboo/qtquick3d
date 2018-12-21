#ifndef QDEMONUTILS_H
#define QDEMONUTILS_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemondataref.h>

#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix3x3>
#include <QtGui/QMatrix4x4>

#include <QtCore/qdebug.h>
#include <QtCore/QString>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

#define QDEMON_FOREACH(varname, stop) for (quint32 varname = 0, end = stop; varname < end; ++varname)

namespace vec3 {
QVector3D Q_DEMON_EXPORT minimum(const QVector3D &v1, const QVector3D &v2);
QVector3D Q_DEMON_EXPORT maximum(const QVector3D &v1, const QVector3D &v2);
bool Q_DEMON_EXPORT isFinite(const QVector3D &v);
float Q_DEMON_EXPORT magnitude(const QVector3D &v);
float Q_DEMON_EXPORT magnitudeSquared(const QVector3D &v);
float Q_DEMON_EXPORT normalize(QVector3D &v);

}

namespace mat33 {
QVector3D Q_DEMON_EXPORT transform(const QMatrix3x3 &m, const QVector3D &v);
QMatrix3x3 Q_DEMON_EXPORT getInverse(const QMatrix3x3 &m);
}

namespace mat44 {
QMatrix4x4 Q_DEMON_EXPORT getInverse(const QMatrix4x4 &m);
QVector3D Q_DEMON_EXPORT rotate(const QMatrix4x4 &m, const QVector3D &v);
QVector4D Q_DEMON_EXPORT rotate(const QMatrix4x4 &m, const QVector4D &v);
QVector3D Q_DEMON_EXPORT transform(const QMatrix4x4 &m, const QVector3D &v);
QVector4D Q_DEMON_EXPORT transform(const QMatrix4x4 &m, const QVector4D &v);
}

namespace quant {
bool Q_DEMON_EXPORT isFinite(const QQuaternion &q);

float Q_DEMON_EXPORT magnitude(const QQuaternion &q);

bool Q_DEMON_EXPORT isSane(const QQuaternion &q);

bool Q_DEMON_EXPORT isUnit(const QQuaternion &q);


QVector3D Q_DEMON_EXPORT rotated(const QQuaternion &q, const QVector3D &v);

QVector3D Q_DEMON_EXPORT inverseRotated(const QQuaternion &q, const QVector3D &v);

}

template <typename TDataType>
QDemonDataRef<TDataType> PtrAtOffset(quint8 *baseData, quint32 offset, quint32 byteSize)
{
    return QDemonDataRef<TDataType>(byteSize ? reinterpret_cast<TDataType *>(baseData + offset) : nullptr, byteSize / sizeof(TDataType));
}

void Q_DEMON_EXPORT memZero(void *ptr, size_t size);

inline Q_DEMON_EXPORT const char *nonNull(const char *src);

namespace CFileTools {
QString Q_DEMON_EXPORT NormalizePathForQtUsage(const QString &path);
}

QT_END_NAMESPACE

#endif // QDEMONUTILS_H
