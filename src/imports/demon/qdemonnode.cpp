#include "qdemonnode.h"

#include <QtDemonRuntimeRender/qdemonrendernode.h>

QDemonNode::QDemonNode()
{

}

QDemonNode::~QDemonNode()
{

}

QVector3D QDemonNode::rotation() const
{
    return m_rotation;
}

QVector3D QDemonNode::position() const
{
    return m_position;
}

QVector3D QDemonNode::scale() const
{
    return m_scale;
}

QVector3D QDemonNode::pivot() const
{
    return m_pivot;
}

float QDemonNode::localOpacity() const
{
    return m_opacity;
}

qint32 QDemonNode::skeletonId() const
{
    return m_boneid;
}

QDemonNode::RotationOrder QDemonNode::rotationOrder() const
{
    return m_rotationorder;
}

QDemonNode::Orientation QDemonNode::orientation() const
{
    return m_orientation;
}

bool QDemonNode::visible() const
{
    return m_visible;
}

QDemonObject::Type QDemonNode::type() const
{
    return QDemonObject::Node;
}

void QDemonNode::setRotation(QVector3D rotation)
{
    if (m_rotation == rotation)
        return;

    m_rotation = rotation;
    emit rotationChanged(m_rotation);
    update();
}

void QDemonNode::setPosition(QVector3D position)
{
    if (m_position == position)
        return;

    m_position = position;
    emit positionChanged(m_position);
    update();
}

void QDemonNode::setScale(QVector3D scale)
{
    if (m_scale == scale)
        return;

    m_scale = scale;
    emit scaleChanged(m_scale);
    update();
}

void QDemonNode::setPivot(QVector3D pivot)
{
    if (m_pivot == pivot)
        return;

    m_pivot = pivot;
    emit pivotChanged(m_pivot);
    update();
}

void QDemonNode::setLocalOpacity(float opacity)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit localOpacityChanged(m_opacity);
    update();
}

void QDemonNode::setSkeletonId(qint32 boneid)
{
    if (m_boneid == boneid)
        return;

    m_boneid = boneid;
    emit skeletonIdChanged(m_boneid);
    update();
}

void QDemonNode::setRotationOrder(QDemonNode::RotationOrder rotationorder)
{
    if (m_rotationorder == rotationorder)
        return;

    m_rotationorder = rotationorder;
    emit rotationOrderChanged(m_rotationorder);
    update();
}

void QDemonNode::setOrientation(QDemonNode::Orientation orientation)
{
    if (m_orientation == orientation)
        return;

    m_orientation = orientation;
    emit orientationChanged(m_orientation);
    update();
}

void QDemonNode::setVisible(bool visible)
{
    if (m_visible == visible)
        return;

    m_visible = visible;
    emit visibleChanged(m_visible);
    update();
}

SGraphObject *QDemonNode::updateSpatialNode(SGraphObject *node)
{
    if (!node) {
        node = new SNode();
    }

    auto spacialNode = static_cast<SNode*>(node);
    spacialNode->m_Position = m_position;
    spacialNode->m_Rotation = m_rotation;
    spacialNode->m_Scale = m_scale;
    spacialNode->m_Pivot = m_pivot;
    spacialNode->m_LocalOpacity = m_opacity;
    spacialNode->m_RotationOrder = quint32(m_rotationorder);
    spacialNode->m_SkeletonId = m_boneid;

    return spacialNode;

}
