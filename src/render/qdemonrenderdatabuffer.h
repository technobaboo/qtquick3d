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
#ifndef QDEMON_RENDER_DATA_BUFFER_H
#define QDEMON_RENDER_DATA_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;
class QDemonRenderBackend;

///< Base class
class Q_DEMONRENDER_EXPORT QDemonRenderDataBuffer
{
    Q_DISABLE_COPY(QDemonRenderDataBuffer)
public:
    QAtomicInt ref;

protected:
    QDemonRef<QDemonRenderContext> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBufferUsageType m_usageType; ///< usage type
    QDemonRenderBufferType m_type; ///< bind flags
    QDemonByteView m_bufferData; ///< buffer data pointer
    quint32 m_bufferCapacity; ///< size of internal backup buffer (m_bufferData)
    size_t m_bufferSize; ///< size of buffer
    bool m_mapped; ///< true when locked for reading or writing to m_bufferData
    QDemonRenderBackend::QDemonRenderBackendBufferObject m_handle; ///< opaque backend handle

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
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
    QDemonRenderDataBuffer(const QDemonRef<QDemonRenderContext> &context,
                           size_t size,
                           QDemonRenderBufferType bindFlags,
                           QDemonRenderBufferUsageType usageType,
                           QDemonByteView data);

    virtual ~QDemonRenderDataBuffer();

    /**
     * @brief Get Buffer usage type
     *
     * @return Return usage tyoe
     */
    QDemonRenderBufferUsageType usageType() const { return m_usageType; }

    /**
     * @brief Get Buffer usage type
     *		  Should be overwritten
     *
     * @return Return usage tyoe
     */
    QDemonRenderBufferType type() const { return m_type; }

    /**
     * @brief Return buffer size in byte
     *
     * @return Return size
     */
    inline quint32 size() { return quint32(m_bufferSize); }

    /**
     * @brief Get a pointer to the foundation
     *
     * @return pointer to foundation
     */

    /**
     * @brief bind the buffer bypasses the context state
     *
     * @return no return.
     */
    virtual void bind() = 0;

    /**
     * @brief Map the buffer
     *		  Mapped buffers cannot be used for rendering
     *
     * @return Return mapped pointer to data
     */
    QDemonByteRef mapBuffer();

    /**
     * @brief Map a range of a  buffer
     *		  Mapped buffers cannot be used for rendering
     *
     * @param[in] offset	Offset in bytes into the buffer
     * @param[in] size		Range in bytes to map
     * @param[in] flags		Buffer access flags
     *
     * @return Return mapped pointer to data
     */
    QDemonByteRef mapBufferRange(size_t offset, size_t size, QDemonRenderBufferAccessFlags flags);

    /**
     * @brief Unmap the buffer
     *		  This updates the data to the hardware
     *
     * @return no return
     */
    void unmapBuffer();

    /**
     * @brief constructor
     *
     * @param[in] data			A pointer to the buffer data that is allocated by the
     * application.
     * @param[in] ownsMemory	If true data will be owned by this object and can therefore be
     * released
     *
     * @return No return.
     */
    void updateBuffer(QDemonByteView data);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendBufferObject handle() const { return m_handle; }
};

QT_END_NAMESPACE

#endif
