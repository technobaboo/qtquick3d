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

    m_cameraNode = camera;

    return node;
}

QT_END_NAMESPACE
