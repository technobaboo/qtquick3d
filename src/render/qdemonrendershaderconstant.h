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
#ifndef QDEMON_RENDER_SHADER_CONSTANT_H
#define QDEMON_RENDER_SHADER_CONSTANT_H

#include <QtDemonRender/qdemonrendercontext.h>

#include <QtCore/QString>

#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>

#include <limits>

QT_BEGIN_NAMESPACE

///< forward declarations
class QDemonRenderContextImpl;
class QDemonRenderConstantBuffer;

///< A shader constant belongs to a program
class Q_DEMONRENDER_EXPORT QDemonRenderShaderConstantBase
{
public:
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QString m_name; ///< register constant name
    qint32 m_location; ///< constant index
    qint32 m_elementCount; ///< constant element count for arrays
    QDemonRenderShaderDataTypes::Enum m_type; ///< constant type
    qint32 m_binding; ///< sampler/imnage binding point

public:
    QDemonRenderShaderConstantBase(const QDemonRef<QDemonRenderBackend> &backend,
                                   const QString &name,
                                   qint32 location,
                                   qint32 elementCount,
                                   QDemonRenderShaderDataTypes::Enum type,
                                   qint32 binding)
        : m_backend(backend)
        , m_name(name)
        , m_location(location)
        , m_elementCount(elementCount)
        , m_type(type)
        , m_binding(binding)
    {
    }

    virtual ~QDemonRenderShaderConstantBase() {}

    QDemonRenderShaderDataTypes::Enum getShaderConstantType() const { return m_type; }

    virtual void release() = 0;
};

///< A general class for shader types
template <typename TDataType>
class QDemonRenderShaderConstant : public QDemonRenderShaderConstantBase
{
public:
    TDataType m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QDemonRef<QDemonRenderBackend> &backend,
                               const QString &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        memset(&m_value, 0, sizeof(TDataType));
    }

    void release() override {  }
};

///< A specialized class for textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override { }
};

///< A specialized class for textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DHandle> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value.resize(elementCount);
        m_value.fill(std::numeric_limits<quint32>::max());
    }

    void release() override {  }
};

///< A specialized class for texture arrays
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DArrayPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {  }
};

///< A specialized class for cubemap textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTextureCubePtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {  }
};

///< A specialized class for cubemap textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTextureCubeHandle> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value.resize(elementCount);
        m_value.fill(std::numeric_limits<quint32>::max());
    }

    void release() override {  }
};

///< A specialized class for texture image buffer
template <>
class QDemonRenderShaderConstant<QDemonRenderImage2DPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(QDemonRef<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {  }
};

///< Base for any buffer ( constant, texture, ... ) which is used by this program
class QDemonRenderShaderBufferBase
{
public:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QString m_name; ///< buffer name
    quint32 m_location; ///< program buffer block location
    quint32 m_binding; ///< program buffer binding
    qint32 m_size; ///< buffer size

public:
    QDemonRenderShaderBufferBase(QDemonRef<QDemonRenderContextImpl> context, const QString &name,
                                 qint32 location, qint32 binding, qint32 size)
        : m_context(context)
        , m_name(name)
        , m_location(location)
        , m_binding(binding)
        , m_size(size)
    {
    }

    virtual ~QDemonRenderShaderBufferBase() {}

    virtual void release() = 0;

    virtual void validate(QDemonRef<QDemonRenderShaderProgram> inShader) = 0;
    virtual void bindToProgram(QDemonRef<QDemonRenderShaderProgram> inShader) = 0;
    virtual void update() = 0;
};

class QDemonRenderShaderConstantBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QDemonRef<QDemonRenderConstantBuffer> m_constBuffer; ///< pointer to constant buffer

public:
    QDemonRenderShaderConstantBuffer(QDemonRef<QDemonRenderContextImpl> context,
                                     const QString &name,
                                     quint32 location,
                                     qint32 binding,
                                     qint32 size,
                                     qint32 count,
                                     QDemonRef<QDemonRenderConstantBuffer> pCB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_paramCount(count)
        , m_constBuffer(pCB)
    {
    }

    void release() override
    {
    }

    void validate(QDemonRef<QDemonRenderShaderProgram> inShader) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compiled from a cache file
        // Now it must exists.
        if (m_constBuffer)
            return;

        QDemonRef<QDemonRenderConstantBuffer> cb = m_context->getConstantBuffer(m_name);
        if (cb) {
            cb->setupBuffer(inShader.data(), m_location, m_size, m_paramCount);
            //cb->addRef();
            m_constBuffer = cb;
        } else {
            Q_ASSERT(false);
        }
    }

    void update() override
    {
        if (m_constBuffer)
            m_constBuffer->update();
    }

    void bindToProgram(QDemonRef<QDemonRenderShaderProgram> inShader) override
    {
        if (m_constBuffer)
            m_constBuffer->bindToShaderProgram(inShader, m_location, m_binding);
    }
};

class QDemonRenderShaderStorageBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QDemonRef<QDemonRenderStorageBuffer> m_storageBuffer; ///< pointer to storage buffer

public:
    QDemonRenderShaderStorageBuffer(QDemonRef<QDemonRenderContextImpl> context,
                                    const QString &name,
                                    quint32 location,
                                    qint32 binding,
                                    qint32 size,
                                    qint32 count,
                                    QDemonRef<QDemonRenderStorageBuffer> pSB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_paramCount(count)
        , m_storageBuffer(pSB)
    {
    }

    void release() override
    {
    }

    void validate(QDemonRef<QDemonRenderShaderProgram> /*inShader*/) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compile from a cache file
        // Now it must exists.
        if (m_storageBuffer)
            return;

        QDemonRef<QDemonRenderStorageBuffer> sb = m_context->getStorageBuffer(m_name);
        if (sb) {
            m_storageBuffer = sb;
        } else {
            Q_ASSERT(false);
        }
    }

    void update() override
    {
        if (m_storageBuffer)
            m_storageBuffer->update();
    }

    void bindToProgram(QDemonRef<QDemonRenderShaderProgram> /*inShader*/) override
    {
        if (m_storageBuffer)
            m_storageBuffer->bindToShaderProgram(m_location);
    }
};

class QDemonRenderShaderAtomicCounterBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QSharedPointer<QDemonRenderAtomicCounterBuffer> m_atomicCounterBuffer; ///< pointer to atomic counter buffer

public:
    QDemonRenderShaderAtomicCounterBuffer(QDemonRef<QDemonRenderContextImpl> context,
                                          const QString &name,
                                          quint32 location,
                                          qint32 binding,
                                          qint32 size,
                                          qint32 count,
                                          QSharedPointer<QDemonRenderAtomicCounterBuffer> pAcB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_paramCount(count)
        , m_atomicCounterBuffer(pAcB)
    {
    }

    void release() override
    {
    }

    void validate(QDemonRef<QDemonRenderShaderProgram> /*inShader*/) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compile from a cache file
        // Now it must exists.
        if (m_atomicCounterBuffer)
            return;

        QSharedPointer<QDemonRenderAtomicCounterBuffer> acb = m_context->getAtomicCounterBuffer(m_name);
        if (acb) {
            m_atomicCounterBuffer = acb;
        } else {
            Q_ASSERT(false);
        }
    }

    void update() override
    {
        if (m_atomicCounterBuffer)
            m_atomicCounterBuffer->update();
    }

    void bindToProgram(QDemonRef<QDemonRenderShaderProgram> /*inShader*/) override
    {
        if (m_atomicCounterBuffer)
            m_atomicCounterBuffer->bindToShaderProgram(m_location);
    }
};

QT_END_NAMESPACE

#endif
