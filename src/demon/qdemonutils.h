/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
Q_DEMON_EXPORT inline constexpr QVector3D minimum(const QVector3D &v1, const QVector3D &v2) Q_DECL_NOTHROW { return { qMin(v1.x(), v2.x()), qMin(v1.y(), v2.y()), qMin(v1.z(), v2.z()) }; }
Q_DEMON_EXPORT inline constexpr QVector3D maximum(const QVector3D &v1, const QVector3D &v2) Q_DECL_NOTHROW { return { qMax(v1.x(), v2.x()), qMax(v1.y(), v2.y()), qMax(v1.z(), v2.z()) }; }
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

Q_DEMON_EXPORT const char *nonNull(const char *src);

Q_DEMON_EXPORT float radToDeg(const float a);
Q_DEMON_EXPORT double radToDeg(const double a);
Q_DEMON_EXPORT float degToRad(const float a);
Q_DEMON_EXPORT double degToRad(const double a);


QT_END_NAMESPACE

#endif // QDEMONUTILS_H
