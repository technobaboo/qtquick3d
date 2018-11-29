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

QDemonRenderStorageBuffer::QDemonRenderStorageBuffer(QDemonRenderContextImpl &context,
                                                     const QString &bufferName, size_t size,
                                                     QDemonRenderBufferUsageType::Enum usageType,
                                                     QDemonDataRef<quint8> data, QDemonRenderDataBuffer *pBuffer)
    : QDemonRenderDataBuffer(context, size,
                             QDemonRenderBufferBindValues::Storage, usageType, data)
    , m_Name(bufferName)
    , m_WrappedBuffer(pBuffer)
    , m_Dirty(true)
{
    Q_ASSERT(context.IsStorageBufferSupported());

//    if (pBuffer)
//        pBuffer->addRef();
}

QDemonRenderStorageBuffer::~QDemonRenderStorageBuffer()
{
//    if (m_WrappedBuffer)
//        m_WrappedBuffer->release();

    m_Context.BufferDestroyed(*this);
}

void QDemonRenderStorageBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    if (m_WrappedBuffer)
        m_WrappedBuffer->Bind();
    else
        m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

void QDemonRenderStorageBuffer::BindToShaderProgram(quint32 index)
{
    m_Backend->ProgramSetStorageBuffer(
                index, (m_WrappedBuffer) ? m_WrappedBuffer->GetBuffertHandle() : m_BufferHandle);
}

void QDemonRenderStorageBuffer::Update()
{
    // we only update the buffer if it is dirty and we actually have some data
    if (m_Dirty && m_BufferData.size()) {
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, m_BufferData.size(), m_UsageType,
                                m_BufferData.begin());
        m_Dirty = false;
    }
}

void QDemonRenderStorageBuffer::UpdateData(qint32 offset, QDemonDataRef<quint8> data)
{
    // we only update the buffer if it is not just a wrapper
    if (!m_WrappedBuffer)
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, data.size(), m_UsageType,
                                data.begin() + offset);
}

QDemonRenderStorageBuffer *
QDemonRenderStorageBuffer::Create(QDemonRenderContextImpl &context, const char *bufferName,
                                  QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                  QDemonConstDataRef<quint8> bufferData, QDemonRenderDataBuffer *pBuffer)
{
    QDemonRenderStorageBuffer *retval = nullptr;

    if (context.IsStorageBufferSupported()) {
        const QString theBufferName = QString::fromLocal8Bit(bufferName);
        quint32 cbufSize = sizeof(QDemonRenderStorageBuffer);
        quint8 *newMem = static_cast<quint8 *>(::malloc(cbufSize));
        retval = new (newMem) QDemonRenderStorageBuffer(
                    context, theBufferName, size, usageType,
                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size()), pBuffer);
    } else {
        QString errorMsg = QObject::tr("Shader storage buffers are not supported: %1")
                .arg(bufferName);
        qCCritical(INVALID_OPERATION) << errorMsg;
    }
    return retval;
}

QT_END_NAMESPACE
