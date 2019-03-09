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
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRender/qdemonrenderbasetypes.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <qdemonoffscreenrenderkey.h>
#include <qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcetexture2d.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <qdemonrendererutil.h>
#include <QtDemonRender/qdemonrendertexture2d.h>

#include <limits>

QT_BEGIN_NAMESPACE

uint qHash(const QDemonOffscreenRendererKey &key)
{
    if (key.isString())
        return qHash(key.string);
    else
        return qHash(reinterpret_cast<size_t>(key.key));
}

namespace {

struct QDemonRendererData : QDemonOffscreenRenderResult
{
    QDemonRef<QDemonResourceManagerInterface> resourceManager;
    quint32 frameCount = std::numeric_limits<quint32>::max();
    bool rendering = false;

    QDemonRendererData(const QDemonRef<QDemonResourceManagerInterface> &inResourceManager)
        : resourceManager(inResourceManager)
    {
    }
};

struct QDemonScopedRenderDataRenderMarker
{
    QDemonRendererData &data;
    QDemonScopedRenderDataRenderMarker(QDemonRendererData &d) : data(d)
    {
        Q_ASSERT(data.rendering == false);
        data.rendering = true;
    }
    ~QDemonScopedRenderDataRenderMarker() { data.rendering = false; }
};

struct QDemonRenderDataReleaser
{
    // TODO:
    QDemonRef<QDemonRendererData> dataPtr;
};
struct QDemonOffscreenRenderManager;

struct QDemonOffscreenRunnable : public QDemonRenderTask
{
    QDemonOffscreenRenderManager &m_renderManager;
    QDemonRendererData &m_data;
    QDemonOffscreenRendererEnvironment m_desiredEnvironment;
    QDemonOffscreenRunnable(QDemonOffscreenRenderManager &rm, QDemonRendererData &data, const QDemonOffscreenRendererEnvironment &env)
        : m_renderManager(rm), m_data(data), m_desiredEnvironment(env)
    {
    }
    void run() override;
};

struct QDemonOffscreenRenderManager : public QDemonOffscreenRenderManagerInterface
{
    typedef QHash<QDemonOffscreenRendererKey, QDemonRendererData> TRendererMap;
    QDemonRenderContextInterface *m_context;
    QDemonRef<QDemonResourceManagerInterface> m_resourceManager;
    TRendererMap m_renderers;
    quint32 m_frameCount; // cheap per-

    QDemonOffscreenRenderManager(const QDemonRef<QDemonResourceManagerInterface> &inManager, QDemonRenderContextInterface *inContext)
        : m_context(inContext), m_resourceManager(inManager), m_frameCount(0)
    {
    }

    ~QDemonOffscreenRenderManager() override = default;

    QDemonOption<bool> maybeRegisterOffscreenRenderer(const QDemonOffscreenRendererKey &inKey,
                                                      QDemonRef<QDemonOffscreenRendererInterface> inRenderer) override
    {
        TRendererMap::iterator theIter = m_renderers.find(inKey);
        if (theIter != m_renderers.end()) {
            QDemonRendererData &theData = theIter.value();
            if (theData.renderer != inRenderer) {
                if (inKey.isString()) {
                    qCCritical(INVALID_OPERATION,
                               "Different renderers registered under same key: %s",
                               inKey.string.toLatin1().constData());
                }
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            return false;
        }
        registerOffscreenRenderer(inKey, inRenderer);
        return true;
    }

    void registerOffscreenRenderer(const QDemonOffscreenRendererKey &inKey, QDemonRef<QDemonOffscreenRendererInterface> inRenderer) override
    {
        auto inserter = m_renderers.find(inKey);
        if (inserter == m_renderers.end())
            inserter = m_renderers.insert(inKey, QDemonRendererData(m_resourceManager));
        QDemonRendererData &theData = inserter.value();
        theData.renderer = inRenderer;
    }

    bool hasOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) override
    {
        return m_renderers.find(inKey) != m_renderers.end();
    }

    QDemonRef<QDemonOffscreenRendererInterface> getOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) override
    {
        TRendererMap::iterator theRenderer = m_renderers.find(inKey);
        if (theRenderer != m_renderers.end()) {
            QDemonRendererData &theData = theRenderer.value();
            return theData.renderer;
        }
        return nullptr;
    }
    void releaseOffscreenRenderer(const QDemonOffscreenRendererKey &inKey) override { m_renderers.remove(inKey); }

    void renderItem(QDemonRendererData &theData, QDemonOffscreenRendererEnvironment theDesiredEnvironment)
    {
        auto theContext = m_resourceManager->getRenderContext();
        QVector2D thePresScaleFactor = m_context->getPresentationScaleFactor();
        QDemonOffscreenRendererEnvironment theOriginalDesiredEnvironment(theDesiredEnvironment);
        // Ensure that our overall render context comes back no matter what the client does.
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(*theContext,
                                                                  &QDemonRenderContext::clearColor,
                                                                  &QDemonRenderContext::setClearColor,
                                                                  QVector4D(0, 0, 0, 0));
        QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theContext,
                                                                 &QDemonRenderContext::isScissorTestEnabled,
                                                                 &QDemonRenderContext::setScissorTestEnabled,
                                                                 false);
        QDemonRenderContextScopedProperty<QRect> __scissorRect(*theContext,
                                                               &QDemonRenderContext::scissorRect,
                                                               &QDemonRenderContext::setScissorRect);
        QDemonRenderContextScopedProperty<QRect> __viewportRect(*theContext,
                                                                &QDemonRenderContext::viewport,
                                                                &QDemonRenderContext::setViewport);
        QDemonRenderContextScopedProperty<bool> __depthWrite(*theContext,
                                                             &QDemonRenderContext::isDepthWriteEnabled,
                                                             &QDemonRenderContext::setDepthWriteEnabled,
                                                             false);
        QDemonRenderContextScopedProperty<QDemonRenderBoolOp> __depthFunction(*theContext,
                                                                                    &QDemonRenderContext::depthFunction,
                                                                                    &QDemonRenderContext::setDepthFunction,
                                                                                    QDemonRenderBoolOp::Less);
        QDemonRenderContextScopedProperty<bool> __blendEnabled(*theContext,
                                                               &QDemonRenderContext::isBlendingEnabled,
                                                               &QDemonRenderContext::setBlendingEnabled,
                                                               false);
        QDemonRenderContextScopedProperty<QDemonRenderBlendFunctionArgument> __blendFunction(*theContext,
                                                                                             &QDemonRenderContext::blendFunction,
                                                                                             &QDemonRenderContext::setBlendFunction,
                                                                                             QDemonRenderBlendFunctionArgument());
        QDemonRenderContextScopedProperty<QDemonRenderBlendEquationArgument> __blendEquation(*theContext,
                                                                                             &QDemonRenderContext::blendEquation,
                                                                                             &QDemonRenderContext::setBlendEquation,
                                                                                             QDemonRenderBlendEquationArgument());
        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __rendertarget(*theContext,
                                                                                             &QDemonRenderContext::renderTarget,
                                                                                             &QDemonRenderContext::setRenderTarget);

        quint32 theSampleCount = 1;
        bool isMultisamplePass = false;
        if (theDesiredEnvironment.msaaMode != AAModeValues::NoAA) {
            switch (theDesiredEnvironment.msaaMode) {
            case AAModeValues::SSAA:
                theSampleCount = 1;
                isMultisamplePass = true;
                break;
            case AAModeValues::X2:
                theSampleCount = 2;
                isMultisamplePass = true;
                break;
            case AAModeValues::X4:
                theSampleCount = 4;
                isMultisamplePass = true;
                break;
            case AAModeValues::X8:
                theSampleCount = 8;
                isMultisamplePass = true;
                break;
            default:
                Q_ASSERT(false);
                break;
            };

            // adjust render size for SSAA
            if (theDesiredEnvironment.msaaMode == AAModeValues::SSAA) {
                QDemonRendererUtil::getSSAARenderSize(theOriginalDesiredEnvironment.width,
                                                      theOriginalDesiredEnvironment.height,
                                                      theDesiredEnvironment.width,
                                                      theDesiredEnvironment.height);
            }
        }
        QDemonResourceFrameBuffer theFrameBuffer(m_resourceManager);
        theFrameBuffer.ensureFrameBuffer();
        auto &renderTargetTexture = theData.texture;
        QDemonRenderTextureTargetType fboAttachmentType = QDemonRenderTextureTargetType::Texture2D;
        if (isMultisamplePass) {
            renderTargetTexture = nullptr;
            if (theSampleCount > 1)
                fboAttachmentType = QDemonRenderTextureTargetType::Texture2D_MS;
        }

        QDemonResourceTexture2D renderColorTexture(m_resourceManager, renderTargetTexture);

        QDemonResourceTexture2D renderDepthStencilTexture(m_resourceManager);

        if (theSampleCount > 1)
            m_context->getRenderContext()->setMultisampleEnabled(true);

        QDemonRenderClearFlags theClearFlags;
        QDemonRenderTextureFormat theDepthStencilTextureFormat(QDemonRenderTextureFormat::Unknown);
        QDemonRenderFrameBufferAttachment theAttachmentLocation(QDemonRenderFrameBufferAttachment::Unknown);
        if (theDesiredEnvironment.stencil) {
            theDepthStencilTextureFormat = QDemonRenderTextureFormat::Depth24Stencil8;
            theAttachmentLocation = QDemonRenderFrameBufferAttachment::DepthStencil;
        } else if (theDesiredEnvironment.depth != QDemonOffscreenRendererDepthValues::NoDepthBuffer) {
            theAttachmentLocation = QDemonRenderFrameBufferAttachment::Depth;
            switch (theDesiredEnvironment.depth) {
            case QDemonOffscreenRendererDepthValues::Depth16:
                theDepthStencilTextureFormat = QDemonRenderTextureFormat::Depth16;
                break;
            case QDemonOffscreenRendererDepthValues::Depth24:
                theDepthStencilTextureFormat = QDemonRenderTextureFormat::Depth24;
                break;
            case QDemonOffscreenRendererDepthValues::Depth32:
                theDepthStencilTextureFormat = QDemonRenderTextureFormat::Depth32;
                break;
            default:
                theAttachmentLocation = QDemonRenderFrameBufferAttachment::Unknown;
                theDepthStencilTextureFormat = QDemonRenderTextureFormat::Unknown;
                break;
            }
        }
        renderColorTexture.ensureTexture(theDesiredEnvironment.width,
                                         theDesiredEnvironment.height,
                                         theDesiredEnvironment.format,
                                         theSampleCount);
        theFrameBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, renderColorTexture.getTexture(), fboAttachmentType);

        if (theDepthStencilTextureFormat != QDemonRenderTextureFormat::Unknown) {
            renderDepthStencilTexture.ensureTexture(theDesiredEnvironment.width,
                                                    theDesiredEnvironment.height,
                                                    theDepthStencilTextureFormat,
                                                    theSampleCount);
            theFrameBuffer->attach(theAttachmentLocation, renderDepthStencilTexture.getTexture(), fboAttachmentType);
        }
        // IsComplete check takes a really long time so I am not going to worry about it for now.

        theContext->setRenderTarget(theFrameBuffer);
        theContext->setViewport(QRect(0, 0, theDesiredEnvironment.width, theDesiredEnvironment.height));
        theContext->setScissorTestEnabled(false);

        theContext->setBlendingEnabled(false);
        theData.renderer->render(theDesiredEnvironment, *theContext, thePresScaleFactor, QDemonRenderScene::AlwaysClear, this);

        if (theSampleCount > 1) {
            QDemonRef<QDemonResourceTexture2D> theResult(new QDemonResourceTexture2D(m_resourceManager, theData.texture));

            if (theDesiredEnvironment.msaaMode != AAModeValues::SSAA) {
                // Have to downsample the FBO.
                QDemonRendererUtil::resolveMutisampleFBOColorOnly(m_resourceManager,
                                                                  *theResult,
                                                                  *m_context->getRenderContext(),
                                                                  theDesiredEnvironment.width,
                                                                  theDesiredEnvironment.height,
                                                                  theDesiredEnvironment.format,
                                                                  theFrameBuffer);

                m_context->getRenderContext()->setMultisampleEnabled(false);
            } else {
                // Resolve the FBO to the layer texture
                QDemonRendererUtil::resolveSSAAFBOColorOnly(m_resourceManager,
                                                            *theResult,
                                                            theOriginalDesiredEnvironment.width,
                                                            theOriginalDesiredEnvironment.height,
                                                            *m_context->getRenderContext(),
                                                            theDesiredEnvironment.width,
                                                            theDesiredEnvironment.height,
                                                            theDesiredEnvironment.format,
                                                            theFrameBuffer);
            }

            Q_ASSERT(theData.texture == theResult->getTexture());
            theResult->forgetTexture();
        } else {
            renderColorTexture.forgetTexture();
        }
        theFrameBuffer->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer(), fboAttachmentType);
        if (theAttachmentLocation != QDemonRenderFrameBufferAttachment::Unknown)
            theFrameBuffer->attach(theAttachmentLocation, QDemonRenderTextureOrRenderBuffer(), fboAttachmentType);
    }

    QDemonOffscreenRenderResult getRenderedItem(const QDemonOffscreenRendererKey &inKey) override
    {
        TRendererMap::iterator theRenderer = m_renderers.find(inKey);
        QVector2D thePresScaleFactor = m_context->getPresentationScaleFactor();
        if (theRenderer != m_renderers.end() && theRenderer.value().rendering == false) {
            QDemonRendererData &theData = theRenderer.value();
            QDemonScopedRenderDataRenderMarker __renderMarker(theData);

            bool renderedThisFrame = theData.texture && theData.frameCount == m_frameCount;
            theData.frameCount = m_frameCount;
            // Two different quick-out pathways.
            if (renderedThisFrame)
                return theData;

            QDemonOffscreenRendererEnvironment theDesiredEnvironment = theData.renderer->getDesiredEnvironment(thePresScaleFactor);
            // Ensure we get a valid width and height
            theDesiredEnvironment.width = QDemonTextRendererInterface::nextMultipleOf4(theDesiredEnvironment.width);
            theDesiredEnvironment.height = QDemonTextRendererInterface::nextMultipleOf4(theDesiredEnvironment.height);
            if (theDesiredEnvironment.width == 0 || theDesiredEnvironment.height == 0) {
                return QDemonOffscreenRenderResult();
            }

            QRect theViewport(0, 0, theDesiredEnvironment.width, theDesiredEnvironment.height);
            auto theRenderList = m_context->getRenderList();
            auto theContext = m_context->getRenderContext();
            // This happens here because if there are any fancy render steps
            QDemonRenderListScopedProperty<bool> scissor(*theRenderList,
                                                         &QDemonRenderListInterface::isScissorTestEnabled,
                                                         &QDemonRenderListInterface::setScissorTestEnabled,
                                                         false);
            QDemonRenderListScopedProperty<QRect> viewport(*theRenderList,
                                                           &QDemonRenderListInterface::getViewport,
                                                           &QDemonRenderListInterface::setViewport,
                                                           theViewport);
            // Some plugins don't use the render list so they need the actual gl context setup.
            QDemonRenderContextScopedProperty<bool> scissorEnabled(*theContext,
                                                                   &QDemonRenderContext::isScissorTestEnabled,
                                                                   &QDemonRenderContext::setScissorTestEnabled,
                                                                   false);
            QDemonRenderContextScopedProperty<QRect> __viewportRect(*theContext,
                                                                    &QDemonRenderContext::viewport,
                                                                    &QDemonRenderContext::setViewport,
                                                                    theViewport);

            quint32 taskId = m_context->getRenderList()->addRenderTask(
                    QDemonRef<QDemonOffscreenRunnable>(new QDemonOffscreenRunnable(*this, theData, theDesiredEnvironment)));

            QDemonOffscreenRenderFlags theFlags = theData.renderer->needsRender(theDesiredEnvironment, thePresScaleFactor, this);
            theData.hasTransparency = theFlags.hasTransparency;
            theData.hasChangedSinceLastFrame = theFlags.hasChangedSinceLastFrame;
            if (theData.texture) {
                // Quick-out if the renderer doesn't need to render itself.
                if (theData.hasChangedSinceLastFrame == false) {
                    m_context->getRenderList()->discardRenderTask(taskId);
                    return theData;
                }
            } else
                theData.hasChangedSinceLastFrame = true;

            // Release existing texture if it doesn't match latest environment request.
            if (theData.texture) {
                QDemonTextureDetails theDetails = theData.texture->textureDetails();
                if (theDesiredEnvironment.width != theDetails.width || theDesiredEnvironment.height != theDetails.height
                    || theDesiredEnvironment.format != theDetails.format) {
                    m_resourceManager->release(theData.texture);
                    theData.texture = nullptr;
                }
            }

            if (theData.texture == nullptr)
                theData.texture = m_resourceManager->allocateTexture2D(theDesiredEnvironment.width,
                                                                       theDesiredEnvironment.height,
                                                                       theDesiredEnvironment.format);

            // Add the node to the graph and get on with it.

            return theData;
        }
        return QDemonOffscreenRenderResult();
    }

    void beginFrame() override
    { /* TODO: m_PerFrameAllocator.reset();*/
    }
    void endFrame() override { ++m_frameCount; }
};

void QDemonOffscreenRunnable::run()
{
    m_renderManager.renderItem(m_data, m_desiredEnvironment);
}
}

QDemonOffscreenRenderManagerInterface::~QDemonOffscreenRenderManagerInterface() = default;

QDemonRef<QDemonOffscreenRenderManagerInterface> QDemonOffscreenRenderManagerInterface::createOffscreenRenderManager(
        const QDemonRef<QDemonResourceManagerInterface> &inManager,
        QDemonRenderContextInterface *inContext)
{
    return QDemonRef<QDemonOffscreenRenderManagerInterface>(new QDemonOffscreenRenderManager(inManager, inContext));
}

QDemonOffscreenRendererInterface::~QDemonOffscreenRendererInterface() = default;

QDemonOffscreenRendererInterface::QDemonOffscreenRendererCallbackInterface::~QDemonOffscreenRendererCallbackInterface() = default;

QT_END_NAMESPACE
