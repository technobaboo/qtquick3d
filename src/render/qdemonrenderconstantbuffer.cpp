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

#include <QtDemonRender/qdemonrenderconstantbuffer.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemon/qdemonutils.h>

QT_BEGIN_NAMESPACE

///< struct handling a constant buffer entry
class ConstantBufferParamEntry
{
public:
    QString m_name; ///< parameter Name
    QDemonRenderShaderDataTypes::Enum m_type; ///< parameter type
    qint32 m_count; ///< one or array size
    qint32 m_offset; ///< offset into the memory buffer

    ConstantBufferParamEntry(const QString &name, QDemonRenderShaderDataTypes::Enum type,
                             qint32 count, qint32 offset)
        : m_name(name)
        , m_type(type)
        , m_count(count)
        , m_offset(offset)
    {
    }
};

QDemonRenderConstantBuffer::QDemonRenderConstantBuffer(const QSharedPointer<QDemonRenderContextImpl> &context,
                                                       const QString &bufferName, size_t size,
                                                       QDemonRenderBufferUsageType::Enum usageType,
                                                       QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size, QDemonRenderBufferBindValues::Constant, usageType, QDemonDataRef<quint8>())
    , m_name(bufferName)
    , m_currentOffset(0)
    , m_currentSize(0)
    , m_hwBufferInitialized(false)
    , m_dirty(true)
    , m_rangeStart(0)
    , m_rangeEnd(0)
    , m_maxBlockSize(0)
{
    Q_ASSERT(context->getConstantBufferSupport());

    m_backend->getRenderBackendValue(QDemonRenderBackend::QDemonRenderBackendQuery::MaxConstantBufferBlockSize, &m_maxBlockSize);

    if (size && data.size() && size == data.size()) {
        Q_ASSERT(size < (quint32)m_maxBlockSize);
        if (allocateShadowBuffer(data.size())) {
            memcpy(m_shadowCopy.begin(), data.begin(), data.size());
        }
    }
}

QDemonRenderConstantBuffer::~QDemonRenderConstantBuffer()
{
    // check if we should release memory
    if (m_shadowCopy.size())
        ::free(m_shadowCopy.begin());

    m_shadowCopy = QDemonDataRef<quint8>();

    for (TRenderConstantBufferEntryMap::iterator iter = m_constantBufferEntryMap.begin(),
         end = m_constantBufferEntryMap.end();
         iter != end; ++iter) {
        delete iter.value();
    }

    m_constantBufferEntryMap.clear();

    m_context->bufferDestroyed(this);
}

void QDemonRenderConstantBuffer::bind()
{
    if (m_mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_backend->bindBuffer(m_bufferHandle, m_bindFlags);
}

void QDemonRenderConstantBuffer::bindToShaderProgram(const QSharedPointer<QDemonRenderShaderProgram> &inShader,
                                                     quint32 blockIndex,
                                                     quint32 binding)
{
    if ((qint32)binding == -1) {
        binding = m_context->getNextConstantBufferUnit();
        m_backend->programSetConstantBlock(inShader->getShaderProgramHandle(), blockIndex,
                                           binding);
    }

    m_backend->programSetConstantBuffer(binding, m_bufferHandle);
}

bool QDemonRenderConstantBuffer::setupBuffer(const QDemonRenderShaderProgram *program, qint32 index,
                                             qint32 bufSize, qint32 paramCount)
{
    bool bSuccess = false;

    if (!m_hwBufferInitialized) {
        // allocate shadow buffer
        quint8 *newMem = static_cast<quint8 *>(::malloc(size_t(bufSize)));
        if (!newMem)
            return false;

        // allocate temp buffers to hold constant buffer information
        qint32 *theIndices = nullptr;
        qint32 *theTypes = nullptr;
        qint32 *theSizes = nullptr;
        qint32 *theOffsets = nullptr;

        theIndices = static_cast<qint32 *>(::malloc(size_t(paramCount) * sizeof(qint32)));
        if (!theIndices)
            goto fail;
        theTypes = static_cast<qint32 *>(::malloc(size_t(paramCount) * sizeof(qint32)));
        if (!theTypes)
            goto fail;
        theSizes = static_cast<qint32 *>(::malloc(size_t(paramCount) * sizeof(qint32)));
        if (!theSizes)
            goto fail;
        theOffsets = static_cast<qint32 *>(::malloc(size_t(paramCount) * sizeof(qint32)));
        if (!theOffsets)
            goto fail;

        bSuccess = true;

        // get indices for the individal constant buffer entries
        m_backend->getConstantBufferParamIndices(program->getShaderProgramHandle(), index,
                                                 theIndices);

        // get constant buffer uniform information
        m_backend->getConstantBufferParamInfoByIndices(program->getShaderProgramHandle(),
                                                       paramCount, (quint32 *)theIndices,
                                                       theTypes, theSizes, theOffsets);

        // get the names of the uniforms
        char nameBuf[512];
        qint32 elementCount, binding;
        QDemonRenderShaderDataTypes::Enum type;

        QDEMON_FOREACH(idx, paramCount)
        {
            m_backend->getConstantInfoByID(program->getShaderProgramHandle(), theIndices[idx],
                                           512, &elementCount, &type, &binding, nameBuf);
            // check if we already have this entry
            const QString theName = QString::fromLocal8Bit(nameBuf);
            TRenderConstantBufferEntryMap::iterator entry =
                    m_constantBufferEntryMap.find(theName);
            if (entry != m_constantBufferEntryMap.end()) {
                ConstantBufferParamEntry *pParam = entry.value();
                // copy content
                if (m_shadowCopy.size())
                    memcpy(newMem + theOffsets[idx],
                           m_shadowCopy.begin() + entry.value()->m_offset,
                           entry.value()->m_count * getUniformTypeSize(pParam->m_type));

                pParam->m_offset = theOffsets[idx];
                Q_ASSERT(type == pParam->m_type);
                Q_ASSERT(elementCount == pParam->m_count);
            } else {
                // create one
                m_constantBufferEntryMap.insert(theName,
                                                createParamEntry(theName, (QDemonRenderShaderDataTypes::Enum)theTypes[idx],
                                                                 theSizes[idx], theOffsets[idx]));
            }
        }

        // release previous one
        if (m_shadowCopy.size()) {
            ::free(m_shadowCopy.begin());
        }
        // set new one
        m_shadowCopy = QDemonDataRef<quint8>(newMem, bufSize);

        m_hwBufferInitialized = true;

fail:
        if (theIndices)
            ::free(theIndices);
        if (theTypes)
            ::free(theTypes);
        if (theSizes)
            ::free(theSizes);
        if (theOffsets)
            ::free(theOffsets);

    } else {
        // some sanity checks
        bSuccess = true;
        bSuccess &= (m_shadowCopy.size() <= (quint32)bufSize);
    }

    return bSuccess;
}

void QDemonRenderConstantBuffer::update()
{
    // we only update the buffer if the buffer is already on hardware
    // and if it is dirty
    if (m_dirty && m_hwBufferInitialized) {
        if (m_rangeEnd == 0)
            m_backend->updateBuffer(m_bufferHandle, m_bindFlags, m_shadowCopy.size(),
                                    m_usageType, m_shadowCopy.begin());
        else
            updateRange();

        m_dirty = false;
        m_rangeStart = m_rangeEnd = 0;
    }
}

void QDemonRenderConstantBuffer::updateRange()
{
    if ((m_rangeStart + m_rangeEnd) > m_shadowCopy.size()) {
        Q_ASSERT(false);
        return;
    }

    m_backend->updateBufferRange(m_bufferHandle, m_bindFlags, m_rangeStart,
                                 m_rangeEnd - m_rangeStart,
                                 m_shadowCopy.begin() + m_rangeStart);
}

void QDemonRenderConstantBuffer::addParam(const QString &name,
                                          QDemonRenderShaderDataTypes::Enum type, qint32 count)
{
    if (m_constantBufferEntryMap.find(name) == m_constantBufferEntryMap.end()) {
        ConstantBufferParamEntry *newEntry = new ConstantBufferParamEntry(name, type, count, m_currentOffset);

        if (newEntry)
            m_constantBufferEntryMap.insert(name, newEntry);
    } else {
        // no duplicated entries
        return;
    }

    // compute new current buffer size and offset
    qint32 constantSize = getUniformTypeSize(type) * count;
    m_currentSize += constantSize;
    m_currentOffset += constantSize;
}

void QDemonRenderConstantBuffer::updateParam(const char *inName, QDemonDataRef<quint8> value)
{
    // allocate space if not done yet
    // NOTE this gets reallocated once we get the real constant buffer size from a program
    if (!m_shadowCopy.size()) {
        // allocate shadow buffer
        if (!allocateShadowBuffer(m_currentSize))
            return;
    }
    const QString theName = QString::fromLocal8Bit(inName);
    TRenderConstantBufferEntryMap::iterator entry = m_constantBufferEntryMap.find(theName);
    if (entry != m_constantBufferEntryMap.end()) {
        if (!memcmp(m_shadowCopy.begin() + entry.value()->m_offset, value.begin(),
                    entry.value()->m_count * getUniformTypeSize(entry.value()->m_type))) {
            return;
        }
        memcpy(m_shadowCopy.begin() + entry.value()->m_offset, value.begin(),
               entry.value()->m_count * getUniformTypeSize(entry.value()->m_type));
        m_dirty = true;
    }
}

void QDemonRenderConstantBuffer::updateRaw(qint32 offset, QDemonDataRef<quint8> data)
{
    // allocate space if yet done
    if (!m_shadowCopy.size()) {
        // allocate shadow buffer
        if (!allocateShadowBuffer(data.size()))
            return;
    }

    Q_ASSERT((offset + data.size()) < (quint32)m_maxBlockSize);

    // we do not initialize anything when this is used
    m_hwBufferInitialized = true;

    // we do not allow resize once allocated
    if ((offset + data.size()) > m_shadowCopy.size())
        return;

    // copy data
    if (!memcmp(m_shadowCopy.begin() + offset, data.begin(), data.size())) {
        return;
    }
    memcpy(m_shadowCopy.begin() + offset, data.begin(), data.size());

    // update start
    m_rangeStart = (m_dirty) ? (m_rangeStart > (quint32)offset) ? offset : m_rangeStart : offset;
    m_rangeEnd = (offset + data.size() > m_rangeEnd) ? offset + data.size() : m_rangeEnd;

    m_dirty = true;
}

ConstantBufferParamEntry *QDemonRenderConstantBuffer::createParamEntry(const QString &name, QDemonRenderShaderDataTypes::Enum type, qint32 count, qint32 offset)
{
    ConstantBufferParamEntry *newEntry = new ConstantBufferParamEntry(name, type, count, offset);

    return newEntry;
}

qint32
QDemonRenderConstantBuffer::getUniformTypeSize(QDemonRenderShaderDataTypes::Enum type)
{
    switch (type) {
    case QDemonRenderShaderDataTypes::Float:
        return sizeof(float);
    case QDemonRenderShaderDataTypes::Integer:
        return sizeof(qint32);
    case QDemonRenderShaderDataTypes::IntegerVec2:
        return sizeof(qint32) * 2;
    case QDemonRenderShaderDataTypes::IntegerVec3:
        return sizeof(qint32) * 3;
    case QDemonRenderShaderDataTypes::IntegerVec4:
        return sizeof(qint32) * 4;
    case QDemonRenderShaderDataTypes::UnsignedInteger:
        return sizeof(quint32);
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec2:
        return sizeof(quint32) * 2;
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec3:
        return sizeof(quint32) * 3;
    case QDemonRenderShaderDataTypes::UnsignedIntegerVec4:
        return sizeof(quint32) * 4;
    case QDemonRenderShaderDataTypes::Vec2:
        return sizeof(float) * 2;
    case QDemonRenderShaderDataTypes::Vec3:
        return sizeof(float) * 3;
    case QDemonRenderShaderDataTypes::Vec4:
        return sizeof(float) * 4;
    case QDemonRenderShaderDataTypes::Matrix3x3:
        return sizeof(float) * 9;
    case QDemonRenderShaderDataTypes::Matrix4x4:
        return sizeof(float) * 16;
    default:
        Q_ASSERT(!"Unhandled type in QDemonRenderConstantBuffer::getUniformTypeSize");
        break;
    }

    return 0;
}

bool QDemonRenderConstantBuffer::allocateShadowBuffer(quint32 size)
{
    // allocate shadow buffer
    quint8 *newMem = static_cast<quint8 *>(::malloc(size));
    if (!newMem)
        return false;

    m_shadowCopy = QDemonDataRef<quint8>(newMem, size);

    m_bufferCapacity = size;

    return true;
}

QSharedPointer<QDemonRenderConstantBuffer> QDemonRenderConstantBuffer::create(const QSharedPointer<QDemonRenderContextImpl> &context,
                                                                              const char *bufferName,
                                                                              QDemonRenderBufferUsageType::Enum usageType,
                                                                              size_t size,
                                                                              QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderConstantBuffer> retval = nullptr;

    if (context->getConstantBufferSupport()) {
        const QString theBufferName = QString::fromLocal8Bit(bufferName);
        retval.reset(new QDemonRenderConstantBuffer(context, theBufferName, size, usageType,
                                                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size())));
    } else {
        Q_ASSERT(false);
    }
    return retval;
}

QT_END_NAMESPACE
