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

class IOffscreenRenderer;

struct QDemonRenderPickSubResult
{
    QSharedPointer<IOffscreenRenderer> m_SubRenderer;
    QMatrix4x4 m_TextureMatrix;
    QDemonRenderTextureCoordOp::Enum m_HorizontalTilingMode;
    QDemonRenderTextureCoordOp::Enum m_VerticalTilingMode;
    quint32 m_ViewportWidth;
    quint32 m_ViewportHeight;
    QDemonRenderPickSubResult *m_NextSibling;

    QDemonRenderPickSubResult()
        : m_SubRenderer(nullptr)
        , m_NextSibling(nullptr)
    {
    }
    QDemonRenderPickSubResult(QSharedPointer<IOffscreenRenderer> inSubRenderer, QMatrix4x4 inTextureMatrix,
                             QDemonRenderTextureCoordOp::Enum inHorizontalTilingMode,
                             QDemonRenderTextureCoordOp::Enum inVerticalTilingMode, quint32 width,
                             quint32 height)
        : m_SubRenderer(inSubRenderer)
        , m_TextureMatrix(inTextureMatrix)
        , m_HorizontalTilingMode(inHorizontalTilingMode)
        , m_VerticalTilingMode(inVerticalTilingMode)
        , m_ViewportWidth(width)
        , m_ViewportHeight(height)
        , m_NextSibling(nullptr)
    {
    }
};

struct QDemonRenderPickResult
{
    const SGraphObject *m_HitObject;
    float m_CameraDistanceSq;
    // The local coordinates in X,Y UV space where the hit occured
    QVector2D m_LocalUVCoords;
    // The local mouse coordinates will be the same on all of the sub objects.
    QDemonRenderPickSubResult *m_FirstSubObject;
    // The offscreen renderer that was used to render the scene graph this result was produced
    // from.
    QSharedPointer<IOffscreenRenderer> m_OffscreenRenderer;

    QDemonRenderPickResult(const SGraphObject &inHitObject, float inCameraDistance,
                          const QVector2D &inLocalUVCoords)
        : m_HitObject(&inHitObject)
        , m_CameraDistanceSq(inCameraDistance)
        , m_LocalUVCoords(inLocalUVCoords)
        , m_FirstSubObject(nullptr)
        , m_OffscreenRenderer(nullptr)
    {
    }
    QDemonRenderPickResult()
        : m_HitObject(nullptr)
        , m_CameraDistanceSq(std::numeric_limits<float>::max())
        , m_LocalUVCoords(0, 0)
        , m_FirstSubObject(nullptr)
        , m_OffscreenRenderer(nullptr)
    {
    }
};

class IGraphObjectPickQuery
{
protected:
    virtual ~IGraphObjectPickQuery() {}

public:
    // Implementors have the option of batching the results to allow fewer virtual calls
    // or returning one item each pick.
    // Results are guaranteed to be returned nearest to furthest
    // If the return value has size of zero then we assume nothing more can be picked and the
    // pick
    // is finished.
    virtual QDemonRenderPickResult Pick(const QVector2D &inMouseCoords,
                                       const QVector2D &inViewportDimensions,
                                       bool inPickEverything) = 0;
};
QT_END_NAMESPACE
#endif
