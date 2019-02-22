#include "qdemoncamera.h"

#include <QtDemonRuntimeRender/qdemonrendercamera.h>

QT_BEGIN_NAMESPACE

QDemonCamera::QDemonCamera()
{

}

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

QDemonCamera::CameraScaleModes QDemonCamera::scaleMode() const
{
    return m_scaleMode;
}

QDemonCamera::CameraScaleAnchors QDemonCamera::scaleAnchor() const
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

void QDemonCamera::setClipNear(float clipNear)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_clipNear, clipNear))
        return;

    m_clipNear = clipNear;
    emit clipNearChanged(m_clipNear);
}

void QDemonCamera::setClipFar(float clipFar)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_clipFar, clipFar))
        return;

    m_clipFar = clipFar;
    emit clipFarChanged(m_clipFar);
}

void QDemonCamera::setFieldOfView(float fieldOfView)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_fieldOfView, fieldOfView))
        return;

    m_fieldOfView = fieldOfView;
    emit fieldOfViewChanged(m_fieldOfView);
}

void QDemonCamera::setIsFieldOFViewHorizontal(bool isFieldOFViewHorizontal)
{
    if (m_isFieldOFViewHorizontal == isFieldOFViewHorizontal)
        return;

    m_isFieldOFViewHorizontal = isFieldOFViewHorizontal;
    emit isFieldOFViewHorizontalChanged(m_isFieldOFViewHorizontal);
}

void QDemonCamera::setScaleMode(QDemonCamera::CameraScaleModes scaleMode)
{
    if (m_scaleMode == scaleMode)
        return;

    m_scaleMode = scaleMode;
    emit scaleModeChanged(m_scaleMode);
}

void QDemonCamera::setScaleAnchor(QDemonCamera::CameraScaleAnchors scaleAnchor)
{
    if (m_scaleAnchor == scaleAnchor)
        return;

    m_scaleAnchor = scaleAnchor;
    emit scaleAnchorChanged(m_scaleAnchor);
}

void QDemonCamera::setFrustumScaleX(float frustumScaleX)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_frustumScaleX, frustumScaleX))
        return;

    m_frustumScaleX = frustumScaleX;
    emit frustumScaleXChanged(m_frustumScaleX);
}

void QDemonCamera::setFrustumScaleY(float frustumScaleY)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_frustumScaleY, frustumScaleY))
        return;

    m_frustumScaleY = frustumScaleY;
    emit frustumScaleYChanged(m_frustumScaleY);
}

QDemonGraphObject *QDemonCamera::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node) {
        node = new QDemonRenderCamera();
    }

    // ### TODO:  update camera properties

    return node;
}

QT_END_NAMESPACE
