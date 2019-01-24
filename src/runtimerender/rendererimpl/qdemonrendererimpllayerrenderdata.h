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
#ifndef QDEMON_RENDERER_IMPL_LAYER_RENDER_DATA_H
#define QDEMON_RENDERER_IMPL_LAYER_RENDER_DATA_H

#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderpreparationdata.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcetexture2d.h>

QT_BEGIN_NAMESPACE

struct AdvancedBlendModes
{
    enum Enum {
        None = 0,
        Overlay,
        ColorBurn,
        ColorDodge
    };
};
struct SLayerRenderData : public SLayerRenderPreparationData
{

    // Layers can be rendered offscreen for many reasons; effects, progressive aa,
    // or just because a flag forces it.  If they are rendered offscreen we can then
    // cache the result so we don't render the layer again if it isn't dirty.
    CResourceTexture2D m_LayerTexture;
    CResourceTexture2D m_TemporalAATexture;
    // Sometimes we need to render our depth buffer to a depth texture.
    CResourceTexture2D m_LayerDepthTexture;
    CResourceTexture2D m_LayerPrepassDepthTexture;
    CResourceTexture2D m_LayerWidgetTexture;
    CResourceTexture2D m_LayerSsaoTexture;
    // if we render multisampled we need resolve buffers
    CResourceTexture2D m_LayerMultisampleTexture;
    CResourceTexture2D m_LayerMultisamplePrepassDepthTexture;
    CResourceTexture2D m_LayerMultisampleWidgetTexture;
    // the texture contains the render result inclusive post effects
    QSharedPointer<QDemonRenderTexture2D> m_LayerCachedTexture;

    QSharedPointer<QDemonRenderTexture2D> m_AdvancedBlendDrawTexture;
    QSharedPointer<QDemonRenderTexture2D> m_AdvancedBlendBlendTexture;
    QSharedPointer<QDemonRenderFrameBuffer> m_AdvancedModeDrawFB;
    QSharedPointer<QDemonRenderFrameBuffer> m_AdvancedModeBlendFB;

    // True if this layer was rendered offscreen.
    // If this object has no value then this layer wasn't rendered at all.
    SOffscreenRendererEnvironment m_LastOffscreenRenderEnvironment;

    // GPU profiler per layer
    QSharedPointer<IRenderProfiler> m_LayerProfilerGpu;

    SCamera m_SceneCamera;
    QVector2D m_SceneDimensions;

    // ProgressiveAA algorithm details.
    quint32 m_ProgressiveAAPassIndex;
    // Increments every frame regardless to provide appropriate jittering
    quint32 m_TemporalAAPassIndex;
    // Ensures we don't stop on an in-between frame; we will run two frames after the dirty flag
    // is clear.
    quint32 m_NonDirtyTemporalAAPassIndex;
    float m_TextScale;

    QDemonOption<QVector3D> m_BoundingRectColor;
    QDemonRenderTextureFormats::Enum m_DepthBufferFormat;

    QSize m_previousDimensions;

    SLayerRenderData(SLayer &inLayer, QSharedPointer<QDemonRendererImpl> inRenderer);

    virtual ~SLayerRenderData();

    void PrepareForRender();

    // Internal Call
    void PrepareForRender(const QSize &inViewportDimensions) override;

    QDemonRenderTextureFormats::Enum GetDepthBufferFormat();
    QDemonRenderFrameBufferAttachments::Enum
    GetFramebufferDepthAttachmentFormat(QDemonRenderTextureFormats::Enum depthFormat);

    // Render this layer assuming viewport and RT are setup.  Just renders exactly this item
    // no effects.
    void RenderDepthPass(bool inEnableTransparentDepthWrite = false);
    void RenderAoPass();
    void RenderFakeDepthMapPass(QDemonRenderTexture2D *theDepthTex, QDemonRenderTextureCube *theDepthCube);
    void RenderShadowMapPass(CResourceFrameBuffer *theFB);
    void RenderShadowCubeBlurPass(CResourceFrameBuffer *theFB,
                                  QSharedPointer<QDemonRenderTextureCube> target0,
                                  QSharedPointer<QDemonRenderTextureCube> target1,
                                  float filterSz,
                                  float clipFar);
    void RenderShadowMapBlurPass(CResourceFrameBuffer *theFB,
                                 QSharedPointer<QDemonRenderTexture2D> target0,
                                 QSharedPointer<QDemonRenderTexture2D> target1,
                                 float filterSz,
                                 float clipFar);

    void Render(CResourceFrameBuffer *theFB = nullptr);
    void ResetForFrame() override;

    void CreateGpuProfiler();
    void StartProfiling(QString &nameID, bool sync);
    void EndProfiling(QString &nameID);
    void StartProfiling(const char *nameID, bool sync);
    void EndProfiling(const char *nameID);
    void AddVertexCount(quint32 count);

    void RenderToViewport();
    // Render this layer's data to a texture.  Required if we have any effects,
    // prog AA, or if forced.
    void RenderToTexture();

    void ApplyLayerPostEffects();

    void RunnableRenderToViewport(QSharedPointer<QDemonRenderFrameBuffer> theFB);

    void AddLayerRenderStep();

    void RenderRenderWidgets();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    void BlendAdvancedEquationSwFallback(QSharedPointer<QDemonRenderTexture2D> drawTexture,
                                         QSharedPointer<QDemonRenderTexture2D> m_LayerTexture,
                                         AdvancedBlendModes::Enum blendMode);
#endif
    // test method to render this layer to a given view projection without running the entire
    // layer setup system.  This assumes the client has setup the viewport, scissor, and render
    // target
    // the way they want them.
    void PrepareAndRender(const QMatrix4x4 &inViewProjection);

    SOffscreenRendererEnvironment CreateOffscreenRenderEnvironment() override;
    QSharedPointer<IRenderTask> CreateRenderToTextureRunnable() override;

protected:
    // Used for both the normal passes and the depth pass.
    // When doing the depth pass, we disable blending completely because it does not really make
    // sense
    // to write blend equations into
    void RunRenderPass(TRenderRenderableFunction renderFn,
                       bool inEnableBlending,
                       bool inEnableDepthWrite,
                       bool inEnableTransparentDepthWrite,
                       quint32 indexLight,
                       const SCamera &inCamera,
                       CResourceFrameBuffer *theFB = nullptr);
#ifdef ADVANCED_BLEND_SW_FALLBACK
    //Functions for advanced blending mode fallback
    void SetupDrawFB(bool depthEnabled);
    void BlendAdvancedToFB(DefaultMaterialBlendMode::Enum blendMode,
                           bool depthEnabled,
                           CResourceFrameBuffer *theFB);
#endif
};
QT_END_NAMESPACE
#endif
