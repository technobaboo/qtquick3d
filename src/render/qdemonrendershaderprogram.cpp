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
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32 &inValue,
                       qint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_2 &inValue,
                       qint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_3 &inValue,
                       qint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<qint32_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const qint32_4 &inValue,
                       qint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type,
                       const bool inValue, bool &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_2 &inValue,
                       bool_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_3 &inValue,
                       bool_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<bool_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const bool_4 &inValue,
                       bool_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<float>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const float &inValue,
                       float &oldValue)
    {
        if (count > 1 || !(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector2D>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector2D &inValue,
                       QVector2D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector3D>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector3D &inValue,
                       QVector3D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QVector4D>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const QVector4D &inValue,
                       QVector4D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32>
{
    void applyConstant(const QDemonRenderShaderProgram *program, const QDemonRef<QDemonRenderBackend> &backend, qint32 location,
                       qint32 count, QDemonRenderShaderDataTypes::Enum type, const quint32 &inValue,
                       quint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       const quint32_2 &inValue,
                       quint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       const quint32_3 &inValue,
                       quint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<quint32_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       const quint32_4 &inValue,
                       quint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                      &inValue.x);
            oldValue = inValue;
        }
    }
};

template <>
struct ShaderConstantApplier<QMatrix3x3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       const QMatrix3x3 inValue,
                       QMatrix3x3 &,
                       bool inTranspose)
    {
        backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                  inValue.constData(), inTranspose);
    }
};

template <>
struct ShaderConstantApplier<QMatrix4x4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type, const QMatrix4x4 inValue,
                       QMatrix4x4 &, bool inTranspose)
    {
        backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                  inValue.constData(), inTranspose);
    }

    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonConstDataRef<QMatrix4x4> inValue,
                       QMatrix4x4 &,
                       bool inTranspose)
    {
        backend->setConstantValue(program->getShaderProgramHandle(), location, type, count,
                                  reinterpret_cast<const GLfloat *>(inValue.begin()),
                                  inTranspose);
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DPtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2D *texObj = reinterpret_cast<QDemonRenderTexture2D *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->getTextureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->getShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DHandle>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DHandle inValue,
                       QVector<quint32> &oldValue)
    {
        Q_UNUSED(type)
        if (inValue) {
            bool update = false;
            for (int i = 0; i < count; i++) {
                QDemonRenderTexture2D *texObj = reinterpret_cast<QDemonRenderTexture2D *>(inValue[i]);
                quint32 texUnit = std::numeric_limits<quint32>::max();
                if (texObj) {
                    texObj->bind();
                    texUnit = texObj->getTextureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->setConstantValue(program->getShaderProgramHandle(), location,
                                          QDemonRenderShaderDataTypes::Texture2D,
                                          count, oldValue.data());
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTexture2DArrayPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTexture2DArrayPtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2DArray *texObj = reinterpret_cast<QDemonRenderTexture2DArray *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->getTextureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->getShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTextureCubePtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTextureCubePtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTextureCube *texObj = reinterpret_cast<QDemonRenderTextureCube *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->getTextureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->getShaderProgramHandle(), location, type,
                                          count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderTextureCubeHandle>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderTextureCubeHandle inValue,
                       QVector<quint32> &oldValue)
    {
        Q_UNUSED(type)
        if (inValue) {
            bool update = false;
            for (int i = 0; i < count; i++) {
                QDemonRenderTextureCube *texObj = reinterpret_cast<QDemonRenderTextureCube *>(inValue[i]);
                quint32 texUnit = std::numeric_limits<quint32>::max();
                if (texObj) {
                    texObj->bind();
                    texUnit = texObj->getTextureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->setConstantValue(program->getShaderProgramHandle(), location,
                                          QDemonRenderShaderDataTypes::TextureCube,
                                          count, oldValue.data());
        }
    }
};

template <>
struct ShaderConstantApplier<QDemonRenderImage2DPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataTypes::Enum type,
                       QDemonRenderImage2DPtr inValue,
                       quint32 &oldValue,
                       qint32 binding)
    {
        if (inValue) {
            QDemonRenderImage2D *imgObj = reinterpret_cast<QDemonRenderImage2D *>(inValue);
            imgObj->bind(binding);
            quint32 texUnit = imgObj->getTextureUnit();
            if (texUnit != oldValue) {
                // on ES we need a explicit binding value
                Q_ASSERT(backend->getRenderContextType() != QDemonRenderContextValues::GLES3PLUS
                        || binding != -1);
                // this is not allowed on ES 3+ for image types
                if (backend->getRenderContextType() != QDemonRenderContextValues::GLES3PLUS)
                    backend->setConstantValue(program->getShaderProgramHandle(), location, type,
                                              count, &texUnit);

                oldValue = texUnit;
            }
        }
    }
};

QDemonRenderShaderProgram::QDemonRenderShaderProgram(const QDemonRef<QDemonRenderContextImpl> &context,
                                                     const char *programName,
                                                     bool separableProgram)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_programName(programName)
    , m_programHandle(nullptr)
    , m_programType(ProgramType::Graphics)
{
    m_programHandle = m_backend->createShaderProgram(separableProgram);

    Q_ASSERT(m_programHandle);
}

QDemonRenderShaderProgram::~QDemonRenderShaderProgram()
{
    m_context->shaderDestroyed(this);

    if (m_programHandle)
        m_backend->releaseShaderProgram(m_programHandle);

    m_constants.clear();
    m_shaderBuffers.clear();

    m_programHandle = nullptr;
}

template <typename TShaderObject>
void QDemonRenderShaderProgram::attach(TShaderObject *pShader)
{
    m_backend->attachShader(m_programHandle, pShader->getShaderHandle());
}

template <typename TShaderObject>
void QDemonRenderShaderProgram::detach(TShaderObject *pShader)
{
    m_backend->detachShader(m_programHandle, pShader->getShaderHandle());
}

static QDemonRef<QDemonRenderShaderConstantBase> shaderConstantFactory(const QDemonRef<QDemonRenderBackend> &backend,
                                                                            const QString &inName,
                                                                            qint32 uniLoc,
                                                                            qint32 elementCount,
                                                                            QDemonRenderShaderDataTypes::Enum inConstantType,
                                                                            qint32 binding)
{
    switch (inConstantType) {
    case QDemonRenderShaderDataTypes::Integer:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<qint32>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::IntegerVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<qint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::IntegerVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<qint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::IntegerVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<qint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Boolean:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<bool>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::BooleanVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<bool_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::BooleanVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<bool_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::BooleanVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<bool_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Float:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<float>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Vec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QVector2D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Vec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QVector3D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Vec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QVector4D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::UnsignedInteger:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<quint32>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<quint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<quint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<quint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Matrix3x3:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QMatrix3x3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Matrix4x4:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QMatrix4x4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Texture2D:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderTexture2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Texture2DHandle:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderTexture2DHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Texture2DArray:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderTexture2DArrayPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::TextureCube:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderTextureCubePtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::TextureCubeHandle:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderTextureCubeHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::Image2D:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderImage2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataTypes::DataBuffer:
        return QDemonRef<QDemonRenderShaderConstantBase>(new QDemonRenderShaderConstant<QDemonRenderDataBufferPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    default:
        break;
    }
    Q_ASSERT(false);
    return nullptr;
}

template <typename TShaderBufferType, typename TBufferDataType>
static QDemonRef<QDemonRenderShaderBufferBase> shaderBufferFactory(QDemonRef<QDemonRenderContextImpl> context,
                                                                        const QString &inName,
                                                                        qint32 cbLoc,
                                                                        qint32 cbBinding,
                                                                        qint32 cbSize,
                    qint32 cbCount, QDemonRef<TBufferDataType> pBuffer)
{
    return QDemonRef<QDemonRenderShaderBufferBase>(new TShaderBufferType(context, inName, cbLoc, cbBinding, cbSize, cbCount, pBuffer));
}

template <typename TShaderBufferType, typename TBufferDataType>
static QSharedPointer<QDemonRenderShaderBufferBase> shaderBufferFactory(QSharedPointer<QDemonRenderContextImpl> context,
                                                                        const QString &inName,
                                                                        qint32 cbLoc,
                                                                        qint32 cbBinding,
                                                                        qint32 cbSize,
                    qint32 cbCount, QExplicitlySharedDataPointer<TBufferDataType> pBuffer)
{
    return QSharedPointer<QDemonRenderShaderBufferBase>(new TShaderBufferType(context, inName, cbLoc, cbBinding, cbSize, cbCount, pBuffer));
}

bool QDemonRenderShaderProgram::link()
{
    bool success = m_backend->linkProgram(m_programHandle, m_errorMessage);

    if (success) {
        char nameBuf[512];
        qint32 location, elementCount, binding;
        QDemonRenderShaderDataTypes::Enum type;

        qint32 constantCount = m_backend->getConstantCount(m_programHandle);

        QDEMON_FOREACH(idx, constantCount)
        {
            location = m_backend->getConstantInfoByID(m_programHandle, idx, 512, &elementCount,
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
                m_constants.insert(theName, shaderConstantFactory(m_backend, theName, location, elementCount, type, binding));
            }
        }

        // next query constant buffers info
        qint32 length, bufferSize, paramCount;
        qint32 constantBufferCount = m_backend->getConstantBufferCount(m_programHandle);
        QDEMON_FOREACH(idx, constantBufferCount)
        {
            location = m_backend->getConstantBufferInfoByID(
                        m_programHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);

                // find constant buffer in our DB
                QDemonRef<QDemonRenderConstantBuffer> cb = m_context->getConstantBuffer(theName);
                if (cb) {
                    cb->setupBuffer(this, location, bufferSize, paramCount);
                }

                m_shaderBuffers.insert(theName, shaderBufferFactory<QDemonRenderShaderConstantBuffer, QDemonRenderConstantBuffer>(
                                           m_context, theName, location, -1, bufferSize,
                                           paramCount, cb));
            }
        }

        // next query storage buffers
        qint32 storageBufferCount = m_backend->getStorageBufferCount(m_programHandle);
        QDEMON_FOREACH(idx, storageBufferCount)
        {
            location = m_backend->getStorageBufferInfoByID(
                        m_programHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);

                // find constant buffer in our DB
                QDemonRef<QDemonRenderStorageBuffer> sb = m_context->getStorageBuffer(theName);
                m_shaderBuffers.insert(theName, shaderBufferFactory<QDemonRenderShaderStorageBuffer, QDemonRenderStorageBuffer>(
                                           m_context, theName, location, -1, bufferSize,
                                           paramCount, sb));
            }
        }

        // next query atomic counter buffers
        qint32 atomicBufferCount = m_backend->getAtomicCounterBufferCount(m_programHandle);
        QDEMON_FOREACH(idx, atomicBufferCount)
        {
            location = m_backend->getAtomicCounterBufferInfoByID(
                        m_programHandle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

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
                QDemonRef<QDemonRenderAtomicCounterBuffer> acb =
                        m_context->getAtomicCounterBufferByParam(theName);
                if (acb) {
                    m_shaderBuffers.insert(acb->getBufferName(), shaderBufferFactory<QDemonRenderShaderAtomicCounterBuffer,
                                           QDemonRenderAtomicCounterBuffer>(m_context, acb->getBufferName(),
                                                                            location, -1, bufferSize, paramCount, acb));
                }
            }
        }
    }

    return success;
}

void QDemonRenderShaderProgram::getErrorMessage(qint32 *messageLength, const char *errorMessage)
{
    *messageLength = m_errorMessage.size();
    errorMessage = qPrintable(m_errorMessage);
}

const char *QDemonRenderShaderProgram::getErrorMessage() { return qPrintable(m_errorMessage); }

QDemonRef<QDemonRenderShaderConstantBase> QDemonRenderShaderProgram::getShaderConstant(const char *constantName)
{
    TShaderConstantMap::iterator theIter =
            m_constants.find(QString::fromLocal8Bit(constantName));

    if (theIter != m_constants.end())
        return theIter.value();

    return nullptr;
}

QDemonRef<QDemonRenderShaderBufferBase> QDemonRenderShaderProgram::getShaderBuffer(const char *bufferName)
{
    TShaderBufferMap::iterator theIter =
            m_shaderBuffers.find(QString::fromLocal8Bit(bufferName));

    if (theIter != m_shaderBuffers.end()) {
        return theIter.value();
    }

    return nullptr;
}

QDemonRef<QDemonRenderContextImpl> QDemonRenderShaderProgram::getRenderContext() { return m_context; }

template <typename TDataType>
void setConstantValueOfType(const QDemonRenderShaderProgram *program,
                            QDemonRenderShaderConstantBase *inConstantBase,
                            const TDataType &inValue, const qint32 inCount)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_elementCount >= inCount);

    if (inConstantBase->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant = static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().applyConstant(program,
                                                         inConstant->m_backend,
                                                         inConstant->m_location,
                                                         inCount,
                                                         inConstant->m_type,
                                                         inValue,
                                                         inConstant->m_value);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void setSamplerConstantValueOfType(const QDemonRenderShaderProgram *program,
                                   QDemonRenderShaderConstantBase *inConstantBase,
                                   const TDataType &inValue, const qint32 inCount)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_elementCount >= inCount);

    if (inConstantBase->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant = static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().applyConstant(program,
                                                         inConstant->m_backend,
                                                         inConstant->m_location,
                                                         inCount,
                                                         inConstant->m_type,
                                                         inValue,
                                                         inConstant->m_value,
                                                         inConstant->m_binding);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void setMatrixConstantValueOfType(const QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const TDataType &inValue, const qint32 inCount,
                                  bool inTranspose)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_elementCount >= inCount);

    if (inConstantBase->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant = static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().applyConstant(program,
                                                         inConstant->m_backend,
                                                         inConstant->m_location,
                                                         inCount,
                                                         inConstant->m_type,
                                                         inValue,
                                                         inConstant->m_value,
                                                         inTranspose);
    } else {
        Q_ASSERT(false);
    }
}

template <typename TDataType>
void setMatrixConstantValueOfType(const QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const QDemonConstDataRef<TDataType> inValue,
                                  const qint32 /*inCount*/, bool inTranspose)
{
    if (inConstantBase == nullptr) {
        Q_ASSERT(false);
        return;
    }

    Q_ASSERT(inConstantBase->m_elementCount >= (qint32)inValue.size());

    if (inConstantBase->getShaderConstantType() == QDemonDataTypeToShaderDataTypeMap<TDataType>::getType()) {
        QDemonRenderShaderConstant<TDataType> *inConstant = static_cast<QDemonRenderShaderConstant<TDataType> *>(inConstantBase);
        ShaderConstantApplier<TDataType>().applyConstant(program,
                                                         inConstant->m_backend,
                                                         inConstant->m_location,
                                                         inValue.size(),
                                                         inConstant->m_type,
                                                         inValue,
                                                         inConstant->m_value,
                                                         inTranspose);
    } else {
        Q_ASSERT(false);
    }
}

void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 qint32 inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const qint32_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 bool inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const bool_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const float &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector2D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector3D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QVector4D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const quint32_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix3x3 &inValue, const qint32 inCount,
                                                 bool inTranspose)
{
    setMatrixConstantValueOfType(this, inConstant, inValue, inCount, inTranspose);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix4x4 &inValue, const qint32 inCount,
                                                 bool inTranspose)
{
    setMatrixConstantValueOfType(this, inConstant, inValue, inCount, inTranspose);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QDemonConstDataRef<QMatrix4x4> inValue,
                                                 const qint32 inCount)
{
    setMatrixConstantValueOfType(this, inConstant, inValue, inCount, false);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2D *inValue, const qint32 inCount)
{
    Q_UNUSED(inCount)
    setConstantValueOfType(this, inConstant, inValue, 1);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2D **inValue,
                                                 const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2DArray *inValue,
                                                 const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTextureCube *inValue, const qint32 inCount)
{
    Q_UNUSED(inCount)
    setConstantValueOfType(this, inConstant, inValue, 1);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTextureCube **inValue,
                                                 const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderImage2D *inValue, const qint32 inCount)
{
    setSamplerConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *, QDemonRenderDataBuffer *,
                                                 const qint32)
{
    // this is merely a dummy right now
}

void QDemonRenderShaderProgram::bindComputeInput(QDemonRenderDataBuffer *inBuffer, quint32 inIndex)
{
    QDemonRenderBackend::QDemonRenderBackendBufferObject obj(nullptr);
    if (inBuffer)
        obj = inBuffer->getBuffertHandle();
    m_backend->programSetStorageBuffer(inIndex, obj);
}

namespace {
void writeErrorMessage(const char *tag, const char *message)
{
    const QString messageData = QString::fromLocal8Bit(message);
    const auto lines = messageData.splitRef('\n');
    for (const auto &line : lines )
        qCCritical(INVALID_OPERATION, "%s: %s", tag, line.toLocal8Bit().constData());
}
}

QDemonOption<QDemonRenderVertexShader *> QDemonRenderShaderProgram::createVertexShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                       QDemonConstDataRef<qint8> vertexShaderSource,
                                                                                       bool binaryProgram)
{
    if (vertexShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderVertexShader(context, vertexShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderFragmentShader *> QDemonRenderShaderProgram::createFragmentShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                           QDemonConstDataRef<qint8> fragmentShaderSource,
                                                                                           bool binaryProgram)
{
    if (fragmentShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderFragmentShader(context, fragmentShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderTessControlShader *>
QDemonRenderShaderProgram::createTessControlShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                   QDemonConstDataRef<qint8> tessControlShaderSource,
                                                   bool binaryProgram)
{
    if (tessControlShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderTessControlShader(context, tessControlShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderTessEvaluationShader *>
QDemonRenderShaderProgram::createTessEvaluationShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                      QDemonConstDataRef<qint8> tessControlShaderSource,
                                                      bool binaryProgram)
{
    if (tessControlShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderTessEvaluationShader(context, tessControlShaderSource, binaryProgram);
}

QDemonOption<QDemonRenderGeometryShader *> QDemonRenderShaderProgram::createGeometryShader(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                                           QDemonConstDataRef<qint8> geometryShaderSource,
                                                                                           bool binaryProgram)
{
    if (geometryShaderSource.size() == 0)
        return QDemonEmpty();

    return new QDemonRenderGeometryShader(context, geometryShaderSource, binaryProgram);
}

QDemonRenderVertFragCompilationResult QDemonRenderShaderProgram::create(const QDemonRef<QDemonRenderContextImpl> &context,
                                                                        const char *programName,
                                                                        QDemonConstDataRef<qint8> vertShaderSource,
                                                                        QDemonConstDataRef<qint8> fragShaderSource,
                                                                        QDemonConstDataRef<qint8> tessControlShaderSource,
                                                                        QDemonConstDataRef<qint8> tessEvaluationShaderSource,
                                                                        QDemonConstDataRef<qint8> geometryShaderSource,
                                                                        bool separateProgram,
                                                                        QDemonRenderShaderProgramBinaryType::Enum type,
                                                                        bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result;
    QDemonRef<QDemonRenderShaderProgram> pProgram = nullptr;
    bool bProgramIsValid = false;

    result.m_shaderName = programName;

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
    QDemonOption<QDemonRenderVertexShader *> vtxShader = createVertexShader(context, vertShaderSource, binaryProgram);
    QDemonOption<QDemonRenderFragmentShader *> fragShader = createFragmentShader(context, fragShaderSource, binaryProgram);
    QDemonOption<QDemonRenderTessControlShader *> tcShader = createTessControlShader(context, tessControlShaderSource, binaryProgram);
    QDemonOption<QDemonRenderTessEvaluationShader *> teShader = createTessEvaluationShader(context, tessEvaluationShaderSource, binaryProgram);
    QDemonOption<QDemonRenderGeometryShader *> geShader = createGeometryShader(context, geometryShaderSource, binaryProgram);

    bool vertexValid = (vtxShader.hasValue()) ? vtxShader.getValue()->isValid() : true;
    bool fragValid = (fragShader.hasValue()) ? fragShader.getValue()->isValid() : true;
    bool tcValid = (tcShader.hasValue()) ? tcShader.getValue()->isValid() : true;
    bool teValid = (teShader.hasValue()) ? teShader.getValue()->isValid() : true;
    bool geValid = (geShader.hasValue()) ? geShader.getValue()->isValid() : true;

    if (vertexValid && fragValid && tcValid && teValid && geValid) {
        // shaders were succesfuly created
        pProgram.reset(new QDemonRenderShaderProgram(context, programName, separateProgram));

        if (pProgram) {
            // attach programs
            if (vtxShader.hasValue() && vtxShader.getValue()->isValid())
                pProgram->attach(vtxShader.getValue());
            if (fragShader.hasValue() && fragShader.getValue()->isValid())
                pProgram->attach(fragShader.getValue());
            if (tcShader.hasValue() && tcShader.getValue()->isValid())
                pProgram->attach(tcShader.getValue());
            if (teShader.hasValue() && teShader.getValue()->isValid())
                pProgram->attach(teShader.getValue());
            if (geShader.hasValue() && geShader.getValue()->isValid())
                pProgram->attach(geShader.getValue());

            // link program
            bProgramIsValid = pProgram->link();
        }
    }

    // if anything went wrong print out
    if (!vertexValid || !fragValid || !tcValid || !teValid || !geValid || !bProgramIsValid) {

        if (!vertexValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate vertex shader!!");
            qCCritical(INTERNAL_ERROR, "Vertex source:\n%s",
                       nonNull((const char *)vertShaderSource.begin()));
            writeErrorMessage("Vertex compilation output:",
                              vtxShader.getValue()->getErrorMessage());
        }

        if (!fragValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate fragment shader!!");
            qCCritical(INTERNAL_ERROR, "Fragment source:\n%s",
                       nonNull((const char *)fragShaderSource.begin()));
            writeErrorMessage("Fragment compilation output:",
                              fragShader.getValue()->getErrorMessage());
        }

        if (!tcValid) {
            qCCritical(INTERNAL_ERROR,
                       "Failed to generate tessellation control shader!!");
            qCCritical(INTERNAL_ERROR, "Tessellation control source:\n%s",
                       nonNull((const char *)tessControlShaderSource.begin()));
            writeErrorMessage("Tessellation control compilation output:",
                              tcShader.getValue()->getErrorMessage());
        }

        if (!teValid) {
            qCCritical(INTERNAL_ERROR,
                       "Failed to generate tessellation evaluation shader!!");
            qCCritical(INTERNAL_ERROR, "Tessellation evaluation source:\n%s",
                       nonNull((const char *)tessEvaluationShaderSource.begin()));
            writeErrorMessage("Tessellation evaluation compilation output:",
                              teShader.getValue()->getErrorMessage());
        }

        if (!geValid) {
            qCCritical(INTERNAL_ERROR, "Failed to generate geometry shader!!");
            qCCritical(INTERNAL_ERROR, "Geometry source:\n%s",
                       nonNull((const char *)geometryShaderSource.begin()));
            writeErrorMessage("Geometry compilation output:",
                              geShader.getValue()->getErrorMessage());
        }

        if (!bProgramIsValid && pProgram) {
            qCCritical(INTERNAL_ERROR, "Failed to link program!!");
            writeErrorMessage("Program link output:", pProgram->getErrorMessage());

            // delete program
            pProgram.clear();
        }
    }

    // clean up
    if (vtxShader.hasValue()) {
        if (bProgramIsValid && vtxShader.getValue()->isValid())
            pProgram->detach(vtxShader.getValue());
        delete vtxShader.getValue();
    }
    if (fragShader.hasValue()) {
        if (bProgramIsValid && fragShader.getValue()->isValid())
            pProgram->detach(fragShader.getValue());
        delete fragShader.getValue();
    }
    if (tcShader.hasValue()) {
        if (bProgramIsValid && tcShader.getValue()->isValid())
            pProgram->detach(tcShader.getValue());
        delete tcShader.getValue();
    }
    if (teShader.hasValue()) {
        if (bProgramIsValid && teShader.getValue()->isValid())
            pProgram->detach(teShader.getValue());
        delete teShader.getValue();
    }
    if (geShader.hasValue()) {
        if (bProgramIsValid && geShader.getValue()->isValid())
            pProgram->detach(geShader.getValue());
        delete geShader.getValue();
    }

    // set program
    result.m_shader = pProgram;

    return result;
}

QDemonRenderVertFragCompilationResult
QDemonRenderShaderProgram::createCompute(const QDemonRef<QDemonRenderContextImpl> &context,
                                         const char *programName,
                                         QDemonConstDataRef<qint8> computeShaderSource)
{
    QDemonRenderVertFragCompilationResult result;
    QDemonRef<QDemonRenderShaderProgram> pProgram;
    bool bProgramIsValid = true;

    result.m_shaderName = programName;

    // check source
    if (computeShaderSource.size() == 0) {
        qCCritical(INVALID_PARAMETER, "compute source has 0 length");
        Q_ASSERT(false);
        return result;
    }

    QDemonRenderComputeShader computeShader(context, computeShaderSource, false);

    if (computeShader.isValid()) {
        // shaders were succesfuly created
        pProgram.reset(new QDemonRenderShaderProgram(context, programName, false));

        if (pProgram) {
            // attach programs
            pProgram->attach(&computeShader);

            // link program
            bProgramIsValid = pProgram->link();

            // set program type
            pProgram->setProgramType(ProgramType::Compute);
        }
    }

    // if anything went wrong print out
    if (!computeShader.isValid() || !bProgramIsValid) {

        if (!computeShader.isValid()) {
            qCCritical(INTERNAL_ERROR, "Failed to generate compute shader!!");
            qCCritical(INTERNAL_ERROR, "Vertex source:\n%s",
                       nonNull((const char *)computeShaderSource.begin()));
            writeErrorMessage("Compute shader compilation output:",
                              computeShader.getErrorMessage());
        }
    }

    // set program
    result.m_shader = pProgram;

    return result;
}

QDemonRenderVertFragCompilationResult::QDemonRenderVertFragCompilationResult()
    : m_shaderName("")
    , m_shader(nullptr)
{
}

QDemonRenderVertFragCompilationResult::~QDemonRenderVertFragCompilationResult()
{

}

QDemonRenderVertFragCompilationResult::QDemonRenderVertFragCompilationResult(const QDemonRenderVertFragCompilationResult &other)
    : m_shaderName(other.m_shaderName),
      m_shader(other.m_shader)
{

}

QDemonRenderVertFragCompilationResult &QDemonRenderVertFragCompilationResult::operator=(const QDemonRenderVertFragCompilationResult &other)
{
    m_shaderName = other.m_shaderName;
    m_shader = other.m_shader;
}

QT_END_NAMESPACE
