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

#include <QtDemonRender/qdemonrenderinputassembler.h>
#include <QtDemonRender/qdemonrenderattriblayout.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

///< constructor
QDemonRenderInputAssembler::QDemonRenderInputAssembler(QSharedPointer<QDemonRenderContextImpl> context, QSharedPointer<QDemonRenderAttribLayout> attribLayout,
        QDemonConstDataRef<QSharedPointer<QDemonRenderVertexBuffer> > buffers, const QSharedPointer<QDemonRenderIndexBuffer> indexBuffer,
        QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets,
        QDemonRenderDrawMode::Enum primType, quint32 patchVertexCount)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_AttribLayout(attribLayout)
    , m_IndexBuffer(indexBuffer)
    , m_PrimitiveType(primType)
    , m_PatchVertexCount(patchVertexCount)
{
    // we cannot currently attach more than 16  vertex buffers
    Q_ASSERT(buffers.size() < 16);
    // if primitive is "Patch" we need a patch per vertex count > 0
    Q_ASSERT(m_PrimitiveType != QDemonRenderDrawMode::Patches || m_PatchVertexCount > 1);

    quint32 entrySize = sizeof(QDemonRenderBackend::QDemonRenderBackendBufferObject) * buffers.size();
    QDemonRenderBackend::QDemonRenderBackendBufferObject *bufferHandle = static_cast<QDemonRenderBackend::QDemonRenderBackendBufferObject *>(::malloc(entrySize));
    // setup vertex buffer backend handle array
    QDEMON_FOREACH(idx, buffers.size())
    {
        m_VertexBuffers.push_back(buffers.mData[idx]);
        bufferHandle[idx] = buffers.mData[idx]->GetBuffertHandle();
    };

    m_VertexbufferHandles = toConstDataRef(bufferHandle, buffers.size());

    m_InputAssemblertHandle = m_Backend->CreateInputAssembler(
                m_AttribLayout->GetAttribLayoutHandle(), m_VertexbufferHandles,
                (m_IndexBuffer) ? m_IndexBuffer->GetBuffertHandle() : nullptr, strides, offsets,
                patchVertexCount);

    //attribLayout->addRef();
}

///< destructor
QDemonRenderInputAssembler::~QDemonRenderInputAssembler()
{
    //m_AttribLayout->release();

    if (m_InputAssemblertHandle) {
        m_Backend->ReleaseInputAssembler(m_InputAssemblertHandle);
    }
    // ### sketchy
    ::free(const_cast<QDemonRenderBackend::QDemonRenderBackendBufferObject *>(m_VertexbufferHandles.mData));
}

quint32 QDemonRenderInputAssembler::GetIndexCount() const
{
    return (m_IndexBuffer) ? m_IndexBuffer->GetNumIndices() : 0;
}

quint32 QDemonRenderInputAssembler::GetVertexCount() const
{
    // makes only sense if we have a single vertex buffer
    Q_ASSERT(m_VertexBuffers.size() == 1);

    return m_VertexBuffers[0]->GetNumVertexes();
}

QT_END_NAMESPACE
