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
#ifndef QDEMON_RENDER_RENDER_FRAME_BUFFER_H
#define QDEMON_RENDER_RENDER_FRAME_BUFFER_H

#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderbackend.h>

QT_BEGIN_NAMESPACE

class QDemonRenderContextImpl;
class QDemonRenderTexture2D;
class QDemonRenderRenderBuffer;
class QDemonRenderTexture2DArray;
class QDemonRenderTextureCube;

class Q_DEMONRENDER_EXPORT QDemonRenderTextureOrRenderBuffer
{
    QDemonRef<QDemonRenderTexture2D> m_texture2D;
    QDemonRef<QDemonRenderTexture2DArray> m_texture2DArray;
    QDemonRef<QDemonRenderTextureCube> m_textureCube;
    QDemonRef<QDemonRenderRenderBuffer> m_renderBuffer;

public:
    QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTexture2D> texture);
    QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderRenderBuffer> render);
    QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTexture2DArray> textureArray);
    QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTextureCube> textureCube);
    QDemonRenderTextureOrRenderBuffer();
    QDemonRenderTextureOrRenderBuffer(const QDemonRenderTextureOrRenderBuffer &other);
    ~QDemonRenderTextureOrRenderBuffer();

    QDemonRenderTextureOrRenderBuffer &operator=(const QDemonRenderTextureOrRenderBuffer &other);

    bool hasTexture2D() const { return m_texture2D != nullptr; }
    bool hasTexture2DArray() const { return m_texture2DArray != nullptr; }
    bool hasTextureCube() const { return m_textureCube != nullptr; }
    bool hasRenderBuffer() const { return m_renderBuffer != nullptr; }

    QDemonRef<QDemonRenderTexture2D> getTexture2D() const;
    QDemonRef<QDemonRenderTexture2DArray> getTexture2DArray() const;
    QDemonRef<QDemonRenderTextureCube> getTextureCube() const;
    QDemonRef<QDemonRenderRenderBuffer> getRenderBuffer() const;
};

class QDemonRenderFrameBuffer : public QDemonRenderImplemented
{
public:
    QAtomicInt ref;

private:
    QDemonRef<QDemonRenderContextImpl> m_context; ///< pointer to context
    QDemonRef<QDemonRenderBackend> m_backend; ///< pointer to backend

    QDemonRenderTextureOrRenderBuffer m_attachments[static_cast<int>(QDemonRenderFrameBufferAttachment::LastAttachment)]; ///< attachments array
    QDemonRenderBackend::QDemonRenderBackendRenderTargetObject m_bufferHandle; ///< opaque backend handle

public:
    /**
     * @brief constructor
     *
     * @param[in] context		Pointer to context
     * @param[in] fnd			Pointer to foundation
     *
     * @return No return.
     */
    QDemonRenderFrameBuffer(const QDemonRef<QDemonRenderContextImpl> &context);

    /// destructor
    virtual ~QDemonRenderFrameBuffer() override;

    /**
     * @brief query attachment
     *
     *
     * @return buffer format
     */
    virtual QDemonRenderTextureOrRenderBuffer getAttachment(QDemonRenderFrameBufferAttachment attachment);

    /**
     * @brief Attach a render or texture buffer to a render target
     *		  For texture attachments we use always level 0
     *
     * @param[in] attachment		Attachment point (e.g. COLOR0, DEPTH...)
     * @param[in] buffer			Contains a pointer to the attachment
     * @param[in] target			Attachment texture target
     *
     * @return no return
     */
    virtual void attach(QDemonRenderFrameBufferAttachment attachment,
                        QDemonRenderTextureOrRenderBuffer buffer,
                        QDemonRenderTextureTargetType target = QDemonRenderTextureTargetType::Texture2D);

    /**
     * @brief Attach a particular layer of the texture 2D array to a render target
     *
     * @param[in] attachment		Attachment point (e.g. COLOR0, DEPTH...)
     * @param[in] buffer			Pointer to the Texture Array which contains the
     * layers
     * @param[in] layer				The index to the layer that will be attached to the
     * target
     * @param[in] level				Mip level of the texture that will be attached
     * (default 0)
     *
     * @return no return
     */
    virtual void attachLayer(QDemonRenderFrameBufferAttachment attachment,
                             QDemonRenderTextureOrRenderBuffer buffer,
                             qint32 layer,
                             qint32 level = 0);

    /**
     * @brief Attach a particular face of the texture cubemap to a render target
     *
     * @param[in] attachment		Attachment point (e.g. COLOR0, DEPTH...)
     * @param[in] buffer			Pointer to the Texture Array which contains the
     * layers
     * @param[in] face				The face of the cubemap that will be attached to the
     * target
     * @param[in] level				Mip level of the texture that will be attached
     * (default 0)
     *
     * @return no return
     */
    virtual void attachFace(QDemonRenderFrameBufferAttachment attachment,
                            QDemonRenderTextureOrRenderBuffer buffer,
                            QDemonRenderTextureCubeFace face);

    /**
     * @brief Check that this framebuffer is complete and can be rendered to.
     *
     *
     * @return true if complete
     */
    virtual bool isComplete();

    /**
     * @brief query if framebuffer has any attachment
     *
     * @return true if any attachment
     */
    virtual bool hasAnyAttachment() { return (m_attachmentBits != 0); }

    /**
     * @brief get the backend object handle
     *
     * @return the backend object handle.
     */
    virtual QDemonRenderBackend::QDemonRenderBackendRenderTargetObject getFrameBuffertHandle()
    {
        return m_bufferHandle;
    }

    // this will be obsolete
    const void *getImplementationHandle() const override { return reinterpret_cast<const void *>(m_bufferHandle); }

    /**
     * @brief static creator function
     *
     * @param[in] context		Pointer to context
     *
     * @return a pointer to framebuffer object.
     */
    static QDemonRef<QDemonRenderFrameBuffer> create(const QDemonRef<QDemonRenderContextImpl> &context);

private:
    /**
     * @brief releaes an attached object
     *
     * @return which target we released
     */
    QDemonRenderTextureTargetType releaseAttachment(QDemonRenderFrameBufferAttachment idx);

    quint32 m_attachmentBits; ///< holds flags for current attached buffers
};

QT_END_NAMESPACE

#endif
