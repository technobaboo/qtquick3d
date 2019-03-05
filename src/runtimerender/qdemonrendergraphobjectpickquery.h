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
#ifndef QDEMON_RENDER_GRAPH_OBJECT_PICK_QUERY_H
#define QDEMON_RENDER_GRAPH_OBJECT_PICK_QUERY_H

#include <QtGui/QVector2D>
#include <QtGui/QMatrix4x4>

#include <QtDemon/qdemondataref.h>

#include <QtDemonRender/qdemonrenderbasetypes.h>

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>

#include <limits>

QT_BEGIN_NAMESPACE

class QDemonOffscreenRendererInterface;

struct QDemonRenderPickSubResult
{
    QDemonRef<QDemonOffscreenRendererInterface> m_subRenderer;
    QMatrix4x4 m_textureMatrix;
    QDemonRenderTextureCoordOp::Enum m_horizontalTilingMode;
    QDemonRenderTextureCoordOp::Enum m_verticalTilingMode;
    quint32 m_viewportWidth;
    quint32 m_viewportHeight;
    QDemonRenderPickSubResult *m_nextSibling;

    QDemonRenderPickSubResult();
    QDemonRenderPickSubResult(const QDemonRef<QDemonOffscreenRendererInterface> &inSubRenderer,
                              const QMatrix4x4 &inTextureMatrix,
                              QDemonRenderTextureCoordOp::Enum inHorizontalTilingMode,
                              QDemonRenderTextureCoordOp::Enum inVerticalTilingMode,
                              quint32 width,
                              quint32 height);
    ~QDemonRenderPickSubResult();
};

struct QDemonRenderPickResult
{
    const QDemonGraphObject *m_hitObject;
    float m_cameraDistanceSq;
    // The local coordinates in X,Y UV space where the hit occured
    QVector2D m_localUVCoords;
    // The local mouse coordinates will be the same on all of the sub objects.
    QDemonRenderPickSubResult *m_firstSubObject;
    // The offscreen renderer that was used to render the scene graph this result was produced
    // from.
    QDemonRef<QDemonOffscreenRendererInterface> m_offscreenRenderer;

    QDemonRenderPickResult(const QDemonGraphObject &inHitObject, float inCameraDistance, const QVector2D &inLocalUVCoords);
    QDemonRenderPickResult();
    ~QDemonRenderPickResult();
};

class QDemonGraphObjectPickQueryInterface
{
protected:
    virtual ~QDemonGraphObjectPickQueryInterface() {}

public:
    // Implementors have the option of batching the results to allow fewer virtual calls
    // or returning one item each pick.
    // Results are guaranteed to be returned nearest to furthest
    // If the return value has size of zero then we assume nothing more can be picked and the
    // pick
    // is finished.
    virtual QDemonRenderPickResult pick(const QVector2D &inMouseCoords, const QVector2D &inViewportDimensions, bool inPickEverything) = 0;
};
QT_END_NAMESPACE
#endif
