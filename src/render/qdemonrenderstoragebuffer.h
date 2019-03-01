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
#ifndef QDEMON_RENDER_STORAGE_BUFFER_H
#define QDEMON_RENDER_STORAGE_BUFFER_H

#include <QtDemonRender/qdemonrenderdatabuffer.h>

#include <QtCore/QString>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContextImpl;
class QDemonRenderVertexBuffer;

///< Constant (uniform) buffer representation
class QDemonRenderStorageBuffer : public QDemonRenderDataBuffer
{
public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to context
         * @param[in] bufferName	Name of the buffer. Must match the name used in programs
         * @param[in] size			Size of the buffer
         * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
         * @param[in] data			A pointer to the buffer data that is allocated by the
         * application.
         * @param[in] pBuffer		Pointer to the buffer
         *
         * @return No return.
         */
    QDemonRenderStorageBuffer(const QDemonRef<QDemonRenderContextImpl> &context, const QString &bufferName,
                              size_t size, QDemonRenderBufferUsageType::Enum usageType,
                              QDemonDataRef<quint8> data, QDemonRenderDataBuffer *pBuffer = nullptr);

    ///< destructor
    virtual ~QDemonRenderStorageBuffer();

    /**
         * @brief bind the buffer bypasses the context state
         *
         * @return no return.
         */
    void bind() override;

    /**
         * @brief bind the buffer to a shader program
         *
         * @param[in] index			Index of the constant buffer within the program
         *
         * @return no return.
         */
    virtual void bindToShaderProgram(quint32 index);

    /**
         * @brief update the buffer to hardware
         *
         * @return no return.
         */
    virtual void update();

    /**
         * @brief update a piece of memory directly within the storage buffer
         *
         * Note: When you use this function you should know what you are doing.
         *		 The memory layout within C++ must exactly match the memory layout in the
         *shader.
         *		 We use std140 (430) layout which guarantees a specific layout behavior across
         *all HW vendors.
         *		 How the memory layout is computed can be found in the GL spec.
         *
         * @param[in] offset	offset into storage buffer
         * @param[in] data		pointer to data
         *
         * @return no return
         */
    void updateData(qint32 offset, QDemonDataRef<quint8> data);

    /**
         * @brief get the buffer name
         *
         * @return the buffer name
         */
    QString getBufferName() const { return m_name; }

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    QDemonRenderBackend::QDemonRenderBackendBufferObject getBuffertHandle() const override
    {
        return m_bufferHandle;
    }

    // this will be obsolete
    const void *getImplementationHandle() const override
    {
        return reinterpret_cast<void *>(m_bufferHandle);
    }

    /**
         * @brief create a QDemonRenderStorageBuffer object
         *
         * @param[in] context		Pointer to context
         * @param[in] size			Size of the buffer
         * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
         * @param[in] data			A pointer to the buffer data that is allocated by the
         * application.
         *
         * @return the buffer object or nullptr
         */
    static QDemonRef<QDemonRenderStorageBuffer> create(const QDemonRef<QDemonRenderContextImpl> &context, const char *bufferName,
                                             QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                             QDemonConstDataRef<quint8> bufferData,
                                             QDemonRenderDataBuffer *pBuffer);

private:
    QString m_name; ///< buffer name
    QDemonRenderDataBuffer *m_wrappedBuffer; ///< pointer to wrapped buffer
    bool m_dirty; ///< true if buffer is dirty
};

QT_END_NAMESPACE

#endif
