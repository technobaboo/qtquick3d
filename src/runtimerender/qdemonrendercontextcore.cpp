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
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrendertexttextureatlas.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonperframeallocator.h>
#include <QtDemonRuntimeRender/qdemonrendererimpl.h>

QT_BEGIN_NAMESPACE

namespace {

struct QDemonRenderContextCore : public QDemonRenderContextCoreInterface
{
    QDemonPerfTimer m_perfTimer;
    QDemonRef<QDemonInputStreamFactoryInterface> m_inputStreamFactory;
    QDemonRef<QDemonAbstractThreadPool> m_threadPool;
    QDemonRef<QDemonDynamicObjectSystemInterface> m_dynamicObjectSystem;
    QDemonMaterialSystem m_materialSystem;
    QDemonRef<QDemonEffectSystemInterface> m_effectSystem;
    QDemonRef<QDemonTextRendererInterface> m_textRenderer;
    QDemonRef<QDemonTextRendererInterface> m_onscreenTexRenderer;
    QDemonRef<QDemonPathManagerInterface> m_pathManagerCore;

    QDemonRenderContextCore()
        : m_perfTimer(QDemonPerfTimer::create())
        , m_inputStreamFactory(QDemonInputStreamFactoryInterface::create())
        , m_threadPool(QDemonAbstractThreadPool::createThreadPool(4))
    {
        m_dynamicObjectSystem = QDemonDynamicObjectSystemInterface::createDynamicSystem(this);
        m_materialSystem = QDemonMaterialSystem(this);
        m_effectSystem = QDemonEffectSystemInterface::createEffectSystem(this);
        m_pathManagerCore = QDemonPathManagerInterface::createPathManager(this);
    }

    ~QDemonRenderContextCore() override = default;

    QDemonRef<QDemonInputStreamFactoryInterface> getInputStreamFactory() override { return m_inputStreamFactory; }
    QDemonRef<QDemonAbstractThreadPool> getThreadPool() override { return m_threadPool; }
    QDemonRef<QDemonDynamicObjectSystemInterface> getDynamicObjectSystemCore() override
    {
        return m_dynamicObjectSystem;
    }
    QDemonMaterialSystem getMaterialSystemCore() override { return m_materialSystem; }
    QDemonRef<QDemonEffectSystemInterface> getEffectSystemCore() override { return m_effectSystem; }
    QDemonPerfTimer getPerfTimer() override { return m_perfTimer; }
    QDemonRef<QDemonPathManagerInterface> getPathManagerCore() override { return m_pathManagerCore; }
    QDemonRef<QDemonRenderContextInterface> createRenderContext(QDemonRef<QDemonRenderContext> inContext,
                                                                const char *inPrimitivesDirectory) override;
    void setTextRendererCore(QDemonRef<QDemonTextRendererInterface> inRenderer) override
    {
        m_textRenderer = inRenderer;
    }
    QDemonRef<QDemonTextRendererInterface> getTextRendererCore() override { return m_textRenderer; }
    void setOnscreenTextRendererCore(QDemonRef<QDemonTextRendererInterface> inRenderer) override
    {
        m_onscreenTexRenderer = inRenderer;
    }
    QDemonRef<QDemonTextRendererInterface> getOnscreenTextRendererCore() override { return m_onscreenTexRenderer; }
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
void swapXY(QVector2D &v)
{
    const auto tmp = v.x();
    v.setX(v.y());
    v.setY(tmp);
}
}

struct QDemonRenderContextData : public QDemonRenderContextInterface
{
    QDemonRef<QDemonRenderContext> m_renderContext;
    QDemonRenderContextCoreInterface *m_coreContext;
    QDemonPerfTimer m_perfTimer;
    QDemonRef<QDemonInputStreamFactoryInterface> m_inputStreamFactory;
    QDemonBufferManager m_bufferManager;
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    QDemonRef<QDemonOffscreenRenderManagerInterface> m_offscreenRenderManager;
    QDemonRef<QDemonRendererInterface> m_renderer;
    QDemonRef<QDemonTextRendererInterface> m_textRenderer;
    QDemonRef<QDemonTextRendererInterface> m_onscreenTextRenderer;
    QDemonRef<QDemonTextTextureCacheInterface> m_textTextureCache;
    QDemonRef<QDemonTextTextureAtlasInterface> m_textTextureAtlas;
    QDemonRef<QDemonDynamicObjectSystemInterface> m_dynamicObjectSystem;
    QDemonRef<QDemonEffectSystemInterface> m_effectSystem;
    QDemonRef<QDemonShaderCacheInterface> m_shaderCache;
    QDemonRef<QDemonAbstractThreadPool> m_threadPool;
    QDemonRef<IImageBatchLoader> m_imageBatchLoader;
    QDemonMaterialSystem m_customMaterialSystem;
    QDemonRef<QDemonPixelGraphicsRendererInterface> m_pixelGraphicsRenderer;
    QDemonRef<QDemonPathManagerInterface> m_pathManager;
    QDemonRef<QDemonShaderProgramGeneratorInterface> m_shaderProgramGenerator;
    QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> m_defaultMaterialShaderGenerator;
    QDemonRef<ICustomMaterialShaderGenerator> m_customMaterialShaderGenerator;
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
    QDemonRenderFrameBuffer m_rotationFbo;
    QDemonRef<QDemonRenderTexture2D> m_rotationTexture;
    QDemonRenderRenderBuffer m_rotationDepthBuffer;
    QDemonRenderFrameBuffer m_contextRenderTarget;
    QRect m_presentationViewport;
    QSize m_presentationDimensions;
    QSize m_renderPresentationDimensions;
    QSize m_preRenderPresentationDimensions;
    QVector2D m_presentationScale;
    QRect m_virtualViewport;
    QPair<float, int> m_fps;
    bool m_authoringMode;

    QDemonRenderContextData(const QDemonRef<QDemonRenderContext> &ctx, QDemonRenderContextCoreInterface *inCore, const char *inApplicationDirectory)
        : m_renderContext(ctx)
        , m_coreContext(inCore)
        , m_perfTimer(inCore->getPerfTimer())
        , m_inputStreamFactory(inCore->getInputStreamFactory())
        , m_bufferManager(ctx, m_inputStreamFactory, m_perfTimer)
        , m_resourceManager(QDemonResourceManagerInterface::createResourceManager(ctx))
        , m_shaderCache(QDemonShaderCacheInterface::createShaderCache(ctx, m_inputStreamFactory, m_perfTimer))
        , m_threadPool(inCore->getThreadPool())
        , m_frameCount(0)
        , m_windowDimensions(800, 480)
        , m_scaleMode(ScaleModes::ExactSize)
        , m_wireframeMode(false)
        , m_isInSubPresentation(false)
        , m_rotation(RenderRotationValues::NoRotation)
        , m_presentationScale(0, 0)
        , m_fps(qMakePair(0.0, 0))
        , m_authoringMode(false)
    {
        m_renderList = QDemonRenderListInterface::createRenderList();
        m_offscreenRenderManager = QDemonOffscreenRenderManagerInterface::createOffscreenRenderManager(m_resourceManager, this);
        m_renderer = QDemonRendererInterface::createRenderer(this);
        if (inApplicationDirectory && *inApplicationDirectory)
            m_inputStreamFactory->addSearchDirectory(inApplicationDirectory);

        m_imageBatchLoader = IImageBatchLoader::createBatchLoader(m_inputStreamFactory, m_bufferManager, m_threadPool, m_perfTimer);
        m_dynamicObjectSystem = inCore->getDynamicObjectSystemCore();
        m_dynamicObjectSystem->setContextInterface(this);
        m_effectSystem = inCore->getEffectSystemCore()->getEffectSystem(this);
        m_customMaterialSystem = inCore->getMaterialSystemCore();
        m_customMaterialSystem.setRenderContextInterface(this);
        // as does the custom material system
        m_pixelGraphicsRenderer = QDemonPixelGraphicsRendererInterface::createRenderer(this);
        QDemonRef<QDemonTextRendererInterface> theTextCore = inCore->getTextRendererCore();
        m_shaderProgramGenerator = QDemonShaderProgramGeneratorInterface::createProgramGenerator(this);
        m_defaultMaterialShaderGenerator = QDemonDefaultMaterialShaderGeneratorInterface::createDefaultMaterialShaderGenerator(this);
        m_customMaterialShaderGenerator = ICustomMaterialShaderGenerator::createCustomMaterialShaderGenerator(this);
        if (theTextCore) {
            m_textRenderer = theTextCore->getTextRenderer(ctx);
            m_textTextureCache = QDemonTextTextureCacheInterface::createTextureCache(m_textRenderer, m_renderContext);
        }

        QDemonRef<QDemonTextRendererInterface> theOnscreenTextCore = inCore->getOnscreenTextRendererCore();
        if (theOnscreenTextCore) {
            m_onscreenTextRenderer = theOnscreenTextCore->getTextRenderer(ctx);
            m_textTextureAtlas = QDemonTextTextureAtlasInterface::createTextureAtlas(m_onscreenTextRenderer, m_renderContext);
        }
        m_pathManager = inCore->getPathManagerCore()->onRenderSystemInitialize(this);

        const char *versionString;
        switch (ctx->getRenderContextType()) {
        case QDemonRenderContextType::GLES2:
            versionString = "gles2";
            break;
        case QDemonRenderContextType::GL2:
            versionString = "gl2";
            break;
        case QDemonRenderContextType::GLES3:
            versionString = "gles3";
            break;
        case QDemonRenderContextType::GL3:
            versionString = "gl3";
            break;
        case QDemonRenderContextType::GLES3PLUS:
            versionString = "gles3x";
            break;
        case QDemonRenderContextType::GL4:
            versionString = "gl4";
            break;
        default:
            break;
        }

        getDynamicObjectSystem()->setShaderCodeLibraryVersion(versionString);
#if defined(QDEMON_SHADER_PLATFORM_LIBRARY_DIR)
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

    QDemonRef<QDemonRendererInterface> getRenderer() override { return m_renderer; }
    QDemonBufferManager getBufferManager() override { return m_bufferManager; }
    QDemonRef<QDemonResourceManagerInterface> getResourceManager() override { return m_resourceManager; }
    QDemonRef<QDemonRenderContext> getRenderContext() override { return m_renderContext; }
    QDemonRef<QDemonOffscreenRenderManagerInterface> getOffscreenRenderManager() override
    {
        return m_offscreenRenderManager;
    }
    QDemonRef<QDemonInputStreamFactoryInterface> getInputStreamFactory() override { return m_inputStreamFactory; }
    QDemonRef<QDemonEffectSystemInterface> getEffectSystem() override { return m_effectSystem; }
    QDemonRef<QDemonShaderCacheInterface> getShaderCache() override { return m_shaderCache; }
    QDemonRef<QDemonAbstractThreadPool> getThreadPool() override { return m_threadPool; }
    QDemonRef<IImageBatchLoader> getImageBatchLoader() override { return m_imageBatchLoader; }
    QDemonRef<QDemonTextTextureCacheInterface> getTextureCache() override { return m_textTextureCache; }
    QDemonRef<QDemonTextTextureAtlasInterface> getTextureAtlas() override { return m_textTextureAtlas; }
    QDemonRef<QDemonDynamicObjectSystemInterface> getDynamicObjectSystem() override { return m_dynamicObjectSystem; }
    QDemonMaterialSystem getCustomMaterialSystem() override { return m_customMaterialSystem; }
    QDemonRef<QDemonPixelGraphicsRendererInterface> getPixelGraphicsRenderer() override
    {
        return m_pixelGraphicsRenderer;
    }
    QDemonPerfTimer getPerfTimer() override { return m_perfTimer; }
    QDemonRef<QDemonRenderListInterface> getRenderList() override { return m_renderList; }
    QDemonRef<QDemonPathManagerInterface> getPathManager() override { return m_pathManager; }
    QDemonRef<QDemonShaderProgramGeneratorInterface> getShaderProgramGenerator() override
    {
        return m_shaderProgramGenerator;
    }
    QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> getDefaultMaterialShaderGenerator() override
    {
        return m_defaultMaterialShaderGenerator;
    }
    QDemonRef<ICustomMaterialShaderGenerator> getCustomMaterialShaderGenerator() override
    {
        return m_customMaterialShaderGenerator;
    }

    QDemonPerFrameAllocator &getPerFrameAllocator() override { return m_perFrameAllocator; }

    quint32 getFrameCount() override { return m_frameCount; }
    void setFPS(QPair<float, int> inFPS) override { m_fps = inFPS; }
    QPair<float, int> getFPS(void) override { return m_fps; }

    bool isAuthoringMode() override { return m_authoringMode; }
    void setAuthoringMode(bool inMode) override { m_authoringMode = inMode; }

    bool isInSubPresentation() override { return m_isInSubPresentation; }
    void setInSubPresentation(bool inValue) override { m_isInSubPresentation = inValue; }

    QDemonRef<QDemonTextRendererInterface> getTextRenderer() override { return m_textRenderer; }

    QDemonRef<QDemonTextRendererInterface> getOnscreenTextRenderer() override { return m_onscreenTextRenderer; }

    void setSceneColor(QDemonOption<QVector4D> inSceneColor) override { m_sceneColor = inSceneColor; }
    void setMatteColor(QDemonOption<QVector4D> inMatteColor) override { m_matteColor = inMatteColor; }

    void setWindowDimensions(const QSize &inWindowDimensions) override { m_windowDimensions = inWindowDimensions; }

    QSize getWindowDimensions() override { return m_windowDimensions; }

    void setScaleMode(ScaleModes inMode) override { m_scaleMode = inMode; }

    ScaleModes getScaleMode() override { return m_scaleMode; }

    void setWireframeMode(bool inEnable) override { m_wireframeMode = inEnable; }

    bool getWireframeMode() override { return m_wireframeMode; }

    void setViewport(QDemonOption<QRect> inViewport) override { m_viewport = inViewport; }
    QDemonOption<QRect> getViewport() const override { return m_viewport; }

    QDemonRef<QDemonRendererImpl> getRenderWidgetContext() override
    {
        return static_cast<QDemonRendererImpl *>(m_renderer.get());
    }

    QPair<QRect, QRect> getPresentationViewportAndOuterViewport() const
    {
        QSize thePresentationDimensions(m_presentationDimensions);
        QRect theOuterViewport(getContextViewport());
        if (m_rotation == RenderRotationValues::Clockwise90 || m_rotation == RenderRotationValues::Clockwise270) {
            theOuterViewport = { theOuterViewport.y(), theOuterViewport.x(), theOuterViewport.height(), theOuterViewport.width() };
        }
        // Calculate the presentation viewport perhaps with the window width and height swapped.
        return QPair<QRect, QRect>(getPresentationViewport(theOuterViewport, m_scaleMode, thePresentationDimensions), theOuterViewport);
    }

    QRectF getDisplayViewport() const override { return getPresentationViewportAndOuterViewport().first; }

    void setPresentationDimensions(const QSize &inPresentationDimensions) override
    {
        m_presentationDimensions = inPresentationDimensions;
    }
    QSize getCurrentPresentationDimensions() const override { return m_presentationDimensions; }

    void setRenderRotation(RenderRotationValues inRotation) override { m_rotation = inRotation; }

    RenderRotationValues getRenderRotation() const override { return m_rotation; }
    QVector2D getMousePickViewport() const override
    {
        bool renderOffscreen = m_rotation != RenderRotationValues::NoRotation;
        if (renderOffscreen)
            return QVector2D((float)m_presentationViewport.width(), (float)m_presentationViewport.height());
        else
            return QVector2D((float)m_windowDimensions.width(), (float)m_windowDimensions.height());
    }
    QRect getContextViewport() const override
    {
        QRect retval;
        if (m_viewport.hasValue())
            retval = *m_viewport;
        else
            retval = QRect(0, 0, m_windowDimensions.width(), m_windowDimensions.height());

        return retval;
    }

    QVector2D getMousePickMouseCoords(const QVector2D &inMouseCoords) const override
    {
        bool renderOffscreen = m_rotation != RenderRotationValues::NoRotation;
        if (renderOffscreen) {
            QSize thePresentationDimensions(m_renderPresentationDimensions);
            QRect theViewport(getContextViewport());
            // Calculate the presentation viewport perhaps with the presentation width and height
            // swapped.
            QRect thePresentationViewport = getPresentationViewport(theViewport, m_scaleMode, thePresentationDimensions);
            // Translate pick into presentation space without rotations or anything else.
            float YHeightDiff = (float)((float)m_windowDimensions.height() - (float)thePresentationViewport.height());
            QVector2D theLocalMouse((inMouseCoords.x() - thePresentationViewport.x()),
                                    (inMouseCoords.y() - YHeightDiff + thePresentationViewport.y()));
            switch (m_rotation) {
            default:
            case RenderRotationValues::NoRotation:
                Q_ASSERT(false);
                break;
            case RenderRotationValues::Clockwise90:
                swapXY(theLocalMouse);
                theLocalMouse.setY(thePresentationViewport.width() - theLocalMouse.y());
                break;
            case RenderRotationValues::Clockwise180:
                theLocalMouse.setY(thePresentationViewport.height() - theLocalMouse.y());
                theLocalMouse.setX(thePresentationViewport.width() - theLocalMouse.x());
                break;
            case RenderRotationValues::Clockwise270:
                swapXY(theLocalMouse);
                theLocalMouse.setX(thePresentationViewport.height() - theLocalMouse.x());
                break;
            }
            return theLocalMouse;
        }
        return inMouseCoords;
    }

    QRect getPresentationViewport(const QRect &inViewerViewport, ScaleModes inScaleToFit, const QSize &inPresDimensions) const
    {
        const qint32 viewerViewportWidth = inViewerViewport.width();
        const qint32 viewerViewportHeight = inViewerViewport.height();
        qint32 width, height, x, y;
        if (inPresDimensions.width() == 0 || inPresDimensions.height() == 0)
            return QRect(0, 0, 0, 0);
        // Setup presentation viewport.  This may or may not match the physical viewport that we
        // want to setup.
        // Avoiding scaling keeps things as sharp as possible.
        if (inScaleToFit == ScaleModes::ExactSize) {
            width = inPresDimensions.width();
            height = inPresDimensions.height();
            x = (viewerViewportWidth - (qint32)inPresDimensions.width()) / 2;
            y = (viewerViewportHeight - (qint32)inPresDimensions.height()) / 2;
        } else if (inScaleToFit == ScaleModes::ScaleToFit || inScaleToFit == ScaleModes::FitSelected) {
            // Scale down in such a way to preserve aspect ratio.
            float screenAspect = (float)viewerViewportWidth / (float)viewerViewportHeight;
            float thePresentationAspect = (float)inPresDimensions.width() / (float)inPresDimensions.height();
            if (screenAspect >= thePresentationAspect) {
                // if the screen height is the limiting factor
                y = 0;
                height = viewerViewportHeight;
                width = (qint32)(thePresentationAspect * height);
                x = (viewerViewportWidth - width) / 2;
            } else {
                x = 0;
                width = viewerViewportWidth;
                height = (qint32)(width / thePresentationAspect);
                y = (viewerViewportHeight - height) / 2;
            }
        } else {
            // Setup the viewport for everything and let the presentations figure it out.
            x = 0;
            y = 0;
            width = viewerViewportWidth;
            height = viewerViewportHeight;
        }
        x += inViewerViewport.x();
        y += inViewerViewport.y();
        return { x, y, width, height };
    }

    void renderGpuProfilerStats(float x, float y, QDemonOption<QVector3D> inColor) override
    {
        m_renderer->renderGpuProfilerStats(x, y, inColor);
    }

    QRect getPresentationViewport() const override { return m_presentationViewport; }
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

    void beginFrame() override
    {
        m_preRenderPresentationDimensions = m_presentationDimensions;
        QSize thePresentationDimensions(m_preRenderPresentationDimensions);
        QRect theContextViewport(getContextViewport());
        m_perFrameAllocator.reset();
        QDemonRenderListInterface &theRenderList(*m_renderList);
        theRenderList.beginFrame();
        if (m_viewport.hasValue()) {
            theRenderList.setScissorTestEnabled(true);
            theRenderList.setScissorRect(theContextViewport);
        } else {
            theRenderList.setScissorTestEnabled(false);
        }
        bool renderOffscreen = m_rotation != RenderRotationValues::NoRotation;
        QPair<QRect, QRect> thePresViewportAndOuterViewport = getPresentationViewportAndOuterViewport();
        QRect theOuterViewport = thePresViewportAndOuterViewport.second;
        // Calculate the presentation viewport perhaps with the window width and height swapped.
        QRect thePresentationViewport = thePresViewportAndOuterViewport.first;
        m_presentationViewport = thePresentationViewport;
        m_presentationScale = QVector2D((float)thePresentationViewport.width() / (float)thePresentationDimensions.width(),
                                        (float)thePresentationViewport.height() / (float)thePresentationDimensions.height());
        QSize fboDimensions;
        if (thePresentationViewport.width() > 0 && thePresentationViewport.height() > 0) {
            if (renderOffscreen == false) {
                m_presentationDimensions = QSize(thePresentationViewport.width(), thePresentationViewport.height());
                m_renderList->setViewport(thePresentationViewport);
                if (thePresentationViewport.x() || thePresentationViewport.y()
                    || thePresentationViewport.width() != (qint32)theOuterViewport.width()
                    || thePresentationViewport.height() != (qint32)theOuterViewport.height()) {
                    m_renderList->setScissorRect(thePresentationViewport);
                    m_renderList->setScissorTestEnabled(true);
                }
            } else {
                quint32 imageWidth = QDemonTextRendererInterface::nextMultipleOf4(thePresentationViewport.width());
                quint32 imageHeight = QDemonTextRendererInterface::nextMultipleOf4(thePresentationViewport.height());
                fboDimensions = QSize(imageWidth, imageHeight);
                m_presentationDimensions = QSize(thePresentationViewport.width(), thePresentationViewport.height());
                QRect theSceneViewport = QRect(0, 0, imageWidth, imageHeight);
                m_renderList->setScissorTestEnabled(false);
                m_renderList->setViewport(theSceneViewport);
            }
        }

        m_beginFrameResult = BeginFrameResult(renderOffscreen,
                                              m_presentationDimensions,
                                              m_renderList->isScissorTestEnabled(),
                                              m_renderList->getScissor(),
                                              m_renderList->getViewport(),
                                              fboDimensions);

        m_renderer->beginFrame();
        m_offscreenRenderManager->beginFrame();
        if (m_textRenderer)
            m_textRenderer->beginFrame();
        if (m_textTextureCache)
            m_textTextureCache->beginFrame();
        m_imageBatchLoader->beginFrame();
    }

    QVector2D getPresentationScaleFactor() const override { return m_presentationScale; }

    virtual void setupRenderTarget()
    {
        QRect theContextViewport(getContextViewport());
        if (m_viewport.hasValue()) {
            m_renderContext->setScissorTestEnabled(true);
            m_renderContext->setScissorRect(theContextViewport);
        } else {
            m_renderContext->setScissorTestEnabled(false);
        }
        {
            QVector4D theClearColor;
            if (m_matteColor.hasValue())
                theClearColor = m_matteColor;
            else
                theClearColor = m_sceneColor;
            Q_ASSERT(m_sceneColor.hasValue());
            m_renderContext->setClearColor(theClearColor);
            m_renderContext->clear(QDemonRenderClearValues::Color);
        }
        bool renderOffscreen = m_beginFrameResult.renderOffscreen;
        m_renderContext->setViewport(m_beginFrameResult.viewport);
        m_renderContext->setScissorRect(m_beginFrameResult.scissorRect);
        m_renderContext->setScissorTestEnabled(m_beginFrameResult.scissorTestEnabled);

        if (m_presentationViewport.width() > 0 && m_presentationViewport.height() > 0) {
            if (renderOffscreen == false) {
                if (!m_rotationFbo.isNull()) {
                    m_resourceManager->release(m_rotationFbo);
                    m_resourceManager->release(m_rotationTexture);
                    m_resourceManager->release(m_rotationDepthBuffer);
                    m_rotationFbo.clear();
                    m_rotationTexture = nullptr;
                    m_rotationDepthBuffer.clear();
                }
                if (m_sceneColor.hasValue() && m_sceneColor.getValue().w() != 0.0f) {
                    m_renderContext->setClearColor(m_sceneColor);
                    m_renderContext->clear(QDemonRenderClearValues::Color);
                }
            } else {
                qint32 imageWidth = m_beginFrameResult.fboDimensions.width();
                qint32 imageHeight = m_beginFrameResult.fboDimensions.height();
                QDemonRenderTextureFormat theColorBufferFormat = QDemonRenderTextureFormat::RGBA8;
                QDemonRenderRenderBufferFormat theDepthBufferFormat = QDemonRenderRenderBufferFormat::Depth16;
                m_contextRenderTarget = m_renderContext->getRenderTarget();
                if (!m_rotationFbo) {
                    m_rotationFbo = m_resourceManager->allocateFrameBuffer();
                    m_rotationTexture = m_resourceManager->allocateTexture2D(imageWidth, imageHeight, theColorBufferFormat);
                    m_rotationDepthBuffer = m_resourceManager->allocateRenderBuffer(imageWidth, imageHeight, theDepthBufferFormat);
                    m_rotationFbo.attach(QDemonRenderFrameBufferAttachment::Color0, m_rotationTexture);
                    m_rotationFbo.attach(QDemonRenderFrameBufferAttachment::Depth, m_rotationDepthBuffer);
                } else {
                    QDemonTextureDetails theDetails = m_rotationTexture->getTextureDetails();
                    if (theDetails.width != imageWidth || theDetails.height != imageHeight) {
                        m_rotationTexture->setTextureData(QDemonDataRef<quint8>(), 0, imageWidth, imageHeight, theColorBufferFormat);
                        m_rotationDepthBuffer.setSize(QSize(imageWidth, imageHeight));
                    }
                }
                m_renderContext->setRenderTarget(m_rotationFbo);
                if (m_sceneColor.hasValue()) {
                    m_renderContext->setClearColor(m_sceneColor);
                    m_renderContext->clear(QDemonRenderClearValues::Color);
                }
            }
        }
    }

    void runRenderTasks() override
    {
        m_renderList->runRenderTasks();
        setupRenderTarget();
    }

    // Note this runs before EndFrame
    virtual void teardownRenderTarget()
    {
        if (!m_rotationFbo.isNull()) {
            ScaleModes theScaleToFit = m_scaleMode;
            QRect theOuterViewport(getContextViewport());
            m_renderContext->setRenderTarget(m_contextRenderTarget);
            QSize thePresentationDimensions = getCurrentPresentationDimensions();
            if (m_rotation == RenderRotationValues::Clockwise90 || m_rotation == RenderRotationValues::Clockwise270) {
                thePresentationDimensions = QSize(thePresentationDimensions.height(), thePresentationDimensions.width());
            }
            m_renderPresentationDimensions = thePresentationDimensions;
            // Calculate the presentation viewport perhaps with the presentation width and height
            // swapped.
            QRect thePresentationViewport = getPresentationViewport(theOuterViewport, theScaleToFit, thePresentationDimensions);
            QDemonRenderCamera theCamera;
            switch (m_rotation) {
            default:
                Q_ASSERT(false);
                break;
            case RenderRotationValues::Clockwise90:
                theCamera.rotation.setZ(90);
                break;
            case RenderRotationValues::Clockwise180:
                theCamera.rotation.setZ(180);
                break;
            case RenderRotationValues::Clockwise270:
                theCamera.rotation.setZ(270);
                break;
            }
            float z = theCamera.rotation.z();
            TORAD(z);
            theCamera.rotation.setZ(z);
            theCamera.markDirty(NodeTransformDirtyFlag::TransformIsDirty);
            theCamera.flags.setOrthographic(true);
            m_renderContext->setViewport(thePresentationViewport);
            QVector2D theCameraDimensions((float)thePresentationViewport.width(), (float)thePresentationViewport.height());
            theCamera.calculateGlobalVariables(QRect(0,
                                                     0,
                                                     (quint32)thePresentationViewport.width(),
                                                     (quint32)thePresentationViewport.height()),
                                               theCameraDimensions);
            QMatrix4x4 theVP;
            theCamera.calculateViewProjectionMatrix(theVP);
            QDemonGraphNode theTempNode;
            theTempNode.calculateGlobalVariables();
            QMatrix4x4 theMVP;
            QMatrix3x3 theNormalMat;
            theTempNode.calculateMVPAndNormalMatrix(theVP, theMVP, theNormalMat);
            m_renderContext->setCullingEnabled(false);
            m_renderContext->setBlendingEnabled(false);
            m_renderContext->setDepthTestEnabled(false);
            m_renderer->renderQuad(QVector2D((float)m_presentationViewport.width(), (float)m_presentationViewport.height()),
                                   theMVP,
                                   *m_rotationTexture);
        }
    }

    void endFrame() override
    {
        teardownRenderTarget();
        m_imageBatchLoader->endFrame();
        if (m_textTextureCache)
            m_textTextureCache->endFrame();
        if (m_textRenderer)
            m_textRenderer->endFrame();
        m_offscreenRenderManager->endFrame();
        m_renderer->endFrame();
        m_customMaterialSystem.endFrame();
        m_presentationDimensions = m_preRenderPresentationDimensions;
        ++m_frameCount;
    }
};

QDemonRef<QDemonRenderContextInterface> QDemonRenderContextCore::createRenderContext(QDemonRef<QDemonRenderContext> inContext,
                                                                                     const char *inPrimitivesDirectory)
{
    return QDemonRef<QDemonRenderContextData>(new QDemonRenderContextData(inContext, this, inPrimitivesDirectory));
}
}

QDemonRenderContextCoreInterface::~QDemonRenderContextCoreInterface() = default;

QDemonRef<QDemonRenderContextCoreInterface> QDemonRenderContextCoreInterface::create()
{
    return QDemonRef<QDemonRenderContextCore>(new QDemonRenderContextCore());
}

QDemonRenderContextInterface::~QDemonRenderContextInterface() = default;

QT_END_NAMESPACE
