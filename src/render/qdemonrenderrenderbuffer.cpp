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

#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderRenderBuffer::QDemonRenderRenderBuffer(QDemonRenderContextImpl &context,
                                                   QDemonRenderRenderBufferFormats::Enum format,
                                                   quint32 width, quint32 height)
    : m_Context(context)
    , m_Backend(context.GetBackend())
    , m_Width(width)
    , m_Height(height)
    , m_StorageFormat(format)
    , m_BufferHandle(nullptr)
{
    SetDimensions(QDemonRenderRenderBufferDimensions(width, height));
}

QDemonRenderRenderBuffer::~QDemonRenderRenderBuffer()
{
    m_Context.RenderBufferDestroyed(this);
    m_Backend->ReleaseRenderbuffer(m_BufferHandle);
    m_BufferHandle = nullptr;
}

void QDemonRenderRenderBuffer::SetDimensions(const QDemonRenderRenderBufferDimensions &inDimensions)
{
    quint32 maxWidth, maxHeight;
    m_Width = inDimensions.m_Width;
    m_Height = inDimensions.m_Height;

    // get max size and clamp to max value
    m_Context.getMaxTextureSize(maxWidth, maxHeight);
    if (m_Width > maxWidth || m_Height > maxHeight) {
        qCCritical(INVALID_OPERATION, "Width or height is greater than max texture size (%d, %d)",
                   maxWidth, maxHeight);
        m_Width = qMin(m_Width, maxWidth);
        m_Height = qMin(m_Height, maxHeight);
    }

    bool success = true;

    if (m_BufferHandle == nullptr)
        m_BufferHandle = m_Backend->CreateRenderbuffer(m_StorageFormat, m_Width, m_Height);
    else
        success =
                m_Backend->ResizeRenderbuffer(m_BufferHandle, m_StorageFormat, m_Width, m_Height);

    if (m_BufferHandle == nullptr || !success) {
        // We could try smaller sizes
        Q_ASSERT(false);
        qCCritical(INTERNAL_ERROR, "Unable to create render buffer %s, %dx%d",
                   QDemonRenderRenderBufferFormats::toString(m_StorageFormat), m_Width,
                   m_Height);
    }
}



QSharedPointer<QDemonRenderRenderBuffer>
QDemonRenderRenderBuffer::Create(QDemonRenderContextImpl &context,
                                 QDemonRenderRenderBufferFormats::Enum format, quint32 width,
                                 quint32 height)
{
    QSharedPointer<QDemonRenderRenderBuffer> retval = nullptr;
    if (width == 0 || height == 0) {
        qCCritical(INVALID_PARAMETER, "Invalid renderbuffer width or height");
        return retval;
    }

    retval.reset(new QDemonRenderRenderBuffer(context, format, width, height));

    return retval;
}

QT_END_NAMESPACE
