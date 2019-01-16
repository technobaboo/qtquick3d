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

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>

#include <QtDemonAssetImport/private/qdemonmeshutilities_p.h>

#include <QtGui/QMatrix4x4>


using namespace QDemonMeshUtilities;

class MeshOffsetTracker
{
public:
    MeshOffsetTracker(qint64 startOffset)
        : m_startOffset(startOffset)
    {
    }

    bool needsAlignment() { return getAlignmentAmount() > 0; }
    quint32 getAlignmentAmount() { return 4 - (m_byteCounter % 4); }
    void align()
    {
        if (needsAlignment())
            m_byteCounter += getAlignmentAmount();
    }

    void advance(qint64 offset, bool forceAlign = true) {
        m_byteCounter += offset;
        if (forceAlign)
            align();
    }

    template <typename TDataType>
    void advance(OffsetDataRef<TDataType> &data, bool forceAlign = true)
    {
        data.m_Offset = m_byteCounter;
        m_byteCounter += data.size() * sizeof(TDataType);
        if (forceAlign)
            align();
    }

    qint64 startOffset() const
    {
        return m_startOffset;
    }

    qint64 offset() const
    {
        return m_startOffset + m_byteCounter;
    }

private:
    qint64 m_startOffset = 0;
    qint64 m_byteCounter = 0;
};


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Setup command line arguments
    QCommandLineParser cmdLineParser;
    cmdLineParser.process(app);
    QStringList meshFileNames = cmdLineParser.positionalArguments();

    // if there is nothing to do return early
    if (meshFileNames.isEmpty())
        return -1;

    // Process Mesh files
    for (const auto &meshFileName : meshFileNames) {
        QFile meshFile(meshFileName);
        if (!meshFile.open(QIODevice::ReadOnly)) {
            qWarning() << "could not open " << meshFileName;
            continue;
        }

        if (Mesh::IsMulti(meshFile)) {
            MeshMultiHeader* multiHeader = Mesh::LoadMultiHeader(meshFile);
            if (!multiHeader) {
                qWarning() << "could not read MeshMultiHeader in " << meshFileName;
                continue;
            }

            // Print Multiheader information
            qDebug() << " -- Multiheader -- ";
            qDebug() << "fileId: " << multiHeader->m_FileId;
            qDebug() << "version: " << multiHeader->m_Version;
            qDebug() << "mesh entries: " << multiHeader->m_Entries.m_Size;

            quint8 *theHeaderBaseAddr = reinterpret_cast<quint8 *>(multiHeader);
            bool foundMesh = false;
            for (quint32 idx = 0, end = multiHeader->m_Entries.size(); idx < end && !foundMesh; ++idx) {
                const MeshMultiEntry &entry(multiHeader->m_Entries.index(theHeaderBaseAddr, idx));
                qDebug() << "\t -- mesh entry" << idx << " -- ";
                qDebug() << "\tid: " << entry.m_MeshId;
                qDebug() << "\toffset: " << hex << entry.m_MeshOffset;
                qDebug() << "\tpadding: " << entry.m_Padding;

                meshFile.seek(entry.m_MeshOffset);

                // Read mesh header
                MeshDataHeader header;
                meshFile.read(reinterpret_cast<char *>(&header), sizeof(MeshDataHeader));
                qDebug() << "\t -- MeshDataHeader -- ";
                qDebug() << "\tfileId: " << header.m_FileId;
                qDebug() << "\tfileVersion: " << header.m_FileVersion;
                qDebug() << "\tflags: " << header.m_HeaderFlags;
                qDebug() << "\tsize(in bytes): " << header.m_SizeInBytes;

                QByteArray meshMetadataData = meshFile.read(sizeof(Mesh));
                Mesh *mesh = reinterpret_cast<Mesh *>(meshMetadataData.data());
                MeshOffsetTracker offsetTracker(entry.m_MeshOffset + sizeof(MeshDataHeader));
                offsetTracker.advance(sizeof(Mesh), false);

                // Vertex Buffer
                qDebug() << "\t\t -- Vertex Buffer --";
                meshFile.seek(offsetTracker.offset());
                offsetTracker.advance(mesh->m_VertexBuffer.m_Entries);
                qDebug() << "\t\tentries offset: " << hex << mesh->m_VertexBuffer.m_Entries.m_Offset;
                qDebug() << "\t\tentries size: " << mesh->m_VertexBuffer.m_Entries.m_Size * sizeof(MeshVertexBufferEntry);
                QByteArray vertexBufferEntriesData = meshFile.read(mesh->m_VertexBuffer.m_Entries.m_Size * sizeof(MeshVertexBufferEntry));
                for (quint32 idx = 0, end = mesh->m_VertexBuffer.m_Entries.size(); idx < end; ++idx) {
                    // get name length
                    meshFile.seek(offsetTracker.offset());
                    QByteArray lenghtBuffer = meshFile.read(sizeof (quint32));
                    const quint32 &nameLenght = *reinterpret_cast<const quint32 *>(lenghtBuffer.constData());
                    offsetTracker.advance(sizeof(quint32), false);
                    QByteArray nameBuffer = meshFile.read(nameLenght);
                    offsetTracker.advance(nameLenght);
                    qDebug() << "\t\t\t -- Vertex Buffer Entry " << idx << "-- ";
                    const MeshVertexBufferEntry &entry = reinterpret_cast<const MeshVertexBufferEntry *>(vertexBufferEntriesData.constData())[idx];
                    qDebug() << "\t\t\tname: " << nameBuffer.constData();
                    qDebug() << "\t\t\ttype: " << entry.m_ComponentType;
                    qDebug() << "\t\t\tnumComponents: " << entry.m_NumComponents;
                    qDebug() << "\t\t\tfirstItemOffset: " << entry.m_FirstItemOffset;
                }
                offsetTracker.advance(mesh->m_VertexBuffer.m_Data);
                qDebug() << "\t\tstride: " << mesh->m_VertexBuffer.m_Stride;
                qDebug() << "\t\tdata Offset: " << hex << mesh->m_VertexBuffer.m_Data.m_Offset;
                qDebug() << "\t\tdata Size: " << mesh->m_VertexBuffer.m_Data.m_Size * sizeof(quint8);

                // Index Buffer
                qDebug() << "\t\t -- Index Buffer -- ";
                offsetTracker.advance(mesh->m_IndexBuffer.m_Data);
                qDebug() << "\t\tcomponentType: " << mesh->m_IndexBuffer.m_ComponentType;
                qDebug() << "\t\tdata Offset: " << hex << mesh->m_IndexBuffer.m_Data.m_Offset;
                qDebug() << "\t\tdata Size: " << mesh->m_IndexBuffer.m_Data.m_Size * sizeof(quint8);

                // Subsets
                qDebug() << "\t\t -- Subsets -- ";
                meshFile.seek(offsetTracker.offset());
                offsetTracker.advance(mesh->m_Subsets);
                qDebug() << "\t\toffset: " << hex << mesh->m_Subsets.m_Offset;
                qDebug() << "\t\tsize: " << mesh->m_Subsets.m_Size * sizeof(MeshSubset);
                QByteArray subsetEntriesData = meshFile.read(mesh->m_Subsets.m_Size * sizeof(MeshSubset));
                for (quint32 idx = 0, end = mesh->m_Subsets.size(); idx < end; ++idx) {
                    qDebug() << "\t\t -- Subset " << idx << "-- ";
                    MeshSubset &subset = reinterpret_cast<MeshSubset *>(subsetEntriesData.data())[idx];
                    qDebug() << "\t\thasCount: " << subset.HasCount();
                    qDebug() << "\t\tcount: " << subset.m_Count;
                    qDebug() << "\t\toffset(size): " << subset.m_Offset;
                    qDebug() << "\t\tbounds: (" << subset.m_Bounds.minimum.x() << "," <<
                                subset.m_Bounds.minimum.y() << "," <<
                                subset.m_Bounds.minimum.z() << ") (" <<
                                subset.m_Bounds.maximum.x() << "," <<
                                subset.m_Bounds.maximum.y() << "," <<
                                subset.m_Bounds.maximum.z() << ")";
                    meshFile.seek(offsetTracker.offset());
                    offsetTracker.advance(subset.m_Name);
                    qDebug() << "\t\tname offset: " << hex << subset.m_Name.m_Offset;
                    qDebug() << "\t\tname size: " << subset.m_Name.m_Size * sizeof(char16_t);
                    QByteArray subsetNameBuffer = meshFile.read(subset.m_Name.m_Size * sizeof(char16_t));
                    const char16_t* name = reinterpret_cast<const char16_t*>(subsetNameBuffer.constData());
                    qDebug() << "\t\tname: " << QString::fromUtf16(name);
                }

                // Joints
                qDebug() << "\t\t -- Joints -- ";
                meshFile.seek(offsetTracker.offset());
                offsetTracker.advance(mesh->m_Joints);
                qDebug() << "\t\toffset: " << hex << mesh->m_Joints.m_Offset;
                qDebug() << "\t\tsize: " << mesh->m_Joints.m_Size * sizeof(Joint);
                QByteArray jointsData = meshFile.read(mesh->m_Joints.m_Size * sizeof(Joint));
                for (quint32 idx = 0, end = mesh->m_Joints.size(); idx < end; ++idx) {
                    qDebug() << "\t\t -- Joint " << idx << "-- ";
                    const Joint &joint = reinterpret_cast<const Joint *>(jointsData.constData())[idx];
                    qDebug() << "\t\tid: " << joint.m_JointID;
                    qDebug() << "\t\tparentId: " << joint.m_ParentID;
                    qDebug() << "\t\tinvBindPose: " << QMatrix4x4(joint.m_invBindPose);
                    qDebug() << "\t\tlocalToGlobalBoneSpace: " << QMatrix4x4(joint.m_localToGlobalBoneSpace);
                }

                // Draw Mode
                qDebug() << "\t\tdraw Mode: " << mesh->m_DrawMode;

                // Winding
                qDebug() << "\t\twinding: " << mesh->m_Winding;

            }

        }

        meshFile.close();
        qDebug() << "closed meshFile";
    }

    return 0;
}
