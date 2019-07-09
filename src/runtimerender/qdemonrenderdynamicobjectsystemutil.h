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

#ifndef QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_UTIL_H
#define QDEMON_RENDER_DYNAMIC_OBJECT_SYSTEM_UTIL_H

#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE
namespace dynamic {

constexpr inline quint32 align(quint32 inValue) Q_DECL_NOTHROW
{
    return (inValue % 4) ? (inValue + (4 - (inValue % 4))) : inValue;
}

constexpr inline quint32 align8(quint32 inValue) Q_DECL_NOTHROW
{
    return (inValue % 8) ? (inValue + (8 - (inValue % 8))) : inValue;
}

inline quint32 getSizeofShaderDataType(QDemonRenderShaderDataType value)
{
    switch (value) {
    case QDemonRenderShaderDataType::Unknown:
        return 0;
    case QDemonRenderShaderDataType::Integer: // qint32,
        return sizeof(qint32);
    case QDemonRenderShaderDataType::IntegerVec2: // qint32_2,
        return sizeof(qint32_2);
    case QDemonRenderShaderDataType::IntegerVec3: // qint32_3,
        return sizeof(qint32_3);
    case QDemonRenderShaderDataType::IntegerVec4: // qint32_4,
        return sizeof(qint32_4);
    case QDemonRenderShaderDataType::Boolean: // bool
        return sizeof(bool);
    case QDemonRenderShaderDataType::BooleanVec2: // bool_2,
        return sizeof(bool_2);
    case QDemonRenderShaderDataType::BooleanVec3: // bool_3,
        return sizeof(bool_3);
    case QDemonRenderShaderDataType::BooleanVec4: // bool_4,
        return sizeof(bool_4);
    case QDemonRenderShaderDataType::Float: // float,
        return sizeof(float);
    case QDemonRenderShaderDataType::Vec2: // QVector2D,
        return sizeof(QVector2D);
    case QDemonRenderShaderDataType::Vec3: // QVector3D,
        return sizeof(QVector3D);
    case QDemonRenderShaderDataType::Vec4: // QVector4D,
        return sizeof(QVector4D);
    case QDemonRenderShaderDataType::UnsignedInteger: // quint32,
        return sizeof(quint32);
    case QDemonRenderShaderDataType::UnsignedIntegerVec2: // quint32_2,
        return sizeof(quint32_2);
    case QDemonRenderShaderDataType::UnsignedIntegerVec3: // quint32_3,
        return sizeof(quint32_3);
    case QDemonRenderShaderDataType::UnsignedIntegerVec4: // quint32_4,
        return sizeof(quint32_4);
    case QDemonRenderShaderDataType::Matrix3x3: // QMatrix3x3,
        return sizeof(QMatrix3x3);
    case QDemonRenderShaderDataType::Matrix4x4: // QMatrix4x4,
        return sizeof(QMatrix4x4);
    case QDemonRenderShaderDataType::Texture2D: // QDemonRenderTexture2D *,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::Texture2DHandle: // QDemonRenderTexture2D **,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::Texture2DArray: // QDemonRenderTexture2DArray *,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::TextureCube: // QDemonRenderTextureCube *,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::TextureCubeHandle: // QDemonRenderTextureCube **,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::Image2D: // QDemonRenderImage2D *,
        Q_FALLTHROUGH();
    case QDemonRenderShaderDataType::DataBuffer: // QDemonRenderDataBufferPtr
        return sizeof (void *);
    }
    Q_UNREACHABLE();
    return 0;
}
}
QT_END_NAMESPACE

#endif
