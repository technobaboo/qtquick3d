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
#pragma once
#ifndef QDEMON_RENDER_MESH_H
#define QDEMON_RENDER_MESH_H
#include <Qt3DSRender.h>
#include <Qt3DSRenderVertexBuffer.h>
#include <Qt3DSRenderIndexBuffer.h>
#include <Qt3DSRenderInputAssembler.h>
#include <Qt3DSBounds3.h>
#include <StringTable.h>
#include <Qt3DSContainers.h>
#include <QDemonRefCounted>
#include <Qt3DSQDemonNoCopy.h>

namespace qt3ds {
namespace render {

    struct SRenderSubsetBase
    {
        quint32 m_Count;
        quint32 m_Offset;
        NVBounds3 m_Bounds; // Vertex buffer bounds
        SRenderSubsetBase() {}
        SRenderSubsetBase(const SRenderSubsetBase &inOther)
            : m_Count(inOther.m_Count)
            , m_Offset(inOther.m_Offset)
            , m_Bounds(inOther.m_Bounds)
        {
        }

        SRenderSubsetBase &operator=(const SRenderSubsetBase &inOther)
        {
            m_Count = inOther.m_Count;
            m_Offset = inOther.m_Offset;
            m_Bounds = inOther.m_Bounds;
            return *this;
        }
    };

    struct SRenderJoint
    {
        qint32 m_JointID;
        qint32 m_ParentID;
        float m_invBindPose[16];
        float m_localToGlobalBoneSpace[16];
    };

    struct SRenderSubset : public SRenderSubsetBase
    {
        QDemonRenderInputAssembler *m_InputAssembler;
        QDemonRenderInputAssembler *m_InputAssemblerDepth;
        QDemonRenderInputAssembler
            *m_InputAssemblerPoints; ///< similar to depth but ignores index buffer.
        QDemonRenderVertexBuffer *m_VertexBuffer;
        QDemonRenderVertexBuffer
            *m_PosVertexBuffer; ///< separate position buffer for fast depth path rendering
        QDemonRenderIndexBuffer *m_IndexBuffer;
        QDemonRenderDrawMode::Enum m_PrimitiveType; ///< primitive type used for drawing
        float m_EdgeTessFactor; ///< edge tessellation amount used for tessellation shaders
        float m_InnerTessFactor; ///< inner tessellation amount used for tessellation shaders
        bool m_WireframeMode; ///< true if we should draw the object as wireframe ( currently ony if
                              ///tessellation is enabled )
        QDemonConstDataRef<SRenderJoint> m_Joints;
        CRegisteredString m_Name;
        nvvector<SRenderSubsetBase> m_SubSubsets;

        SRenderSubset(NVAllocatorCallback &alloc)
            : m_InputAssembler(nullptr)
            , m_InputAssemblerDepth(nullptr)
            , m_InputAssemblerPoints(nullptr)
            , m_VertexBuffer(nullptr)
            , m_PosVertexBuffer(nullptr)
            , m_IndexBuffer(nullptr)
            , m_PrimitiveType(QDemonRenderDrawMode::Triangles)
            , m_EdgeTessFactor(1.0)
            , m_InnerTessFactor(1.0)
            , m_WireframeMode(false)
            , m_SubSubsets(alloc, "SRenderSubset::m_SubSubsets")
        {
        }
        SRenderSubset(const SRenderSubset &inOther)
            : SRenderSubsetBase(inOther)
            , m_InputAssembler(inOther.m_InputAssembler)
            , m_InputAssemblerDepth(inOther.m_InputAssemblerDepth)
            , m_InputAssemblerPoints(inOther.m_InputAssemblerPoints)
            , m_VertexBuffer(inOther.m_VertexBuffer)
            , m_PosVertexBuffer(inOther.m_PosVertexBuffer)
            , m_IndexBuffer(inOther.m_IndexBuffer)
            , m_PrimitiveType(inOther.m_PrimitiveType)
            , m_EdgeTessFactor(inOther.m_EdgeTessFactor)
            , m_InnerTessFactor(inOther.m_InnerTessFactor)
            , m_WireframeMode(inOther.m_WireframeMode)
            , m_Joints(inOther.m_Joints)
            , m_Name(inOther.m_Name)
            , m_SubSubsets(inOther.m_SubSubsets)
        {
        }
        // Note that subSubsets is *not* copied.
        SRenderSubset(NVAllocatorCallback &alloc, const SRenderSubset &inOther,
                      const SRenderSubsetBase &inBase)
            : SRenderSubsetBase(inBase)
            , m_InputAssembler(inOther.m_InputAssembler)
            , m_InputAssemblerDepth(inOther.m_InputAssemblerDepth)
            , m_InputAssemblerPoints(inOther.m_InputAssemblerPoints)
            , m_VertexBuffer(inOther.m_VertexBuffer)
            , m_PosVertexBuffer(inOther.m_PosVertexBuffer)
            , m_IndexBuffer(inOther.m_IndexBuffer)
            , m_PrimitiveType(inOther.m_PrimitiveType)
            , m_EdgeTessFactor(inOther.m_EdgeTessFactor)
            , m_InnerTessFactor(inOther.m_InnerTessFactor)
            , m_WireframeMode(inOther.m_WireframeMode)
            , m_Name(inOther.m_Name)
            , m_SubSubsets(alloc, "SRenderSubset::m_SubSubsets")
        {
        }

        SRenderSubset &operator=(const SRenderSubset &inOther)
        {
            if (this != &inOther) {
                SRenderSubsetBase::operator=(inOther);
                m_InputAssembler = inOther.m_InputAssembler;
                m_InputAssemblerDepth = inOther.m_InputAssemblerDepth;
                m_VertexBuffer = inOther.m_VertexBuffer;
                m_PosVertexBuffer = inOther.m_PosVertexBuffer;
                m_IndexBuffer = inOther.m_IndexBuffer;
                m_PrimitiveType = inOther.m_PrimitiveType;
                m_EdgeTessFactor = inOther.m_EdgeTessFactor;
                m_InnerTessFactor = inOther.m_InnerTessFactor;
                m_WireframeMode = inOther.m_WireframeMode;
                m_Joints = inOther.m_Joints;
                m_Name = inOther.m_Name;
                m_SubSubsets = inOther.m_SubSubsets;
            }
            return *this;
        }
    };

    struct SRenderMesh : public QDemonNoCopy
    {
        nvvector<SRenderSubset> m_Subsets;
        nvvector<SRenderJoint> m_Joints;
        QDemonRenderDrawMode::Enum m_DrawMode;
        QDemonRenderWinding::Enum m_Winding; // counterclockwise
        quint32 m_MeshId; // Id from the file of this mesh.

        SRenderMesh(QDemonRenderDrawMode::Enum inDrawMode, QDemonRenderWinding::Enum inWinding,
                    quint32 inMeshId, NVAllocatorCallback &alloc)
            : m_Subsets(alloc, "SRenderMesh::m_Subsets")
            , m_Joints(alloc, "SRenderMesh::Joints")
            , m_DrawMode(inDrawMode)
            , m_Winding(inWinding)
            , m_MeshId(inMeshId)
        {
        }
    };
}
}

#endif