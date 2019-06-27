#include "qdemoncamera.h"

#include <QtDemonRuntimeRender/qdemonrendercamera.h>

#include <QtMath>

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

float QDemonCamera::frustumScaleX() const
{
    return m_frustumScaleX;
}

float QDemonCamera::frustumScaleY() const
{
    return m_frustumScaleY;
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

void QDemonCamera::setFrustumScaleX(float frustumScaleX)
{
    if (qFuzzyCompare(m_frustumScaleX, frustumScaleX))
        return;

    m_frustumScaleX = frustumScaleX;
    emit frustumScaleXChanged(m_frustumScaleX);
    update();
}

void QDemonCamera::setFrustumScaleY(float frustumScaleY)
{
    if (qFuzzyCompare(m_frustumScaleY, frustumScaleY))
        return;

    m_frustumScaleY = frustumScaleY;
    emit frustumScaleYChanged(m_frustumScaleY);
    update();
}

void QDemonCamera::setProjectionMode(QDemonCamera::QDemonCameraProjectionMode projectionMode)
{
    if (m_projectionMode == projectionMode)
        return;

    m_projectionMode = projectionMode;
    emit projectionModeChanged(m_projectionMode);
}

/*!
 * Transforms \a worldPos from world space into viewport space. The position
 * is normalized between 0 and 1. The top-left of the viewport is (0,0) and
 * the botton-right is (1,1). If the position is not visible in the viewport, a
 * position of [-1, -1] is returned.
 *
 * \sa QDemonView3D::worldToView
 */
QVector2D QDemonCamera::worldToViewport(const QVector3D &worldPos) const
{
    if (!m_cameraNode)
        return QVector2D(-1, -1);

    // Convert from left-handed to right-handed
    QVector3D worldPosRightHand = worldPos * QVector3D(0, 0, -1);

    // Transform position
    const QMatrix4x4 worldToCamera = m_cameraNode->globalTransform.inverted();
    QMatrix4x4 projectionViewMatrix = m_cameraNode->projection * worldToCamera;
    QVector4D pos4d = mat44::transform(projectionViewMatrix, QVector4D(worldPosRightHand, 1));

    // Check if the position is visible in the viewport
    if (pos4d.w() <= 0)
        return QVector2D(-1, -1);

    QVector2D pos2d = pos4d.toVector2D();

    // Normalize screenPos between [-1, 1]
    pos2d = pos2d / pos4d.w();
    // Normalize screenPos between [0, 1]
    pos2d = (pos2d / 2) + QVector2D(0.5, 0.5);
    // Convert origin from bottom-left to top-left
    pos2d.setY(1 - pos2d.y());

    const bool visibleX = (pos2d.x() - 1) * pos2d.x() <= 0;
    const bool visibleY = (pos2d.y() - 1) * pos2d.y() <= 0;
    return visibleX && visibleY ? pos2d : QVector2D(-1, -1);
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
    camera->frustumScale = QVector2D(m_frustumScaleX, m_frustumScaleY);

    if (m_projectionMode == Orthographic)
        camera->flags.setFlag(QDemonRenderNode::Flag::Orthographic);

    m_cameraNode = camera;

    return node;
}

QT_END_NAMESPACE
