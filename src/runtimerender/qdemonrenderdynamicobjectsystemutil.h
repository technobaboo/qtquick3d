/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
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

// ### Not sure what to do with these guys
//struct SStringLoadRemapper
//{
//    CStrTableOrDataRef m_StrData;
//    QString m_PathMapper;
//    const char *m_ProjectDir;
//    SStringLoadRemapper(CStrTableOrDataRef inData, const char *inProjectDir)
//        : m_StrData(inData)
//        , m_ProjectDir(inProjectDir)
//    {
//    }
//    void Remap(QString &inStr) { inStr.Remap(m_StrData); }
//};

//struct SStringSaveRemapper
//{
//    const SStrRemapMap &m_Map;
//    QString m_RelativeBuffer;
//    QString m_ProjectDir;
//    QString m_FinalBuffer;
//    SStringSaveRemapper(const SStrRemapMap &map, const char *inProjectDir)
//        : m_Map(map)
//        , m_StringTable(inStrTable)
//    {
//        m_ProjectDir = QString::fromLocal8Bit(inProjectDir);
//    }
//    void Remap(QString &inStr) { inStr.Remap(m_Map); }
//};

inline quint32 Align(quint32 inValue)
{
    if (inValue % 4)
        return inValue + (4 - (inValue % 4));
    return inValue;
}

inline quint32 Align8(quint32 inValue)
{
    if (inValue % 8)
        return inValue + (8 - (inValue % 8));
    return inValue;
}

inline quint32 getSizeofShaderDataType(QDemonRenderShaderDataTypes::Enum value)
{
    switch (value) {
    case QDemonRenderShaderDataTypes::Unknown:
        return 0;
    case QDemonRenderShaderDataTypes::Integer:            //qint32,
        return sizeof(qint32);
    case QDemonRenderShaderDataTypes::IntegerVec2:        //qint32_2,
        return sizeof(qint32_2);
    case QDemonRenderShaderDataTypes::IntegerVec3:        //qint32_3,
        return sizeof(qint32_3);
    case QDemonRenderShaderDataTypes::IntegerVec4:        //qint32_4,
        return sizeof(qint32_4);
    case QDemonRenderShaderDataTypes::Boolean:        //bool
        return sizeof(bool);
    case QDemonRenderShaderDataTypes::BooleanVec2:        //bool_2,
        return sizeof(bool_2);
    case QDemonRenderShaderDataTypes::BooleanVec3:        //bool_3,
        return sizeof(bool_3);
    case QDemonRenderShaderDataTypes::BooleanVec4:        //bool_4,
        return sizeof(bool_4);
    case QDemonRenderShaderDataTypes::Float:              //float,
        return sizeof(float);
    case QDemonRenderShaderDataTypes::Vec2:              //QVector2D,
        return sizeof(QVector2D);
    case QDemonRenderShaderDataTypes::Vec3:               //QVector3D,
        return sizeof(QVector3D);
    case QDemonRenderShaderDataTypes::Vec4:               //QVector4D,
        return sizeof(QVector4D);
    case QDemonRenderShaderDataTypes::UnsignedInteger:    //quint32,
        return sizeof(quint32);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2://quint32_2,
        return sizeof(quint32_2);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3://quint32_3,
        return sizeof(quint32_3);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4://quint32_4,
        return sizeof(quint32_4);
    case QDemonRenderShaderDataTypes::Matrix3x3:          //QMatrix3x3,
        return sizeof(QMatrix3x3);
    case QDemonRenderShaderDataTypes::Matrix4x4:          //QMatrix4x4,
        return sizeof(QMatrix4x4);
    case QDemonRenderShaderDataTypes::Texture2D:          //QDemonRenderTexture2DPtr,
        return sizeof(QDemonRenderTexture2DPtr);
    case QDemonRenderShaderDataTypes::Texture2DHandle:    //QDemonRenderTexture2DHandle,
        return sizeof(QDemonRenderTexture2DHandle);
    case QDemonRenderShaderDataTypes::Texture2DArray:     //QDemonRenderTexture2DArrayPtr,
        return sizeof(QDemonRenderTexture2DArrayPtr);
    case QDemonRenderShaderDataTypes::TextureCube:        //QDemonRenderTextureCubePtr,
        return sizeof(QDemonRenderTextureCubePtr);
    case QDemonRenderShaderDataTypes::TextureCubeHandle:  //QDemonRenderTextureCubeHandle,
        return sizeof(QDemonRenderTextureCubeHandle);
    case QDemonRenderShaderDataTypes::Image2D:            //QDemonRenderImage2DPtr,
        return sizeof(QDemonRenderImage2DPtr);
    case QDemonRenderShaderDataTypes::DataBuffer:          //QDemonRenderDataBufferPtr
        return sizeof(QDemonRenderDataBufferPtr);
    }
    Q_UNREACHABLE();
    return 0;
}

inline const char *GetShaderDatatypeName(QDemonRenderShaderDataTypes::Enum inValue)
{
    switch (inValue) {
    case QDemonRenderShaderDataTypes::Unknown:
        return nullptr;
    case QDemonRenderShaderDataTypes::Integer:            //qint32,
        return "qint32";
    case QDemonRenderShaderDataTypes::IntegerVec2:        //qint32_2,
        return "qint32_2";
    case QDemonRenderShaderDataTypes::IntegerVec3:        //qint32_3,
        return "qint32_3";
    case QDemonRenderShaderDataTypes::IntegerVec4:        //qint32_4,
        return "qint32_4";
    case QDemonRenderShaderDataTypes::Boolean:        //bool
        return "bool";
    case QDemonRenderShaderDataTypes::BooleanVec2:        //bool_2,
        return "bool_2";
    case QDemonRenderShaderDataTypes::BooleanVec3:        //bool_3,
        return "bool_3";
    case QDemonRenderShaderDataTypes::BooleanVec4:        //bool_4,
        return "bool_4";
    case QDemonRenderShaderDataTypes::Float:              //float,
        return "float";
    case QDemonRenderShaderDataTypes::Vec2:              //QVector2D,
        return "QVector2D";
    case QDemonRenderShaderDataTypes::Vec3:               //QVector3D,
        return "QVector3D";
    case QDemonRenderShaderDataTypes::Vec4:               //QVector4D,
        return "QVector4D";
    case QDemonRenderShaderDataTypes::UnsignedInteger:    //quint32,
        return "quint32";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2://quint32_2,
        return "quint32_2";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3://quint32_3,
        return "quint32_3";
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4://quint32_4,
        return "quint32_4";
    case QDemonRenderShaderDataTypes::Matrix3x3:          //QMatrix3x3,
        return "QMatrix3x3";
    case QDemonRenderShaderDataTypes::Matrix4x4:          //QMatrix4x4,
        return "QMatrix4x4";
    case QDemonRenderShaderDataTypes::Texture2D:          //QDemonRenderTexture2DPtr,
        return "QDemonRenderTexture2DPtr";
    case QDemonRenderShaderDataTypes::Texture2DHandle:    //QDemonRenderTexture2DHandle,
        return "QDemonRenderTexture2DHandle";
    case QDemonRenderShaderDataTypes::Texture2DArray:     //QDemonRenderTexture2DArrayPtr,
        return "QDemonRenderTexture2DArrayPtr";
    case QDemonRenderShaderDataTypes::TextureCube:        //QDemonRenderTextureCubePtr,
        return "QDemonRenderTextureCubePtr";
    case QDemonRenderShaderDataTypes::TextureCubeHandle:  //QDemonRenderTextureCubeHandle,
        return "QDemonRenderTextureCubeHandle";
    case QDemonRenderShaderDataTypes::Image2D:            //QDemonRenderImage2DPtr,
        return "QDemonRenderImage2DPtr";
    case QDemonRenderShaderDataTypes::DataBuffer:          //QDemonRenderDataBufferPtr
        return "QDemonRenderDataBufferPtr";
    }
}
}
QT_END_NAMESPACE

#endif
