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
class QDemonRenderContextImpl;
class QDemonRenderBackend;

///< Base class
class QDemonRenderDataBuffer : public QDemonRenderImplemented
{
protected:
    QSharedPointer<QDemonRenderContextImpl> m_context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_backend; ///< pointer to backend
    QDemonRenderBufferUsageType::Enum m_usageType; ///< usage type
    QDemonRenderBufferBindFlags m_bindFlags; ///< bind flags
    QDemonDataRef<quint8> m_bufferData; ///< buffer data pointer
    quint32 m_bufferCapacity; ///< size of internal backup buffer (m_bufferData)
    size_t m_bufferSize; ///< size of buffer
    bool m_ownsData; ///< true when we own m_bufferData
    bool m_mapped; ///< true when locked for reading or writing to m_bufferData
    QDemonRenderBackend::QDemonRenderBackendBufferObject m_bufferHandle; ///< opaque backend handle

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
    QDemonRenderDataBuffer(const QSharedPointer<QDemonRenderContextImpl> &context, size_t size,
                           QDemonRenderBufferBindFlags bindFlags,
                           QDemonRenderBufferUsageType::Enum usageType, QDemonDataRef<quint8> data);

    virtual ~QDemonRenderDataBuffer();

    /**
         * @brief Get Buffer usage type
         *
         * @return Return usage tyoe
         */
    virtual QDemonRenderBufferUsageType::Enum getBufferUsageType() const
    {
        return m_usageType;
    }

    /**
         * @brief Get Buffer usage type
         *		  Should be overwritten
         *
         * @return Return usage tyoe
         */
    virtual QDemonRenderBufferBindFlags getBufferBindings() const { return m_bindFlags; }

    /**
         * @brief Return buffer size in byte
         *
         * @return Return size
         */
    virtual quint32 size() { return quint32(m_bufferSize); }

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
    virtual QDemonDataRef<quint8> mapBuffer();

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
    virtual QDemonDataRef<quint8> mapBufferRange(size_t offset, size_t size, QDemonRenderBufferAccessFlags flags);

    /**
         * @brief Unmap the buffer
         *		  This updates the data to the hardware
         *
         * @return no return
         */
    virtual void unmapBuffer();

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
    virtual void updateBuffer(QDemonConstDataRef<quint8> data, bool ownsMemory = false);

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendBufferObject getBuffertHandle() const = 0;

    // this will be obsolete
    const void *getImplementationHandle() const override = 0;

private:
    void releaseMemory();
};

QT_END_NAMESPACE

#endif
