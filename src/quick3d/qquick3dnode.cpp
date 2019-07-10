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

#include "qquick3dnode.h"

#include <QtDemonRuntimeRender/qdemonrendernode.h>
#include <QtDemon/qdemonutils.h>

#include <QtMath>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Node
    \inqmlmodule QtQuick3D
    \brief Lets you define a 3D item with necessary properties
*/
QQuick3DNode::QQuick3DNode()
{
}

QQuick3DNode::~QQuick3DNode() {}

float QQuick3DNode::x() const
{
    return m_position.x();
}

float QQuick3DNode::y() const
{
    return m_position.y();
}

float QQuick3DNode::z() const
{
    return m_position.z();
}

QVector3D QQuick3DNode::rotation() const
{
    return m_rotation;
}

QVector3D QQuick3DNode::position() const
{
    return m_position;
}

QVector3D QQuick3DNode::scale() const
{
    return m_scale;
}

QVector3D QQuick3DNode::pivot() const
{
    return m_pivot;
}

float QQuick3DNode::localOpacity() const
{
    return m_opacity;
}

qint32 QQuick3DNode::skeletonId() const
{
    return m_boneid;
}

QQuick3DNode::RotationOrder QQuick3DNode::rotationOrder() const
{
    return m_rotationorder;
}

QQuick3DNode::Orientation QQuick3DNode::orientation() const
{
    return m_orientation;
}

bool QQuick3DNode::visible() const
{
    return m_visible;
}

QVector3D QQuick3DNode::forward() const
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D frontVector(0, 0, 1);
    return mat33::transform(theDirMatrix, frontVector).normalized();
}

QVector3D QQuick3DNode::up() const
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D upVector(0, 1, 0);
    return mat33::transform(theDirMatrix, upVector).normalized();
}

QVector3D QQuick3DNode::right() const
{
    QMatrix3x3 theDirMatrix = mat44::getUpper3x3(m_globalTransform);
    theDirMatrix = mat33::getInverse(theDirMatrix).transposed();

    const QVector3D rightVector(1, 0, 0);
    return mat33::transform(theDirMatrix, rightVector).normalized();
}

QVector3D QQuick3DNode::globalPosition() const
{
    return QVector3D(m_globalTransform(0, 3), m_globalTransform(1, 3), m_globalTransform(2, 3));
}

QMatrix4x4 QQuick3DNode::globalTransform() const
{
    return m_globalTransform;
}

QQuick3DObject::Type QQuick3DNode::type() const
{
    return QQuick3DObject::Node;
}

void QQuick3DNode::setX(float x)
{
    if (qFuzzyCompare(m_position.x(), x))
        return;

    m_position.setX(x);
    emit positionChanged(m_position);
    emit xChanged(x);
    update();
}

void QQuick3DNode::setY(float y)
{
    if (qFuzzyCompare(m_position.y(), y))
        return;

    m_position.setY(y);
    emit positionChanged(m_position);
    emit yChanged(y);
    update();
}

void QQuick3DNode::setZ(float z)
{
    if (qFuzzyCompare(m_position.z(), z))
        return;

    m_position.setZ(z);
    emit positionChanged(m_position);
    emit zChanged(z);
    update();
}

void QQuick3DNode::setRotation(QVector3D rotation)
{
    if (m_rotation == rotation)
        return;

    m_rotation = rotation;
    emit rotationChanged(m_rotation);
    update();
}

void QQuick3DNode::setPosition(QVector3D position)
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

void QQuick3DNode::setScale(QVector3D scale)
{
    if (m_scale == scale)
        return;

    m_scale = scale;
    emit scaleChanged(m_scale);
    update();
}

void QQuick3DNode::setPivot(QVector3D pivot)
{
    if (m_pivot == pivot)
        return;

    m_pivot = pivot;
    emit pivotChanged(m_pivot);
    update();
}

void QQuick3DNode::setLocalOpacity(float opacity)
{
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit localOpacityChanged(m_opacity);
    update();
}

void QQuick3DNode::setSkeletonId(qint32 boneid)
{
    if (m_boneid == boneid)
        return;

    m_boneid = boneid;
    emit skeletonIdChanged(m_boneid);
    update();
}

void QQuick3DNode::setRotationOrder(QQuick3DNode::RotationOrder rotationorder)
{
    if (m_rotationorder == rotationorder)
        return;

    m_rotationorder = rotationorder;
    emit rotationOrderChanged(m_rotationorder);
    update();
}

void QQuick3DNode::setOrientation(QQuick3DNode::Orientation orientation)
{
    if (m_orientation == orientation)
        return;

    m_orientation = orientation;
    emit orientationChanged(m_orientation);
    update();
}

void QQuick3DNode::setVisible(bool visible)
{
    if (m_visible == visible)
        return;

    m_visible = visible;
    emit visibleChanged(m_visible);
    update();
}

QDemonRenderGraphObject *QQuick3DNode::updateSpatialNode(QDemonRenderGraphObject *node)
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
