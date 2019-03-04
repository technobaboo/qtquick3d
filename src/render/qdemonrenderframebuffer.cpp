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

QDemonRenderFrameBuffer::QDemonRenderFrameBuffer(const QDemonRef<QDemonRenderContextImpl> &context)
    : m_context(context)
    , m_backend(context->getBackend())
    , m_bufferHandle(nullptr)
    , m_attachmentBits(0)
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
    QDEMON_FOREACH(idx, (quint32)QDemonRenderFrameBufferAttachments::LastAttachment)
    {
        if ((QDemonRenderFrameBufferAttachments::Enum)idx
                != QDemonRenderFrameBufferAttachments::DepthStencil
                || m_context->isDepthStencilSupported())
            releaseAttachment((QDemonRenderFrameBufferAttachments::Enum)idx);
    }
}

inline void CheckAttachment(QDemonRef<QDemonRenderContext> ctx,
                            QDemonRenderFrameBufferAttachments::Enum attachment)
{
#ifdef _DEBUG
    Q_ASSERT(attachment != QDemonRenderFrameBufferAttachments::DepthStencil
            || ctx->isDepthStencilSupported());
#endif
    (void)ctx;
    (void)attachment;
}

QDemonRenderTextureTargetType::Enum
QDemonRenderFrameBuffer::releaseAttachment(QDemonRenderFrameBufferAttachments::Enum idx)
{
    QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Unknown;

    QDemonRenderTextureOrRenderBuffer Attach = m_attachments[idx];
    if (Attach.hasTexture2D()) {
        target = (Attach.getTexture2D()->isMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::Texture2D;
        //Attach.GetTexture2D()->release();
    } else if (Attach.hasTexture2DArray()) {
        target = (Attach.getTexture2DArray()->isMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::Texture2D_Array;
        //Attach.GetTexture2DArray()->release();
    } else if (Attach.hasTextureCube()) {
        target = (Attach.getTextureCube()->isMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::TextureCube;
        //Attach.GetTextureCube()->release();
    } else if (Attach.hasRenderBuffer())
        //Attach.GetRenderBuffer()->release();

    CheckAttachment(m_context, idx);
    m_attachments[idx] = QDemonRenderTextureOrRenderBuffer();

    m_attachmentBits &= ~(1 << idx);

    return target;
}

QDemonRenderTextureOrRenderBuffer
QDemonRenderFrameBuffer::getAttachment(QDemonRenderFrameBufferAttachments::Enum attachment)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return QDemonRenderTextureOrRenderBuffer();
    }
    CheckAttachment(m_context, attachment);
    return m_attachments[attachment];
}

void QDemonRenderFrameBuffer::attach(QDemonRenderFrameBufferAttachments::Enum attachment,
                                     QDemonRenderTextureOrRenderBuffer buffer,
                                     QDemonRenderTextureTargetType::Enum target)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    // early out
    // if there is nothing to detach
    if (!buffer.hasTexture2D() && !buffer.hasRenderBuffer() && !buffer.hasTexture2DArray()
            && !(m_attachmentBits & (1 << attachment)))
        return;

    CheckAttachment(m_context, attachment);
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    // release previous attachments
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

    if (buffer.hasTexture2D()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                          QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                          theRelTarget);

        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      buffer.getTexture2D()->getTextureObjectHandle(), target);
        //buffer.GetTexture2D()->addRef();
        m_attachmentBits |= (1 << attachment);
    } else if (buffer.hasTexture2DArray()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                          QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                          theRelTarget);

        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      buffer.getTexture2D()->getTextureObjectHandle(), target);
        //buffer.GetTexture2DArray()->addRef();
        m_attachmentBits |= (1 << attachment);
    } else if (buffer.hasRenderBuffer()) {
        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      buffer.getRenderBuffer()->getRenderBuffertHandle());
        //buffer.GetRenderBuffer()->addRef();
        m_attachmentBits |= (1 << attachment);
    } else if (theRelTarget == QDemonRenderTextureTargetType::Unknown) {
        // detach renderbuffer
        m_backend->renderTargetAttach(m_bufferHandle, attachment, QDemonRenderBackend::QDemonRenderBackendRenderbufferObject(nullptr));
    } else {
        // detach texture
        m_backend->renderTargetAttach(m_bufferHandle,
                                      attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);
    }
    m_attachments[attachment] = buffer;
}

void QDemonRenderFrameBuffer::attachLayer(QDemonRenderFrameBufferAttachments::Enum attachment,
                                          QDemonRenderTextureOrRenderBuffer buffer,
                                          qint32 layer,
                                          qint32 level)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
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
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

    // On the same attachment point there could be a something attached with a different target
    // MSAA <--> NoMSAA
    if (theRelTarget != QDemonRenderTextureTargetType::Unknown
            && theRelTarget != QDemonRenderTextureTargetType::Texture2D_Array)
        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);

    m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                  buffer.getTexture2DArray()->getTextureObjectHandle(), level,
                                  layer);
    //buffer.GetTexture2DArray()->addRef();
    m_attachmentBits |= (1 << attachment);

    m_attachments[attachment] = buffer;
}

void QDemonRenderFrameBuffer::attachFace(QDemonRenderFrameBufferAttachments::Enum attachment,
                                         QDemonRenderTextureOrRenderBuffer buffer,
                                         QDemonRenderTextureCubeFaces::Enum face)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    if (face == QDemonRenderTextureCubeFaces::InvalidFace) {
        Q_ASSERT(false);
        return;
    }

    CheckAttachment(m_context, attachment);
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    // release previous attachments
    QDemonRenderTextureTargetType::Enum attachTarget = static_cast<QDemonRenderTextureTargetType::Enum>(
                (int)QDemonRenderTextureTargetType::TextureCube + (int)face);
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

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
        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);
    }

    if (attachTarget != QDemonRenderTextureTargetType::Unknown) {
        m_backend->renderTargetAttach(m_bufferHandle, attachment,
                                      buffer.getTextureCube()->getTextureObjectHandle(),
                                      attachTarget);
        //buffer.GetTextureCube()->addRef();
        m_attachmentBits |= (1 << attachment);
    }

    m_attachments[attachment] = buffer;
}

bool QDemonRenderFrameBuffer::isComplete()
{
    // Ensure we are the bound framebuffer
    m_context->setRenderTarget(this);

    return m_backend->renderTargetIsValid(m_bufferHandle);
}

QDemonRef<QDemonRenderFrameBuffer> QDemonRenderFrameBuffer::create(const QDemonRef<QDemonRenderContextImpl> &context)
{
    return QDemonRef<QDemonRenderFrameBuffer>(new QDemonRenderFrameBuffer(context));
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

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer()
{

}

QDemonRenderTextureOrRenderBuffer::QDemonRenderTextureOrRenderBuffer(const QDemonRenderTextureOrRenderBuffer &other)
    : m_texture2D(other.m_texture2D)
    , m_texture2DArray(other.m_texture2DArray)
    , m_textureCube(other.m_textureCube)
    , m_renderBuffer(other.m_renderBuffer)
{
}

QDemonRenderTextureOrRenderBuffer::~QDemonRenderTextureOrRenderBuffer()
{

}

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

QDemonRef<QDemonRenderTexture2D> QDemonRenderTextureOrRenderBuffer::getTexture2D() const
{
    Q_ASSERT(hasTexture2D());
    return m_texture2D;
}

QDemonRef<QDemonRenderTexture2DArray> QDemonRenderTextureOrRenderBuffer::getTexture2DArray() const
{
    Q_ASSERT(hasTexture2DArray());
    return m_texture2DArray;
}

QDemonRef<QDemonRenderTextureCube> QDemonRenderTextureOrRenderBuffer::getTextureCube() const
{
    Q_ASSERT(hasTextureCube());
    return m_textureCube;
}

QDemonRef<QDemonRenderRenderBuffer> QDemonRenderTextureOrRenderBuffer::getRenderBuffer() const
{
    Q_ASSERT(hasRenderBuffer());
    return m_renderBuffer;
}


QT_END_NAMESPACE
