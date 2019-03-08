#ifndef QDEMONUTILS_H
#define QDEMONUTILS_H

#include <QtDemon/qtdemonglobal.h>
#include <QtDemon/qdemondataref.h>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix3x3>
#include <QtGui/QMatrix4x4>

#include <QtCore/qdebug.h>
#include <QtCore/QString>
#include <QtCore/qloggingcategory.h>
#include <QtCore/QIODevice>

QT_BEGIN_NAMESPACE

namespace vec2 {
float Q_DEMON_EXPORT magnitude(const QVector2D &v);
}

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
QMatrix3x3 Q_DEMON_EXPORT getUpper3x3(const QMatrix4x4 &m);
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

template<typename TDataType>
QDemonDataRef<TDataType> PtrAtOffset(quint8 *baseData, quint32 offset, quint32 byteSize)
{
    return QDemonDataRef<TDataType>(byteSize ? reinterpret_cast<TDataType *>(baseData + offset) : nullptr,
                                    byteSize / sizeof(TDataType));
}

void Q_DEMON_EXPORT memZero(void *ptr, size_t size);
void Q_DEMON_EXPORT memSet(void *ptr, quint8 value, size_t size);

Q_DEMON_EXPORT const char *nonNull(const char *src);

Q_DEMON_EXPORT float radToDeg(const float a);
Q_DEMON_EXPORT double radToDeg(const double a);
Q_DEMON_EXPORT float degToRad(const float a);
Q_DEMON_EXPORT double degToRad(const double a);

namespace IOStream {
enum class SeekPosition
{
    Unknown,
    Begin,
    Current,
    End,
};
qint64 Q_DEMON_EXPORT positionHelper(const QIODevice &device, qint64 offset, SeekPosition seekPosition);
}

namespace CFileTools {
QString Q_DEMON_EXPORT normalizePathForQtUsage(const QString &path);
void Q_DEMON_EXPORT combineBaseAndRelative(const char *inBase, const char *inRelative, QString &outString);
void Q_DEMON_EXPORT combineBaseAndRelative(const QString &inBase, const QString &inRelative, QString &outString);
}

QT_END_NAMESPACE

#endif // QDEMONUTILS_H
