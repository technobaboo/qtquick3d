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
#include <qdemonrender.h>
#include <Qt3DSAllocatorCallback.h>
#include <QDemonRefCounted>
#include <StringTable.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>

#include <QtCore/qpair.h>
#include <QtCore/qsize.h>

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
class IQt3DSRenderContextCore : public QDemonRefCounted
{
public:
    virtual IStringTable &GetStringTable() = 0;
    virtual NVFoundationBase &GetFoundation() = 0;
    virtual NVAllocatorCallback &GetAllocator() = 0;
    virtual IInputStreamFactory &GetInputStreamFactory() = 0;
    virtual IThreadPool &GetThreadPool() = 0;
    virtual IDynamicObjectSystemCore &GetDynamicObjectSystemCore() = 0;
    virtual ICustomMaterialSystemCore &GetMaterialSystemCore() = 0;
    virtual IEffectSystemCore &GetEffectSystemCore() = 0;
    virtual IPerfTimer &GetPerfTimer() = 0;
    virtual IBufferLoader &GetBufferLoader() = 0;
    virtual IRenderPluginManagerCore &GetRenderPluginCore() = 0;
    virtual IPathManagerCore &GetPathManagerCore() = 0;
    // Text renderers may be provided by clients at runtime.
    virtual void SetTextRendererCore(ITextRendererCore &inRenderer) = 0;
    virtual ITextRendererCore *GetTextRendererCore() = 0;
    // this is our default 2D text onscreen renderer
    virtual void SetOnscreenTextRendererCore(ITextRendererCore &inRenderer) = 0;
    virtual ITextRendererCore *GetOnscreenTextRendererCore() = 0;
    // The render context maintains a reference to this object.
    virtual IQt3DSRenderContext &CreateRenderContext(QDemonRenderContext &inContext,
                                                     const char8_t *inPrimitivesDirectory) = 0;

    static IQt3DSRenderContextCore &Create(NVFoundationBase &fnd, IStringTable &strt);
};

class IQt3DSRenderContext : public QDemonRefCounted
{
protected:
    virtual ~IQt3DSRenderContext() {}
public:
    virtual IStringTable &GetStringTable() = 0;
    virtual NVFoundationBase &GetFoundation() = 0;
    virtual NVAllocatorCallback &GetAllocator() = 0;
    virtual IQt3DSRenderer &GetRenderer() = 0;
    virtual IRenderWidgetContext &GetRenderWidgetContext() = 0;
    virtual IBufferManager &GetBufferManager() = 0;
    virtual IResourceManager &GetResourceManager() = 0;
    virtual QDemonRenderContext &GetRenderContext() = 0;
    virtual IOffscreenRenderManager &GetOffscreenRenderManager() = 0;
    virtual IInputStreamFactory &GetInputStreamFactory() = 0;
    virtual IEffectSystem &GetEffectSystem() = 0;
    virtual IShaderCache &GetShaderCache() = 0;
    virtual IThreadPool &GetThreadPool() = 0;
    virtual IImageBatchLoader &GetImageBatchLoader() = 0;
    virtual IRenderPluginManager &GetRenderPluginManager() = 0;
    virtual IDynamicObjectSystem &GetDynamicObjectSystem() = 0;
    virtual ICustomMaterialSystem &GetCustomMaterialSystem() = 0;
    virtual IPixelGraphicsRenderer &GetPixelGraphicsRenderer() = 0;
    virtual IPerfTimer &GetPerfTimer() = 0;
    virtual ITextTextureCache *GetTextureCache() = 0;
    virtual ITextRenderer *GetTextRenderer() = 0;
    virtual IRenderList &GetRenderList() = 0;
    virtual IPathManager &GetPathManager() = 0;
    virtual IShaderProgramGenerator &GetShaderProgramGenerator() = 0;
    virtual IDefaultMaterialShaderGenerator &GetDefaultMaterialShaderGenerator() = 0;
    virtual ICustomMaterialShaderGenerator &GetCustomMaterialShaderGenerator() = 0;
    // The memory used for the per frame allocator is released as the first step in BeginFrame.
    // This is useful for short lived objects and datastructures.
    virtual NVAllocatorCallback &GetPerFrameAllocator() = 0;
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
    virtual ITextRenderer *GetOnscreenTextRenderer() = 0;
    virtual ITextTextureAtlas *GetTextureAtlas() = 0;

    // Sub presentations change the rendering somewhat.
    virtual bool IsInSubPresentation() = 0;
    virtual void SetInSubPresentation(bool inValue) = 0;
    virtual void SetSceneColor(Option<QVector4D> inSceneColor) = 0;
    virtual void SetMatteColor(Option<QVector4D> inMatteColor) = 0;

    // Render screen aligned 2D text at x,y
    virtual void RenderText2D(float x, float y, Option<QVector3D> inColor,
                              const char *text) = 0;
    // render Gpu profiler values
    virtual void RenderGpuProfilerStats(float x, float y,
                                        Option<QVector3D> inColor) = 0;

    // The reason you can set both window dimensions and an overall viewport is that the mouse
    // needs to be inverted
    // which requires the window height, and then the rest of the system really requires the
    // viewport.
    virtual void SetWindowDimensions(const QSize &inWindowDimensions) = 0;
    virtual QSize GetWindowDimensions() = 0;

    // In addition to the window dimensions which really have to be set, you can optionally
    // set the viewport which will force the entire viewer to render specifically to this
    // viewport.
    virtual void SetViewport(Option<QDemonRenderRect> inViewport) = 0;
    virtual Option<QDemonRenderRect> GetViewport() const = 0;
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
