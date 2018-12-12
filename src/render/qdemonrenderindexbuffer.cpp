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

#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

QDemonRenderIndexBuffer::QDemonRenderIndexBuffer(QDemonRenderContextImpl &context, size_t size,
                                                 QDemonRenderComponentTypes::Enum componentType,
                                                 QDemonRenderBufferUsageType::Enum usageType,
                                                 QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size,
                             QDemonRenderBufferBindValues::Index, usageType, data)
    , m_ComponentType(componentType)
{
}

QDemonRenderIndexBuffer::~QDemonRenderIndexBuffer()
{
    m_Context.BufferDestroyed(this);
}

quint32 QDemonRenderIndexBuffer::GetNumIndices() const
{
    quint32 dtypeSize = QDemonRenderComponentTypes::getSizeofType(m_ComponentType);
    return m_BufferCapacity / dtypeSize;
}

void QDemonRenderIndexBuffer::Draw(QDemonRenderDrawMode::Enum drawMode, quint32 count, quint32 offset)
{
    m_Backend->DrawIndexed(
                drawMode, count, m_ComponentType,
                (const void *)(offset * QDemonRenderComponentTypes::getSizeofType(m_ComponentType)));
}

void QDemonRenderIndexBuffer::DrawIndirect(QDemonRenderDrawMode::Enum drawMode, quint32 offset)
{
    m_Backend->DrawIndexedIndirect(drawMode, m_ComponentType, (const void *)offset);
}

void QDemonRenderIndexBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

QSharedPointer<QDemonRenderIndexBuffer> QDemonRenderIndexBuffer::Create(QDemonRenderContextImpl &context,
                                                                        QDemonRenderBufferUsageType::Enum usageType,
                                                                        QDemonRenderComponentTypes::Enum componentType,
                                                                        size_t size, QDemonConstDataRef<quint8> bufferData)
{
    if (componentType != QDemonRenderComponentTypes::UnsignedInteger32
            && componentType != QDemonRenderComponentTypes::UnsignedInteger16
            && componentType != QDemonRenderComponentTypes::UnsignedInteger8) {
        qCCritical(INVALID_PARAMETER, "Invalid component type for index buffer");
        Q_ASSERT(false);
        return nullptr;
    }

    quint32 ibufSize = sizeof(QDemonRenderIndexBuffer);
    quint8 *baseMem = static_cast<quint8 *>(::malloc(ibufSize));
    return QSharedPointer<QDemonRenderIndexBuffer>(new (baseMem) QDemonRenderIndexBuffer(
                                                       context, size, componentType, usageType,
                                                       toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size())));
}
QT_END_NAMESPACE
