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

struct SLayerRenderData;
struct SRenderSubset;
struct SCustomMaterial;
struct SRenderableImage;

struct SCustomMaterialRenderContext
{
    // The lights and camera will not change per layer,
    // so that information can be set once for all the shaders.
    const SLayer &m_Layer;
    const SLayerRenderData &m_LayerData;
    const QVector<SLight *> &m_Lights;
    const SCamera &m_Camera;

    // Per-object information.
    const SModel &m_Model;
    const SRenderSubset &m_Subset;
    const QMatrix4x4 &m_ModelViewProjection;
    const QMatrix4x4 &m_ModelMatrix; ///< model to world transformation
    const QMatrix3x3 &m_NormalMatrix;
    const SCustomMaterial &m_Material;
    const QSharedPointer<QDemonRenderTexture2D> m_DepthTexture;
    const QSharedPointer<QDemonRenderTexture2D> m_AOTexture;
    SShaderDefaultMaterialKey m_MaterialKey;
    SRenderableImage *m_FirstImage;
    float m_Opacity;

    SCustomMaterialRenderContext(
            const SLayer &layer, const SLayerRenderData &data, const QVector<SLight *> &lights,
            const SCamera &cam, const SModel &m, const SRenderSubset &subset, const QMatrix4x4 &mvp,
            const QMatrix4x4 &world, const QMatrix3x3 &nm, const SCustomMaterial &material,
            const QSharedPointer<QDemonRenderTexture2D> depthTex, const QSharedPointer<QDemonRenderTexture2D> aoTex,
            SShaderDefaultMaterialKey inMaterialKey, SRenderableImage *inFirstImage = nullptr,
            float opacity = 1.0)
        : m_Layer(layer)
        , m_LayerData(data)
        , m_Lights(lights)
        , m_Camera(cam)
        , m_Model(m)
        , m_Subset(subset)
        , m_ModelViewProjection(mvp)
        , m_ModelMatrix(world)
        , m_NormalMatrix(nm)
        , m_Material(material)
        , m_DepthTexture(depthTex)
        , m_AOTexture(aoTex)
        , m_MaterialKey(inMaterialKey)
        , m_FirstImage(inFirstImage)
        , m_Opacity(opacity)
    {
    }
};

QT_END_NAMESPACE

#endif
