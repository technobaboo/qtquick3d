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

#include "qdemonrendercontextcore.h"
#include <QtDemonRuntimeRender/qdemonrendernode.h>
//#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRender/qdemonrendercontext.h>
//#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
//#include <qdemonrendereffectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
//#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrendertexttextureatlas.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
//#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
//#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderbufferloader.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
//#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
//#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
//#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
//#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>

QT_BEGIN_NAMESPACE

namespace {

struct SRenderContextCore : public IQDemonRenderContextCore, public QEnableSharedFromThis<SRenderContextCore>
{
    QSharedPointer<IPerfTimer> m_PerfTimer;
    QSharedPointer<IInputStreamFactory> m_InputStreamFactory;
    QSharedPointer<IThreadPool> m_ThreadPool;
    QSharedPointer<IDynamicObjectSystemCore> m_DynamicObjectSystem;
    //QSharedPointer<ICustomMaterialSystemCore> m_MaterialSystem;
    //QSharedPointer<IEffectSystemCore> m_EffectSystem;
    QSharedPointer<IBufferLoader> m_BufferLoader;
    //QSharedPointer<IRenderPluginManagerCore> m_RenderPluginManagerCore;
    QSharedPointer<ITextRendererCore> m_TextRenderer;
    QSharedPointer<ITextRendererCore> m_OnscreenTexRenderer;
    //QSharedPointer<IPathManagerCore> m_PathManagerCore;

    SRenderContextCore()
        : m_PerfTimer(IPerfTimer::CreatePerfTimer())
        , m_InputStreamFactory(IInputStreamFactory::Create())
        , m_ThreadPool(IThreadPool::CreateThreadPool(4))
    {
        m_DynamicObjectSystem = IDynamicObjectSystemCore::CreateDynamicSystemCore(sharedFromThis());
        //m_MaterialSystem = ICustomMaterialSystemCore::CreateCustomMaterialSystemCore(*this);
        //m_EffectSystem = IEffectSystemCore::CreateEffectSystemCore(*this);
        //m_RenderPluginManagerCore = IRenderPluginManagerCore::Create(fnd, strTable, *m_InputStreamFactory);
        //m_BufferLoader = IBufferLoader::Create(m_Foundation, *m_InputStreamFactory, *m_ThreadPool);
        //m_PathManagerCore = IPathManagerCore::CreatePathManagerCore(*this);
    }

    ~SRenderContextCore() override {}

    QSharedPointer<IInputStreamFactory> GetInputStreamFactory() override { return m_InputStreamFactory; }
    QSharedPointer<IThreadPool> GetThreadPool() override { return m_ThreadPool; }
    QSharedPointer<IDynamicObjectSystemCore> GetDynamicObjectSystemCore() override
    {
        return m_DynamicObjectSystem;
    }
    //QSharedPointer<ICustomMaterialSystemCore> GetMaterialSystemCore() override { return m_MaterialSystem; }
    //QSharedPointer<IEffectSystemCore> GetEffectSystemCore() override { return m_EffectSystem; }
    QSharedPointer<IPerfTimer> GetPerfTimer() override { return m_PerfTimer; }
    QSharedPointer<IBufferLoader> GetBufferLoader() override { return m_BufferLoader; }
    //QSharedPointer<IRenderPluginManagerCore> GetRenderPluginCore() override { return m_RenderPluginManagerCore; }
    //QSharedPointer<IPathManagerCore> GetPathManagerCore() override { return m_PathManagerCore; }
    QSharedPointer<IQDemonRenderContext> CreateRenderContext(QSharedPointer<QDemonRenderContext> inContext, const char *inPrimitivesDirectory) override;
    void SetTextRendererCore(QSharedPointer<ITextRendererCore> inRenderer) override { m_TextRenderer = inRenderer; }
    QSharedPointer<ITextRendererCore> GetTextRendererCore() override { return m_TextRenderer; }
    void SetOnscreenTextRendererCore(QSharedPointer<ITextRendererCore> inRenderer) override
    {
        m_OnscreenTexRenderer = inRenderer;
    }
    QSharedPointer<ITextRendererCore> GetOnscreenTextRendererCore() override { return m_OnscreenTexRenderer; }
};

inline float Clamp(float val, float inMin = 0.0f, float inMax = 1.0f)
{
    if (val < inMin)
        return inMin;
    if (val > inMax)
        return inMax;
    return val;
}

namespace {
void swapXY(QVector2D &v) {
    const auto tmp = v.x();
    v.setX(v.y());
    v.setY(tmp);
}
}

struct SRenderContext : public IQDemonRenderContext, public QEnableSharedFromThis<SRenderContext>
{
    QSharedPointer<QDemonRenderContext> m_RenderContext;
    QSharedPointer<IQDemonRenderContextCore> m_CoreContext;
    QSharedPointer<IPerfTimer> m_PerfTimer;
    QSharedPointer<IInputStreamFactory> m_InputStreamFactory;
    //QSharedPointer<IBufferManager> m_BufferManager;
    QSharedPointer<IResourceManager> m_ResourceManager;
    //QSharedPointer<IOffscreenRenderManager> m_OffscreenRenderManager;
    QSharedPointer<IQDemonRenderer> m_Renderer;
    QSharedPointer<ITextRenderer> m_TextRenderer;
    QSharedPointer<ITextRenderer> m_OnscreenTextRenderer;
    QSharedPointer<ITextTextureCache> m_TextTextureCache;
    QSharedPointer<ITextTextureAtlas> m_TextTextureAtlas;
    QSharedPointer<IDynamicObjectSystem> m_DynamicObjectSystem;
    //QSharedPointer<IEffectSystem> m_EffectSystem;
    QSharedPointer<IShaderCache> m_ShaderCache;
    QSharedPointer<IThreadPool> m_ThreadPool;
    //QSharedPointer<IImageBatchLoader> m_ImageBatchLoader;
    //QSharedPointer<IRenderPluginManager> m_RenderPluginManager;
    //QSharedPointer<ICustomMaterialSystem> m_CustomMaterialSystem;
    //QSharedPointer<IPixelGraphicsRenderer> m_PixelGraphicsRenderer;
    QSharedPointer<IPathManager> m_PathManager;
    //QSharedPointer<IShaderProgramGenerator> m_ShaderProgramGenerator;
    //QSharedPointer<IDefaultMaterialShaderGenerator> m_DefaultMaterialShaderGenerator;
    //QSharedPointer<ICustomMaterialShaderGenerator> m_CustomMaterialShaderGenerator;
    QSharedPointer<IRenderList> m_RenderList;
    quint32 m_FrameCount;
    // Viewport that this render context should use
    QDemonOption<QDemonRenderRect> m_Viewport;
    QSize m_WindowDimensions;
    ScaleModes::Enum m_ScaleMode;
    bool m_WireframeMode;
    bool m_IsInSubPresentation;
    QDemonOption<QVector4D> m_SceneColor;
    QDemonOption<QVector4D> m_MatteColor;
    RenderRotationValues::Enum m_Rotation;
    QSharedPointer<QDemonRenderFrameBuffer> m_RotationFBO;
    QSharedPointer<QDemonRenderTexture2D> m_RotationTexture;
    QSharedPointer<QDemonRenderRenderBuffer> m_RotationDepthBuffer;
    QSharedPointer<QDemonRenderFrameBuffer> m_ContextRenderTarget;
    QDemonRenderRect m_PresentationViewport;
    QSize m_PresentationDimensions;
    QSize m_RenderPresentationDimensions;
    QSize m_PreRenderPresentationDimensions;
    QVector2D m_PresentationScale;
    QDemonRenderRect m_VirtualViewport;
    QPair<float, int> m_FPS;
    bool m_AuthoringMode;

    SRenderContext(QSharedPointer<QDemonRenderContext> ctx, QSharedPointer<IQDemonRenderContextCore> inCore, const char *inApplicationDirectory)
        : m_RenderContext(ctx)
        , m_CoreContext(inCore)
        , m_PerfTimer(inCore->GetPerfTimer())
        , m_InputStreamFactory(inCore->GetInputStreamFactory())
        //, m_ResourceManager(IResourceManager::CreateResourceManager(ctx))
        , m_ShaderCache(IShaderCache::CreateShaderCache(ctx, m_InputStreamFactory, m_PerfTimer))
        , m_ThreadPool(inCore->GetThreadPool())
        , m_FrameCount(0)
        , m_WindowDimensions(800, 480)
        , m_ScaleMode(ScaleModes::ExactSize)
        , m_WireframeMode(false)
        , m_IsInSubPresentation(false)
        , m_Rotation(RenderRotationValues::NoRotation)
        , m_ContextRenderTarget(nullptr)
        , m_PresentationScale(0, 0)
        , m_FPS(qMakePair(0.0, 0))
        , m_AuthoringMode(false)
    {
        //m_BufferManager.reset(IBufferManager::Create(ctx, m_InputStreamFactory, m_PerfTimer))
        //m_RenderList.reset(IRenderList::CreateRenderList())
//        m_OffscreenRenderManager = IOffscreenRenderManager::CreateOffscreenRenderManager(
//                    ctx.GetAllocator(), *m_StringTable, *m_ResourceManager, *this);
//        m_Renderer = IQDemonRenderer::CreateRenderer(*this); // ### "this" should be fixed
        if (inApplicationDirectory && *inApplicationDirectory)
            m_InputStreamFactory->AddSearchDirectory(inApplicationDirectory);

        //m_ImageBatchLoader = IImageBatchLoader::CreateBatchLoader(m_InputStreamFactory,m_BufferManager, m_ThreadPool, m_PerfTimer);

        //m_RenderPluginManager = inCore.GetRenderPluginCore().GetRenderPluginManager(ctx);
        m_DynamicObjectSystem = inCore->GetDynamicObjectSystemCore()->CreateDynamicSystem(this->sharedFromThis());
        //m_EffectSystem = inCore.GetEffectSystemCore().GetEffectSystem(*this);
        //m_CustomMaterialSystem = inCore.GetMaterialSystemCore().GetCustomMaterialSystem(*this);
        // as does the custom material system
        //m_PixelGraphicsRenderer = IPixelGraphicsRenderer::CreateRenderer(*this, *m_StringTable);
        QSharedPointer<ITextRendererCore> theTextCore = inCore->GetTextRendererCore();
        //m_ShaderProgramGenerator = IShaderProgramGenerator::CreateProgramGenerator(*this);
//        m_DefaultMaterialShaderGenerator =
//                IDefaultMaterialShaderGenerator::CreateDefaultMaterialShaderGenerator(*this);
//        m_CustomMaterialShaderGenerator =
//                ICustomMaterialShaderGenerator::CreateCustomMaterialShaderGenerator(*this);
        if (theTextCore) {
            m_TextRenderer = theTextCore->GetTextRenderer(ctx);
            m_TextTextureCache = ITextTextureCache::CreateTextureCache(m_TextRenderer, m_RenderContext);
        }

        QSharedPointer<ITextRendererCore> theOnscreenTextCore = inCore->GetOnscreenTextRendererCore();
        if (theOnscreenTextCore) {
            m_OnscreenTextRenderer = theOnscreenTextCore->GetTextRenderer(ctx);
            m_TextTextureAtlas = ITextTextureAtlas::CreateTextureAtlas(m_OnscreenTextRenderer, m_RenderContext);
        }
        //m_PathManager = inCore->GetPathManagerCore().OnRenderSystemInitialize(*this); // ### "this" should be fixed

        QString versionString;
        switch ((quint32)ctx->GetRenderContextType()) {
        case QDemonRenderContextValues::GLES2:
            versionString = QLatin1Literal("gles2");
            break;
        case QDemonRenderContextValues::GL2:
            versionString = QLatin1Literal("gl2");
            break;
        case QDemonRenderContextValues::GLES3:
            versionString = QLatin1Literal("gles3");
            break;
        case QDemonRenderContextValues::GL3:
            versionString = QLatin1Literal("gl3");
            break;
        case QDemonRenderContextValues::GLES3PLUS:
            versionString = QLatin1Literal("gles3x");
            break;
        case QDemonRenderContextValues::GL4:
            versionString = QLatin1Literal("gl4");
            break;
        default:
            break;
        }

        GetDynamicObjectSystem()->setShaderCodeLibraryVersion(versionString);
#if defined (QDEMON_SHADER_PLATFORM_LIBRARY_DIR)
        const QString platformDirectory;
#if defined(_WIN32)
        platformDirectory = QStringLiteral("res/platform/win");
#elif defined(_LINUX)
        platformDirectory = QStringLiteral("res/platform/linux");
#elif defined(_MACOSX)
        platformDirectory = QStringLiteral("res/platform/macos");
#endif
        GetDynamicObjectSystem().setShaderCodeLibraryPlatformDirectory(platformDirectory);
#endif
    }

    QSharedPointer<IQDemonRenderer> GetRenderer() override { return m_Renderer; }
    //QSharedPointer<IBufferManager> GetBufferManager() override { return m_BufferManager; }
    //QSharedPointer<IResourceManager> GetResourceManager() override { return m_ResourceManager; }
    QSharedPointer<QDemonRenderContext> GetRenderContext() override { return m_RenderContext; }
//    QSharedPointer<IOffscreenRenderManager> GetOffscreenRenderManager() override
//    {
//        return m_OffscreenRenderManager;
//    }
    QSharedPointer<IInputStreamFactory> GetInputStreamFactory() override { return m_InputStreamFactory; }
    //QSharedPointer<IEffectSystem> GetEffectSystem() override { return m_EffectSystem; }
    QSharedPointer<IShaderCache> GetShaderCache() override { return m_ShaderCache; }
    QSharedPointer<IThreadPool> GetThreadPool() override { return m_ThreadPool; }
    //QSharedPointer<IImageBatchLoader> GetImageBatchLoader() override { return m_ImageBatchLoader; }
    QSharedPointer<ITextTextureCache> GetTextureCache() override { return m_TextTextureCache; }
    QSharedPointer<ITextTextureAtlas> GetTextureAtlas() override { return m_TextTextureAtlas; }
    //QSharedPointer<IRenderPluginManager> GetRenderPluginManager() override { return m_RenderPluginManager; }
    QSharedPointer<IDynamicObjectSystem> GetDynamicObjectSystem() override { return m_DynamicObjectSystem; }
    //QSharedPointer<ICustomMaterialSystem> GetCustomMaterialSystem() override { return m_CustomMaterialSystem; }
    //QSharedPointer<IPixelGraphicsRenderer> GetPixelGraphicsRenderer() override { return m_PixelGraphicsRenderer; }
    QSharedPointer<IPerfTimer> GetPerfTimer() override { return m_PerfTimer; }
    QSharedPointer<IRenderList> GetRenderList() override { return m_RenderList; }
    //QSharedPointer<IPathManager> GetPathManager() override { return m_PathManager; }
//    QSharedPointer<IShaderProgramGenerator> GetShaderProgramGenerator() override
//    {
//        return m_ShaderProgramGenerator;
//    }
//    QSharedPointer<IDefaultMaterialShaderGenerator> GetDefaultMaterialShaderGenerator() override
//    {
//        return m_DefaultMaterialShaderGenerator;
//    }
//    QSharedPointer<ICustomMaterialShaderGenerator> GetCustomMaterialShaderGenerator() override
//    {
//        return m_CustomMaterialShaderGenerator;
//    }

    quint32 GetFrameCount() override { return m_FrameCount; }
    void SetFPS(QPair<float, int> inFPS) override { m_FPS = inFPS; }
    QPair<float, int> GetFPS(void) override { return m_FPS; }

    bool IsAuthoringMode() override { return m_AuthoringMode; }
    void SetAuthoringMode(bool inMode) override { m_AuthoringMode = inMode; }

    bool IsInSubPresentation() override { return m_IsInSubPresentation; }
    void SetInSubPresentation(bool inValue) override { m_IsInSubPresentation = inValue; }

    QSharedPointer<ITextRenderer> GetTextRenderer() override { return m_TextRenderer; }

    QSharedPointer<ITextRenderer> GetOnscreenTextRenderer() override { return m_OnscreenTextRenderer; }

    void SetSceneColor(QDemonOption<QVector4D> inSceneColor) override { m_SceneColor = inSceneColor; }
    void SetMatteColor(QDemonOption<QVector4D> inMatteColor) override { m_MatteColor = inMatteColor; }

    void SetWindowDimensions(const QSize &inWindowDimensions) override
    {
        m_WindowDimensions = inWindowDimensions;
    }

    QSize GetWindowDimensions() override { return m_WindowDimensions; }

    void SetScaleMode(ScaleModes::Enum inMode) override { m_ScaleMode = inMode; }

    ScaleModes::Enum GetScaleMode() override { return m_ScaleMode; }

    void SetWireframeMode(bool inEnable) override { m_WireframeMode = inEnable; }

    bool GetWireframeMode() override { return m_WireframeMode; }

    void SetViewport(QDemonOption<QDemonRenderRect> inViewport) override { m_Viewport = inViewport; }
    QDemonOption<QDemonRenderRect> GetViewport() const override { return m_Viewport; }

//    QSharedPointer<IRenderWidgetContext> GetRenderWidgetContext() override
//    {
//        return m_Renderer->GetRenderWidgetContext();
//    }

    QPair<QDemonRenderRect, QDemonRenderRect> GetPresentationViewportAndOuterViewport() const
    {
        QSize thePresentationDimensions(m_PresentationDimensions);
        QDemonRenderRect theOuterViewport(GetContextViewport());
        if (m_Rotation == RenderRotationValues::Clockwise90
                || m_Rotation == RenderRotationValues::Clockwise270) {
            std::swap(theOuterViewport.m_Width, theOuterViewport.m_Height);
            std::swap(theOuterViewport.m_X, theOuterViewport.m_Y);
        }
        // Calculate the presentation viewport perhaps with the window width and height swapped.
        return QPair<QDemonRenderRect, QDemonRenderRect>(
                    GetPresentationViewport(theOuterViewport, m_ScaleMode, thePresentationDimensions),
                    theOuterViewport);
    }

    QDemonRenderRectF GetDisplayViewport() const override
    {
        return GetPresentationViewportAndOuterViewport().first;
    }

    void SetPresentationDimensions(const QSize &inPresentationDimensions) override
    {
        m_PresentationDimensions = inPresentationDimensions;
    }
    QSize GetCurrentPresentationDimensions() const override
    {
        return m_PresentationDimensions;
    }

    void SetRenderRotation(RenderRotationValues::Enum inRotation) override
    {
        m_Rotation = inRotation;
    }

    RenderRotationValues::Enum GetRenderRotation() const override { return m_Rotation; }
    QVector2D GetMousePickViewport() const override
    {
        bool renderOffscreen = m_Rotation != RenderRotationValues::NoRotation;
        if (renderOffscreen)
            return QVector2D((float)m_PresentationViewport.m_Width,
                             (float)m_PresentationViewport.m_Height);
        else
            return QVector2D((float)m_WindowDimensions.width(), (float)m_WindowDimensions.height());
    }
    QDemonRenderRect GetContextViewport() const override
    {
        QDemonRenderRect retval;
        if (m_Viewport.hasValue())
            retval = *m_Viewport;
        else
            retval = QDemonRenderRect(0, 0, m_WindowDimensions.width(), m_WindowDimensions.height());

        return retval;
    }

    QVector2D GetMousePickMouseCoords(const QVector2D &inMouseCoords) const override
    {
        bool renderOffscreen = m_Rotation != RenderRotationValues::NoRotation;
        if (renderOffscreen) {
            QSize thePresentationDimensions(m_RenderPresentationDimensions);
            QDemonRenderRect theViewport(GetContextViewport());
            // Calculate the presentation viewport perhaps with the presentation width and height
            // swapped.
            QDemonRenderRect thePresentationViewport =
                    GetPresentationViewport(theViewport, m_ScaleMode, thePresentationDimensions);
            // Translate pick into presentation space without rotations or anything else.
            float YHeightDiff = (float)((float)m_WindowDimensions.height()
                                        - (float)thePresentationViewport.m_Height);
            QVector2D theLocalMouse((inMouseCoords.x() - thePresentationViewport.m_X),
                                    (inMouseCoords.y() - YHeightDiff + thePresentationViewport.m_Y));
            switch (m_Rotation) {
            default:
            case RenderRotationValues::NoRotation:
                Q_ASSERT(false);
                break;
            case RenderRotationValues::Clockwise90:
                swapXY(theLocalMouse);
                theLocalMouse.setY(thePresentationViewport.m_Width - theLocalMouse.y());
                break;
            case RenderRotationValues::Clockwise180:
                theLocalMouse.setY(thePresentationViewport.m_Height - theLocalMouse.y());
                theLocalMouse.setX(thePresentationViewport.m_Width - theLocalMouse.x());
                break;
            case RenderRotationValues::Clockwise270:
                swapXY(theLocalMouse);
                theLocalMouse.setX(thePresentationViewport.m_Height - theLocalMouse.x());
                break;
            }
            return theLocalMouse;
        }
        return inMouseCoords;
    }

    QDemonRenderRect GetPresentationViewport(const QDemonRenderRect &inViewerViewport,
                                             ScaleModes::Enum inScaleToFit,
                                             const QSize &inPresDimensions) const
    {
        QDemonRenderRect retval;
        qint32 theWidth = inViewerViewport.m_Width;
        qint32 theHeight = inViewerViewport.m_Height;
        if (inPresDimensions.width() == 0 || inPresDimensions.height() == 0)
            return QDemonRenderRect(0, 0, 0, 0);
        // Setup presentation viewport.  This may or may not match the physical viewport that we
        // want to setup.
        // Avoiding scaling keeps things as sharp as possible.
        if (inScaleToFit == ScaleModes::ExactSize) {
            retval.m_Width = inPresDimensions.width();
            retval.m_Height = inPresDimensions.height();
            retval.m_X = (theWidth - (qint32)inPresDimensions.width()) / 2;
            retval.m_Y = (theHeight - (qint32)inPresDimensions.height()) / 2;
        } else if (inScaleToFit == ScaleModes::ScaleToFit
                   || inScaleToFit == ScaleModes::FitSelected) {
            // Scale down in such a way to preserve aspect ratio.
            float screenAspect = (float)theWidth / (float)theHeight;
            float thePresentationAspect =
                    (float)inPresDimensions.width() / (float)inPresDimensions.height();
            if (screenAspect >= thePresentationAspect) {
                // if the screen height is the limiting factor
                retval.m_Y = 0;
                retval.m_Height = theHeight;
                retval.m_Width = (qint32)(thePresentationAspect * retval.m_Height);
                retval.m_X = (theWidth - retval.m_Width) / 2;
            } else {
                retval.m_X = 0;
                retval.m_Width = theWidth;
                retval.m_Height = (qint32)(retval.m_Width / thePresentationAspect);
                retval.m_Y = (theHeight - retval.m_Height) / 2;
            }
        } else {
            // Setup the viewport for everything and let the presentations figure it out.
            retval.m_X = 0;
            retval.m_Y = 0;
            retval.m_Width = theWidth;
            retval.m_Height = theHeight;
        }
        retval.m_X += inViewerViewport.m_X;
        retval.m_Y += inViewerViewport.m_Y;
        return retval;
    }

    void RenderText2D(float x, float y, QDemonOption<QVector3D> inColor,
                      const char *text) override
    {
        m_Renderer->RenderText2D(x, y, inColor, text);
    }

    void RenderGpuProfilerStats(float x, float y,
                                QDemonOption<QVector3D> inColor) override
    {
        m_Renderer->RenderGpuProfilerStats(x, y, inColor);
    }

    QDemonRenderRect GetPresentationViewport() const override { return m_PresentationViewport; }
    struct SBeginFrameResult
    {
        bool m_RenderOffscreen;
        QSize m_PresentationDimensions;
        bool m_ScissorTestEnabled;
        QDemonRenderRect m_ScissorRect;
        QDemonRenderRect m_Viewport;
        QSize m_FBODimensions;
        SBeginFrameResult(bool ro, QSize presDims, bool scissorEnabled,
                          QDemonRenderRect scissorRect, QDemonRenderRect viewport,
                          QSize fboDims)
            : m_RenderOffscreen(ro)
            , m_PresentationDimensions(presDims)
            , m_ScissorTestEnabled(scissorEnabled)
            , m_ScissorRect(scissorRect)
            , m_Viewport(viewport)
            , m_FBODimensions(fboDims)
        {
        }
        SBeginFrameResult() {}
    };

    // Calculated values passed from beginframe to setupRenderTarget.
    // Trying to avoid duplicate code as much as possible.
    SBeginFrameResult m_BeginFrameResult;

    void BeginFrame() override
    {
        m_PreRenderPresentationDimensions = m_PresentationDimensions;
        QSize thePresentationDimensions(m_PreRenderPresentationDimensions);
        QDemonRenderRect theContextViewport(GetContextViewport());
        IRenderList &theRenderList(*m_RenderList);
        theRenderList.BeginFrame();
        if (m_Viewport.hasValue()) {
            theRenderList.SetScissorTestEnabled(true);
            theRenderList.SetScissorRect(theContextViewport);
        } else {
            theRenderList.SetScissorTestEnabled(false);
        }
        bool renderOffscreen = m_Rotation != RenderRotationValues::NoRotation;
        QPair<QDemonRenderRect, QDemonRenderRect> thePresViewportAndOuterViewport =
                GetPresentationViewportAndOuterViewport();
        QDemonRenderRect theOuterViewport = thePresViewportAndOuterViewport.second;
        // Calculate the presentation viewport perhaps with the window width and height swapped.
        QDemonRenderRect thePresentationViewport = thePresViewportAndOuterViewport.first;
        m_PresentationViewport = thePresentationViewport;
        m_PresentationScale = QVector2D(
                    (float)thePresentationViewport.m_Width / (float)thePresentationDimensions.width(),
                    (float)thePresentationViewport.m_Height / (float)thePresentationDimensions.height());
        QSize fboDimensions;
        if (thePresentationViewport.m_Width > 0 && thePresentationViewport.m_Height > 0) {
            if (renderOffscreen == false) {
                m_PresentationDimensions = QSize(thePresentationViewport.m_Width,
                                                 thePresentationViewport.m_Height);
                m_RenderList->SetViewport(thePresentationViewport);
                if (thePresentationViewport.m_X || thePresentationViewport.m_Y
                        || thePresentationViewport.m_Width != (qint32)theOuterViewport.m_Width
                        || thePresentationViewport.m_Height != (qint32)theOuterViewport.m_Height) {
                    m_RenderList->SetScissorRect(thePresentationViewport);
                    m_RenderList->SetScissorTestEnabled(true);
                }
            } else {
                quint32 imageWidth = ITextRenderer::NextMultipleOf4(thePresentationViewport.m_Width);
                quint32 imageHeight =
                        ITextRenderer::NextMultipleOf4(thePresentationViewport.m_Height);
                fboDimensions = QSize(imageWidth, imageHeight);
                m_PresentationDimensions = QSize(thePresentationViewport.m_Width,
                                                 thePresentationViewport.m_Height);
                QDemonRenderRect theSceneViewport = QDemonRenderRect(0, 0, imageWidth, imageHeight);
                m_RenderList->SetScissorTestEnabled(false);
                m_RenderList->SetViewport(theSceneViewport);
            }
        }

        m_BeginFrameResult = SBeginFrameResult(
                    renderOffscreen, m_PresentationDimensions, m_RenderList->IsScissorTestEnabled(),
                    m_RenderList->GetScissor(), m_RenderList->GetViewport(), fboDimensions);

        m_Renderer->BeginFrame();
        //m_OffscreenRenderManager->BeginFrame();
        if (m_TextRenderer)
            m_TextRenderer->BeginFrame();
        if (m_TextTextureCache)
            m_TextTextureCache->BeginFrame();
        //m_ImageBatchLoader->BeginFrame();
    }

    QVector2D GetPresentationScaleFactor() const override { return m_PresentationScale; }

    virtual void SetupRenderTarget()
    {
        QDemonRenderRect theContextViewport(GetContextViewport());
        if (m_Viewport.hasValue()) {
            m_RenderContext->SetScissorTestEnabled(true);
            m_RenderContext->SetScissorRect(theContextViewport);
        } else {
            m_RenderContext->SetScissorTestEnabled(false);
        }
        {
            QVector4D theClearColor;
            if (m_MatteColor.hasValue())
                theClearColor = m_MatteColor;
            else
                theClearColor = m_SceneColor;
            m_RenderContext->SetClearColor(theClearColor);
            m_RenderContext->Clear(QDemonRenderClearValues::Color);
        }
        bool renderOffscreen = m_BeginFrameResult.m_RenderOffscreen;
        m_RenderContext->SetViewport(m_BeginFrameResult.m_Viewport);
        m_RenderContext->SetScissorRect(m_BeginFrameResult.m_ScissorRect);
        m_RenderContext->SetScissorTestEnabled(m_BeginFrameResult.m_ScissorTestEnabled);

        if (m_PresentationViewport.m_Width > 0 && m_PresentationViewport.m_Height > 0) {
            if (renderOffscreen == false) {
                if (m_RotationFBO != nullptr) {
//                    m_ResourceManager->Release(*m_RotationFBO);
//                    m_ResourceManager->Release(*m_RotationTexture);
//                    m_ResourceManager->Release(*m_RotationDepthBuffer);
                    m_RotationFBO = nullptr;
                    m_RotationTexture = nullptr;
                    m_RotationDepthBuffer = nullptr;
                }
                if (m_SceneColor.hasValue() && m_SceneColor.getValue().w() != 0.0f) {
                    m_RenderContext->SetClearColor(m_SceneColor);
                    m_RenderContext->Clear(QDemonRenderClearValues::Color);
                }
            } else {
                quint32 imageWidth = m_BeginFrameResult.m_FBODimensions.width();
                quint32 imageHeight = m_BeginFrameResult.m_FBODimensions.height();
                QDemonRenderTextureFormats::Enum theColorBufferFormat = QDemonRenderTextureFormats::RGBA8;
                QDemonRenderRenderBufferFormats::Enum theDepthBufferFormat = QDemonRenderRenderBufferFormats::Depth16;
                m_ContextRenderTarget = m_RenderContext->GetRenderTarget();
                if (m_RotationFBO == nullptr) {
                    m_RotationFBO = m_ResourceManager->AllocateFrameBuffer();
                    m_RotationTexture = m_ResourceManager->AllocateTexture2D(
                                imageWidth, imageHeight, theColorBufferFormat);
                    m_RotationDepthBuffer = m_ResourceManager->AllocateRenderBuffer(
                                imageWidth, imageHeight, theDepthBufferFormat);
                    m_RotationFBO->Attach(QDemonRenderFrameBufferAttachments::Color0,
                                          m_RotationTexture);
                    m_RotationFBO->Attach(QDemonRenderFrameBufferAttachments::Depth,
                                          m_RotationDepthBuffer);
                } else {
                    STextureDetails theDetails = m_RotationTexture->GetTextureDetails();
                    if (theDetails.m_Width != imageWidth || theDetails.m_Height != imageHeight) {
                        m_RotationTexture->SetTextureData(QDemonDataRef<quint8>(), 0, imageWidth,
                                                          imageHeight, theColorBufferFormat);
                        m_RotationDepthBuffer->SetDimensions(
                                    QDemonRenderRenderBufferDimensions(imageWidth, imageHeight));
                    }
                }
                m_RenderContext->SetRenderTarget(m_RotationFBO);
                if (m_SceneColor.hasValue()) {
                    m_RenderContext->SetClearColor(m_SceneColor);
                    m_RenderContext->Clear(QDemonRenderClearValues::Color);
                }
            }
        }
    }

    void RunRenderTasks() override
    {
        m_RenderList->RunRenderTasks();
        SetupRenderTarget();
    }

    // Note this runs before EndFrame
    virtual void TeardownRenderTarget()
    {
        if (m_RotationFBO) {
            ScaleModes::Enum theScaleToFit = m_ScaleMode;
            QDemonRenderRect theOuterViewport(GetContextViewport());
            m_RenderContext->SetRenderTarget(m_ContextRenderTarget);
            QSize thePresentationDimensions = GetCurrentPresentationDimensions();
            if (m_Rotation == RenderRotationValues::Clockwise90
                    || m_Rotation == RenderRotationValues::Clockwise270) {
                thePresentationDimensions = QSize(thePresentationDimensions.height(),
                                                  thePresentationDimensions.width());
            }
            m_RenderPresentationDimensions = thePresentationDimensions;
            // Calculate the presentation viewport perhaps with the presentation width and height
            // swapped.
            QDemonRenderRect thePresentationViewport =
                    GetPresentationViewport(theOuterViewport, theScaleToFit, thePresentationDimensions);
            SCamera theCamera;
            switch (m_Rotation) {
            default:
                Q_ASSERT(false);
                break;
            case RenderRotationValues::Clockwise90:
                theCamera.m_Rotation.setZ(90);
                break;
            case RenderRotationValues::Clockwise180:
                theCamera.m_Rotation.setZ(180);
                break;
            case RenderRotationValues::Clockwise270:
                theCamera.m_Rotation.setZ(270);
                break;
            }
            float z = theCamera.m_Rotation.z();
            TORAD(z);
            theCamera.m_Rotation.setZ(z);
            theCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
            theCamera.m_Flags.SetOrthographic(true);
            m_RenderContext->SetViewport(thePresentationViewport);
            QVector2D theCameraDimensions((float)thePresentationViewport.m_Width,
                                          (float)thePresentationViewport.m_Height);
            theCamera.CalculateGlobalVariables(
                        QDemonRenderRect(0, 0, (quint32)thePresentationViewport.m_Width,
                                         (quint32)thePresentationViewport.m_Height),
                        theCameraDimensions);
            QMatrix4x4 theVP;
            theCamera.CalculateViewProjectionMatrix(theVP);
            SNode theTempNode;
            theTempNode.CalculateGlobalVariables();
            QMatrix4x4 theMVP;
            QMatrix3x3 theNormalMat;
            theTempNode.CalculateMVPAndNormalMatrix(theVP, theMVP, theNormalMat);
            m_RenderContext->SetCullingEnabled(false);
            m_RenderContext->SetBlendingEnabled(false);
            m_RenderContext->SetDepthTestEnabled(false);
            m_Renderer->RenderQuad(QVector2D((float)m_PresentationViewport.m_Width,
                                             (float)m_PresentationViewport.m_Height),
                                   theMVP, *m_RotationTexture);
        }
    }

    void EndFrame() override
    {
        TeardownRenderTarget();
        //m_ImageBatchLoader->EndFrame();
        if (m_TextTextureCache)
            m_TextTextureCache->EndFrame();
        if (m_TextRenderer)
            m_TextRenderer->EndFrame();
        //m_OffscreenRenderManager->EndFrame();
        m_Renderer->EndFrame();
        //m_CustomMaterialSystem->EndFrame();
        m_PresentationDimensions = m_PreRenderPresentationDimensions;
        ++m_FrameCount;
    }
};

QSharedPointer<IQDemonRenderContext> SRenderContextCore::CreateRenderContext(QSharedPointer<QDemonRenderContext> inContext, const char *inPrimitivesDirectory)
{
    return QSharedPointer<IQDemonRenderContext>(new SRenderContext(inContext, this->sharedFromThis(), inPrimitivesDirectory));
}
}

IQDemonRenderContextCore::~IQDemonRenderContextCore()
{

}

QSharedPointer<IQDemonRenderContextCore> IQDemonRenderContextCore::Create()
{
    return QSharedPointer<SRenderContextCore>(new SRenderContextCore());
}

QT_END_NAMESPACE
