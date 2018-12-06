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
#ifndef QDEMON_RENDER_NODE_H
#define QDEMON_RENDER_NODE_H
#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QMatrix4x4.h>
#include <QVector3D.h>
#include <Qt3DSBounds3.h>
#include <Qt3DSFlags.h>
#include <Qt3DSQDemonNoCopy.h>
#include <QtDemonRuntimeRender/qdemonrendereulerangles.h>
#include <StringTable.h>

QT_BEGIN_NAMESPACE

struct SModel;
struct SLight;
struct SCamera;
struct SText;
struct SNode;
class IBufferManager;

class INodeQueue
{
protected:
    virtual ~INodeQueue() {}
public:
    virtual void Enqueue(SModel &inModel) = 0;
    virtual void Enqueue(SLight &inLight) = 0;
    virtual void Enqueue(SCamera &inCamera) = 0;
    // virtual void Enqueue( SText& inText ) = 0;
};

struct NodeFlagValues
{
    enum Enum {
        Dirty = 1,
        TransformDirty = 1 << 1,
        Active = 1 << 2, ///< Is this exact object active
        LeftHanded = 1 << 3,
        Orthographic = 1 << 4,
        PointLight = 1 << 5,
        GlobalActive = 1 << 6, ///< set based in Active and if a parent is active.
        TextDirty = 1 << 7,
        LocallyPickable = 1 << 8,
        GloballyPickable = 1 << 9,
        LayerEnableDepthTest = 1 << 10,
        LayerRenderToTarget = 1 << 11, ///< Does this layer render to the normal render target,
        ///or is it offscreen-only
        ForceLayerOffscreen = 1 << 12, ///< Forces a layer to always use the offscreen rendering
        ///mechanism.  This can be usefulf or caching purposes.
        IgnoreParentTransform = 1 << 13,
        LayerEnableDepthPrePass = 1 << 14, ///< True when we render a depth pass before
    };
};

struct NodeTransformDirtyFlag
{
    enum Enum {
        TransformNotDirty,
        TransformIsDirty,
    };
};
struct NodeFlags : public QDemonFlags<NodeFlagValues::Enum, quint32>
{
    NodeFlags()
        : QDemonFlags<NodeFlagValues::Enum, quint32>((quint32)0)
    {
    }
    void ClearOrSet(bool value, NodeFlagValues::Enum enumVal) { clearOrSet(value, enumVal); }
    void SetActive(bool value) { ClearOrSet(value, NodeFlagValues::Active); }
    bool IsActive() const { return this->operator&(NodeFlagValues::Active); }

    void SetGlobalActive(bool value) { ClearOrSet(value, NodeFlagValues::GlobalActive); }
    bool IsGloballyActive() const { return this->operator&(NodeFlagValues::GlobalActive); }

    void SetTransformDirty(bool value) { ClearOrSet(value, NodeFlagValues::TransformDirty); }
    bool IsTransformDirty() const { return this->operator&(NodeFlagValues::TransformDirty); }

    void SetDirty(bool value) { ClearOrSet(value, NodeFlagValues::Dirty); }
    bool IsDirty() const { return this->operator&(NodeFlagValues::Dirty); }

    bool IsLeftHanded() const { return this->operator&(NodeFlagValues::LeftHanded); }
    void SetLeftHanded(bool value) { ClearOrSet(value, NodeFlagValues::LeftHanded); }

    bool IsOrthographic() const { return this->operator&(NodeFlagValues::Orthographic); }
    void SetOrthographic(bool value) { ClearOrSet(value, NodeFlagValues::Orthographic); }

    bool IsPointLight() const { return this->operator&(NodeFlagValues::PointLight); }
    void SetPointLight(bool value) { ClearOrSet(value, NodeFlagValues::PointLight); }

    bool IsTextDirty() const { return this->operator&(NodeFlagValues::TextDirty); }
    void SetTextDirty(bool value) { ClearOrSet(value, NodeFlagValues::TextDirty); }

    bool IsLocallyPickable() const { return this->operator&(NodeFlagValues::LocallyPickable); }
    void SetLocallyPickable(bool value) { ClearOrSet(value, NodeFlagValues::LocallyPickable); }

    bool IsGloballyPickable() const
    {
        return this->operator&(NodeFlagValues::GloballyPickable);
    }
    void SetGloballyPickable(bool value)
    {
        ClearOrSet(value, NodeFlagValues::GloballyPickable);
    }

    bool IsLayerRenderToTarget() const
    {
        return this->operator&(NodeFlagValues::LayerRenderToTarget);
    }
    void SetLayerRenderToTarget(bool value)
    {
        ClearOrSet(value, NodeFlagValues::LayerRenderToTarget);
    }

    bool IsLayerEnableDepthTest() const
    {
        return this->operator&(NodeFlagValues::LayerEnableDepthTest);
    }
    void SetLayerEnableDepthTest(bool value)
    {
        ClearOrSet(value, NodeFlagValues::LayerEnableDepthTest);
    }

    bool IsForceLayerOffscreen() const
    {
        return this->operator&(NodeFlagValues::ForceLayerOffscreen);
    }
    void SetForceLayerOffscreen(bool value)
    {
        ClearOrSet(value, NodeFlagValues::ForceLayerOffscreen);
    }

    bool IsIgnoreParentTransform() const
    {
        return this->operator&(NodeFlagValues::IgnoreParentTransform);
    }
    void SetIgnoreParentTransform(bool value)
    {
        ClearOrSet(value, NodeFlagValues::IgnoreParentTransform);
    }

    bool IsLayerEnableDepthPrepass() const
    {
        return this->operator&(NodeFlagValues::LayerEnableDepthPrePass);
    }
    void SetLayerEnableDepthPrepass(bool value)
    {
        ClearOrSet(value, NodeFlagValues::LayerEnableDepthPrePass);
    }
};

struct QDEMON_AUTOTEST_EXPORT SNode : public SGraphObject
{
    // changing any one of these means you have to
    // set this object dirty
    QVector3D m_Rotation; // Radians
    QVector3D m_Position;
    QVector3D m_Scale;
    QVector3D m_Pivot;
    quint32 m_RotationOrder; // UICEulerOrder::EulOrd, defaults YXZs

    // This only sets dirty, not transform dirty
    // Opacity of 1 means opaque, opacity of zero means transparent.
    float m_LocalOpacity;

    // results of clearing dirty.
    NodeFlags m_Flags;
    // These end up right handed
    QMatrix4x4 m_LocalTransform;
    QMatrix4x4 m_GlobalTransform;
    float m_GlobalOpacity;
    qint32 m_SkeletonId;

    // node graph members.
    SNode *m_Parent;
    SNode *m_NextSibling;
    SNode *m_PreviousSibling;
    SNode *m_FirstChild;
    // Property maintained solely by the render system.
    // Depth-first-search index assigned and maintained by render system.
    quint32 m_DFSIndex;

    SNode(GraphObjectTypes::Enum inType = GraphObjectTypes::Node);
    SNode(const SNode &inCloningObject, NVAllocatorCallback &inAllocator);
    ~SNode() {}

    // Sets this object dirty and walks down the graph setting all
    // children who are not dirty to be dirty.
    void MarkDirty(NodeTransformDirtyFlag::Enum inTransformDirty =
            NodeTransformDirtyFlag::TransformNotDirty);

    void AddChild(SNode &inChild);
    void RemoveChild(SNode &inChild);
    SNode *GetLastChild();

    // Remove this node from the graph.
    // It is no longer the the parent's child lists
    // and all of its children no longer have a parent
    // finally they are no longer siblings of each other.
    void RemoveFromGraph();

    // Calculate global transform and opacity
    // Walks up the graph ensure all parents are not dirty so they have
    // valid global transforms.
    bool CalculateGlobalVariables();

    // Given our rotation order and handedness, calculate the final rotation matrix
    // Only the upper 3x3 of this matrix is filled in.
    // If this object is left handed, then you need to call FlipCoordinateSystem
    // to get a result identical to the result produced in CalculateLocalTransform
    void CalculateRotationMatrix(QMatrix4x4 &outMatrix) const;

    // Get a rotation vector that would produce the given 3x.3 matrix.
    // Takes m_RotationOrder and m_Flags.IsLeftHandled into account.
    // Returns a rotation vector in radians.
    QVector3D GetRotationVectorFromRotationMatrix(const QMatrix3x3 &inMatrix) const;

    static QVector3D GetRotationVectorFromEulerAngles(const EulerAngles &inAngles);

    // Flip a matrix from left-handed to right-handed and vice versa
    static void FlipCoordinateSystem(QMatrix4x4 &ioMatrix);

    // Force the calculation of the local transform
    void CalculateLocalTransform();

    /**
         * @brief setup local tranform from a matrix.
         *		  This function decomposes a SRT matrix.
         *		  This will fail if this matrix contains non-affine transformations
         *
         * @param inTransform[in]	input transformation
         *
         * @return true backend type
         */
    void SetLocalTransformFromMatrix(QMatrix4x4 &inTransform);

    // Get the bounds of us and our children in our local space.
    NVBounds3 GetBounds(IBufferManager &inManager, IPathManager &inPathManager,
                        bool inIncludeChildren = true,
                        IQt3DSRenderNodeFilter *inChildFilter = nullptr) const;
    NVBounds3 GetChildBounds(IBufferManager &inManager, IPathManager &inPathManager,
                             IQt3DSRenderNodeFilter *inChildFilter = nullptr) const;
    // Assumes CalculateGlobalVariables has already been called.
    QVector3D GetGlobalPos() const;
    QVector3D GetGlobalPivot() const;
    // Pulls the 3rd column out of the global transform.
    QVector3D GetDirection() const;
    // Multiplies (0,0,-1) by the inverse transpose of the upper 3x3 of the global transform.
    // This is correct w/r/t to scaling and which the above getDirection is not.
    QVector3D GetScalingCorrectDirection() const;

    // outMVP and outNormalMatrix are returned ready to upload to openGL, meaning they are
    // row-major.
    void CalculateMVPAndNormalMatrix(const QMatrix4x4 &inViewProjection, QMatrix4x4 &outMVP,
                                     QMatrix3x3 &outNormalMatrix) const;

    // This should be in a utility file somewhere
    static void GetMatrixUpper3x3(QMatrix3x3 &inDest, const QMatrix4x4 &inSrc);
    void CalculateNormalMatrix(QMatrix3x3 &outNormalMatrix) const;

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void Remap(TRemapperType &inRemapper)
    {
        SGraphObject::Remap(inRemapper);
        inRemapper.Remap(m_Parent);
        inRemapper.Remap(m_FirstChild);
        inRemapper.Remap(m_NextSibling);
        inRemapper.Remap(m_PreviousSibling);
    }
};
QT_END_NAMESPACE

#endif
