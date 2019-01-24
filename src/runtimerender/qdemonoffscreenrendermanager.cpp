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

uint qHash(const SOffscreenRendererKey &key)
{
    switch (key.getType()) {
    case OffscreenRendererKeyTypes::RegisteredString:
        return qHash(key.getData<QString>());
    case OffscreenRendererKeyTypes::VoidPtr:
        return qHash(reinterpret_cast<size_t>(key.getData<void *>()));
    default:
        Q_UNREACHABLE();
        return 0;
    }
}

namespace {

struct SRendererData : SOffscreenRenderResult
{
    QSharedPointer<IResourceManager> m_ResourceManager;
    quint32 m_FrameCount;
    bool m_Rendering;

    SRendererData(QSharedPointer<IResourceManager> inResourceManager)
        : m_ResourceManager(inResourceManager)
        , m_FrameCount(std::numeric_limits<quint32>::max())
        , m_Rendering(false)
    {
    }
    ~SRendererData()
    {
    }
};

struct SScopedRenderDataRenderMarker
{
    SRendererData &m_Data;
    SScopedRenderDataRenderMarker(SRendererData &d)
        : m_Data(d)
    {
        Q_ASSERT(m_Data.m_Rendering == false);
        m_Data.m_Rendering = true;
    }
    ~SScopedRenderDataRenderMarker() { m_Data.m_Rendering = false; }
};

struct SRenderDataReleaser
{
    QSharedPointer<SRendererData> mPtr;
    SRenderDataReleaser(QSharedPointer<SRendererData> inItem)
        : mPtr(inItem)
    {
    }
    // Transfer ownership
    SRenderDataReleaser(const SRenderDataReleaser &inOther)
        : mPtr(inOther.mPtr)
    {
        const_cast<SRenderDataReleaser &>(inOther).mPtr = nullptr;
    }

    ~SRenderDataReleaser()
    {
        // if (mPtr)
        //     mPtr->Release();
    }
};
struct SOffscreenRenderManager;

struct SOffscreenRunnable : public IRenderTask
{
    SOffscreenRenderManager &m_RenderManager;
    SRendererData &m_Data;
    SOffscreenRendererEnvironment m_DesiredEnvironment;
    SOffscreenRunnable(SOffscreenRenderManager &rm, SRendererData &data,
                       SOffscreenRendererEnvironment env)
        : m_RenderManager(rm)
        , m_Data(data)
        , m_DesiredEnvironment(env)
    {
    }
    void Run() override;
};

struct SOffscreenRenderManager : public IOffscreenRenderManager
{
    typedef QHash<SOffscreenRendererKey, SRendererData> TRendererMap;
    QSharedPointer<IQDemonRenderContext> m_Context;
    QSharedPointer<IResourceManager> m_ResourceManager;
    TRendererMap m_Renderers;
    quint32 m_FrameCount; // cheap per-

    SOffscreenRenderManager(QSharedPointer<IResourceManager> inManager, QSharedPointer<IQDemonRenderContext> inContext)
        : m_Context(inContext)
        , m_ResourceManager(inManager)
        , m_FrameCount(0)
    {
    }

    virtual ~SOffscreenRenderManager() override {}

    QDemonOption<bool> MaybeRegisterOffscreenRenderer(const SOffscreenRendererKey &inKey,
                                                      QSharedPointer<IOffscreenRenderer> inRenderer) override
    {
        TRendererMap::iterator theIter = m_Renderers.find(inKey);
        if (theIter != m_Renderers.end()) {
            SRendererData &theData = theIter.value();
            if (theData.m_Renderer != inRenderer) {
                if (inKey.getType() == OffscreenRendererKeyTypes::RegisteredString) {
                    qCCritical(INVALID_OPERATION, "Different renderers registered under same key: %s",
                               inKey.getData<QString>().toLatin1().constData());
                }
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            return false;
        }
        RegisterOffscreenRenderer(inKey, inRenderer);
        return true;
    }

    void RegisterOffscreenRenderer(const SOffscreenRendererKey &inKey,
                                   QSharedPointer<IOffscreenRenderer> inRenderer) override
    {
        auto inserter = m_Renderers.find(inKey);
        if (inserter == m_Renderers.end())
            inserter = m_Renderers.insert(inKey, SRendererData(m_ResourceManager));
        SRendererData &theData = inserter.value();
        theData.m_Renderer = inRenderer;
    }

    bool HasOffscreenRenderer(const SOffscreenRendererKey &inKey) override
    {
        return m_Renderers.find(inKey) != m_Renderers.end();
    }

    QSharedPointer<IOffscreenRenderer> GetOffscreenRenderer(const SOffscreenRendererKey &inKey) override
    {
        TRendererMap::iterator theRenderer = m_Renderers.find(inKey);
        if (theRenderer != m_Renderers.end()) {
            SRendererData &theData = theRenderer.value();
            return theData.m_Renderer;
        }
        return nullptr;
    }
    void ReleaseOffscreenRenderer(const SOffscreenRendererKey &inKey) override
    {
        m_Renderers.remove(inKey);
    }

    void RenderItem(SRendererData &theData, SOffscreenRendererEnvironment theDesiredEnvironment)
    {
        auto theContext = m_ResourceManager->GetRenderContext();
        QVector2D thePresScaleFactor = m_Context->GetPresentationScaleFactor();
        SOffscreenRendererEnvironment theOriginalDesiredEnvironment(theDesiredEnvironment);
        // Ensure that our overall render context comes back no matter what the client does.
        QDemonRenderContextScopedProperty<QVector4D> __clearColor(
                    *theContext, &QDemonRenderContext::GetClearColor, &QDemonRenderContext::SetClearColor,
                    QVector4D(0, 0, 0, 0));
        QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                    *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled, false);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __scissorRect(
                    *theContext, &QDemonRenderContext::GetScissorRect, &QDemonRenderContext::SetScissorRect);
        QDemonRenderContextScopedProperty<QDemonRenderRect> __viewportRect(
                    *theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport);
        QDemonRenderContextScopedProperty<bool> __depthWrite(
                    *theContext, &QDemonRenderContext::IsDepthWriteEnabled,
                    &QDemonRenderContext::SetDepthWriteEnabled, false);
        QDemonRenderContextScopedProperty<QDemonRenderBoolOp::Enum> __depthFunction(
                    *theContext, &QDemonRenderContext::GetDepthFunction, &QDemonRenderContext::SetDepthFunction,
                    QDemonRenderBoolOp::Less);
        QDemonRenderContextScopedProperty<bool> __blendEnabled(
                    *theContext, &QDemonRenderContext::IsBlendingEnabled, &QDemonRenderContext::SetBlendingEnabled,
                    false);
        QDemonRenderContextScopedProperty<QDemonRenderBlendFunctionArgument>
                __blendFunction(*theContext, &QDemonRenderContext::GetBlendFunction,
                                &QDemonRenderContext::SetBlendFunction,
                                QDemonRenderBlendFunctionArgument());
        QDemonRenderContextScopedProperty<QDemonRenderBlendEquationArgument>
                __blendEquation(*theContext, &QDemonRenderContext::GetBlendEquation,
                                &QDemonRenderContext::SetBlendEquation,
                                QDemonRenderBlendEquationArgument());
        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer> >
                __rendertarget(*theContext, &QDemonRenderContext::GetRenderTarget,
                               &QDemonRenderContext::SetRenderTarget);

        quint32 theSampleCount = 1;
        bool isMultisamplePass = false;
        if (theDesiredEnvironment.m_MSAAMode != AAModeValues::NoAA) {
            switch (theDesiredEnvironment.m_MSAAMode) {
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
            if (theDesiredEnvironment.m_MSAAMode == AAModeValues::SSAA) {
                CRendererUtil::GetSSAARenderSize(
                            theOriginalDesiredEnvironment.m_Width, theOriginalDesiredEnvironment.m_Height,
                            theDesiredEnvironment.m_Width, theDesiredEnvironment.m_Height);
            }
        }
        CResourceFrameBuffer theFrameBuffer(m_ResourceManager);
        theFrameBuffer.EnsureFrameBuffer();
        auto &renderTargetTexture = theData.m_Texture;
        QDemonRenderTextureTargetType::Enum fboAttachmentType =
                QDemonRenderTextureTargetType::Texture2D;
        if (isMultisamplePass) {
            renderTargetTexture = nullptr;
            if (theSampleCount > 1)
                fboAttachmentType = QDemonRenderTextureTargetType::Texture2D_MS;
        }

        CResourceTexture2D renderColorTexture(m_ResourceManager, renderTargetTexture);

        CResourceTexture2D renderDepthStencilTexture(m_ResourceManager);

        if (theSampleCount > 1)
            m_Context->GetRenderContext()->SetMultisampleEnabled(true);

        QDemonRenderClearFlags theClearFlags;
        QDemonRenderTextureFormats::Enum theDepthStencilTextureFormat(QDemonRenderTextureFormats::Unknown);
        QDemonRenderFrameBufferAttachments::Enum theAttachmentLocation(
                    QDemonRenderFrameBufferAttachments::Unknown);
        if (theDesiredEnvironment.m_Stencil) {
            theDepthStencilTextureFormat = QDemonRenderTextureFormats::Depth24Stencil8;
            theAttachmentLocation = QDemonRenderFrameBufferAttachments::DepthStencil;
        } else if (theDesiredEnvironment.m_Depth != OffscreenRendererDepthValues::NoDepthBuffer) {
            theAttachmentLocation = QDemonRenderFrameBufferAttachments::Depth;
            switch (theDesiredEnvironment.m_Depth) {
            case OffscreenRendererDepthValues::Depth16:
                theDepthStencilTextureFormat = QDemonRenderTextureFormats::Depth16;
                break;
            case OffscreenRendererDepthValues::Depth24:
                theDepthStencilTextureFormat = QDemonRenderTextureFormats::Depth24;
                break;
            case OffscreenRendererDepthValues::Depth32:
                theDepthStencilTextureFormat = QDemonRenderTextureFormats::Depth32;
                break;
            default:
                theAttachmentLocation = QDemonRenderFrameBufferAttachments::Unknown;
                theDepthStencilTextureFormat = QDemonRenderTextureFormats::Unknown;
                break;
            }
        }
        renderColorTexture.EnsureTexture(theDesiredEnvironment.m_Width,
                                         theDesiredEnvironment.m_Height,
                                         theDesiredEnvironment.m_Format, theSampleCount);
        theFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0, renderColorTexture.GetTexture(),
                               fboAttachmentType);

        if (theDepthStencilTextureFormat != QDemonRenderTextureFormats::Unknown) {
            renderDepthStencilTexture.EnsureTexture(theDesiredEnvironment.m_Width,
                                                    theDesiredEnvironment.m_Height,
                                                    theDepthStencilTextureFormat, theSampleCount);
            theFrameBuffer->Attach(theAttachmentLocation, renderDepthStencilTexture.GetTexture(),
                                   fboAttachmentType);
        }
        // IsComplete check takes a really long time so I am not going to worry about it for now.

        theContext->SetRenderTarget(theFrameBuffer);
        theContext->SetViewport(
                    QDemonRenderRect(0, 0, theDesiredEnvironment.m_Width, theDesiredEnvironment.m_Height));
        theContext->SetScissorTestEnabled(false);

        theContext->SetBlendingEnabled(false);
        theData.m_Renderer->Render(theDesiredEnvironment, *theContext, thePresScaleFactor,
                                   SScene::AlwaysClear, this);

        if (theSampleCount > 1) {
            QSharedPointer<CResourceTexture2D> theResult(new CResourceTexture2D(m_ResourceManager, theData.m_Texture));

            if (theDesiredEnvironment.m_MSAAMode != AAModeValues::SSAA) {
                // Have to downsample the FBO.
                CRendererUtil::ResolveMutisampleFBOColorOnly(
                            m_ResourceManager, *theResult, *m_Context->GetRenderContext(),
                            theDesiredEnvironment.m_Width, theDesiredEnvironment.m_Height,
                            theDesiredEnvironment.m_Format, theFrameBuffer);

                m_Context->GetRenderContext()->SetMultisampleEnabled(false);
            } else {
                // Resolve the FBO to the layer texture
                CRendererUtil::ResolveSSAAFBOColorOnly(
                            m_ResourceManager, *theResult, theOriginalDesiredEnvironment.m_Width,
                            theOriginalDesiredEnvironment.m_Height, *m_Context->GetRenderContext(),
                            theDesiredEnvironment.m_Width, theDesiredEnvironment.m_Height,
                            theDesiredEnvironment.m_Format, theFrameBuffer);
            }

            Q_ASSERT(theData.m_Texture == theResult->GetTexture());
            theResult->ForgetTexture();
        } else {
            renderColorTexture.ForgetTexture();
        }
        theFrameBuffer->Attach(QDemonRenderFrameBufferAttachments::Color0,
                               QDemonRenderTextureOrRenderBuffer(), fboAttachmentType);
        if (theAttachmentLocation != QDemonRenderFrameBufferAttachments::Unknown)
            theFrameBuffer->Attach(theAttachmentLocation, QDemonRenderTextureOrRenderBuffer(),
                                   fboAttachmentType);
    }

    SOffscreenRenderResult GetRenderedItem(const SOffscreenRendererKey &inKey) override
    {
        TRendererMap::iterator theRenderer = m_Renderers.find(inKey);
        QVector2D thePresScaleFactor = m_Context->GetPresentationScaleFactor();
        if (theRenderer != m_Renderers.end() && theRenderer.value().m_Rendering == false) {
            SRendererData &theData = theRenderer.value();
            SScopedRenderDataRenderMarker __renderMarker(theData);

            bool renderedThisFrame = theData.m_Texture && theData.m_FrameCount == m_FrameCount;
            theData.m_FrameCount = m_FrameCount;
            // Two different quick-out pathways.
            if (renderedThisFrame)
                return theData;

            SOffscreenRendererEnvironment theDesiredEnvironment =
                    theData.m_Renderer->GetDesiredEnvironment(thePresScaleFactor);
            // Ensure we get a valid width and height
            theDesiredEnvironment.m_Width =
                    ITextRenderer::NextMultipleOf4(theDesiredEnvironment.m_Width);
            theDesiredEnvironment.m_Height =
                    ITextRenderer::NextMultipleOf4(theDesiredEnvironment.m_Height);
            if (theDesiredEnvironment.m_Width == 0 || theDesiredEnvironment.m_Height == 0) {
                return SOffscreenRenderResult();
            }

            QDemonRenderRect theViewport(0, 0, theDesiredEnvironment.m_Width,
                                         theDesiredEnvironment.m_Height);
            auto theRenderList = m_Context->GetRenderList();
            auto theContext = m_Context->GetRenderContext();
            // This happens here because if there are any fancy render steps
            SRenderListScopedProperty<bool> scissor(*theRenderList,
                                                     &IRenderList::IsScissorTestEnabled,
                                                     &IRenderList::SetScissorTestEnabled, false);
            SRenderListScopedProperty<QDemonRenderRect> viewport(
                        *theRenderList, &IRenderList::GetViewport, &IRenderList::SetViewport, theViewport);
            // Some plugins don't use the render list so they need the actual gl context setup.
            QDemonRenderContextScopedProperty<bool> scissorEnabled(
                        *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                        &QDemonRenderContext::SetScissorTestEnabled, false);
            QDemonRenderContextScopedProperty<QDemonRenderRect> __viewportRect(
                        *theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport,
                        theViewport);

            quint32 taskId = m_Context->GetRenderList()->AddRenderTask(QSharedPointer<SOffscreenRunnable>(new SOffscreenRunnable(*this, theData, theDesiredEnvironment)));

            SOffscreenRenderFlags theFlags =
                    theData.m_Renderer->NeedsRender(theDesiredEnvironment, thePresScaleFactor, this);
            theData.m_HasTransparency = theFlags.m_HasTransparency;
            theData.m_HasChangedSinceLastFrame = theFlags.m_HasChangedSinceLastFrame;
            if (theData.m_Texture) {
                // Quick-out if the renderer doesn't need to render itself.
                if (theData.m_HasChangedSinceLastFrame == false) {
                    m_Context->GetRenderList()->DiscardRenderTask(taskId);
                    return theData;
                }
            } else
                theData.m_HasChangedSinceLastFrame = true;

            // Release existing texture if it doesn't match latest environment request.
            if (theData.m_Texture) {
                STextureDetails theDetails = theData.m_Texture->GetTextureDetails();
                if (theDesiredEnvironment.m_Width != theDetails.m_Width
                        || theDesiredEnvironment.m_Height != theDetails.m_Height
                        || theDesiredEnvironment.m_Format != theDetails.m_Format) {
                    m_ResourceManager->Release(theData.m_Texture);
                    theData.m_Texture = nullptr;
                }
            }

            if (theData.m_Texture == nullptr)
                theData.m_Texture = m_ResourceManager->AllocateTexture2D(
                            theDesiredEnvironment.m_Width, theDesiredEnvironment.m_Height,
                            theDesiredEnvironment.m_Format);

            // Add the node to the graph and get on with it.

            return theData;
        }
        return SOffscreenRenderResult();
    }

    void BeginFrame() override { /* TODO: m_PerFrameAllocator.reset();*/ }
    void EndFrame() override { ++m_FrameCount; }
};

void SOffscreenRunnable::Run()
{
    m_RenderManager.RenderItem(m_Data, m_DesiredEnvironment);
}
}

IOffscreenRenderManager::~IOffscreenRenderManager()
{

}

QSharedPointer<IOffscreenRenderManager> IOffscreenRenderManager::CreateOffscreenRenderManager(
        QSharedPointer<IResourceManager> inManager,
        QSharedPointer<IQDemonRenderContext> inContext)
{
    return QSharedPointer<IOffscreenRenderManager>(new SOffscreenRenderManager(inManager, inContext));
}

IOffscreenRenderer::~IOffscreenRenderer()
{

}

QT_END_NAMESPACE
