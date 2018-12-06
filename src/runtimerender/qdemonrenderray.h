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
#pragma once
#ifndef QDEMON_RENDER_RAY_H
#define QDEMON_RENDER_RAY_H

#include <QtDemonRuntimeRender/qdemonrender.h>
#include <QVector2D.h>
#include <QVector3D.h>
#include <Qt3DSOption.h>
#include <QMatrix4x4.h>
#include <Qt3DSBounds3.h>

QT_BEGIN_NAMESPACE
struct SBasisPlanes
{
    enum Enum {
        XY = 0,
        YZ,
        XZ,
    };
};

struct SRayIntersectionResult
{
    float m_RayLengthSquared; // Length of the ray in world coordinates for the hit.
    QVector2D m_RelXY; // UV coords for further mouse picking against a offscreen-rendered object.
    SRayIntersectionResult()
        : m_RayLengthSquared(0)
        , m_RelXY(0, 0)
    {
    }
    SRayIntersectionResult(float rl, QVector2D relxy)
        : m_RayLengthSquared(rl)
        , m_RelXY(relxy)
    {
    }
};

struct SRay
{
    QVector3D m_Origin;
    QVector3D m_Direction;
    SRay()
        : m_Origin(0, 0, 0)
        , m_Direction(0, 0, 0)
    {
    }
    SRay(const QVector3D &inOrigin, const QVector3D &inDirection)
        : m_Origin(inOrigin)
        , m_Direction(inDirection)
    {
    }
    // If we are parallel, then no intersection of course.
    Option<QVector3D> Intersect(const NVPlane &inPlane) const;

    Option<SRayIntersectionResult> IntersectWithAABB(const QMatrix4x4 &inGlobalTransform,
                                                     const NVBounds3 &inBounds,
                                                     bool inForceIntersect = false) const;

    Option<QVector2D> GetRelative(const QMatrix4x4 &inGlobalTransform, const NVBounds3 &inBounds,
                                  SBasisPlanes::Enum inPlane) const;

    Option<QVector2D> GetRelativeXY(const QMatrix4x4 &inGlobalTransform,
                                    const NVBounds3 &inBounds) const
    {
        return GetRelative(inGlobalTransform, inBounds, SBasisPlanes::XY);
    }
};
QT_END_NAMESPACE
#endif
