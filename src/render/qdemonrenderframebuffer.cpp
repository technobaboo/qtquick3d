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

#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderFrameBuffer::QDemonRenderFrameBuffer(const QDemonRef<QDemonRenderContext> &context)
    : m_context(context), m_backend(context->backend()), m_bufferHandle(nullptr), m_attachmentBits(0)
{
    m_bufferHandle = m_backend->createRenderTarget();
    Q_ASSERT(m_bufferHandle);
}

QDemonRenderFrameBuffer::~QDemonRenderFrameBuffer()
{
    m_context->frameBufferDestroyed(this);
    m_backend->releaseRenderTarget(m_bufferHandle);
    m_bufferHandle = nullptr;
    m_attachmentBits = 0;

    // release attachments
    for (int idx = 0; idx != int(QDemonRenderFrameBufferAttachment::LastAttachment); ++idx) {
        if ((QDemonRenderFrameBufferAttachment)idx != QDemonRenderFrameBufferAttachment::DepthStencil
            || m_context->supportsDepthStencil())
            releaseAttachment((QDemonRenderFrameBufferAttachment)idx);
    }
}

inline void CheckAttachment(QDemonRef<QDemonRenderContext> ctx, QDemonRenderFrameBufferAttachment attachment)
{
#ifdef _DEBUG
    Q_ASSERT(attachment != QDemonRenderFrameBufferAttachment::DepthStencil || ctx->isDepthStencilSupported());
#endif
    (void)ctx;
    (void)attachment;
}

QDemonRenderTextureTargetType QDemonRenderFrameBuffer::releaseAttachment(QDemonRenderFrameBufferAttachment idx)
{
    QDemonRenderTextureTargetType target = QDemonRenderTextureTargetType::Unknown;
    int index = static_cast<int>(idx);

    QDemonRenderTextureOrRenderBuffer attachment = m_attachments[index];
    if (attachment.hasTexture2D()) {
        target = (attachment.texture2D()->isMultisampleTexture()) ? QDemonRenderTextureTargetType::Texture2D_MS
                                                                 : QDemonRenderTextureTargetType::Texture2D;
        // Attach.GetTexture2D()->release();
    } else if (attachment.hasTexture2DArray()) {
        target = (attachment.texture2DArray()->isMultisampleTexture()) ? QDemonRenderTextureTargetType::Texture2D_MS
                                                                      : QDemonRenderTextureTargetType::Texture2D_Array;
        // Attach.GetTexture2DArray()->release();
    } else if (attachment.hasTextureCube()) {
        target = (attachment.textureCube()->isMultisampleTexture()) ? QDemonRenderTextureTargetType::Texture2D_MS
                                                                   : QDemonRenderTextureTargetType::TextureCube;
        // Attach.GetTextureCube()->release();
    } else if (attachment.hasRenderBuffer())
        // Attach.GetRenderBuffer()->release();

        CheckAttachment(m_context, idx);
    m_attachments[index] = QDemonRenderTextureOrRenderBuffer();

    m_attachmentBits &= ~(1 << index);

    return target;
}

QDemonRenderTextureOrRenderBuffer QDemonRenderFrameBuffer::attachment(QDemonRenderFrameBufferAttachment attachment)
{
    if (attachment == QDemonRenderFrameBufferAttachment::Unknown || attachment > QDemonRenderFrameBufferAttachment::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return QDemonRenderTextureOrRenderBuffer();
    }
    CheckAttachment(m_context, attachment);
    return m_attachments[static_cast<int>(attachment)];
}

void QDemonRenderFrameBuffer::attach(QDemonRenderFrameBufferAttachment attachment,
                                     const QDemonRenderTextureOrRenderBuffer &buffer,
                                     QDemonRenderTextureTargetType target)
{
    if (attachment == QDemonRenderFrameBufferAttachment::Unknown || attachment > QDemonRenderFrameBufferAttachment::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    quint32 attachmentBit = (1 << static_cast<int>(attachment));

    // early out
    // if there is nothing to detach
    if (!buffer.hasTexture2D() && !buffer.hasRenderBuffer() && !buffer.hasTexture2DArray() && !(m_attachmentBits & attachmentBit))
        return;

    CheckAttachment(m_context, attachment);
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    // release previous attachments
    QDemonRenderTextureTargetType theRelTarget = releaseAttachment(attachment);

    if (buffer.hasTexture2D()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr), theRelTarget);

        m_backend->renderTargetAttach(m_bufferHandle, attachment, buffer.texture2D()->handle(), target);
        // buffer.GetTexture2D()->addRef();
        m_attachmentBits |= attachmentBit;
    } else if (buffer.hasTexture2DArray()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr), theRelTarget);

        m_backend->renderTargetAttach(m_bufferHandle, attachment, buffer.texture2D()->handle(), target);
        // buffer.GetTexture2DArray()->addRef();
        m_attachmentBits |= attachmentBit;
    } else if (buffer.hasRenderBuffer()) {
        m_backend->renderTargetAttach(m_bufferHandle, attachment, buffer.renderBuffer()->handle());
        // buffer.GetRenderBuffer()->addRef();
        m_attachmentBits |= attachmentBit;
    } else if (theRelTarget == QDemonRenderTextureTargetType::Unknown) {
        // detach renderbuffer
        m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendRenderbufferObject(nullptr));
    } else {
        // detach texture
        m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr), theRelTarget);
    }
    m_attachments[static_cast<int>(attachment)] = buffer;
}

void QDemonRenderFrameBuffer::attachLayer(QDemonRenderFrameBufferAttachment attachment,
                                          const QDemonRenderTextureOrRenderBuffer &buffer,
                                          qint32 layer,
                                          qint32 level)
{
    if (attachment == QDemonRenderFrameBufferAttachment::Unknown || attachment > QDemonRenderFrameBufferAttachment::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    // This function is only used for attaching a layer
    // If texture exists probably something is wrong
    if (!buffer.hasTexture2DArray()) {
        Q_ASSERT(false);
        return;
    }

    CheckAttachment(m_context, attachment);
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    // release previous attachments
    QDemonRenderTextureTargetType theRelTarget = releaseAttachment(attachment);

    // On the same attachment point there could be a something attached with a different target
    // MSAA <--> NoMSAA
    if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != QDemonRenderTextureTargetType::Texture2D_Array)
        m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr), theRelTarget);

    m_backend->renderTargetAttach(m_bufferHandle, attachment, buffer.texture2DArray()->handle(), level, layer);
    // buffer.GetTexture2DArray()->addRef();
    m_attachmentBits |= (1 << static_cast<int>(attachment));

    m_attachments[static_cast<int>(attachment)] = buffer;
}

void QDemonRenderFrameBuffer::attachFace(QDemonRenderFrameBufferAttachment attachment,
                                         const QDemonRenderTextureOrRenderBuffer &buffer,
                                         QDemonRenderTextureCubeFace face)
{
    if (attachment == QDemonRenderFrameBufferAttachment::Unknown || attachment > QDemonRenderFrameBufferAttachment::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    if (face == QDemonRenderTextureCubeFace::InvalidFace) {
        Q_ASSERT(false);
        return;
    }

    CheckAttachment(m_context, attachment);
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    // release previous attachments
    QDemonRenderTextureTargetType attachTarget = static_cast<QDemonRenderTextureTargetType>(
            (int)QDemonRenderTextureTargetType::TextureCube + (int)face);
    QDemonRenderTextureTargetType theRelTarget = releaseAttachment(attachment);

    // If buffer has no texture cube, this call is used to detach faces.
    // If release target is not cube, there is something else attached to that
    // attachment point, so we want to release that first. E.g (MSAA <--> NoMSAA)
    if (theRelTarget == QDemonRenderTextureTargetType::TextureCube && !buffer.hasTextureCube()) {
        theRelTarget = attachTarget;
        attachTarget = QDemonRenderTextureTargetType::Unknown;
    } else if (theRelTarget == QDemonRenderTextureTargetType::TextureCube) {
        theRelTarget = QDemonRenderTextureTargetType::Unknown;
    }
    if (theRelTarget != QDemonRenderTextureTargetType::Unknown) {
        m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr), theRelTarget);
    }

    if (attachTarget != QDemonRenderTextureTargetType::Unknown) {
        m_backend->renderTargetAttach(m_bufferHandle, attachment, buffer.textureCube()->handle(), attachTarget);
        // buffer.GetTextureCube()->addRef();
        m_attachmentBits |= (1 << static_cast<int>(attachment));
    }

    m_attachments[static_cast<int>(attachment)] = buffer;
}

bool QDemonRenderFrameBuffer::isComplete()
{
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    return m_backend->renderTargetIsValid(m_bufferHandle);
}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTexture2D> texture)
    : m_texture2D(texture)
{
}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderRenderBuffer> render)
    : m_renderBuffer(render)
{
}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTexture2DArray> textureArray)
    : m_texture2DArray(textureArray)
{
}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(QDemonRef<QDemonRenderTextureCube> textureCube)
    : m_textureCube(textureCube)
{
}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer() = default;

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(const QDemonRenderTextureOrRenderBuffer &other) = default;

QDemonRenderTextureOrRenderBuffer::~QDemonRenderTextureOrRenderBuffer() = default;


QDemonRenderTextureOrRenderBuffer &QDemonRenderTextureOrRenderBuffer::operator=(const QDemonRenderTextureOrRenderBuffer &other)
{
    if (this != &other) {
        m_texture2D = QDemonRef<QDemonRenderTexture2D>(other.m_texture2D);
        m_texture2DArray = QDemonRef<QDemonRenderTexture2DArray>(other.m_texture2DArray);
        m_renderBuffer = QDemonRef<QDemonRenderRenderBuffer>(other.m_renderBuffer);
        m_textureCube = QDemonRef<QDemonRenderTextureCube>(other.m_textureCube);
    }
    return *this;
}

QDemonRef<QDemonRenderTexture2D> QDemonRenderTextureOrRenderBuffer::texture2D() const
{
    Q_ASSERT(hasTexture2D());
    return m_texture2D;
}

QDemonRef<QDemonRenderTexture2DArray> QDemonRenderTextureOrRenderBuffer::texture2DArray() const
{
    Q_ASSERT(hasTexture2DArray());
    return m_texture2DArray;
}

QDemonRef<QDemonRenderTextureCube> QDemonRenderTextureOrRenderBuffer::textureCube() const
{
    Q_ASSERT(hasTextureCube());
    return m_textureCube;
}

QDemonRef<QDemonRenderRenderBuffer> QDemonRenderTextureOrRenderBuffer::renderBuffer() const
{
    Q_ASSERT(hasRenderBuffer());
    return m_renderBuffer;
}

QT_END_NAMESPACE
