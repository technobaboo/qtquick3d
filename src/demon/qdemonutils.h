#ifndef QDEMONUTILS_H
#define QDEMONUTILS_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemondataref.h>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix3x3>

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

#define QDEMON_FOREACH(varname, stop) for (quint32 varname = 0, end = stop; varname < end; ++varname)

namespace vec3 {
QVector3D minimum(const QVector3D &v1, const QVector3D &v2);
QVector3D maximum(const QVector3D &v1, const QVector3D &v2);
bool isFinite(const QVector3D &v);

}

namespace mat33 {
QVector3D transform(const QMatrix3x3 &m, const QVector3D &v);
}

namespace quant {
bool isFinite(const QQuaternion &q);

float magnitude(const QQuaternion &q);

bool isSane(const QQuaternion &q);

bool isUnit(const QQuaternion &q);


QVector3D rotated(const QQuaternion &q, const QVector3D &v);

QVector3D inverseRotated(const QQuaternion &q, const QVector3D &v);

}

template <typename TDataType>
QDemonDataRef<TDataType> PtrAtOffset(quint8 *baseData, quint32 offset, quint32 byteSize)
{
    return QDemonDataRef<TDataType>(byteSize ? reinterpret_cast<TDataType *>(baseData + offset) : nullptr, byteSize / sizeof(TDataType));
}

void Q_DEMON_EXPORT memZero(void *ptr, size_t size);

inline Q_DEMON_EXPORT const char *nonNull(const char *src);

QT_END_NAMESPACE

#endif // QDEMONUTILS_H
