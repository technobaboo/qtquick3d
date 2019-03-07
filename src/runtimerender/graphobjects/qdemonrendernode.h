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
#ifndef QDEMON_RENDER_NODE_H
#define QDEMON_RENDER_NODE_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendereulerangles.h>
//#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>

#include <QtDemon/QDemonBounds3>

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

struct QDemonRenderModel;
struct QDemonRenderLight;
struct QDemonRenderCamera;
struct QDemonText;
struct QDemonGraphNode;
class QDemonBufferManager;

class INodeQueue
{
protected:
    virtual ~INodeQueue();

public:
    virtual void enqueue(QDemonRenderModel &inModel) = 0;
    virtual void enqueue(QDemonRenderLight &inLight) = 0;
    virtual void enqueue(QDemonRenderCamera &inCamera) = 0;
    // virtual void Enqueue( SText& inText ) = 0;
};

enum class QDemonNodeFlagValues
{
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
    /// or is it offscreen-only
    ForceLayerOffscreen = 1 << 12, ///< Forces a layer to always use the offscreen rendering
    /// mechanism.  This can be usefulf or caching purposes.
    IgnoreParentTransform = 1 << 13,
    LayerEnableDepthPrePass = 1 << 14, ///< True when we render a depth pass before
};

enum class NodeTransformDirtyFlag
{
    TransformNotDirty,
    TransformIsDirty,
};

struct QDemonNodeFlags : public QFlags<QDemonNodeFlagValues>
{
    QDemonNodeFlags() : QFlags() {}
    void setActive(bool value) { setFlag(QDemonNodeFlagValues::Active, value); }
    bool isActive() const { return this->operator&(QDemonNodeFlagValues::Active); }

    void setGlobalActive(bool value) { setFlag(QDemonNodeFlagValues::GlobalActive, value); }
    bool isGloballyActive() const { return this->operator&(QDemonNodeFlagValues::GlobalActive); }

    void setTransformDirty(bool value) { setFlag(QDemonNodeFlagValues::TransformDirty, value); }
    bool isTransformDirty() const { return this->operator&(QDemonNodeFlagValues::TransformDirty); }

    void setDirty(bool value) { setFlag(QDemonNodeFlagValues::Dirty, value); }
    bool isDirty() const { return this->operator&(QDemonNodeFlagValues::Dirty); }

    bool isLeftHanded() const { return this->operator&(QDemonNodeFlagValues::LeftHanded); }
    void setLeftHanded(bool value) { setFlag(QDemonNodeFlagValues::LeftHanded, value); }

    bool isOrthographic() const { return this->operator&(QDemonNodeFlagValues::Orthographic); }
    void setOrthographic(bool value) { setFlag(QDemonNodeFlagValues::Orthographic, value); }

    bool isPointLight() const { return this->operator&(QDemonNodeFlagValues::PointLight); }
    void setPointLight(bool value) { setFlag(QDemonNodeFlagValues::PointLight, value); }

    bool isTextDirty() const { return this->operator&(QDemonNodeFlagValues::TextDirty); }
    void setTextDirty(bool value) { setFlag(QDemonNodeFlagValues::TextDirty, value); }

    bool isLocallyPickable() const { return this->operator&(QDemonNodeFlagValues::LocallyPickable); }
    void setLocallyPickable(bool value) { setFlag(QDemonNodeFlagValues::LocallyPickable, value); }

    bool isGloballyPickable() const { return this->operator&(QDemonNodeFlagValues::GloballyPickable); }
    void setGloballyPickable(bool value) { setFlag(QDemonNodeFlagValues::GloballyPickable, value); }

    bool isLayerRenderToTarget() const { return this->operator&(QDemonNodeFlagValues::LayerRenderToTarget); }
    void setLayerRenderToTarget(bool value) { setFlag(QDemonNodeFlagValues::LayerRenderToTarget, value); }

    bool isLayerEnableDepthTest() const { return this->operator&(QDemonNodeFlagValues::LayerEnableDepthTest); }
    void setLayerEnableDepthTest(bool value) { setFlag(QDemonNodeFlagValues::LayerEnableDepthTest, value); }

    bool isForceLayerOffscreen() const { return this->operator&(QDemonNodeFlagValues::ForceLayerOffscreen); }
    void setForceLayerOffscreen(bool value) { setFlag(QDemonNodeFlagValues::ForceLayerOffscreen, value); }

    bool isIgnoreParentTransform() const { return this->operator&(QDemonNodeFlagValues::IgnoreParentTransform); }
    void setIgnoreParentTransform(bool value) { setFlag(QDemonNodeFlagValues::IgnoreParentTransform, value); }

    bool isLayerEnableDepthPrepass() const { return this->operator&(QDemonNodeFlagValues::LayerEnableDepthPrePass); }
    void setLayerEnableDepthPrepass(bool value) { setFlag(QDemonNodeFlagValues::LayerEnableDepthPrePass, value); }
};

class QDemonRenderNodeFilterInterface;
class QDemonPathManagerInterface;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonGraphNode : public QDemonGraphObject
{
    // changing any one of these means you have to
    // set this object dirty
    QVector3D rotation; // Radians
    QVector3D position;
    QVector3D scale;
    QVector3D pivot;
    quint32 rotationOrder; // UICEulerOrder::EulOrd, defaults YXZs

    // This only sets dirty, not transform dirty
    // Opacity of 1 means opaque, opacity of zero means transparent.
    float localOpacity;

    // results of clearing dirty.
    QDemonNodeFlags flags;
    // These end up right handed
    QMatrix4x4 localTransform;
    QMatrix4x4 globalTransform;
    float globalOpacity;
    qint32 skeletonId;

    // node graph members.
    QDemonGraphNode *parent;
    QDemonGraphNode *nextSibling;
    QDemonGraphNode *previousSibling;
    QDemonGraphNode *firstChild;
    // Property maintained solely by the render system.
    // Depth-first-search index assigned and maintained by render system.
    quint32 dfsIndex;

    QDemonGraphNode(QDemonGraphObjectTypes::Enum inType = QDemonGraphObjectTypes::Node);
    QDemonGraphNode(const QDemonGraphNode &inCloningObject);
    ~QDemonGraphNode() {}

    // Sets this object dirty and walks down the graph setting all
    // children who are not dirty to be dirty.
    void markDirty(NodeTransformDirtyFlag inTransformDirty = NodeTransformDirtyFlag::TransformNotDirty);

    void addChild(QDemonGraphNode &inChild);
    void removeChild(QDemonGraphNode &inChild);
    QDemonGraphNode *getLastChild();

    // Remove this node from the graph.
    // It is no longer the the parent's child lists
    // and all of its children no longer have a parent
    // finally they are no longer siblings of each other.
    void removeFromGraph();

    // Calculate global transform and opacity
    // Walks up the graph ensure all parents are not dirty so they have
    // valid global transforms.
    bool calculateGlobalVariables();

    // Given our rotation order and handedness, calculate the final rotation matrix
    // Only the upper 3x3 of this matrix is filled in.
    // If this object is left handed, then you need to call FlipCoordinateSystem
    // to get a result identical to the result produced in CalculateLocalTransform
    void calculateRotationMatrix(QMatrix4x4 &outMatrix) const;

    // Get a rotation vector that would produce the given 3x.3 matrix.
    // Takes m_RotationOrder and m_Flags.IsLeftHandled into account.
    // Returns a rotation vector in radians.
    QVector3D getRotationVectorFromRotationMatrix(const QMatrix3x3 &inMatrix) const;

    static QVector3D getRotationVectorFromEulerAngles(const EulerAngles &inAngles);

    // Flip a matrix from left-handed to right-handed and vice versa
    static void flipCoordinateSystem(QMatrix4x4 &ioMatrix);

    // Force the calculation of the local transform
    void calculateLocalTransform();

    /**
     * @brief setup local tranform from a matrix.
     *		  This function decomposes a SRT matrix.
     *		  This will fail if this matrix contains non-affine transformations
     *
     * @param inTransform[in]	input transformation
     *
     * @return true backend type
     */
    void setLocalTransformFromMatrix(QMatrix4x4 &inTransform);

    // Get the bounds of us and our children in our local space.
    QDemonBounds3 getBounds(const QDemonBufferManager &inManager,
                            const QDemonRef<QDemonPathManagerInterface> &inPathManager,
                            bool inIncludeChildren = true,
                            QDemonRenderNodeFilterInterface *inChildFilter = nullptr) const;
    QDemonBounds3 getChildBounds(const QDemonBufferManager &inManager,
                                 const QDemonRef<QDemonPathManagerInterface> &inPathManager,
                                 QDemonRenderNodeFilterInterface *inChildFilter = nullptr) const;
    // Assumes CalculateGlobalVariables has already been called.
    QVector3D getGlobalPos() const;
    QVector3D getGlobalPivot() const;
    // Pulls the 3rd column out of the global transform.
    QVector3D getDirection() const;
    // Multiplies (0,0,-1) by the inverse transpose of the upper 3x3 of the global transform.
    // This is correct w/r/t to scaling and which the above getDirection is not.
    QVector3D getScalingCorrectDirection() const;

    // outMVP and outNormalMatrix are returned ready to upload to openGL, meaning they are
    // row-major.
    void calculateMVPAndNormalMatrix(const QMatrix4x4 &inViewProjection, QMatrix4x4 &outMVP, QMatrix3x3 &outNormalMatrix) const;

    // This should be in a utility file somewhere
    void calculateNormalMatrix(QMatrix3x3 &outNormalMatrix) const;
};
QT_END_NAMESPACE

#endif
