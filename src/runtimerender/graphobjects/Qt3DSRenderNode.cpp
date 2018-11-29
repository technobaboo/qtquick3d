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

#include <Qt3DSRenderModel.h>
#include <Qt3DSRenderNode.h>
#include <Qt3DSRenderText.h>
#include <Qt3DSRenderer.h>
#include <Qt3DSRenderPathManager.h>
#include <Qt3DSRenderPath.h>

using namespace render;

SNode::SNode(GraphObjectTypes::Enum inGraphObjectType)
    : SGraphObject(inGraphObjectType)
    , m_Rotation(0, 0, 0) // Radians
    , m_Position(0, 0, 0)
    , m_Scale(1, 1, 1)
    , m_Pivot(0, 0, 0)
    , m_RotationOrder(EulOrdYXZs)
    , m_LocalOpacity(1.0f)
    , m_GlobalOpacity(1.0f)
    , m_SkeletonId(-1)
    , m_Parent(nullptr)
    , m_NextSibling(nullptr)
    , m_PreviousSibling(nullptr)
    , m_FirstChild(nullptr)
    , m_DFSIndex(0)
{
    m_Flags.SetDirty(true);
    m_Flags.SetTransformDirty(true);
    m_Flags.SetLeftHanded(true);
    m_Flags.SetActive(true);
    m_Flags.SetLocallyPickable(true);
}

SNode::SNode(const SNode &inCloningObject, NVAllocatorCallback &inAllocator)
    : SGraphObject(inCloningObject, inAllocator)
    , m_Rotation(inCloningObject.m_Rotation) // Radians
    , m_Position(inCloningObject.m_Position)
    , m_Scale(inCloningObject.m_Scale)
    , m_Pivot(inCloningObject.m_Pivot)
    , m_RotationOrder(inCloningObject.m_RotationOrder)
    , m_LocalOpacity(inCloningObject.m_LocalOpacity)
    , m_LocalTransform(inCloningObject.m_LocalTransform)
    , m_GlobalTransform(inCloningObject.m_GlobalTransform)
    , m_GlobalOpacity(inCloningObject.m_GlobalOpacity)
    , m_SkeletonId(inCloningObject.m_SkeletonId)
    , m_Parent(nullptr)
    , m_NextSibling(nullptr)
    , m_PreviousSibling(nullptr)
    , m_FirstChild(nullptr)
    , m_DFSIndex(0)
{
    m_Flags.SetDirty(true);
    m_Flags.SetTransformDirty(true);
    m_Flags.SetLeftHanded(true);
    m_Flags.SetActive(true);
    m_Flags.SetLocallyPickable(true);

    // for ( SNode* theChild = m_FirstChild; theChild != nullptr; theChild = theChild->m_NextSibling )
    //{
    //	SNode* theClonedChild = static_cast<SNode*>( CGraphObjectFactory::CloneGraphObject(
    //*theChild, inAllocator ) );
    //	AddChild( *theClonedChild );
    //}
}

// Sets this object dirty and walks down the graph setting all
// children who are not dirty to be dirty.
void SNode::MarkDirty(NodeTransformDirtyFlag::Enum inTransformDirty)
{
    if (m_Flags.IsTransformDirty() == false)
        m_Flags.SetTransformDirty(inTransformDirty != NodeTransformDirtyFlag::TransformNotDirty);
    if (m_Flags.IsDirty() == false) {
        m_Flags.SetDirty(true);
        for (SNode *child = m_FirstChild; child; child = child->m_NextSibling)
            child->MarkDirty(inTransformDirty);
    }
}

// Calculate global transform and opacity
// Walks up the graph ensure all parents are not dirty so they have
// valid global transforms.

bool SNode::CalculateGlobalVariables()
{
    bool retval = m_Flags.IsDirty();
    if (retval) {
        m_Flags.SetDirty(false);
        if (m_Flags.IsTransformDirty())
            CalculateLocalTransform();
        m_GlobalOpacity = m_LocalOpacity;
        if (m_Parent) {
            // Layer transforms do not flow down but affect the final layer's rendered
            // representation.
            retval = m_Parent->CalculateGlobalVariables() || retval;
            if (m_Parent->m_Type != GraphObjectTypes::Layer) {
                m_GlobalOpacity *= m_Parent->m_GlobalOpacity;
                if (m_Flags.IsIgnoreParentTransform() == false)
                    m_GlobalTransform = m_Parent->m_GlobalTransform * m_LocalTransform;
                else
                    m_GlobalTransform = m_LocalTransform;
            } else
                m_GlobalTransform = m_LocalTransform;

            m_Flags.SetGlobalActive(m_Flags.IsActive() && m_Parent->m_Flags.IsGloballyActive());
            m_Flags.SetGloballyPickable(m_Flags.IsLocallyPickable()
                                        || m_Parent->m_Flags.IsGloballyPickable());
        } else {
            m_GlobalTransform = m_LocalTransform;
            m_Flags.SetGlobalActive(m_Flags.IsActive());
            m_Flags.SetGloballyPickable(m_Flags.IsLocallyPickable());
        }
    }
    // We always clear dirty in a reasonable manner but if we aren't active
    // there is no reason to tell the universe if we are dirty or not.
    return retval && m_Flags.IsActive();
}

// Create some mapping of euler angles to their axis mapping.
#define ITERATE_POSSIBLE_EULER_ANGLES                                                              \
    HANDLE_EULER_ANGLE(EulOrdXYZs, X, Y, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXYXs, X, Y, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXZYs, X, Z, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXZXs, X, Z, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYZXs, Y, Z, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYZYs, Y, Z, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYXZs, Y, X, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYXYs, Y, X, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZXYs, Z, X, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZXZs, Z, X, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZYXs, Z, Y, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZYZs, Z, Y, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZYXr, Z, Y, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXYXr, X, Y, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYZXr, Y, Z, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXZXr, X, Z, X)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXZYr, X, Z, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYZYr, Y, Z, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZXYr, Z, X, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYXYr, Y, X, Y)                                                        \
    HANDLE_EULER_ANGLE(EulOrdYXZr, Y, X, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZXZr, Z, X, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdXYZr, X, Y, Z)                                                        \
    HANDLE_EULER_ANGLE(EulOrdZYZr, Z, Y, Z)

inline EulerAngles RotationAndOrderToShoemake(QVector3D inRotation, quint32 inOrder)
{
    EulerAngles retval;
    retval.w = (float)inOrder;
    int X = 0;
    int Y = 1;
    int Z = 2;

    switch (inOrder) {
#define HANDLE_EULER_ANGLE(order, xIdx, yIdx, zIdx)                                                \
    case order:                                                                                    \
        retval.x = -inRotation[xIdx];                                                              \
        retval.y = -inRotation[yIdx];                                                              \
        retval.z = -inRotation[zIdx];                                                              \
        break;
        ITERATE_POSSIBLE_EULER_ANGLES
#undef HANDLE_EULER_ANGLE
    default:
        Q_ASSERT(false);
        retval.x = inRotation[X];
        retval.y = inRotation[Y];
        retval.z = inRotation[Z];
        break;
    }
    return retval;
}

QVector3D SNode::GetRotationVectorFromRotationMatrix(const QMatrix3x3 &inMatrix) const
{
    QMatrix4x4 theConvertMatrix(inMatrix, QVector3D(0, 0, 0));
    if (m_Flags.IsLeftHanded())
        SNode::FlipCoordinateSystem(theConvertMatrix);
    CEulerAngleConverter theConverter;
    HMatrix *theHMatrix =
        reinterpret_cast<HMatrix *>(theConvertMatrix.front());
    EulerAngles theAngles = theConverter.Eul_FromHMatrix(*theHMatrix, m_RotationOrder);
    return GetRotationVectorFromEulerAngles(theAngles);
}

QVector3D SNode::GetRotationVectorFromEulerAngles(const EulerAngles &inAngles)
{
    QVector3D retval(0, 0, 0);
    int X = 0;
    int Y = 1;
    int Z = 2;
    switch ((int)inAngles.w) {
#define HANDLE_EULER_ANGLE(order, xIdx, yIdx, zIdx)                                                \
    case order:                                                                                    \
        retval[xIdx] = -inAngles.x;                                                                \
        retval[yIdx] = -inAngles.y;                                                                \
        retval[zIdx] = -inAngles.z;                                                                \
        break;
        ITERATE_POSSIBLE_EULER_ANGLES
#undef HANDLE_EULER_ANGLE
    default:
        Q_ASSERT(false);
        retval.x = inAngles.x;
        retval.y = inAngles.y;
        retval.z = inAngles.z;
        break;
    }

    return retval;
}

void SNode::CalculateRotationMatrix(QMatrix4x4 &outMatrix) const
{
    StaticAssert<sizeof(QMatrix4x4) == sizeof(HMatrix)>::valid_expression();
    CEulerAngleConverter theConverter;
    EulerAngles theAngles(RotationAndOrderToShoemake(m_Rotation, (int)m_RotationOrder));
    HMatrix *theMatrix = reinterpret_cast<HMatrix *>(&outMatrix);
    theConverter.Eul_ToHMatrix(theAngles, *theMatrix);
}

void SNode::FlipCoordinateSystem(QMatrix4x4 &inMatrix)
{
    float *writePtr(inMatrix.front());
    // rotation conversion
    writePtr[0 * 4 + 2] *= -1;
    writePtr[1 * 4 + 2] *= -1;
    writePtr[2 * 4 + 0] *= -1;
    writePtr[2 * 4 + 1] *= -1;

    // translation conversion
    writePtr[3 * 4 + 2] *= -1;
}

void SNode::CalculateLocalTransform()
{
    m_Flags.SetTransformDirty(false);
    bool leftHanded = m_Flags.IsLeftHanded();
    m_LocalTransform = QMatrix4x4::createIdentity();
    m_GlobalTransform = m_LocalTransform;
    float *writePtr = m_LocalTransform.front();
    QVector3D theScaledPivot(-m_Pivot[0] * m_Scale[0], -m_Pivot[1] * m_Scale[1],
                          -m_Pivot[2] * m_Scale[2]);
    m_LocalTransform.column0[0] = m_Scale[0];
    m_LocalTransform.column1[1] = m_Scale[1];
    m_LocalTransform.column2[2] = m_Scale[2];

    writePtr[12] = theScaledPivot[0];
    writePtr[13] = theScaledPivot[1];
    if (leftHanded)
        writePtr[14] = theScaledPivot[2];
    else
        writePtr[14] = -theScaledPivot[2];

    QMatrix4x4 theRotationTransform;
    CalculateRotationMatrix(theRotationTransform);
    // may need column conversion in here somewhere.
    m_LocalTransform = theRotationTransform * m_LocalTransform;

    writePtr[12] += m_Position[0];
    writePtr[13] += m_Position[1];
    if (leftHanded)
        writePtr[14] = writePtr[14] + m_Position[2];
    else
        writePtr[14] = writePtr[14] - m_Position[2];

    if (leftHanded) {
        FlipCoordinateSystem(m_LocalTransform);
    }
}

void SNode::SetLocalTransformFromMatrix(QMatrix4x4 &inTransform)
{
    m_Flags.SetTransformDirty(true);

    // clear pivot
    m_Pivot[0] = m_Pivot[1] = m_Pivot[2] = 0.0f;

    // set translation
    m_Position[0] = inTransform[3][0];
    m_Position[1] = inTransform[3][1];
    m_Position[2] = inTransform[3][2];
    // set scale
    m_Scale[0] = inTransform.column0.magnitude();
    m_Scale[1] = inTransform.column1.magnitude();
    m_Scale[2] = inTransform.column2.magnitude();

    // make sure there is no zero value
    m_Scale[0] = (m_Scale[0] == 0.0) ? 1.0f : m_Scale[0];
    m_Scale[1] = (m_Scale[1] == 0.0) ? 1.0f : m_Scale[1];
    m_Scale[2] = (m_Scale[2] == 0.0) ? 1.0f : m_Scale[2];

    // extract rotation by first dividing through scale value
    float invScaleX = 1.0f / m_Scale[0];
    float invScaleY = 1.0f / m_Scale[1];
    float invScaleZ = 1.0f / m_Scale[2];

    inTransform[0][0] *= invScaleX;
    inTransform[0][1] *= invScaleX;
    inTransform[0][2] *= invScaleX;
    inTransform[1][0] *= invScaleY;
    inTransform[1][1] *= invScaleY;
    inTransform[1][2] *= invScaleY;
    inTransform[2][0] *= invScaleZ;
    inTransform[2][1] *= invScaleZ;
    inTransform[2][2] *= invScaleZ;

    QMatrix3x3 theRotationMatrix(inTransform.column0.getXYZ(), inTransform.column1.getXYZ(),
                              inTransform.column2.getXYZ());
    m_Rotation = GetRotationVectorFromRotationMatrix(theRotationMatrix);
}

void SNode::AddChild(SNode &inChild)
{
    if (inChild.m_Parent)
        inChild.m_Parent->RemoveChild(inChild);
    inChild.m_Parent = this;
    if (m_FirstChild == nullptr) {
        m_FirstChild = &inChild;
        inChild.m_NextSibling = nullptr;
        inChild.m_PreviousSibling = nullptr;
    } else {
        SNode *lastChild = GetLastChild();
        if (lastChild) {
            lastChild->m_NextSibling = &inChild;
            inChild.m_PreviousSibling = lastChild;
            inChild.m_NextSibling = nullptr;
        } else {
            Q_ASSERT(false); // no last child but first child isn't null?
        }
    }
}

void SNode::RemoveChild(SNode &inChild)
{
    if (inChild.m_Parent != this) {
        Q_ASSERT(false);
        return;
    }
    for (SNode *child = m_FirstChild; child; child = child->m_NextSibling) {
        if (child == &inChild) {
            if (child->m_PreviousSibling)
                child->m_PreviousSibling->m_NextSibling = child->m_NextSibling;
            if (child->m_NextSibling)
                child->m_NextSibling->m_PreviousSibling = child->m_PreviousSibling;
            child->m_Parent = nullptr;
            if (m_FirstChild == child)
                m_FirstChild = child->m_NextSibling;
            child->m_NextSibling = nullptr;
            child->m_PreviousSibling = nullptr;
            return;
        }
    }
    Q_ASSERT(false);
}

SNode *SNode::GetLastChild()
{
    SNode *lastChild = nullptr;
    // empty loop intentional
    for (lastChild = m_FirstChild; lastChild && lastChild->m_NextSibling;
         lastChild = lastChild->m_NextSibling) {
    }
    return lastChild;
}

void SNode::RemoveFromGraph()
{
    if (m_Parent)
        m_Parent->RemoveChild(*this);

    m_NextSibling = nullptr;

    // Orphan all of my children.
    SNode *nextSibling = nullptr;
    for (SNode *child = m_FirstChild; child != nullptr; child = nextSibling) {
        child->m_PreviousSibling = nullptr;
        child->m_Parent = nullptr;
        nextSibling = child->m_NextSibling;
        child->m_NextSibling = nullptr;
    }
}

NVBounds3 SNode::GetBounds(IBufferManager &inManager, IPathManager &inPathManager,
                           bool inIncludeChildren, IQt3DSRenderNodeFilter *inChildFilter) const
{
    NVBounds3 retval;
    retval.setEmpty();
    if (inIncludeChildren)
        retval = GetChildBounds(inManager, inPathManager, inChildFilter);

    if (m_Type == GraphObjectTypes::Model)
        retval.include(static_cast<const SModel *>(this)->GetModelBounds(inManager));
    else if (m_Type == GraphObjectTypes::Text)
        retval.include(static_cast<const SText *>(this)->GetTextBounds());
    else if (m_Type == GraphObjectTypes::Path)
        retval.include(inPathManager.GetBounds(*static_cast<const SPath *>(this)));
    return retval;
}

NVBounds3 SNode::GetChildBounds(IBufferManager &inManager, IPathManager &inPathManager,
                                IQt3DSRenderNodeFilter *inChildFilter) const
{
    NVBounds3 retval;
    retval.setEmpty();
    for (SNode *child = m_FirstChild; child != nullptr; child = child->m_NextSibling) {
        if (inChildFilter == nullptr || inChildFilter->IncludeNode(*child)) {
            NVBounds3 childBounds;
            if (child->m_Flags.IsTransformDirty())
                child->CalculateLocalTransform();
            childBounds = child->GetBounds(inManager, inPathManager);
            if (childBounds.isEmpty() == false) {
                // Transform the bounds into our local space.
                childBounds.transform(child->m_LocalTransform);
                retval.include(childBounds);
            }
        }
    }
    return retval;
}

QVector3D SNode::GetGlobalPos() const
{
    return m_GlobalTransform.getPosition();
}

QVector3D SNode::GetDirection() const
{
    const float *dataPtr(m_GlobalTransform.front());
    QVector3D retval(dataPtr[8], dataPtr[9], dataPtr[10]);
    retval.normalize();
    return retval;
}

QVector3D SNode::GetScalingCorrectDirection() const
{
    QMatrix3x3 theDirMatrix(m_GlobalTransform.getUpper3x3().getInverse().getTranspose());
    QVector3D theOriginalDir(0, 0, -1);
    QVector3D retval = theDirMatrix.transform(theOriginalDir);
    retval.normalize();
    return retval;
}

QVector3D SNode::GetGlobalPivot() const
{
    QVector3D retval(m_Position);
    retval.z *= -1;

    if (m_Parent && m_Parent->m_Type != GraphObjectTypes::Layer)
        return m_Parent->m_GlobalTransform.transform(retval);

    return retval;
}

void SNode::CalculateMVPAndNormalMatrix(const QMatrix4x4 &inViewProjection, QMatrix4x4 &outMVP,
                                        QMatrix3x3 &outNormalMatrix) const
{
    outMVP = inViewProjection * m_GlobalTransform;
    CalculateNormalMatrix(outNormalMatrix);
}

void SNode::GetMatrixUpper3x3(QMatrix3x3 &outDest, const QMatrix4x4 &inSrc)
{
    outDest.column0 = QVector3D(inSrc.column0[0], inSrc.column0[1], inSrc.column0[2]);
    outDest.column1 = QVector3D(inSrc.column1[0], inSrc.column1[1], inSrc.column1[2]);
    outDest.column2 = QVector3D(inSrc.column2[0], inSrc.column2[1], inSrc.column2[2]);
}

void SNode::CalculateNormalMatrix(QMatrix3x3 &outNormalMatrix) const
{
    GetMatrixUpper3x3(outNormalMatrix, m_GlobalTransform);
    outNormalMatrix = outNormalMatrix.getInverse().getTranspose();
}
