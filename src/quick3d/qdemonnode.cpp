#include "qdemonnode.h"

#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemon/qdemonutils.h>

#include <QtMath>

QT_BEGIN_NAMESPACE

QDemonNode::QDemonNode()
{
    // Update global transform properties from backend
    connect(this, &QDemonNode::transformPropertiesDirty, this, &QDemonNode::updateTransformProperties, Qt::QueuedConnection);
}

QDemonNode::~QDemonNode() {}

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

QVector3D QDemonNode::forward() const
{
    return m_front;
}

QVector3D QDemonNode::up() const
{
    return m_up;
}

QVector3D QDemonNode::right() const
{
    return m_right;
}

QVector3D QDemonNode::globalPosition() const
{
    return m_globalPosition;
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

    const bool xUnchanged = qFuzzyCompare(position.x(), m_position.x());
    const bool yUnchanged = qFuzzyCompare(position.y(), m_position.y());
    const bool zUnchanged = qFuzzyCompare(position.z(), m_position.z());

    m_position = position;
    emit positionChanged(m_position);

    if (!xUnchanged)
        emit xChanged(m_position.x());
    if (!yUnchanged)
        emit yChanged(m_position.y());
    if (!zUnchanged)
        emit zChanged(m_position.z());

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

QDemonRenderGraphObject *QDemonNode::updateSpatialNode(QDemonRenderGraphObject *node)
{
    if (!node)
        node = new QDemonRenderNode();

    auto spacialNode = static_cast<QDemonRenderNode *>(node);
    bool transformIsDirty = false;
    if (spacialNode->position != m_position) {
        transformIsDirty = true;
        spacialNode->position = m_position;
    }
    if (spacialNode->rotation != m_rotation) {
        transformIsDirty = true;
        spacialNode->rotation = QVector3D(qDegreesToRadians(m_rotation.x()),
                                          qDegreesToRadians(m_rotation.y()),
                                          qDegreesToRadians(m_rotation.z()));
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

    if (m_orientation == LeftHanded)
        spacialNode->flags.setFlag(QDemonRenderNode::Flag::LeftHanded, true);
    else
        spacialNode->flags.setFlag(QDemonRenderNode::Flag::LeftHanded, false);

    spacialNode->flags.setFlag(QDemonRenderNode::Flag::Active, m_visible);

    if (transformIsDirty) {
        spacialNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformIsDirty);
        spacialNode->calculateGlobalVariables();
        m_globalTransform = spacialNode->globalTransform;
        // Might need to switch it regardless because it is always in right hand coordinates
        if (m_orientation == LeftHanded)
            spacialNode->flipCoordinateSystem(m_globalTransform);
        emit transformPropertiesDirty();
        // Still needs to be marked dirty if it will show up correctly in the backend
        spacialNode->flags.setFlag(QDemonRenderNode::Flag::Dirty, true);
    } else {
        spacialNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformNotDirty);
    }

    return spacialNode;
}

void QDemonNode::updateTransformProperties()
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    // front
    const QVector3D frontVector(0, 0, 1);
    const QVector3D front = mat33::transform(theDirMatrix, frontVector).normalized();
    if (front != m_front) {
        m_front = front;
        emit forwardChanged(front);
    }

    // up
    const QVector3D upVector(0, 1, 0);
    const QVector3D up = mat33::transform(theDirMatrix, upVector).normalized();
    if (up != m_up) {
        m_up = up;
        emit upChanged(up);
    }

    // right
    const QVector3D rightVector(1, 0, 0);
    const QVector3D right = mat33::transform(theDirMatrix, rightVector).normalized();
    if (right != m_right) {
        m_right = right;
        emit rightChanged(right);
    }

    // globalPosition
    const QVector3D globalPos(m_globalTransform(0, 3), m_globalTransform(1, 3), m_globalTransform(2, 3));
    if (m_globalPosition != globalPos) {
        m_globalPosition= globalPos;
        emit globalPositionChanged(globalPos);
    }
}

QT_END_NAMESPACE
