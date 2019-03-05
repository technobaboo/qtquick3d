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
#ifndef QDEMON_RENDERER_IMPL_LAYER_RENDER_DATA_H
#define QDEMON_RENDERER_IMPL_LAYER_RENDER_DATA_H

#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderpreparationdata.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcetexture2d.h>

QT_BEGIN_NAMESPACE

struct AdvancedBlendModes
{
    enum Enum { None = 0, Overlay, ColorBurn, ColorDodge };
};
struct QDemonLayerRenderData : public QDemonLayerRenderPreparationData
{
    QAtomicInt ref;

    // Layers can be rendered offscreen for many reasons; effects, progressive aa,
    // or just because a flag forces it.  If they are rendered offscreen we can then
    // cache the result so we don't render the layer again if it isn't dirty.
    QDemonResourceTexture2D m_layerTexture;
    QDemonResourceTexture2D m_temporalAATexture;
    // Sometimes we need to render our depth buffer to a depth texture.
    QDemonResourceTexture2D m_layerDepthTexture;
    QDemonResourceTexture2D m_layerPrepassDepthTexture;
    QDemonResourceTexture2D m_layerWidgetTexture;
    QDemonResourceTexture2D m_layerSsaoTexture;
    // if we render multisampled we need resolve buffers
    QDemonResourceTexture2D m_layerMultisampleTexture;
    QDemonResourceTexture2D m_layerMultisamplePrepassDepthTexture;
    QDemonResourceTexture2D m_layerMultisampleWidgetTexture;
    // the texture contains the render result inclusive post effects
    QDemonRef<QDemonRenderTexture2D> m_layerCachedTexture;

    QDemonRef<QDemonRenderTexture2D> m_advancedBlendDrawTexture;
    QDemonRef<QDemonRenderTexture2D> m_advancedBlendBlendTexture;
    QDemonRef<QDemonRenderFrameBuffer> m_advancedModeDrawFB;
    QDemonRef<QDemonRenderFrameBuffer> m_advancedModeBlendFB;

    // True if this layer was rendered offscreen.
    // If this object has no value then this layer wasn't rendered at all.
    QDemonOffscreenRendererEnvironment m_lastOffscreenRenderEnvironment;

    // GPU profiler per layer
    QDemonRef<QDemonRenderProfilerInterface> m_layerProfilerGpu;

    QDemonRenderCamera m_sceneCamera;
    QVector2D m_sceneDimensions;

    // ProgressiveAA algorithm details.
    quint32 m_progressiveAAPassIndex;
    // Increments every frame regardless to provide appropriate jittering
    quint32 m_temporalAAPassIndex;
    // Ensures we don't stop on an in-between frame; we will run two frames after the dirty flag
    // is clear.
    quint32 m_nonDirtyTemporalAAPassIndex;
    float m_textScale;

    QDemonOption<QVector3D> m_boundingRectColor;
    QDemonRenderTextureFormats::Enum m_depthBufferFormat;

    QSize m_previousDimensions;

    QDemonLayerRenderData(QDemonRenderLayer &inLayer, const QDemonRef<QDemonRendererImpl> &inRenderer);

    virtual ~QDemonLayerRenderData() override;

    void prepareForRender();

    // Internal Call
    void prepareForRender(const QSize &inViewportDimensions) override;

    QDemonRenderTextureFormats::Enum getDepthBufferFormat();
    QDemonRenderFrameBufferAttachments::Enum getFramebufferDepthAttachmentFormat(QDemonRenderTextureFormats::Enum depthFormat);

    // Render this layer assuming viewport and RT are setup.  Just renders exactly this item
    // no effects.
    void renderDepthPass(bool inEnableTransparentDepthWrite = false);
    void renderAoPass();
    void renderFakeDepthMapPass(QDemonRenderTexture2D *theDepthTex, QDemonRenderTextureCube *theDepthCube);
    void renderShadowMapPass(QDemonResourceFrameBuffer *theFB);
    void renderShadowCubeBlurPass(QDemonResourceFrameBuffer *theFB,
                                  const QDemonRef<QDemonRenderTextureCube> &target0,
                                  const QDemonRef<QDemonRenderTextureCube> &target1,
                                  float filterSz,
                                  float clipFar);
    void renderShadowMapBlurPass(QDemonResourceFrameBuffer *theFB,
                                 const QDemonRef<QDemonRenderTexture2D> &target0,
                                 const QDemonRef<QDemonRenderTexture2D> &target1,
                                 float filterSz,
                                 float clipFar);

    void render(QDemonResourceFrameBuffer *theFB = nullptr);
    void resetForFrame() override;

    void createGpuProfiler();
    void startProfiling(QString &nameID, bool sync);
    void endProfiling(QString &nameID);
    void startProfiling(const char *nameID, bool sync);
    void endProfiling(const char *nameID);
    void addVertexCount(quint32 count);

    void renderToViewport();
    // Render this layer's data to a texture.  Required if we have any effects,
    // prog AA, or if forced.
    void renderToTexture();

    void applyLayerPostEffects();

    void runnableRenderToViewport(const QDemonRef<QDemonRenderFrameBuffer> &theFB);

    void addLayerRenderStep();

    void renderRenderWidgets();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    void blendAdvancedEquationSwFallback(const QDemonRef<QDemonRenderTexture2D> &drawTexture,
                                         const QDemonRef<QDemonRenderTexture2D> &m_layerTexture,
                                         AdvancedBlendModes::Enum blendMode);
#endif
    // test method to render this layer to a given view projection without running the entire
    // layer setup system.  This assumes the client has setup the viewport, scissor, and render
    // target
    // the way they want them.
    void prepareAndRender(const QMatrix4x4 &inViewProjection);

    QDemonOffscreenRendererEnvironment createOffscreenRenderEnvironment() override;
    QDemonRef<QDemonRenderTask> createRenderToTextureRunnable() override;

protected:
    // Used for both the normal passes and the depth pass.
    // When doing the depth pass, we disable blending completely because it does not really make
    // sense
    // to write blend equations into
    void runRenderPass(TRenderRenderableFunction renderFn,
                       bool inEnableBlending,
                       bool inEnableDepthWrite,
                       bool inEnableTransparentDepthWrite,
                       quint32 indexLight,
                       const QDemonRenderCamera &inCamera,
                       QDemonResourceFrameBuffer *theFB = nullptr);
#ifdef ADVANCED_BLEND_SW_FALLBACK
    // Functions for advanced blending mode fallback
    void setupDrawFB(bool depthEnabled);
    void blendAdvancedToFB(DefaultMaterialBlendMode::Enum blendMode, bool depthEnabled, QDemonResourceFrameBuffer *theFB);
#endif
};
QT_END_NAMESPACE
#endif
