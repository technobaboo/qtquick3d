/****************************************************************************
**
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

#include "qdemoncamera.h"

#include <QtDemonRuntimeRender/qdemonrendercamera.h>

#include <QtMath>
#include <qdemonutils.h>

QT_BEGIN_NAMESPACE
/*!
    \qmltype DemonCamera
    \inqmlmodule QtDemon
    \brief Lets you add a camera to view the 3D content
*/
QDemonCamera::QDemonCamera() {}

float QDemonCamera::clipNear() const
{
    return m_clipNear;
}

float QDemonCamera::clipFar() const
{
    return m_clipFar;
}

float QDemonCamera::fieldOfView() const
{
    return m_fieldOfView;
}

bool QDemonCamera::isFieldOFViewHorizontal() const
{
    return m_isFieldOFViewHorizontal;
}

QDemonCamera::QDemonCameraScaleModes QDemonCamera::scaleMode() const
{
    return m_scaleMode;
}

QDemonCamera::QDemonCameraScaleAnchors QDemonCamera::scaleAnchor() const
{
    return m_scaleAnchor;
}

QDemonObject::Type QDemonCamera::type() const
{
    return QDemonObject::Camera;
}

QDemonRenderCamera *QDemonCamera::getCameraNode() const
{
    return m_cameraNode;
}

QDemonCamera::QDemonCameraProjectionMode QDemonCamera::projectionMode() const
{
    return m_projectionMode;
}

void QDemonCamera::setClipNear(float clipNear)
{
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;
    emit clipNearChanged(m_clipNear);
    update();
}

void QDemonCamera::setClipFar(float clipFar)
{
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;
    emit clipFarChanged(m_clipFar);
    update();
}

void QDemonCamera::setFieldOfView(float fieldOfView)
{
    if (qFuzzyCompare(m_fieldOfView, fieldOfView))
        return;

    m_fieldOfView = fieldOfView;
    emit fieldOfViewChanged(m_fieldOfView);
    update();
}

void QDemonCamera::setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal)
{
    if (m_isFieldOFViewHorizontal == isFieldOFViewHorizontal)
        return;

    m_isFieldOFViewHorizontal = isFieldOFViewHorizontal;
    emit isFieldOFViewHorizontalChanged(m_isFieldOFViewHorizontal);
    update();
}

void QDemonCamera::setScaleMode(QDemonCamera::QDemonCameraScaleModes scaleMode)
{
    if (m_scaleMode == scaleMode)
        return;

    m_scaleMode = scaleMode;
    emit scaleModeChanged(m_scaleMode);
    update();
}

void QDemonCamera::setScaleAnchor(QDemonCamera::QDemonCameraScaleAnchors scaleAnchor)
{
    if (m_scaleAnchor == scaleAnchor)
        return;

    m_scaleAnchor = scaleAnchor;
    emit scaleAnchorChanged(m_scaleAnchor);
    update();
}


void QDemonCamera::setProjectionMode(QDemonCamera::QDemonCameraProjectionMode projectionMode)
{
    if (m_projectionMode == projectionMode)
        return;

    m_projectionMode = projectionMode;
    emit projectionModeChanged(m_projectionMode);
    update();
}

void QDemonCamera::setEnableFrustumCulling(bool enableFrustumCulling)
{
    if (m_enableFrustumCulling == enableFrustumCulling)
        return;

    m_enableFrustumCulling = enableFrustumCulling;
    emit enableFrustumCullingChanged(m_enableFrustumCulling);
    update();
}

/*!
 * Transforms \a worldPos from world space into viewport space. The position
 * is normalized, with the top-left of the viewport being [0,0] and
 * the botton-right being [1,1]. The returned z value will contain the distance from the
 * back of the frustum (clipNear) to \a worldPos in world units. If \a worldPos
 * cannot be mapped to a position in the viewport, a position of [0, 0, 0] is returned.
 *
 * \sa QDemonView3D::worldToView QDemonCamera::viewportToWorld
 */
QVector3D QDemonCamera::worldToViewport(const QVector3D &worldPos) const
{
    if (!m_cameraNode)
        return QVector3D(-1, -1, -1);

    // Convert from left-handed to right-handed
    const QVector4D worldPosRightHand(worldPos.x(), worldPos.y(), -worldPos.z(), 1);

    // Transform position
    const QMatrix4x4 worldToCamera = m_cameraNode->globalTransform.inverted();
    const QMatrix4x4 projectionViewMatrix = m_cameraNode->projection * worldToCamera;
    const QVector4D transformedWorldPos = mat44::transform(projectionViewMatrix, worldPosRightHand);

    if (qFuzzyIsNull(transformedWorldPos.w()))
        return QVector3D(0, 0, 0);

    // Normalize worldPosView between [-1, 1]
    QVector3D worldPosView = transformedWorldPos.toVector3D() / transformedWorldPos.w();

    // Set z to be the world distance from clipNear so that the return value
    // can be used as argument to viewportToWorld() to reverse the call.
    const QVector4D clipNearPos(worldPosView.x(), worldPosView.y(), -1, 1);
    const QVector4D clipNearPosTransformed = mat44::transform(projectionViewMatrix.inverted(), clipNearPos);
    const QVector4D clipNearPosWorld = clipNearPosTransformed / clipNearPosTransformed.w();
    const float distanceToWorldPos = (worldPosRightHand - clipNearPosWorld).length();
    worldPosView.setZ(distanceToWorldPos);

    // Convert x and y to be between [0, 1]
    worldPosView.setX((worldPosView.x() / 2) + 0.5f);
    worldPosView.setY((worldPosView.y() / 2) + 0.5f);
    // And convert origin from bottom-left to top-left
    worldPosView.setY(1 - worldPosView.y());
    return worldPosView;
}

/*!
 * Transforms \a viewportPos from viewport space into world space. \a The x-, and y
 * values of \l viewportPos needs to be normalized, with the top-left of the viewport
 * being [0,0] and the botton-right being [1,1]. The z value should be the distance
 * from the back of the frustum (clipNear) into the world in world units.
 * If \a viewportPos cannot be mapped to a position in the world, a position of
 * [0, 0, 0] is returned.
 *
 * \sa QDemonView3D::viewToWorld QDemonCamera::worldToViewport
 */
QVector3D QDemonCamera::viewportToWorld(const QVector3D &viewportPos) const
{
    if (!m_cameraNode)
        return QVector3D(-1, -1, -1);

    // Pick two positions in the frustum
    QVector4D clipNearPos(viewportPos, 1);
    // Convert origin from top-left to bottom-left
    clipNearPos.setY(1 - clipNearPos.y());
    // Convert to homogenous position between [-1, 1]
    clipNearPos.setX((clipNearPos.x() * 2.0f) - 1.0f);
    clipNearPos.setY((clipNearPos.y() * 2.0f) - 1.0f);
    QVector4D clipFarPos = clipNearPos;
    // clipNear: z = -1, clipFar: z = 1. It's recommended to use 0 as
    // far pos instead of clipFar because of infinite projection issues.
    clipNearPos.setZ(-1);
    clipFarPos.setZ(0);

    // Transform position to world
    const QMatrix4x4 worldToCamera = m_cameraNode->globalTransform.inverted();
    const QMatrix4x4 projectionViewMatrixInv = (m_cameraNode->projection * worldToCamera).inverted();
    const QVector4D transformedClipNearPos = mat44::transform(projectionViewMatrixInv, clipNearPos);
    const QVector4D transformedClipFarPos = mat44::transform(projectionViewMatrixInv, clipFarPos);

    if (qFuzzyIsNull(transformedClipNearPos.w()))
        return QVector3D(0, 0, 0);

    // Reverse the projection
    const QVector3D clipNearPosWorld = transformedClipNearPos.toVector3D() / transformedClipNearPos.w();
    const QVector3D clipFarPosWorld = transformedClipFarPos.toVector3D() / transformedClipFarPos.w();

    // Calculate the position in the world
    const QVector3D direction = (clipFarPosWorld - clipNearPosWorld).normalized();
    const float distanceFromClipNear = viewportPos.z();
    QVector3D worldPos = clipNearPosWorld + (direction * distanceFromClipNear);
    // Convert right-handt to left-hand
    worldPos.setZ(-worldPos.z());
    return worldPos;
}

bool QDemonCamera::enableFrustumCulling() const
{
    return m_enableFrustumCulling;
}

QDemonRenderGraphObject *QDemonCamera::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderCamera();

    QDemonNode::updateSpatialNode(node);

    QDemonRenderCamera *camera = static_cast<QDemonRenderCamera *>(node);

    camera->clipNear = m_clipNear;
    camera->clipFar = m_clipFar;
    camera->fov = qDegreesToRadians(m_fieldOfView);
    camera->fovHorizontal = m_isFieldOFViewHorizontal;

    camera->scaleMode = QDemonRenderCamera::ScaleModes(m_scaleMode);
    camera->scaleAnchor = QDemonRenderCamera::ScaleAnchors(m_scaleAnchor);
    camera->enableFrustumClipping = m_enableFrustumCulling;

    if (m_projectionMode == Orthographic)
        camera->flags.setFlag(QDemonRenderNode::Flag::Orthographic);

    m_cameraNode = camera;

    return node;
}

QT_END_NAMESPACE
