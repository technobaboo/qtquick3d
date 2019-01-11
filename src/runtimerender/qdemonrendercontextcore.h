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
#ifndef QDEMON_RENDER_CONTEXT_CORE_H
#define QDEMON_RENDER_CONTEXT_CORE_H

// ### TODO Re-enable subsystems when they are done

#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
//#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
//#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
//#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderbufferloader.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
//#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
//#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
//#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
//#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>

#include <QtDemon/qdemonperftimer.h>

#include <QtCore/QPair>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE
struct ScaleModes
{
    enum Enum {
        ExactSize = 0, // Ensure the viewport is exactly same size as application
        ScaleToFit = 1, // Resize viewport keeping aspect ratio
        ScaleToFill = 2, // Resize viewport to entire window
        FitSelected = 3, // Resize presentation to fit into viewport
    };
};

// Part of render context that does not require the render system.
class IQDemonRenderContextCore
{
public:
    virtual QSharedPointer<IInputStreamFactory> GetInputStreamFactory() = 0;
    virtual QSharedPointer<IThreadPool> GetThreadPool() = 0;
    //    virtual QSharedPointer<IDynamicObjectSystemCore> GetDynamicObjectSystemCore() = 0;
    //    virtual QSharedPointer<ICustomMaterialSystemCore> GetMaterialSystemCore() = 0;
    //    virtual QSharedPointer<IEffectSystemCore> GetEffectSystemCore() = 0;
    virtual QSharedPointer<IPerfTimer> GetPerfTimer() = 0;
    virtual QSharedPointer<IBufferLoader> GetBufferLoader() = 0;
    //    virtual QSharedPointer<IRenderPluginManagerCore> GetRenderPluginCore() = 0;
    //    virtual QSharedPointer<IPathManagerCore> GetPathManagerCore() = 0;
    // Text renderers may be provided by clients at runtime.
    virtual void SetTextRendererCore(QSharedPointer<ITextRendererCore> inRenderer) = 0;
    virtual QSharedPointer<ITextRendererCore> GetTextRendererCore() = 0;
    // this is our default 2D text onscreen renderer
    virtual void SetOnscreenTextRendererCore(QSharedPointer<ITextRendererCore> inRenderer) = 0;
    virtual QSharedPointer<ITextRendererCore> GetOnscreenTextRendererCore() = 0;
    // The render context maintains a reference to this object.
    virtual QSharedPointer<IQDemonRenderContext> CreateRenderContext(QSharedPointer<QDemonRenderContext> inContext, const char *inPrimitivesDirectory) = 0;

    static QSharedPointer<IQDemonRenderContextCore> Create();
};

class ITextTextureAtlas;
class IQDemonRenderer;

class IQDemonRenderContext
{
protected:
    virtual ~IQDemonRenderContext() {}
public:
        virtual QSharedPointer<IQDemonRenderer> GetRenderer() = 0;
    //    virtual QSharedPointer<IRenderWidgetContext> GetRenderWidgetContext() = 0;
    //    virtual QSharedPointer<IBufferManager> GetBufferManager() = 0;
    //    virtual QSharedPointer<IResourceManager> GetResourceManager() = 0;
    virtual QSharedPointer<QDemonRenderContext> GetRenderContext() = 0;
    //    virtual QSharedPointer<IOffscreenRenderManager> GetOffscreenRenderManager() = 0;
    virtual QSharedPointer<IInputStreamFactory> GetInputStreamFactory() = 0;
    //    virtual QSharedPointer<IEffectSystem> GetEffectSystem() = 0;
    //    virtual QSharedPointer<IShaderCache> GetShaderCache() = 0;
    virtual QSharedPointer<IThreadPool> GetThreadPool() = 0;
    //    virtual QSharedPointer<IImageBatchLoader> GetImageBatchLoader() = 0;
    //    virtual QSharedPointer<IRenderPluginManager> GetRenderPluginManager() = 0;
    //    virtual QSharedPointer<IDynamicObjectSystem> GetDynamicObjectSystem() = 0;
    //    virtual QSharedPointer<ICustomMaterialSystem> GetCustomMaterialSystem() = 0;
    //    virtual QSharedPointer<IPixelGraphicsRenderer> GetPixelGraphicsRenderer() = 0;
    virtual QSharedPointer<IPerfTimer> GetPerfTimer() = 0;
    virtual QSharedPointer<ITextTextureCache> GetTextureCache() = 0;
    virtual QSharedPointer<ITextRenderer> GetTextRenderer() = 0;
    virtual QSharedPointer<IRenderList> GetRenderList() = 0;
    //virtual QSharedPointer<IPathManager> GetPathManager() = 0;
    //    virtual QSharedPointer<IShaderProgramGenerator> GetShaderProgramGenerator() = 0;
    //    virtual QSharedPointer<IDefaultMaterialShaderGenerator> GetDefaultMaterialShaderGenerator() = 0;
    //    virtual QSharedPointer<ICustomMaterialShaderGenerator> GetCustomMaterialShaderGenerator() = 0;
    // Get the number of times EndFrame has been called
    virtual quint32 GetFrameCount() = 0;

    // Get fps
    virtual QPair<float, int> GetFPS() = 0;
    // Set fps by higher level, etc application
    virtual void SetFPS(QPair<float, int> inFPS) = 0;

    // Currently there are a few things that need to work differently
    // in authoring mode vs. runtime.  The particle effects, for instance
    // need to be framerate-independent at runtime but framerate-dependent during
    // authoring time assuming virtual 16 ms frames.
    // Defaults to falst.
    virtual bool IsAuthoringMode() = 0;
    virtual void SetAuthoringMode(bool inMode) = 0;

    // This one is setup by the runtime binding
    virtual QSharedPointer<ITextRenderer> GetOnscreenTextRenderer() = 0;
    virtual QSharedPointer<ITextTextureAtlas> GetTextureAtlas() = 0;

    // Sub presentations change the rendering somewhat.
    virtual bool IsInSubPresentation() = 0;
    virtual void SetInSubPresentation(bool inValue) = 0;
    virtual void SetSceneColor(QDemonOption<QVector4D> inSceneColor) = 0;
    virtual void SetMatteColor(QDemonOption<QVector4D> inMatteColor) = 0;

    // Render screen aligned 2D text at x,y
    virtual void RenderText2D(float x, float y, QDemonOption<QVector3D> inColor,
                              const char *text) = 0;
    // render Gpu profiler values
    virtual void RenderGpuProfilerStats(float x, float y,
                                        QDemonOption<QVector3D> inColor) = 0;

    // The reason you can set both window dimensions and an overall viewport is that the mouse
    // needs to be inverted
    // which requires the window height, and then the rest of the system really requires the
    // viewport.
    virtual void SetWindowDimensions(const QSize &inWindowDimensions) = 0;
    virtual QSize GetWindowDimensions() = 0;

    // In addition to the window dimensions which really have to be set, you can optionally
    // set the viewport which will force the entire viewer to render specifically to this
    // viewport.
    virtual void SetViewport(QDemonOption<QDemonRenderRect> inViewport) = 0;
    virtual QDemonOption<QDemonRenderRect> GetViewport() const = 0;
    virtual QDemonRenderRect GetContextViewport() const = 0;
    // Only valid between calls to Begin,End.
    virtual QDemonRenderRect GetPresentationViewport() const = 0;

    virtual void SetScaleMode(ScaleModes::Enum inMode) = 0;
    virtual ScaleModes::Enum GetScaleMode() = 0;

    virtual void SetWireframeMode(bool inEnable) = 0;
    virtual bool GetWireframeMode() = 0;

    // Return the viewport the system is using to render data to.  This gives the the dimensions
    // of the rendered system.  It is dependent on but not equal to the viewport.
    virtual QDemonRenderRectF GetDisplayViewport() const = 0;

    // Layers require the current presentation dimensions in order to render.
    virtual void
    SetPresentationDimensions(const QSize &inPresentationDimensions) = 0;
    virtual QSize GetCurrentPresentationDimensions() const = 0;

    virtual void SetRenderRotation(RenderRotationValues::Enum inRotation) = 0;
    virtual RenderRotationValues::Enum GetRenderRotation() const = 0;

    virtual QVector2D GetMousePickViewport() const = 0;
    virtual QVector2D GetMousePickMouseCoords(const QVector2D &inMouseCoords) const = 0;

    // Valid during and just after prepare for render.
    virtual QVector2D GetPresentationScaleFactor() const = 0;

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
    virtual void BeginFrame() = 0;

    // This runs through the added tasks in reverse order.  This is used to render dependencies
    // before rendering to the main render target.
    virtual void RunRenderTasks() = 0;
    // Now you can render to the main render target if you want to render over the top
    // of everything.
    // Next call end frame.
    virtual void EndFrame() = 0;
};
QT_END_NAMESPACE

#endif
