#include "qdemoncamera.h"

#include <QtDemonRuntimeRender/qdemonrendercamera.h>

#include <QtMath>
#include <qdemonutils.h>

QT_BEGIN_NAMESPACE

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
 * is normalized between 0 and 1, with the top-left of the viewport being (0,0) and
 * the botton-right (1,1). The returned z value will contain the distance
 * from the camera to \a worldPos in world units. If the position is not
 * visible in the viewport, a position of [-1, -1, -1] is returned.
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
    const QVector4D pos4d = mat44::transform(projectionViewMatrix, worldPosRightHand);

    // Check if the position is visible in the viewport
    if (pos4d.w() <= 0)
        return QVector3D(-1, -1, -1);

    QVector3D pos3d = pos4d.toVector3D();
    // Normalize screenPos between [-1, 1]
    pos3d = pos3d / pos4d.w();
    // Normalize screenPos between [0, 1]
    pos3d = (pos3d / 2) + QVector3D(0.5, 0.5, 0.0);
    // Convert origin from bottom-left to top-left
    pos3d.setY(1 - pos3d.y());
    // Set z to be the distance from clipNear to worldPos
    pos3d.setZ((position() - worldPos).length() - clipNear());

    const bool visibleX = (pos3d.x() - 1) * pos3d.x() <= 0;
    const bool visibleY = (pos3d.y() - 1) * pos3d.y() <= 0;
    return visibleX && visibleY ? pos3d : QVector3D(-1, -1, -1);
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
