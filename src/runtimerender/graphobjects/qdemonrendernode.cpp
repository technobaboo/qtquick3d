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

#include "qdemonrendernode.h"

#include <QtDemon/qdemonutils.h>
#include <QtDemonRuntimeRender/qdemonrendereulerangles.h>

#include <QtDemonRuntimeRender/qdemonrendermodel.h>

#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrendertext.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>

QT_BEGIN_NAMESPACE

INodeQueue::~INodeQueue() = default;

QDemonGraphNode::QDemonGraphNode(QDemonGraphObjectTypes::Enum inGraphObjectType)
    : QDemonGraphObject(inGraphObjectType)
    , rotation(0, 0, 0) // Radians
    , position(0, 0, 0)
    , scale(1, 1, 1)
    , pivot(0, 0, 0)
    , rotationOrder(EulOrdYXZs)
    , localOpacity(1.0f)
    , globalOpacity(1.0f)
    , skeletonId(-1)
    , parent(nullptr)
    , nextSibling(nullptr)
    , previousSibling(nullptr)
    , firstChild(nullptr)
    , dfsIndex(0)
{
    flags.setDirty(true);
    flags.setTransformDirty(true);
    flags.setLeftHanded(true);
    flags.setActive(true);
    flags.setLocallyPickable(true);
}

QDemonGraphNode::QDemonGraphNode(const QDemonGraphNode &inCloningObject)
    : QDemonGraphObject(inCloningObject)
    , rotation(inCloningObject.rotation) // Radians
    , position(inCloningObject.position)
    , scale(inCloningObject.scale)
    , pivot(inCloningObject.pivot)
    , rotationOrder(inCloningObject.rotationOrder)
    , localOpacity(inCloningObject.localOpacity)
    , localTransform(inCloningObject.localTransform)
    , globalTransform(inCloningObject.globalTransform)
    , globalOpacity(inCloningObject.globalOpacity)
    , skeletonId(inCloningObject.skeletonId)
    , parent(nullptr)
    , nextSibling(nullptr)
    , previousSibling(nullptr)
    , firstChild(nullptr)
    , dfsIndex(0)
{
    flags.setDirty(true);
    flags.setTransformDirty(true);
    flags.setLeftHanded(true);
    flags.setActive(true);
    flags.setLocallyPickable(true);

    // for ( SNode* theChild = m_FirstChild; theChild != nullptr; theChild = theChild->m_NextSibling )
    //{
    //	SNode* theClonedChild = static_cast<SNode*>( CGraphObjectFactory::CloneGraphObject(
    //*theChild, inAllocator ) );
    //	AddChild( *theClonedChild );
    //}
}

// Sets this object dirty and walks down the graph setting all
// children who are not dirty to be dirty.
void QDemonGraphNode::markDirty(NodeTransformDirtyFlag::Enum inTransformDirty)
{
    if (flags.isTransformDirty() == false)
        flags.setTransformDirty(inTransformDirty != NodeTransformDirtyFlag::TransformNotDirty);
    if (flags.isDirty() == false) {
        flags.setDirty(true);
        for (QDemonGraphNode *child = firstChild; child; child = child->nextSibling)
            child->markDirty(inTransformDirty);
    }
}

// Calculate global transform and opacity
// Walks up the graph ensure all parents are not dirty so they have
// valid global transforms.

bool QDemonGraphNode::calculateGlobalVariables()
{
    bool retval = flags.isDirty();
    if (retval) {
        flags.setDirty(false);
        if (flags.isTransformDirty())
            calculateLocalTransform();
        globalOpacity = localOpacity;
        if (parent) {
            // Layer transforms do not flow down but affect the final layer's rendered
            // representation.
            retval = parent->calculateGlobalVariables() || retval;
            if (parent->type != QDemonGraphObjectTypes::Layer) {
                globalOpacity *= parent->globalOpacity;
                if (flags.isIgnoreParentTransform() == false)
                    globalTransform = parent->globalTransform * localTransform;
                else
                    globalTransform = localTransform;
            } else
                globalTransform = localTransform;

            flags.setGlobalActive(flags.isActive() && parent->flags.isGloballyActive());
            flags.setGloballyPickable(flags.isLocallyPickable() || parent->flags.isGloballyPickable());
        } else {
            globalTransform = localTransform;
            flags.setGlobalActive(flags.isActive());
            flags.setGloballyPickable(flags.isLocallyPickable());
        }
    }
    // We always clear dirty in a reasonable manner but if we aren't active
    // there is no reason to tell the universe if we are dirty or not.
    return retval && flags.isActive();
}

// Create some mapping of euler angles to their axis mapping.
#define ITERATE_POSSIBLE_EULER_ANGLES                                                                                  \
    HANDLE_EULER_ANGLE(EulOrdXYZs, X, Y, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXYXs, X, Y, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXZYs, X, Z, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXZXs, X, Z, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYZXs, Y, Z, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYZYs, Y, Z, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYXZs, Y, X, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYXYs, Y, X, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZXYs, Z, X, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZXZs, Z, X, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZYXs, Z, Y, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZYZs, Z, Y, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZYXr, Z, Y, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXYXr, X, Y, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYZXr, Y, Z, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXZXr, X, Z, X)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXZYr, X, Z, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYZYr, Y, Z, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZXYr, Z, X, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYXYr, Y, X, Y)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdYXZr, Y, X, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZXZr, Z, X, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdXYZr, X, Y, Z)                                                                            \
    HANDLE_EULER_ANGLE(EulOrdZYZr, Z, Y, Z)

inline EulerAngles rotationAndOrderToShoemake(QVector3D inRotation, quint32 inOrder)
{
    EulerAngles retval;
    retval.w = float(inOrder);
    int X = 0;
    int Y = 1;
    int Z = 2;

    switch (inOrder) {
#define HANDLE_EULER_ANGLE(order, xIdx, yIdx, zIdx)                                                                    \
    case order:                                                                                                        \
        retval.x = -inRotation[xIdx];                                                                                  \
        retval.y = -inRotation[yIdx];                                                                                  \
        retval.z = -inRotation[zIdx];                                                                                  \
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

QVector3D QDemonGraphNode::getRotationVectorFromRotationMatrix(const QMatrix3x3 &inMatrix) const
{
    float theConvertMatrixData[16] = { inMatrix(0, 0),
                                       inMatrix(0, 1),
                                       inMatrix(0, 2),
                                       0.0f,
                                       inMatrix(1, 0),
                                       inMatrix(1, 1),
                                       inMatrix(1, 2),
                                       0.0f,
                                       inMatrix(2, 0),
                                       inMatrix(2, 1),
                                       inMatrix(2, 2),
                                       0.0f,
                                       0,
                                       0,
                                       0,
                                       1 };

    QMatrix4x4 theConvertMatrix(theConvertMatrixData);
    if (flags.isLeftHanded())
        QDemonGraphNode::flipCoordinateSystem(theConvertMatrix);
    QDemonEulerAngleConverter theConverter;
    HMatrix *theHMatrix = reinterpret_cast<HMatrix *>(theConvertMatrix.data());
    EulerAngles theAngles = theConverter.eulerFromHMatrix(*theHMatrix, rotationOrder);
    return getRotationVectorFromEulerAngles(theAngles);
}

QVector3D QDemonGraphNode::getRotationVectorFromEulerAngles(const EulerAngles &inAngles)
{
    QVector3D retval(0, 0, 0);
    int X = 0;
    int Y = 1;
    int Z = 2;
    switch (int(inAngles.w)) {
#define HANDLE_EULER_ANGLE(order, xIdx, yIdx, zIdx)                                                                    \
    case order:                                                                                                        \
        retval[xIdx] = -inAngles.x;                                                                                    \
        retval[yIdx] = -inAngles.y;                                                                                    \
        retval[zIdx] = -inAngles.z;                                                                                    \
        break;
        ITERATE_POSSIBLE_EULER_ANGLES
#undef HANDLE_EULER_ANGLE
    default:
        Q_ASSERT(false);
        retval.setX(inAngles.x);
        retval.setY(inAngles.y);
        retval.setZ(inAngles.z);
        break;
    }

    return retval;
}

void QDemonGraphNode::calculateRotationMatrix(QMatrix4x4 &outMatrix) const
{
    QDemonEulerAngleConverter theConverter;
    EulerAngles theAngles(rotationAndOrderToShoemake(rotation, int(rotationOrder)));
    HMatrix *theMatrix = reinterpret_cast<HMatrix *>(&outMatrix);
    theConverter.eulerToHMatrix(theAngles, *theMatrix);
}

void QDemonGraphNode::flipCoordinateSystem(QMatrix4x4 &inMatrix)
{
    float *writePtr(inMatrix.data());
    // rotation conversion
    writePtr[0 * 4 + 2] *= -1;
    writePtr[1 * 4 + 2] *= -1;
    writePtr[2 * 4 + 0] *= -1;
    writePtr[2 * 4 + 1] *= -1;

    // translation conversion
    writePtr[3 * 4 + 2] *= -1;
}

void QDemonGraphNode::calculateLocalTransform()
{
    flags.setTransformDirty(false);
    bool leftHanded = flags.isLeftHanded();
    localTransform = QMatrix4x4();
    globalTransform = localTransform;
    float *writePtr = localTransform.data();
    QVector3D theScaledPivot(-pivot[0] * scale[0], -pivot[1] * scale[1], -pivot[2] * scale[2]);
    localTransform(0, 0) = scale[0];
    localTransform(1, 1) = scale[1];
    localTransform(2, 2) = scale[2];

    writePtr[12] = theScaledPivot[0];
    writePtr[13] = theScaledPivot[1];
    if (leftHanded)
        writePtr[14] = theScaledPivot[2];
    else
        writePtr[14] = -theScaledPivot[2];

    QMatrix4x4 theRotationTransform;
    calculateRotationMatrix(theRotationTransform);
    // may need column conversion in here somewhere.
    localTransform = theRotationTransform * localTransform;

    writePtr[12] += position[0];
    writePtr[13] += position[1];
    if (leftHanded)
        writePtr[14] = writePtr[14] + position[2];
    else
        writePtr[14] = writePtr[14] - position[2];

    if (leftHanded) {
        flipCoordinateSystem(localTransform);
    }
}

void QDemonGraphNode::setLocalTransformFromMatrix(QMatrix4x4 &inTransform)
{
    flags.setTransformDirty(true);

    // clear pivot
    pivot[0] = pivot[1] = pivot[2] = 0.0f;

    // set translation
    position[0] = inTransform(3, 0);
    position[1] = inTransform(3, 1);
    position[2] = inTransform(3, 2);
    // set scale
    const QVector3D column0(inTransform(0, 0), inTransform(0, 1), inTransform(0, 2));
    const QVector3D column1(inTransform(1, 0), inTransform(1, 1), inTransform(1, 2));
    const QVector3D column2(inTransform(2, 0), inTransform(2, 1), inTransform(2, 2));
    scale[0] = vec3::magnitude(column0);
    scale[1] = vec3::magnitude(column1);
    scale[2] = vec3::magnitude(column2);
    // make sure there is no zero value
    scale[0] = (scale[0] == 0.0f) ? 1.0f : scale[0];
    scale[1] = (scale[1] == 0.0f) ? 1.0f : scale[1];
    scale[2] = (scale[2] == 0.0f) ? 1.0f : scale[2];

    // extract rotation by first dividing through scale value
    float invScaleX = 1.0f / scale[0];
    float invScaleY = 1.0f / scale[1];
    float invScaleZ = 1.0f / scale[2];

    inTransform(0, 0) *= invScaleX;
    inTransform(0, 1) *= invScaleX;
    inTransform(0, 2) *= invScaleX;
    inTransform(1, 0) *= invScaleY;
    inTransform(1, 1) *= invScaleY;
    inTransform(1, 2) *= invScaleY;
    inTransform(2, 0) *= invScaleZ;
    inTransform(2, 1) *= invScaleZ;
    inTransform(2, 2) *= invScaleZ;

    float rotationMatrixData[9] = { inTransform(0, 0), inTransform(0, 1), inTransform(0, 2),
                                    inTransform(1, 0), inTransform(1, 1), inTransform(1, 2),
                                    inTransform(2, 0), inTransform(2, 1), inTransform(2, 2) };

    QMatrix3x3 theRotationMatrix(rotationMatrixData);
    rotation = getRotationVectorFromRotationMatrix(theRotationMatrix);
}

void QDemonGraphNode::addChild(QDemonGraphNode &inChild)
{
    if (inChild.parent)
        inChild.parent->removeChild(inChild);
    inChild.parent = this;
    if (firstChild == nullptr) {
        firstChild = &inChild;
        inChild.nextSibling = nullptr;
        inChild.previousSibling = nullptr;
    } else {
        QDemonGraphNode *lastChild = getLastChild();
        if (lastChild) {
            lastChild->nextSibling = &inChild;
            inChild.previousSibling = lastChild;
            inChild.nextSibling = nullptr;
        } else {
            Q_ASSERT(false); // no last child but first child isn't null?
        }
    }
}

void QDemonGraphNode::removeChild(QDemonGraphNode &inChild)
{
    if (inChild.parent != this) {
        Q_ASSERT(false);
        return;
    }
    for (QDemonGraphNode *child = firstChild; child; child = child->nextSibling) {
        if (child == &inChild) {
            if (child->previousSibling)
                child->previousSibling->nextSibling = child->nextSibling;
            if (child->nextSibling)
                child->nextSibling->previousSibling = child->previousSibling;
            child->parent = nullptr;
            if (firstChild == child)
                firstChild = child->nextSibling;
            child->nextSibling = nullptr;
            child->previousSibling = nullptr;
            return;
        }
    }
    Q_ASSERT(false);
}

QDemonGraphNode *QDemonGraphNode::getLastChild()
{
    QDemonGraphNode *lastChild = nullptr;
    // empty loop intentional
    for (lastChild = firstChild; lastChild && lastChild->nextSibling; lastChild = lastChild->nextSibling) {
    }
    return lastChild;
}

void QDemonGraphNode::removeFromGraph()
{
    if (parent)
        parent->removeChild(*this);

    nextSibling = nullptr;

    // Orphan all of my children.
    QDemonGraphNode *nextSibling = nullptr;
    for (QDemonGraphNode *child = firstChild; child != nullptr; child = nextSibling) {
        child->previousSibling = nullptr;
        child->parent = nullptr;
        nextSibling = child->nextSibling;
        child->nextSibling = nullptr;
    }
}

QDemonBounds3 QDemonGraphNode::getBounds(const QDemonBufferManager &inManager,
                                         const QDemonRef<QDemonPathManagerInterface> &inPathManager,
                                         bool inIncludeChildren,
                                         QDemonRenderNodeFilterInterface *inChildFilter) const
{
    QDemonBounds3 retval;
    retval.setEmpty();
    if (inIncludeChildren)
        retval = getChildBounds(inManager, inPathManager, inChildFilter);

    if (type == QDemonGraphObjectTypes::Model)
        retval.include(static_cast<const QDemonRenderModel *>(this)->getModelBounds(inManager));
    else if (type == QDemonGraphObjectTypes::Text)
        retval.include(static_cast<const QDemonText *>(this)->getTextBounds());
    else if (type == QDemonGraphObjectTypes::Path)
        retval.include(inPathManager->getBounds(*static_cast<const QDemonPath *>(this)));
    return retval;
}

QDemonBounds3 QDemonGraphNode::getChildBounds(const QDemonBufferManager &inManager,
                                              const QDemonRef<QDemonPathManagerInterface> &inPathManager,
                                              QDemonRenderNodeFilterInterface *inChildFilter) const
{
    QDemonBounds3 retval;
    retval.setEmpty();
    for (QDemonGraphNode *child = firstChild; child != nullptr; child = child->nextSibling) {
        if (inChildFilter == nullptr || inChildFilter->includeNode(*child)) {
            QDemonBounds3 childBounds;
            if (child->flags.isTransformDirty())
                child->calculateLocalTransform();
            childBounds = child->getBounds(inManager, inPathManager);
            if (childBounds.isEmpty() == false) {
                // Transform the bounds into our local space.
                childBounds.transform(child->localTransform);
                retval.include(childBounds);
            }
        }
    }
    return retval;
}

QVector3D QDemonGraphNode::getGlobalPos() const
{
    return QVector3D(globalTransform(0, 3), globalTransform(1, 3), globalTransform(2, 3));
}

QVector3D QDemonGraphNode::getDirection() const
{
    const float *dataPtr(globalTransform.data());
    QVector3D retval(dataPtr[8], dataPtr[9], dataPtr[10]);
    retval.normalize();
    return retval;
}

QVector3D QDemonGraphNode::getScalingCorrectDirection() const
{
    // ### This code has been checked to be corect
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();
    QVector3D theOriginalDir(0, 0, -1);
    QVector3D retval = mat33::transform(theDirMatrix, theOriginalDir);
    // Should already be normalized, but whatever
    retval.normalize();
    return retval;
}

QVector3D QDemonGraphNode::getGlobalPivot() const
{
    QVector3D retval(position);
    retval.setZ(retval.z() * -1);

    if (parent && parent->type != QDemonGraphObjectTypes::Layer) {
        const QVector4D direction(retval.x(), retval.y(), retval.z(), 1.0f);
        const QVector4D result = parent->globalTransform * direction;
        return QVector3D(result.x(), result.y(), result.z());
    }

    return retval;
}

void QDemonGraphNode::calculateMVPAndNormalMatrix(const QMatrix4x4 &inViewProjection, QMatrix4x4 &outMVP, QMatrix3x3 &outNormalMatrix) const
{
    outMVP = inViewProjection * globalTransform;
    calculateNormalMatrix(outNormalMatrix);
}

void QDemonGraphNode::calculateNormalMatrix(QMatrix3x3 &outNormalMatrix) const
{
    outNormalMatrix = mat44::getUpper3x3(globalTransform);
    outNormalMatrix = mat33::getInverse(outNormalMatrix).transposed();
}

QT_END_NAMESPACE
