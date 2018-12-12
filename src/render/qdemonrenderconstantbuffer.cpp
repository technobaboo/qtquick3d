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
    QString m_Name; ///< parameter Name
    QDemonRenderShaderDataTypes::Enum m_Type; ///< parameter type
    qint32 m_Count; ///< one or array size
    qint32 m_Offset; ///< offset into the memory buffer

    ConstantBufferParamEntry(const QString &name, QDemonRenderShaderDataTypes::Enum type,
                             qint32 count, qint32 offset)
        : m_Name(name)
        , m_Type(type)
        , m_Count(count)
        , m_Offset(offset)
    {
    }
};

QDemonRenderConstantBuffer::QDemonRenderConstantBuffer(QSharedPointer<QDemonRenderContextImpl> context,
                                                       const QString &bufferName, size_t size,
                                                       QDemonRenderBufferUsageType::Enum usageType,
                                                       QDemonDataRef<quint8> data)
    : QDemonRenderDataBuffer(context, size,
                             QDemonRenderBufferBindValues::Constant, usageType, QDemonDataRef<quint8>())
    , m_Name(bufferName)
    , m_CurrentOffset(0)
    , m_CurrentSize(0)
    , m_HWBufferInitialized(false)
    , m_Dirty(true)
    , m_RangeStart(0)
    , m_RangeEnd(0)
    , m_MaxBlockSize(0)
{
    Q_ASSERT(context->GetConstantBufferSupport());

    m_Backend->GetRenderBackendValue(
                QDemonRenderBackend::QDemonRenderBackendQuery::MaxConstantBufferBlockSize, &m_MaxBlockSize);

    if (size && data.size() && size == data.size()) {
        Q_ASSERT(size < (quint32)m_MaxBlockSize);
        if (allocateShadowBuffer(data.size())) {
            memcpy(m_ShadowCopy.begin(), data.begin(), data.size());
        }
    }
}

QDemonRenderConstantBuffer::~QDemonRenderConstantBuffer()
{
    // check if we should release memory
    if (m_ShadowCopy.size()) {
        ::free(m_ShadowCopy.begin());
    }

    m_ShadowCopy = QDemonDataRef<quint8>();

    for (TRenderConstantBufferEntryMap::iterator iter = m_ConstantBufferEntryMap.begin(),
         end = m_ConstantBufferEntryMap.end();
         iter != end; ++iter) {
        delete iter.value();
    }

    m_ConstantBufferEntryMap.clear();

    m_Context->BufferDestroyed(this);
}

void QDemonRenderConstantBuffer::Bind()
{
    if (m_Mapped) {
        qCCritical(INVALID_OPERATION, "Attempting to Bind a locked buffer");
        Q_ASSERT(false);
    }

    m_Backend->BindBuffer(m_BufferHandle, m_BindFlags);
}

void QDemonRenderConstantBuffer::BindToShaderProgram(QSharedPointer<QDemonRenderShaderProgram> inShader,
                                                     quint32 blockIndex, quint32 binding)
{
    if ((qint32)binding == -1) {
        binding = m_Context->GetNextConstantBufferUnit();
        m_Backend->ProgramSetConstantBlock(inShader->GetShaderProgramHandle(), blockIndex,
                                           binding);
    }

    m_Backend->ProgramSetConstantBuffer(binding, m_BufferHandle);
}

bool QDemonRenderConstantBuffer::SetupBuffer(const QDemonRenderShaderProgram *program, qint32 index,
                                             qint32 bufSize, qint32 paramCount)
{
    bool bSuccess = false;

    if (!m_HWBufferInitialized) {
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
        m_Backend->GetConstantBufferParamIndices(program->GetShaderProgramHandle(), index,
                                                 theIndices);

        // get constant buffer uniform information
        m_Backend->GetConstantBufferParamInfoByIndices(program->GetShaderProgramHandle(),
                                                       paramCount, (quint32 *)theIndices,
                                                       theTypes, theSizes, theOffsets);

        // get the names of the uniforms
        char nameBuf[512];
        qint32 elementCount, binding;
        QDemonRenderShaderDataTypes::Enum type;

        QDEMON_FOREACH(idx, paramCount)
        {
            m_Backend->GetConstantInfoByID(program->GetShaderProgramHandle(), theIndices[idx],
                                           512, &elementCount, &type, &binding, nameBuf);
            // check if we already have this entry
            const QString theName = QString::fromLocal8Bit(nameBuf);
            TRenderConstantBufferEntryMap::iterator entry =
                    m_ConstantBufferEntryMap.find(theName);
            if (entry != m_ConstantBufferEntryMap.end()) {
                ConstantBufferParamEntry *pParam = entry.value();
                // copy content
                if (m_ShadowCopy.size())
                    memcpy(newMem + theOffsets[idx],
                           m_ShadowCopy.begin() + entry.value()->m_Offset,
                           entry.value()->m_Count * getUniformTypeSize(pParam->m_Type));

                pParam->m_Offset = theOffsets[idx];
                Q_ASSERT(type == pParam->m_Type);
                Q_ASSERT(elementCount == pParam->m_Count);
            } else {
                // create one
                m_ConstantBufferEntryMap.insert(theName,
                                                createParamEntry(theName, (QDemonRenderShaderDataTypes::Enum)theTypes[idx],
                                                                 theSizes[idx], theOffsets[idx]));
            }
        }

        // release previous one
        if (m_ShadowCopy.size()) {
            ::free(m_ShadowCopy.begin());
        }
        // set new one
        m_ShadowCopy = QDemonDataRef<quint8>(newMem, bufSize);

        m_HWBufferInitialized = true;

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
        bSuccess &= (m_ShadowCopy.size() <= (quint32)bufSize);
    }

    return bSuccess;
}

void QDemonRenderConstantBuffer::Update()
{
    // we only update the buffer if the buffer is already on hardware
    // and if it is dirty
    if (m_Dirty && m_HWBufferInitialized) {
        if (m_RangeEnd == 0)
            m_Backend->UpdateBuffer(m_BufferHandle, m_BindFlags, m_ShadowCopy.size(),
                                    m_UsageType, m_ShadowCopy.begin());
        else
            UpdateRange();

        m_Dirty = false;
        m_RangeStart = m_RangeEnd = 0;
    }
}

void QDemonRenderConstantBuffer::UpdateRange()
{
    if ((m_RangeStart + m_RangeEnd) > m_ShadowCopy.size()) {
        Q_ASSERT(false);
        return;
    }

    m_Backend->UpdateBufferRange(m_BufferHandle, m_BindFlags, m_RangeStart,
                                 m_RangeEnd - m_RangeStart,
                                 m_ShadowCopy.begin() + m_RangeStart);
}

void QDemonRenderConstantBuffer::AddParam(const QString &name,
                                          QDemonRenderShaderDataTypes::Enum type, qint32 count)
{
    if (m_ConstantBufferEntryMap.find(name) == m_ConstantBufferEntryMap.end()) {
        ConstantBufferParamEntry *newEntry = new ConstantBufferParamEntry(name, type, count, m_CurrentOffset);

        if (newEntry)
            m_ConstantBufferEntryMap.insert(name, newEntry);
    } else {
        // no duplicated entries
        return;
    }

    // compute new current buffer size and offset
    qint32 constantSize = getUniformTypeSize(type) * count;
    m_CurrentSize += constantSize;
    m_CurrentOffset += constantSize;
}

void QDemonRenderConstantBuffer::UpdateParam(const char *inName, QDemonDataRef<quint8> value)
{
    // allocate space if not done yet
    // NOTE this gets reallocated once we get the real constant buffer size from a program
    if (!m_ShadowCopy.size()) {
        // allocate shadow buffer
        if (!allocateShadowBuffer(m_CurrentSize))
            return;
    }
    const QString theName = QString::fromLocal8Bit(inName);
    TRenderConstantBufferEntryMap::iterator entry = m_ConstantBufferEntryMap.find(theName);
    if (entry != m_ConstantBufferEntryMap.end()) {
        if (!memcmp(m_ShadowCopy.begin() + entry.value()->m_Offset, value.begin(),
                    entry.value()->m_Count * getUniformTypeSize(entry.value()->m_Type))) {
            return;
        }
        memcpy(m_ShadowCopy.begin() + entry.value()->m_Offset, value.begin(),
               entry.value()->m_Count * getUniformTypeSize(entry.value()->m_Type));
        m_Dirty = true;
    }
}

void QDemonRenderConstantBuffer::UpdateRaw(qint32 offset, QDemonDataRef<quint8> data)
{
    // allocate space if yet done
    if (!m_ShadowCopy.size()) {
        // allocate shadow buffer
        if (!allocateShadowBuffer(data.size()))
            return;
    }

    Q_ASSERT((offset + data.size()) < (quint32)m_MaxBlockSize);

    // we do not initialize anything when this is used
    m_HWBufferInitialized = true;

    // we do not allow resize once allocated
    if ((offset + data.size()) > m_ShadowCopy.size())
        return;

    // copy data
    if (!memcmp(m_ShadowCopy.begin() + offset, data.begin(), data.size())) {
        return;
    }
    memcpy(m_ShadowCopy.begin() + offset, data.begin(), data.size());

    // update start
    m_RangeStart = (m_Dirty) ? (m_RangeStart > (quint32)offset) ? offset : m_RangeStart : offset;
    m_RangeEnd = (offset + data.size() > m_RangeEnd) ? offset + data.size() : m_RangeEnd;

    m_Dirty = true;
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

    m_ShadowCopy = QDemonDataRef<quint8>(newMem, size);

    m_BufferCapacity = size;

    return true;
}

QSharedPointer<QDemonRenderConstantBuffer> QDemonRenderConstantBuffer::Create(QSharedPointer<QDemonRenderContextImpl> context,
                                                                              const char *bufferName,
                                                                              QDemonRenderBufferUsageType::Enum usageType,
                                                                              size_t size,
                                                                              QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderConstantBuffer> retval = nullptr;

    if (context->GetConstantBufferSupport()) {
        const QString theBufferName = QString::fromLocal8Bit(bufferName);
        quint32 cbufSize = sizeof(QDemonRenderConstantBuffer);
        quint8 *newMem = static_cast<quint8 *>(::malloc(cbufSize));
        retval.reset(new (newMem) QDemonRenderConstantBuffer(
                    context, theBufferName, size, usageType,
                    toDataRef(const_cast<quint8 *>(bufferData.begin()), bufferData.size())));
    } else {
        Q_ASSERT(false);
    }
    return retval;
}

QT_END_NAMESPACE
