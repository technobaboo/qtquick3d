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

#ifndef QDEMON_RENDER_SHADER_CONSTANT_H
#define QDEMON_RENDER_SHADER_CONSTANT_H

#include <QtDemonRender/qdemonrendercontext.h>

#include <QtCore/QByteArray>

#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>

#include <limits>

QT_BEGIN_NAMESPACE

///< forward declarations
class QDemonRenderContext;
class QDemonRenderConstantBuffer;

///< A shader constant belongs to a program
class Q_DEMONRENDER_EXPORT QDemonRenderShaderConstantBase
{
public:
    QAtomicInt ref;
    QByteArray m_name; ///< register constant name
    qint32 m_location; ///< constant index
    qint32 m_elementCount; ///< constant element count for arrays
    QDemonRenderShaderDataType m_type; ///< constant type
    qint32 m_binding; ///< sampler/imnage binding point

public:
    QDemonRenderShaderConstantBase(const QByteArray &name,
                                   qint32 location,
                                   qint32 elementCount,
                                   QDemonRenderShaderDataType type,
                                   qint32 binding)
        : m_name(name), m_location(location), m_elementCount(elementCount), m_type(type), m_binding(binding)
    {
    }

    virtual ~QDemonRenderShaderConstantBase() {}

    QDemonRenderShaderDataType getShaderConstantType() const { return m_type; }

    virtual void release() = 0;
};

///< A general class for shader types
template<typename TDataType>
class QDemonRenderShaderConstant : public QDemonRenderShaderConstantBase
{
public:
    TDataType m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        memset(&m_value, 0, sizeof(TDataType));
    }

    void release() override {}
};

///< A specialized class for textures
template<>
class QDemonRenderShaderConstant<QDemonRenderTexture2D *> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {}
};

///< A specialized class for textures
template<>
class QDemonRenderShaderConstant<QDemonRenderTexture2D **> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value.resize(elementCount);
        m_value.fill(std::numeric_limits<quint32>::max());
    }

    void release() override {}
};

///< A specialized class for texture arrays
template<>
class QDemonRenderShaderConstant<QDemonRenderTexture2DArray *> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {}
};

///< A specialized class for cubemap textures
template<>
class QDemonRenderShaderConstant<QDemonRenderTextureCube *> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {}
};

///< A specialized class for cubemap textures
template<>
class QDemonRenderShaderConstant<QDemonRenderTextureCube **> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value.resize(elementCount);
        m_value.fill(std::numeric_limits<quint32>::max());
    }

    void release() override {}
};

///< A specialized class for texture image buffer
template<>
class QDemonRenderShaderConstant<QDemonRenderImage2D *> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_value; ///< constant value

public:
    QDemonRenderShaderConstant(const QByteArray &name,
                               qint32 location,
                               qint32 elementCount,
                               QDemonRenderShaderDataType type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(name, location, elementCount, type, binding)
    {
        m_value = std::numeric_limits<quint32>::max();
    }

    void release() override {}
};

///< Base for any buffer ( constant, texture, ... ) which is used by this program
class QDemonRenderShaderBufferBase
{
public:
    QAtomicInt ref;
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QByteArray m_name; ///< buffer name
    quint32 m_location; ///< program buffer block location
    quint32 m_binding; ///< program buffer binding
    qint32 m_size; ///< buffer size

public:
    QDemonRenderShaderBufferBase(const QDemonRef<QDemonRenderContext> &context, const QByteArray &name, qint32 location, qint32 binding, qint32 size)
        : m_context(context), m_name(name), m_location(location), m_binding(binding), m_size(size)
    {
    }

    virtual ~QDemonRenderShaderBufferBase() {}

    virtual void release() = 0;

    virtual void validate(const QDemonRef<QDemonRenderShaderProgram> &inShader) = 0;
    virtual void bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &inShader) = 0;
    virtual void update() = 0;
};

class QDemonRenderShaderConstantBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QDemonRef<QDemonRenderConstantBuffer> m_constBuffer; ///< pointer to constant buffer

public:
    QDemonRenderShaderConstantBuffer(const QDemonRef<QDemonRenderContext> &context,
                                     const QByteArray &name,
                                     quint32 location,
                                     qint32 binding,
                                     qint32 size,
                                     qint32 count,
                                     QDemonRef<QDemonRenderConstantBuffer> pCB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size), m_paramCount(count), m_constBuffer(pCB)
    {
    }

    void release() override {}

    void validate(const QDemonRef<QDemonRenderShaderProgram> &inShader) override;

    void update() override
    {
        if (m_constBuffer)
            m_constBuffer->update();
    }

    void bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &inShader) override;
};

class QDemonRenderShaderStorageBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QDemonRef<QDemonRenderStorageBuffer> m_storageBuffer; ///< pointer to storage buffer

public:
    QDemonRenderShaderStorageBuffer(const QDemonRef<QDemonRenderContext> &context,
                                    const QByteArray &name,
                                    quint32 location,
                                    qint32 binding,
                                    qint32 size,
                                    qint32 count,
                                    QDemonRef<QDemonRenderStorageBuffer> pSB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size), m_paramCount(count), m_storageBuffer(pSB)
    {
    }

    void release() override {}

    void validate(const QDemonRef<QDemonRenderShaderProgram> &/*inShader*/) override;

    void update() override
    {
        if (m_storageBuffer)
            m_storageBuffer->update();
    }

    void bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &/*inShader*/) override;
};

class QDemonRenderShaderAtomicCounterBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_paramCount; ///< count of parameters contained in the constant buffer
    QDemonRef<QDemonRenderAtomicCounterBuffer> m_atomicCounterBuffer; ///< pointer to atomic counter buffer

public:
    QDemonRenderShaderAtomicCounterBuffer(const QDemonRef<QDemonRenderContext> &context,
                                          const QByteArray &name,
                                          quint32 location,
                                          qint32 binding,
                                          qint32 size,
                                          qint32 count,
                                          QDemonRef<QDemonRenderAtomicCounterBuffer> pAcB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size), m_paramCount(count), m_atomicCounterBuffer(pAcB)
    {
    }

    void release() override {}

    void validate(const QDemonRef<QDemonRenderShaderProgram> &/*inShader*/) override;

    void update() override
    {
        if (m_atomicCounterBuffer)
            m_atomicCounterBuffer->update();
    }

    void bindToProgram(const QDemonRef<QDemonRenderShaderProgram> &/*inShader*/) override;
};

QT_END_NAMESPACE

#endif
