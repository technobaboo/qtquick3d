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

#include "qdemonnode.h"

#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemon/qdemonutils.h>

#include <QtMath>

QT_BEGIN_NAMESPACE

/*!
    \qmltype DemonNode
    \inqmlmodule QtDemon
    \brief Lets you define a 3D item with necessary properties
*/
QDemonNode::QDemonNode()
{
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
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D frontVector(0, 0, 1);
    return mat33::transform(theDirMatrix, frontVector).normalized();
}

QVector3D QDemonNode::up() const
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D upVector(0, 1, 0);
    return mat33::transform(theDirMatrix, upVector).normalized();
}

QVector3D QDemonNode::right() const
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D rightVector(1, 0, 0);
    return mat33::transform(theDirMatrix, rightVector).normalized();
}

QVector3D QDemonNode::globalPosition() const
{
    return QVector3D(m_globalTransform(0, 3), m_globalTransform(1, 3), m_globalTransform(2, 3));
}

QMatrix4x4 QDemonNode::globalTransform() const
{
    return m_globalTransform;
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
        QMatrix4x4 globalTransformMatrix = spacialNode->globalTransform;
        // Might need to switch it regardless because it is always in right hand coordinates
        if (m_orientation == LeftHanded)
            spacialNode->flipCoordinateSystem(globalTransformMatrix);
        if (globalTransformMatrix != m_globalTransform) {
            m_globalTransform = globalTransformMatrix;
            emit globalTransformChanged(m_globalTransform);
        }
        // Still needs to be marked dirty if it will show up correctly in the backend
        spacialNode->flags.setFlag(QDemonRenderNode::Flag::Dirty, true);
    } else {
        spacialNode->markDirty(QDemonRenderNode::TransformDirtyFlag::TransformNotDirty);
    }

    return spacialNode;
}

QT_END_NAMESPACE
