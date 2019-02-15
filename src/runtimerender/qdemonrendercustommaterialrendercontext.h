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
#ifndef QDEMON_RENDER_CUSTOM_MATERIAL_RENDER_CONTEXT_H
#define QDEMON_RENDER_CUSTOM_MATERIAL_RENDER_CONTEXT_H

#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>

QT_BEGIN_NAMESPACE

struct QDemonLayerRenderData;
struct QDemonRenderSubset;
struct QDemonCustomMaterial;
struct QDemonRenderableImage;

struct QDemonCustomMaterialRenderContext
{
    // The lights and camera will not change per layer,
    // so that information can be set once for all the shaders.
    const QDemonLayer &layer;
    const QDemonLayerRenderData &layerData;
    const QVector<QDemonRenderLight *> &lights;
    const QDemonRenderCamera &camera;

    // Per-object information.
    const QDemonRenderModel &model;
    const QDemonRenderSubset &subset;
    const QMatrix4x4 &modelViewProjection;
    const QMatrix4x4 &modelMatrix; ///< model to world transformation
    const QMatrix3x3 &normalMatrix;
    const QDemonCustomMaterial &material;
    const QSharedPointer<QDemonRenderTexture2D> depthTexture;
    const QSharedPointer<QDemonRenderTexture2D> aoTexture;
    QDemonShaderDefaultMaterialKey materialKey;
    QDemonRenderableImage *firstImage;
    float opacity;

    QDemonCustomMaterialRenderContext(const QDemonLayer &inLayer,
                                      const QDemonLayerRenderData &inData,
                                      const QVector<QDemonRenderLight *> &inLights,
                                      const QDemonRenderCamera &inCamera,
                                      const QDemonRenderModel &inModel,
                                      const QDemonRenderSubset &inSubset,
                                      const QMatrix4x4 &inMvp,
                                      const QMatrix4x4 &inWorld,
                                      const QMatrix3x3 &inNormal,
                                      const QDemonCustomMaterial &inMaterial,
                                      const QSharedPointer<QDemonRenderTexture2D> inDepthTex,
                                      const QSharedPointer<QDemonRenderTexture2D> inAoTex,
                                      QDemonShaderDefaultMaterialKey inMaterialKey,
                                      QDemonRenderableImage *inFirstImage = nullptr,
                                      float inOpacity = 1.0)
        : layer(inLayer)
        , layerData(inData)
        , lights(inLights)
        , camera(inCamera)
        , model(inModel)
        , subset(inSubset)
        , modelViewProjection(inMvp)
        , modelMatrix(inWorld)
        , normalMatrix(inNormal)
        , material(inMaterial)
        , depthTexture(inDepthTex)
        , aoTexture(inAoTex)
        , materialKey(inMaterialKey)
        , firstImage(inFirstImage)
        , opacity(inOpacity)
    {
    }
};

QT_END_NAMESPACE

#endif
