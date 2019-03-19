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
#ifndef QDEMON_RENDER_VERTEX_BUFFER_H
#define QDEMON_RENDER_VERTEX_BUFFER_H

#include <QtDemonRender/qdemonrenderdatabuffer.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;

///< Vertex buffer representation
class Q_DEMONRENDER_EXPORT QDemonRenderVertexBuffer : public QDemonRenderDataBuffer
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
    QDemonRenderVertexBuffer(const QDemonRef<QDemonRenderContext> &context,
                             QDemonRenderBufferUsageType usageType,
                             size_t size,
                             quint32 stride,
                             QDemonDataView<quint8> data);

    ///< destructor
    virtual ~QDemonRenderVertexBuffer() override;

    /**
     * @brief return vertex data stride
     *
     * @return data stride.
     */
    quint32 stride() const { return m_stride; }

    /**
     * @brief get vertex count
     *
     * @return vertex count
     */
    quint32 numVertexes() const
    {
        Q_ASSERT((m_bufferCapacity % m_stride) == 0);
        return m_bufferCapacity / m_stride;
    }

    /**
     * @brief bind the buffer bypasses the context state
     *
     * @return no return.
     */
    void bind() override;

private:
    quint32 m_stride;
};

QT_END_NAMESPACE

#endif
