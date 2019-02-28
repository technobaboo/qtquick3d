#include "qdemonnode.h"

#include <QtDemonRuntimeRender/qdemonrendernode.h>

QT_BEGIN_NAMESPACE

QDemonNode::QDemonNode()
{

}

QDemonNode::~QDemonNode()
{

}

float QDemonNode::x() const
{
    return m_position.x();
}

float QDemonNode::y() const
{
    return m_position.y();
}

float QDemonNode::z() const
{
    return m_position.z();
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

void QDemonNode::setX(float x)
{
    if (qFuzzyCompare(m_position.x(), x))
        return;

    m_position.setX(x);
    emit positionChanged(m_position);
    emit xChanged(x);
    update();
}

void QDemonNode::setY(float y)
{
    if (qFuzzyCompare(m_position.y(), y))
        return;

    m_position.setY(y);
    emit positionChanged(m_position);
    emit yChanged(y);
    update();
}

void QDemonNode::setZ(float z)
{
    if (qFuzzyCompare(m_position.z(), z))
        return;

    m_position.setZ(z);
    emit positionChanged(m_position);
    emit zChanged(z);
    update();
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

QDemonGraphObject *QDemonNode::updateSpatialNode(QDemonGraphObject *node)
{
    if (!node)
        node = new QDemonGraphNode();

    auto spacialNode = static_cast<QDemonGraphNode*>(node);
    bool transformIsDirty = false;
    if (spacialNode->position != m_position) {
        transformIsDirty = true;
        spacialNode->position = m_position;
    }
    if (spacialNode->rotation != m_rotation) {
        transformIsDirty = true;
        spacialNode->rotation = m_rotation;
    }
    if (spacialNode->scale != m_scale) {
        transformIsDirty = true;
        spacialNode->scale = m_scale;
    }
    if (spacialNode->pivot != m_pivot) {
        transformIsDirty = true;
        spacialNode->pivot = m_pivot;
    }

    if (spacialNode->rotationOrder != quint32(m_rotationorder)) {
        transformIsDirty = true;
        spacialNode->rotationOrder = quint32(m_rotationorder);
    }

    spacialNode->localOpacity = m_opacity;
    spacialNode->skeletonId = m_boneid;

    if (transformIsDirty)
        spacialNode->markDirty(NodeTransformDirtyFlag::TransformIsDirty);
    else
        spacialNode->markDirty(NodeTransformDirtyFlag::TransformNotDirty);

    return spacialNode;

}

QT_END_NAMESPACE
