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
#ifndef QDEMON_RENDER_INDEX_BUFFER_H
#define QDEMON_RENDER_INDEX_BUFFER_H
#include <QtDemonRender/qdemonrenderdatabuffer.h>
#include <QtDemonRender/qdemonrenderdrawable.h>

QT_BEGIN_NAMESPACE

// forward declaration
class QDemonRenderContext;

class QDemonRenderIndexBuffer : public QDemonRenderDataBuffer, public QDemonRenderDrawable
{
public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] size			Size of the buffer
     * @param[in] componentType	Size of the buffer
     * @param[in] usage			Usage of the buffer (e.g. static, dynamic...)
     * @param[in] data			A pointer to the buffer data that is allocated by the
     * application.
     *
     * @return No return.
     */
    QDemonRenderIndexBuffer(const QDemonRef<QDemonRenderContext> &context,
                            size_t size,
                            QDemonRenderComponentType componentType,
                            QDemonRenderBufferUsageType usageType,
                            QDemonDataRef<quint8> data);

    ///< destruvtor
    ~QDemonRenderIndexBuffer();

    /**
     * @brief get the component type (quint8, quint16)
     *
     * @return the component type
     */
    virtual QDemonRenderComponentType getComponentType() const { return m_componentType; }

    /**
     * @brief get the index count
     *
     * @return actual index count
     */
    virtual quint32 getNumIndices() const;

    /**
     * @brief bind the buffer bypasses the context state
     *
     * @return no return.
     */
    void bind() override;

    /**
     * @brief draw the buffer
     *
     * @param[in] drawMode		draw mode (e.g Triangles...)
     * @param[in] count			vertex count
     * @param[in] offset		start offset in byte
     *
     * @return no return.
     */
    void draw(QDemonRenderDrawMode drawMode, quint32 count, quint32 offset) override;

    /**
     * @brief draw the buffer via indirec draw buffer setup
     *
     * @param[in] drawMode		draw mode (e.g Triangles...)
     * @param[in] offset		byte offset into the bound drawIndirectBuffer see
     * QDemonRenderDrawIndirectBuffer
     *
     * @return no return.
     */
    virtual void drawIndirect(QDemonRenderDrawMode drawMode, quint32 offset);

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    QDemonRenderBackend::QDemonRenderBackendBufferObject handle() const override { return m_bufferHandle; }

    static QDemonRef<QDemonRenderIndexBuffer> create(const QDemonRef<QDemonRenderContext> &context,
                                                     QDemonRenderBufferUsageType usageType,
                                                     QDemonRenderComponentType componentType,
                                                     size_t size,
                                                     QDemonConstDataRef<quint8> bufferData);

private:
    QDemonRenderComponentType m_componentType; ///< component type (quint8, quint16)
};

QT_END_NAMESPACE

#endif
