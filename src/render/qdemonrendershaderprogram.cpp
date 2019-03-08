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
#include <QtDemonRender/qdemonrenderimagetexture.h>
#include <QtDemon/qdemonutils.h>

#include <limits>

QT_BEGIN_NAMESPACE

template<typename TDataType>
struct ShaderConstantApplier
{
    bool force_compile_error;
};

template<>
struct ShaderConstantApplier<qint32>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const qint32 &inValue,
                       qint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<qint32_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const qint32_2 &inValue,
                       qint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<qint32_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const qint32_3 &inValue,
                       qint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<qint32_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const qint32_4 &inValue,
                       qint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<bool>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const bool inValue,
                       bool &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<bool_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const bool_2 &inValue,
                       bool_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<bool_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const bool_3 &inValue,
                       bool_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<bool_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const bool_4 &inValue,
                       bool_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<float>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const float &inValue,
                       float &oldValue)
    {
        if (count > 1 || !(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<QVector2D>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const QVector2D &inValue,
                       QVector2D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<QVector3D>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const QVector3D &inValue,
                       QVector3D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<QVector4D>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const QVector4D &inValue,
                       QVector4D &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<quint32>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const quint32 &inValue,
                       quint32 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<quint32_2>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const quint32_2 &inValue,
                       quint32_2 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<quint32_3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const quint32_3 &inValue,
                       quint32_3 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<quint32_4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const quint32_4 &inValue,
                       quint32_4 &oldValue)
    {
        if (!(inValue == oldValue)) {
            backend->setConstantValue(program->handle(), location, type, count, &inValue.x);
            oldValue = inValue;
        }
    }
};

template<>
struct ShaderConstantApplier<QMatrix3x3>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const QMatrix3x3 inValue,
                       QMatrix3x3 &,
                       bool inTranspose)
    {
        backend->setConstantValue(program->handle(), location, type, count, inValue.constData(), inTranspose);
    }
};

template<>
struct ShaderConstantApplier<QMatrix4x4>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       const QMatrix4x4 inValue,
                       QMatrix4x4 &,
                       bool inTranspose)
    {
        backend->setConstantValue(program->handle(), location, type, count, inValue.constData(), inTranspose);
    }

    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       QDemonConstDataRef<QMatrix4x4> inValue,
                       QMatrix4x4 &,
                       bool inTranspose)
    {
        backend->setConstantValue(program->handle(),
                                  location,
                                  type,
                                  count,
                                  reinterpret_cast<const GLfloat *>(inValue.begin()),
                                  inTranspose);
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderTexture2DPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       QDemonRenderTexture2DPtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2D *texObj = reinterpret_cast<QDemonRenderTexture2D *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->textureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->handle(), location, type, count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderTexture2DHandle>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
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
                    texUnit = texObj->textureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->setConstantValue(program->handle(),
                                          location,
                                          QDemonRenderShaderDataType::Texture2D,
                                          count,
                                          oldValue.data());
        }
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderTexture2DArrayPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       QDemonRenderTexture2DArrayPtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTexture2DArray *texObj = reinterpret_cast<QDemonRenderTexture2DArray *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->textureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->handle(), location, type, count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderTextureCubePtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       QDemonRenderTextureCubePtr inValue,
                       quint32 &oldValue)
    {
        if (inValue) {
            QDemonRenderTextureCube *texObj = reinterpret_cast<QDemonRenderTextureCube *>(inValue);
            texObj->bind();
            quint32 texUnit = texObj->textureUnit();
            if (texUnit != oldValue) {
                backend->setConstantValue(program->handle(), location, type, count, &texUnit);
                oldValue = texUnit;
            }
        }
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderTextureCubeHandle>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
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
                    texUnit = texObj->textureUnit();
                }
                if (texUnit != oldValue[i]) {
                    update = true;
                    oldValue[i] = texUnit;
                }
            }
            if (update)
                backend->setConstantValue(program->handle(),
                                          location,
                                          QDemonRenderShaderDataType::TextureCube,
                                          count,
                                          oldValue.data());
        }
    }
};

template<>
struct ShaderConstantApplier<QDemonRenderImage2DPtr>
{
    void applyConstant(const QDemonRenderShaderProgram *program,
                       const QDemonRef<QDemonRenderBackend> &backend,
                       qint32 location,
                       qint32 count,
                       QDemonRenderShaderDataType type,
                       QDemonRenderImage2DPtr image,
                       quint32 &oldValue,
                       qint32 binding)
    {
        if (image) {
            image->bind(binding);
            quint32 texUnit = image->textureUnit();
            if (texUnit != oldValue) {
                // on ES we need a explicit binding value
                Q_ASSERT(backend->getRenderContextType() != QDemonRenderContextType::GLES3PLUS || binding != -1);
                // this is not allowed on ES 3+ for image types
                if (backend->getRenderContextType() != QDemonRenderContextType::GLES3PLUS)
                    backend->setConstantValue(program->handle(), location, type, count, &texUnit);

                oldValue = texUnit;
            }
        }
    }
};

QDemonRenderShaderProgram::QDemonRenderShaderProgram(const QDemonRef<QDemonRenderContext> &context, const char *programName, bool separableProgram)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_programName(programName)
    , m_handle(nullptr)
    , m_programType(ProgramType::Graphics)
{
    m_handle = m_backend->createShaderProgram(separableProgram);

    Q_ASSERT(m_handle);
}

QDemonRenderShaderProgram::~QDemonRenderShaderProgram()
{
    m_context->shaderDestroyed(this);

    if (m_handle)
        m_backend->releaseShaderProgram(m_handle);

    m_constants.clear();
    m_shaderBuffers.clear();

    m_handle = nullptr;
}

template<typename TShaderObject>
void QDemonRenderShaderProgram::attach(TShaderObject *pShader)
{
    m_backend->attachShader(m_handle, pShader);
}

template<typename TShaderObject>
void QDemonRenderShaderProgram::detach(TShaderObject *pShader)
{
    m_backend->detachShader(m_handle, pShader);
}

static QDemonRef<QDemonRenderShaderConstantBase> shaderConstantFactory(const QDemonRef<QDemonRenderBackend> &backend,
                                                                       const QString &inName,
                                                                       qint32 uniLoc,
                                                                       qint32 elementCount,
                                                                       QDemonRenderShaderDataType inConstantType,
                                                                       qint32 binding)
{
    switch (inConstantType) {
    case QDemonRenderShaderDataType::Integer:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<qint32>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::IntegerVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<qint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::IntegerVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<qint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::IntegerVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<qint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Boolean:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<bool>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::BooleanVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<bool_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::BooleanVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<bool_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::BooleanVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<bool_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Float:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<float>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Vec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QVector2D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Vec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QVector3D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Vec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QVector4D>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::UnsignedInteger:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<quint32>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::UnsignedIntegerVec2:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<quint32_2>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::UnsignedIntegerVec3:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<quint32_3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::UnsignedIntegerVec4:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<quint32_4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Matrix3x3:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QMatrix3x3>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Matrix4x4:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QMatrix4x4>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Texture2D:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderTexture2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Texture2DHandle:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderTexture2DHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Texture2DArray:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderTexture2DArrayPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::TextureCube:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderTextureCubePtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::TextureCubeHandle:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderTextureCubeHandle>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::Image2D:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderImage2DPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    case QDemonRenderShaderDataType::DataBuffer:
        return QDemonRef<QDemonRenderShaderConstantBase>(
                new QDemonRenderShaderConstant<QDemonRenderDataBufferPtr>(backend, inName, uniLoc, elementCount, inConstantType, binding));
    default:
        break;
    }
    Q_ASSERT(false);
    return nullptr;
}

template<typename TShaderBufferType, typename TBufferDataType>
static QDemonRef<QDemonRenderShaderBufferBase> shaderBufferFactory(QDemonRef<QDemonRenderContext> context,
                                                                   const QByteArray &inName,
                                                                   qint32 cbLoc,
                                                                   qint32 cbBinding,
                                                                   qint32 cbSize,
                                                                   qint32 cbCount,
                                                                   QDemonRef<TBufferDataType> pBuffer)
{
    return QDemonRef<QDemonRenderShaderBufferBase>(new TShaderBufferType(context, inName, cbLoc, cbBinding, cbSize, cbCount, pBuffer));
}

bool QDemonRenderShaderProgram::link()
{
    bool success = m_backend->linkProgram(m_handle, m_errorMessage);

    if (success) {
        char nameBuf[512];
        qint32 location, elementCount, binding;
        QDemonRenderShaderDataType type;

        qint32 constantCount = m_backend->getConstantCount(m_handle);

        for (int idx = 0; idx != constantCount; ++idx) {
            location = m_backend->getConstantInfoByID(m_handle, idx, 512, &elementCount, &type, &binding, nameBuf);

            // sampler arrays have different type
            if (type == QDemonRenderShaderDataType::Texture2D && elementCount > 1) {
                type = QDemonRenderShaderDataType::Texture2DHandle;
            } else if (type == QDemonRenderShaderDataType::TextureCube && elementCount > 1) {
                type = QDemonRenderShaderDataType::TextureCubeHandle;
            }
            if (location != -1) {
                const QString theName = QString::fromLocal8Bit(nameBuf);
                m_constants.insert(theName, shaderConstantFactory(m_backend, theName, location, elementCount, type, binding));
            }
        }

        // next query constant buffers info
        qint32 length, bufferSize, paramCount;
        qint32 constantBufferCount = m_backend->getConstantBufferCount(m_handle);
        for (int idx = 0; idx != constantBufferCount; ++idx) {
            location = m_backend->getConstantBufferInfoByID(m_handle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                // find constant buffer in our DB
                QDemonRef<QDemonRenderConstantBuffer> cb = m_context->getConstantBuffer(nameBuf);
                if (cb) {
                    cb->setupBuffer(this, location, bufferSize, paramCount);
                }

                m_shaderBuffers.insert(nameBuf,
                                       shaderBufferFactory<QDemonRenderShaderConstantBuffer,
                                                           QDemonRenderConstantBuffer>(m_context, nameBuf, location, -1, bufferSize, paramCount, cb));
            }
        }

        // next query storage buffers
        qint32 storageBufferCount = m_backend->getStorageBufferCount(m_handle);
        for (int idx = 0; idx != storageBufferCount; ++idx) {
            location = m_backend->getStorageBufferInfoByID(m_handle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                // find constant buffer in our DB
                QDemonRef<QDemonRenderStorageBuffer> sb = m_context->getStorageBuffer(nameBuf);
                m_shaderBuffers.insert(nameBuf,
                                       shaderBufferFactory<QDemonRenderShaderStorageBuffer,
                                                           QDemonRenderStorageBuffer>(m_context, nameBuf, location, -1, bufferSize, paramCount, sb));
            }
        }

        // next query atomic counter buffers
        qint32 atomicBufferCount = m_backend->getAtomicCounterBufferCount(m_handle);
        for (int idx = 0; idx != atomicBufferCount; ++idx) {
            location = m_backend->getAtomicCounterBufferInfoByID(m_handle, idx, 512, &paramCount, &bufferSize, &length, nameBuf);

            if (location != -1) {
                // find atomic counter buffer in our DB
                // The buffer itself is not used in the program itself.
                // Instead uniform variables are used but the interface to set the value is like
                // for buffers.
                // This is a bit insane but that is how it is.
                // The theName variable contains the uniform name associated with an atomic
                // counter buffer.
                // We get the actual buffer name by searching for this uniform name
                // See NVRenderTestAtomicCounterBuffer.cpp how the setup works
                QDemonRef<QDemonRenderAtomicCounterBuffer> acb = m_context->getAtomicCounterBufferByParam(nameBuf);
                if (acb) {
                    m_shaderBuffers.insert(acb->getBufferName(),
                                           shaderBufferFactory<QDemonRenderShaderAtomicCounterBuffer,
                                                               QDemonRenderAtomicCounterBuffer>(m_context,
                                                                                                acb->getBufferName(),
                                                                                                location,
                                                                                                -1,
                                                                                                bufferSize,
                                                                                                paramCount,
                                                                                                acb));
                }
            }
        }
    }

    return success;
}

QByteArray QDemonRenderShaderProgram::errorMessage()
{
    return m_errorMessage;
}

QDemonRef<QDemonRenderShaderConstantBase> QDemonRenderShaderProgram::shaderConstant(const char *constantName)
{
    TShaderConstantMap::iterator theIter = m_constants.find(QString::fromLocal8Bit(constantName));

    if (theIter != m_constants.end())
        return theIter.value();

    return nullptr;
}

QDemonRef<QDemonRenderShaderBufferBase> QDemonRenderShaderProgram::shaderBuffer(const char *bufferName)
{
    TShaderBufferMap::iterator theIter = m_shaderBuffers.find(bufferName);

    if (theIter != m_shaderBuffers.end()) {
        return theIter.value();
    }

    return nullptr;
}

QDemonRef<QDemonRenderContext> QDemonRenderShaderProgram::renderContext()
{
    return m_context;
}

template<typename TDataType>
void setConstantValueOfType(const QDemonRenderShaderProgram *program,
                            QDemonRenderShaderConstantBase *inConstantBase,
                            const TDataType &inValue,
                            const qint32 inCount)
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

template<typename TDataType>
void setSamplerConstantValueOfType(const QDemonRenderShaderProgram *program,
                                   QDemonRenderShaderConstantBase *inConstantBase,
                                   const TDataType &inValue,
                                   const qint32 inCount)
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

template<typename TDataType>
void setMatrixConstantValueOfType(const QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const TDataType &inValue,
                                  const qint32 inCount,
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

template<typename TDataType>
void setMatrixConstantValueOfType(const QDemonRenderShaderProgram *program,
                                  QDemonRenderShaderConstantBase *inConstantBase,
                                  const QDemonConstDataRef<TDataType> inValue,
                                  const qint32 /*inCount*/,
                                  bool inTranspose)
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

void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, qint32 inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const qint32_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, bool inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const bool_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const float &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector2D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector3D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const QVector4D &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_2 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_3 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, const quint32_4 &inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix3x3 &inValue,
                                                 const qint32 inCount,
                                                 bool inTranspose)
{
    setMatrixConstantValueOfType(this, inConstant, inValue, inCount, inTranspose);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 const QMatrix4x4 &inValue,
                                                 const qint32 inCount,
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
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D *inValue, const qint32 inCount)
{
    Q_UNUSED(inCount)
    setConstantValueOfType(this, inConstant, inValue, 1);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTexture2D **inValue, const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant,
                                                 QDemonRenderTexture2DArray *inValue,
                                                 const qint32 inCount)
{
    setConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderTextureCube *inValue, const qint32 inCount)
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
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *inConstant, QDemonRenderImage2D *inValue, const qint32 inCount)
{
    setSamplerConstantValueOfType(this, inConstant, inValue, inCount);
}
void QDemonRenderShaderProgram::setConstantValue(QDemonRenderShaderConstantBase *, QDemonRenderDataBuffer *, const qint32)
{
    // this is merely a dummy right now
}

void QDemonRenderShaderProgram::bindComputeInput(QDemonRenderDataBuffer *inBuffer, quint32 inIndex)
{
    QDemonRenderBackend::QDemonRenderBackendBufferObject obj(nullptr);
    if (inBuffer)
        obj = inBuffer->handle();
    m_backend->programSetStorageBuffer(inIndex, obj);
}

namespace {
void writeErrorMessage(const char *tag, const QByteArray &message)
{
    const QString messageData = QString::fromLocal8Bit(message);
    const auto lines = messageData.splitRef('\n');
    for (const auto &line : lines)
        qCCritical(INVALID_OPERATION, "%s: %s", tag, line.toLocal8Bit().constData());
}
}

QDemonRenderVertFragCompilationResult QDemonRenderShaderProgram::create(const QDemonRef<QDemonRenderContext> &context,
                                                                        const char *programName,
                                                                        QDemonConstDataRef<qint8> vertShaderSource,
                                                                        QDemonConstDataRef<qint8> fragShaderSource,
                                                                        QDemonConstDataRef<qint8> tessControlShaderSource,
                                                                        QDemonConstDataRef<qint8> tessEvaluationShaderSource,
                                                                        QDemonConstDataRef<qint8> geometryShaderSource,
                                                                        bool separateProgram,
                                                                        QDemonRenderShaderProgramBinaryType type,
                                                                        bool binaryProgram)
{
    QDemonRenderVertFragCompilationResult result;
    result.m_shaderName = programName;

    // our minimum requirement is a vertex and a fragment shader or geometry shader
    // if we should treat it as a separate program we don't care
    if (!separateProgram && (vertShaderSource.size() == 0 || (fragShaderSource.size() == 0 && geometryShaderSource.size() == 0))) {
        qCCritical(INVALID_PARAMETER, "Vertex or fragment (geometry) source have 0 length");
        Q_ASSERT(false);
        return result;
    }

    if (binaryProgram && type != QDemonRenderShaderProgramBinaryType::NVBinary) {
        qCCritical(INVALID_PARAMETER, "Unrecoginzed binary format");
        Q_ASSERT(false);
        return result;
    }

    QDemonRef<QDemonRenderBackend> backend = context->getBackend();

    // first create and compile shaders
    QDemonRenderBackend::QDemonRenderBackendVertexShaderObject vtxShader = nullptr;
    if (vertShaderSource.size()) {
        QByteArray errorMessage;
        vtxShader = backend->createVertexShader(vertShaderSource, errorMessage, binaryProgram);
        if (!vtxShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate vertex shader!!");
            qCCritical(INTERNAL_ERROR, "Vertex source:\n%s", nonNull((const char *)vertShaderSource.begin()));
            writeErrorMessage("Vertex compilation output:", errorMessage);
            return result;
        }
    }
    QDemonRenderBackend::QDemonRenderBackendFragmentShaderObject fragShader = nullptr;
    if (fragShaderSource.size()) {
        QByteArray errorMessage;
        fragShader = backend->createFragmentShader(fragShaderSource, errorMessage, binaryProgram);
        if (!fragShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate fragment shader!!");
            qCCritical(INTERNAL_ERROR, "Fragment source:\n%s", nonNull((const char *)fragShaderSource.begin()));
            writeErrorMessage("Fragment compilation output:", errorMessage);
            return result;
        }
    }
    QDemonRenderBackend::QDemonRenderBackendTessControlShaderObject tcShader = nullptr;
    if (tessControlShaderSource.size()) {
        QByteArray errorMessage;
        tcShader = backend->createTessControlShader(tessControlShaderSource, errorMessage, binaryProgram);
        if (!tcShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate tessellation control shader!!");
            qCCritical(INTERNAL_ERROR, "Tessellation control source:\n%s", nonNull((const char *)tessControlShaderSource.begin()));
            writeErrorMessage("Tessellation control compilation output:", errorMessage);
            return result;
        }
    }
    QDemonRenderBackend::QDemonRenderBackendTessEvaluationShaderObject teShader = nullptr;
    if (tessEvaluationShaderSource.size()) {
        QByteArray errorMessage;
        teShader = backend->createTessEvaluationShader(tessEvaluationShaderSource, errorMessage, binaryProgram);
        if (!teShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate tessellation evaluation shader!!");
            qCCritical(INTERNAL_ERROR,
                       "Tessellation evaluation source:\n%s",
                       nonNull((const char *)tessEvaluationShaderSource.begin()));
            writeErrorMessage("Tessellation evaluation compilation output:", errorMessage);
            return result;
        }
    }
    QDemonRenderBackend::QDemonRenderBackendGeometryShaderObject geShader = nullptr;
    if (geometryShaderSource.size()) {
        QByteArray errorMessage;
        geShader = backend->createGeometryShader(geometryShaderSource, errorMessage, binaryProgram);
        if (!geShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate geometry shader!!");
            qCCritical(INTERNAL_ERROR, "Geometry source:\n%s", nonNull((const char *)geometryShaderSource.begin()));
            writeErrorMessage("Geometry compilation output:", errorMessage);
            return result;
        }
    }

    // shaders were succesfully created
    result.m_shader = new QDemonRenderShaderProgram(context, programName, separateProgram);

    // attach programs
    if (vtxShader)
        result.m_shader->attach(vtxShader);
    if (fragShader)
        result.m_shader->attach(fragShader);
    if (tcShader)
        result.m_shader->attach(tcShader);
    if (teShader)
        result.m_shader->attach(teShader);
    if (geShader)
        result.m_shader->attach(geShader);

    // link program
    if (!result.m_shader->link()) {
        qCCritical(INTERNAL_ERROR, "Failed to link program!!");
        writeErrorMessage("Program link output:", result.m_shader->errorMessage());

        // delete program
        result.m_shader = nullptr;
    } else {
        // clean up
        if (vtxShader)
            result.m_shader->detach(vtxShader);
        if (fragShader)
            result.m_shader->detach(fragShader);
        if (tcShader)
            result.m_shader->detach(tcShader);
        if (teShader)
            result.m_shader->detach(teShader);
        if (geShader)
            result.m_shader->detach(geShader);
    }

    backend->releaseVertexShader(vtxShader);
    backend->releaseFragmentShader(fragShader);
    backend->releaseTessControlShader(tcShader);
    backend->releaseTessEvaluationShader(teShader);
    backend->releaseGeometryShader(geShader);

    return result;
}

QDemonRenderVertFragCompilationResult QDemonRenderShaderProgram::createCompute(const QDemonRef<QDemonRenderContext> &context,
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

    auto backend = context->getBackend();
    QByteArray errorMessage;
    QDemonRenderBackend::QDemonRenderBackendComputeShaderObject computeShader =
        backend->createComputeShader(computeShaderSource, errorMessage, false);

    if (computeShader) {
        // shaders were succesfuly created
        pProgram = new QDemonRenderShaderProgram(context, programName, false);

        if (pProgram) {
            // attach programs
            pProgram->attach(computeShader);

            // link program
            bProgramIsValid = pProgram->link();

            // set program type
            pProgram->m_programType = ProgramType::Compute;
        }
    }

    // if anything went wrong print out
    if (!computeShader || !bProgramIsValid) {
        if (!computeShader) {
            qCCritical(INTERNAL_ERROR, "Failed to generate compute shader!!");
            qCCritical(INTERNAL_ERROR, "Shader source:\n%s", nonNull((const char *)computeShaderSource.begin()));
            writeErrorMessage("Compute shader compilation output:", errorMessage);
        }
    }

    // set program
    result.m_shader = pProgram;

    return result;
}

QDemonRenderVertFragCompilationResult::QDemonRenderVertFragCompilationResult() = default;

QDemonRenderVertFragCompilationResult::~QDemonRenderVertFragCompilationResult() = default;

QDemonRenderVertFragCompilationResult::QDemonRenderVertFragCompilationResult(const QDemonRenderVertFragCompilationResult &other) = default;

QDemonRenderVertFragCompilationResult &QDemonRenderVertFragCompilationResult::operator=(const QDemonRenderVertFragCompilationResult &other) = default;

QT_END_NAMESPACE
