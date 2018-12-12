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

class QDemonRenderTextureOrRenderBuffer
{
    QSharedPointer<QDemonRenderTexture2D> m_Texture2D;
    QSharedPointer<QDemonRenderTexture2DArray> m_Texture2DArray;
    QDemonRenderTextureCube *m_TextureCube;
    QSharedPointer<QDemonRenderRenderBuffer> m_RenderBuffer;

public:
    QDemonRenderTextureOrRenderBuffer(QSharedPointer<QDemonRenderTexture2D> texture)
        : m_Texture2D(texture)
        , m_Texture2DArray(nullptr)
        , m_TextureCube(nullptr)
        , m_RenderBuffer(nullptr)
    {
    }
    QDemonRenderTextureOrRenderBuffer(QSharedPointer<QDemonRenderRenderBuffer> render)
        : m_Texture2D(nullptr)
        , m_Texture2DArray(nullptr)
        , m_TextureCube(nullptr)
        , m_RenderBuffer(render)
    {
    }
    QDemonRenderTextureOrRenderBuffer(QSharedPointer<QDemonRenderTexture2DArray> textureArray)
        : m_Texture2D(nullptr)
        , m_Texture2DArray(textureArray)
        , m_TextureCube(nullptr)
        , m_RenderBuffer(nullptr)
    {
    }
    QDemonRenderTextureOrRenderBuffer(QDemonRenderTextureCube &textureCube)
        : m_Texture2D(nullptr)
        , m_Texture2DArray(nullptr)
        , m_TextureCube(&textureCube)
        , m_RenderBuffer(nullptr)
    {
    }
    QDemonRenderTextureOrRenderBuffer()
        : m_Texture2D(nullptr)
        , m_Texture2DArray(nullptr)
        , m_TextureCube(nullptr)
        , m_RenderBuffer(nullptr)
    {
    }
    QDemonRenderTextureOrRenderBuffer(const QDemonRenderTextureOrRenderBuffer &other)
        : m_Texture2D(other.m_Texture2D)
        , m_Texture2DArray(other.m_Texture2DArray)
        , m_TextureCube(other.m_TextureCube)
        , m_RenderBuffer(other.m_RenderBuffer)
    {
    }
    QDemonRenderTextureOrRenderBuffer &operator=(const QDemonRenderTextureOrRenderBuffer &other)
    {
        if (this != &other) {
            m_Texture2D = QSharedPointer<QDemonRenderTexture2D>(other.m_Texture2D);
            m_Texture2DArray = QSharedPointer<QDemonRenderTexture2DArray>(other.m_Texture2DArray);
            m_RenderBuffer = QSharedPointer<QDemonRenderRenderBuffer>(other.m_RenderBuffer);
            m_TextureCube = const_cast<QDemonRenderTextureCube *>(other.m_TextureCube);
        }
        return *this;
    }

    bool HasTexture2D() const { return m_Texture2D != nullptr; }
    bool HasTexture2DArray() const { return m_Texture2DArray != nullptr; }
    bool HasTextureCube() const { return m_TextureCube != nullptr; }
    bool HasRenderBuffer() const { return m_RenderBuffer != nullptr; }

    QSharedPointer<QDemonRenderTexture2D> GetTexture2D() const
    {
        Q_ASSERT(HasTexture2D());
        return m_Texture2D;
    }
    QSharedPointer<QDemonRenderTexture2DArray> GetTexture2DArray() const
    {
        Q_ASSERT(HasTexture2DArray());
        return m_Texture2DArray;
    }
    QDemonRenderTextureCube *GetTextureCube() const
    {
        Q_ASSERT(HasTextureCube());
        return m_TextureCube;
    }
    QSharedPointer<QDemonRenderRenderBuffer> GetRenderBuffer() const
    {
        Q_ASSERT(HasRenderBuffer());
        return m_RenderBuffer;
    }
};

class QDemonRenderFrameBuffer : public QDemonRenderImplemented, public QEnableSharedFromThis<QDemonRenderFrameBuffer>
{
private:
    QDemonRenderContextImpl &m_Context; ///< pointer to context
    QSharedPointer<QDemonRenderBackend> m_Backend; ///< pointer to backend

    QDemonRenderTextureOrRenderBuffer m_Attachments[QDemonRenderFrameBufferAttachments::LastAttachment]; ///< attachments array
    QDemonRenderBackend::QDemonRenderBackendRenderTargetObject m_BufferHandle; ///< opaque backend handle

public:
    /**
         * @brief constructor
         *
         * @param[in] context		Pointer to context
         * @param[in] fnd			Pointer to foundation
         *
         * @return No return.
         */
    QDemonRenderFrameBuffer(QDemonRenderContextImpl &context);

    /// destructor
    virtual ~QDemonRenderFrameBuffer();

    /**
         * @brief query attachment
         *
         *
         * @return buffer format
         */
    virtual QDemonRenderTextureOrRenderBuffer
    GetAttachment(QDemonRenderFrameBufferAttachments::Enum attachment);

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
    virtual void
    Attach(QDemonRenderFrameBufferAttachments::Enum attachment,
           QDemonRenderTextureOrRenderBuffer buffer,
           QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Texture2D);

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
    virtual void AttachLayer(QDemonRenderFrameBufferAttachments::Enum attachment,
                             QDemonRenderTextureOrRenderBuffer buffer, qint32 layer,
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
    virtual void AttachFace(QDemonRenderFrameBufferAttachments::Enum attachment,
                            QDemonRenderTextureOrRenderBuffer buffer,
                            QDemonRenderTextureCubeFaces::Enum face);

    /**
         * @brief Check that this framebuffer is complete and can be rendered to.
         *
         *
         * @return true if complete
         */
    virtual bool IsComplete();

    /**
         * @brief query if framebuffer has any attachment
         *
         * @return true if any attachment
         */
    virtual bool HasAnyAttachment() { return (m_AttachmentBits != 0); }

    /**
         * @brief get the backend object handle
         *
         * @return the backend object handle.
         */
    virtual QDemonRenderBackend::QDemonRenderBackendRenderTargetObject GetFrameBuffertHandle()
    {
        return m_BufferHandle;
    }

    // this will be obsolete
    const void *GetImplementationHandle() const override
    {
        return reinterpret_cast<const void *>(m_BufferHandle);
    }

    /**
         * @brief static creator function
         *
         * @param[in] context		Pointer to context
         *
         * @return a pointer to framebuffer object.
         */
    static QSharedPointer<QDemonRenderFrameBuffer> Create(QDemonRenderContextImpl &context);

private:
    /**
         * @brief releaes an attached object
         *
         * @return which target we released
         */
    QDemonRenderTextureTargetType::Enum releaseAttachment(QDemonRenderFrameBufferAttachments::Enum idx);

    quint32 m_AttachmentBits; ///< holds flags for current attached buffers
};

QT_END_NAMESPACE

#endif
