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
    : m_context(context)
    , m_backend(context->getBackend())
    , m_usageType(usageType)
    , m_bindFlags(bindFlags)
    , m_bufferData(data)
    , m_bufferCapacity(data.size())
    , m_bufferSize(size)
    , m_ownsData(false)
    , m_mapped(false)
{
    m_bufferHandle =
            m_backend->createBuffer(size, bindFlags, usageType, (const void *)m_bufferData.begin());
}

QDemonRenderDataBuffer::~QDemonRenderDataBuffer()
{
    if (m_bufferHandle) {
        m_backend->releaseBuffer(m_bufferHandle);
    }
    m_bufferHandle = 0;

    releaseMemory();
}

void QDemonRenderDataBuffer::releaseMemory()
{
    // chekc if we should release memory
    if (m_bufferData.size() && m_ownsData) {
        ::free(m_bufferData.begin());
    }

    m_bufferData = QDemonDataRef<quint8>();
    m_ownsData = false;
}

QDemonDataRef<quint8> QDemonRenderDataBuffer::mapBuffer()
{
    // don't map twice
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to map a mapped buffer");
        Q_ASSERT(false);
    }

    quint8 *pData = (quint8 *)m_backend->mapBuffer(
                m_bufferHandle, m_bindFlags, 0, m_bufferSize,
                QDemonRenderBufferAccessFlags(QDemonRenderBufferAccessTypeValues::Read
                                              | QDemonRenderBufferAccessTypeValues::Write));

    releaseMemory();
    m_bufferData = toDataRef(const_cast<quint8 *>(pData), (quint32)m_bufferSize);
    m_bufferCapacity = (quint32)m_bufferSize;
    m_ownsData = false;

    // currently we return a reference to the system memory
    m_mapped = true;
    return m_bufferData;
}

QDemonDataRef<quint8> QDemonRenderDataBuffer::mapBufferRange(size_t offset, size_t size,
                                                             QDemonRenderBufferAccessFlags flags)
{
    // don't map twice
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to map a mapped buffer");
        Q_ASSERT(false);
    }
    // don't map out of range
    if ((m_bufferSize < (offset + size)) || (size == 0)) {
        qCCritical(INVALID_OPERATION, "Attempting to map out of buffer range");
        Q_ASSERT(false);
    }

    quint8 *pData =
            (quint8 *)m_backend->mapBuffer(m_bufferHandle, m_bindFlags, offset, size, flags);

    releaseMemory();
    m_bufferData = toDataRef(const_cast<quint8 *>(pData), (quint32)size);
    m_bufferCapacity = (quint32)size;
    m_ownsData = false;

    // currently we return a reference to the system memory
    m_mapped = true;
    return m_bufferData;
}

void QDemonRenderDataBuffer::unmapBuffer()
{
    if (m_mapped) {
        // update hardware
        m_backend->unmapBuffer(m_bufferHandle, m_bindFlags);
        m_mapped = false;
        releaseMemory();
    }
}

void QDemonRenderDataBuffer::updateBuffer(QDemonConstDataRef<quint8> data, bool ownsMemory)
{
    // don't update a mapped buffer
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to update a mapped buffer");
        Q_ASSERT(false);
    }

    releaseMemory();

    m_bufferData = toDataRef(const_cast<quint8 *>(data.begin()), data.size());
    m_bufferCapacity = data.mSize;
    m_ownsData = ownsMemory;
    // update hardware
    m_backend->updateBuffer(m_bufferHandle, m_bindFlags, m_bufferCapacity, m_usageType,
                            (const void *)m_bufferData.begin());
}
QT_END_NAMESPACE
