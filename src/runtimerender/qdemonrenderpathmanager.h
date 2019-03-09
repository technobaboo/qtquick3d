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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonrendershadowmap.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemon/QDemonDataRef>
#include <QtDemon/QDemonBounds3>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathsubpath.h>
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

struct QDemonLayerGlobalRenderProperties;
class QDemonPathManagerInterface;

struct QDemonPathAnchorPoint
{
    QVector2D position;
    float incomingAngle;
    float outgoingAngle;
    float incomingDistance;
    float outgoingDistance;
};

struct QDemonPathRenderContext; // UICRenderPathRenderContext.h

class Q_DEMONRUNTIMERENDER_EXPORT QDemonPathManagerInterface
{
public:
    QAtomicInt ref;
    // returns the path buffer id
    //!! Note this call is made from multiple threads simultaneously during binary load.
    //!! - see UICRenderGraphObjectSerializer.cpp
    virtual void setPathSubPathData(const QDemonPathSubPath &inPathSubPath,
                                    QDemonConstDataRef<QDemonPathAnchorPoint> inPathSubPathAnchorPoints) = 0;

    virtual ~QDemonPathManagerInterface();
    virtual QDemonDataRef<QDemonPathAnchorPoint> getPathSubPathBuffer(const QDemonPathSubPath &inPathSubPath) = 0;
    // Marks the PathSubPath anchor points as dirty.  This will mean rebuilding any PathSubPath
    // context required to render the PathSubPath.
    virtual QDemonDataRef<QDemonPathAnchorPoint> resizePathSubPathBuffer(const QDemonPathSubPath &inPathSubPath,
                                                                         quint32 inNumAnchors) = 0;
    virtual QDemonBounds3 getBounds(const QDemonPath &inPath) = 0;

    // Helper functions used in various locations
    // Angles here are in degrees because that is how they are represented in the data.
    static QVector2D getControlPointFromAngleDistance(QVector2D inPosition, float inAngle, float inDistance);

    // Returns angle in x, distance in y.
    static QVector2D getAngleDistanceFromControlPoint(QVector2D inPosition, QVector2D inControlPoint);

    virtual QDemonRef<QDemonPathManagerInterface> onRenderSystemInitialize(QDemonRenderContextInterface *context) = 0;

    static QDemonRef<QDemonPathManagerInterface> createPathManager(QDemonRenderContextCoreInterface *inContext);

    // The path segments are next expected to change after this call; changes will be ignored.
    virtual bool prepareForRender(const QDemonPath &inPath) = 0;

    virtual void renderDepthPrepass(QDemonPathRenderContext &inRenderContext,
                                    QDemonLayerGlobalRenderProperties inRenderProperties,
                                    TShaderFeatureSet inFeatureSet) = 0;

    virtual void renderShadowMapPass(QDemonPathRenderContext &inRenderContext,
                                     QDemonLayerGlobalRenderProperties inRenderProperties,
                                     TShaderFeatureSet inFeatureSet) = 0;

    virtual void renderCubeFaceShadowPass(QDemonPathRenderContext &inRenderContext,
                                          QDemonLayerGlobalRenderProperties inRenderProperties,
                                          TShaderFeatureSet inFeatureSet) = 0;

    virtual void renderPath(QDemonPathRenderContext &inRenderContext,
                            QDemonLayerGlobalRenderProperties inRenderProperties,
                            TShaderFeatureSet inFeatureSet) = 0;
};
QT_END_NAMESPACE
#endif
