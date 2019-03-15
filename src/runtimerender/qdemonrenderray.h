/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
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
#ifndef QDEMON_RENDER_RAY_H
#define QDEMON_RENDER_RAY_H

#include <QtDemon/QDemonOption>
#include <QtDemon/QDemonBounds3>
#include <QtDemon/QDemonPlane>

#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>

QT_BEGIN_NAMESPACE
enum class QDemonRenderBasisPlanes
{
    XY,
    YZ,
    XZ,
};

struct QDemonRenderRay
{
    QVector3D origin;
    QVector3D direction;
    QDemonRenderRay() = default;
    QDemonRenderRay(const QVector3D &origin, const QVector3D &direction)
        : origin(origin), direction(direction)
    {
    }
    // If we are parallel, then no intersection of course.
    QDemonOption<QVector3D> intersect(const QDemonPlane &inPlane) const;

    struct IntersectionResult
    {
        bool intersects = false;
        float rayLengthSquared = 0.; // Length of the ray in world coordinates for the hit.
        QVector2D relXY; // UV coords for further mouse picking against a offscreen-rendered object.
        IntersectionResult() = default;
        IntersectionResult(float rl, QVector2D relxy) : intersects(true), rayLengthSquared(rl), relXY(relxy) {}
    };

    IntersectionResult intersectWithAABB(const QMatrix4x4 &inGlobalTransform, const QDemonBounds3 &inBounds,
                                         bool inForceIntersect = false) const;

    QDemonOption<QVector2D> relative(const QMatrix4x4 &inGlobalTransform,
                                        const QDemonBounds3 &inBounds,
                                        QDemonRenderBasisPlanes inPlane) const;

    QDemonOption<QVector2D> relativeXY(const QMatrix4x4 &inGlobalTransform, const QDemonBounds3 &inBounds) const
    {
        return relative(inGlobalTransform, inBounds, QDemonRenderBasisPlanes::XY);
    }
};
QT_END_NAMESPACE
#endif
