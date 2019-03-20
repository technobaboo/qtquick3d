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
#include <QtDemonRuntimeRender/qdemonrenderinputstreamfactory.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRender/qdemonrendertexture2d.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderthreadpool.h>
#include <QtDemonRuntimeRender/qdemonrenderimagebatchloader.h>
#include <QtDemonRuntimeRender/qdemonrenderdynamicobjectsystem.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderpixelgraphicsrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonperframeallocator.h>
#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemonRuntimeRender/qdemonrendererutil.h>

QT_BEGIN_NAMESPACE

namespace {
void swapXY(QVector2D &v)
{
    const auto tmp = v.x();
    v.setX(v.y());
    v.setY(tmp);
}
}

QDemonRenderContextInterface::~QDemonRenderContextInterface() = default;

QDemonRenderContextInterface::QDemonRenderContextInterface(const QDemonRef<QDemonRenderContext> &ctx, const QString &inApplicationDirectory)
    : m_renderContext(ctx)
    , m_inputStreamFactory(new QDemonInputStreamFactory)
    , m_bufferManager(new QDemonBufferManager(ctx, m_inputStreamFactory, &m_perfTimer))
    , m_resourceManager(new QDemonResourceManager(ctx))
    , m_shaderCache(QDemonShaderCache::createShaderCache(ctx, m_inputStreamFactory, &m_perfTimer))
    , m_threadPool(QDemonAbstractThreadPool::createThreadPool(4))
    , m_windowDimensions(800, 480)
    , m_presentationScale(0, 0)
    , m_fps(qMakePair(0.0, 0))
{
    m_renderList = QDemonRenderList::createRenderList();
    m_offscreenRenderManager = QDemonOffscreenRenderManager::createOffscreenRenderManager(m_resourceManager, this);
    m_renderer = QDemonRendererInterface::createRenderer(this);
    if (!inApplicationDirectory.isEmpty())
        m_inputStreamFactory->addSearchDirectory(inApplicationDirectory);

    m_imageBatchLoader = IImageBatchLoader::createBatchLoader(m_inputStreamFactory, m_bufferManager, m_threadPool, &m_perfTimer);
    m_dynamicObjectSystem = QDemonDynamicObjectSystemInterface::createDynamicSystem(this);
    m_effectSystem = QDemonEffectSystemInterface::createEffectSystem(this);
    m_customMaterialSystem = new QDemonMaterialSystem(this);
    m_customMaterialSystem->setRenderContextInterface(this);
    // as does the custom material system
    m_pixelGraphicsRenderer = QDemonPixelGraphicsRendererInterface::createRenderer(this);
    m_shaderProgramGenerator = QDemonShaderProgramGeneratorInterface::createProgramGenerator(this);
    m_defaultMaterialShaderGenerator = QDemonDefaultMaterialShaderGeneratorInterface::createDefaultMaterialShaderGenerator(this);
    m_customMaterialShaderGenerator = QDemonMaterialShaderGeneratorInterface::createCustomMaterialShaderGenerator(this);

    m_pathManager = QDemonPathManagerInterface::createPathManager(this);

    const char *versionString;
    switch (ctx->renderContextType()) {
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

    dynamicObjectSystem()->setShaderCodeLibraryVersion(versionString);
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

QDemonRef<QDemonRendererInterface> QDemonRenderContextInterface::renderer() { return m_renderer; }

QDemonRef<QDemonBufferManager> QDemonRenderContextInterface::bufferManager() { return m_bufferManager; }

QDemonRef<QDemonResourceManager> QDemonRenderContextInterface::resourceManager() { return m_resourceManager; }

QDemonRef<QDemonRenderContext> QDemonRenderContextInterface::renderContext() { return m_renderContext; }

QDemonRef<QDemonOffscreenRenderManager> QDemonRenderContextInterface::offscreenRenderManager()
{
    return m_offscreenRenderManager;
}

QDemonRef<QDemonInputStreamFactory> QDemonRenderContextInterface::inputStreamFactory() { return m_inputStreamFactory; }

QDemonRef<QDemonEffectSystemInterface> QDemonRenderContextInterface::effectSystem() { return m_effectSystem; }

QDemonRef<QDemonShaderCache> QDemonRenderContextInterface::shaderCache() { return m_shaderCache; }

QDemonRef<QDemonAbstractThreadPool> QDemonRenderContextInterface::threadPool() { return m_threadPool; }

QDemonRef<IImageBatchLoader> QDemonRenderContextInterface::imageBatchLoader() { return m_imageBatchLoader; }

QDemonRef<QDemonDynamicObjectSystemInterface> QDemonRenderContextInterface::dynamicObjectSystem() { return m_dynamicObjectSystem; }

QDemonRef<QDemonMaterialSystem> QDemonRenderContextInterface::customMaterialSystem() { return m_customMaterialSystem; }

QDemonRef<QDemonPixelGraphicsRendererInterface> QDemonRenderContextInterface::pixelGraphicsRenderer()
{
    return m_pixelGraphicsRenderer;
}

QDemonRef<QDemonRenderList> QDemonRenderContextInterface::renderList() { return m_renderList; }

QDemonRef<QDemonPathManagerInterface> QDemonRenderContextInterface::pathManager() { return m_pathManager; }

QDemonRef<QDemonShaderProgramGeneratorInterface> QDemonRenderContextInterface::shaderProgramGenerator()
{
    return m_shaderProgramGenerator;
}

QDemonRef<QDemonDefaultMaterialShaderGeneratorInterface> QDemonRenderContextInterface::defaultMaterialShaderGenerator()
{
    return m_defaultMaterialShaderGenerator;
}

QDemonRef<QDemonMaterialShaderGeneratorInterface> QDemonRenderContextInterface::customMaterialShaderGenerator()
{
    return m_customMaterialShaderGenerator;
}

QDemonRef<QDemonRendererImpl> QDemonRenderContextInterface::renderWidgetContext()
{
    return static_cast<QDemonRendererImpl *>(m_renderer.get());
}

QPair<QRect, QRect> QDemonRenderContextInterface::getPresentationViewportAndOuterViewport() const
{
    QSize thePresentationDimensions(m_presentationDimensions);
    QRect theOuterViewport(contextViewport());
    if (m_rotation == RenderRotationValues::Clockwise90 || m_rotation == RenderRotationValues::Clockwise270) {
        theOuterViewport = { theOuterViewport.y(), theOuterViewport.x(), theOuterViewport.height(), theOuterViewport.width() };
    }
    // Calculate the presentation viewport perhaps with the window width and height swapped.
    return QPair<QRect, QRect>(presentationViewport(theOuterViewport, m_scaleMode, thePresentationDimensions), theOuterViewport);
}

QVector2D QDemonRenderContextInterface::mousePickViewport() const
{
    bool renderOffscreen = m_rotation != RenderRotationValues::NoRotation;
    if (renderOffscreen)
        return QVector2D((float)m_presentationViewport.width(), (float)m_presentationViewport.height());
    else
        return QVector2D((float)m_windowDimensions.width(), (float)m_windowDimensions.height());
}

QRect QDemonRenderContextInterface::contextViewport() const
{
    QRect retval;
    if (m_viewport.hasValue())
        retval = *m_viewport;
    else
        retval = QRect(0, 0, m_windowDimensions.width(), m_windowDimensions.height());

    return retval;
}

QVector2D QDemonRenderContextInterface::mousePickMouseCoords(const QVector2D &inMouseCoords) const
{
    bool renderOffscreen = m_rotation != RenderRotationValues::NoRotation;
    if (renderOffscreen) {
        QSize thePresentationDimensions(m_renderPresentationDimensions);
        QRect theViewport(contextViewport());
        // Calculate the presentation viewport perhaps with the presentation width and height
        // swapped.
        QRect thePresentationViewport = presentationViewport(theViewport, m_scaleMode, thePresentationDimensions);
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

QRect QDemonRenderContextInterface::presentationViewport(const QRect &inViewerViewport, ScaleModes inScaleToFit, const QSize &inPresDimensions) const
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

void QDemonRenderContextInterface::dumpGpuProfilerStats()
{
    m_renderer->dumpGpuProfilerStats();
}

void QDemonRenderContextInterface::beginFrame()
{
    m_preRenderPresentationDimensions = m_presentationDimensions;
    QSize thePresentationDimensions(m_preRenderPresentationDimensions);
    QRect theContextViewport(contextViewport());
    m_perFrameAllocator.reset();
    QDemonRenderList &theRenderList(*m_renderList);
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
            quint32 imageWidth = QDemonRendererUtil::nextMultipleOf4(thePresentationViewport.width());
            quint32 imageHeight = QDemonRendererUtil::nextMultipleOf4(thePresentationViewport.height());
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
    m_imageBatchLoader->beginFrame();
}

void QDemonRenderContextInterface::setupRenderTarget()
{
    QRect theContextViewport(contextViewport());
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
            if (m_rotationFbo != nullptr) {
                m_resourceManager->release(m_rotationFbo);
                m_resourceManager->release(m_rotationTexture);
                m_resourceManager->release(m_rotationDepthBuffer);
                m_rotationFbo = nullptr;
                m_rotationTexture = nullptr;
                m_rotationDepthBuffer = nullptr;
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
            m_contextRenderTarget = m_renderContext->renderTarget();
            if (m_rotationFbo == nullptr) {
                m_rotationFbo = m_resourceManager->allocateFrameBuffer();
                m_rotationTexture = m_resourceManager->allocateTexture2D(imageWidth, imageHeight, theColorBufferFormat);
                m_rotationDepthBuffer = m_resourceManager->allocateRenderBuffer(imageWidth, imageHeight, theDepthBufferFormat);
                m_rotationFbo->attach(QDemonRenderFrameBufferAttachment::Color0, m_rotationTexture);
                m_rotationFbo->attach(QDemonRenderFrameBufferAttachment::Depth, m_rotationDepthBuffer);
            } else {
                QDemonTextureDetails theDetails = m_rotationTexture->textureDetails();
                if (theDetails.width != imageWidth || theDetails.height != imageHeight) {
                    m_rotationTexture->setTextureData(QDemonByteView(), 0, imageWidth, imageHeight, theColorBufferFormat);
                    m_rotationDepthBuffer->setSize(QSize(imageWidth, imageHeight));
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

void QDemonRenderContextInterface::runRenderTasks()
{
    m_renderList->runRenderTasks();
    setupRenderTarget();
}

void QDemonRenderContextInterface::teardownRenderTarget()
{
    if (m_rotationFbo) {
        ScaleModes theScaleToFit = m_scaleMode;
        QRect theOuterViewport(contextViewport());
        m_renderContext->setRenderTarget(m_contextRenderTarget);
        QSize thePresentationDimensions = currentPresentationDimensions();
        if (m_rotation == RenderRotationValues::Clockwise90 || m_rotation == RenderRotationValues::Clockwise270) {
            thePresentationDimensions = QSize(thePresentationDimensions.height(), thePresentationDimensions.width());
        }
        m_renderPresentationDimensions = thePresentationDimensions;
        // Calculate the presentation viewport perhaps with the presentation width and height
        // swapped.
        QRect thePresentationViewport = presentationViewport(theOuterViewport, theScaleToFit, thePresentationDimensions);
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
        theCamera.markDirty(QDemonRenderCamera::TransformDirtyFlag::TransformIsDirty);
        theCamera.flags.setFlag(QDemonRenderCamera::Flag::Orthographic);
        m_renderContext->setViewport(thePresentationViewport);
        QVector2D theCameraDimensions((float)thePresentationViewport.width(), (float)thePresentationViewport.height());
        theCamera.calculateGlobalVariables(QRect(0,
                                                 0,
                                                 (quint32)thePresentationViewport.width(),
                                                 (quint32)thePresentationViewport.height()),
                                           theCameraDimensions);
        QMatrix4x4 theVP;
        theCamera.calculateViewProjectionMatrix(theVP);
        QDemonRenderNode theTempNode;
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

void QDemonRenderContextInterface::endFrame()
{
    teardownRenderTarget();
    m_imageBatchLoader->endFrame();
    m_offscreenRenderManager->endFrame();
    m_renderer->endFrame();
    m_customMaterialSystem->endFrame();
    m_presentationDimensions = m_preRenderPresentationDimensions;
    ++m_frameCount;
}

QT_END_NAMESPACE

