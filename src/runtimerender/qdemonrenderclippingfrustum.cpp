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
#include <qdemonrenderclippingfrustum.h>

QT_BEGIN_NAMESPACE

SClippingFrustum::SClippingFrustum(const QMatrix4x4 &modelviewprojection, SClipPlane nearPlane)
{
    SClipPlane *_cullingPlanes = mPlanes;
    const QMatrix4x4 &modelViewProjectionMat(modelviewprojection);
    const float *modelviewProjection = modelViewProjectionMat.front();

    // update planes (http://read.pudn.com/downloads128/doc/542641/Frustum.pdf)
    // Google for Gribb plane extraction if that link doesn't work.
    // http://www.google.com/search?q=ravensoft+plane+extraction
#define M(_x, _y) modelviewProjection[(4 * (_y)) + (_x)]
    // left plane
    _cullingPlanes[0].normal.x = M(3, 0) + M(0, 0);
    _cullingPlanes[0].normal.y = M(3, 1) + M(0, 1);
    _cullingPlanes[0].normal.z = M(3, 2) + M(0, 2);
    _cullingPlanes[0].d = M(3, 3) + M(0, 3);
    _cullingPlanes[0].d /= _cullingPlanes[0].normal.normalize();

    // right plane
    _cullingPlanes[1].normal.x = M(3, 0) - M(0, 0);
    _cullingPlanes[1].normal.y = M(3, 1) - M(0, 1);
    _cullingPlanes[1].normal.z = M(3, 2) - M(0, 2);
    _cullingPlanes[1].d = M(3, 3) - M(0, 3);
    _cullingPlanes[1].d /= _cullingPlanes[1].normal.normalize();

    // far plane
    _cullingPlanes[2].normal.x = M(3, 0) - M(2, 0);
    _cullingPlanes[2].normal.y = M(3, 1) - M(2, 1);
    _cullingPlanes[2].normal.z = M(3, 2) - M(2, 2);
    _cullingPlanes[2].d = M(3, 3) - M(2, 3);
    _cullingPlanes[2].d /= _cullingPlanes[2].normal.normalize();

    // bottom plane
    _cullingPlanes[3].normal.x = M(3, 0) + M(1, 0);
    _cullingPlanes[3].normal.y = M(3, 1) + M(1, 1);
    _cullingPlanes[3].normal.z = M(3, 2) + M(1, 2);
    _cullingPlanes[3].d = M(3, 3) + M(1, 3);
    _cullingPlanes[3].d /= _cullingPlanes[3].normal.normalize();

    // top plane
    _cullingPlanes[4].normal.x = M(3, 0) - M(1, 0);
    _cullingPlanes[4].normal.y = M(3, 1) - M(1, 1);
    _cullingPlanes[4].normal.z = M(3, 2) - M(1, 2);
    _cullingPlanes[4].d = M(3, 3) - M(1, 3);
    _cullingPlanes[4].d /= _cullingPlanes[4].normal.normalize();
#undef M
    _cullingPlanes[5] = nearPlane;
    // http://www.openscenegraph.org/projects/osg/browser/OpenSceneGraph/trunk/include/osg/Plane?rev=5328
    // setup the edges of the plane that we will clip against an axis-aligned bounding box.
    for (quint32 idx = 0; idx < 6; ++idx) {
        _cullingPlanes[idx].calculateBBoxEdges();
    }
}

QT_END_NAMESPACE
