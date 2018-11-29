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

#include <QtDemonRender/qdemonrenderdrawindirectbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemon/qdemonutils.h>


QT_BEGIN_NAMESPACE
QDemonRenderDrawIndirectBuffer::QDemonRenderDrawIndirectBuffer(QDemonRenderContextImpl &context,
                                                               size_t size,
                                                               QDemonRenderBufferUsageType::Enum usageType,
                                                               QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size,
                             QDemonRenderBufferBindValues::Draw_Indirect, usageType, data)
    , m_Dirty(true)
{
}

QDemonRenderDrawIndirectBuffer::~QDemonRenderDrawIndirectBuffer() { m_Context.BufferDestroyed(*this); }

void QDemonRenderDrawIndirectBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

void QDemonRenderDrawIndirectBuffer::Update()
{
    // we only update the buffer if it is dirty and we actually have some data
    if (m_Dirty && m_BufferData.size()) {
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, m_BufferData.size(), m_UsageType,
                                m_BufferData.begin());
        m_Dirty = false;
    }
}

void QDemonRenderDrawIndirectBuffer::UpdateData(qint32 offset, QDemonDataRef<quint8> data)
{
    // we only update the buffer if we something
    if (data.size())
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, data.size(), m_UsageType,
                                data.begin() + offset);
}

QDemonRenderDrawIndirectBuffer *
QDemonRenderDrawIndirectBuffer::Create(QDemonRenderContextImpl &context,
                                       QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                       QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderDrawIndirectBuffer *retval = nullptr;

    // these are the context flags which do not support this drawing mode
    QDemonRenderContextType noDrawIndirectSupported(
                QDemonRenderContextValues::GL2 | QDemonRenderContextValues::GLES2 | QDemonRenderContextValues::GL3
                | QDemonRenderContextValues::GLES3);
    QDemonRenderContextType ctxType = context.GetRenderContextType();

    if (!(ctxType & noDrawIndirectSupported)) {
        quint32 bufSize = sizeof(QDemonRenderDrawIndirectBuffer);
        quint8 *newMem = static_cast<quint8 *>(::malloc(bufSize));
        retval = new (newMem) QDemonRenderDrawIndirectBuffer(
                    context, size, usageType,
                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size()));
    } else {
        Q_ASSERT(false);
    }
    return retval;
}
QT_END_NAMESPACE
