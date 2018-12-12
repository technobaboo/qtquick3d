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
#ifndef QDEMON_RENDER_VERTEX_BUFFER_H
#define QDEMON_RENDER_VERTEX_BUFFER_H

#include <QtDemonRender/qdemonrenderdrawable.h>
#include <QtDemonRender/qdemonrenderdatabuffer.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContextImpl;

///< Vertex buffer representation
class QDemonRenderVertexBuffer : public QDemonRenderDataBuffer, public QEnableSharedFromThis<QDemonRenderVertexBuffer>
{
public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to context
         * @param[in] entries		Vertex buffer attribute layout entries
         * @param[in] size			Size of the buffer
         * @param[in] bindFlags		Where to binf this buffer (e.g. vertex, index, ...)
         *							For OpenGL this should be a single
         *value
         * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
         * @param[in] data			A pointer to the buffer data that is allocated by the
         *application.
         *
         * @return No return.
         */
    QDemonRenderVertexBuffer(QDemonRenderContextImpl &context, size_t size, quint32 stride,
                             QDemonRenderBufferBindFlags bindFlags,
                             QDemonRenderBufferUsageType::Enum usageType, QDemonDataRef<quint8> data);

    ///< destructor
    virtual ~QDemonRenderVertexBuffer();

    /**
         * @brief return vertex data stride
         *
         * @return data stride.
         */
    virtual quint32 GetStride() const { return m_Stride; }

    /**
         * @brief get vertex count
         *
         * @return vertex count
         */
    virtual quint32 GetNumVertexes() const
    {
        Q_ASSERT((m_BufferCapacity % m_Stride) == 0);
        return m_BufferCapacity / m_Stride;
    }

    /**
         * @brief bind the buffer bypasses the context state
         *
         * @return no return.
         */
    void Bind() override;

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendBufferObject GetBuffertHandle() const override
    {
        return m_BufferHandle;
    }

    // this will be obsolete
    const void *GetImplementationHandle() const override
    {
        return reinterpret_cast<void *>(m_BufferHandle);
    }

    // No stride means that stride is calculated from the size of last entry found via entry
    // offset
    // Leaves this buffer temporarily bound.
    static QSharedPointer<QDemonRenderVertexBuffer> Create(QDemonRenderContextImpl &context,
                                            QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                            quint32 stride, QDemonConstDataRef<quint8> bufferData);

private:
    quint32 m_Stride; ///< veretex data stride
};

QT_END_NAMESPACE

#endif
