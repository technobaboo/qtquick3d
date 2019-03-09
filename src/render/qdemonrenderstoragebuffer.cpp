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

#include <QtDemonRender/qdemonrenderstoragebuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderStorageBuffer::QDemonRenderStorageBuffer(const QDemonRef<QDemonRenderContext> &context,
                                                     const QByteArray &bufferName,
                                                     size_t size,
                                                     QDemonRenderBufferUsageType usageType,
                                                     QDemonDataRef<quint8> data,
                                                     QDemonRenderDataBuffer *pBuffer)
    : QDemonRenderDataBuffer(context, size, QDemonRenderBufferBindType::Storage, usageType, data)
    , m_name(bufferName)
    , m_wrappedBuffer(pBuffer)
    , m_dirty(true)
{
    Q_ASSERT(context->isStorageBufferSupported());
}

QDemonRenderStorageBuffer::~QDemonRenderStorageBuffer()
{

    m_context->bufferDestroyed(this);
}

void QDemonRenderStorageBuffer::bind()
{
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    if (m_wrappedBuffer)
        m_wrappedBuffer->bind();
    else
        m_backend->bindBuffer(m_bufferHandle, m_bindFlags);
}

void QDemonRenderStorageBuffer::bindToShaderProgram(quint32 index)
{
    m_backend->programSetStorageBuffer(index, (m_wrappedBuffer) ? m_wrappedBuffer->handle() : m_bufferHandle);
}

void QDemonRenderStorageBuffer::update()
{
    // we only update the buffer if it is dirty and we actually have some data
    if (m_dirty && m_bufferData.size()) {
        m_backend->updateBuffer(m_bufferHandle, m_bindFlags, m_bufferData.size(), m_usageType, m_bufferData.begin());
        m_dirty = false;
    }
}

void QDemonRenderStorageBuffer::updateData(qint32 offset, QDemonDataRef<quint8> data)
{
    // we only update the buffer if it is not just a wrapper
    if (!m_wrappedBuffer)
        m_backend->updateBuffer(m_bufferHandle, m_bindFlags, data.size(), m_usageType, data.begin() + offset);
}

QDemonRef<QDemonRenderStorageBuffer> QDemonRenderStorageBuffer::create(const QDemonRef<QDemonRenderContext> &context,
                                                                       const char *bufferName,
                                                                       QDemonRenderBufferUsageType usageType,
                                                                       size_t size,
                                                                       QDemonConstDataRef<quint8> bufferData,
                                                                       QDemonRenderDataBuffer *pBuffer)
{
    QDemonRef<QDemonRenderStorageBuffer> retval = nullptr;

    if (context->isStorageBufferSupported()) {
        retval = new QDemonRenderStorageBuffer(context,
                                               bufferName,
                                               size,
                                               usageType,
                                               toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size()),
                                               pBuffer);
    } else {
        QString errorMsg = QObject::tr("Shader storage buffers are not supported: %1").arg(QString::fromUtf8(bufferName));
        qCCritical(INVALID_OPERATION) << errorMsg;
    }
    return retval;
}

QT_END_NAMESPACE
