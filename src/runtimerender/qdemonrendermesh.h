/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

#ifndef QDEMON_RENDER_MESH_H
#define QDEMON_RENDER_MESH_H

#include <QtDemonRender/qdemonrendervertexbuffer.h>
#include <QtDemonRender/qdemonrenderindexbuffer.h>
#include <QtDemonRender/qdemonrenderinputassembler.h>

#include <QtDemon/QDemonBounds3>

QT_BEGIN_NAMESPACE

struct QDemonRenderSubsetBase
{
    quint32 count;
    quint32 offset;
    QDemonBounds3 bounds; // Vertex buffer bounds
    QDemonRenderSubsetBase() = default;
    QDemonRenderSubsetBase(const QDemonRenderSubsetBase &inOther)
        : count(inOther.count), offset(inOther.offset), bounds(inOther.bounds)
    {
    }

    QDemonRenderSubsetBase &operator=(const QDemonRenderSubsetBase &inOther)
    {
        count = inOther.count;
        offset = inOther.offset;
        bounds = inOther.bounds;
        return *this;
    }
};

struct QDemonRenderJoint
{
    qint32 jointID;
    qint32 parentID;
    float invBindPose[16];
    float localToGlobalBoneSpace[16];
};

struct QDemonRenderSubset : public QDemonRenderSubsetBase
{
    QDemonRef<QDemonRenderInputAssembler> inputAssembler;
    QDemonRef<QDemonRenderInputAssembler> inputAssemblerDepth;
    QDemonRef<QDemonRenderInputAssembler> inputAssemblerPoints; ///< similar to depth but ignores index buffer.
    QDemonRef<QDemonRenderVertexBuffer> vertexBuffer;
    QDemonRef<QDemonRenderVertexBuffer> posVertexBuffer; ///< separate position buffer for fast depth path rendering
    QDemonRef<QDemonRenderIndexBuffer> indexBuffer;
    QDemonRenderDrawMode primitiveType; ///< primitive type used for drawing
    float edgeTessFactor = 1.0f; ///< edge tessellation amount used for tessellation shaders
    float innerTessFactor = 1.0f; ///< inner tessellation amount used for tessellation shaders
    bool wireframeMode; ///< true if we should draw the object as wireframe ( currently ony if
    /// tessellation is enabled )
    QVector<QDemonRenderJoint> joints;
    QString name;
    QVector<QDemonRenderSubsetBase> subSubsets;

    QDemonRenderSubset() = default;
    QDemonRenderSubset(const QDemonRenderSubset &inOther)
        : QDemonRenderSubsetBase(inOther)
        , inputAssembler(inOther.inputAssembler)
        , inputAssemblerDepth(inOther.inputAssemblerDepth)
        , inputAssemblerPoints(inOther.inputAssemblerPoints)
        , vertexBuffer(inOther.vertexBuffer)
        , posVertexBuffer(inOther.posVertexBuffer)
        , indexBuffer(inOther.indexBuffer)
        , primitiveType(inOther.primitiveType)
        , edgeTessFactor(inOther.edgeTessFactor)
        , innerTessFactor(inOther.innerTessFactor)
        , wireframeMode(inOther.wireframeMode)
        , joints(inOther.joints)
        , name(inOther.name)
        , subSubsets(inOther.subSubsets)
    {
    }
    // Note that subSubsets is *not* copied.
    QDemonRenderSubset(const QDemonRenderSubset &inOther, const QDemonRenderSubsetBase &inBase)
        : QDemonRenderSubsetBase(inBase)
        , inputAssembler(inOther.inputAssembler)
        , inputAssemblerDepth(inOther.inputAssemblerDepth)
        , inputAssemblerPoints(inOther.inputAssemblerPoints)
        , vertexBuffer(inOther.vertexBuffer)
        , posVertexBuffer(inOther.posVertexBuffer)
        , indexBuffer(inOther.indexBuffer)
        , primitiveType(inOther.primitiveType)
        , edgeTessFactor(inOther.edgeTessFactor)
        , innerTessFactor(inOther.innerTessFactor)
        , wireframeMode(inOther.wireframeMode)
        , name(inOther.name)
    {
    }

    QDemonRenderSubset &operator=(const QDemonRenderSubset &inOther)
    {
        if (this != &inOther) {
            QDemonRenderSubsetBase::operator=(inOther);
            inputAssembler = inOther.inputAssembler;
            inputAssemblerDepth = inOther.inputAssemblerDepth;
            vertexBuffer = inOther.vertexBuffer;
            posVertexBuffer = inOther.posVertexBuffer;
            indexBuffer = inOther.indexBuffer;
            primitiveType = inOther.primitiveType;
            edgeTessFactor = inOther.edgeTessFactor;
            innerTessFactor = inOther.innerTessFactor;
            wireframeMode = inOther.wireframeMode;
            joints = inOther.joints;
            name = inOther.name;
            subSubsets = inOther.subSubsets;
        }
        return *this;
    }
};

struct QDemonRenderMeshPath
{
    QString path;
    uint key = 0;

    inline bool isNull() const { return path.isNull(); }
    static QDemonRenderMeshPath create(const QString &path) { return { path, qHash(path) }; }
};

inline bool operator==(const QDemonRenderMeshPath &p1, const QDemonRenderMeshPath &p2)
{
    return (p1.path == p2.path);
}

inline uint qHash(const QDemonRenderMeshPath &path, uint seed) Q_DECL_NOTHROW
{
    return (path.key) ? path.key : qHash(path.path, seed);
}

struct QDemonRenderMesh
{
    Q_DISABLE_COPY(QDemonRenderMesh)

    QVector<QDemonRenderSubset> subsets;
    QVector<QDemonRenderJoint> joints;
    QDemonRenderDrawMode drawMode;
    QDemonRenderWinding winding; // counterclockwise
    quint32 meshId; // Id from the file of this mesh.

    QDemonRenderMesh(QDemonRenderDrawMode inDrawMode, QDemonRenderWinding inWinding, quint32 inMeshId)
        : drawMode(inDrawMode), winding(inWinding), meshId(inMeshId)
    {
    }
};
QT_END_NAMESPACE

#endif
