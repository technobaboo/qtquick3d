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
#ifndef QDEMON_RENDER_CONTEXT_CORE_H
#define QDEMON_RENDER_CONTEXT_CORE_H

#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonperframeallocator.h>

#include <QtDemon/qdemonperftimer.h>

#include <QtCore/QPair>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

enum class ScaleModes
{
    ExactSize = 0, // Ensure the viewport is exactly same size as application
    ScaleToFit = 1, // Resize viewport keeping aspect ratio
    ScaleToFill = 2, // Resize viewport to entire window
    FitSelected = 3, // Resize presentation to fit into viewport
};

class QDemonPathManagerInterface;
class QDemonMaterialSystem;

// Part of render context that does not require the render system.
class Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderContextCore
{
public:
    QAtomicInt ref;
private:
    QDemonPerfTimer m_perfTimer;
    QDemonRef<QDemonInputStreamFactoryInterface> m_inputStreamFactory;
    QDemonRef<QDemonAbstractThreadPool> m_threadPool;
    QDemonRef<QDemonDynamicObjectSystemInterface> m_dynamicObjectSystem;
    QDemonRef<QDemonMaterialSystem> m_materialSystem;
    QDemonRef<QDemonEffectSystemInterface> m_effectSystem;
    QDemonRef<QDemonPathManagerInterface> m_pathManagerCore;

public:
    QDemonRenderContextCore();
    ~QDemonRenderContextCore();
    QDemonRef<QDemonInputStreamFactoryInterface> inputStreamFactory()
    { return m_inputStreamFactory; }
    QDemonRef<QDemonAbstractThreadPool> threadPool()
    { return m_threadPool; }
    QDemonRef<QDemonDynamicObjectSystemInterface> dynamicObjectSystem()
    { return m_dynamicObjectSystem; }
    QDemonRef<QDemonMaterialSystem> materialSystem()
    { return m_materialSystem; }
    QDemonRef<QDemonEffectSystemInterface> effectSystem()
    { return m_effectSystem; }
    QDemonPerfTimer *performanceTimer() { return &m_perfTimer; }
    QDemonRef<QDemonPathManagerInterface> pathManager();

    // The render context maintains a reference to this object.
    QDemonRef<QDemonRenderContextInterface> createRenderContext(QDemonRef<QDemonRenderContext> inContext,
                                                                        const char *inPrimitivesDirectory);
};

class QDemonRendererInterface;
class QDemonShaderCacheInterface;
class QDemonOffscreenRenderManagerInterface;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderContextInterface
{
public:
    QAtomicInt ref;
private:
    QDemonRef<QDemonRenderContext> m_renderContext;
    QDemonRenderContextCore *m_coreContext;
    QDemonPerfTimer *m_perfTimer;
    QDemonRef<QDemonInputStreamFactoryInterface> m_inputStreamFactory;
    QDemonRef<QDemonBufferManager> m_bufferManager;
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonOffscreenRenderManagerInterface> m_offscreenRenderManager;
    QDemonRef<QDemonRendererInterface> m_renderer;
    QDemonRef<QDemonDynamicObjectSystemInterface> m_dynamicObjectSystem;
    QDemonRef<QDemonEffectSystemInterface> m_effectSystem;
    QDemonRef<QDemonShaderCacheInterface> m_shaderCache;
    QDemonRef<QDemonAbstractThreadPool> m_threadPool;
    QDemonRef<IImageBatchLoader> m_imageBatchLoader;
    QDemonRef<QDemonMaterialSystem> m_customMaterialSystem;
    QDemonRef<QDemonPixelGraphicsRendererInterface> m_pixelGraphicsRenderer;
    QDemonRef<QDemonPathManagerInterface> m_pathManager;
    QDemonRef<QDemonShaderProgramGeneratorInterface> m_shaderProgramGenerator;
    QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> m_defaultMaterialShaderGenerator;
    QDemonRef<QDemonMaterialShaderGeneratorInterface> m_customMaterialShaderGenerator;
    QDemonPerFrameAllocator m_perFrameAllocator;
    QDemonRef<QDemonRenderListInterface> m_renderList;
    quint32 m_frameCount;
    // Viewport that this render context should use
    QDemonOption<QRect> m_viewport;
    QSize m_windowDimensions;
    ScaleModes m_scaleMode;
    bool m_wireframeMode;
    bool m_isInSubPresentation;
    QDemonOption<QVector4D> m_sceneColor;
    QDemonOption<QVector4D> m_matteColor;
    RenderRotationValues m_rotation;
    QDemonRef<QDemonRenderFrameBuffer> m_rotationFbo;
    QDemonRef<QDemonRenderTexture2D> m_rotationTexture;
    QDemonRef<QDemonRenderRenderBuffer> m_rotationDepthBuffer;
    QDemonRef<QDemonRenderFrameBuffer> m_contextRenderTarget;
    QRect m_presentationViewport;
    QSize m_presentationDimensions;
    QSize m_renderPresentationDimensions;
    QSize m_preRenderPresentationDimensions;
    QVector2D m_presentationScale;
    QRect m_virtualViewport;
    QPair<float, int> m_fps;
    bool m_authoringMode;

    struct BeginFrameResult
    {
        bool renderOffscreen;
        QSize presentationDimensions;
        bool scissorTestEnabled;
        QRect scissorRect;
        QRect viewport;
        QSize fboDimensions;
        BeginFrameResult(bool ro, QSize presDims, bool scissorEnabled, QRect inScissorRect, QRect inViewport, QSize fboDims)
            : renderOffscreen(ro)
            , presentationDimensions(presDims)
            , scissorTestEnabled(scissorEnabled)
            , scissorRect(inScissorRect)
            , viewport(inViewport)
            , fboDimensions(fboDims)
        {
        }
        BeginFrameResult() = default;
    };

    // Calculated values passed from beginframe to setupRenderTarget.
    // Trying to avoid duplicate code as much as possible.
    BeginFrameResult m_beginFrameResult;

    QPair<QRect, QRect> getPresentationViewportAndOuterViewport() const;
    QRect getPresentationViewport(const QRect &inViewerViewport, ScaleModes inScaleToFit, const QSize &inPresDimensions) const;
    void setupRenderTarget();
    void teardownRenderTarget();

public:
    QDemonRenderContextInterface(const QDemonRef<QDemonRenderContext> &ctx, QDemonRenderContextCore *inCore, const char *inApplicationDirectory);
    ~QDemonRenderContextInterface();
    QDemonRef<QDemonRendererInterface> getRenderer();
    QDemonRef<QDemonRendererImpl> getRenderWidgetContext();
    QDemonRef<QDemonBufferManager> getBufferManager();
    QDemonRef<QDemonResourceManagerInterface> getResourceManager();
    QDemonRef<QDemonRenderContext> getRenderContext();
    QDemonRef<QDemonOffscreenRenderManagerInterface> getOffscreenRenderManager();
    QDemonRef<QDemonInputStreamFactoryInterface> getInputStreamFactory();
    QDemonRef<QDemonEffectSystemInterface> getEffectSystem();
    QDemonRef<QDemonShaderCacheInterface> getShaderCache();
    QDemonRef<QDemonAbstractThreadPool> getThreadPool();
    QDemonRef<IImageBatchLoader> getImageBatchLoader();
    QDemonRef<QDemonDynamicObjectSystemInterface> dynamicObjectSystem();
    QDemonRef<QDemonMaterialSystem> getCustomMaterialSystem();
    QDemonRef<QDemonPixelGraphicsRendererInterface> getPixelGraphicsRenderer();
    QDemonPerfTimer *performanceTimer();
    QDemonRef<QDemonRenderListInterface> getRenderList();
    QDemonRef<QDemonPathManagerInterface> getPathManager();
    QDemonRef<QDemonShaderProgramGeneratorInterface> getShaderProgramGenerator();
    QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> getDefaultMaterialShaderGenerator();
    QDemonRef<QDemonMaterialShaderGeneratorInterface> getCustomMaterialShaderGenerator();
    // The memory used for the per frame allocator is released as the first step in BeginFrame.
    // This is useful for short lived objects and datastructures.
    QDemonPerFrameAllocator &getPerFrameAllocator();

    // Get the number of times EndFrame has been called
    quint32 getFrameCount();

    // Get fps
    QPair<float, int> getFPS();
    // Set fps by higher level, etc application
    void setFPS(QPair<float, int> inFPS);

    // Currently there are a few things that need to work differently
    // in authoring mode vs. runtime.  The particle effects, for instance
    // need to be framerate-independent at runtime but framerate-dependent during
    // authoring time assuming virtual 16 ms frames.
    // Defaults to falst.
    bool isAuthoringMode();
    void setAuthoringMode(bool inMode);

    // Sub presentations change the rendering somewhat.
    bool isInSubPresentation();
    void setInSubPresentation(bool inValue);
    void setSceneColor(QDemonOption<QVector4D> inSceneColor);
    void setMatteColor(QDemonOption<QVector4D> inMatteColor);

    // render Gpu profiler values
    void dumpGpuProfilerStats();

    // The reason you can set both window dimensions and an overall viewport is that the mouse
    // needs to be inverted
    // which requires the window height, and then the rest of the system really requires the
    // viewport.
    void setWindowDimensions(const QSize &inWindowDimensions);
    QSize getWindowDimensions();

    // In addition to the window dimensions which really have to be set, you can optionally
    // set the viewport which will force the entire viewer to render specifically to this
    // viewport.
    void setViewport(QDemonOption<QRect> inViewport);
    QDemonOption<QRect> getViewport() const;
    QRect getContextViewport() const;
    // Only valid between calls to Begin,End.
    QRect getPresentationViewport() const;

    void setScaleMode(ScaleModes inMode);
    ScaleModes getScaleMode();

    void setWireframeMode(bool inEnable);
    bool getWireframeMode();

    // Return the viewport the system is using to render data to.  This gives the the dimensions
    // of the rendered system.  It is dependent on but not equal to the viewport.
    QRectF getDisplayViewport() const;

    // Layers require the current presentation dimensions in order to render.
    void setPresentationDimensions(const QSize &inPresentationDimensions);
    QSize getCurrentPresentationDimensions() const;

    void setRenderRotation(RenderRotationValues inRotation);
    RenderRotationValues getRenderRotation() const;

    QVector2D getMousePickViewport() const;
    QVector2D getMousePickMouseCoords(const QVector2D &inMouseCoords) const;

    // Valid during and just after prepare for render.
    QVector2D getPresentationScaleFactor() const;

    // Steps needed to render:
    // 1.  BeginFrame - sets up new target in render graph
    // 2.  Add everything you need to the render graph
    // 3.  RunRenderGraph - runs the graph, rendering things to main render target
    // 4.  Render any additional stuff to main render target on top of previously rendered
    // information
    // 5.  EndFrame

    // Clients need to call this every frame in order for various subsystems to release
    // temporary per-frame allocated objects.
    // Also sets up the viewport according to SetViewportInfo
    // and the topmost presentation dimensions.  Expects there to be exactly one presentation
    // dimension pushed at this point.
    // This also starts a render target in the render graph.
    void beginFrame();

    // This runs through the added tasks in reverse order.  This is used to render dependencies
    // before rendering to the main render target.
    void runRenderTasks();
    // Now you can render to the main render target if you want to render over the top
    // of everything.
    // Next call end frame.
    void endFrame();
};
QT_END_NAMESPACE

#endif
