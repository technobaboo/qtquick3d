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

QDemonRenderFrameBuffer::QDemonRenderFrameBuffer(QSharedPointer<QDemonRenderContextImpl> context)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_BufferHandle(nullptr)
    , m_AttachmentBits(0)
{
    m_BufferHandle = m_Backend->CreateRenderTarget();
    Q_ASSERT(m_BufferHandle);
}

QDemonRenderFrameBuffer::~QDemonRenderFrameBuffer()
{
    m_Context->FrameBufferDestroyed(this);
    m_Backend->ReleaseRenderTarget(m_BufferHandle);
    m_BufferHandle = 0;
    m_AttachmentBits = 0;

    // release attachments
    QDEMON_FOREACH(idx, (quint32)QDemonRenderFrameBufferAttachments::LastAttachment)
    {
        if ((QDemonRenderFrameBufferAttachments::Enum)idx
                != QDemonRenderFrameBufferAttachments::DepthStencil
                || m_Context->IsDepthStencilSupported())
            releaseAttachment((QDemonRenderFrameBufferAttachments::Enum)idx);
    }
}

inline void CheckAttachment(QSharedPointer<QDemonRenderContext> ctx,
                            QDemonRenderFrameBufferAttachments::Enum attachment)
{
#ifdef _DEBUG
    Q_ASSERT(attachment != QDemonRenderFrameBufferAttachments::DepthStencil
            || ctx->IsDepthStencilSupported());
#endif
    (void)ctx;
    (void)attachment;
}

QDemonRenderTextureTargetType::Enum
QDemonRenderFrameBuffer::releaseAttachment(QDemonRenderFrameBufferAttachments::Enum idx)
{
    QDemonRenderTextureTargetType::Enum target = QDemonRenderTextureTargetType::Unknown;

    QDemonRenderTextureOrRenderBuffer Attach = m_Attachments[idx];
    if (Attach.HasTexture2D()) {
        target = (Attach.GetTexture2D()->IsMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::Texture2D;
        //Attach.GetTexture2D()->release();
    } else if (Attach.HasTexture2DArray()) {
        target = (Attach.GetTexture2DArray()->IsMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::Texture2D_Array;
        //Attach.GetTexture2DArray()->release();
    } else if (Attach.HasTextureCube()) {
        target = (Attach.GetTextureCube()->IsMultisampleTexture())
                ? QDemonRenderTextureTargetType::Texture2D_MS
                : QDemonRenderTextureTargetType::TextureCube;
        //Attach.GetTextureCube()->release();
    } else if (Attach.HasRenderBuffer())
        //Attach.GetRenderBuffer()->release();

    CheckAttachment(m_Context, idx);
    m_Attachments[idx] = QDemonRenderTextureOrRenderBuffer();

    m_AttachmentBits &= ~(1 << idx);

    return target;
}

QDemonRenderTextureOrRenderBuffer
QDemonRenderFrameBuffer::GetAttachment(QDemonRenderFrameBufferAttachments::Enum attachment)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return QDemonRenderTextureOrRenderBuffer();
    }
    CheckAttachment(m_Context, attachment);
    return m_Attachments[attachment];
}

void QDemonRenderFrameBuffer::Attach(QDemonRenderFrameBufferAttachments::Enum attachment,
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
    if (!buffer.HasTexture2D() && !buffer.HasRenderBuffer() && !buffer.HasTexture2DArray()
            && !(m_AttachmentBits & (1 << attachment)))
        return;

    CheckAttachment(m_Context, attachment);
    // Ensure we are the bound framebuffer
    m_Context->SetRenderTarget(sharedFromThis());

    // release previous attachments
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

    if (buffer.HasTexture2D()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                          QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                          theRelTarget);

        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      buffer.GetTexture2D()->GetTextureObjectHandle(), target);
        //buffer.GetTexture2D()->addRef();
        m_AttachmentBits |= (1 << attachment);
    } else if (buffer.HasTexture2DArray()) {
        // On the same attachment point there could be a something attached with a different
        // target MSAA <--> NoMSAA
        if (theRelTarget != QDemonRenderTextureTargetType::Unknown && theRelTarget != target)
            m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                          QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                          theRelTarget);

        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      buffer.GetTexture2D()->GetTextureObjectHandle(), target);
        //buffer.GetTexture2DArray()->addRef();
        m_AttachmentBits |= (1 << attachment);
    } else if (buffer.HasRenderBuffer()) {
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      buffer.GetRenderBuffer()->GetRenderBuffertHandle());
        //buffer.GetRenderBuffer()->addRef();
        m_AttachmentBits |= (1 << attachment);
    } else if (theRelTarget == QDemonRenderTextureTargetType::Unknown) {
        // detach renderbuffer
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendRenderbufferObject(nullptr));
    } else {
        // detach texture
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);
    }
    m_Attachments[attachment] = buffer;
}

void QDemonRenderFrameBuffer::AttachLayer(QDemonRenderFrameBufferAttachments::Enum attachment,
                                          QDemonRenderTextureOrRenderBuffer buffer, qint32 layer,
                                          qint32 level)
{
    if (attachment == QDemonRenderFrameBufferAttachments::Unknown
            || attachment > QDemonRenderFrameBufferAttachments::LastAttachment) {
        qCCritical(INVALID_PARAMETER, "Attachment out of range");
        return;
    }

    // This function is only used for attaching a layer
    // If texture exists probably something is wrong
    if (!buffer.HasTexture2DArray()) {
        Q_ASSERT(false);
        return;
    }

    CheckAttachment(m_Context, attachment);
    // Ensure we are the bound framebuffer
    m_Context->SetRenderTarget(sharedFromThis());

    // release previous attachments
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

    // On the same attachment point there could be a something attached with a different target
    // MSAA <--> NoMSAA
    if (theRelTarget != QDemonRenderTextureTargetType::Unknown
            && theRelTarget != QDemonRenderTextureTargetType::Texture2D_Array)
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);

    m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                  buffer.GetTexture2DArray()->GetTextureObjectHandle(), level,
                                  layer);
    //buffer.GetTexture2DArray()->addRef();
    m_AttachmentBits |= (1 << attachment);

    m_Attachments[attachment] = buffer;
}

void QDemonRenderFrameBuffer::AttachFace(QDemonRenderFrameBufferAttachments::Enum attachment,
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

    CheckAttachment(m_Context, attachment);
    // Ensure we are the bound framebuffer
    m_Context->SetRenderTarget(sharedFromThis());

    // release previous attachments
    QDemonRenderTextureTargetType::Enum attachTarget = static_cast<QDemonRenderTextureTargetType::Enum>(
                (int)QDemonRenderTextureTargetType::TextureCube + (int)face);
    QDemonRenderTextureTargetType::Enum theRelTarget = releaseAttachment(attachment);

    // If buffer has no texture cube, this call is used to detach faces.
    // If release target is not cube, there is something else attached to that
    // attachment point, so we want to release that first. E.g (MSAA <--> NoMSAA)
    if (theRelTarget == QDemonRenderTextureTargetType::TextureCube && !buffer.HasTextureCube()) {
        theRelTarget = attachTarget;
        attachTarget = QDemonRenderTextureTargetType::Unknown;
    } else if (theRelTarget == QDemonRenderTextureTargetType::TextureCube) {
        theRelTarget = QDemonRenderTextureTargetType::Unknown;
    }
    if (theRelTarget != QDemonRenderTextureTargetType::Unknown) {
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      QDemonRenderBackend::QDemonRenderBackendTextureObject(nullptr),
                                      theRelTarget);
    }

    if (attachTarget != QDemonRenderTextureTargetType::Unknown) {
        m_Backend->RenderTargetAttach(m_BufferHandle, attachment,
                                      buffer.GetTextureCube()->GetTextureObjectHandle(),
                                      attachTarget);
        //buffer.GetTextureCube()->addRef();
        m_AttachmentBits |= (1 << attachment);
    }

    m_Attachments[attachment] = buffer;
}

bool QDemonRenderFrameBuffer::IsComplete()
{
    // Ensure we are the bound framebuffer
    m_Context->SetRenderTarget(sharedFromThis());

    return m_Backend->RenderTargetIsValid(m_BufferHandle);
}

QSharedPointer<QDemonRenderFrameBuffer> QDemonRenderFrameBuffer::Create(QSharedPointer<QDemonRenderContextImpl> context)
{
    return QSharedPointer<QDemonRenderFrameBuffer>(new QDemonRenderFrameBuffer(context));
}

QT_END_NAMESPACE
