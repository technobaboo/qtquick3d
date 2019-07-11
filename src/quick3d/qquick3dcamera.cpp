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

#include "qquick3dcamera.h"

#include <QtDemonRuntimeRender/qdemonrendercamera.h>

#include <QtMath>
#include <qdemonutils.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Camera
    \inqmlmodule QtQuick3D
    \brief Defines a Camera node for viewing the content of a 3D scene.
*/

/*!
 * \internal
 */
QQuick3DCamera::QQuick3DCamera() {}


/*!
 * \qmlproperty real Camera::clipNear
 *
 * This property holds the near value of the camara's view frustum.  This value determines
 * what the closest distance to the camera that items will be shown.
 *
 */

float QQuick3DCamera::clipNear() const
{
    return m_clipNear;
}

/*!
 * \qmlproperty real Camera::clipFar
 *
 * This property holds the far value of the camara's view frustum.  This value determines
 * what the furthest distance to the camera that items will be shown.
 *
 */

float QQuick3DCamera::clipFar() const
{
    return m_clipFar;
}

/*!
 * \qmlproperty real Camera::fieldOfView
 *
 * This property holds the field of view of the camera in degrees. This can be either the
 * vertical or horizontal field of view depending on if the isFieldOfViewHorizontal property
 * is set to \c true or not.
 *
 *
 */

float QQuick3DCamera::fieldOfView() const
{
    return m_fieldOfView;
}

/*!
 * \qmlproperty bool Camera::isFieldOfViewHorizontal
 *
 * This property determines if the field of view property reflects the horizontal
 * field of view. If this value is \c false then it is assumed field of view is vertical.
 *
 */

bool QQuick3DCamera::isFieldOfViewHorizontal() const
{
    return m_isFieldOfViewHorizontal;
}

/*!
 * \qmlproperty enumeration Camera::scaleMode
 *
 * This property defines how the cameras output retangle is scaled to match the viewport.
 *
 * \list
 * \li Camera.Fit
 * \li Camera.SameSize
 * \li Camera.FitHorizontal
 * \li Camera.FitVertical
 * \endlist
 *
 */

QQuick3DCamera::QDemonCameraScaleModes QQuick3DCamera::scaleMode() const
{
    return m_scaleMode;
}


/*!
 * \qmlproperty enumeration Camera::scaleAnchor
 *
 * This property sets the anchor point to perform scaling when necessary.
 *
 * \list
 * \li Camera.Center
 * \li Camera.North
 * \li Camera.NorthEast
 * \li Camera.East
 * \li Camera.SouthEast
 * \li Camera.South
 * \li Camera.SouthWest
 * \li Camera.West
 * \li Camera.NorthWest
 * \endlist
 */

QQuick3DCamera::QDemonCameraScaleAnchors QQuick3DCamera::scaleAnchor() const
{
    return m_scaleAnchor;
}

/*!
 * \internal
 */\
QQuick3DObject::Type QQuick3DCamera::type() const
{
    return QQuick3DObject::Camera;
}

/*!
 * \internal
 */\
QDemonRenderCamera *QQuick3DCamera::getCameraNode() const
{
    return m_cameraNode;
}

/*!
 * \qmlproperty enumeration Camera::projectionMode
 *
 * \list
 * \li Camera.Projection
 * \li Camera.Orthographic
 * \endlist
 *
 */

QQuick3DCamera::QDemonCameraProjectionMode QQuick3DCamera::projectionMode() const
{
    return m_projectionMode;
}

void QQuick3DCamera::setClipNear(float clipNear)
{
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;
    emit clipNearChanged(m_clipNear);
    update();
}

void QQuick3DCamera::setClipFar(float clipFar)
{
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;
    emit clipFarChanged(m_clipFar);
    update();
}

void QQuick3DCamera::setFieldOfView(float fieldOfView)
{
    if (qFuzzyCompare(m_fieldOfView, fieldOfView))
        return;

    m_fieldOfView = fieldOfView;
    emit fieldOfViewChanged(m_fieldOfView);
    update();
}

void QQuick3DCamera::setIsFieldOfViewHorizontal(bool isFieldOfViewHorizontal)
{
    if (m_isFieldOfViewHorizontal == isFieldOfViewHorizontal)
        return;

    m_isFieldOfViewHorizontal = isFieldOfViewHorizontal;
    emit isFieldOfViewHorizontalChanged(m_isFieldOfViewHorizontal);
    update();
}

void QQuick3DCamera::setScaleMode(QQuick3DCamera::QDemonCameraScaleModes scaleMode)
{
    if (m_scaleMode == scaleMode)
        return;

    m_scaleMode = scaleMode;
    emit scaleModeChanged(m_scaleMode);
    update();
}

void QQuick3DCamera::setScaleAnchor(QQuick3DCamera::QDemonCameraScaleAnchors scaleAnchor)
{
    if (m_scaleAnchor == scaleAnchor)
        return;

    m_scaleAnchor = scaleAnchor;
    emit scaleAnchorChanged(m_scaleAnchor);
    update();
}


void QQuick3DCamera::setProjectionMode(QQuick3DCamera::QDemonCameraProjectionMode projectionMode)
{
    if (m_projectionMode == projectionMode)
        return;

    m_projectionMode = projectionMode;
    emit projectionModeChanged(m_projectionMode);
    update();
}

void QQuick3DCamera::setEnableFrustumCulling(bool enableFrustumCulling)
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
 * \sa QQuick3DViewport::worldToView QQuick3DCamera::viewportToWorld
 */
QVector3D QQuick3DCamera::worldToViewport(const QVector3D &worldPos) const
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
 * \sa QQuick3DViewport::viewToWorld QQuick3DCamera::worldToViewport
 */
QVector3D QQuick3DCamera::viewportToWorld(const QVector3D &viewportPos) const
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

/*!
 * \qmlproperty bool Camera::enableFrustumCulling
 *
 *
 *
 */

bool QQuick3DCamera::enableFrustumCulling() const
{
    return m_enableFrustumCulling;
}

/*!
 * \internal
 */\
QDemonRenderGraphObject *QQuick3DCamera::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderCamera();

    QQuick3DNode::updateSpatialNode(node);

    QDemonRenderCamera *camera = static_cast<QDemonRenderCamera *>(node);

    camera->clipNear = m_clipNear;
    camera->clipFar = m_clipFar;
    camera->fov = qDegreesToRadians(m_fieldOfView);
    camera->fovHorizontal = m_isFieldOfViewHorizontal;

    camera->scaleMode = QDemonRenderCamera::ScaleModes(m_scaleMode);
    camera->scaleAnchor = QDemonRenderCamera::ScaleAnchors(m_scaleAnchor);
    camera->enableFrustumClipping = m_enableFrustumCulling;

    camera->flags.setFlag(QDemonRenderNode::Flag::Orthographic, m_projectionMode == Orthographic);

    m_cameraNode = camera;

    return node;
}

QT_END_NAMESPACE
