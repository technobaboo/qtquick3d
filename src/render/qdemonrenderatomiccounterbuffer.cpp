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

#include <QtDemonRender/qdemonrenderatomiccounterbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>
#include <QtDemon/qdemonutils.h>


#include <QtCore/QString>

QT_BEGIN_NAMESPACE

///< struct handling a constant buffer entry
class AtomicCounterBufferEntry
{
public:
    QString m_Name; ///< parameter Name
    qint32 m_Offset; ///< offset into the memory buffer

    AtomicCounterBufferEntry(const QString &name, qint32 offset)
        : m_Name(name)
        , m_Offset(offset)
    {
    }
};

QDemonRenderAtomicCounterBuffer::QDemonRenderAtomicCounterBuffer(
        QDemonRenderContextImpl &context, const QString &bufferName, size_t size,
        QDemonRenderBufferUsageType::Enum usageType, QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size,
                             QDemonRenderBufferBindValues::Storage, usageType, data)
    , m_Name(bufferName)
    , m_Dirty(true)
{
    Q_ASSERT(context.IsStorageBufferSupported());
}

QDemonRenderAtomicCounterBuffer::~QDemonRenderAtomicCounterBuffer()
{
    for (TRenderAtomiCounterBufferEntryMap::iterator
         iter = m_AtomicCounterBufferEntryMap.begin(),
         end = m_AtomicCounterBufferEntryMap.end();
         iter != end; ++iter) {
         delete iter.value();
    }

    m_AtomicCounterBufferEntryMap.clear();

    m_Context.BufferDestroyed(*this);
}

void QDemonRenderAtomicCounterBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

void QDemonRenderAtomicCounterBuffer::BindToShaderProgram(quint32 index)
{
    m_Backend->ProgramSetAtomicCounterBuffer(index, m_BufferHandle);
}

void QDemonRenderAtomicCounterBuffer::Update()
{
    // we only update the buffer if it is dirty and we actually have some data
    if (m_Dirty && m_BufferData.size()) {
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, m_BufferData.size(), m_UsageType,
                                m_BufferData.begin());
        m_Dirty = false;
    }
}

void QDemonRenderAtomicCounterBuffer::UpdateData(qint32 offset, QDemonDataRef<quint8> data)
{
    // we only update the buffer if we something
    if (data.size())
        m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, data.size(), m_UsageType,
                                data.begin() + offset);
}

void QDemonRenderAtomicCounterBuffer::AddParam(const QString &name, quint32 offset)
{
    if (m_AtomicCounterBufferEntryMap.find(name) == m_AtomicCounterBufferEntryMap.end()) {
        AtomicCounterBufferEntry *newEntry = new AtomicCounterBufferEntry(name, offset);

        if (newEntry)
            m_AtomicCounterBufferEntryMap.insert(name, newEntry);
    } else {
        // no duplicated entries
        return;
    }
}

bool QDemonRenderAtomicCounterBuffer::ContainsParam(const QString &name)
{
    if (m_AtomicCounterBufferEntryMap.find(name) != m_AtomicCounterBufferEntryMap.end())
        return true;
    else
        return false;
}

QDemonRenderAtomicCounterBuffer *
QDemonRenderAtomicCounterBuffer::Create(QDemonRenderContextImpl &context, const char *bufferName,
                                        QDemonRenderBufferUsageType::Enum usageType, size_t size,
                                        QDemonConstDataRef<quint8> bufferData)
{
    QDemonRenderAtomicCounterBuffer *retval = nullptr;

    if (context.IsAtomicCounterBufferSupported()) {
        const QString theBufferName = QString::fromLocal8Bit(bufferName);
        quint32 bufSize = sizeof(QDemonRenderAtomicCounterBuffer);
        quint8 *newMem = static_cast<quint8 *>(::malloc(bufSize));
        retval = new (newMem) QDemonRenderAtomicCounterBuffer(
                    context, theBufferName, size, usageType,
                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size()));
    } else {
        Q_ASSERT(false);
    }
    return retval;
}

QT_END_NAMESPACE
