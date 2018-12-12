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

#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderVertexBuffer::QDemonRenderVertexBuffer(QDemonRenderContextImpl &context, size_t size,
                                                   quint32 stride, QDemonRenderBufferBindFlags bindFlags,
                                                   QDemonRenderBufferUsageType::Enum usageType,
                                                   QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size, bindFlags, usageType, data)
    , m_Stride(stride)
{
    Q_ASSERT(m_Stride);
}

QDemonRenderVertexBuffer::~QDemonRenderVertexBuffer()
{
    m_Context.BufferDestroyed(this);
}

void QDemonRenderVertexBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

QSharedPointer<QDemonRenderVertexBuffer> QDemonRenderVertexBuffer::Create(QDemonRenderContextImpl &context,
                                                                          QDemonRenderBufferUsageType::Enum usageType,
                                                                          size_t size, quint32 stride,
                                                                          QDemonConstDataRef<quint8> bufferData)
{
    quint32 vbufSize = sizeof(QDemonRenderVertexBuffer);
    quint8 *newMem = static_cast<quint8 *>(::malloc(vbufSize));
    return QSharedPointer<QDemonRenderVertexBuffer>(new (newMem) QDemonRenderVertexBuffer(
                                                        context, size, stride, QDemonRenderBufferBindValues::Vertex, usageType,
                                                        toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size())));
}

QT_END_NAMESPACE
