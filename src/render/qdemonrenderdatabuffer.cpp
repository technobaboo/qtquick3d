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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrenderdatabuffer.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderDataBuffer::QDemonRenderDataBuffer(QSharedPointer<QDemonRenderContextImpl> context,
                                               size_t size, QDemonRenderBufferBindFlags bindFlags,
                                               QDemonRenderBufferUsageType::Enum usageType,
                                               QDemonDataRef<quint8> data)
    : m_Context(context)
    , m_Backend(context->GetBackend())
    , m_UsageType(usageType)
    , m_BindFlags(bindFlags)
    , m_BufferData(data)
    , m_BufferCapacity(data.size())
    , m_BufferSize(size)
    , m_OwnsData(false)
    , m_Mapped(false)
{
    m_BufferHandle =
            m_Backend->CreateBuffer(size, bindFlags, usageType, (const void *)m_BufferData.begin());
}

QDemonRenderDataBuffer::~QDemonRenderDataBuffer()
{
    if (m_BufferHandle) {
        m_Backend->ReleaseBuffer(m_BufferHandle);
    }
    m_BufferHandle = 0;

    releaseMemory();
}

void QDemonRenderDataBuffer::releaseMemory()
{
    // chekc if we should release memory
    if (m_BufferData.size() && m_OwnsData) {
        ::free(m_BufferData.begin());
    }

    m_BufferData = QDemonDataRef<quint8>();
    m_OwnsData = false;
}

QDemonDataRef<quint8> QDemonRenderDataBuffer::MapBuffer()
{
    // don't map twice
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to map a mapped buffer");
        Q_ASSERT(false);
    }

    quint8 *pData = (quint8 *)m_Backend->MapBuffer(
                m_BufferHandle, m_BindFlags, 0, m_BufferSize,
                QDemonRenderBufferAccessFlags(QDemonRenderBufferAccessTypeValues::Read
                                              | QDemonRenderBufferAccessTypeValues::Write));

    releaseMemory();
    m_BufferData = toDataRef(const_cast<quint8 *>(pData), (quint32)m_BufferSize);
    m_BufferCapacity = (quint32)m_BufferSize;
    m_OwnsData = false;

    // currently we return a reference to the system memory
    m_Mapped = true;
    return m_BufferData;
}

QDemonDataRef<quint8> QDemonRenderDataBuffer::MapBufferRange(size_t offset, size_t size,
                                                             QDemonRenderBufferAccessFlags flags)
{
    // don't map twice
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to map a mapped buffer");
        Q_ASSERT(false);
    }
    // don't map out of range
    if ((m_BufferSize < (offset + size)) || (size == 0)) {
        qCCritical(INVALID_OPERATION, "Attempting to map out of buffer range");
        Q_ASSERT(false);
    }

    quint8 *pData =
            (quint8 *)m_Backend->MapBuffer(m_BufferHandle, m_BindFlags, offset, size, flags);

    releaseMemory();
    m_BufferData = toDataRef(const_cast<quint8 *>(pData), (quint32)size);
    m_BufferCapacity = (quint32)size;
    m_OwnsData = false;

    // currently we return a reference to the system memory
    m_Mapped = true;
    return m_BufferData;
}

void QDemonRenderDataBuffer::UnmapBuffer()
{
    if (m_Mapped) {
        // update hardware
        m_Backend->UnmapBuffer(m_BufferHandle, m_BindFlags);
        m_Mapped = false;
        releaseMemory();
    }
}

void QDemonRenderDataBuffer::UpdateBuffer(QDemonConstDataRef<quint8> data, bool ownsMemory)
{
    // don't update a mapped buffer
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to update a mapped buffer");
        Q_ASSERT(false);
    }

    releaseMemory();

    m_BufferData = toDataRef(const_cast<quint8 *>(data.begin()), data.size());
    m_BufferCapacity = data.mSize;
    m_OwnsData = ownsMemory;
    // update hardware
    m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, m_BufferCapacity, m_UsageType,
                            (const void *)m_BufferData.begin());
}
QT_END_NAMESPACE
