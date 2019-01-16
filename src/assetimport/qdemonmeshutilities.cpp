/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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

#include "qdemonmeshutilities_p.h"

#include <QtCore/QVector>
#include <QtCore/QBuffer>

QT_BEGIN_NAMESPACE

namespace QDemonMeshUtilities {

struct MeshSubsetV1
{
    // See description of a logical vertex buffer below
    quint32 m_LogicalVbufIndex;
    // QDEMON_MAX_U32 means use all available items
    quint32 m_Count;
    // Offset is in item size, not bytes.
    quint32 m_Offset;
    // Bounds of this subset.  This is filled in by the builder
    // see AddMeshSubset
    QDemonBounds3 m_Bounds;
};

struct LogicalVertexBuffer
{
    quint32 m_ByteOffset;
    quint32 m_ByteSize;
    LogicalVertexBuffer(quint32 byteOff, quint32 byteSize)
        : m_ByteOffset(byteOff)
        , m_ByteSize(byteSize)
    {
    }
    LogicalVertexBuffer()
        : m_ByteOffset(0)
        , m_ByteSize(0)
    {
    }
};

struct MeshV1
{
    VertexBuffer m_VertexBuffer;
    IndexBuffer m_IndexBuffer;
    OffsetDataRef<LogicalVertexBuffer> m_LogicalVertexBuffers; // may be empty
    OffsetDataRef<MeshSubsetV1> m_Subsets;
    QDemonRenderDrawMode::Enum m_DrawMode;
    QDemonRenderWinding::Enum m_Winding;
    typedef MeshSubsetV1 TSubsetType;
};


template <typename TSerializer>
void Serialize(TSerializer &serializer, MeshV1 &mesh)
{
    quint8 *baseAddr = reinterpret_cast<quint8 *>(&mesh);
    serializer.streamify(mesh.m_VertexBuffer.m_Entries);
    serializer.align();
    for (quint32 entry = 0, __numItems = (quint32)mesh.m_VertexBuffer.m_Entries.size(); entry < __numItems; ++entry)
    {
        MeshVertexBufferEntry &entryData = const_cast<MeshVertexBufferEntry &>(
            mesh.m_VertexBuffer.m_Entries.index(baseAddr, entry));
        serializer.streamifyCharPointerOffset(entryData.m_NameOffset);
        serializer.align();
    }
    serializer.streamify(mesh.m_VertexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_IndexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_LogicalVertexBuffers);
    serializer.align();
    serializer.streamify(mesh.m_Subsets);
    serializer.align();
}

struct MeshSubsetV2
{
    quint32 m_LogicalVbufIndex;
    quint32 m_Count;
    quint32 m_Offset;
    QDemonBounds3 m_Bounds;
    OffsetDataRef<char16_t> m_Name;
};

struct MeshV2
{
    static const char16_t *s_DefaultName;

    VertexBuffer m_VertexBuffer;
    IndexBuffer m_IndexBuffer;
    OffsetDataRef<LogicalVertexBuffer> m_LogicalVertexBuffers; // may be empty
    OffsetDataRef<MeshSubsetV2> m_Subsets;
    QDemonRenderDrawMode::Enum m_DrawMode;
    QDemonRenderWinding::Enum m_Winding;
    typedef MeshSubsetV2 TSubsetType;
};

template <typename TSerializer>
void Serialize(TSerializer &serializer, MeshV2 &mesh)
{
    quint8 *baseAddr = reinterpret_cast<quint8 *>(&mesh);
    serializer.streamify(mesh.m_VertexBuffer.m_Entries);
    serializer.align();
    for (quint32 entry = 0, __numItems = (quint32)mesh.m_VertexBuffer.m_Entries.size(); entry < __numItems; ++entry)
    {
        MeshVertexBufferEntry &entryData = const_cast<MeshVertexBufferEntry &>(
            mesh.m_VertexBuffer.m_Entries.index(baseAddr, entry));
        serializer.streamifyCharPointerOffset(entryData.m_NameOffset);
        serializer.align();
    }
    serializer.streamify(mesh.m_VertexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_IndexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_LogicalVertexBuffers);
    serializer.align();
    serializer.streamify(mesh.m_Subsets);
    serializer.align();
    for (quint32 entry = 0, __numItems = (quint32)mesh.m_Subsets.size(); entry < __numItems; ++entry)
    {
        MeshSubsetV2 &theSubset = const_cast<MeshSubsetV2 &>(mesh.m_Subsets.index(baseAddr, entry));
        serializer.streamify(theSubset.m_Name);
        serializer.align();
    }
}

// Localize the knowledge required to read/write a mesh into one function
// written in such a way that you can both read and write by passing
// in one serializer type or another.
// This function needs to be careful to request alignment after every write of a
// buffer that may leave us unaligned.  The easiest way to be correct is to request
// alignment a lot.  The hardest way is to use knowledge of the datatypes and
// only request alignment when necessary.
template <typename TSerializer>
void Serialize(TSerializer &serializer, Mesh &mesh)
{
    quint8 *baseAddr = reinterpret_cast<quint8 *>(&mesh);
    serializer.streamify(mesh.m_VertexBuffer.m_Entries);
    serializer.align();

    for (quint32 entry = 0, numItems = mesh.m_VertexBuffer.m_Entries.size(); entry < numItems; ++entry)
    {
        MeshVertexBufferEntry &entryData = mesh.m_VertexBuffer.m_Entries.index(baseAddr, entry);
        serializer.streamifyCharPointerOffset(entryData.m_NameOffset);
        serializer.align();
    }
    serializer.streamify(mesh.m_VertexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_IndexBuffer.m_Data);
    serializer.align();
    serializer.streamify(mesh.m_Subsets);
    serializer.align();

    for (quint32 entry = 0, numItems = mesh.m_Subsets.size(); entry < numItems; ++entry)
    {
        MeshSubset &theSubset = const_cast<MeshSubset &>(mesh.m_Subsets.index(baseAddr, entry));
        serializer.streamify(theSubset.m_Name);
        serializer.align();
    }
    serializer.streamify(mesh.m_Joints);
    serializer.align();
}

struct TotallingSerializer
{
    quint32 m_NumBytes;
    quint8 *m_BaseAddress;
    TotallingSerializer(quint8 *inBaseAddr)
        : m_NumBytes(0)
        , m_BaseAddress(inBaseAddr)
    {
    }
    template <typename TDataType>
    void streamify(const OffsetDataRef<TDataType> &data)
    {
        m_NumBytes += data.size() * sizeof(TDataType);
    }
    void streamify(const char *data)
    {
        if (data == nullptr)
            data = "";
        quint32 len = (quint32)strlen(data) + 1;
        m_NumBytes += 4;
        m_NumBytes += len;
    }
    void streamifyCharPointerOffset(quint32 inOffset)
    {
        if (inOffset) {
            const char *dataPtr = (const char *)(inOffset + m_BaseAddress);
            streamify(dataPtr);
        } else
            streamify("");
    }
    bool needsAlignment() const { return getAlignmentAmount() > 0; }
    quint32 getAlignmentAmount() const { return 4 - (m_NumBytes % 4); }
    void align()
    {
        if (needsAlignment())
            m_NumBytes += getAlignmentAmount();
    }
};

struct ByteWritingSerializer
{
    QIODevice &m_Stream;
    TotallingSerializer m_ByteCounter;
    quint8 *m_BaseAddress;
    ByteWritingSerializer(QIODevice &str, quint8 *inBaseAddress)
        : m_Stream(str)
        , m_ByteCounter(inBaseAddress)
        , m_BaseAddress(inBaseAddress)
    {
    }

    template <typename TDataType>
    void streamify(const OffsetDataRef<TDataType> &data)
    {
        m_ByteCounter.streamify(data);
        m_Stream.write(reinterpret_cast<const char *>(data.begin(m_BaseAddress)), data.size());
    }
    void streamify(const char *data)
    {
        m_ByteCounter.streamify(data);
        if (data == nullptr)
            data = "";
        quint32 len = (quint32)strlen(data) + 1;
        m_Stream.write(reinterpret_cast<const char *>(&len), sizeof(quint32));
        m_Stream.write(data, len);
    }
    void streamifyCharPointerOffset(quint32 inOffset)
    {
        const char *dataPtr = (const char *)(inOffset + m_BaseAddress);
        streamify(dataPtr);
    }

    void align()
    {
        if (m_ByteCounter.needsAlignment()) {
            quint8 buffer[] = { 0, 0, 0, 0 };
            m_Stream.write(reinterpret_cast<const char *>(buffer), m_ByteCounter.getAlignmentAmount());
            m_ByteCounter.align();
        }
    }
};


struct MemoryAssigningSerializer
{
    quint8 *m_Memory;
    quint8 *m_BaseAddress;
    quint32 m_Size;
    TotallingSerializer m_ByteCounter;
    bool m_Failure;
    MemoryAssigningSerializer(quint8 *data, quint32 size, quint32 startOffset)
        : m_Memory(data + startOffset)
        , m_BaseAddress(data)
        , m_Size(size)
        , m_ByteCounter(data)
        , m_Failure(false)
    {
        // We expect 4 byte aligned memory to begin with
        Q_ASSERT((((size_t)m_Memory) % 4) == 0);
    }

    template <typename TDataType>
    void streamify(const OffsetDataRef<TDataType> &_data)
    {
        OffsetDataRef<TDataType> &data = const_cast<OffsetDataRef<TDataType> &>(_data);
        if (m_Failure) {
            data.m_Size = 0;
            data.m_Offset = 0;
            return;
        }
        quint32 current = m_ByteCounter.m_NumBytes;
        m_ByteCounter.streamify(_data);
        if (m_ByteCounter.m_NumBytes > m_Size) {
            data.m_Size = 0;
            data.m_Offset = 0;
            m_Failure = true;
            return;
        }
        quint32 numBytes = m_ByteCounter.m_NumBytes - current;
        if (numBytes) {
            data.m_Offset = (quint32)(m_Memory - m_BaseAddress);
            updateMemoryBuffer(numBytes);
        } else {
            data.m_Offset = 0;
            data.m_Size = 0;
        }
    }
    void streamify(const char *&_data)
    {
        quint32 len;
        m_ByteCounter.m_NumBytes += 4;
        if (m_ByteCounter.m_NumBytes > m_Size) {
            _data = "";
            m_Failure = true;
            return;
        }
        memcpy(&len, m_Memory, 4);
        updateMemoryBuffer(4);
        m_ByteCounter.m_NumBytes += len;
        if (m_ByteCounter.m_NumBytes > m_Size) {
            _data = "";
            m_Failure = true;
            return;
        }
        _data = (const char *)m_Memory;
        updateMemoryBuffer(len);
    }
    void streamifyCharPointerOffset(quint32 &inOffset)
    {
        const char *dataPtr;
        streamify(dataPtr);
        inOffset = (quint32)(dataPtr - (const char *)m_BaseAddress);
    }
    void align()
    {
        if (m_ByteCounter.needsAlignment()) {
            quint32 numBytes = m_ByteCounter.getAlignmentAmount();
            m_ByteCounter.align();
            updateMemoryBuffer(numBytes);
        }
    }
    void updateMemoryBuffer(quint32 numBytes) { m_Memory += numBytes; }
};

inline quint32 GetMeshDataSize(Mesh &mesh)
{
    TotallingSerializer s(reinterpret_cast<quint8 *>(&mesh));
    Serialize(s, mesh);
    return s.m_NumBytes;
}

template <typename TDataType>
quint32 NextIndex(const quint8 *inBaseAddress, const OffsetDataRef<quint8> data, quint32 idx)
{
    quint32 numItems = data.size() / sizeof(TDataType);
    if (idx < numItems) {
        const TDataType *dataPtr(reinterpret_cast<const TDataType *>(data.begin(inBaseAddress)));
        return dataPtr[idx];
    } else {
        QT3DS_ASSERT(false);
        return 0;
    }
}

template <typename TDataType>
quint32 NextIndex(const QByteArray &data, quint32 idx)
{
    quint32 numItems = data.size() / sizeof(TDataType);
    if (idx < numItems) {
        const TDataType *dataPtr(reinterpret_cast<const TDataType *>(data.begin()));
        return dataPtr[idx];
    } else {
        Q_ASSERT(false);
        return 0;
    }
}

inline quint32 NextIndex(const QByteArray &inData, QDemonRenderComponentTypes::Enum inCompType, quint32 idx)
{
    if (inData.size() == 0)
        return idx;
    switch (inCompType) {
    case QDemonRenderComponentTypes::UnsignedInteger8:
        return NextIndex<quint8>(inData, idx);
    case QDemonRenderComponentTypes::Integer8:
        return NextIndex<quint8>(inData, idx);
    case QDemonRenderComponentTypes::UnsignedInteger16:
        return NextIndex<quint16>(inData, idx);
    case QDemonRenderComponentTypes::Integer16:
        return NextIndex<qint16>(inData, idx);
    case QDemonRenderComponentTypes::UnsignedInteger32:
        return NextIndex<quint32>(inData, idx);
    case QDemonRenderComponentTypes::Integer32:
        return NextIndex<qint32>(inData, idx);
    default:
        // Invalid index buffer index type.
        Q_ASSERT(false);
    }

    return 0;
}

template <typename TMeshType>
// Not exposed to the outside world
TMeshType *DoInitialize(quint16 /*meshFlags*/, QByteArray &data)
{
    quint8 *newMem = reinterpret_cast<quint8*>(data.begin());
    quint32 amountLeft = data.size() - sizeof(TMeshType);
    MemoryAssigningSerializer s(newMem, amountLeft, sizeof(TMeshType));
    TMeshType *retval = (TMeshType *)newMem;
    Serialize(s, *retval);
    if (s.m_Failure)
        return nullptr;
    return retval;
}

static char16_t g_DefaultName[] = { 0 };

const char16_t *Mesh::s_DefaultName = g_DefaultName;

template <typename TMeshType>
struct SubsetNameHandler
{
};

template <>
struct SubsetNameHandler<MeshV1>
{
    void AssignName(const quint8 * /*v1BaseAddress*/, const MeshSubsetV1 & /*mesh*/,
                    quint8 * /*baseAddress*/, quint8 *& /*nameBuffer*/, MeshSubset &outDest)
    {
        outDest.m_Name = OffsetDataRef<char16_t>();
    }
    quint32 NameLength(const MeshSubsetV1 &) { return 0; }
};

template <>
struct SubsetNameHandler<MeshV2>
{
    void AssignName(const quint8 *v2BaseAddress, const MeshSubsetV2 &mesh, quint8 *baseAddress,
                    quint8 *&nameBuffer, MeshSubset &outDest)
    {
        outDest.m_Name.m_Size = mesh.m_Name.m_Size;
        outDest.m_Name.m_Offset = (quint32)(nameBuffer - baseAddress);
        quint32 dtypeSize = mesh.m_Name.m_Size * 2;
        memcpy(nameBuffer, mesh.m_Name.begin(v2BaseAddress), dtypeSize);
        nameBuffer += dtypeSize;
    }
    quint32 NameLength(const MeshSubsetV2 &mesh) { return (mesh.m_Name.size() + 1) * 2; }
};

quint32 GetAlignedOffset(quint32 offset, quint32 align)
{
    quint32 leftover = offset % align;
    if (leftover)
        return offset + (align - leftover);
    return offset;
}

template <typename TPreviousMeshType>
Mesh *CreateMeshFromPreviousMesh(TPreviousMeshType *temp)
{
    quint32 newMeshSize = sizeof(Mesh);
    quint8 *tempBaseAddress = reinterpret_cast<quint8 *>(temp);
    quint32 alignment = sizeof(void *);

    quint32 vertBufferSize = GetAlignedOffset(temp->m_VertexBuffer.m_Data.size(), alignment);
    newMeshSize += vertBufferSize;
    quint32 entryDataSize = temp->m_VertexBuffer.m_Entries.size() * sizeof(MeshVertexBufferEntry);
    newMeshSize += entryDataSize;
    quint32 indexBufferSize = GetAlignedOffset(temp->m_IndexBuffer.m_Data.size(), alignment);
    newMeshSize += indexBufferSize;
    quint32 entryNameSize = 0;
    for (quint32 entryIdx = 0, entryEnd = temp->m_VertexBuffer.m_Entries.size(); entryIdx < entryEnd;
         ++entryIdx) {
        const QDemonRenderVertexBufferEntry theEntry =
            temp->m_VertexBuffer.m_Entries.index(tempBaseAddress, entryIdx).ToVertexBufferEntry(tempBaseAddress);
        const char *namePtr = theEntry.m_Name;
        if (namePtr == nullptr)
            namePtr = "";

        entryNameSize += (quint32)strlen(theEntry.m_Name) + 1;
    }
    entryNameSize = GetAlignedOffset(entryNameSize, alignment);

    newMeshSize += entryNameSize;
    quint32 subsetBufferSize = temp->m_Subsets.size() * sizeof(MeshSubset);
    newMeshSize += subsetBufferSize;
    quint32 nameLength = 0;
    for (quint32 subsetIdx = 0, subsetEnd = temp->m_Subsets.size(); subsetIdx < subsetEnd;
         ++subsetIdx) {
        nameLength += SubsetNameHandler<TPreviousMeshType>().NameLength(
            temp->m_Subsets.index(tempBaseAddress, subsetIdx));
    }
    nameLength = GetAlignedOffset(nameLength, alignment);

    newMeshSize += nameLength;

    Mesh *retval = new Mesh();
    quint8 *baseOffset = reinterpret_cast<quint8 *>(retval);
    quint8 *vertBufferData = baseOffset + sizeof(Mesh);
    quint8 *entryBufferData = vertBufferData + vertBufferSize;
    quint8 *entryNameBuffer = entryBufferData + entryDataSize;
    quint8 *indexBufferData = entryNameBuffer + entryNameSize;
    quint8 *subsetBufferData = indexBufferData + indexBufferSize;
    quint8 *nameData = subsetBufferData + subsetBufferSize;

    retval->m_DrawMode = temp->m_DrawMode;
    retval->m_Winding = temp->m_Winding;
    retval->m_VertexBuffer = temp->m_VertexBuffer;
    retval->m_VertexBuffer.m_Data.m_Offset = (quint32)(vertBufferData - baseOffset);
    retval->m_VertexBuffer.m_Entries.m_Offset = (quint32)(entryBufferData - baseOffset);
    memcpy(vertBufferData, temp->m_VertexBuffer.m_Data.begin(tempBaseAddress),
            temp->m_VertexBuffer.m_Data.size());
    memcpy(entryBufferData, temp->m_VertexBuffer.m_Entries.begin(tempBaseAddress), entryDataSize);
    for (quint32 idx = 0, __numItems = (quint32)temp->m_VertexBuffer.m_Entries.size(); idx < __numItems; ++idx)
    {
        const MeshVertexBufferEntry &src =
            temp->m_VertexBuffer.m_Entries.index(tempBaseAddress, idx);
        MeshVertexBufferEntry &dest = retval->m_VertexBuffer.m_Entries.index(baseOffset, idx);

        const char *targetName = reinterpret_cast<const char *>(src.m_NameOffset + tempBaseAddress);
        if (src.m_NameOffset == 0)
            targetName = "";
        quint32 nameLen = (quint32)strlen(targetName) + 1;
        dest.m_NameOffset = (quint32)(entryNameBuffer - baseOffset);
        memcpy(entryNameBuffer, targetName, nameLen);
        entryNameBuffer += nameLen;
    }

    retval->m_IndexBuffer = temp->m_IndexBuffer;
    retval->m_IndexBuffer.m_Data.m_Offset = (quint32)(indexBufferData - baseOffset);
    memcpy(indexBufferData, temp->m_IndexBuffer.m_Data.begin(tempBaseAddress),
            temp->m_IndexBuffer.m_Data.size());

    retval->m_Subsets.m_Size = temp->m_Subsets.m_Size;
    retval->m_Subsets.m_Offset = (quint32)(subsetBufferData - baseOffset);

    for (quint32 idx = 0, __numItems = (quint32)temp->m_Subsets.size(); idx < __numItems; ++idx)
    {
        MeshSubset &dest = const_cast<MeshSubset &>(retval->m_Subsets.index(baseOffset, idx));
        const typename TPreviousMeshType::TSubsetType &src =
            temp->m_Subsets.index(tempBaseAddress, idx);
        dest.m_Count = src.m_Count;
        dest.m_Offset = src.m_Offset;
        dest.m_Bounds = src.m_Bounds;
        SubsetNameHandler<TPreviousMeshType>().AssignName(tempBaseAddress, src, baseOffset,
                                                          nameData, dest);
    }
    return retval;
}

QDemonBounds3 Mesh::CalculateSubsetBounds(const QDemonRenderVertexBufferEntry &inEntry, const QByteArray &inVertxData, quint32 inStride, const QByteArray &inIndexData, QDemonRenderComponentTypes::Enum inIndexCompType, quint32 inSubsetCount, quint32 inSubsetOffset)
{
    QDemonBounds3 retval = QDemonBounds3();
    const QDemonRenderVertexBufferEntry &entry(inEntry);
    if (entry.m_ComponentType != QDemonRenderComponentTypes::Float32 || entry.m_NumComponents != 3) {
        Q_ASSERT(false);
        return retval;
    }

    const quint8 *beginPtr = reinterpret_cast<const quint8*>(inVertxData.constData());
    quint32 numBytes = inVertxData.size();
    quint32 dataStride = inStride;
    quint32 posOffset = entry.m_FirstItemOffset;
    // The loop below could be template specialized *if* we wanted to do this.
    // and the perf of the existing loop was determined to be a problem.
    // Else I would rather stay way from the template specialization.
    for (quint32 idx = 0, __numItems = (quint32)inSubsetCount; idx < __numItems; ++idx)
    {
        quint32 dataIdx = NextIndex(inIndexData, inIndexCompType, idx + inSubsetOffset);
        quint32 finalOffset = (dataIdx * dataStride) + posOffset;
        if (finalOffset + sizeof(Vec3) <= numBytes) {
            const quint8 *dataPtr = beginPtr + finalOffset;
            const auto vec3 = *reinterpret_cast<const Vec3 *>(dataPtr);
            retval.include(QVector3D(vec3.x, vec3.y, vec3.z));
        } else {
            Q_ASSERT(false);
        }
    }

    return retval;
}

void Mesh::Save(QIODevice &outStream) const
{
    Mesh &mesh(const_cast<Mesh &>(*this));
    quint8 *baseAddress = reinterpret_cast<quint8 *>(&mesh);
    quint32 numBytes = sizeof(Mesh) + GetMeshDataSize(mesh);
    MeshDataHeader header(numBytes);
    outStream.write(reinterpret_cast<const char *>(&header), sizeof(MeshDataHeader));
    outStream.write(reinterpret_cast<const char *>(this), sizeof(Mesh));
    ByteWritingSerializer writer(outStream, baseAddress);
    Serialize(writer, mesh);
}

bool Mesh::Save(const char *inFilePath) const
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadWrite)) {
        Q_ASSERT(false);
        return false;
    }

    Save(file);
    file.close();
    return true;
}

Mesh *Mesh::Load(QIODevice &inStream)
{
    MeshDataHeader header;
    inStream.read(reinterpret_cast<char *>(&header), sizeof(MeshDataHeader));
    Q_ASSERT(header.m_FileId == MeshDataHeader::GetFileId());
    if (header.m_FileId != MeshDataHeader::GetFileId())
        return nullptr;
    if (header.m_FileVersion < 1 || header.m_FileVersion > MeshDataHeader::GetCurrentFileVersion())
        return nullptr;
    if (header.m_SizeInBytes < sizeof(Mesh))
        return nullptr;
    QByteArray meshBuffer = inStream.read(header.m_SizeInBytes);
    if (meshBuffer.size() != header.m_SizeInBytes)
        goto failure;

    if (header.m_FileVersion == 1) {
        MeshV1 *temp = DoInitialize<MeshV1>(header.m_HeaderFlags, meshBuffer);
        if (temp == nullptr)
            goto failure;
        return CreateMeshFromPreviousMesh(temp);

    } else if (header.m_FileVersion == 2) {
        MeshV2 *temp = DoInitialize<MeshV2>(header.m_HeaderFlags, meshBuffer);
        if (temp == nullptr)
            goto failure;
        return CreateMeshFromPreviousMesh(temp);
    } else {
        Mesh *retval = Initialize(header.m_FileVersion, header.m_HeaderFlags, meshBuffer);
        if (retval == nullptr)
            goto failure;
        return retval;
    }

failure:
    Q_ASSERT(false);
    return nullptr;
}

Mesh *Mesh::Load(const char *inFilePath)
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadOnly)) {
        Q_ASSERT(false);
        return nullptr;
    }

    auto mesh = Load(file);
    file.close();
    return mesh;
}

Mesh *Mesh::Initialize(quint16 meshVersion, quint16 meshFlags, QByteArray &data)
{
    if (meshVersion != MeshDataHeader::GetCurrentFileVersion())
        return nullptr;
    return DoInitialize<Mesh>(meshFlags, data);
}

quint32 Mesh::SaveMulti(QIODevice &inStream, quint32 inId) const
{
    quint32 nextId = 1;
    MeshMultiHeader tempHeader;
    MeshMultiHeader *theHeader = nullptr;
    MeshMultiHeader *theWriteHeader = nullptr;

    qint64 newMeshStartPos = 0;
    if (inStream.size() != 0) {
        theHeader = LoadMultiHeader(inStream);
        if (theHeader == nullptr) {
            Q_ASSERT(false);
            return 0;
        }
        quint8 *headerBaseAddr = reinterpret_cast<quint8 *>(theHeader);
        for (quint32 idx = 0, end = theHeader->m_Entries.size(); idx < end; ++idx) {
            if (inId != 0) {
                Q_ASSERT(inId != theHeader->m_Entries.index(headerBaseAddr, idx).m_MeshId);
            }
            nextId = qMax(nextId, theHeader->m_Entries.index(headerBaseAddr, idx).m_MeshId + 1);
        }
        newMeshStartPos =
            sizeof(MeshMultiHeader) + theHeader->m_Entries.size() * sizeof(MeshMultiEntry);
        theWriteHeader = theHeader;
    } else {
        theWriteHeader = &tempHeader;
    }

    //inStream.SetPosition(-newMeshStartPos, SeekPosition::End);
    inStream.seek(inStream.size() - newMeshStartPos); // ### not sure about this one
    qint64 meshOffset = inStream.pos();

    Save(inStream);

    if (inId != 0)
        nextId = inId;
    quint8 *theWriteBaseAddr = reinterpret_cast<quint8 *>(theWriteHeader);
    // Now write a new header out.
    inStream.write(reinterpret_cast<char *>(theWriteHeader->m_Entries.begin(theWriteBaseAddr)),
                   theWriteHeader->m_Entries.size());
    MeshMultiEntry newEntry(static_cast<qint64>(meshOffset), nextId);
    inStream.write(reinterpret_cast<char *>(&newEntry), sizeof(MeshMultiEntry));
    theWriteHeader->m_Entries.m_Size++;
    inStream.write(reinterpret_cast<char *>(theWriteHeader), sizeof(MeshMultiHeader));

    return static_cast<quint32>(nextId);
}

quint32 Mesh::SaveMulti(const char *inFilePath) const
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadWrite)) {
        Q_ASSERT(false);
        return (quint32)-1;
    }

    quint32 id = SaveMulti(file);
    file.close();
    return id;
}

MultiLoadResult Mesh::LoadMulti(QIODevice &inStream, quint32 inId)
{
    MeshMultiHeader *theHeader(LoadMultiHeader(inStream));
    if (theHeader == nullptr) {
        return MultiLoadResult();
    }
    quint64 fileOffset = (quint64)-1;
    quint32 theId = inId;
    quint8 *theHeaderBaseAddr = reinterpret_cast<quint8 *>(theHeader);
    bool foundMesh = false;
    for (quint32 idx = 0, end = theHeader->m_Entries.size(); idx < end && !foundMesh; ++idx) {
        const MeshMultiEntry &theEntry(theHeader->m_Entries.index(theHeaderBaseAddr, idx));
        if (theEntry.m_MeshId == inId || (inId == 0 && theEntry.m_MeshId > theId)) {
            if (theEntry.m_MeshId == inId)
                foundMesh = true;
            theId = qMax(theId, (quint32)theEntry.m_MeshId);
            fileOffset = theEntry.m_MeshOffset;
        }
    }
    Mesh *retval = nullptr;
    if (fileOffset == (quint64)-1) {
        goto endFunction;
    }

    inStream.seek(static_cast<qint64>(fileOffset));
    retval = Load(inStream);
endFunction:
    return MultiLoadResult(retval, theId);
}

MultiLoadResult Mesh::LoadMulti(const char *inFilePath, quint32 inId)
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadOnly)) {
        Q_ASSERT(false);
        return MultiLoadResult();
    }

    auto result = LoadMulti(file, inId);
    file.close();
    return result;
}

bool Mesh::IsMulti(QIODevice &inStream)
{
    MeshMultiHeader theHeader;
    inStream.seek(inStream.size() -((qint64)(sizeof(MeshMultiHeader))));
    quint32 numBytes = inStream.read(reinterpret_cast<char *>(&theHeader), sizeof(MeshMultiHeader));
    if (numBytes != sizeof(MeshMultiHeader))
        return false;
    return theHeader.m_Version == MeshMultiHeader::GetMultiStaticVersion();
}

MeshMultiHeader *Mesh::LoadMultiHeader(QIODevice &inStream)
{
    MeshMultiHeader theHeader;
    inStream.seek(inStream.size() -((qint64)sizeof(MeshMultiHeader)));
    quint32 numBytes = inStream.read(reinterpret_cast<char *>(&theHeader), sizeof(MeshMultiHeader));
    if (numBytes != sizeof(MeshMultiHeader)
        || theHeader.m_FileId != MeshMultiHeader::GetMultiStaticFileId()
        || theHeader.m_Version > MeshMultiHeader::GetMultiStaticVersion()) {
        return nullptr;
    }
    size_t allocSize =
        sizeof(MeshMultiHeader) + theHeader.m_Entries.m_Size * sizeof(MeshMultiEntry);
    MeshMultiHeader *retval = new MeshMultiHeader;
    if (retval == nullptr) {
        Q_ASSERT(false);
        return nullptr;
    }
    quint8 *baseAddr = reinterpret_cast<quint8 *>(retval);
    quint8 *entryData = baseAddr + sizeof(MeshMultiHeader);
    *retval = theHeader;
    retval->m_Entries.m_Offset = (quint32)(entryData - baseAddr);
    inStream.seek(inStream.size() -((qint64)allocSize));

    numBytes = inStream.read(reinterpret_cast<char *>(entryData),
                             retval->m_Entries.m_Size * sizeof(MeshMultiEntry));
    if (numBytes != retval->m_Entries.m_Size * sizeof(MeshMultiEntry)) {
        Q_ASSERT(false);
        delete retval;
        retval = nullptr;
    }
    return retval;
}

MeshMultiHeader *Mesh::LoadMultiHeader(const char *inFilePath)
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadOnly)) {
        Q_ASSERT(false);
        return nullptr;
    }

    auto result = LoadMultiHeader(file);
    file.close();
    return result;
}

quint32 GetHighestId(MeshMultiHeader *inHeader)
{
    if (inHeader == nullptr) {
        Q_ASSERT(false);
        return 0;
    }
    quint8 *baseHeaderAddr = reinterpret_cast<quint8 *>(inHeader);
    quint32 highestId = 0;
    for (quint32 idx = 0, end = inHeader->m_Entries.size(); idx < end; ++idx)
        highestId = qMax(highestId, inHeader->m_Entries.index(baseHeaderAddr, idx).m_MeshId);
    return highestId;
}

quint32 Mesh::GetHighestMultiVersion(QIODevice &inStream)
{
    return GetHighestId(LoadMultiHeader(inStream));
}

quint32 Mesh::GetHighestMultiVersion(const char *inFilePath)
{
    QFile file(QString::fromLocal8Bit(inFilePath));
    if (!file.open(QIODevice::ReadOnly)) {
        Q_ASSERT(false);
        return (quint32)-1;
    }

    auto result = GetHighestMultiVersion(file);
    file.close();
    return result;
}

namespace  {

MeshBuilderVBufEntry ToEntry(const QVector<float> &data, const char *name, quint32 numComponents)
{
    return MeshBuilderVBufEntry(name, QByteArray(reinterpret_cast<const char *>(data.data())), QDemonRenderComponentTypes::Float32, numComponents);
}

struct DynamicVBuf
{
    quint32 m_Stride;
    QVector<QDemonRenderVertexBufferEntry> m_VertexBufferEntries;
    QByteArray m_VertexData;

    void clear()
    {
        m_Stride = 0;
        m_VertexBufferEntries.clear();
        m_VertexData.clear();
    }
};
struct DynamicIndexBuf
{
    QDemonRenderComponentTypes::Enum m_CompType;
    QByteArray m_IndexData;
    DynamicIndexBuf() {}

    void clear() { m_IndexData.clear(); }
};

struct SubsetDesc
{
    quint32 m_Count;
    quint32 m_Offset;

    QDemonBounds3 m_Bounds;
    QString m_Name;
    SubsetDesc(quint32 c, quint32 off)
        : m_Count(c)
        , m_Offset(off)
    {
    }
    SubsetDesc()
        : m_Count(0)
        , m_Offset(0)
    {
    }
};

class MeshBuilderImpl : public MeshBuilder
{
    DynamicVBuf m_VertexBuffer;
    DynamicIndexBuf m_IndexBuffer;
    QVector<Joint> m_Joints;
    QVector<SubsetDesc> m_MeshSubsetDescs;
    QDemonRenderDrawMode::Enum m_DrawMode;
    QDemonRenderWinding::Enum m_Winding;
    QByteArray m_RemappedVertexData;
    QByteArray m_NewIndexBuffer;
    QVector<quint8> m_MeshBuffer;

public:
    MeshBuilderImpl() { Reset(); }
    ~MeshBuilderImpl() override { Reset(); }
    void Release() override { delete this; }
    void Reset() override
    {
        m_VertexBuffer.clear();
        m_IndexBuffer.clear();
        m_Joints.clear();
        m_MeshSubsetDescs.clear();
        m_DrawMode = QDemonRenderDrawMode::Triangles;
        m_Winding = QDemonRenderWinding::CounterClockwise;
        m_MeshBuffer.clear();
    }

    void SetDrawParameters(QDemonRenderDrawMode::Enum drawMode, QDemonRenderWinding::Enum winding) override
    {
        m_DrawMode = drawMode;
        m_Winding = winding;
    }

    // Somewhat burly method to interleave the data as tightly as possible
    // while taking alignment into account.
    bool SetVertexBuffer(const QVector<MeshBuilderVBufEntry> &entries) override
    {
        quint32 currentOffset = 0;
        quint32 bufferAlignment = 0;
        quint32 numItems = 0;
        bool retval = true;
        for (quint32 idx = 0, __numItems = (quint32)entries.size(); idx < __numItems; ++idx)
        {
            const MeshBuilderVBufEntry &entry(entries[idx]);
            // Ignore entries with no data.
            if (entry.m_Data.begin() == nullptr || entry.m_Data.size() == 0)
                continue;

            quint32 alignment = (quint32)QDemonRenderComponentTypes::getSizeOfType(entry.m_ComponentType);
            bufferAlignment = qMax(bufferAlignment, alignment);
            quint32 byteSize = alignment * entry.m_NumComponents;

            if (entry.m_Data.size() % alignment != 0) {
                Q_ASSERT(false);
                retval = false;
            }

            quint32 localNumItems = entry.m_Data.size() / byteSize;
            if (numItems == 0) {
                numItems = localNumItems;
            } else if (numItems != localNumItems) {
                Q_ASSERT(false);
                retval = false;
                numItems = qMin(numItems, localNumItems);
            }
            // Lots of platforms can't handle non-aligned data.
            // so ensure we are aligned.
            currentOffset = GetAlignedOffset(currentOffset, alignment);
            QDemonRenderVertexBufferEntry vbufEntry(entry.m_Name, entry.m_ComponentType,
                                                entry.m_NumComponents, currentOffset);
            m_VertexBuffer.m_VertexBufferEntries.push_back(vbufEntry);
            currentOffset += byteSize;
        }
        m_VertexBuffer.m_Stride = GetAlignedOffset(currentOffset, bufferAlignment);

        // Packed interleave the data
        for (quint32 idx = 0, __numItems = (quint32)numItems; idx < __numItems; ++idx)
        {
            quint32 dataOffset = 0;
            for (quint32 entryIdx = 0, __numItems = (quint32)entries.size(); entryIdx < __numItems; ++entryIdx)
            {
                const MeshBuilderVBufEntry &entry(entries[entryIdx]);
                // Ignore entries with no data.
                if (entry.m_Data.begin() == NULL || entry.m_Data.size() == 0)
                    continue;

                quint32 alignment = (quint32)QDemonRenderComponentTypes::getSizeOfType(entry.m_ComponentType);
                quint32 byteSize = alignment * entry.m_NumComponents;
                quint32 offset = byteSize * idx;
                quint32 newOffset = GetAlignedOffset(dataOffset, alignment);
                QBuffer vertexDataBuffer(&m_VertexBuffer.m_VertexData);
                if (newOffset != dataOffset) {
                    QByteArray filler(newOffset - dataOffset, '\0');
                    vertexDataBuffer.write(filler);
                }
                vertexDataBuffer.write(entry.m_Data.begin() + offset, byteSize);
                dataOffset = newOffset + byteSize;
            }
            Q_ASSERT(dataOffset == m_VertexBuffer.m_Stride);
        }
        return retval;
    }

    void SetVertexBuffer(const QVector<QDemonRenderVertexBufferEntry> &entries, quint32 stride,
                                 QByteArray data) override
    {
        for (quint32 idx = 0, __numItems = (quint32)entries.size(); idx < __numItems; ++idx)
        {
            m_VertexBuffer.m_VertexBufferEntries.push_back(entries[idx]);
        }
        QBuffer vertexDataBuffer(&m_VertexBuffer.m_VertexData);
        vertexDataBuffer.write(data);
        if (stride == 0) {
            // Calculate the stride of the buffer using the vbuf entries
            for (quint32 idx = 0, __numItems = (quint32)entries.size(); idx < __numItems; ++idx)
            {
                const QDemonRenderVertexBufferEntry &entry(entries[idx]);
                stride = qMax(stride, (quint32)(entry.m_FirstItemOffset + (entry.m_NumComponents * QDemonRenderComponentTypes::getSizeOfType(entry.m_ComponentType))));
            }
        }
        m_VertexBuffer.m_Stride = stride;
    }

    void SetIndexBuffer(const QByteArray &data, QDemonRenderComponentTypes::Enum comp) override
    {
        m_IndexBuffer.m_CompType = comp;
        QBuffer indexBuffer(&m_IndexBuffer.m_IndexData);
        indexBuffer.write(data);
    }

    void AddJoint(qint32 jointID, qint32 parentID, const float *invBindPose,
                          const float *localToGlobalBoneSpace) override
    {
        m_Joints.push_back(Joint(jointID, parentID, invBindPose, localToGlobalBoneSpace));
    }

    SubsetDesc CreateSubset(const char16_t *inName, quint32 count, quint32 offset)
    {
        if (inName == nullptr)
            inName = u"";
        SubsetDesc retval(count, offset);
        retval.m_Name = QString::fromUtf16(inName);
        return retval;
    }

    // indexBuffer QDEMON_MAX_U32 means no index buffer.
    // count of QDEMON_MAX_U32 means use all available items
    // offset means exactly what you would think.  Offset is in item size, not bytes.
    void AddMeshSubset(const char16_t *inName, quint32 count, quint32 offset,
                               quint32 boundsPositionEntryIndex) override
    {
        SubsetDesc retval = CreateSubset(inName, count, offset);
        if (boundsPositionEntryIndex != QDEMON_MAX_U32) {
            retval.m_Bounds = Mesh::CalculateSubsetBounds(
                        m_VertexBuffer.m_VertexBufferEntries[boundsPositionEntryIndex],
                        m_VertexBuffer.m_VertexData, m_VertexBuffer.m_Stride, m_IndexBuffer.m_IndexData,
                        m_IndexBuffer.m_CompType, count, offset);
        }
        m_MeshSubsetDescs.push_back(retval);
    }

    void AddMeshSubset(const char16_t *inName, quint32 count, quint32 offset, const QDemonBounds3 &inBounds) override
    {
        SubsetDesc retval = CreateSubset(inName, count, offset);
        retval.m_Bounds = inBounds;
        m_MeshSubsetDescs.push_back(retval);
    }

    // We connect sub meshes which habe the same material
    void ConnectSubMeshes() override
    {
        if (m_MeshSubsetDescs.size() < 2) {
            // nothing to do
            return;
        }

        quint32 matDuplicates = 0;

        // as a pre-step we check if we have duplicate material at all
        for (quint32 i = 0, subsetEnd = m_MeshSubsetDescs.size(); i < subsetEnd && !matDuplicates;
             ++i) {
            SubsetDesc &currentSubset = m_MeshSubsetDescs[i];

            for (quint32 j = 0, subsetEnd = m_MeshSubsetDescs.size(); j < subsetEnd; ++j) {
                SubsetDesc &theSubset = m_MeshSubsetDescs[j];

                if (i == j)
                    continue;

                if (currentSubset.m_Name == theSubset.m_Name) {
                    matDuplicates++;
                    break; // found a duplicate bail out
                }
            }
        }

        // did we find some duplicates?
        if (matDuplicates) {
            QVector<SubsetDesc> newMeshSubsetDescs;
            QVector<SubsetDesc>::iterator theIter;
            QString curMatName;
            m_NewIndexBuffer.clear();

            for (theIter = m_MeshSubsetDescs.begin(); theIter != m_MeshSubsetDescs.end();
                 ++theIter) {
                bool bProcessed = false;

                for (QVector<SubsetDesc>::iterator iter = newMeshSubsetDescs.begin();
                     iter != newMeshSubsetDescs.end(); ++iter) {
                    if (theIter->m_Name == iter->m_Name) {
                        bProcessed = true;
                        break;
                    }
                }

                if (bProcessed)
                    continue;

                curMatName = theIter->m_Name;

                quint32 theIndexCompSize = (quint32)QDemonRenderComponentTypes::getSizeOfType(m_IndexBuffer.m_CompType);
                // get pointer to indices
                char *theIndices = (m_IndexBuffer.m_IndexData.begin()) + (theIter->m_Offset * theIndexCompSize);
                // write new offset
                theIter->m_Offset = m_NewIndexBuffer.size() / theIndexCompSize;
                // store indices
                QBuffer newIndexBuffer(&m_NewIndexBuffer);
                newIndexBuffer.write(theIndices, theIter->m_Count * theIndexCompSize);

                for (quint32 j = 0, subsetEnd = m_MeshSubsetDescs.size(); j < subsetEnd; ++j) {
                    if (theIter == &m_MeshSubsetDescs[j])
                        continue;

                    SubsetDesc &theSubset = m_MeshSubsetDescs[j];

                    if (curMatName == theSubset.m_Name) {
                        // get pointer to indices
                        char *theIndices = (m_IndexBuffer.m_IndexData.data()) + (theSubset.m_Offset * theIndexCompSize);
                        // store indices
                        newIndexBuffer.write(theIndices, theSubset.m_Count * theIndexCompSize);
                        // increment indices count
                        theIter->m_Count += theSubset.m_Count;
                    }
                }

                newMeshSubsetDescs.push_back(*theIter);
            }

            m_MeshSubsetDescs.clear();
            m_MeshSubsetDescs = newMeshSubsetDescs;
            m_IndexBuffer.m_IndexData.clear();
            QBuffer indexBuffer(&m_IndexBuffer.m_IndexData);
            indexBuffer.write(m_NewIndexBuffer);

            // compute new bounding box
            for (theIter = m_MeshSubsetDescs.begin(); theIter != m_MeshSubsetDescs.end();
                 ++theIter) {
                theIter->m_Bounds = Mesh::CalculateSubsetBounds(
                            m_VertexBuffer.m_VertexBufferEntries[0], m_VertexBuffer.m_VertexData,
                        m_VertexBuffer.m_Stride, m_IndexBuffer.m_IndexData, m_IndexBuffer.m_CompType,
                        theIter->m_Count, theIter->m_Offset);
            }
        }
    }

    // Here is the NVTriStrip magic.
    void OptimizeMesh() override
    {
        if (QDemonRenderComponentTypes::getSizeOfType(m_IndexBuffer.m_CompType) != 2) {
            // we currently re-arrange unsigned int indices.
            // this is because NvTriStrip only supports short indices
            Q_ASSERT(QDemonRenderComponentTypes::getSizeOfType(m_IndexBuffer.m_CompType) == 4);
            return;
        }
    }

    template <typename TDataType>
    static void Assign(quint8 *inBaseAddress, quint8 *inDataAddress,
                       OffsetDataRef<TDataType> &inBuffer, const QByteArray &inDestData)
    {
        inBuffer.m_Offset = (quint32)(inDataAddress - inBaseAddress);
        inBuffer.m_Size = inDestData.size();
        memcpy(inDataAddress, inDestData.data(), inDestData.size());
    }
    template <typename TDataType>
    static void Assign(quint8 *inBaseAddress, quint8 *inDataAddress,
                       OffsetDataRef<TDataType> &inBuffer, const QVector<TDataType> &inDestData)
    {
        inBuffer.m_Offset = (quint32)(inDataAddress - inBaseAddress);
        inBuffer.m_Size = inDestData.size();
        memcpy(inDataAddress, inDestData.data(), inDestData.size());
    }
    template <typename TDataType>
    static void Assign(quint8 *inBaseAddress, quint8 *inDataAddress,
                       OffsetDataRef<TDataType> &inBuffer, quint32 inDestSize)
    {
        inBuffer.m_Offset = (quint32)(inDataAddress - inBaseAddress);
        inBuffer.m_Size = inDestSize;
    }
    // Return the current mesh.  This is only good for this function call, item may change or be
    // released
    // due to any further function calls.
    Mesh &GetMesh() override
    {
        quint32 meshSize = sizeof(Mesh);
        quint32 alignment = sizeof(void *);
        quint32 vertDataSize = GetAlignedOffset(m_VertexBuffer.m_VertexData.size(), alignment);
        meshSize += vertDataSize;
        quint32 entrySize = m_VertexBuffer.m_VertexBufferEntries.size()
                * sizeof(QDemonRenderVertexBufferEntry);
        meshSize += entrySize;
        quint32 entryNameSize = 0;
        for (quint32 idx = 0, end = m_VertexBuffer.m_VertexBufferEntries.size(); idx < end; ++idx) {
            const QDemonRenderVertexBufferEntry &theEntry(m_VertexBuffer.m_VertexBufferEntries[idx]);
            const char *entryName = theEntry.m_Name;
            if (entryName == nullptr)
                entryName = "";
            entryNameSize += (quint32)(strlen(theEntry.m_Name)) + 1;
        }
        entryNameSize = GetAlignedOffset(entryNameSize, alignment);
        meshSize += entryNameSize;
        quint32 indexBufferSize = GetAlignedOffset(m_IndexBuffer.m_IndexData.size(), alignment);
        meshSize += indexBufferSize;
        quint32 subsetSize = m_MeshSubsetDescs.size() * sizeof(MeshSubset);
        quint32 nameSize = 0;
        for (quint32 idx = 0, end = m_MeshSubsetDescs.size(); idx < end; ++idx) {
            if (!m_MeshSubsetDescs[idx].m_Name.isEmpty())
                nameSize += m_MeshSubsetDescs[idx].m_Name.size() + 1;
        }
        nameSize *= sizeof(char16_t);
        nameSize = GetAlignedOffset(nameSize, alignment);

        meshSize += subsetSize + nameSize;
        quint32 jointsSize = m_Joints.size() * sizeof(Joint);
        meshSize += jointsSize;
        m_MeshBuffer.resize(meshSize);
        quint8 *baseAddress = m_MeshBuffer.data();
        Mesh *retval = reinterpret_cast<Mesh *>(baseAddress);
        retval->m_DrawMode = m_DrawMode;
        retval->m_Winding = m_Winding;
        quint8 *vertBufferData = baseAddress + sizeof(Mesh);
        quint8 *vertEntryData = vertBufferData + vertDataSize;
        quint8 *vertEntryNameData = vertEntryData + entrySize;
        quint8 *indexBufferData = vertEntryNameData + entryNameSize;
        quint8 *subsetBufferData = indexBufferData + indexBufferSize;
        quint8 *nameBufferData = subsetBufferData + subsetSize;
        quint8 *jointBufferData = nameBufferData + nameSize;

        retval->m_VertexBuffer.m_Stride = m_VertexBuffer.m_Stride;
        Assign(baseAddress,
               vertBufferData,
               retval->m_VertexBuffer.m_Data,
               m_VertexBuffer.m_VertexData);
        retval->m_VertexBuffer.m_Entries.m_Size = m_VertexBuffer.m_VertexBufferEntries.size();
        retval->m_VertexBuffer.m_Entries.m_Offset = (quint32)(vertEntryData - baseAddress);
        for (quint32 idx = 0, end = m_VertexBuffer.m_VertexBufferEntries.size(); idx < end; ++idx) {
            const QDemonRenderVertexBufferEntry &theEntry(m_VertexBuffer.m_VertexBufferEntries[idx]);
            MeshVertexBufferEntry &theDestEntry(
                        retval->m_VertexBuffer.m_Entries.index(baseAddress, idx));
            theDestEntry.m_ComponentType = theEntry.m_ComponentType;
            theDestEntry.m_FirstItemOffset = theEntry.m_FirstItemOffset;
            theDestEntry.m_NumComponents = theEntry.m_NumComponents;
            const char *targetName = theEntry.m_Name;
            if (targetName == nullptr)
                targetName = "";

            quint32 entryNameLen = (quint32)(strlen(targetName)) + 1;
            theDestEntry.m_NameOffset = (quint32)(vertEntryNameData - baseAddress);
            memcpy(vertEntryNameData, theEntry.m_Name, entryNameLen);
            vertEntryNameData += entryNameLen;
        }

        retval->m_IndexBuffer.m_ComponentType = m_IndexBuffer.m_CompType;
        Assign(baseAddress,
               indexBufferData,
               retval->m_IndexBuffer.m_Data,
               m_IndexBuffer.m_IndexData);
        Assign(baseAddress, subsetBufferData, retval->m_Subsets, m_MeshSubsetDescs.size());
        for (quint32 idx = 0, end = m_MeshSubsetDescs.size(); idx < end; ++idx) {
            SubsetDesc &theDesc = m_MeshSubsetDescs[idx];
            MeshSubset &theSubset = reinterpret_cast<MeshSubset *>(subsetBufferData)[idx];
            theSubset.m_Bounds = theDesc.m_Bounds;
            theSubset.m_Count = theDesc.m_Count;
            theSubset.m_Offset = theDesc.m_Offset;
            if (!theDesc.m_Name.isEmpty()) {
                theSubset.m_Name.m_Size = theDesc.m_Name.size() + 1;
                theSubset.m_Name.m_Offset = (quint32)(nameBufferData - baseAddress);
                std::transform(theDesc.m_Name.begin(), theDesc.m_Name.end(),
                               reinterpret_cast<char16_t *>(nameBufferData),
                               [](QChar c) { return static_cast<char16_t>(c.unicode()); });
                reinterpret_cast<char16_t *>(nameBufferData)[theDesc.m_Name.size()] = 0;
                nameBufferData += (theDesc.m_Name.size() + 1) * sizeof(char16_t);
            } else {
                theSubset.m_Name.m_Size = 0;
                theSubset.m_Name.m_Offset = 0;
            }
        }
        Assign(baseAddress, jointBufferData, retval->m_Joints, m_Joints);
        return *retval;
    }
};
}

QSharedPointer<MeshBuilder> MeshBuilder::CreateMeshBuilder()
{
    return QSharedPointer<MeshBuilder>(new MeshBuilderImpl());
}

}

QT_END_NAMESPACE
