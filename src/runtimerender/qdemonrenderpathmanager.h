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
#ifndef QDEMON_RENDER_PATH_MANAGER_H
#define QDEMON_RENDER_PATH_MANAGER_H

#include <QtDemonRender/QDemonRenderContext>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemon/QDemonDataRef>
#include <QtDemon/QDemonBounds3>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathsubpath.h>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

struct SLayerGlobalRenderProperties;
class IPathManager;

struct SPathAnchorPoint
{
    QVector2D m_Position;
    float m_IncomingAngle;
    float m_OutgoingAngle;
    float m_IncomingDistance;
    float m_OutgoingDistance;
    SPathAnchorPoint() {}
    SPathAnchorPoint(QVector2D inPos, float inAngle, float outAngle, float inDis, float outDis)
        : m_Position(inPos)
        , m_IncomingAngle(inAngle)
        , m_OutgoingAngle(outAngle)
        , m_IncomingDistance(inDis)
        , m_OutgoingDistance(outDis)
    {
    }
};

class Q_DEMONRUNTIMERENDER_EXPORT IPathManagerCore
{
public:
    // returns the path buffer id
    //!! Note this call is made from multiple threads simultaneously during binary load.
    //!! - see UICRenderGraphObjectSerializer.cpp
    virtual void
    SetPathSubPathData(const SPathSubPath &inPathSubPath,
                       QDemonConstDataRef<SPathAnchorPoint> inPathSubPathAnchorPoints) = 0;

    virtual ~IPathManagerCore();
    virtual QDemonDataRef<SPathAnchorPoint>
    GetPathSubPathBuffer(const SPathSubPath &inPathSubPath) = 0;
    // Marks the PathSubPath anchor points as dirty.  This will mean rebuilding any PathSubPath
    // context required to render the PathSubPath.
    virtual QDemonDataRef<SPathAnchorPoint>
    ResizePathSubPathBuffer(const SPathSubPath &inPathSubPath, quint32 inNumAnchors) = 0;
    virtual QDemonBounds3 GetBounds(const SPath &inPath) = 0;

    // Helper functions used in various locations
    // Angles here are in degrees because that is how they are represented in the data.
    static QVector2D GetControlPointFromAngleDistance(QVector2D inPosition, float inAngle,
                                                      float inDistance);

    // Returns angle in x, distance in y.
    static QVector2D GetAngleDistanceFromControlPoint(QVector2D inPosition, QVector2D inControlPoint);

    virtual QSharedPointer<IPathManager> OnRenderSystemInitialize(IQDemonRenderContext *context) = 0;

    static QSharedPointer<IPathManagerCore> CreatePathManagerCore(IQDemonRenderContextCore * inContext);
};

struct SPathRenderContext; // UICRenderPathRenderContext.h

class Q_DEMONRUNTIMERENDER_EXPORT IPathManager : public IPathManagerCore
{
public:
    // The path segments are next expected to change after this call; changes will be ignored.
    virtual bool PrepareForRender(const SPath &inPath) = 0;

    virtual void RenderDepthPrepass(SPathRenderContext &inRenderContext,
                                    SLayerGlobalRenderProperties inRenderProperties,
                                    TShaderFeatureSet inFeatureSet) = 0;

    virtual void RenderShadowMapPass(SPathRenderContext &inRenderContext,
                                     SLayerGlobalRenderProperties inRenderProperties,
                                     TShaderFeatureSet inFeatureSet) = 0;

    virtual void RenderCubeFaceShadowPass(SPathRenderContext &inRenderContext,
                                          SLayerGlobalRenderProperties inRenderProperties,
                                          TShaderFeatureSet inFeatureSet) = 0;

    virtual void RenderPath(SPathRenderContext &inRenderContext,
                            SLayerGlobalRenderProperties inRenderProperties,
                            TShaderFeatureSet inFeatureSet) = 0;
};
QT_END_NAMESPACE
#endif
