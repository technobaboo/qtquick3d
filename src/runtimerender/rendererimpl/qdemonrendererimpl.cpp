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


#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendererimpl.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrendertexttextureatlas.h>
#include <QtDemonRuntimeRender/qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>

#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemon/QDemonDataRef>
#include <QtDemon/qdemonutils.h>

#include <stdlib.h>
#include <algorithm>

#ifdef _WIN32
#pragma warning(disable : 4355)
#endif

// Quick tests you can run to find performance problems

//#define QDEMON_RENDER_DISABLE_HARDWARE_BLENDING 1
//#define QDEMON_RENDER_DISABLE_LIGHTING 1
//#define QDEMON_RENDER_DISABLE_TEXTURING 1
//#define QDEMON_RENDER_DISABLE_TRANSPARENCY 1
//#define QDEMON_RENDER_DISABLE_FRUSTUM_CULLING 1

// If you are fillrate bound then sorting opaque objects can help in some circumstances
//#define QDEMON_RENDER_DISABLE_OPAQUE_SORT 1

QT_BEGIN_NAMESPACE

struct SRenderableImage;
struct SShaderGeneratorGeneratedShader;
struct SSubsetRenderable;

static SRenderInstanceId combineLayerAndId(const SLayer *layer, const SRenderInstanceId id)
{
    uint64_t x = (uint64_t)layer;
    x += 31u * (uint64_t)id;
    return (SRenderInstanceId)x;
}

QDemonRendererImpl::QDemonRendererImpl(IQDemonRenderContext *ctx)
    : m_demonContext(ctx)
    , m_Context(ctx->GetRenderContext())
    , m_BufferManager(ctx->GetBufferManager())
    , m_OffscreenRenderManager(ctx->GetOffscreenRenderManager())
    #ifdef ADVANCED_BLEND_SW_FALLBACK
    , m_LayerBlendTexture(ctx->GetResourceManager())
    , m_BlendFB(nullptr)
    #endif
    , m_CurrentLayer(nullptr)
    , m_PickRenderPlugins(true)
    , m_LayerCachingEnabled(true)
    , m_LayerGPuProfilingEnabled(false)
{
}
QDemonRendererImpl::~QDemonRendererImpl()
{
    m_LayerShaders.clear();
    m_Shaders.clear();
    m_InstanceRenderMap.clear();
    m_ConstantBuffers.clear();
}

void QDemonRendererImpl::ChildrenUpdated(SNode &inParent)
{
    if (inParent.m_Type == GraphObjectTypes::Layer) {
        TInstanceRenderMap::iterator theIter = m_InstanceRenderMap.find(static_cast<SRenderInstanceId>(&inParent));
        if (theIter != m_InstanceRenderMap.end()) {
            theIter.value()->m_CamerasAndLights.clear();
            theIter.value()->m_RenderableNodes.clear();
        }
    } else if (inParent.m_Parent)
        ChildrenUpdated(*inParent.m_Parent);
}

float QDemonRendererImpl::GetTextScale(const SText &inText)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inText);
    if (theData)
        return theData->m_TextScale;
    return 1.0f;
}

static inline SLayer *GetNextLayer(SLayer &inLayer)
{
    if (inLayer.m_NextSibling && inLayer.m_NextSibling->m_Type == GraphObjectTypes::Layer)
        return static_cast<SLayer *>(inLayer.m_NextSibling);
    return nullptr;
}

static inline void MaybePushLayer(SLayer &inLayer, QVector<SLayer *> &outLayerList)
{
    inLayer.CalculateGlobalVariables();
    if (inLayer.m_Flags.IsGloballyActive() && inLayer.m_Flags.IsLayerRenderToTarget())
        outLayerList.push_back(&inLayer);
}
static void BuildRenderableLayers(SLayer &inLayer, QVector<SLayer *> &renderableLayers,
                                  bool inRenderSiblings)
{
    MaybePushLayer(inLayer, renderableLayers);
    if (inRenderSiblings) {
        for (SLayer *theNextLayer = GetNextLayer(inLayer); theNextLayer;
             theNextLayer = GetNextLayer(*theNextLayer))
            MaybePushLayer(*theNextLayer, renderableLayers);
    }
}

bool QDemonRendererImpl::PrepareLayerForRender(SLayer &inLayer,
                                               const QVector2D &inViewportDimensions,
                                               bool inRenderSiblings,
                                               const SRenderInstanceId id)
{
    (void)inViewportDimensions;
    QVector<SLayer *> renderableLayers;
    // Found by fair roll of the dice.
    renderableLayers.reserve(4);

    BuildRenderableLayers(inLayer, renderableLayers, inRenderSiblings);

    bool retval = false;

    for (QVector<SLayer *>::reverse_iterator iter = renderableLayers.rbegin(),
         end = renderableLayers.rend();
         iter != end; ++iter) {
        // Store the previous state of if we were rendering a layer.
        SLayer *theLayer = *iter;
        QSharedPointer<SLayerRenderData> theRenderData = GetOrCreateLayerRenderDataForNode(*theLayer, id);

        if (theRenderData) {
            theRenderData->PrepareForRender();
            retval = retval || theRenderData->m_LayerPrepResult->m_Flags.WasDirty();
        } else {
            Q_ASSERT(false);
        }
    }

    return retval;
}

void QDemonRendererImpl::RenderLayer(SLayer &inLayer, const QVector2D &inViewportDimensions,
                                     bool clear, QVector3D clearColor, bool inRenderSiblings,
                                     const SRenderInstanceId id)
{
    (void)inViewportDimensions;
    QVector<SLayer *> renderableLayers;
    // Found by fair roll of the dice.
    renderableLayers.reserve(4);

    BuildRenderableLayers(inLayer, renderableLayers, inRenderSiblings);

    QSharedPointer<QDemonRenderContext> theRenderContext(m_demonContext->GetRenderContext());
    QSharedPointer<QDemonRenderFrameBuffer> theFB = theRenderContext->GetRenderTarget();
    for (QVector<SLayer *>::reverse_iterator iter = renderableLayers.rbegin(),
         end = renderableLayers.rend();
         iter != end; ++iter) {
        SLayer *theLayer = *iter;
        QSharedPointer<SLayerRenderData> theRenderData = GetOrCreateLayerRenderDataForNode(*theLayer, id);
        SLayerRenderPreparationResult &prepRes(*theRenderData->m_LayerPrepResult);
        LayerBlendTypes::Enum layerBlend = prepRes.GetLayer()->GetLayerBlend();
#ifdef ADVANCED_BLEND_SW_FALLBACK
        if ((layerBlend == LayerBlendTypes::Overlay ||
             layerBlend == LayerBlendTypes::ColorBurn ||
             layerBlend == LayerBlendTypes::ColorDodge) &&
                !theRenderContext->IsAdvancedBlendHwSupported() &&
                !theRenderContext->IsAdvancedBlendHwSupportedKHR()) {
            // Create and set up FBO and texture for advanced blending SW fallback
            QDemonRenderRect viewport = theRenderContext->GetViewport();
            m_LayerBlendTexture.EnsureTexture(viewport.m_Width + viewport.m_X,
                                              viewport.m_Height + viewport.m_Y,
                                              QDemonRenderTextureFormats::RGBA8);
            if (m_BlendFB == nullptr)
                m_BlendFB = theRenderContext->CreateFrameBuffer();
            m_BlendFB->Attach(QDemonRenderFrameBufferAttachments::Color0, m_LayerBlendTexture.GetTexture());
            theRenderContext->SetRenderTarget(m_BlendFB);
            theRenderContext->SetScissorTestEnabled(false);
            QVector4D color(0.0f, 0.0f, 0.0f, 0.0f);
            if (clear) {
                color.setX(clearColor.x());
                color.setY(clearColor.y());
                color.setZ(clearColor.z());
                color.setW(1.0f);
            }
            QVector4D origColor = theRenderContext->GetClearColor();
            theRenderContext->SetClearColor(color);
            theRenderContext->Clear(QDemonRenderClearValues::Color);
            theRenderContext->SetClearColor(origColor);
            theRenderContext->SetRenderTarget(theFB);
            break;
        } else {
            m_LayerBlendTexture.ReleaseTexture();
        }
#endif
    }
    for (QVector<SLayer *>::reverse_iterator iter = renderableLayers.rbegin(),
         end = renderableLayers.rend();
         iter != end; ++iter) {
        // Store the previous state of if we were rendering a layer.
        SLayer *theLayer = *iter;
        QSharedPointer<SLayerRenderData> theRenderData = GetOrCreateLayerRenderDataForNode(*theLayer, id);

        if (theRenderData) {
            if (theRenderData->m_LayerPrepResult->IsLayerVisible())
                theRenderData->RunnableRenderToViewport(theFB);
        } else {
            Q_ASSERT(false);
        }
    }
}

SLayer *QDemonRendererImpl::GetLayerForNode(const SNode &inNode) const
{
    if (inNode.m_Type == GraphObjectTypes::Layer) {
        return &const_cast<SLayer &>(static_cast<const SLayer &>(inNode));
    }
    if (inNode.m_Parent)
        return GetLayerForNode(*inNode.m_Parent);
    return nullptr;
}

QSharedPointer<SLayerRenderData> QDemonRendererImpl::GetOrCreateLayerRenderDataForNode(const SNode &inNode, const SRenderInstanceId id)
{
    const SLayer *theLayer = GetLayerForNode(inNode);
    if (theLayer) {
        TInstanceRenderMap::const_iterator theIter = m_InstanceRenderMap.find(combineLayerAndId(theLayer, id));
        if (theIter != m_InstanceRenderMap.end())
            return QSharedPointer<SLayerRenderData>(theIter.value());

        QSharedPointer<SLayerRenderData> theRenderData = QSharedPointer<SLayerRenderData>(new SLayerRenderData(
                                                                                              const_cast<SLayer &>(*theLayer), sharedFromThis()));
        m_InstanceRenderMap.insert(combineLayerAndId(theLayer, id), theRenderData);

        // create a profiler if enabled
        if (IsLayerGpuProfilingEnabled() && theRenderData)
            theRenderData->CreateGpuProfiler();

        return theRenderData;
    }
    return nullptr;
}

SCamera *QDemonRendererImpl::GetCameraForNode(const SNode &inNode) const
{
    QSharedPointer<SLayerRenderData> theLayer =
            const_cast<QDemonRendererImpl &>(*this).GetOrCreateLayerRenderDataForNode(inNode);
    if (theLayer)
        return theLayer->m_Camera;
    return nullptr;
}

QDemonOption<SCuboidRect> QDemonRendererImpl::GetCameraBounds(const SGraphObject &inObject)
{
    if (GraphObjectTypes::IsNodeType(inObject.m_Type)) {
        const SNode &theNode = static_cast<const SNode &>(inObject);
        QSharedPointer<SLayerRenderData> theLayer = GetOrCreateLayerRenderDataForNode(theNode);
        if (theLayer->GetOffscreenRenderer() == false) {
            SCamera *theCamera = theLayer->m_Camera;
            if (theCamera)
                return theCamera->GetCameraBounds(
                            theLayer->m_LayerPrepResult->GetLayerToPresentationViewport(),
                            theLayer->m_LayerPrepResult->GetPresentationDesignDimensions());
        }
    }
    return QDemonOption<SCuboidRect>();
}

void QDemonRendererImpl::DrawScreenRect(QDemonRenderRectF inRect, const QVector3D &inColor)
{
    SCamera theScreenCamera;
    theScreenCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
    QDemonRenderRectF theViewport(m_Context->GetViewport());
    theScreenCamera.m_Flags.SetOrthographic(true);
    theScreenCamera.CalculateGlobalVariables(theViewport,
                                             QVector2D(theViewport.m_Width, theViewport.m_Height));
    GenerateXYQuad();
    if (!m_ScreenRectShader) {
        QSharedPointer<IShaderProgramGenerator> theGenerator(GetProgramGenerator());
        theGenerator->BeginProgram();
        IShaderStageGenerator &vertexGenerator(
                    *theGenerator->GetStage(ShaderGeneratorStages::Vertex));
        IShaderStageGenerator &fragmentGenerator(
                    *theGenerator->GetStage(ShaderGeneratorStages::Fragment));
        vertexGenerator.AddIncoming("attr_pos", "vec3");
        vertexGenerator.AddUniform("model_view_projection", "mat4");
        vertexGenerator.AddUniform("rectangle_dims", "vec3");
        vertexGenerator.Append("void main() {");
        vertexGenerator.Append(
                    "\tgl_Position = model_view_projection * vec4(attr_pos * rectangle_dims, 1.0);");
        vertexGenerator.Append("}");
        fragmentGenerator.AddUniform("output_color", "vec3");
        fragmentGenerator.Append("void main() {");
        fragmentGenerator.Append("\tgl_FragColor.rgb = output_color;");
        fragmentGenerator.Append("\tgl_FragColor.a = 1.0;");
        fragmentGenerator.Append("}");
        // No flags enabled
        m_ScreenRectShader = theGenerator->CompileGeneratedShader(
                    "DrawScreenRect", SShaderCacheProgramFlags(), TShaderFeatureSet());
    }
    if (m_ScreenRectShader) {
        // Fudge the rect by one pixel to ensure we see all the corners.
        if (inRect.m_Width > 1)
            inRect.m_Width -= 1;
        if (inRect.m_Height > 1)
            inRect.m_Height -= 1;
        inRect.m_X += 1;
        inRect.m_Y += 1;
        // Figure out the rect center.
        SNode theNode;

        QVector2D rectGlobalCenter = inRect.Center();
        QVector2D rectCenter(theViewport.ToNormalizedRectRelative(rectGlobalCenter));
        theNode.m_Position.setX(rectCenter.x());
        theNode.m_Position.setY(rectCenter.y());
        theNode.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
        theNode.CalculateGlobalVariables();
        QMatrix4x4 theViewProjection;
        theScreenCamera.CalculateViewProjectionMatrix(theViewProjection);
        QMatrix4x4 theMVP;
        QMatrix3x3 theNormal;
        theNode.CalculateMVPAndNormalMatrix(theViewProjection, theMVP, theNormal);
        m_Context->SetBlendingEnabled(false);
        m_Context->SetDepthWriteEnabled(false);
        m_Context->SetDepthTestEnabled(false);
        m_Context->SetCullingEnabled(false);
        m_Context->SetActiveShader(m_ScreenRectShader);
        m_ScreenRectShader->SetPropertyValue("model_view_projection", theMVP);
        m_ScreenRectShader->SetPropertyValue("output_color", inColor);
        m_ScreenRectShader->SetPropertyValue(
                    "rectangle_dims", QVector3D(inRect.m_Width / 2.0f, inRect.m_Height / 2.0f, 0.0f));
    }
    if (!m_RectInputAssembler) {
        Q_ASSERT(m_QuadVertexBuffer);
        quint8 indexData[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

        m_RectIndexBuffer = m_Context->CreateIndexBuffer(
                    QDemonRenderBufferUsageType::Static,
                    QDemonRenderComponentTypes::UnsignedInteger8, sizeof(indexData),
                    toConstDataRef(indexData, sizeof(indexData)));

        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos",
            QDemonRenderComponentTypes::Float32, 3),
        };

        // create our attribute layout
        m_RectAttribLayout = m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 1));

        quint32 strides = m_QuadVertexBuffer->GetStride();
        quint32 offsets = 0;
        m_RectInputAssembler = m_Context->CreateInputAssembler(
                    m_RectAttribLayout, toConstDataRef(&m_QuadVertexBuffer, 1), m_RectIndexBuffer,
                    toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
    }

    m_Context->SetInputAssembler(m_RectInputAssembler);
    m_Context->Draw(QDemonRenderDrawMode::Lines, m_RectIndexBuffer->GetNumIndices(), 0);
}

void QDemonRendererImpl::SetupWidgetLayer()
{
    QSharedPointer<QDemonRenderContext> theContext = m_demonContext->GetRenderContext();

    if (!m_WidgetTexture) {
        QSharedPointer<IResourceManager> theManager = m_demonContext->GetResourceManager();
        m_WidgetTexture = theManager->AllocateTexture2D(m_BeginFrameViewport.m_Width,
                                                        m_BeginFrameViewport.m_Height,
                                                        QDemonRenderTextureFormats::RGBA8);
        m_WidgetFBO = theManager->AllocateFrameBuffer();
        m_WidgetFBO->Attach(QDemonRenderFrameBufferAttachments::Color0,
                            QDemonRenderTextureOrRenderBuffer(m_WidgetTexture));
        theContext->SetRenderTarget(m_WidgetFBO);

        // QDemonRenderRect theScissorRect( 0, 0, m_BeginFrameViewport.m_Width,
        // m_BeginFrameViewport.m_Height );
        // QDemonRenderContextScopedProperty<QDemonRenderRect> __scissorRect( theContext,
        // &QDemonRenderContext::GetScissorRect, &QDemonRenderContext::SetScissorRect, theScissorRect );
        QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                    *theContext, &QDemonRenderContext::IsScissorTestEnabled,
                    &QDemonRenderContext::SetScissorTestEnabled, false);
        m_Context->SetClearColor(QVector4D(0, 0, 0, 0));
        m_Context->Clear(QDemonRenderClearValues::Color);

    } else
        theContext->SetRenderTarget(m_WidgetFBO);
}

void QDemonRendererImpl::BeginFrame()
{
    for (quint32 idx = 0, end = m_LastFrameLayers.size(); idx < end; ++idx)
        m_LastFrameLayers[idx]->ResetForFrame();
    m_LastFrameLayers.clear();
    m_BeginFrameViewport = m_demonContext->GetRenderList()->GetViewport();
}
void QDemonRendererImpl::EndFrame()
{
    if (m_WidgetTexture) {
        // Releasing the widget FBO can set it as the active frame buffer.
        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __fbo(
                    *m_Context, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);
        STextureDetails theDetails = m_WidgetTexture->GetTextureDetails();
        m_Context->SetBlendingEnabled(true);
        // Colors are expected to be non-premultiplied, so we premultiply alpha into them at
        // this point.
        m_Context->SetBlendFunction(QDemonRenderBlendFunctionArgument(
                                        QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                        QDemonRenderSrcBlendFunc::One, QDemonRenderDstBlendFunc::OneMinusSrcAlpha));
        m_Context->SetBlendEquation(QDemonRenderBlendEquationArgument(
                                        QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add));

        m_Context->SetDepthTestEnabled(false);
        m_Context->SetScissorTestEnabled(false);
        m_Context->SetViewport(m_BeginFrameViewport);
        SCamera theCamera;
        theCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
        theCamera.m_Flags.SetOrthographic(true);
        QVector2D theTextureDims((float)theDetails.m_Width, (float)theDetails.m_Height);
        theCamera.CalculateGlobalVariables(
                    QDemonRenderRect(0, 0, theDetails.m_Width, theDetails.m_Height), theTextureDims);
        QMatrix4x4 theViewProj;
        theCamera.CalculateViewProjectionMatrix(theViewProj);
        RenderQuad(theTextureDims, theViewProj, *m_WidgetTexture);

        QSharedPointer<IResourceManager> theManager(m_demonContext->GetResourceManager());
        theManager->Release(m_WidgetFBO);
        theManager->Release(m_WidgetTexture);
        m_WidgetTexture = nullptr;
        m_WidgetFBO = nullptr;
    }
}

inline bool PickResultLessThan(const QDemonRenderPickResult &lhs, const QDemonRenderPickResult &rhs)
{
    return FloatLessThan(lhs.m_CameraDistanceSq, rhs.m_CameraDistanceSq);
}

inline float ClampUVCoord(float inUVCoord, QDemonRenderTextureCoordOp::Enum inCoordOp)
{
    if (inUVCoord > 1.0f || inUVCoord < 0.0f) {
        switch (inCoordOp) {
        default:
            Q_ASSERT(false);
            break;
        case QDemonRenderTextureCoordOp::ClampToEdge:
            inUVCoord = qMin(inUVCoord, 1.0f);
            inUVCoord = qMax(inUVCoord, 0.0f);
            break;
        case QDemonRenderTextureCoordOp::Repeat: {
            float multiplier = inUVCoord > 0.0f ? 1.0f : -1.0f;
            float clamp = fabs(inUVCoord);
            clamp = clamp - floor(clamp);
            if (multiplier < 0)
                inUVCoord = 1.0f - clamp;
            else
                inUVCoord = clamp;
        } break;
        case QDemonRenderTextureCoordOp::MirroredRepeat: {
            float multiplier = inUVCoord > 0.0f ? 1.0f : -1.0f;
            float clamp = fabs(inUVCoord);
            if (multiplier > 0.0f)
                clamp -= 1.0f;
            quint32 isMirrored = ((quint32)clamp) % 2 == 0;
            float remainder = clamp - floor(clamp);
            inUVCoord = remainder;
            if (isMirrored) {
                if (multiplier > 0.0f)
                    inUVCoord = 1.0f - inUVCoord;
            } else {
                if (multiplier < 0.0f)
                    inUVCoord = 1.0f - remainder;
            }
        } break;
        }
    }
    return inUVCoord;
}

static QPair<QVector2D, QVector2D>
GetMouseCoordsAndViewportFromSubObject(QVector2D inLocalHitUVSpace,
                                       QDemonRenderPickSubResult &inSubResult)
{
    QMatrix4x4 theTextureMatrix(inSubResult.m_TextureMatrix);
    QVector3D theNewUVCoords(
                mat44::transform(theTextureMatrix, (QVector3D(inLocalHitUVSpace.x(), inLocalHitUVSpace.y(), 0))));
    theNewUVCoords.setX(ClampUVCoord(theNewUVCoords.x(), inSubResult.m_HorizontalTilingMode));
    theNewUVCoords.setY(ClampUVCoord(theNewUVCoords.y(), inSubResult.m_VerticalTilingMode));
    QVector2D theViewportDimensions =
            QVector2D(float(inSubResult.m_ViewportWidth), float(inSubResult.m_ViewportHeight));
    QVector2D theMouseCoords(theNewUVCoords.x() * theViewportDimensions.x(),
                             (1.0f - theNewUVCoords.y()) * theViewportDimensions.y());

    return QPair<QVector2D, QVector2D>(theMouseCoords, theViewportDimensions);
}

SPickResultProcessResult QDemonRendererImpl::ProcessPickResultList(bool inPickEverything)
{
    if (m_LastPickResults.empty())
        return SPickResultProcessResult();
    // Things are rendered in a particular order and we need to respect that ordering.
    std::stable_sort(m_LastPickResults.begin(), m_LastPickResults.end(), PickResultLessThan);

    // We need to pick against sub objects basically somewhat recursively
    // but if we don't hit any sub objects and the parent isn't pickable then
    // we need to move onto the next item in the list.
    // We need to keep in mind that theQuery->Pick will enter this method in a later
    // stack frame so *if* we get to sub objects we need to pick against them but if the pick
    // completely misses *and* the parent object locally pickable is false then we need to move
    // onto the next object.

    quint32 numToCopy = (quint32)m_LastPickResults.size();
    quint32 numCopyBytes = numToCopy * sizeof(QDemonRenderPickResult);
    QDemonRenderPickResult *thePickResults = reinterpret_cast<QDemonRenderPickResult *>(::malloc(numCopyBytes));
    ::memcpy(thePickResults, m_LastPickResults.data(), numCopyBytes);
    m_LastPickResults.clear();
    bool foundValidResult = false;
    SPickResultProcessResult thePickResult(thePickResults[0]);
    for (size_t idx = 0; idx < numToCopy && foundValidResult == false; ++idx) {
        thePickResult = thePickResults[idx];
        // Here we do a hierarchy.  Picking against sub objects takes precedence.
        // If picking against the sub object doesn't return a valid result *and*
        // the current object isn't globally pickable then we move onto the next object returned
        // by the pick query.
        if (thePickResult.m_HitObject != nullptr && thePickResult.m_FirstSubObject != nullptr
                && m_PickRenderPlugins) {
            QVector2D theUVCoords(thePickResult.m_LocalUVCoords.x(),
                                  thePickResult.m_LocalUVCoords.y());
            QSharedPointer<IOffscreenRenderer> theSubRenderer(thePickResult.m_FirstSubObject->m_SubRenderer);
            QPair<QVector2D, QVector2D> mouseAndViewport =
                    GetMouseCoordsAndViewportFromSubObject(theUVCoords,
                                                           *thePickResult.m_FirstSubObject);
            QVector2D theMouseCoords = mouseAndViewport.first;
            QVector2D theViewportDimensions = mouseAndViewport.second;
            IGraphObjectPickQuery *theQuery = theSubRenderer->GetGraphObjectPickQuery(this);
            if (theQuery) {
                QDemonRenderPickResult theInnerPickResult =
                        theQuery->Pick(theMouseCoords, theViewportDimensions, inPickEverything);
                if (theInnerPickResult.m_HitObject) {
                    thePickResult = theInnerPickResult;
                    thePickResult.m_OffscreenRenderer = theSubRenderer;
                    foundValidResult = true;
                    thePickResult.m_WasPickConsumed = true;
                } else if (GraphObjectTypes::IsNodeType(thePickResult.m_HitObject->m_Type)) {
                    const SNode *theNode =
                            static_cast<const SNode *>(thePickResult.m_HitObject);
                    if (theNode->m_Flags.IsGloballyPickable() == true) {
                        foundValidResult = true;
                        thePickResult.m_WasPickConsumed = true;
                    }
                }
            } else {
                // If the sub renderer doesn't consume the pick then we return the picked object
                // itself.  So no matter what, if we get to here the pick was consumed.
                thePickResult.m_WasPickConsumed = true;
                bool wasPickConsumed =
                        theSubRenderer->Pick(theMouseCoords, theViewportDimensions, this);
                if (wasPickConsumed) {
                    thePickResult.m_HitObject = nullptr;
                    foundValidResult = true;
                }
            }
        } else {
            foundValidResult = true;
            thePickResult.m_WasPickConsumed = true;
        }
    }
    return thePickResult;
}

QDemonRenderPickResult QDemonRendererImpl::Pick(SLayer &inLayer, const QVector2D &inViewportDimensions,
                                                const QVector2D &inMouseCoords, bool inPickSiblings,
                                                bool inPickEverything, const SRenderInstanceId id)
{
    m_LastPickResults.clear();

    SLayer *theLayer = &inLayer;
    // Stepping through how the original runtime did picking it picked layers in order
    // stopping at the first hit.  So objects on the top layer had first crack at the pick
    // vector itself.
    do {
        if (theLayer->m_Flags.IsActive()) {
            TInstanceRenderMap::iterator theIter
                    = m_InstanceRenderMap.find(combineLayerAndId(theLayer, id));
            if (theIter != m_InstanceRenderMap.end()) {
                m_LastPickResults.clear();
                GetLayerHitObjectList(*theIter.value(), inViewportDimensions, inMouseCoords,
                                      inPickEverything, m_LastPickResults);
                SPickResultProcessResult retval(ProcessPickResultList(inPickEverything));
                if (retval.m_WasPickConsumed)
                    return retval;
            } else {
                // Q_ASSERT( false );
            }
        }

        if (inPickSiblings)
            theLayer = GetNextLayer(*theLayer);
        else
            theLayer = nullptr;
    } while (theLayer != nullptr);

    return QDemonRenderPickResult();
}

static inline QDemonOption<QVector2D> IntersectRayWithNode(const SNode &inNode,
                                                           SRenderableObject &inRenderableObject,
                                                           const SRay &inPickRay)
{
    if (inRenderableObject.m_RenderableFlags.IsText()) {
        STextRenderable &theRenderable = static_cast<STextRenderable &>(inRenderableObject);
        if (&theRenderable.m_Text == &inNode)
            return inPickRay.GetRelativeXY(inRenderableObject.m_GlobalTransform,
                                           inRenderableObject.m_Bounds);
    } else if (inRenderableObject.m_RenderableFlags.IsDefaultMaterialMeshSubset()) {
        SSubsetRenderable &theRenderable = static_cast<SSubsetRenderable &>(inRenderableObject);
        if (&theRenderable.m_ModelContext.m_Model == &inNode)
            return inPickRay.GetRelativeXY(inRenderableObject.m_GlobalTransform,
                                           inRenderableObject.m_Bounds);
    } else if (inRenderableObject.m_RenderableFlags.IsCustomMaterialMeshSubset()) {
        SCustomMaterialRenderable &theRenderable =
                static_cast<SCustomMaterialRenderable &>(inRenderableObject);
        if (&theRenderable.m_ModelContext.m_Model == &inNode)
            return inPickRay.GetRelativeXY(inRenderableObject.m_GlobalTransform,
                                           inRenderableObject.m_Bounds);
    } else {
        Q_ASSERT(false);
    }
    return QDemonEmpty();
}

static inline QDemonRenderPickSubResult ConstructSubResult(SImage &inImage)
{
    STextureDetails theDetails = inImage.m_TextureData.m_Texture->GetTextureDetails();
    return QDemonRenderPickSubResult(inImage.m_LastFrameOffscreenRenderer,
                                     inImage.m_TextureTransform, inImage.m_HorizontalTilingMode,
                                     inImage.m_VerticalTilingMode, theDetails.m_Width,
                                     theDetails.m_Height);
}

QDemonOption<QVector2D> QDemonRendererImpl::FacePosition(SNode &inNode, QDemonBounds3 inBounds,
                                                         const QMatrix4x4 &inGlobalTransform,
                                                         const QVector2D &inViewportDimensions,
                                                         const QVector2D &inMouseCoords,
                                                         QDemonDataRef<SGraphObject *> inMapperObjects,
                                                         SBasisPlanes::Enum inPlane)
{
    QSharedPointer<SLayerRenderData> theLayerData = GetOrCreateLayerRenderDataForNode(inNode);
    if (theLayerData == nullptr)
        return QDemonEmpty();
    // This function assumes the layer was rendered to the scene itself.  There is another
    // function
    // for completely offscreen layers that don't get rendered to the scene.
    bool wasRenderToTarget(theLayerData->m_Layer.m_Flags.IsLayerRenderToTarget());
    if (wasRenderToTarget == false || theLayerData->m_Camera == nullptr
            || theLayerData->m_LayerPrepResult.hasValue() == false
            || theLayerData->m_LastFrameOffscreenRenderer != nullptr)
        return QDemonEmpty();

    QVector2D theMouseCoords(inMouseCoords);
    QVector2D theViewportDimensions(inViewportDimensions);

    for (quint32 idx = 0, end = inMapperObjects.size(); idx < end; ++idx) {
        SGraphObject &currentObject = *inMapperObjects[idx];
        if (currentObject.m_Type == GraphObjectTypes::Layer) {
            // The layer knows its viewport so it can take the information directly.
            // This is extremely counter intuitive but a good sign.
        } else if (currentObject.m_Type == GraphObjectTypes::Image) {
            SImage &theImage = static_cast<SImage &>(currentObject);
            SModel *theParentModel = nullptr;
            if (theImage.m_Parent
                    && theImage.m_Parent->m_Type == GraphObjectTypes::DefaultMaterial) {
                SDefaultMaterial *theMaterial =
                        static_cast<SDefaultMaterial *>(theImage.m_Parent);
                if (theMaterial) {
                    theParentModel = theMaterial->m_Parent;
                }
            }
            if (theParentModel == nullptr) {
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            QDemonBounds3 theModelBounds = theParentModel->GetBounds(
                        GetDemonContext()->GetBufferManager(), GetDemonContext()->GetPathManager(), false);

            if (theModelBounds.isEmpty()) {
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            QDemonOption<QVector2D> relativeHit =
                    FacePosition(*theParentModel, theModelBounds, theParentModel->m_GlobalTransform,
                                 theViewportDimensions, theMouseCoords, QDemonDataRef<SGraphObject *>(),
                                 SBasisPlanes::XY);
            if (relativeHit.isEmpty()) {
                return QDemonEmpty();
            }
            QDemonRenderPickSubResult theResult = ConstructSubResult(theImage);
            QVector2D hitInUVSpace = (*relativeHit) + QVector2D(.5f, .5f);
            QPair<QVector2D, QVector2D> mouseAndViewport =
                    GetMouseCoordsAndViewportFromSubObject(hitInUVSpace, theResult);
            theMouseCoords = mouseAndViewport.first;
            theViewportDimensions = mouseAndViewport.second;
        }
    }

    QDemonOption<SRay> theHitRay = theLayerData->m_LayerPrepResult->GetPickRay(
                theMouseCoords, theViewportDimensions, false);
    if (theHitRay.hasValue() == false)
        return QDemonEmpty();

    // Scale the mouse coords to change them into the camera's numerical space.
    SRay thePickRay = *theHitRay;
    QDemonOption<QVector2D> newValue = thePickRay.GetRelative(inGlobalTransform, inBounds, inPlane);
    return newValue;
}

QDemonRenderPickResult
QDemonRendererImpl::PickOffscreenLayer(SLayer &/*inLayer*/, const QVector2D & /*inViewportDimensions*/
                                       ,
                                       const QVector2D & /*inMouseCoords*/
                                       ,
                                       bool /*inPickEverything*/)
{
    return QDemonRenderPickResult();
}

QVector3D QDemonRendererImpl::UnprojectToPosition(SNode &inNode, QVector3D &inPosition,
                                                  const QVector2D &inMouseVec) const
{
    // Translate mouse into layer's coordinates
    QSharedPointer<SLayerRenderData> theData =
            const_cast<QDemonRendererImpl &>(*this).GetOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        return QVector3D(0, 0, 0);
    } // Q_ASSERT( false ); return QVector3D(0,0,0); }

    QSize theWindow = m_demonContext->GetWindowDimensions();
    QVector2D theDims((float)theWindow.width(), (float)theWindow.height());

    SLayerRenderPreparationResult &thePrepResult(*theData->m_LayerPrepResult);
    SRay theRay = thePrepResult.GetPickRay(inMouseVec, theDims, true);

    return theData->m_Camera->UnprojectToPosition(inPosition, theRay);
}

QVector3D QDemonRendererImpl::UnprojectWithDepth(SNode &inNode, QVector3D &,
                                                 const QVector3D &inMouseVec) const
{
    // Translate mouse into layer's coordinates
    QSharedPointer<SLayerRenderData> theData =
            const_cast<QDemonRendererImpl &>(*this).GetOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        return QVector3D(0, 0, 0);
    } // Q_ASSERT( false ); return QVector3D(0,0,0); }

    // Flip the y into gl coordinates from window coordinates.
    QVector2D theMouse(inMouseVec.x(), inMouseVec.y());
    float theDepth = inMouseVec.z();

    SLayerRenderPreparationResult &thePrepResult(*theData->m_LayerPrepResult);
    QSize theWindow = m_demonContext->GetWindowDimensions();
    SRay theRay = thePrepResult.GetPickRay(
                theMouse, QVector2D((float)theWindow.width(), (float)theWindow.height()), true);
    QVector3D theTargetPosition = theRay.m_Origin + theRay.m_Direction * theDepth;
    if (inNode.m_Parent != nullptr && inNode.m_Parent->m_Type != GraphObjectTypes::Layer)
        theTargetPosition =
                mat44::transform(mat44::getInverse(inNode.m_Parent->m_GlobalTransform), theTargetPosition);
    // Our default global space is right handed, so if you are left handed z means something
    // opposite.
    if (inNode.m_Flags.IsLeftHanded())
        theTargetPosition.setZ(theTargetPosition.z() * -1);
    return theTargetPosition;
}

QVector3D QDemonRendererImpl::ProjectPosition(SNode &inNode, const QVector3D &inPosition) const
{
    // Translate mouse into layer's coordinates
    QSharedPointer<SLayerRenderData> theData =
            const_cast<QDemonRendererImpl &>(*this).GetOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        return QVector3D(0, 0, 0);
    }

    QMatrix4x4 viewProj;
    theData->m_Camera->CalculateViewProjectionMatrix(viewProj);
    QVector4D projPos = mat44::transform(viewProj, QVector4D(inPosition, 1.0f));
    projPos.setX(projPos.x() / projPos.w());
    projPos.setY(projPos.y() / projPos.w());

    QDemonRenderRectF theViewport = theData->m_LayerPrepResult->GetLayerToPresentationViewport();
    QVector2D theDims((float)theViewport.m_Width, (float)theViewport.m_Height);
    projPos.setX(projPos.x() + 1.0);
    projPos.setY(projPos.y() + 1.0);
    projPos.setX(projPos.x() * 0.5);
    projPos.setY(projPos.y() * 0.5);
    QVector3D cameraToObject = theData->m_Camera->GetGlobalPos() - inPosition;
    projPos.setZ(sqrtf(QVector3D::dotProduct(cameraToObject, cameraToObject)));
    QVector3D mouseVec = QVector3D(projPos.x(), projPos.y(), projPos.z());
    mouseVec.setX(mouseVec.x() * theDims.x());
    mouseVec.setY(mouseVec.y() * theDims.y());

    mouseVec.setX(mouseVec.x() + theViewport.m_X);
    mouseVec.setY(mouseVec.y() + theViewport.m_Y);

    // Flip the y into window coordinates so it matches the mouse.
    QSize theWindow = m_demonContext->GetWindowDimensions();
    mouseVec.setY(theWindow.height() - mouseVec.y());

    return mouseVec;
}

QDemonOption<SLayerPickSetup> QDemonRendererImpl::GetLayerPickSetup(SLayer &inLayer,
                                                                    const QVector2D &inMouseCoords,
                                                                    const QSize &inPickDims)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inLayer);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        Q_ASSERT(false);
        return QDemonEmpty();
    }
    QSize theWindow = m_demonContext->GetWindowDimensions();
    QVector2D theDims((float)theWindow.width(), (float)theWindow.height());
    // The mouse is relative to the layer
    QDemonOption<QVector2D> theLocalMouse = GetLayerMouseCoords(*theData, inMouseCoords, theDims, false);
    if (theLocalMouse.hasValue() == false) {
        return QDemonEmpty();
    }

    SLayerRenderPreparationResult &thePrepResult(*theData->m_LayerPrepResult);
    if (thePrepResult.GetCamera() == nullptr) {
        return QDemonEmpty();
    }
    // Perform gluPickMatrix and pre-multiply it into the view projection
    QMatrix4x4 theTransScale;
    SCamera &theCamera(*thePrepResult.GetCamera());

    QDemonRenderRectF layerToPresentation = thePrepResult.GetLayerToPresentationViewport();
    // Offsetting is already taken care of in the camera's projection.
    // All we need to do is to scale and translate the image.
    layerToPresentation.m_X = 0;
    layerToPresentation.m_Y = 0;
    QVector2D theMouse(*theLocalMouse);
    // The viewport will need to center at this location
    QVector2D viewportDims((float)inPickDims.width(), (float)inPickDims.height());
    QVector2D bottomLeft =
            QVector2D(theMouse.x() - viewportDims.x() / 2.0f, theMouse.y() - viewportDims.y() / 2.0f);
    // For some reason, and I haven't figured out why yet, the offsets need to be backwards for
    // this to work.
    // bottomLeft.x = layerToPresentation.m_Width - bottomLeft.x;
    // bottomLeft.y = layerToPresentation.m_Height - bottomLeft.y;
    // Virtual rect is relative to the layer.
    QDemonRenderRectF thePickRect(bottomLeft.x(), bottomLeft.y(), viewportDims.x(), viewportDims.y());
    QMatrix4x4 projectionPremult;
    projectionPremult = QDemonRenderContext::ApplyVirtualViewportToProjectionMatrix(
                projectionPremult, layerToPresentation, thePickRect);
    projectionPremult = mat44::getInverse(projectionPremult);

    QMatrix4x4 globalInverse = mat44::getInverse(theCamera.m_GlobalTransform);
    QMatrix4x4 theVP = theCamera.m_Projection * globalInverse;
    // For now we won't setup the scissor, so we may be off by inPickDims at most because
    // GetLayerMouseCoords will return
    // false if the mouse is too far off the layer.
    return SLayerPickSetup(projectionPremult, theVP,
                           QDemonRenderRect(0, 0, (quint32)layerToPresentation.m_Width,
                                            (quint32)layerToPresentation.m_Height));
}

QDemonOption<QDemonRenderRectF> QDemonRendererImpl::GetLayerRect(SLayer &inLayer)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inLayer);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        Q_ASSERT(false);
        return QDemonEmpty();
    }
    SLayerRenderPreparationResult &thePrepResult(*theData->m_LayerPrepResult);
    return thePrepResult.GetLayerToPresentationViewport();
}

// This doesn't have to be cheap.
void QDemonRendererImpl::RunLayerRender(SLayer &inLayer, const QMatrix4x4 &inViewProjection)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inLayer);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        Q_ASSERT(false);
        return;
    }
    theData->PrepareAndRender(inViewProjection);
}

void QDemonRendererImpl::AddRenderWidget(IRenderWidget &inWidget)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inWidget.GetNode());
    if (theData)
        theData->AddRenderWidget(inWidget);
}

void QDemonRendererImpl::RenderLayerRect(SLayer &inLayer, const QVector3D &inColor)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inLayer);
    if (theData)
        theData->m_BoundingRectColor = inColor;
}

SScaleAndPosition QDemonRendererImpl::GetWorldToPixelScaleFactor(const SCamera &inCamera,
                                                                 const QVector3D &inWorldPoint,
                                                                 SLayerRenderData &inRenderData)
{
    if (inCamera.m_Flags.IsOrthographic() == true) {
        // There are situations where the camera can scale.
        return SScaleAndPosition(
                    inWorldPoint,
                    inCamera.GetOrthographicScaleFactor(
                        inRenderData.m_LayerPrepResult->GetLayerToPresentationViewport(),
                        inRenderData.m_LayerPrepResult->GetPresentationDesignDimensions()));
    } else {
        QVector3D theCameraPos(0, 0, 0);
        QVector3D theCameraDir(0, 0, -1);
        SRay theRay(theCameraPos, inWorldPoint - theCameraPos);
        QDemonPlane thePlane(theCameraDir, -600);
        QVector3D theItemPosition(inWorldPoint);
        QDemonOption<QVector3D> theIntersection = theRay.Intersect(thePlane);
        if (theIntersection.hasValue())
            theItemPosition = *theIntersection;
        // The special number comes in from physically measuring how off we are on the screen.
        float theScaleFactor = (1.0f / inCamera.m_Projection(1, 1));
        QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inCamera);
        quint32 theHeight = theData->m_LayerPrepResult->GetTextureDimensions().height();
        float theScaleMultiplier = 600.0f / ((float)theHeight / 2.0f);
        theScaleFactor *= theScaleMultiplier;

        return SScaleAndPosition(theItemPosition, theScaleFactor);
    }
}

SScaleAndPosition QDemonRendererImpl::GetWorldToPixelScaleFactor(SLayer &inLayer,
                                                                 const QVector3D &inWorldPoint)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inLayer);
    if (theData == nullptr || theData->m_Camera == nullptr) {
        Q_ASSERT(false);
        return SScaleAndPosition();
    }
    return GetWorldToPixelScaleFactor(*theData->m_Camera, inWorldPoint, *theData);
}

void QDemonRendererImpl::ReleaseLayerRenderResources(SLayer &inLayer, const SRenderInstanceId id)
{
    TInstanceRenderMap::iterator theIter
            = m_InstanceRenderMap.find(combineLayerAndId(&inLayer, id));
    if (theIter != m_InstanceRenderMap.end()) {
        TLayerRenderList::iterator theLastFrm = std::find(
                    m_LastFrameLayers.begin(), m_LastFrameLayers.end(), theIter.value());
        if (theLastFrm != m_LastFrameLayers.end()) {
            theIter.value()->ResetForFrame();
            m_LastFrameLayers.erase(theLastFrm);
        }
        m_InstanceRenderMap.erase(theIter);
    }
}

void QDemonRendererImpl::RenderQuad(const QVector2D inDimensions, const QMatrix4x4 &inMVP,
                                    QDemonRenderTexture2D &inQuadTexture)
{
    m_Context->SetCullingEnabled(false);
    QSharedPointer<SLayerSceneShader> theShader = GetSceneLayerShader();
    QDemonRenderContext &theContext(*m_Context);
    theContext.SetActiveShader(theShader->m_Shader);
    theShader->m_MVP.Set(inMVP);
    theShader->m_Dimensions.Set(inDimensions);
    theShader->m_Sampler.Set(&inQuadTexture);

    GenerateXYQuad();
    theContext.SetInputAssembler(m_QuadInputAssembler);
    theContext.Draw(QDemonRenderDrawMode::Triangles, m_QuadIndexBuffer->GetNumIndices(), 0);
}

void QDemonRendererImpl::RenderQuad()
{
    m_Context->SetCullingEnabled(false);
    GenerateXYQuad();
    m_Context->SetInputAssembler(m_QuadInputAssembler);
    m_Context->Draw(QDemonRenderDrawMode::Triangles, m_QuadIndexBuffer->GetNumIndices(), 0);
}

void QDemonRendererImpl::RenderPointsIndirect()
{
    m_Context->SetCullingEnabled(false);
    GenerateXYZPoint();
    m_Context->SetInputAssembler(m_PointInputAssembler);
    m_Context->DrawIndirect(QDemonRenderDrawMode::Points, 0);
}

void QDemonRendererImpl::LayerNeedsFrameClear(SLayerRenderData &inLayer)
{
    m_LastFrameLayers.push_back(&inLayer);
}

void QDemonRendererImpl::BeginLayerDepthPassRender(SLayerRenderData &inLayer)
{
    m_CurrentLayer = &inLayer;
}

void QDemonRendererImpl::EndLayerDepthPassRender() { m_CurrentLayer = nullptr; }

void QDemonRendererImpl::BeginLayerRender(SLayerRenderData &inLayer)
{
    m_CurrentLayer = &inLayer;
    // Remove all of the shaders from the layer shader set
    // so that we can only apply the camera and lighting properties to
    // shaders that are in the layer.
    m_LayerShaders.clear();
}
void QDemonRendererImpl::EndLayerRender() { m_CurrentLayer = nullptr; }

// Allocate an object that lasts only this frame.
#define RENDER_FRAME_NEW(type)                                                                     \
    new (m_PerFrameAllocator.m_FastAllocator.allocate(sizeof(type), __FILE__, __LINE__)) type

void QDemonRendererImpl::PrepareImageForIbl(SImage &inImage)
{
    if (inImage.m_TextureData.m_Texture && inImage.m_TextureData.m_Texture->GetNumMipmaps() < 1)
        inImage.m_TextureData.m_Texture->GenerateMipmaps();
}

bool NodeContainsBoneRoot(SNode &childNode, qint32 rootID)
{
    for (SNode *childChild = childNode.m_FirstChild; childChild != nullptr;
         childChild = childChild->m_NextSibling) {
        if (childChild->m_SkeletonId == rootID)
            return true;
    }

    return false;
}

void FillBoneIdNodeMap(SNode &childNode, QHash<long, SNode *> &ioMap)
{
    if (childNode.m_SkeletonId >= 0)
        ioMap[childNode.m_SkeletonId] = &childNode;
    for (SNode *childChild = childNode.m_FirstChild; childChild != nullptr;
         childChild = childChild->m_NextSibling)
        FillBoneIdNodeMap(*childChild, ioMap);
}

bool QDemonRendererImpl::PrepareTextureAtlasForRender()
{
    QSharedPointer<ITextTextureAtlas> theTextureAtlas = m_demonContext->GetTextureAtlas();
    if (theTextureAtlas == nullptr)
        return false;

    // this is a one time creation
    if (!theTextureAtlas->IsInitialized()) {
        QDemonRenderContext &theContext(*m_Context);
        QSharedPointer<QDemonRenderVertexBuffer> mVertexBuffer;
        QSharedPointer<QDemonRenderInputAssembler> mInputAssembler;
        QSharedPointer<QDemonRenderAttribLayout> mAttribLayout;
        // temporay FB
        QDemonRenderContextScopedProperty<QSharedPointer<QDemonRenderFrameBuffer>> __fbo(
                    *m_Context, &QDemonRenderContext::GetRenderTarget, &QDemonRenderContext::SetRenderTarget);

        ITextRenderer &theTextRenderer(*m_demonContext->GetOnscreenTextRenderer());
        TTextTextureAtlasDetailsAndTexture theResult = theTextureAtlas->PrepareTextureAtlas();
        if (!theResult.first.m_EntryCount) {
            Q_ASSERT(theResult.first.m_EntryCount);
            return false;
        }

        // generate the index buffer we need
        GenerateXYQuad();

        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos",
            QDemonRenderComponentTypes::Float32, 3),
            QDemonRenderVertexBufferEntry(
            "attr_uv", QDemonRenderComponentTypes::Float32, 2, 12),
        };

        // create our attribute layout
        mAttribLayout = m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 2));

        QSharedPointer<QDemonRenderFrameBuffer> theAtlasFB(
                    m_demonContext->GetResourceManager()->AllocateFrameBuffer());
        theAtlasFB->Attach(QDemonRenderFrameBufferAttachments::Color0, theResult.second);
        m_demonContext->GetRenderContext()->SetRenderTarget(theAtlasFB);

        // this texture contains our single entries
        QSharedPointer<QDemonRenderTexture2D> theTexture = nullptr;
        if (m_Context->GetRenderContextType() == QDemonRenderContextValues::GLES2) {
            theTexture = m_demonContext->GetResourceManager()
                    ->AllocateTexture2D(32, 32, QDemonRenderTextureFormats::RGBA8);
        } else {
            theTexture = m_demonContext->GetResourceManager()
                    ->AllocateTexture2D(32, 32, QDemonRenderTextureFormats::Alpha8);
        }
        m_Context->SetClearColor(QVector4D(0, 0, 0, 0));
        m_Context->Clear(QDemonRenderClearValues::Color);
        m_Context->SetDepthTestEnabled(false);
        m_Context->SetScissorTestEnabled(false);
        m_Context->SetCullingEnabled(false);
        m_Context->SetBlendingEnabled(false);
        m_Context->SetViewport(
                    QDemonRenderRect(0, 0, theResult.first.m_TextWidth, theResult.first.m_TextHeight));

        SCamera theCamera;
        theCamera.m_ClipNear = -1.0;
        theCamera.m_ClipFar = 1.0;
        theCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
        theCamera.m_Flags.SetOrthographic(true);
        QVector2D theTextureDims((float)theResult.first.m_TextWidth,
                                 (float)theResult.first.m_TextHeight);
        theCamera.CalculateGlobalVariables(
                    QDemonRenderRect(0, 0, theResult.first.m_TextWidth, theResult.first.m_TextHeight),
                    theTextureDims);
        // We want a 2D lower left projection
        float *writePtr(theCamera.m_Projection.data());
        writePtr[12] = -1;
        writePtr[13] = -1;

        // generate render stuff
        // We dynamicall update the vertex buffer
        float tempBuf[20];
        float *bufPtr = tempBuf;
        quint32 bufSize = 20 * sizeof(float); // 4 vertices  3 pos 2 tex
        QDemonDataRef<quint8> vertData((quint8 *)bufPtr, bufSize);
        mVertexBuffer = theContext.CreateVertexBuffer(
                    QDemonRenderBufferUsageType::Dynamic, 20 * sizeof(float),
                    3 * sizeof(float) + 2 * sizeof(float), vertData);
        quint32 strides = mVertexBuffer->GetStride();
        quint32 offsets = 0;
        mInputAssembler = theContext.CreateInputAssembler(
                    mAttribLayout, toConstDataRef(&mVertexBuffer, 1), m_QuadIndexBuffer,
                    toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));

        QSharedPointer<QDemonRenderShaderProgram> theShader = GetTextAtlasEntryShader();
        STextShader theTextShader(theShader);

        if (theShader) {
            theContext.SetActiveShader(theShader);
            theTextShader.m_MVP.Set(theCamera.m_Projection);

            // we are going through all entries and render to the FBO
            for (quint32 i = 0; i < theResult.first.m_EntryCount; i++) {
                STextTextureAtlasEntryDetails theDetails =
                        theTextRenderer.RenderAtlasEntry(i, *theTexture);
                // update vbo
                // we need to mirror coordinates
                float x1 = (float)theDetails.m_X;
                float x2 = (float)theDetails.m_X + theDetails.m_TextWidth;
                float y1 = (float)theDetails.m_Y;
                float y2 = (float)theDetails.m_Y + theDetails.m_TextHeight;

                float box[4][5] = {
                    { x1, y1, 0, 0, 1 },
                    { x1, y2, 0, 0, 0 },
                    { x2, y2, 0, 1, 0 },
                    { x2, y1, 0, 1, 1 },
                };

                QDemonDataRef<quint8> vertData((quint8 *)box, bufSize);
                mVertexBuffer->UpdateBuffer(vertData, false);

                theTextShader.m_Sampler.Set(theTexture.data());

                theContext.SetInputAssembler(mInputAssembler);
                theContext.Draw(QDemonRenderDrawMode::Triangles, m_QuadIndexBuffer->GetNumIndices(),
                                0);
            }
        }

        m_demonContext->GetResourceManager()->Release(theTexture);
        m_demonContext->GetResourceManager()->Release(theAtlasFB);

        return true;
    }

    return theTextureAtlas->IsInitialized();
}

QDemonOption<QVector2D> QDemonRendererImpl::GetLayerMouseCoords(SLayerRenderData &inLayerRenderData,
                                                                const QVector2D &inMouseCoords,
                                                                const QVector2D &inViewportDimensions,
                                                                bool forceImageIntersect) const
{
    if (inLayerRenderData.m_LayerPrepResult.hasValue())
        return inLayerRenderData.m_LayerPrepResult->GetLayerMouseCoords(
                    inMouseCoords, inViewportDimensions, forceImageIntersect);
    return QDemonEmpty();
}

void QDemonRendererImpl::GetLayerHitObjectList(SLayerRenderData &inLayerRenderData,
                                               const QVector2D &inViewportDimensions,
                                               const QVector2D &inPresCoords, bool inPickEverything,
                                               TPickResultArray &outIntersectionResult)
{
    // This function assumes the layer was rendered to the scene itself.  There is another
    // function
    // for completely offscreen layers that don't get rendered to the scene.
    bool wasRenderToTarget(inLayerRenderData.m_Layer.m_Flags.IsLayerRenderToTarget());
    if (wasRenderToTarget && inLayerRenderData.m_Camera != nullptr) {
        QDemonOption<SRay> theHitRay;
        if (inLayerRenderData.m_LayerPrepResult.hasValue())
            theHitRay = inLayerRenderData.m_LayerPrepResult->GetPickRay(
                        inPresCoords, inViewportDimensions, false);
        if (inLayerRenderData.m_LastFrameOffscreenRenderer == nullptr) {
            if (theHitRay.hasValue()) {
                // Scale the mouse coords to change them into the camera's numerical space.
                SRay thePickRay = *theHitRay;
                for (quint32 idx = inLayerRenderData.m_OpaqueObjects.size(), end = 0; idx > end;
                     --idx) {
                    SRenderableObject *theRenderableObject =
                            inLayerRenderData.m_OpaqueObjects[idx - 1];
                    if (inPickEverything
                            || theRenderableObject->m_RenderableFlags.GetPickable())
                        IntersectRayWithSubsetRenderable(thePickRay, *theRenderableObject,
                                                         outIntersectionResult);
                }
                for (quint32 idx = inLayerRenderData.m_TransparentObjects.size(), end = 0;
                     idx > end; --idx) {
                    SRenderableObject *theRenderableObject =
                            inLayerRenderData.m_TransparentObjects[idx - 1];
                    if (inPickEverything
                            || theRenderableObject->m_RenderableFlags.GetPickable())
                        IntersectRayWithSubsetRenderable(thePickRay, *theRenderableObject,
                                                         outIntersectionResult);
                }
            }
        } else {
            IGraphObjectPickQuery *theQuery =
                    inLayerRenderData.m_LastFrameOffscreenRenderer->GetGraphObjectPickQuery(this);
            if (theQuery) {
                QDemonRenderPickResult theResult =
                        theQuery->Pick(inPresCoords, inViewportDimensions, inPickEverything);
                if (theResult.m_HitObject) {
                    theResult.m_OffscreenRenderer =
                            inLayerRenderData.m_LastFrameOffscreenRenderer;
                    outIntersectionResult.push_back(theResult);
                }
            } else
                inLayerRenderData.m_LastFrameOffscreenRenderer->Pick(inPresCoords,
                                                                     inViewportDimensions,
                                                                     this);
        }
    }
}

static inline QDemonRenderPickSubResult ConstructSubResult(SRenderableImage &inImage)
{
    return ConstructSubResult(inImage.m_Image);
}

void QDemonRendererImpl::IntersectRayWithSubsetRenderable(
        const SRay &inRay, SRenderableObject &inRenderableObject,
        TPickResultArray &outIntersectionResultList)
{
    QDemonOption<SRayIntersectionResult> theIntersectionResultOpt(inRay.IntersectWithAABB(
                                                                      inRenderableObject.m_GlobalTransform, inRenderableObject.m_Bounds));
    if (theIntersectionResultOpt.hasValue() == false)
        return;
    SRayIntersectionResult &theResult(*theIntersectionResultOpt);

    // Leave the coordinates relative for right now.
    const SGraphObject *thePickObject = nullptr;
    if (inRenderableObject.m_RenderableFlags.IsDefaultMaterialMeshSubset())
        thePickObject = &static_cast<SSubsetRenderable *>(&inRenderableObject)->m_ModelContext.m_Model;
    else if (inRenderableObject.m_RenderableFlags.IsText())
        thePickObject = &static_cast<STextRenderable *>(&inRenderableObject)->m_Text;
    else if (inRenderableObject.m_RenderableFlags.IsCustomMaterialMeshSubset())
        thePickObject = &static_cast<SCustomMaterialRenderable *>(&inRenderableObject)->m_ModelContext.m_Model;
    else if (inRenderableObject.m_RenderableFlags.IsPath())
        thePickObject = &static_cast<SPathRenderable *>(&inRenderableObject)->m_Path;

    if (thePickObject != nullptr) {
        outIntersectionResultList.push_back(QDemonRenderPickResult(
                                                *thePickObject, theResult.m_RayLengthSquared, theResult.m_RelXY));

        // For subsets, we know we can find images on them which may have been the result
        // of rendering a sub-presentation.
        if (inRenderableObject.m_RenderableFlags.IsDefaultMaterialMeshSubset()) {
            QDemonRenderPickSubResult *theLastResult = nullptr;
            for (SRenderableImage *theImage =
                 static_cast<SSubsetRenderable *>(&inRenderableObject)->m_FirstImage;
                 theImage != nullptr; theImage = theImage->m_NextImage) {
                if (theImage->m_Image.m_LastFrameOffscreenRenderer != nullptr
                        && theImage->m_Image.m_TextureData.m_Texture != nullptr) {
                    QDemonRenderPickSubResult *theSubResult = new QDemonRenderPickSubResult(ConstructSubResult(*theImage));
                    if (theLastResult == nullptr)
                        outIntersectionResultList.back().m_FirstSubObject = theSubResult;
                    else
                        theLastResult->m_NextSibling = theSubResult;
                    theLastResult = theSubResult;
                }
            }
        }
    }
}

QSharedPointer<QDemonRenderShaderProgram> QDemonRendererImpl::CompileShader(const QString &inName,
                                                                            const char *inVert,
                                                                            const char *inFrag)
{
    GetProgramGenerator()->BeginProgram();
    GetProgramGenerator()->GetStage(ShaderGeneratorStages::Vertex)->Append(inVert);
    GetProgramGenerator()->GetStage(ShaderGeneratorStages::Fragment)->Append(inFrag);
    return GetProgramGenerator()->CompileGeneratedShader(inName);
}

const float MINATTENUATION = 0;
const float MAXATTENUATION = 1000;

float ClampFloat(float value, float min, float max)
{
    return value < min ? min : ((value > max) ? max : value);
}

float TranslateConstantAttenuation(float attenuation) { return attenuation * .01f; }

float TranslateLinearAttenuation(float attenuation)
{
    attenuation = ClampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0001f;
}

float TranslateQuadraticAttenuation(float attenuation)
{
    attenuation = ClampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0000001f;
}

QSharedPointer<SShaderGeneratorGeneratedShader> QDemonRendererImpl::GetShader(SSubsetRenderable &inRenderable,
                                                                              TShaderFeatureSet inFeatureSet)
{
    if (m_CurrentLayer == nullptr) {
        Q_ASSERT(false);
        return nullptr;
    }
    TShaderMap::iterator theFind = m_Shaders.find(inRenderable.m_ShaderDescription);
    QSharedPointer<SShaderGeneratorGeneratedShader> retval = nullptr;
    if (theFind == m_Shaders.end()) {
        // Generate the shader.
        QSharedPointer<QDemonRenderShaderProgram> theShader(GenerateShader(inRenderable, inFeatureSet));
        if (theShader) {
            QSharedPointer<SShaderGeneratorGeneratedShader> theGeneratedShader = QSharedPointer<SShaderGeneratorGeneratedShader>(new SShaderGeneratorGeneratedShader(
                                                                                                                                     m_GeneratedShaderString, theShader));
            m_Shaders.insert(inRenderable.m_ShaderDescription, theGeneratedShader);
            retval = theGeneratedShader;
        }
        // We still insert something because we don't to attempt to generate the same bad shader
        // twice.
        else
            m_Shaders.insert(inRenderable.m_ShaderDescription, nullptr);
    } else
        retval = theFind.value();

    if (retval != nullptr) {
        if (!m_LayerShaders.contains(*retval)) {
            m_LayerShaders.insert(*retval);
        }
        if (m_CurrentLayer && m_CurrentLayer->m_Camera) {
            SCamera &theCamera(*m_CurrentLayer->m_Camera);
            if (m_CurrentLayer->m_CameraDirection.hasValue() == false)
                m_CurrentLayer->m_CameraDirection = theCamera.GetScalingCorrectDirection();
        }
    }
    return retval;
}
static QVector3D g_fullScreenRectFace[] = {
    QVector3D(-1, -1, 0), QVector3D(-1, 1, 0), QVector3D(1, 1, 0), QVector3D(1, -1, 0),
};

static QVector2D g_fullScreenRectUVs[] = { QVector2D(0, 0), QVector2D(0, 1), QVector2D(1, 1),
                                           QVector2D(1, 0) };

void QDemonRendererImpl::GenerateXYQuad()
{
    if (m_QuadInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos",
        QDemonRenderComponentTypes::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv",
        QDemonRenderComponentTypes::Float32, 2, 12),
    };

    float tempBuf[20];
    float *bufPtr = tempBuf;
    QVector3D *facePtr(g_fullScreenRectFace);
    QVector2D *uvPtr(g_fullScreenRectUVs);
    for (int j = 0; j < 4; j++, ++facePtr, ++uvPtr, bufPtr += 5) {
        bufPtr[0] = facePtr->x();
        bufPtr[1] = facePtr->y();
        bufPtr[2] = facePtr->z();
        bufPtr[3] = uvPtr->x();
        bufPtr[4] = uvPtr->y();
    }
    m_QuadVertexBuffer = m_Context->CreateVertexBuffer(
                QDemonRenderBufferUsageType::Static, 20 * sizeof(float),
                3 * sizeof(float) + 2 * sizeof(float), toU8DataRef(tempBuf, 20));

    quint8 indexData[] = {
        0, 1, 2, 0, 2, 3,
    };
    m_QuadIndexBuffer = m_Context->CreateIndexBuffer(
                QDemonRenderBufferUsageType::Static, QDemonRenderComponentTypes::UnsignedInteger8,
                sizeof(indexData), toU8DataRef(indexData, sizeof(indexData)));

    // create our attribute layout
    m_QuadAttribLayout = m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 2));

    // create input assembler object
    quint32 strides = m_QuadVertexBuffer->GetStride();
    quint32 offsets = 0;
    m_QuadInputAssembler = m_Context->CreateInputAssembler(
                m_QuadAttribLayout, toConstDataRef(&m_QuadVertexBuffer, 1), m_QuadIndexBuffer,
                toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
}

void QDemonRendererImpl::GenerateXYZPoint()
{
    if (m_PointInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos",
        QDemonRenderComponentTypes::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv",
        QDemonRenderComponentTypes::Float32, 2, 12),
    };

    float tempBuf[5];
    tempBuf[0] = tempBuf[1] = tempBuf[2] = 0.0;
    tempBuf[3] = tempBuf[4] = 0.0;

    m_PointVertexBuffer = m_Context->CreateVertexBuffer(
                QDemonRenderBufferUsageType::Static, 5 * sizeof(float),
                3 * sizeof(float) + 2 * sizeof(float), toU8DataRef(tempBuf, 5));

    // create our attribute layout
    m_PointAttribLayout = m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 2));

    // create input assembler object
    quint32 strides = m_PointVertexBuffer->GetStride();
    quint32 offsets = 0;
    m_PointInputAssembler = m_Context->CreateInputAssembler(
                m_PointAttribLayout, toConstDataRef(&m_PointVertexBuffer, 1), nullptr,
                toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
}

QPair<QSharedPointer<QDemonRenderVertexBuffer>, QSharedPointer<QDemonRenderIndexBuffer >> QDemonRendererImpl::GetXYQuad()
{
    if (!m_QuadInputAssembler)
        GenerateXYQuad();

    return QPair<QSharedPointer<QDemonRenderVertexBuffer>, QSharedPointer<QDemonRenderIndexBuffer >>(m_QuadVertexBuffer, m_QuadIndexBuffer);
}

SLayerGlobalRenderProperties QDemonRendererImpl::GetLayerGlobalRenderProperties()
{
    SLayerRenderData &theData = *m_CurrentLayer;
    SLayer &theLayer = theData.m_Layer;
    if (theData.m_CameraDirection.hasValue() == false)
        theData.m_CameraDirection = theData.m_Camera->GetScalingCorrectDirection();

    return SLayerGlobalRenderProperties(
                theLayer, *theData.m_Camera, *theData.m_CameraDirection, theData.m_Lights,
                theData.m_LightDirections, theData.m_ShadowMapManager, theData.m_LayerDepthTexture,
                theData.m_LayerSsaoTexture, theLayer.m_LightProbe, theLayer.m_LightProbe2,
                theLayer.m_ProbeHorizon, theLayer.m_ProbeBright, theLayer.m_Probe2Window,
                theLayer.m_Probe2Pos, theLayer.m_Probe2Fade, theLayer.m_ProbeFov);
}

void QDemonRendererImpl::GenerateXYQuadStrip()
{
    if (m_QuadStripInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos",
        QDemonRenderComponentTypes::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv",
        QDemonRenderComponentTypes::Float32, 2, 12),
    };

    // this buffer is filled dynmically
    m_QuadStripVertexBuffer =
            m_Context->CreateVertexBuffer(QDemonRenderBufferUsageType::Dynamic, 0,
                                          3 * sizeof(float) + 2 * sizeof(float) // stride
                                          ,
                                          QDemonDataRef<quint8>());

    // create our attribute layout
    m_QuadStripAttribLayout = m_Context->CreateAttributeLayout(toConstDataRef(theEntries, 2));

    // create input assembler object
    quint32 strides = m_QuadStripVertexBuffer->GetStride();
    quint32 offsets = 0;
    m_QuadStripInputAssembler = m_Context->CreateInputAssembler(
                m_QuadStripAttribLayout, toConstDataRef(&m_QuadStripVertexBuffer, 1), nullptr,
                toConstDataRef(&strides, 1), toConstDataRef(&offsets, 1));
}

void QDemonRendererImpl::UpdateCbAoShadow(const SLayer *pLayer, const SCamera *pCamera,
                                          CResourceTexture2D &inDepthTexture)
{
    if (m_Context->GetConstantBufferSupport()) {
        QString theName = QString::fromLocal8Bit("cbAoShadow");
        QSharedPointer<QDemonRenderConstantBuffer> pCB = m_Context->GetConstantBuffer(theName);

        if (!pCB) {
            // the  size is determined automatically later on
            pCB = m_Context->CreateConstantBuffer(
                        theName.toLocal8Bit().constData(), QDemonRenderBufferUsageType::Static, 0, QDemonDataRef<quint8>());
            if (!pCB) {
                Q_ASSERT(false);
                return;
            }
            m_ConstantBuffers.insert(theName, pCB);

            // Add paramters. Note should match the appearance in the shader program
            pCB->AddParam(QString::fromLocal8Bit("ao_properties"),
                          QDemonRenderShaderDataTypes::Vec4, 1);
            pCB->AddParam(QString::fromLocal8Bit("ao_properties2"),
                          QDemonRenderShaderDataTypes::Vec4, 1);
            pCB->AddParam(QString::fromLocal8Bit("shadow_properties"),
                          QDemonRenderShaderDataTypes::Vec4, 1);
            pCB->AddParam(QString::fromLocal8Bit("aoScreenConst"),
                          QDemonRenderShaderDataTypes::Vec4, 1);
            pCB->AddParam(QString::fromLocal8Bit("UvToEyeConst"),
                          QDemonRenderShaderDataTypes::Vec4, 1);
        }

        // update values
        QVector4D aoProps(pLayer->m_AoStrength * 0.01f, pLayer->m_AoDistance * 0.4f,
                          pLayer->m_AoSoftness * 0.02f, pLayer->m_AoBias);
        pCB->UpdateParam("ao_properties", QDemonDataRef<quint8>((quint8 *)&aoProps, 1));
        QVector4D aoProps2((float)pLayer->m_AoSamplerate, (pLayer->m_AoDither) ? 1.0f : 0.0f, 0.0f,
                           0.0f);
        pCB->UpdateParam("ao_properties2", QDemonDataRef<quint8>((quint8 *)&aoProps2, 1));
        QVector4D shadowProps(pLayer->m_ShadowStrength * 0.01f, pLayer->m_ShadowDist,
                              pLayer->m_ShadowSoftness * 0.01f, pLayer->m_ShadowBias);
        pCB->UpdateParam("shadow_properties", QDemonDataRef<quint8>((quint8 *)&shadowProps, 1));

        float R2 = pLayer->m_AoDistance * pLayer->m_AoDistance * 0.16f;
        float rw = 100, rh = 100;

        if (inDepthTexture.GetTexture() && inDepthTexture.GetTexture()) {
            rw = (float)inDepthTexture.GetTexture()->GetTextureDetails().m_Width;
            rh = (float)inDepthTexture.GetTexture()->GetTextureDetails().m_Height;
        }
        float fov = (pCamera) ? pCamera->verticalFov(rw / rh) : 1.0f;
        float tanHalfFovY = tanf(0.5f * fov * (rh / rw));
        float invFocalLenX = tanHalfFovY * (rw / rh);

        QVector4D aoScreenConst(1.0f / R2, rh / (2.0f * tanHalfFovY), 1.0f / rw, 1.0f / rh);
        pCB->UpdateParam("aoScreenConst", QDemonDataRef<quint8>((quint8 *)&aoScreenConst, 1));
        QVector4D UvToEyeConst(2.0f * invFocalLenX, -2.0f * tanHalfFovY, -invFocalLenX,
                               tanHalfFovY);
        pCB->UpdateParam("UvToEyeConst", QDemonDataRef<quint8>((quint8 *)&UvToEyeConst, 1));

        // update buffer to hardware
        pCB->Update();
    }
}

// widget context implementation

QSharedPointer<QDemonRenderVertexBuffer> QDemonRendererImpl::GetOrCreateVertexBuffer(QString &inStr,
                                                                      quint32 stride,
                                                                      QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderVertexBuffer> retval = GetVertexBuffer(inStr);
    if (retval) {
        // we update the buffer
        retval->UpdateBuffer(bufferData, false);
        return retval;
    }
    retval = m_Context->CreateVertexBuffer(QDemonRenderBufferUsageType::Dynamic,
                                           bufferData.size(), stride, bufferData);
    m_WidgetVertexBuffers.insert(inStr, retval);
    return retval;
}
QSharedPointer<QDemonRenderIndexBuffer>
QDemonRendererImpl::GetOrCreateIndexBuffer(QString &inStr,
                                           QDemonRenderComponentTypes::Enum componentType,
                                           size_t size, QDemonConstDataRef<quint8> bufferData)
{
    QSharedPointer<QDemonRenderIndexBuffer> retval = GetIndexBuffer(inStr);
    if (retval) {
        // we update the buffer
        retval->UpdateBuffer(bufferData, false);
        return retval;
    }

    retval = m_Context->CreateIndexBuffer(QDemonRenderBufferUsageType::Dynamic,
                                          componentType, size, bufferData);
    m_WidgetIndexBuffers.insert(inStr, retval);
    return retval;
}

QSharedPointer<QDemonRenderAttribLayout> QDemonRendererImpl::CreateAttributeLayout(
        QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs)
{
    // create our attribute layout
    QSharedPointer<QDemonRenderAttribLayout> theAttribLayout = m_Context->CreateAttributeLayout(attribs);
    return theAttribLayout;
}

QSharedPointer<QDemonRenderInputAssembler> QDemonRendererImpl::GetOrCreateInputAssembler(
        QString &inStr, QSharedPointer<QDemonRenderAttribLayout> attribLayout,
        QDemonConstDataRef<QSharedPointer<QDemonRenderVertexBuffer>> buffers, const QSharedPointer<QDemonRenderIndexBuffer> indexBuffer,
        QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets)
{
    QSharedPointer<QDemonRenderInputAssembler> retval = GetInputAssembler(inStr);
    if (retval)
        return retval;

    retval =
            m_Context->CreateInputAssembler(attribLayout, buffers, indexBuffer, strides, offsets);
    m_WidgetInputAssembler.insert(inStr, retval);
    return retval;
}

QSharedPointer<QDemonRenderVertexBuffer> QDemonRendererImpl::GetVertexBuffer(const QString &inStr)
{
    TStrVertBufMap::iterator theIter = m_WidgetVertexBuffers.find(inStr);
    if (theIter != m_WidgetVertexBuffers.end())
        return theIter.value();
    return nullptr;
}

QSharedPointer<QDemonRenderIndexBuffer> QDemonRendererImpl::GetIndexBuffer(const QString &inStr)
{
    TStrIndexBufMap::iterator theIter = m_WidgetIndexBuffers.find(inStr);
    if (theIter != m_WidgetIndexBuffers.end())
        return theIter.value();
    return nullptr;
}

QSharedPointer<QDemonRenderInputAssembler> QDemonRendererImpl::GetInputAssembler(const QString &inStr)
{
    TStrIAMap::iterator theIter = m_WidgetInputAssembler.find(inStr);
    if (theIter != m_WidgetInputAssembler.end())
        return theIter.value();
    return nullptr;
}

QSharedPointer<QDemonRenderShaderProgram> QDemonRendererImpl::GetShader(const QString &inStr)
{
    TStrShaderMap::iterator theIter = m_WidgetShaders.find(inStr);
    if (theIter != m_WidgetShaders.end())
        return theIter.value();
    return nullptr;
}

QSharedPointer<QDemonRenderShaderProgram> QDemonRendererImpl::CompileAndStoreShader(const QString &inStr)
{
    QSharedPointer<QDemonRenderShaderProgram> newProgram = GetProgramGenerator()->CompileGeneratedShader(inStr);
    if (newProgram)
        m_WidgetShaders.insert(inStr, newProgram);
    return newProgram;
}

QSharedPointer<IShaderProgramGenerator> QDemonRendererImpl::GetProgramGenerator()
{
    return m_demonContext->GetShaderProgramGenerator();
}

STextDimensions QDemonRendererImpl::MeasureText(const STextRenderInfo &inText)
{
    if (m_demonContext->GetTextRenderer() != nullptr)
        return m_demonContext->GetTextRenderer()->MeasureText(inText, 0);
    return STextDimensions();
}

void QDemonRendererImpl::RenderText(const STextRenderInfo &inText, const QVector3D &inTextColor,
                                    const QVector3D &inBackgroundColor, const QMatrix4x4 &inMVP)
{
    if (m_demonContext->GetTextRenderer() != nullptr) {
        ITextRenderer &theTextRenderer(*m_demonContext->GetTextRenderer());
        QSharedPointer<QDemonRenderTexture2D> theTexture = m_demonContext->GetResourceManager()->AllocateTexture2D(
                    32, 32, QDemonRenderTextureFormats::RGBA8);
        STextTextureDetails theTextTextureDetails =
                theTextRenderer.RenderText(inText, *theTexture);
        STextRenderHelper theTextHelper(GetTextWidgetShader());
        if (theTextHelper.m_Shader != nullptr) {
            m_demonContext->GetRenderContext()->SetBlendingEnabled(false);
            STextScaleAndOffset theScaleAndOffset(*theTexture, theTextTextureDetails, inText);
            theTextHelper.m_Shader->Render(theTexture, theScaleAndOffset,
                                           QVector4D(inTextColor, 1.0f), inMVP, QVector2D(0, 0),
                                           GetContext(), theTextHelper.m_QuadInputAssembler,
                                           theTextHelper.m_QuadInputAssembler->GetIndexCount(),
                                           theTextTextureDetails, inBackgroundColor);
        }
        m_demonContext->GetResourceManager()->Release(theTexture);
    }
}

void QDemonRendererImpl::RenderText2D(float x, float y, QDemonOption<QVector3D> inColor, const QString &text)
{
    if (m_demonContext->GetOnscreenTextRenderer() != nullptr) {
        GenerateXYQuadStrip();

        if (PrepareTextureAtlasForRender()) {
            TTextRenderAtlasDetailsAndTexture theRenderTextDetails;
            QSharedPointer<ITextTextureAtlas> theTextureAtlas = m_demonContext->GetTextureAtlas();
            QSize theWindow = m_demonContext->GetWindowDimensions();

            STextRenderInfo theInfo;
            theInfo.m_Text = text;
            theInfo.m_FontSize = 20;
            // text scale 2% of screen we don't scale Y though because it becomes unreadable
            theInfo.m_ScaleX = (theWindow.width() / 100.0f) * 1.5f / (theInfo.m_FontSize);
            theInfo.m_ScaleY = 1.0f;

            theRenderTextDetails = theTextureAtlas->RenderText(theInfo);

            if (theRenderTextDetails.first.m_Vertices.size()) {
                STextRenderHelper theTextHelper(GetOnscreenTextShader());
                if (theTextHelper.m_Shader != nullptr) {
                    // setup 2D projection
                    SCamera theCamera;
                    theCamera.m_ClipNear = -1.0;
                    theCamera.m_ClipFar = 1.0;

                    theCamera.MarkDirty(NodeTransformDirtyFlag::TransformIsDirty);
                    theCamera.m_Flags.SetOrthographic(true);
                    QVector2D theWindowDim((float)theWindow.width(), (float)theWindow.height());
                    theCamera.CalculateGlobalVariables(
                                QDemonRenderRect(0, 0, theWindow.width(), theWindow.height()),
                                theWindowDim);
                    // We want a 2D lower left projection
                    float *writePtr(theCamera.m_Projection.data());
                    writePtr[12] = -1;
                    writePtr[13] = -1;

                    // upload vertices
                    m_QuadStripVertexBuffer->UpdateBuffer(theRenderTextDetails.first.m_Vertices,
                                                          false);

                    theTextHelper.m_Shader->Render2D(
                                theRenderTextDetails.second, QVector4D(inColor, 1.0f),
                                theCamera.m_Projection, GetContext(),
                                theTextHelper.m_QuadInputAssembler,
                                theRenderTextDetails.first.m_VertexCount, QVector2D(x, y));
                }
                // we release the memory here
                ::free(theRenderTextDetails.first.m_Vertices.begin());
            }
        }
    }
}

void QDemonRendererImpl::RenderGpuProfilerStats(float x, float y,
                                                QDemonOption<QVector3D> inColor)
{
    if (!IsLayerGpuProfilingEnabled())
        return;

    char messageLine[1024];
    TInstanceRenderMap::const_iterator theIter;

    float startY = y;

    for (theIter = m_InstanceRenderMap.begin(); theIter != m_InstanceRenderMap.end(); theIter++) {
        float startX = x;
        const QSharedPointer<SLayerRenderData> theLayerRenderData = theIter.value();
        const SLayer *theLayer = &theLayerRenderData->m_Layer;

        if (theLayer->m_Flags.IsActive() && theLayerRenderData->m_LayerProfilerGpu) {
            const IRenderProfiler::TStrIDVec &idList =
                    theLayerRenderData->m_LayerProfilerGpu->GetTimerIDs();
            if (!idList.empty()) {
                startY -= 22;
                startX += 20;
                RenderText2D(startX, startY, inColor, theLayer->m_Id.toLocal8Bit().constData());
                IRenderProfiler::TStrIDVec::const_iterator theIdIter = idList.begin();
                for (theIdIter = idList.begin(); theIdIter != idList.end(); theIdIter++) {
                    startY -= 22;
                    sprintf(messageLine, "%s: %.3f ms", theIdIter,
                            theLayerRenderData->m_LayerProfilerGpu->GetElapsedTime(*theIdIter));
                    RenderText2D(startX + 20, startY, inColor, messageLine);
                }
            }
        }
    }
}

// Given a node and a point in the node's local space (most likely its pivot point), we return
// a normal matrix so you can get the axis out, a transformation from node to camera
// a new position and a floating point scale factor so you can render in 1/2 perspective mode
// or orthographic mode if you would like to.
SWidgetRenderInformation
QDemonRendererImpl::GetWidgetRenderInformation(SNode &inNode, const QVector3D &inPos,
                                               RenderWidgetModes::Enum inWidgetMode)
{
    QSharedPointer<SLayerRenderData> theData = GetOrCreateLayerRenderDataForNode(inNode);
    SCamera *theCamera = theData->m_Camera;
    if (theCamera == nullptr || theData->m_LayerPrepResult.hasValue() == false) {
        Q_ASSERT(false);
        return SWidgetRenderInformation();
    }
    QMatrix4x4 theGlobalTransform;
    if (inNode.m_Parent != nullptr && inNode.m_Parent->m_Type != GraphObjectTypes::Layer
            && !inNode.m_Flags.IsIgnoreParentTransform())
        theGlobalTransform = inNode.m_Parent->m_GlobalTransform;
    QMatrix4x4 theCameraInverse = mat44::getInverse(theCamera->m_GlobalTransform);
    QMatrix4x4 theNodeParentToCamera;
    if (inWidgetMode == RenderWidgetModes::Local)
        theNodeParentToCamera = theCameraInverse * theGlobalTransform;
    else
        theNodeParentToCamera = theCameraInverse;

    float normalMatData[9] = {
        theNodeParentToCamera(0,0), theNodeParentToCamera(0, 1), theNodeParentToCamera(0,2),
        theNodeParentToCamera(1,0), theNodeParentToCamera(1, 1), theNodeParentToCamera(1,2),
        theNodeParentToCamera(2,0), theNodeParentToCamera(2, 1), theNodeParentToCamera(2,2)
    };

    QMatrix3x3 theNormalMat(normalMatData);
    theNormalMat = mat33::getInverse(theNormalMat).transposed();
    QVector3D column0(theNormalMat(0, 0), theNormalMat(0, 1), theNormalMat(0, 2));
    QVector3D column1(theNormalMat(1, 0), theNormalMat(1, 1), theNormalMat(1, 2));
    QVector3D column2(theNormalMat(2, 0), theNormalMat(2, 1), theNormalMat(2, 2));
    column0.normalize();
    column1.normalize();
    column2.normalize();
    float normalizedMatData[9] = {
        column0.x(), column0.y(), column0.z(),
        column1.x(), column1.y(), column1.z(),
        column2.x(), column2.y(), column2.z()
    };

    theNormalMat = QMatrix3x3(normalizedMatData);

    QMatrix4x4 theTranslation;
    theTranslation(3, 0) = inNode.m_Position.x();
    theTranslation(3, 1) = inNode.m_Position.y();
    theTranslation(3, 2) = inNode.m_Position.z();
    theTranslation(3, 2) *= -1.0f;

    theGlobalTransform = theGlobalTransform * theTranslation;

    QMatrix4x4 theNodeToParentPlusTranslation = theCameraInverse * theGlobalTransform;
    QVector3D thePos = mat44::transform(theNodeToParentPlusTranslation, inPos);
    SScaleAndPosition theScaleAndPos = GetWorldToPixelScaleFactor(*theCamera, thePos, *theData);
    QMatrix3x3 theLookAtMatrix;
    if (theCamera->m_Flags.IsOrthographic() == false) {
        QVector3D theNodeToCamera = theScaleAndPos.m_Position;
        theNodeToCamera.normalize();
        QVector3D theOriginalAxis = QVector3D(0, 0, -1);
        QVector3D theRotAxis = QVector3D::crossProduct(theOriginalAxis, theNodeToCamera);
        theRotAxis.normalize();
        float theAxisLen = vec3::magnitude(theRotAxis);
        if (theAxisLen > .05f) {
            float theRotAmount = acos(QVector3D::dotProduct(theOriginalAxis, theNodeToCamera));
            QQuaternion theQuat(theRotAmount, theRotAxis);
            theLookAtMatrix = theQuat.toRotationMatrix();
        }
    }
    QVector3D thePosInWorldSpace = mat44::transform(theGlobalTransform, inPos);
    QVector3D theCameraPosInWorldSpace = theCamera->GetGlobalPos();
    QVector3D theCameraOffset = thePosInWorldSpace - theCameraPosInWorldSpace;
    QVector3D theDir = theCameraOffset;
    theDir.normalize();
    // Things should be 600 units from the camera, as that is how all of our math is setup.
    theCameraOffset = 600.0f * theDir;
    return SWidgetRenderInformation(
                theNormalMat, theNodeParentToCamera, theCamera->m_Projection, theCamera->m_Projection,
                theLookAtMatrix, theCameraInverse, theCameraOffset, theScaleAndPos.m_Position,
                theScaleAndPos.m_Scale, *theCamera);
}

QDemonOption<QVector2D> QDemonRendererImpl::GetLayerMouseCoords(SLayer &inLayer,
                                                                const QVector2D &inMouseCoords,
                                                                const QVector2D &inViewportDimensions,
                                                                bool forceImageIntersect) const
{
    QSharedPointer<SLayerRenderData> theData =
            const_cast<QDemonRendererImpl &>(*this).GetOrCreateLayerRenderDataForNode(inLayer);
    return GetLayerMouseCoords(*theData, inMouseCoords, inViewportDimensions,
                               forceImageIntersect);
}

bool IQDemonRenderer::IsGlEsContext(QDemonRenderContextType inContextType)
{
    QDemonRenderContextType esContextTypes(QDemonRenderContextValues::GLES2
                                           | QDemonRenderContextValues::GLES3
                                           | QDemonRenderContextValues::GLES3PLUS);

    if ((inContextType & esContextTypes))
        return true;

    return false;
}

bool IQDemonRenderer::IsGlEs3Context(QDemonRenderContextType inContextType)
{
    if (inContextType == QDemonRenderContextValues::GLES3
            || inContextType == QDemonRenderContextValues::GLES3PLUS)
        return true;

    return false;
}

bool IQDemonRenderer::IsGl2Context(QDemonRenderContextType inContextType)
{
    if (inContextType == QDemonRenderContextValues::GL2)
        return true;

    return false;
}

QSharedPointer<IQDemonRenderer> IQDemonRenderer::CreateRenderer(IQDemonRenderContext *inContext)
{
    return QSharedPointer<QDemonRendererImpl>(new QDemonRendererImpl(inContext));
}
QT_END_NAMESPACE
