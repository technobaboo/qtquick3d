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
#pragma once
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
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend
    QString m_Name; ///< register constant name
    qint32 m_Location; ///< constant index
    qint32 m_ElementCount; ///< constant element count for arrays
    QDemonRenderShaderDataTypes::Enum m_Type; ///< constant type
    qint32 m_Binding; ///< sampler/imnage binding point

public:
    QDemonRenderShaderConstantBase(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                                   qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                                   qint32 binding)
        : m_Backend(backend)
        , m_Name(name)
        , m_Location(location)
        , m_ElementCount(elementCount)
        , m_Type(type)
        , m_Binding(binding)
    {
    }

    QDemonRenderShaderDataTypes::Enum GetShaderConstantType() const { return m_Type; }

    virtual void Release() = 0;
};

///< A general class for shader types
template <typename TDataType>
class QDemonRenderShaderConstant : public QDemonRenderShaderConstantBase
{
public:
    TDataType m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        memset(&m_Value, 0, sizeof(TDataType));
    }

    void Release() override {  } // ### cleanup
};

///< A specialized class for textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value = std::numeric_limits<quint32>::max();
    }

    void Release() override { } // ### cleanup
};

///< A specialized class for textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DHandle> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value.resize(elementCount);
        m_Value.fill(std::numeric_limits<quint32>::max());
    }

    void Release() override {  } // ### cleanup
};

///< A specialized class for texture arrays
template <>
class QDemonRenderShaderConstant<QDemonRenderTexture2DArrayPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value = std::numeric_limits<quint32>::max();
    }

    void Release() override {  } // ### cleanup
};

///< A specialized class for cubemap textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTextureCubePtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value = std::numeric_limits<quint32>::max();
    }

    void Release() override {  } // ### cleanup
};

///< A specialized class for cubemap textures
template <>
class QDemonRenderShaderConstant<QDemonRenderTextureCubeHandle> : public QDemonRenderShaderConstantBase
{
public:
    QVector<quint32> m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value.resize(elementCount);
        m_Value.fill(std::numeric_limits<quint32>::max());
    }

    void Release() override {  } // ### cleanup
};

///< A specialized class for texture image buffer
template <>
class QDemonRenderShaderConstant<QDemonRenderImage2DPtr> : public QDemonRenderShaderConstantBase
{
public:
    quint32 m_Value; ///< constant value

public:
    QDemonRenderShaderConstant(QSharedPointer<QDemonRenderBackend> backend, const QString &name, qint32 location,
                               qint32 elementCount, QDemonRenderShaderDataTypes::Enum type,
                               qint32 binding)
        : QDemonRenderShaderConstantBase(backend, name, location, elementCount, type, binding)
    {
        m_Value = std::numeric_limits<quint32>::max();
    }

    void Release() override {  } // ### cleanup
};

///< Base for any buffer ( constant, texture, ... ) which is used by this program
class QDemonRenderShaderBufferBase
{
public:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QString m_Name; ///< buffer name
    quint32 m_Location; ///< program buffer block location
    quint32 m_Binding; ///< program buffer binding
    qint32 m_Size; ///< buffer size

public:
    QDemonRenderShaderBufferBase(QDemonRenderContextImpl &context, const QString &name,
                                 qint32 location, qint32 binding, qint32 size)
        : m_Context(context)
        , m_Name(name)
        , m_Location(location)
        , m_Binding(binding)
        , m_Size(size)
    {
    }

    virtual void Release() = 0;

    virtual void Validate(QSharedPointer<QDemonRenderShaderProgram> inShader) = 0;
    virtual void BindToProgram(QSharedPointer<QDemonRenderShaderProgram> inShader) = 0;
    virtual void Update() = 0;
};

class QDemonRenderShaderConstantBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_ParamCount; ///< count of parameters contained in the constant buffer
    QSharedPointer<QDemonRenderConstantBuffer> m_pCB; ///< pointer to constant buffer

public:
    QDemonRenderShaderConstantBuffer(QDemonRenderContextImpl &context, const QString &name,
                                     quint32 location, qint32 binding, qint32 size, qint32 count,
                                     QSharedPointer<QDemonRenderConstantBuffer> pCB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_ParamCount(count)
        , m_pCB(pCB)
    {
    }

    void Release() override
    {
    }

    void Validate(QSharedPointer<QDemonRenderShaderProgram> inShader) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compiled from a cache file
        // Now it must exists.
        if (m_pCB)
            return;

        QSharedPointer<QDemonRenderConstantBuffer> cb = m_Context.GetConstantBuffer(m_Name);
        if (cb) {
            cb->SetupBuffer(inShader.data(), m_Location, m_Size, m_ParamCount);
            //cb->addRef();
            m_pCB = cb;
        } else {
            Q_ASSERT(false);
        }
    }

    void Update() override
    {
        if (m_pCB)
            m_pCB->Update();
    }

    void BindToProgram(QSharedPointer<QDemonRenderShaderProgram> inShader) override
    {
        if (m_pCB)
            m_pCB->BindToShaderProgram(inShader, m_Location, m_Binding);
    }
};

class QDemonRenderShaderStorageBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_ParamCount; ///< count of parameters contained in the constant buffer
    QSharedPointer<QDemonRenderStorageBuffer> m_pSB; ///< pointer to storage buffer

public:
    QDemonRenderShaderStorageBuffer(QDemonRenderContextImpl &context, const QString &name,
                                    quint32 location, qint32 binding, qint32 size, qint32 count,
                                    QSharedPointer<QDemonRenderStorageBuffer> pSB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_ParamCount(count)
        , m_pSB(pSB)
    {
    }

    void Release() override
    {
    }

    void Validate(QSharedPointer<QDemonRenderShaderProgram> /*inShader*/) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compile from a cache file
        // Now it must exists.
        if (m_pSB)
            return;

        QSharedPointer<QDemonRenderStorageBuffer> sb = m_Context.GetStorageBuffer(m_Name);
        if (sb) {
            m_pSB = sb;
        } else {
            Q_ASSERT(false);
        }
    }

    void Update() override
    {
        if (m_pSB)
            m_pSB->Update();
    }

    void BindToProgram(QSharedPointer<QDemonRenderShaderProgram> /*inShader*/) override
    {
        if (m_pSB)
            m_pSB->BindToShaderProgram(m_Location);
    }
};

class QDemonRenderShaderAtomicCounterBuffer : public QDemonRenderShaderBufferBase
{
public:
    qint32 m_ParamCount; ///< count of parameters contained in the constant buffer
    QSharedPointer<QDemonRenderAtomicCounterBuffer> m_pAcB; ///< pointer to atomic counter buffer

public:
    QDemonRenderShaderAtomicCounterBuffer(QDemonRenderContextImpl &context, const QString &name,
                                          quint32 location, qint32 binding, qint32 size, qint32 count,
                                          QSharedPointer<QDemonRenderAtomicCounterBuffer> pAcB)
        : QDemonRenderShaderBufferBase(context, name, location, binding, size)
        , m_ParamCount(count)
        , m_pAcB(pAcB)
    {
    }

    void Release() override
    {
    }

    void Validate(QSharedPointer<QDemonRenderShaderProgram> /*inShader*/) override
    {
        // A constant buffer might not be set at first call
        // due to the fact that they are compile from a cache file
        // Now it must exists.
        if (m_pAcB)
            return;

        QSharedPointer<QDemonRenderAtomicCounterBuffer> acb = m_Context.GetAtomicCounterBuffer(m_Name);
        if (acb) {
            m_pAcB = acb;
        } else {
            Q_ASSERT(false);
        }
    }

    void Update() override
    {
        if (m_pAcB)
            m_pAcB->Update();
    }

    void BindToProgram(QSharedPointer<QDemonRenderShaderProgram> /*inShader*/) override
    {
        if (m_pAcB)
            m_pAcB->BindToShaderProgram(m_Location);
    }
};

QT_END_NAMESPACE

#endif
