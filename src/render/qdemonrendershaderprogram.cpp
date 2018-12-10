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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemonRender/qdemonrendervertexshader.h>
#include <QtDemonRender/qdemonrenderfragmentshader.h>
#include <QtDemonRender/qdemonrendertessellationshader.h>
#include <QtDemonRender/qdemonrendergeometryshader.h>
#include <QtDemonRender/qdemonrendercomputeshader.h>
#include <QtDemonRender/qdemonrenderimagetexture.h>
#include <QtDemon/qdemonutils.h>

#include <limits>

QT_BEGIN_NAMESPACE

template <typename TDataType>
struct ShaderConstantApplier
{
    bool force_compile_error;
};

template <>
struct ShaderConstantApplier<qint32>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32 &inValue,
                       qint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_2>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_2 &inValue,
                       qint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_3>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_3 &inValue,
                       qint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_4>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_4 &inValue,
                       qint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       const bool inValue, bool &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_2>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_2 &inValue,
                       bool_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_3>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_3 &inValue,
                       bool_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_4>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_4 &inValue,
                       bool_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<float>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const float &inValue,
                       float &oldValue)
    {
        if (count > 1 || !(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector2D>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector2D &inValue,
                       QVector2D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector3D>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector3D &inValue,
                       QVector3D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector4D>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector4D &inValue,
                       QVector4D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const quint32 &inValue,
                       quint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_2>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const quint32_2 &inValue,
                       quint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_3>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const quint32_3 &inValue,
                       quint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_4>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const quint32_4 &inValue,
                       quint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QMatrix3x3>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QMatrix3x3 inValue,
                       QMatrix3x3 &, bool inTranspose)
    {
        backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                  inValue.constData(), inTranspose);
    }
};

template <>
struct ShaderConstantApplier<QMatrix4x4>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QMatrix4x4 inValue,
                       QMatrix4x4 &, bool inTranspose)
    {
        backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                  inValue.constData(), inTranspose);
    }

    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonConstDataRef<QMatrix4x4> inValue, QMatrix4x4 &, bool inTranspose)
    {
        backend->SetConstantValue(program->GetShaderProgramHandle(), location, type, count,
                                  reinterpret_cast<const GLfloat *>(inValue.begin()),
                                  inTranspose);
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DPtr>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DPtr inValue, quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2D *texObj = reinterpret_cast<QDemonRenderTexture2D *>(inValue);
            texObj->Bind();
            quint32 texUnit = texObj->GetTextureUnit();
            if (texUnit != oldValue) {
                backend->SetConstantValue(program->GetShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DHandle>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend,
                       qint32 location, qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DHandle inValue, QVector<quint32> &oldValue)
    {
        Q_UNUSED(type)
        if (inValue) {
            bool update = false;
            for (int i = 0; i < count; i++) {
                QDemonRenderTexture2D *texObj = reinterpret_cast<QDemonRenderTexture2D *>(inValue[i]);
                quint32 texUnit = std::numeric_limits<quint32>::max();
                if (texObj) {
                    texObj->Bind();
                    texUnit = texObj->GetTextureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->SetConstantValue(program->GetShaderProgramHandle(), location,
                                          QDemonRenderShaderDataTypes::Texture2D,
                                          count, oldValue.data());
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DArrayPtr>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DArrayPtr inValue, quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2DArray *texObj =
                    reinterpret_cast<QDemonRenderTexture2DArray *>(inValue);
            texObj->Bind();
            quint32 texUnit = texObj->GetTextureUnit();
            if (texUnit != oldValue) {
                backend->SetConstantValue(program->GetShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTextureCubePtr>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTextureCubePtr inValue, quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTextureCube *texObj = reinterpret_cast<QDemonRenderTextureCube *>(inValue);
            texObj->Bind();
            quint32 texUnit = texObj->GetTextureUnit();
            if (texUnit != oldValue) {
                backend->SetConstantValue(program->GetShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTextureCubeHandle>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend,
                       qint32 location, qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTextureCubeHandle inValue, QVector<quint32> &oldValue)
    {
        Q_UNUSED(type)
        if (inValue) {
            bool update = false;
            for (int i = 0; i < count; i++) {
                QDemonRenderTextureCube *texObj = reinterpret_cast<QDemonRenderTextureCube *>(inValue[i]);
                quint32 texUnit = std::numeric_limits<quint32>::max();
                if (texObj) {
                    texObj->Bind();
                    texUnit = texObj->GetTextureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->SetConstantValue(program->GetShaderProgramHandle(), location,
                                          QDemonRenderShaderDataTypes::TextureCube,
                                          count, oldValue.data());
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderImage2DPtr>
{
    void ApplyConstant(QDemonRenderShaderProgram *program, QDemonRenderBackend *backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderImage2DPtr inValue, quint32 &oldValue, qint32 binding)
    {
        if (inValue) {
            QDemonRenderImage2D *imgObj = reinterpret_cast<QDemonRenderImage2D *>(inValue);
            imgObj->Bind(binding);
            quint32 texUnit = imgObj->GetTextureUnit();
            if (texUnit != oldValue) {
                // on ES we need a explicit binding value
                Q_ASSERT(backend->GetRenderContextType() != QDemonRenderContextValues::GLES3PLUS
                        || binding != -1);
                // this is not allowed on ES 3+ for image types
                if (backend->GetRenderContextType() != QDemonRenderContextValues::GLES3PLUS)
                    backend->SetConstantValue(program->GetShaderProgramHandle(), location, type,
                                              count, &texUnit);

                oldValue = texUnit;
            }
        }
    }
};

QDemonRenderShaderProgram::QDemonRenderShaderProgram(QDemonRenderContextImpl &context,
                                                     const char *programName,
                                                     bool separableProgram)
    : m_Context(context)
    , m_Backend(context.GetBackend())
    , m_ProgramName(programName)
    , m_ProgramHandle(nullptr)
    , m_ProgramType(ProgramType::Graphics)
{
    m_ProgramHandle = m_Backend->CreateShaderProgram(separableProgram);

    Q_ASSERT(m_ProgramHandle);
}

QDemonRenderShaderProgram::~QDemonRenderShaderProgram()
{
    m_Context.ShaderDestroyed(this);

    if (m_ProgramHandle)
        m_Backend->ReleaseShaderProgram(m_ProgramHandle);

    for (TShaderConstantMap::iterator iter = m_Constants.begin(), end = m_Constants.end();
         iter != end; ++iter) {
        //iter.value()->Release();
    }

    m_Constants.clear();

    for (TShaderBufferMap::iterator iter = m_ShaderBuffers.begin(), end = m_ShaderBuffers.end();
         iter != end; ++iter) {
        //iter.value()->Release();
    }

    m_ShaderBuffers.clear();

    m_ProgramHandle = nullptr;
}

template <typename TShaderObject>
void QDemonRenderShaderProgram::Attach(TShaderObject *pShader)
{
    m_Backend->AttachShader(m_ProgramHandle, pShader->GetShaderHandle());
}

template <typename TShaderObject>
void QDemonRenderShaderProgram::Detach(TShaderObject *pShader)
{
    m_Backend->DetachShader(m_ProgramHandle, pShader->GetShaderHandle());
}

static QDemonRenderShaderConstantBase *
ShaderConstantFactory(QDemonRenderBackend *backend, const QString &inName,
                      qint32 uniLoc, qint32 elementCount,
                      QDemonRenderShaderDataTypes::Enum inConstantType, qint32 binding)
{
    switch (inConstantType) {
    case QDemonRenderShaderDataTypes::Integer:
        return new QDemonRenderShaderConstant<qint32>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::IntegerVec2:
        return new QDemonRenderShaderConstant<qint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::IntegerVec3:
        return new QDemonRenderShaderConstant<qint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::IntegerVec4:
        return new QDemonRenderShaderConstant<qint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Boolean:
        return new QDemonRenderShaderConstant<bool>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::BooleanVec2:
        return new QDemonRenderShaderConstant<bool_2>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::BooleanVec3:
        return new QDemonRenderShaderConstant<bool_3>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::BooleanVec4:
        return new QDemonRenderShaderConstant<bool_4>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Float:
        return new QDemonRenderShaderConstant<float>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Vec2:
        return new QDemonRenderShaderConstant<QVector2D>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Vec3:
        return new QDemonRenderShaderConstant<QVector3D>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Vec4:
        return new QDemonRenderShaderConstant<QVector4D>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::UnsignedInteger:
        return new QDemonRenderShaderConstant<quint32>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
        return new QDemonRenderShaderConstant<quint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
        return new QDemonRenderShaderConstant<quint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
        return new QDemonRenderShaderConstant<quint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Matrix3x3:
        return new QDemonRenderShaderConstant<QMatrix3x3>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Matrix4x4:
        return new QDemonRenderShaderConstant<QMatrix4x4>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Texture2D:
        return new QDemonRenderShaderConstant<QDemonRenderTexture2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Texture2DHandle:
        return new QDemonRenderShaderConstant<QDemonRenderTexture2DHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Texture2DArray:
        return new QDemonRenderShaderConstant<QDemonRenderTexture2DArrayPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::TextureCube:
        return new QDemonRenderShaderConstant<QDemonRenderTextureCubePtr>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::TextureCubeHandle:
        return new QDemonRenderShaderConstant<QDemonRenderTextureCubeHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::Image2D:
        return new QDemonRenderShaderConstant<QDemonRenderImage2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    case QDemonRenderShaderDataTypes::DataBuffer:
        return new QDemonRenderShaderConstant<QDemonRenderDataBufferPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding);
    default:
        break;
    }
    Q_ASSERT(false);
    return nullptr;
}

template <typename TShaderBufferType, typename TBufferDataType>
static QDemonRenderShaderBufferBase *
ShaderBufferFactory(QDemonRenderContextImpl &context, const QString &inName,
                    qint32 cbLoc, qint32 cbBinding, qint32 cbSize,
                    qint32 cbCount, TBufferDataType *pBuffer)
{
    return new TShaderBufferType(context, inName, cbLoc, cbBinding, cbSize, cbCount, pBuffer);
}

bool QDemonRenderShaderProgram::Link()
{
    bool success = m_Backend->LinkProgram(m_ProgramHandle, m_ErrorMessage);

    if (success) {
        char nameBuf[512];
        qint32 location, elementCount, binding;
        QDemonRenderShaderDataTypes::Enum type;

        qint32 constantCount = m_Backend->GetConstantCount(m_ProgramHandle);

        QDEMON_FOREACH(idx, constantCount)
        {
            location = m_Backend->GetConstantInfoByID(m_ProgramHandle, idx, 512, &elementCount,
                                                      &type, &binding, nameBuf);

            // sampler arrays have different type
            if (type == QDemonRenderShaderDataTypes::Texture2D && elementCount > 1) {
                type = QDemonRenderShaderDataTypes::Texture2DHandle;
            } else if (type == QDemonRenderShaderDataTypes::TextureCube
                       && elementCount > 1) {
                type = QDemonRenderShaderDataTypes::TextureCubeHandle;
            }
            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);
                m_Constants.insert(theName,
                                   ShaderConstantFactory(m_Backend, theName,
                                                         location, elementCount, type, binding));
            }
        }

        // next query constant buffers info
        qint32 length, bufferSize, paramCount;
        qint32 constantBufferCount = m_Backend->GetConstantBufferCount(m_ProgramHandle);
        QDEMON_FOREACH(idx, constantBufferCount)
        {
            location = m_Backend->GetConstantBufferInfoByID(
                        m_ProgramHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);

                // find constant buffer in our DB
                QDemonRenderConstantBuffer *cb = m_Context.GetConstantBuffer(theName);
                if (cb) {
                    cb->SetupBuffer(QSharedPointer<QDemonRenderShaderProgram>(this), location, bufferSize, paramCount);
                    //cb->addRef();
                }

                m_ShaderBuffers.insert(theName, ShaderBufferFactory<QDemonRenderShaderConstantBuffer, QDemonRenderConstantBuffer>(
                                           m_Context, theName, location, -1, bufferSize,
                                           paramCount, cb));
            }
        }

        // next query storage buffers
        qint32 storageBufferCount = m_Backend->GetStorageBufferCount(m_ProgramHandle);
        QDEMON_FOREACH(idx, storageBufferCount)
        {
            location = m_Backend->GetStorageBufferInfoByID(
                        m_ProgramHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);

                // find constant buffer in our DB
                QDemonRenderStorageBuffer *sb = m_Context.GetStorageBuffer(theName);
                if (sb) {
                    //sb->addRef();
                }

                m_ShaderBuffers.insert(theName, ShaderBufferFactory<QDemonRenderShaderStorageBuffer, QDemonRenderStorageBuffer>(
                                           m_Context, theName, location, -1, bufferSize,
                                           paramCount, sb));
            }
        }

        // next query atomic counter buffers
        qint32 atomicBufferCount = m_Backend->GetAtomicCounterBufferCount(m_ProgramHandle);
        QDEMON_FOREACH(idx, atomicBufferCount)
        {
            location = m_Backend->GetAtomicCounterBufferInfoByID(
                        m_ProgramHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);

                // find atomic counter buffer in our DB
                // The buffer itself is not used in the program itself.
                // Instead uniform variables are used but the interface to set the value is like
                // for buffers.
                // This is a bit insane but that is how it is.
                // The theName variable contains the uniform name associated with an atomic
                // counter buffer.
                // We get the actual buffer name by searching for this uniform name
                // See NVRenderTestAtomicCounterBuffer.cpp how the setup works
                QDemonRenderAtomicCounterBuffer *acb =
                        m_Context.GetAtomicCounterBufferByParam(theName);
                if (acb) {
                    //acb->addRef();

                    m_ShaderBuffers.insert(acb->GetBufferName(), ShaderBufferFactory<QDemonRenderShaderAtomicCounterBuffer,
                                           QDemonRenderAtomicCounterBuffer>(m_Context, acb->GetBufferName(),
                                                                            location, -1, bufferSize, paramCount, acb));
                }
            }
        }
    }

    return success;
}

void QDemonRenderShaderProgram::GetErrorMessage(qint32 *messageLength, const char *errorMessage)
{
    *messageLength = m_ErrorMessage.size();
    errorMessage = qPrintable(m_ErrorMessage);
}

const char *QDemonRenderShaderProgram::GetErrorMessage() { return qPrintable(m_ErrorMessage); }

QDemonRenderShaderConstantBase *QDemonRenderShaderProgram::GetShaderConstant(const char *constantName)
{
    TShaderConstantMap::iterator theIter =
            m_Constants.find(QString::fromLocal8Bit(constantName));

    if (theIter != m_Constants.end()) {
        QDemonRenderShaderConstantBase *theConstant =
                static_cast<QDemonRenderShaderConstantBase *>(theIter.value());
        return theConstant;
    }

    return nullptr;
}

QDemonRenderShaderBufferBase *QDemonRenderShaderProgram::GetShaderBuffer(const char *bufferName)
{
    TShaderBufferMap::iterator theIter =
            m_ShaderBuffers.find(QString::fromLocal8Bit(bufferName));

    if (theIter != m_ShaderBuffers.end()) {
        return theIter.value();
    }

    return nullptr;
}

QDemonRenderContextImpl &QDemonRenderShaderProgram::GetRenderContext() { return m_Context; }

template <typename TDataType>
void SetConstantValueOfType(QDemonRenderShaderProgram *program,
                            QDemonRenderShaderConstantBase *inConstantBase,
                            const TDataType &inValue, const qint32 inCount)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_ElementCount >= inCount);

    if (inConstantBase->GetShaderConstantType()
            == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant =
                static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().ApplyConstant(
                    program, inConstant->m_Backend, inConstant->m_Location, inCount, inConstant->m_Type,
                    inValue, inConstant->m_Value);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void SetSamplerConstantValueOfType(QDemonRenderShaderProgram *program,
                                   QDemonRenderShaderConstantBase *inConstantBase,
                                   const TDataType &inValue, const qint32 inCount)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_ElementCount >= inCount);

    if (inConstantBase->GetShaderConstantType()
            == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant =
                static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().ApplyConstant(
                    program, inConstant->m_Backend, inConstant->m_Location, inCount, inConstant->m_Type,
                    inValue, inConstant->m_Value, inConstant->m_Binding);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void SetMatrixConstantValueOfType(QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const TDataType &inValue, const qint32 inCount,
                                  bool inTranspose)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_ElementCount >= inCount);

    if (inConstantBase->GetShaderConstantType()
            == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant =
                static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().ApplyConstant(
                    program, inConstant->m_Backend, inConstant->m_Location, inCount, inConstant->m_Type,
                    inValue, inConstant->m_Value, inTranspose);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void SetMatrixConstantValueOfType(QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const QDemonConstDataRef<TDataType> inValue,
                                  const qint32 /*inCount*/, bool inTranspose)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_ElementCount >= (qint32)inValue.size());

    if (inConstantBase->GetShaderConstantType()
            == QDemonDataTypeToShaderDataTypeMap<TDataType>::GetType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant =
                static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().ApplyConstant(
                    program, inConstant->m_Backend, inConstant->m_Location, inValue.size(),
                    inConstant->m_Type, inValue, inConstant->m_Value, inTranspose);
    } else {
        Q_ASSERT(false);
    }
}

void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 qint32 inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_2 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_3 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_4 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 bool inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_2 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_3 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_4 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const float &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector2D &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector3D &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector4D &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_2 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_3 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_4 &inValue, const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix3x3 &inValue, const qint32 inCount,
                                                 bool inTranspose)
{
    SetMatrixConstantValueOfType(this, inConstant, inValue, inCount, inTranspose);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix4x4 &inValue, const qint32 inCount,
                                                 bool inTranspose)
{
    SetMatrixConstantValueOfType(this, inConstant, inValue, inCount, inTranspose);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QDemonConstDataRef<QMatrix4x4> inValue,
                                                 const qint32 inCount)
{
    SetMatrixConstantValueOfType(this, inConstant, inValue, inCount, false);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2D *inValue, const qint32 inCount)
{
    Q_UNUSED(inCount)
    SetConstantValueOfType(this, inConstant, inValue, 1);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2D **inValue,
                                                 const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2DArray *inValue,
                                                 const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTextureCube *inValue, const qint32 inCount)
{
    Q_UNUSED(inCount)
    SetConstantValueOfType(this, inConstant, inValue, 1);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTextureCube **inValue,
                                                 const qint32 inCount)
{
    SetConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderImage2D *inValue, const qint32 inCount)
{
    SetSamplerConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::SetConstantValue(QDemonRenderShaderConstantBase *, QDemonRenderDataBuffer *,
                                                 const qint32)
{
    // this is merely a dummy right now
}

void QDemonRenderShaderProgram::BindComputeInput(QDemonRenderDataBuffer *inBuffer, quint32 inIndex)
{
    QDemonRenderBackend::QDemonRenderBackendBufferObject obj(nullptr);
    if (inBuffer)
        obj = inBuffer->GetBuffertHandle();
    m_Backend->ProgramSetStorageBuffer(inIndex, obj);
}

namespace {
void WriteErrorMessage(const char *tag, const char *message)
{
    const QString messageData = QString::fromLocal8Bit(message);
    const auto lines = messageData.splitRef('\n');
    for (const auto &line : lines )
        qCCritical(INVALID_OPERATION, "%s: %s", tag, line.toLocal8Bit().constData());
}
}

QDemonOption<QDemonRenderVertexShader *> QDemonRenderShaderProgram::createVertexShader(
        QDemonRenderContextImpl &context, QDemonConstDataRef<qint8> vertexShaderSource, bool binaryProgram)
{
    if (vertexShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderVertexShader(context, vertexShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderFragmentShader *> QDemonRenderShaderProgram::createFragmentShader(
        QDemonRenderContextImpl &context, QDemonConstDataRef<qint8> fragmentShaderSource, bool binaryProgram)
{
    if (fragmentShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderFragmentShader(context, fragmentShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderTessControlShader *>
QDemonRenderShaderProgram::createTessControlShader(QDemonRenderContextImpl &context,
                                                   QDemonConstDataRef<qint8> tessControlShaderSource,
                                                   bool binaryProgram)
{
    if (tessControlShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderTessControlShader(context, tessControlShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderTessEvaluationShader *>
QDemonRenderShaderProgram::createTessEvaluationShader(QDemonRenderContextImpl &context,
                                                      QDemonConstDataRef<qint8> tessControlShaderSource,
                                                      bool binaryProgram)
{
    if (tessControlShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderTessEvaluationShader(context, tessControlShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderGeometryShader *> QDemonRenderShaderProgram::createGeometryShader(
        QDemonRenderContextImpl &context, QDemonConstDataRef<qint8> geometryShaderSource, bool binaryProgram)
{
    if (geometryShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderGeometryShader(context, geometryShaderSource, binaryProgram);
}

QDemonRenderVertFragCompilationResult QDemonRenderShaderProgram::Create(
        QDemonRenderContextImpl &context, const char *programName,
        QDemonConstDataRef<qint8> vertShaderSource, QDemonConstDataRef<qint8> fragShaderSource,
        QDemonConstDataRef<qint8> tessControlShaderSource,
        QDemonConstDataRef<qint8> tessEvaluationShaderSource, QDemonConstDataRef<qint8> geometryShaderSource,
        bool separateProgram, QDemonRenderShaderProgramBinaryType::Enum type, bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result;
    QSharedPointer<QDemonRenderShaderProgram> pProgram = nullptr;
    bool bProgramIsValid = false;

    result.mShaderName = programName;

    // our minimum requirement is a vertex and a fragment shader or geometry shader
    // if we should treat it as a separate program we don't care
    if (!separateProgram
            && (vertShaderSource.size() == 0
                || (fragShaderSource.size() == 0 && geometryShaderSource.size() == 0))) {
        qCCritical(INVALID_PARAMETER,
                   "Vertex or fragment (geometry) source have 0 length");
        Q_ASSERT(false);
        return result;
    }

    if (binaryProgram && type != QDemonRenderShaderProgramBinaryType::NVBinary) {
        qCCritical(INVALID_PARAMETER, "Unrecoginzed binary format");
        Q_ASSERT(false);
        return result;
    }

    // first create and compile shader
    QDemonOption<QDemonRenderVertexShader *> vtxShader =
            createVertexShader(context, vertShaderSource, binaryProgram);
    QDemonOption<QDemonRenderFragmentShader *> fragShader =
            createFragmentShader(context, fragShaderSource, binaryProgram);
    QDemonOption<QDemonRenderTessControlShader *> tcShader =
            createTessControlShader(context, tessControlShaderSource, binaryProgram);
    QDemonOption<QDemonRenderTessEvaluationShader *> teShader =
            createTessEvaluationShader(context, tessEvaluationShaderSource, binaryProgram);
    QDemonOption<QDemonRenderGeometryShader *> geShader =
            createGeometryShader(context, geometryShaderSource, binaryProgram);

    bool vertexValid = (vtxShader.hasValue()) ? vtxShader.getValue()->IsValid() : true;
    bool fragValid = (fragShader.hasValue()) ? fragShader.getValue()->IsValid() : true;
    bool tcValid = (tcShader.hasValue()) ? tcShader.getValue()->IsValid() : true;
    bool teValid = (teShader.hasValue()) ? teShader.getValue()->IsValid() : true;
    bool geValid = (geShader.hasValue()) ? geShader.getValue()->IsValid() : true;

    if (vertexValid && fragValid && tcValid && teValid && geValid) {
        // shaders were succesfuly created
        pProgram.reset(new QDemonRenderShaderProgram(context, programName, separateProgram));

        if (pProgram) {
            // attach programs
            if (vtxShader.hasValue() && vtxShader.getValue()->IsValid())
                pProgram->Attach(vtxShader.getValue());
            if (fragShader.hasValue() && fragShader.getValue()->IsValid())
                pProgram->Attach(fragShader.getValue());
            if (tcShader.hasValue() && tcShader.getValue()->IsValid())
                pProgram->Attach(tcShader.getValue());
            if (teShader.hasValue() && teShader.getValue()->IsValid())
                pProgram->Attach(teShader.getValue());
            if (geShader.hasValue() && geShader.getValue()->IsValid())
                pProgram->Attach(geShader.getValue());

            // link program
            bProgramIsValid = pProgram->Link();
        }
    }

    // if anything went wrong print out
    if (!vertexValid || !fragValid || !tcValid || !teValid || !geValid || !bProgramIsValid) {

        if (!vertexValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate vertex shader!!");
            qCCritical(INTERNAL_ERROR, "Vertex source:\n%s",
                       nonNull((const char *)vertShaderSource.begin()));
            WriteErrorMessage("Vertex compilation output:",
                              vtxShader.getValue()->GetErrorMessage());
        }

        if (!fragValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate fragment shader!!");
            qCCritical(INTERNAL_ERROR, "Fragment source:\n%s",
                       nonNull((const char *)fragShaderSource.begin()));
            WriteErrorMessage("Fragment compilation output:",
                              fragShader.getValue()->GetErrorMessage());
        }

        if (!tcValid) {
            qCCritical(INTERNAL_ERROR,
                       "Failed to generate tessellation control shader!!");
            qCCritical(INTERNAL_ERROR, "Tessellation control source:\n%s",
                       nonNull((const char *)tessControlShaderSource.begin()));
            WriteErrorMessage("Tessellation control compilation output:",
                              tcShader.getValue()->GetErrorMessage());
        }

        if (!teValid) {
            qCCritical(INTERNAL_ERROR,
                       "Failed to generate tessellation evaluation shader!!");
            qCCritical(INTERNAL_ERROR, "Tessellation evaluation source:\n%s",
                       nonNull((const char *)tessEvaluationShaderSource.begin()));
            WriteErrorMessage("Tessellation evaluation compilation output:",
                              teShader.getValue()->GetErrorMessage());
        }

        if (!geValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate geometry shader!!");
            qCCritical(INTERNAL_ERROR, "Geometry source:\n%s",
                       nonNull((const char *)geometryShaderSource.begin()));
            WriteErrorMessage("Geometry compilation output:",
                              geShader.getValue()->GetErrorMessage());
        }

        if (!bProgramIsValid && pProgram) {
            qCCritical(INTERNAL_ERROR, "Failed to link program!!");
            WriteErrorMessage("Program link output:", pProgram->GetErrorMessage());

            // delete program
            pProgram.clear();
        }
    }

    // clean up
    if (vtxShader.hasValue()) {
        if (bProgramIsValid && vtxShader.getValue()->IsValid())
            pProgram->Detach(vtxShader.getValue());
        ::free(vtxShader.getValue());
    }
    if (fragShader.hasValue()) {
        if (bProgramIsValid && fragShader.getValue()->IsValid())
            pProgram->Detach(fragShader.getValue());
        ::free(fragShader.getValue());
    }
    if (tcShader.hasValue()) {
        if (bProgramIsValid && tcShader.getValue()->IsValid())
            pProgram->Detach(tcShader.getValue());
        ::free(tcShader.getValue());
    }
    if (teShader.hasValue()) {
        if (bProgramIsValid && teShader.getValue()->IsValid())
            pProgram->Detach(teShader.getValue());
        ::free(teShader.getValue());
    }
    if (geShader.hasValue()) {
        if (bProgramIsValid && geShader.getValue()->IsValid())
            pProgram->Detach(geShader.getValue());
        ::free(geShader.getValue());
    }

    // set program
    result.mShader = pProgram;

    return result;
}

QDemonRenderVertFragCompilationResult
QDemonRenderShaderProgram::CreateCompute(QDemonRenderContextImpl &context, const char *programName,
                                         QDemonConstDataRef<qint8> computeShaderSource)
{
    QDemonRenderVertFragCompilationResult result;
    QSharedPointer<QDemonRenderShaderProgram> pProgram = nullptr;
    bool bProgramIsValid = true;

    result.mShaderName = programName;

    // check source
    if (computeShaderSource.size() == 0) {
        qCCritical(INVALID_PARAMETER, "compute source has 0 length");
        Q_ASSERT(false);
        return result;
    }

    QDemonRenderComputeShader computeShader(context, computeShaderSource, false);

    if (computeShader.IsValid()) {
        // shaders were succesfuly created
        pProgram.reset(new QDemonRenderShaderProgram(context, programName, false));

        if (pProgram) {
            // attach programs
            pProgram->Attach(&computeShader);

            // link program
            bProgramIsValid = pProgram->Link();

            // set program type
            pProgram->SetProgramType(ProgramType::Compute);
        }
    }

    // if anything went wrong print out
    if (!computeShader.IsValid() || !bProgramIsValid) {

        if (!computeShader.IsValid()) {
            qCCritical(INTERNAL_ERROR, "Failed to generate compute shader!!");
            qCCritical(INTERNAL_ERROR, "Vertex source:\n%s",
                       nonNull((const char *)computeShaderSource.begin()));
            WriteErrorMessage("Compute shader compilation output:",
                              computeShader.GetErrorMessage());
        }
    }

    // set program
    result.mShader = pProgram;

    return result;
}
QT_END_NAMESPACE
