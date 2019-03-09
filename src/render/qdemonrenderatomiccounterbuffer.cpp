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
    QByteArray m_name; ///< parameter Name
    qint32 m_offset; ///< offset into the memory buffer

    AtomicCounterBufferEntry(const QByteArray &name, qint32 offset) : m_name(name), m_offset(offset) {}
};

QDemonRenderAtomicCounterBuffer::QDemonRenderAtomicCounterBuffer(const QDemonRef<QDemonRenderContext> &context,
                                                                 const QByteArray &bufferName,
                                                                 size_t size,
                                                                 QDemonRenderBufferUsageType usageType,
                                                                 QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size, QDemonRenderBufferBindType::Storage, usageType, data)
    , m_name(bufferName)
    , m_dirty(true)
{
    Q_ASSERT(context->isStorageBufferSupported());
}

QDemonRenderAtomicCounterBuffer::~QDemonRenderAtomicCounterBuffer()
{
    for (TRenderAtomiCounterBufferEntryMap::iterator iter = m_atomicCounterBufferEntryMap.begin(),
                                                     end = m_atomicCounterBufferEntryMap.end();
         iter != end;
         ++iter) {
        delete iter.value();
    }

    m_atomicCounterBufferEntryMap.clear();

    m_context->bufferDestroyed(this);
}

void QDemonRenderAtomicCounterBuffer::bind()
{
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_backend->bindBuffer(m_bufferHandle, m_bindFlags);
}

void QDemonRenderAtomicCounterBuffer::bindToShaderProgram(quint32 index)
{
    m_backend->programSetAtomicCounterBuffer(index, m_bufferHandle);
}

void QDemonRenderAtomicCounterBuffer::update()
{
    // we only update the buffer if it is dirty and we actually have some data
    if (m_dirty && m_bufferData.size()) {
        m_backend->updateBuffer(m_bufferHandle, m_bindFlags, m_bufferData.size(), m_usageType, m_bufferData.begin());
        m_dirty = false;
    }
}

void QDemonRenderAtomicCounterBuffer::updateData(qint32 offset, QDemonDataRef<quint8> data)
{
    // we only update the buffer if we something
    if (data.size())
        m_backend->updateBuffer(m_bufferHandle, m_bindFlags, data.size(), m_usageType, data.begin() + offset);
}

void QDemonRenderAtomicCounterBuffer::addParam(const QByteArray &name, quint32 offset)
{
    if (m_atomicCounterBufferEntryMap.find(name) == m_atomicCounterBufferEntryMap.end()) {
        AtomicCounterBufferEntry *newEntry = new AtomicCounterBufferEntry(name, offset);

        if (newEntry)
            m_atomicCounterBufferEntryMap.insert(name, newEntry);
    } else {
        // no duplicated entries
        return;
    }
}

bool QDemonRenderAtomicCounterBuffer::containsParam(const QByteArray &name)
{
    if (m_atomicCounterBufferEntryMap.find(name) != m_atomicCounterBufferEntryMap.end())
        return true;
    else
        return false;
}

QDemonRef<QDemonRenderAtomicCounterBuffer> QDemonRenderAtomicCounterBuffer::create(const QDemonRef<QDemonRenderContext> &context,
                                                                                   const char *bufferName,
                                                                                   QDemonRenderBufferUsageType usageType,
                                                                                   size_t size,
                                                                                   QDemonConstDataRef<quint8> bufferData)
{
    if (context->isAtomicCounterBufferSupported()) {
        return QDemonRef<QDemonRenderAtomicCounterBuffer>(
                new QDemonRenderAtomicCounterBuffer(context,
                                                    bufferName,
                                                    size,
                                                    usageType,
                                                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size())));
    } else {
        Q_ASSERT(false);
    }
    return QDemonRef<QDemonRenderAtomicCounterBuffer>();
}

QT_END_NAMESPACE
