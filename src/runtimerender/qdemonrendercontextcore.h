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
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qtdemonruntimerenderglobal.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>

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
class Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderContextCoreInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonRenderContextCoreInterface();
    virtual QDemonRef<QDemonInputStreamFactoryInterface> getInputStreamFactory() = 0;
    virtual QDemonRef<QDemonAbstractThreadPool> getThreadPool() = 0;
    virtual QDemonRef<QDemonDynamicObjectSystemInterface> getDynamicObjectSystemCore() = 0;
    virtual QDemonMaterialSystem getMaterialSystemCore() = 0;
    virtual QDemonRef<QDemonEffectSystemInterface> getEffectSystemCore() = 0;
    virtual QDemonPerfTimer getPerfTimer() = 0;
    virtual QDemonRef<QDemonPathManagerInterface> getPathManagerCore() = 0;
    // Text renderers may be provided by clients at runtime.
    virtual void setTextRendererCore(QDemonRef<QDemonTextRendererInterface> inRenderer) = 0;
    virtual QDemonRef<QDemonTextRendererInterface> getTextRendererCore() = 0;
    // this is our default 2D text onscreen renderer
    virtual void setOnscreenTextRendererCore(QDemonRef<QDemonTextRendererInterface> inRenderer) = 0;
    virtual QDemonRef<QDemonTextRendererInterface> getOnscreenTextRendererCore() = 0;
    // The render context maintains a reference to this object.
    virtual QDemonRef<QDemonRenderContextInterface> createRenderContext(QDemonRef<QDemonRenderContext> inContext,
                                                                        const char *inPrimitivesDirectory) = 0;

    static QDemonRef<QDemonRenderContextCoreInterface> create();
};

class QDemonTextTextureAtlasInterface;
class QDemonRendererInterface;
class QDemonShaderCacheInterface;
class QDemonOffscreenRenderManagerInterface;
struct QDemonPerFrameAllocator;

class Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderContextInterface
{
public:
    QAtomicInt ref;
    virtual ~QDemonRenderContextInterface();
    virtual QDemonRef<QDemonRendererInterface> getRenderer() = 0;
    virtual QDemonRef<QDemonRendererImpl> getRenderWidgetContext() = 0;
    virtual QDemonBufferManager getBufferManager() = 0;
    virtual QDemonRef<QDemonResourceManagerInterface> getResourceManager() = 0;
    virtual QDemonRef<QDemonRenderContext> getRenderContext() = 0;
    virtual QDemonRef<QDemonOffscreenRenderManagerInterface> getOffscreenRenderManager() = 0;
    virtual QDemonRef<QDemonInputStreamFactoryInterface> getInputStreamFactory() = 0;
    virtual QDemonRef<QDemonEffectSystemInterface> getEffectSystem() = 0;
    virtual QDemonRef<QDemonShaderCacheInterface> getShaderCache() = 0;
    virtual QDemonRef<QDemonAbstractThreadPool> getThreadPool() = 0;
    virtual QDemonRef<IImageBatchLoader> getImageBatchLoader() = 0;
    virtual QDemonRef<QDemonDynamicObjectSystemInterface> getDynamicObjectSystem() = 0;
    virtual QDemonMaterialSystem getCustomMaterialSystem() = 0;
    virtual QDemonRef<QDemonPixelGraphicsRendererInterface> getPixelGraphicsRenderer() = 0;
    virtual QDemonPerfTimer getPerfTimer() = 0;
    virtual QDemonRef<QDemonTextTextureCacheInterface> getTextureCache() = 0;
    virtual QDemonRef<QDemonTextRendererInterface> getTextRenderer() = 0;
    virtual QDemonRef<QDemonRenderListInterface> getRenderList() = 0;
    virtual QDemonRef<QDemonPathManagerInterface> getPathManager() = 0;
    virtual QDemonRef<QDemonShaderProgramGeneratorInterface> getShaderProgramGenerator() = 0;
    virtual QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> getDefaultMaterialShaderGenerator() = 0;
    virtual QDemonRef<ICustomMaterialShaderGenerator> getCustomMaterialShaderGenerator() = 0;
    // The memory used for the per frame allocator is released as the first step in BeginFrame.
    // This is useful for short lived objects and datastructures.
    virtual QDemonPerFrameAllocator &getPerFrameAllocator() = 0;

    // Get the number of times EndFrame has been called
    virtual quint32 getFrameCount() = 0;

    // Get fps
    virtual QPair<float, int> getFPS() = 0;
    // Set fps by higher level, etc application
    virtual void setFPS(QPair<float, int> inFPS) = 0;

    // Currently there are a few things that need to work differently
    // in authoring mode vs. runtime.  The particle effects, for instance
    // need to be framerate-independent at runtime but framerate-dependent during
    // authoring time assuming virtual 16 ms frames.
    // Defaults to falst.
    virtual bool isAuthoringMode() = 0;
    virtual void setAuthoringMode(bool inMode) = 0;

    // This one is setup by the runtime binding
    virtual QDemonRef<QDemonTextRendererInterface> getOnscreenTextRenderer() = 0;
    virtual QDemonRef<QDemonTextTextureAtlasInterface> getTextureAtlas() = 0;

    // Sub presentations change the rendering somewhat.
    virtual bool isInSubPresentation() = 0;
    virtual void setInSubPresentation(bool inValue) = 0;
    virtual void setSceneColor(QDemonOption<QVector4D> inSceneColor) = 0;
    virtual void setMatteColor(QDemonOption<QVector4D> inMatteColor) = 0;

    // render Gpu profiler values
    virtual void renderGpuProfilerStats(float x, float y, QDemonOption<QVector3D> inColor) = 0;

    // The reason you can set both window dimensions and an overall viewport is that the mouse
    // needs to be inverted
    // which requires the window height, and then the rest of the system really requires the
    // viewport.
    virtual void setWindowDimensions(const QSize &inWindowDimensions) = 0;
    virtual QSize getWindowDimensions() = 0;

    // In addition to the window dimensions which really have to be set, you can optionally
    // set the viewport which will force the entire viewer to render specifically to this
    // viewport.
    virtual void setViewport(QDemonOption<QRect> inViewport) = 0;
    virtual QDemonOption<QRect> getViewport() const = 0;
    virtual QRect getContextViewport() const = 0;
    // Only valid between calls to Begin,End.
    virtual QRect getPresentationViewport() const = 0;

    virtual void setScaleMode(ScaleModes inMode) = 0;
    virtual ScaleModes getScaleMode() = 0;

    virtual void setWireframeMode(bool inEnable) = 0;
    virtual bool getWireframeMode() = 0;

    // Return the viewport the system is using to render data to.  This gives the the dimensions
    // of the rendered system.  It is dependent on but not equal to the viewport.
    virtual QRectF getDisplayViewport() const = 0;

    // Layers require the current presentation dimensions in order to render.
    virtual void setPresentationDimensions(const QSize &inPresentationDimensions) = 0;
    virtual QSize getCurrentPresentationDimensions() const = 0;

    virtual void setRenderRotation(RenderRotationValues inRotation) = 0;
    virtual RenderRotationValues getRenderRotation() const = 0;

    virtual QVector2D getMousePickViewport() const = 0;
    virtual QVector2D getMousePickMouseCoords(const QVector2D &inMouseCoords) const = 0;

    // Valid during and just after prepare for render.
    virtual QVector2D getPresentationScaleFactor() const = 0;

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
    virtual void beginFrame() = 0;

    // This runs through the added tasks in reverse order.  This is used to render dependencies
    // before rendering to the main render target.
    virtual void runRenderTasks() = 0;
    // Now you can render to the main render target if you want to render over the top
    // of everything.
    // Next call end frame.
    virtual void endFrame() = 0;
};
QT_END_NAMESPACE

#endif
