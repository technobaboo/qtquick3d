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
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegeneratorv2.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonperframeallocator.h>

#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemon/QDemonDataRef>
#include <QtDemon/qdemonutils.h>

#include <cstdlib>
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

struct QDemonRenderableImage;
struct QDemonShaderGeneratorGeneratedShader;
struct QDemonSubsetRenderable;

static QDemonRenderInstanceId combineLayerAndId(const QDemonRenderLayer *layer, const QDemonRenderInstanceId id)
{
    quint64 x = quint64(layer);
    x += 31u * quint64(id);
    return QDemonRenderInstanceId(x);
}

QDemonRendererImpl::QDemonRendererImpl(const QDemonRef<QDemonRenderContextInterface> &ctx)
    : m_demonContext(ctx)
    , m_context(ctx->renderContext())
    , m_bufferManager(ctx->bufferManager())
    , m_offscreenRenderManager(ctx->offscreenRenderManager())
#ifdef ADVANCED_BLEND_SW_FALLBACK
    , m_layerBlendTexture(ctx->resourceManager())
    , m_blendFb(nullptr)
#endif
    , m_currentLayer(nullptr)
    , m_pickRenderPlugins(true)
    , m_layerCachingEnabled(true)
    , m_layerGPuProfilingEnabled(false)
{
}

QDemonRendererImpl::~QDemonRendererImpl()
{
    m_shaders.clear();
    m_instanceRenderMap.clear();
    m_constantBuffers.clear();
}

void QDemonRendererImpl::childrenUpdated(QDemonRenderNode &inParent)
{
    if (inParent.type == QDemonRenderGraphObject::Type::Layer) {
        auto theIter = m_instanceRenderMap.find(static_cast<QDemonRenderInstanceId>(&inParent));
        if (theIter != m_instanceRenderMap.end()) {
            theIter.value()->cameras.clear();
            theIter.value()->lights.clear();
            theIter.value()->renderableNodes.clear();
        }
    } else if (inParent.parent) {
        childrenUpdated(*inParent.parent);
    }
}

static inline QDemonRenderLayer *getNextLayer(QDemonRenderLayer &inLayer)
{
    if (inLayer.nextSibling && inLayer.nextSibling->type == QDemonRenderGraphObject::Type::Layer)
        return static_cast<QDemonRenderLayer *>(inLayer.nextSibling);
    return nullptr;
}

static inline void maybePushLayer(QDemonRenderLayer &inLayer, QVector<QDemonRenderLayer *> &outLayerList)
{
    inLayer.calculateGlobalVariables();
    if (inLayer.flags.testFlag(QDemonRenderLayer::Flag::GloballyActive) && inLayer.flags.testFlag(QDemonRenderLayer::Flag::LayerRenderToTarget))
        outLayerList.push_back(&inLayer);
}
static void buildRenderableLayers(QDemonRenderLayer &inLayer, QVector<QDemonRenderLayer *> &renderableLayers, bool inRenderSiblings)
{
    maybePushLayer(inLayer, renderableLayers);
    if (inRenderSiblings) {
        for (QDemonRenderLayer *theNextLayer = getNextLayer(inLayer); theNextLayer; theNextLayer = getNextLayer(*theNextLayer))
            maybePushLayer(*theNextLayer, renderableLayers);
    }
}

bool QDemonRendererImpl::prepareLayerForRender(QDemonRenderLayer &inLayer,
                                               const QSize &surfaceSize,
                                               bool inRenderSiblings,
                                               const QDemonRenderInstanceId id,
                                               bool forceDirectRender)
{
    QVector<QDemonRenderLayer *> renderableLayers;
    // Found by fair roll of the dice.
    renderableLayers.reserve(4);

    buildRenderableLayers(inLayer, renderableLayers, inRenderSiblings);

    bool retval = false;

    auto iter = renderableLayers.crbegin();
    const auto end = renderableLayers.crend();
    for (; iter != end; ++iter) {
        // Store the previous state of if we were rendering a layer.
        QDemonRenderLayer *theLayer = *iter;
        QDemonRef<QDemonLayerRenderData> theRenderData = getOrCreateLayerRenderDataForNode(*theLayer, id);

        if (Q_LIKELY(theRenderData)) {
            theRenderData->prepareForRender(surfaceSize, forceDirectRender);
            retval = retval || theRenderData->layerPrepResult->flags.wasDirty();
        } else {
            Q_ASSERT(false);
        }
    }

    return retval;
}

void QDemonRendererImpl::renderLayer(QDemonRenderLayer &inLayer,
                                     const QSize &surfaceSize,
                                     bool clear,
                                     QVector3D clearColor,
                                     bool inRenderSiblings,
                                     const QDemonRenderInstanceId id)
{
    Q_UNUSED(surfaceSize);
    QVector<QDemonRenderLayer *> renderableLayers;
    // Found by fair roll of the dice.
    renderableLayers.reserve(4);

    buildRenderableLayers(inLayer, renderableLayers, inRenderSiblings);

    QDemonRef<QDemonRenderContext> theRenderContext(m_demonContext->renderContext());
    QDemonRef<QDemonRenderFrameBuffer> theFB = theRenderContext->renderTarget();
    auto iter = renderableLayers.rbegin();
    const auto end = renderableLayers.rend();
    for (; iter != end; ++iter) {
        QDemonRenderLayer *theLayer = *iter;
        QDemonRef<QDemonLayerRenderData> theRenderData = getOrCreateLayerRenderDataForNode(*theLayer, id);
        QDemonLayerRenderPreparationResult &prepRes(*theRenderData->layerPrepResult);
        QDemonRenderLayer::BlendMode layerBlend = prepRes.layer()->getLayerBlend();
#ifdef ADVANCED_BLEND_SW_FALLBACK
        if ((layerBlend == QDemonRenderLayer::BlendMode::Overlay || layerBlend == QDemonRenderLayer::BlendMode::ColorBurn || layerBlend == QDemonRenderLayer::BlendMode::ColorDodge)
            && !theRenderContext->supportsAdvancedBlendHW() && !theRenderContext->supportsAdvancedBlendHwKHR()) {
            // Create and set up FBO and texture for advanced blending SW fallback
            QRect viewport = theRenderContext->viewport();
            m_layerBlendTexture.ensureTexture(viewport.width() + viewport.x(),
                                              viewport.height() + viewport.y(),
                                              QDemonRenderTextureFormat::RGBA8);
            if (m_blendFb == nullptr)
                m_blendFb = new QDemonRenderFrameBuffer(theRenderContext);
            m_blendFb->attach(QDemonRenderFrameBufferAttachment::Color0, m_layerBlendTexture.getTexture());
            theRenderContext->setRenderTarget(m_blendFb);
            theRenderContext->setScissorTestEnabled(false);
            QVector4D color(0.0f, 0.0f, 0.0f, 0.0f);
            if (clear && !clearColor.isNull()) {
                color.setX(clearColor.x());
                color.setY(clearColor.y());
                color.setZ(clearColor.z());
                color.setW(1.0f);
            }
            QVector4D origColor = theRenderContext->clearColor();
            theRenderContext->setClearColor(color);
            theRenderContext->clear(QDemonRenderClearValues::Color);
            theRenderContext->setClearColor(origColor);
            theRenderContext->setRenderTarget(theFB);
            break;
        } else {
            m_layerBlendTexture.releaseTexture();
        }
#endif
    }

    for (iter = renderableLayers.rbegin(); iter != end; ++iter) {
        // Store the previous state of if we were rendering a layer.
        QDemonRenderLayer *theLayer = *iter;
        QDemonRef<QDemonLayerRenderData> theRenderData = getOrCreateLayerRenderDataForNode(*theLayer, id);

        if (Q_LIKELY(theRenderData)) {
            // Make sure that we don't clear the window, when requested not to.
            theRenderData->layerPrepResult->flags.setRequiresTransparentClear(clear);
            if (theRenderData->layerPrepResult->isLayerVisible())
                theRenderData->runnableRenderToViewport(theFB);
        } else {
            Q_ASSERT(false);
        }
    }
}

QDemonRenderLayer *QDemonRendererImpl::layerForNode(const QDemonRenderNode &inNode) const
{
    if (inNode.type == QDemonRenderGraphObject::Type::Layer)
        return &const_cast<QDemonRenderLayer &>(static_cast<const QDemonRenderLayer &>(inNode));

    if (inNode.parent)
        return layerForNode(*inNode.parent);

    return nullptr;
}

QDemonRef<QDemonLayerRenderData> QDemonRendererImpl::getOrCreateLayerRenderDataForNode(const QDemonRenderNode &inNode,
                                                                                       const QDemonRenderInstanceId id)
{
    const QDemonRenderLayer *theLayer = layerForNode(inNode);
    if (theLayer) {
        const auto theIter = m_instanceRenderMap.constFind(combineLayerAndId(theLayer, id));
        if (theIter != m_instanceRenderMap.cend())
            return QDemonRef<QDemonLayerRenderData>(theIter.value());

        QDemonRef<QDemonLayerRenderData> theRenderData = QDemonRef<QDemonLayerRenderData>(
                new QDemonLayerRenderData(const_cast<QDemonRenderLayer &>(*theLayer), this));
        m_instanceRenderMap.insert(combineLayerAndId(theLayer, id), theRenderData);

        // create a profiler if enabled
        if (isLayerGpuProfilingEnabled() && theRenderData)
            theRenderData->createGpuProfiler();

        return theRenderData;
    }
    return nullptr;
}

QDemonRenderCamera *QDemonRendererImpl::cameraForNode(const QDemonRenderNode &inNode) const
{
    QDemonRef<QDemonLayerRenderData> theLayer = const_cast<QDemonRendererImpl &>(*this).getOrCreateLayerRenderDataForNode(inNode);
    if (theLayer)
        return theLayer->camera;
    return nullptr;
}

QDemonOption<QDemonCuboidRect> QDemonRendererImpl::cameraBounds(const QDemonRenderGraphObject &inObject)
{
    if (inObject.isNodeType()) {
        const QDemonRenderNode &theNode = static_cast<const QDemonRenderNode &>(inObject);
        QDemonRef<QDemonLayerRenderData> theLayer = getOrCreateLayerRenderDataForNode(theNode);
        if (!theLayer->usesOffscreenRenderer()) {
            QDemonRenderCamera *theCamera = theLayer->camera;
            if (theCamera)
                return theCamera->getCameraBounds(theLayer->layerPrepResult->viewport(),
                                                  theLayer->layerPrepResult->presentationDesignDimensions());
        }
    }
    return QDemonOption<QDemonCuboidRect>();
}

void QDemonRendererImpl::drawScreenRect(QRectF inRect, const QVector3D &inColor)
{
    QDemonRenderCamera theScreenCamera;
    theScreenCamera.markDirty(QDemonRenderCamera::TransformDirtyFlag::TransformIsDirty);
    QRectF theViewport(m_context->viewport());
    theScreenCamera.flags.setFlag(QDemonRenderCamera::Flag::Orthographic);
    theScreenCamera.calculateGlobalVariables(theViewport, QVector2D(float(theViewport.width()), float(theViewport.height())));
    generateXYQuad();
    if (!m_screenRectShader) {
        QDemonRef<QDemonShaderProgramGeneratorInterface> theGenerator(getProgramGenerator());
        theGenerator->beginProgram();
        QDemonShaderStageGeneratorInterface &vertexGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Vertex));
        QDemonShaderStageGeneratorInterface &fragmentGenerator(*theGenerator->getStage(QDemonShaderGeneratorStage::Fragment));
        // TODO: Move out and change type!
        vertexGenerator.addIncoming("attr_pos", "vec3");
        vertexGenerator.addUniform("model_view_projection", "mat4");
        vertexGenerator.addUniform("rectangle_dims", "vec3");
        vertexGenerator.append("void main() {");
        vertexGenerator.append("\tgl_Position = model_view_projection * vec4(attr_pos * rectangle_dims, 1.0);");
        vertexGenerator.append("}");
        fragmentGenerator.addUniform("output_color", "vec3");
        fragmentGenerator.append("void main() {");
        fragmentGenerator.append("\tgl_FragColor.rgb = output_color;");
        fragmentGenerator.append("\tgl_FragColor.a = 1.0;");
        fragmentGenerator.append("}");
        // No flags enabled
        m_screenRectShader = theGenerator->compileGeneratedShader("DrawScreenRect", QDemonShaderCacheProgramFlags(), TShaderFeatureSet());
    }
    if (m_screenRectShader) {
        // Fudge the rect by one pixel to ensure we see all the corners.
        if (inRect.width() > 1)
            inRect.setWidth(inRect.width() - 1);
        if (inRect.height() > 1)
            inRect.setHeight(inRect.height() - 1);
        inRect.setX(inRect.x() + 1);
        inRect.setY(inRect.y() + 1);
        // Figure out the rect center.
        QDemonRenderNode theNode;

        const QPointF &center = inRect.center();
        QVector2D rectGlobalCenter = { float(center.x()), float(center.y()) };
        QVector2D rectCenter(toNormalizedRectRelative(theViewport, rectGlobalCenter));
        theNode.position.setX(rectCenter.x());
        theNode.position.setY(rectCenter.y());
        theNode.markDirty(QDemonRenderNode::TransformDirtyFlag::TransformIsDirty);
        theNode.calculateGlobalVariables();
        QMatrix4x4 theViewProjection;
        theScreenCamera.calculateViewProjectionMatrix(theViewProjection);
        QMatrix4x4 theMVP;
        QMatrix3x3 theNormal;
        theNode.calculateMVPAndNormalMatrix(theViewProjection, theMVP, theNormal);
        m_context->setBlendingEnabled(false);
        m_context->setDepthWriteEnabled(false);
        m_context->setDepthTestEnabled(false);
        m_context->setCullingEnabled(false);
        m_context->setActiveShader(m_screenRectShader);
        m_screenRectShader->setPropertyValue("model_view_projection", theMVP);
        m_screenRectShader->setPropertyValue("output_color", inColor);
        m_screenRectShader->setPropertyValue("rectangle_dims", QVector3D(float(inRect.width()) / 2.0f, float(inRect.height()) / 2.0f, 0.0f));
    }
    if (!m_rectInputAssembler) {
        Q_ASSERT(m_quadVertexBuffer);
        const quint8 indexData[] = { 0, 1, 1, 2, 2, 3, 3, 0 };

        m_rectIndexBuffer = new QDemonRenderIndexBuffer(m_context, QDemonRenderBufferUsageType::Static,
                                                        QDemonRenderComponentType::UnsignedInteger8,
                                                        toDataView(indexData, sizeof(indexData)));

        QDemonRenderVertexBufferEntry theEntries[] = {
            QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 3),
        };

        // create our attribute layout
        m_rectAttribLayout = m_context->createAttributeLayout(toDataView(theEntries, 1));

        quint32 strides = m_quadVertexBuffer->stride();
        quint32 offsets = 0;
        m_rectInputAssembler = m_context->createInputAssembler(m_rectAttribLayout,
                                                               toDataView(&m_quadVertexBuffer, 1),
                                                               m_rectIndexBuffer,
                                                               toDataView(&strides, 1),
                                                               toDataView(&offsets, 1));
    }

    m_context->setInputAssembler(m_rectInputAssembler);
    m_context->draw(QDemonRenderDrawMode::Lines, m_rectIndexBuffer->numIndices(), 0);
}

void QDemonRendererImpl::setupWidgetLayer()
{
    QDemonRef<QDemonRenderContext> theContext = m_demonContext->renderContext();

    if (!m_widgetTexture) {
        QDemonRef<QDemonResourceManager> theManager = m_demonContext->resourceManager();
        m_widgetTexture = theManager->allocateTexture2D(m_beginFrameViewport.width(),
                                                        m_beginFrameViewport.height(),
                                                        QDemonRenderTextureFormat::RGBA8);
        m_widgetFbo = theManager->allocateFrameBuffer();
        m_widgetFbo->attach(QDemonRenderFrameBufferAttachment::Color0, QDemonRenderTextureOrRenderBuffer(m_widgetTexture));
        theContext->setRenderTarget(m_widgetFbo);

        // QDemonRenderRect theScissorRect( 0, 0, m_BeginFrameViewport.m_Width,
        // m_BeginFrameViewport.m_Height );
        // QDemonRenderContextScopedProperty<QDemonRenderRect> __scissorRect( theContext,
        // &QDemonRenderContext::GetScissorRect, &QDemonRenderContext::SetScissorRect, theScissorRect );
        QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theContext,
                                                                 &QDemonRenderContext::isScissorTestEnabled,
                                                                 &QDemonRenderContext::setScissorTestEnabled,
                                                                 false);
        m_context->setClearColor(QVector4D(0, 0, 0, 0));
        m_context->clear(QDemonRenderClearValues::Color);

    } else {
        theContext->setRenderTarget(m_widgetFbo);
    }
}

void QDemonRendererImpl::beginFrame()
{
    for (int idx = 0, end = m_lastFrameLayers.size(); idx < end; ++idx)
        m_lastFrameLayers[idx]->resetForFrame();
    m_lastFrameLayers.clear();
    m_beginFrameViewport = m_demonContext->renderList()->getViewport();
}
void QDemonRendererImpl::endFrame()
{
    if (m_widgetTexture) {
        // Releasing the widget FBO can set it as the active frame buffer.
        QDemonRenderContextScopedProperty<QDemonRef<QDemonRenderFrameBuffer>> __fbo(*m_context,
                                                                                    &QDemonRenderContext::renderTarget,
                                                                                    &QDemonRenderContext::setRenderTarget);
        QDemonTextureDetails theDetails = m_widgetTexture->textureDetails();
        m_context->setBlendingEnabled(true);
        // Colors are expected to be non-premultiplied, so we premultiply alpha into them at
        // this point.
        m_context->setBlendFunction(QDemonRenderBlendFunctionArgument(QDemonRenderSrcBlendFunc::One,
                                                                      QDemonRenderDstBlendFunc::OneMinusSrcAlpha,
                                                                      QDemonRenderSrcBlendFunc::One,
                                                                      QDemonRenderDstBlendFunc::OneMinusSrcAlpha));
        m_context->setBlendEquation(QDemonRenderBlendEquationArgument(QDemonRenderBlendEquation::Add, QDemonRenderBlendEquation::Add));

        m_context->setDepthTestEnabled(false);
        m_context->setScissorTestEnabled(false);
        m_context->setViewport(m_beginFrameViewport);
        QDemonRenderCamera theCamera;
        theCamera.markDirty(QDemonRenderCamera::TransformDirtyFlag::TransformIsDirty);
        theCamera.flags.setFlag(QDemonRenderCamera::Flag::Orthographic);
        QVector2D theTextureDims(float(theDetails.width), float(theDetails.height));
        theCamera.calculateGlobalVariables(QRect(0, 0, theDetails.width, theDetails.height), theTextureDims);
        QMatrix4x4 theViewProj;
        theCamera.calculateViewProjectionMatrix(theViewProj);
        renderQuad(theTextureDims, theViewProj, *m_widgetTexture);

        QDemonRef<QDemonResourceManager> theManager(m_demonContext->resourceManager());
        theManager->release(m_widgetFbo);
        theManager->release(m_widgetTexture);
        m_widgetTexture = nullptr;
        m_widgetFbo = nullptr;
    }
}

inline bool pickResultLessThan(const QDemonRenderPickResult &lhs, const QDemonRenderPickResult &rhs)
{
    return lhs.m_cameraDistanceSq < rhs.m_cameraDistanceSq;
}

inline float clampUVCoord(float inUVCoord, QDemonRenderTextureCoordOp inCoordOp)
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
            float clamp = std::fabs(inUVCoord);
            clamp = clamp - std::floor(clamp);
            if (multiplier < 0)
                inUVCoord = 1.0f - clamp;
            else
                inUVCoord = clamp;
        } break;
        case QDemonRenderTextureCoordOp::MirroredRepeat: {
            float multiplier = inUVCoord > 0.0f ? 1.0f : -1.0f;
            float clamp = std::fabs(inUVCoord);
            if (multiplier > 0.0f)
                clamp -= 1.0f;
            quint32 isMirrored = (quint32(clamp)) % 2 == 0;
            float remainder = clamp - std::floor(clamp);
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

static QPair<QVector2D, QVector2D> getMouseCoordsAndViewportFromSubObject(QVector2D inLocalHitUVSpace,
                                                                          QDemonRenderPickSubResult &inSubResult)
{
    QMatrix4x4 theTextureMatrix(inSubResult.m_textureMatrix);
    QVector3D theNewUVCoords(mat44::transform(theTextureMatrix, (QVector3D(inLocalHitUVSpace.x(), inLocalHitUVSpace.y(), 0))));
    theNewUVCoords.setX(clampUVCoord(theNewUVCoords.x(), inSubResult.m_horizontalTilingMode));
    theNewUVCoords.setY(clampUVCoord(theNewUVCoords.y(), inSubResult.m_verticalTilingMode));
    QVector2D theViewportDimensions = QVector2D(float(inSubResult.m_viewportWidth), float(inSubResult.m_viewportHeight));
    QVector2D theMouseCoords(theNewUVCoords.x() * theViewportDimensions.x(),
                             (1.0f - theNewUVCoords.y()) * theViewportDimensions.y());

    return QPair<QVector2D, QVector2D>(theMouseCoords, theViewportDimensions);
}

QDemonPickResultProcessResult QDemonRendererImpl::processPickResultList(bool inPickEverything)
{
    if (m_lastPickResults.empty())
        return QDemonPickResultProcessResult();
    // Things are rendered in a particular order and we need to respect that ordering.
    std::stable_sort(m_lastPickResults.begin(), m_lastPickResults.end(), pickResultLessThan);

    // We need to pick against sub objects basically somewhat recursively
    // but if we don't hit any sub objects and the parent isn't pickable then
    // we need to move onto the next item in the list.
    // We need to keep in mind that theQuery->Pick will enter this method in a later
    // stack frame so *if* we get to sub objects we need to pick against them but if the pick
    // completely misses *and* the parent object locally pickable is false then we need to move
    // onto the next object.

    const int numToCopy = m_lastPickResults.size();
    Q_ASSERT(numToCopy >= 0);
    size_t numCopyBytes = size_t(numToCopy) * sizeof(QDemonRenderPickResult);
    QDemonRenderPickResult *thePickResults = reinterpret_cast<QDemonRenderPickResult *>(
            m_demonContext->perFrameAllocator().allocate(numCopyBytes));
    ::memcpy(thePickResults, m_lastPickResults.data(), numCopyBytes);
    m_lastPickResults.clear();
    bool foundValidResult = false;
    QDemonPickResultProcessResult thePickResult(thePickResults[0]);
    for (size_t idx = 0; idx < size_t(numToCopy) && !foundValidResult; ++idx) {
        thePickResult = thePickResults[idx];
        // Here we do a hierarchy.  Picking against sub objects takes precedence.
        // If picking against the sub object doesn't return a valid result *and*
        // the current object isn't globally pickable then we move onto the next object returned
        // by the pick query.
        if (thePickResult.m_hitObject != nullptr && thePickResult.m_firstSubObject != nullptr && m_pickRenderPlugins) {
            QVector2D theUVCoords(thePickResult.m_localUVCoords.x(), thePickResult.m_localUVCoords.y());
            QDemonRef<QDemonOffscreenRendererInterface> theSubRenderer(thePickResult.m_firstSubObject->m_subRenderer);
            QPair<QVector2D, QVector2D> mouseAndViewport = getMouseCoordsAndViewportFromSubObject(theUVCoords,
                                                                                                  *thePickResult.m_firstSubObject);
            QVector2D theMouseCoords = mouseAndViewport.first;
            QVector2D theViewportDimensions = mouseAndViewport.second;
            QDemonGraphObjectPickQueryInterface *theQuery = theSubRenderer->getGraphObjectPickQuery(this);
            if (theQuery) {
                QDemonRenderPickResult theInnerPickResult = theQuery->pick(theMouseCoords, theViewportDimensions, inPickEverything);
                if (theInnerPickResult.m_hitObject) {
                    thePickResult = theInnerPickResult;
                    foundValidResult = true;
                    thePickResult.m_wasPickConsumed = true;
                } else if (thePickResult.m_hitObject->isNodeType()) {
                    const QDemonRenderNode *theNode = static_cast<const QDemonRenderNode *>(thePickResult.m_hitObject);
                    if (theNode->flags.testFlag(QDemonRenderNode::Flag::GloballyPickable)) {
                        foundValidResult = true;
                        thePickResult.m_wasPickConsumed = true;
                    }
                }
            } else {
                // If the sub renderer doesn't consume the pick then we return the picked object
                // itself.  So no matter what, if we get to here the pick was consumed.
                thePickResult.m_wasPickConsumed = true;
                bool wasPickConsumed = theSubRenderer->pick(theMouseCoords, theViewportDimensions, this);
                if (wasPickConsumed) {
                    thePickResult.m_hitObject = nullptr;
                    foundValidResult = true;
                }
            }
        } else {
            foundValidResult = true;
            thePickResult.m_wasPickConsumed = true;
        }
    }
    return thePickResult;
}

QDemonRenderPickResult QDemonRendererImpl::pick(QDemonRenderLayer &inLayer,
                                                const QVector2D &inViewportDimensions,
                                                const QVector2D &inMouseCoords,
                                                bool inPickSiblings,
                                                bool inPickEverything,
                                                const QDemonRenderInstanceId id)
{
    m_lastPickResults.clear();

    QDemonRenderLayer *theLayer = &inLayer;
    // Stepping through how the original runtime did picking it picked layers in order
    // stopping at the first hit.  So objects on the top layer had first crack at the pick
    // vector itself.
    do {
        if (theLayer->flags.testFlag(QDemonRenderLayer::Flag::Active)) {
            const auto theIter = m_instanceRenderMap.constFind(combineLayerAndId(theLayer, id));
            if (theIter != m_instanceRenderMap.cend()) {
                m_lastPickResults.clear();
                getLayerHitObjectList(*theIter.value(), inViewportDimensions, inMouseCoords, inPickEverything, m_lastPickResults);
                QDemonPickResultProcessResult retval(processPickResultList(inPickEverything));
                if (retval.m_wasPickConsumed)
                    return retval;
            } else {
                // Q_ASSERT( false );
            }
        }

        if (inPickSiblings)
            theLayer = getNextLayer(*theLayer);
        else
            theLayer = nullptr;
    } while (theLayer != nullptr);

    return QDemonRenderPickResult();
}

static inline QDemonOption<QVector2D> intersectRayWithNode(const QDemonRenderNode &inNode,
                                                           QDemonRenderableObject &inRenderableObject,
                                                           const QDemonRenderRay &inPickRay)
{
    if (inRenderableObject.renderableFlags.isDefaultMaterialMeshSubset()) {
        QDemonSubsetRenderable &theRenderable = static_cast<QDemonSubsetRenderable &>(inRenderableObject);
        if (&theRenderable.modelContext.model == &inNode)
            return inPickRay.relativeXY(inRenderableObject.globalTransform, inRenderableObject.bounds);
    } else if (inRenderableObject.renderableFlags.isCustomMaterialMeshSubset()) {
        QDemonCustomMaterialRenderable &theRenderable = static_cast<QDemonCustomMaterialRenderable &>(inRenderableObject);
        if (&theRenderable.modelContext.model == &inNode)
            return inPickRay.relativeXY(inRenderableObject.globalTransform, inRenderableObject.bounds);
    } else {
        Q_ASSERT(false);
    }
    return QDemonEmpty();
}

static inline QDemonRenderPickSubResult constructSubResult(QDemonRenderImage &inImage)
{
    QDemonTextureDetails theDetails = inImage.m_textureData.m_texture->textureDetails();
    return QDemonRenderPickSubResult(inImage.m_lastFrameOffscreenRenderer,
                                     inImage.m_textureTransform,
                                     inImage.m_horizontalTilingMode,
                                     inImage.m_verticalTilingMode,
                                     theDetails.width,
                                     theDetails.height);
}

QDemonOption<QVector2D> QDemonRendererImpl::facePosition(QDemonRenderNode &inNode,
                                                         QDemonBounds3 inBounds,
                                                         const QMatrix4x4 &inGlobalTransform,
                                                         const QVector2D &inViewportDimensions,
                                                         const QVector2D &inMouseCoords,
                                                         QDemonDataView<QDemonRenderGraphObject *> inMapperObjects,
                                                         QDemonRenderBasisPlanes inPlane)
{
    QDemonRef<QDemonLayerRenderData> theLayerData = getOrCreateLayerRenderDataForNode(inNode);
    if (theLayerData == nullptr)
        return QDemonEmpty();
    // This function assumes the layer was rendered to the scene itself.  There is another
    // function
    // for completely offscreen layers that don't get rendered to the scene.
    bool wasRenderToTarget(theLayerData->layer.flags.testFlag(QDemonRenderLayer::Flag::LayerRenderToTarget));
    if (!wasRenderToTarget || theLayerData->camera == nullptr
        || theLayerData->layerPrepResult.hasValue() == false || theLayerData->lastFrameOffscreenRenderer != nullptr)
        return QDemonEmpty();

    QVector2D theMouseCoords(inMouseCoords);
    QVector2D theViewportDimensions(inViewportDimensions);

    for (const auto &currentObject : qAsConst(inMapperObjects)) {
        if (currentObject->type == QDemonRenderGraphObject::Type::Layer) {
            // The layer knows its viewport so it can take the information directly.
            // This is extremely counter intuitive but a good sign.
        } else if (currentObject->type == QDemonRenderGraphObject::Type::Image) {
            QDemonRenderImage &theImage = static_cast<QDemonRenderImage &>(*currentObject);
            QDemonRenderModel *theParentModel = nullptr;
            if (theImage.m_parent && theImage.m_parent->type == QDemonRenderGraphObject::Type::DefaultMaterial) {
                QDemonRenderDefaultMaterial *theMaterial = static_cast<QDemonRenderDefaultMaterial *>(theImage.m_parent);
                if (theMaterial) {
                    theParentModel = theMaterial->parent;
                }
            }
            if (Q_UNLIKELY(theParentModel == nullptr)) {
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            QDemonBounds3 theModelBounds = theParentModel->getBounds(demonContext()->bufferManager(),
                                                                     demonContext()->pathManager(),
                                                                     false);

            if (Q_UNLIKELY(theModelBounds.isEmpty())) {
                Q_ASSERT(false);
                return QDemonEmpty();
            }
            QDemonOption<QVector2D> relativeHit = facePosition(*theParentModel,
                                                               theModelBounds,
                                                               theParentModel->globalTransform,
                                                               theViewportDimensions,
                                                               theMouseCoords,
                                                               QDemonDataView<QDemonRenderGraphObject *>(),
                                                               QDemonRenderBasisPlanes::XY);
            if (relativeHit.isEmpty())
                return QDemonEmpty();

            QDemonRenderPickSubResult theResult = constructSubResult(theImage);
            QVector2D hitInUVSpace = (*relativeHit) + QVector2D(.5f, .5f);
            QPair<QVector2D, QVector2D> mouseAndViewport = getMouseCoordsAndViewportFromSubObject(hitInUVSpace, theResult);
            theMouseCoords = mouseAndViewport.first;
            theViewportDimensions = mouseAndViewport.second;
        }
    }

    QDemonOption<QDemonRenderRay> theHitRay = theLayerData->layerPrepResult->pickRay(theMouseCoords, theViewportDimensions, false);
    if (!theHitRay.hasValue())
        return QDemonEmpty();

    // Scale the mouse coords to change them into the camera's numerical space.
    QDemonRenderRay thePickRay = *theHitRay;
    QDemonOption<QVector2D> newValue = thePickRay.relative(inGlobalTransform, inBounds, inPlane);
    return newValue;
}

QDemonRenderPickResult QDemonRendererImpl::pickOffscreenLayer(QDemonRenderLayer & /*inLayer*/,
                                                              const QVector2D & /*inViewportDimensions*/,
                                                              const QVector2D & /*inMouseCoords*/,
                                                              bool /*inPickEverything*/)
{
    return QDemonRenderPickResult();
}

QVector3D QDemonRendererImpl::unprojectToPosition(QDemonRenderNode &inNode, QVector3D &inPosition, const QVector2D &inMouseVec) const
{
    // Translate mouse into layer's coordinates
    QDemonRef<QDemonLayerRenderData> theData = const_cast<QDemonRendererImpl &>(*this).getOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->camera == nullptr) {
        return QVector3D(0, 0, 0);
    } // Q_ASSERT( false ); return QVector3D(0,0,0); }

    QSize theWindow = m_demonContext->windowDimensions();
    QVector2D theDims(float(theWindow.width()), float(theWindow.height()));

    QDemonLayerRenderPreparationResult &thePrepResult(*theData->layerPrepResult);
    QDemonRenderRay theRay = thePrepResult.pickRay(inMouseVec, theDims, true);

    return theData->camera->unprojectToPosition(inPosition, theRay);
}

QVector3D QDemonRendererImpl::unprojectWithDepth(QDemonRenderNode &inNode, QVector3D &, const QVector3D &inMouseVec) const
{
    // Translate mouse into layer's coordinates
    QDemonRef<QDemonLayerRenderData> theData = const_cast<QDemonRendererImpl &>(*this).getOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->camera == nullptr) {
        return QVector3D(0, 0, 0);
    } // Q_ASSERT( false ); return QVector3D(0,0,0); }

    // Flip the y into gl coordinates from window coordinates.
    QVector2D theMouse(inMouseVec.x(), inMouseVec.y());
    float theDepth = inMouseVec.z();

    QDemonLayerRenderPreparationResult &thePrepResult(*theData->layerPrepResult);
    QSize theWindow = m_demonContext->windowDimensions();
    QDemonRenderRay theRay = thePrepResult.pickRay(theMouse, QVector2D(float(theWindow.width()), float(theWindow.height())), true);
    QVector3D theTargetPosition = theRay.origin + theRay.direction * theDepth;
    if (inNode.parent != nullptr && inNode.parent->type != QDemonRenderGraphObject::Type::Layer)
        theTargetPosition = mat44::transform(mat44::getInverse(inNode.parent->globalTransform), theTargetPosition);
    // Our default global space is right handed, so if you are left handed z means something
    // opposite.
    if (inNode.flags.testFlag(QDemonRenderNode::Flag::LeftHanded))
        theTargetPosition.setZ(theTargetPosition.z() * -1);
    return theTargetPosition;
}

QVector3D QDemonRendererImpl::projectPosition(QDemonRenderNode &inNode, const QVector3D &inPosition) const
{
    // Translate mouse into layer's coordinates
    QDemonRef<QDemonLayerRenderData> theData = const_cast<QDemonRendererImpl &>(*this).getOrCreateLayerRenderDataForNode(inNode);
    if (theData == nullptr || theData->camera == nullptr) {
        return QVector3D(0, 0, 0);
    }

    QMatrix4x4 viewProj;
    theData->camera->calculateViewProjectionMatrix(viewProj);
    QVector4D projPos = mat44::transform(viewProj, QVector4D(inPosition, 1.0f));
    projPos.setX(projPos.x() / projPos.w());
    projPos.setY(projPos.y() / projPos.w());

    QRectF theViewport = theData->layerPrepResult->viewport();
    QVector2D theDims(float(theViewport.width()), float(theViewport.height()));
    projPos.setX(projPos.x() + 1.0f);
    projPos.setY(projPos.y() + 1.0f);
    projPos.setX(projPos.x() * 0.5f);
    projPos.setY(projPos.y() * 0.5f);
    QVector3D cameraToObject = theData->camera->getGlobalPos() - inPosition;
    projPos.setZ(sqrtf(QVector3D::dotProduct(cameraToObject, cameraToObject)));
    QVector3D mouseVec = QVector3D(projPos.x(), projPos.y(), projPos.z());
    mouseVec.setX(mouseVec.x() * theDims.x());
    mouseVec.setY(mouseVec.y() * theDims.y());

    mouseVec.setX(mouseVec.x() + float(theViewport.x()));
    mouseVec.setY(mouseVec.y() + float(theViewport.y()));

    // Flip the y into window coordinates so it matches the mouse.
    QSize theWindow = m_demonContext->windowDimensions();
    mouseVec.setY(theWindow.height() - mouseVec.y());

    return mouseVec;
}

QDemonOption<QDemonLayerPickSetup> QDemonRendererImpl::getLayerPickSetup(QDemonRenderLayer &inLayer,
                                                                         const QVector2D &inMouseCoords,
                                                                         const QSize &inPickDims)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inLayer);
    if (Q_UNLIKELY(theData == nullptr || theData->camera == nullptr)) {
        Q_ASSERT(false);
        return QDemonEmpty();
    }
    QSize theWindow = m_demonContext->windowDimensions();
    QVector2D theDims(float(theWindow.width()), float(theWindow.height()));
    // The mouse is relative to the layer
    QDemonOption<QVector2D> theLocalMouse = getLayerMouseCoords(*theData, inMouseCoords, theDims, false);
    if (!theLocalMouse.hasValue())
        return QDemonEmpty();

    QDemonLayerRenderPreparationResult &thePrepResult(*theData->layerPrepResult);
    if (thePrepResult.camera() == nullptr) {
        return QDemonEmpty();
    }
    // Perform gluPickMatrix and pre-multiply it into the view projection
    QMatrix4x4 theTransScale;
    QDemonRenderCamera &theCamera(*thePrepResult.camera());

    QRectF layerToPresentation = thePrepResult.viewport();
    // Offsetting is already taken care of in the camera's projection.
    // All we need to do is to scale and translate the image.
    layerToPresentation.setX(0);
    layerToPresentation.setY(0);
    QVector2D theMouse(*theLocalMouse);
    // The viewport will need to center at this location
    QVector2D viewportDims(float(inPickDims.width()), float(inPickDims.height()));
    QVector2D bottomLeft = QVector2D(theMouse.x() - viewportDims.x() / 2.0f, theMouse.y() - viewportDims.y() / 2.0f);
    // For some reason, and I haven't figured out why yet, the offsets need to be backwards for
    // this to work.
    // bottomLeft.x = layerToPresentation.m_Width - bottomLeft.x;
    // bottomLeft.y = layerToPresentation.m_Height - bottomLeft.y;
    // Virtual rect is relative to the layer.
    QRectF thePickRect(qreal(bottomLeft.x()), qreal(bottomLeft.y()), qreal(viewportDims.x()), qreal(viewportDims.y()));
    QMatrix4x4 projectionPremult;
    projectionPremult = QDemonRenderContext::applyVirtualViewportToProjectionMatrix(projectionPremult, layerToPresentation, thePickRect);
    projectionPremult = mat44::getInverse(projectionPremult);

    QMatrix4x4 globalInverse = mat44::getInverse(theCamera.globalTransform);
    QMatrix4x4 theVP = theCamera.projection * globalInverse;
    // For now we won't setup the scissor, so we may be off by inPickDims at most because
    // GetLayerMouseCoords will return
    // false if the mouse is too far off the layer.
    return QDemonLayerPickSetup(projectionPremult,
                                theVP,
                                QRect(0, 0, int(layerToPresentation.width()), int(layerToPresentation.height())));
}

QDemonOption<QRectF> QDemonRendererImpl::layerRect(QDemonRenderLayer &inLayer)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inLayer);
    if (Q_UNLIKELY(theData == nullptr || theData->camera == nullptr)) {
        Q_ASSERT(false);
        return QDemonEmpty();
    }
    QDemonLayerRenderPreparationResult &thePrepResult(*theData->layerPrepResult);
    return thePrepResult.viewport();
}

// This doesn't have to be cheap.
void QDemonRendererImpl::runLayerRender(QDemonRenderLayer &inLayer, const QMatrix4x4 &inViewProjection)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inLayer);
    if (Q_UNLIKELY(theData == nullptr || theData->camera == nullptr)) {
        Q_ASSERT(false);
        return;
    }
    theData->prepareAndRender(inViewProjection);
}

void QDemonRendererImpl::addRenderWidget(QDemonRenderWidgetInterface &inWidget)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inWidget.getNode());
    if (theData)
        theData->addRenderWidget(inWidget);
}

void QDemonRendererImpl::renderLayerRect(QDemonRenderLayer &inLayer, const QVector3D &inColor)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inLayer);
    if (theData)
        theData->m_boundingRectColor = inColor;
}

QDemonScaleAndPosition QDemonRendererImpl::worldToPixelScaleFactor(const QDemonRenderCamera &inCamera,
                                                                      const QVector3D &inWorldPoint,
                                                                      QDemonLayerRenderData &inRenderData)
{
    if (inCamera.flags.testFlag(QDemonRenderCamera::Flag::Orthographic)) {
        // There are situations where the camera can scale.
        return QDemonScaleAndPosition(inWorldPoint,
                                      inCamera.getOrthographicScaleFactor(inRenderData.layerPrepResult->viewport(),
                                                                          inRenderData.layerPrepResult->presentationDesignDimensions()));
    } else {
        QVector3D theCameraPos(0, 0, 0);
        QVector3D theCameraDir(0, 0, -1);
        QDemonRenderRay theRay(theCameraPos, inWorldPoint - theCameraPos);
        QDemonPlane thePlane(theCameraDir, -600);
        QVector3D theItemPosition(inWorldPoint);
        QDemonOption<QVector3D> theIntersection = theRay.intersect(thePlane);
        if (theIntersection.hasValue())
            theItemPosition = *theIntersection;
        // The special number comes in from physically measuring how off we are on the screen.
        float theScaleFactor = (1.0f / inCamera.projection(1, 1));
        QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inCamera);
        const float theHeight = theData->layerPrepResult->textureDimensions().height();
        const float theScaleMultiplier = 600.0f / (theHeight / 2.0f);
        theScaleFactor *= theScaleMultiplier;

        return QDemonScaleAndPosition(theItemPosition, theScaleFactor);
    }
}

QDemonScaleAndPosition QDemonRendererImpl::worldToPixelScaleFactor(QDemonRenderLayer &inLayer, const QVector3D &inWorldPoint)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inLayer);
    if (Q_UNLIKELY(theData == nullptr || theData->camera == nullptr)) {
        Q_ASSERT(false);
        return QDemonScaleAndPosition();
    }
    return worldToPixelScaleFactor(*theData->camera, inWorldPoint, *theData);
}

void QDemonRendererImpl::releaseLayerRenderResources(QDemonRenderLayer &inLayer, const QDemonRenderInstanceId id)
{
    auto theIter = m_instanceRenderMap.find(combineLayerAndId(&inLayer, id));
    if (theIter != m_instanceRenderMap.end()) {
        auto theLastFrm = std::find(m_lastFrameLayers.begin(), m_lastFrameLayers.end(), theIter.value());
        if (theLastFrm != m_lastFrameLayers.end()) {
            theIter.value()->resetForFrame();
            m_lastFrameLayers.erase(theLastFrm);
        }
        m_instanceRenderMap.erase(theIter);
    }
}

void QDemonRendererImpl::renderQuad(const QVector2D inDimensions, const QMatrix4x4 &inMVP, QDemonRenderTexture2D &inQuadTexture)
{
    m_context->setCullingEnabled(false);
    QDemonRef<QDemonLayerSceneShader> theShader = getSceneLayerShader();
    m_context->setActiveShader(theShader->shader);
    theShader->mvp.set(inMVP);
    theShader->dimensions.set(inDimensions);
    theShader->sampler.set(&inQuadTexture);

    generateXYQuad();
    m_context->setInputAssembler(m_quadInputAssembler);
    m_context->draw(QDemonRenderDrawMode::Triangles, m_quadIndexBuffer->numIndices(), 0);
}

void QDemonRendererImpl::renderQuad()
{
    m_context->setCullingEnabled(false);
    generateXYQuad();
    m_context->setInputAssembler(m_quadInputAssembler);
    m_context->draw(QDemonRenderDrawMode::Triangles, m_quadIndexBuffer->numIndices(), 0);
}

void QDemonRendererImpl::renderPointsIndirect()
{
    m_context->setCullingEnabled(false);
    generateXYZPoint();
    m_context->setInputAssembler(m_pointInputAssembler);
    m_context->drawIndirect(QDemonRenderDrawMode::Points, 0);
}

void QDemonRendererImpl::layerNeedsFrameClear(QDemonLayerRenderData &inLayer)
{
    m_lastFrameLayers.push_back(&inLayer);
}

void QDemonRendererImpl::beginLayerDepthPassRender(QDemonLayerRenderData &inLayer)
{
    m_currentLayer = &inLayer;
}

void QDemonRendererImpl::endLayerDepthPassRender()
{
    m_currentLayer = nullptr;
}

void QDemonRendererImpl::beginLayerRender(QDemonLayerRenderData &inLayer)
{
    m_currentLayer = &inLayer;
}
void QDemonRendererImpl::endLayerRender()
{
    m_currentLayer = nullptr;
}

void QDemonRendererImpl::prepareImageForIbl(QDemonRenderImage &inImage)
{
    if (inImage.m_textureData.m_texture && inImage.m_textureData.m_texture->numMipmaps() < 1)
        inImage.m_textureData.m_texture->generateMipmaps();
}

bool nodeContainsBoneRoot(QDemonRenderNode &childNode, qint32 rootID)
{
    for (QDemonRenderNode *childChild = childNode.firstChild; childChild != nullptr; childChild = childChild->nextSibling) {
        if (childChild->skeletonId == rootID)
            return true;
    }

    return false;
}

void fillBoneIdNodeMap(QDemonRenderNode &childNode, QHash<long, QDemonRenderNode *> &ioMap)
{
    if (childNode.skeletonId >= 0)
        ioMap[childNode.skeletonId] = &childNode;
    for (QDemonRenderNode *childChild = childNode.firstChild; childChild != nullptr; childChild = childChild->nextSibling)
        fillBoneIdNodeMap(*childChild, ioMap);
}

QDemonOption<QVector2D> QDemonRendererImpl::getLayerMouseCoords(QDemonLayerRenderData &inLayerRenderData,
                                                                const QVector2D &inMouseCoords,
                                                                const QVector2D &inViewportDimensions,
                                                                bool forceImageIntersect) const
{
    if (inLayerRenderData.layerPrepResult.hasValue())
        return inLayerRenderData.layerPrepResult->layerMouseCoords(inMouseCoords, inViewportDimensions, forceImageIntersect);
    return QDemonEmpty();
}

void QDemonRendererImpl::getLayerHitObjectList(QDemonLayerRenderData &inLayerRenderData,
                                               const QVector2D &inViewportDimensions,
                                               const QVector2D &inPresCoords,
                                               bool inPickEverything,
                                               TPickResultArray &outIntersectionResult)
{
    // This function assumes the layer was rendered to the scene itself.  There is another
    // function
    // for completely offscreen layers that don't get rendered to the scene.
    bool wasRenderToTarget(inLayerRenderData.layer.flags.testFlag(QDemonRenderLayer::Flag::LayerRenderToTarget));
    if (wasRenderToTarget && inLayerRenderData.camera != nullptr) {
        QDemonOption<QDemonRenderRay> theHitRay;
        if (inLayerRenderData.layerPrepResult.hasValue())
            theHitRay = inLayerRenderData.layerPrepResult->pickRay(inPresCoords, inViewportDimensions, false);
        if (inLayerRenderData.lastFrameOffscreenRenderer == nullptr) {
            if (theHitRay.hasValue()) {
                // Scale the mouse coords to change them into the camera's numerical space.
                QDemonRenderRay thePickRay = *theHitRay;
                for (int idx = inLayerRenderData.opaqueObjects.size(), end = 0; idx > end; --idx) {
                    QDemonRenderableObject *theRenderableObject = inLayerRenderData.opaqueObjects.at(idx - 1);
                    if (inPickEverything || theRenderableObject->renderableFlags.isPickable())
                        intersectRayWithSubsetRenderable(thePickRay, *theRenderableObject, outIntersectionResult);
                }
                for (int idx = inLayerRenderData.transparentObjects.size(), end = 0; idx > end; --idx) {
                    QDemonRenderableObject *theRenderableObject = inLayerRenderData.transparentObjects.at(idx - 1);
                    if (inPickEverything || theRenderableObject->renderableFlags.isPickable())
                        intersectRayWithSubsetRenderable(thePickRay, *theRenderableObject, outIntersectionResult);
                }
            }
        } else {
            QDemonGraphObjectPickQueryInterface *theQuery = inLayerRenderData.lastFrameOffscreenRenderer->getGraphObjectPickQuery(this);
            if (theQuery) {
                QDemonRenderPickResult theResult = theQuery->pick(inPresCoords, inViewportDimensions, inPickEverything);
                if (theResult.m_hitObject)
                    outIntersectionResult.push_back(theResult);
            } else {
                inLayerRenderData.lastFrameOffscreenRenderer->pick(inPresCoords, inViewportDimensions, this);
            }
        }
    }
}

static inline QDemonRenderPickSubResult constructSubResult(QDemonRenderableImage &inImage)
{
    return constructSubResult(inImage.m_image);
}

void QDemonRendererImpl::intersectRayWithSubsetRenderable(const QDemonRenderRay &inRay,
                                                          QDemonRenderableObject &inRenderableObject,
                                                          TPickResultArray &outIntersectionResultList)
{
    QDemonRenderRay::IntersectionResult intersectionResult =
            inRay.intersectWithAABB(inRenderableObject.globalTransform, inRenderableObject.bounds);
    if (!intersectionResult.intersects)
        return;

    // Leave the coordinates relative for right now.
    const QDemonRenderGraphObject *thePickObject = nullptr;
    if (inRenderableObject.renderableFlags.isDefaultMaterialMeshSubset())
        thePickObject = &static_cast<QDemonSubsetRenderable *>(&inRenderableObject)->modelContext.model;
    else if (inRenderableObject.renderableFlags.isCustomMaterialMeshSubset())
        thePickObject = &static_cast<QDemonCustomMaterialRenderable *>(&inRenderableObject)->modelContext.model;
    else if (inRenderableObject.renderableFlags.isPath())
        thePickObject = &static_cast<QDemonPathRenderable *>(&inRenderableObject)->m_path;

    if (thePickObject != nullptr) {
        outIntersectionResultList.push_back(
                QDemonRenderPickResult(*thePickObject, intersectionResult.rayLengthSquared, intersectionResult.relXY));

        // For subsets, we know we can find images on them which may have been the result
        // of rendering a sub-presentation.
        if (inRenderableObject.renderableFlags.isDefaultMaterialMeshSubset()) {
            QDemonRenderPickSubResult *theLastResult = nullptr;
            for (QDemonRenderableImage *theImage = static_cast<QDemonSubsetRenderable *>(&inRenderableObject)->firstImage;
                 theImage != nullptr;
                 theImage = theImage->m_nextImage) {
                if (theImage->m_image.m_lastFrameOffscreenRenderer != nullptr && theImage->m_image.m_textureData.m_texture != nullptr) {
                    QDemonRenderPickSubResult *theSubResult = new QDemonRenderPickSubResult(constructSubResult(*theImage));
                    if (theLastResult == nullptr)
                        outIntersectionResultList.back().m_firstSubObject = theSubResult;
                    else
                        theLastResult->m_nextSibling = theSubResult;
                    theLastResult = theSubResult;
                }
            }
        }
    }
}

QDemonRef<QDemonRenderShaderProgram> QDemonRendererImpl::compileShader(const QByteArray &inName, const char *inVert, const char *inFrag)
{
    getProgramGenerator()->beginProgram();
    getProgramGenerator()->getStage(QDemonShaderGeneratorStage::Vertex)->append(inVert);
    getProgramGenerator()->getStage(QDemonShaderGeneratorStage::Fragment)->append(inFrag);
    return getProgramGenerator()->compileGeneratedShader(inName);
}

const float MINATTENUATION = 0;
const float MAXATTENUATION = 1000;

float clampFloat(float value, float min, float max)
{
    return value < min ? min : ((value > max) ? max : value);
}

float translateConstantAttenuation(float attenuation)
{
    return attenuation * .01f;
}

float translateLinearAttenuation(float attenuation)
{
    attenuation = clampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0001f;
}

float translateQuadraticAttenuation(float attenuation)
{
    attenuation = clampFloat(attenuation, MINATTENUATION, MAXATTENUATION);
    return attenuation * 0.0000001f;
}

QDemonRef<QDemonShaderGeneratorGeneratedShader> QDemonRendererImpl::getShader(QDemonSubsetRenderable &inRenderable,
                                                                              const TShaderFeatureSet &inFeatureSet)
{
    if (Q_UNLIKELY(m_currentLayer == nullptr)) {
        Q_ASSERT(false);
        return nullptr;
    }
    const auto theFind = m_shaders.constFind(inRenderable.shaderDescription);
    QDemonRef<QDemonShaderGeneratorGeneratedShader> retval = nullptr;
    if (theFind == m_shaders.cend()) {
        // Generate the shader.
        QDemonRef<QDemonRenderShaderProgram> theShader(generateShader(inRenderable, inFeatureSet));
        if (theShader) {
            QDemonRef<QDemonShaderGeneratorGeneratedShader> theGeneratedShader = QDemonRef<QDemonShaderGeneratorGeneratedShader>(
                    new QDemonShaderGeneratorGeneratedShader(m_generatedShaderString, theShader));
            m_shaders.insert(inRenderable.shaderDescription, theGeneratedShader);
            retval = theGeneratedShader;
        } else {
            // We still insert something because we don't to attempt to generate the same bad shader
            // twice.
            m_shaders.insert(inRenderable.shaderDescription, nullptr);
        }
    } else {
        retval = theFind.value();
    }

    if (retval != nullptr) {
        if (m_currentLayer && m_currentLayer->camera) {
            QDemonRenderCamera &theCamera(*m_currentLayer->camera);
            if (!m_currentLayer->cameraDirection.hasValue())
                m_currentLayer->cameraDirection = theCamera.getScalingCorrectDirection();
        }
    }
    return retval;
}
static QVector3D g_fullScreenRectFace[] = {
    QVector3D(-1, -1, 0),
    QVector3D(-1, 1, 0),
    QVector3D(1, 1, 0),
    QVector3D(1, -1, 0),
};

static QVector2D g_fullScreenRectUVs[] = { QVector2D(0, 0), QVector2D(0, 1), QVector2D(1, 1), QVector2D(1, 0) };

void QDemonRendererImpl::generateXYQuad()
{
    if (m_quadInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv", QDemonRenderComponentType::Float32, 2, 12),
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
    m_quadVertexBuffer = new QDemonRenderVertexBuffer(m_context, QDemonRenderBufferUsageType::Static,
                                                       3 * sizeof(float) + 2 * sizeof(float),
                                                       toByteView(tempBuf, 20));

    quint8 indexData[] = {
        0, 1, 2, 0, 2, 3,
    };
    m_quadIndexBuffer = new QDemonRenderIndexBuffer(m_context, QDemonRenderBufferUsageType::Static,
                                                     QDemonRenderComponentType::UnsignedInteger8,
                                                     toByteView(indexData, sizeof(indexData)));

    // create our attribute layout
    m_quadAttribLayout = m_context->createAttributeLayout(toDataView(theEntries, 2));

    // create input assembler object
    quint32 strides = m_quadVertexBuffer->stride();
    quint32 offsets = 0;
    m_quadInputAssembler = m_context->createInputAssembler(m_quadAttribLayout,
                                                           toDataView(&m_quadVertexBuffer, 1),
                                                           m_quadIndexBuffer,
                                                           toDataView(&strides, 1),
                                                           toDataView(&offsets, 1));
}

void QDemonRendererImpl::generateXYZPoint()
{
    if (m_pointInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv", QDemonRenderComponentType::Float32, 2, 12),
    };

    float tempBuf[5] { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    m_pointVertexBuffer = new QDemonRenderVertexBuffer(m_context, QDemonRenderBufferUsageType::Static,
                                                        3 * sizeof(float) + 2 * sizeof(float),
                                                        toByteView(tempBuf, 5));

    // create our attribute layout
    m_pointAttribLayout = m_context->createAttributeLayout(toDataView(theEntries, 2));

    // create input assembler object
    quint32 strides = m_pointVertexBuffer->stride();
    quint32 offsets = 0;
    m_pointInputAssembler = m_context->createInputAssembler(m_pointAttribLayout,
                                                            toDataView(&m_pointVertexBuffer, 1),
                                                            nullptr,
                                                            toDataView(&strides, 1),
                                                            toDataView(&offsets, 1));
}

QPair<QDemonRef<QDemonRenderVertexBuffer>, QDemonRef<QDemonRenderIndexBuffer>> QDemonRendererImpl::getXYQuad()
{
    if (!m_quadInputAssembler)
        generateXYQuad();

    return QPair<QDemonRef<QDemonRenderVertexBuffer>, QDemonRef<QDemonRenderIndexBuffer>>(m_quadVertexBuffer, m_quadIndexBuffer);
}

QDemonLayerGlobalRenderProperties QDemonRendererImpl::getLayerGlobalRenderProperties()
{
    QDemonLayerRenderData &theData = *m_currentLayer;
    const QDemonRenderLayer &theLayer = theData.layer;
    if (!theData.cameraDirection.hasValue())
        theData.cameraDirection = theData.camera->getScalingCorrectDirection();

    return QDemonLayerGlobalRenderProperties{ theLayer,
                                              *theData.camera,
                                              *theData.cameraDirection,
                                              theData.globalLights,
                                              theData.lightDirections,
                                              theData.shadowMapManager,
                                              theData.m_layerDepthTexture,
                                              theData.m_layerSsaoTexture,
                                              theLayer.lightProbe,
                                              theLayer.lightProbe2,
                                              theLayer.probeHorizon,
                                              theLayer.probeBright,
                                              theLayer.probe2Window,
                                              theLayer.probe2Pos,
                                              theLayer.probe2Fade,
                                              theLayer.probeFov };
}

void QDemonRendererImpl::generateXYQuadStrip()
{
    if (m_quadStripInputAssembler)
        return;

    QDemonRenderVertexBufferEntry theEntries[] = {
        QDemonRenderVertexBufferEntry("attr_pos", QDemonRenderComponentType::Float32, 3),
        QDemonRenderVertexBufferEntry("attr_uv", QDemonRenderComponentType::Float32, 2, 12),
    };

    // this buffer is filled dynmically
    m_quadStripVertexBuffer = new QDemonRenderVertexBuffer(m_context, QDemonRenderBufferUsageType::Dynamic,
                                                            3 * sizeof(float) + 2 * sizeof(float), // stride
                                                            QDemonByteView());

    // create our attribute layout
    m_quadStripAttribLayout = m_context->createAttributeLayout(toDataView(theEntries, 2));

    // create input assembler object
    quint32 strides = m_quadStripVertexBuffer->stride();
    quint32 offsets = 0;
    m_quadStripInputAssembler = m_context->createInputAssembler(m_quadStripAttribLayout,
                                                                toDataView(&m_quadStripVertexBuffer, 1),
                                                                nullptr,
                                                                toDataView(&strides, 1),
                                                                toDataView(&offsets, 1));
}

void QDemonRendererImpl::updateCbAoShadow(const QDemonRenderLayer *pLayer, const QDemonRenderCamera *pCamera, QDemonResourceTexture2D &inDepthTexture)
{
    if (m_context->supportsConstantBuffer()) {
        const char *theName = "cbAoShadow";
        QDemonRef<QDemonRenderConstantBuffer> pCB = m_context->getConstantBuffer(theName);

        if (!pCB) {
            // the  size is determined automatically later on
            pCB = new QDemonRenderConstantBuffer(m_context, theName,
                                                  QDemonRenderBufferUsageType::Static,
                                                  QDemonByteView());
            if (!pCB) {
                Q_ASSERT(false);
                return;
            }
            m_constantBuffers.insert(theName, pCB);

            // Add paramters. Note should match the appearance in the shader program
            pCB->addParam("ao_properties", QDemonRenderShaderDataType::Vec4, 1);
            pCB->addParam("ao_properties2", QDemonRenderShaderDataType::Vec4, 1);
            pCB->addParam("shadow_properties", QDemonRenderShaderDataType::Vec4, 1);
            pCB->addParam("aoScreenConst", QDemonRenderShaderDataType::Vec4, 1);
            pCB->addParam("UvToEyeConst", QDemonRenderShaderDataType::Vec4, 1);
        }

        // update values
        QVector4D aoProps(pLayer->aoStrength * 0.01f, pLayer->aoDistance * 0.4f, pLayer->aoSoftness * 0.02f, pLayer->aoBias);
        pCB->updateParam(QByteArrayLiteral("ao_properties"), toByteView(aoProps));
        QVector4D aoProps2(float(pLayer->aoSamplerate), (pLayer->aoDither) ? 1.0f : 0.0f, 0.0f, 0.0f);
        pCB->updateParam(QByteArrayLiteral("ao_properties2"), toByteView(aoProps2));
        QVector4D shadowProps(pLayer->shadowStrength * 0.01f, pLayer->shadowDist, pLayer->shadowSoftness * 0.01f, pLayer->shadowBias);
        pCB->updateParam(QByteArrayLiteral("shadow_properties"), toByteView(shadowProps));

        float R2 = pLayer->aoDistance * pLayer->aoDistance * 0.16f;
        float rw = 100, rh = 100;

        if (inDepthTexture.getTexture()) {
            rw = float(inDepthTexture.getTexture()->textureDetails().width);
            rh = float(inDepthTexture.getTexture()->textureDetails().height);
        }
        float fov = (pCamera) ? pCamera->verticalFov(rw / rh) : 1.0f;
        float tanHalfFovY = tanf(0.5f * fov * (rh / rw));
        float invFocalLenX = tanHalfFovY * (rw / rh);

        QVector4D aoScreenConst(1.0f / R2, rh / (2.0f * tanHalfFovY), 1.0f / rw, 1.0f / rh);
        pCB->updateParam(QByteArrayLiteral("aoScreenConst"), toByteView(aoScreenConst));
        QVector4D UvToEyeConst(2.0f * invFocalLenX, -2.0f * tanHalfFovY, -invFocalLenX, tanHalfFovY);
        pCB->updateParam(QByteArrayLiteral("UvToEyeConst"), toByteView(UvToEyeConst));

        // update buffer to hardware
        pCB->update();
    }
}

// widget context implementation

QDemonRef<QDemonRenderVertexBuffer> QDemonRendererImpl::getOrCreateVertexBuffer(const QByteArray &inStr,
                                                                                quint32 stride,
                                                                                QDemonByteView bufferData)
{
    QDemonRef<QDemonRenderVertexBuffer> retval = getVertexBuffer(inStr);
    if (retval) {
        // we update the buffer
        retval->updateBuffer(bufferData);
        return retval;
    }
    retval = new QDemonRenderVertexBuffer(m_context, QDemonRenderBufferUsageType::Dynamic, stride, bufferData);
    m_widgetVertexBuffers.insert(inStr, retval);
    return retval;
}
QDemonRef<QDemonRenderIndexBuffer> QDemonRendererImpl::getOrCreateIndexBuffer(const QByteArray &inStr,
                                                                              QDemonRenderComponentType componentType,
                                                                              QDemonByteView bufferData)
{
    QDemonRef<QDemonRenderIndexBuffer> retval = getIndexBuffer(inStr);
    if (retval) {
        // we update the buffer
        retval->updateBuffer(bufferData);
        return retval;
    }

    retval = new QDemonRenderIndexBuffer(m_context, QDemonRenderBufferUsageType::Dynamic, componentType, bufferData);
    m_widgetIndexBuffers.insert(inStr, retval);
    return retval;
}

QDemonRef<QDemonRenderAttribLayout> QDemonRendererImpl::createAttributeLayout(QDemonDataView<QDemonRenderVertexBufferEntry> attribs)
{
    // create our attribute layout
    QDemonRef<QDemonRenderAttribLayout> theAttribLayout = m_context->createAttributeLayout(attribs);
    return theAttribLayout;
}

QDemonRef<QDemonRenderInputAssembler> QDemonRendererImpl::getOrCreateInputAssembler(
        const QByteArray &inStr,
        QDemonRef<QDemonRenderAttribLayout> attribLayout,
        QDemonDataView<QDemonRef<QDemonRenderVertexBuffer>> buffers,
        const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
        QDemonDataView<quint32> strides,
        QDemonDataView<quint32> offsets)
{
    QDemonRef<QDemonRenderInputAssembler> retval = getInputAssembler(inStr);
    if (retval)
        return retval;

    retval = m_context->createInputAssembler(attribLayout, buffers, indexBuffer, strides, offsets);
    m_widgetInputAssembler.insert(inStr, retval);
    return retval;
}

QDemonRef<QDemonRenderVertexBuffer> QDemonRendererImpl::getVertexBuffer(const QByteArray &inStr) const
{
    const auto theIter = m_widgetVertexBuffers.constFind(inStr);
    if (theIter != m_widgetVertexBuffers.cend())
        return theIter.value();
    return nullptr;
}

QDemonRef<QDemonRenderIndexBuffer> QDemonRendererImpl::getIndexBuffer(const QByteArray &inStr) const
{
    const auto theIter = m_widgetIndexBuffers.constFind(inStr);
    if (theIter != m_widgetIndexBuffers.cend())
        return theIter.value();
    return nullptr;
}

QDemonRef<QDemonRenderInputAssembler> QDemonRendererImpl::getInputAssembler(const QByteArray &inStr) const
{
    const auto theIter = m_widgetInputAssembler.constFind(inStr);
    if (theIter != m_widgetInputAssembler.cend())
        return theIter.value();
    return nullptr;
}

QDemonRef<QDemonRenderShaderProgram> QDemonRendererImpl::getShader(const QByteArray &inStr) const
{
    const auto theIter = m_widgetShaders.constFind(inStr);
    if (theIter != m_widgetShaders.cend())
        return theIter.value();
    return nullptr;
}

QDemonRef<QDemonRenderShaderProgram> QDemonRendererImpl::compileAndStoreShader(const QByteArray &inStr)
{
    QDemonRef<QDemonRenderShaderProgram> newProgram = getProgramGenerator()->compileGeneratedShader(inStr);
    if (newProgram)
        m_widgetShaders.insert(inStr, newProgram);
    return newProgram;
}

QDemonRef<QDemonShaderProgramGeneratorInterface> QDemonRendererImpl::getProgramGenerator()
{
    return m_demonContext->shaderProgramGenerator();
}

void QDemonRendererImpl::dumpGpuProfilerStats()
{
    if (!isLayerGpuProfilingEnabled())
        return;

    auto it = m_instanceRenderMap.cbegin();
    const auto end = m_instanceRenderMap.cend();
    for (; it != end; it++) {
        const QDemonRef<QDemonLayerRenderData> &theLayerRenderData = it.value();
        const QDemonRenderLayer *theLayer = &theLayerRenderData->layer;

        if (theLayer->flags.testFlag(QDemonRenderLayer::Flag::Active) && theLayerRenderData->m_layerProfilerGpu) {
            const QVector<QString> &idList = theLayerRenderData->m_layerProfilerGpu->timerIDs();
            if (!idList.empty()) {
#if QDEMON_DEBUG_ID
                qDebug() << theLayer->id;
#endif
                auto theIdIter = idList.begin();
                for (; theIdIter != idList.end(); theIdIter++) {
                    char messageLine[1024];
                    sprintf(messageLine,
                            "%s: %.3f ms",
                            theIdIter->toLatin1().constData(),
                            theLayerRenderData->m_layerProfilerGpu->elapsed(*theIdIter));
                    qDebug() << "    " << messageLine;
                }
            }
        }
    }
}

// Given a node and a point in the node's local space (most likely its pivot point), we return
// a normal matrix so you can get the axis out, a transformation from node to camera
// a new position and a floating point scale factor so you can render in 1/2 perspective mode
// or orthographic mode if you would like to.
QDemonWidgetRenderInformation QDemonRendererImpl::getWidgetRenderInformation(QDemonRenderNode &inNode,
                                                                             const QVector3D &inPos,
                                                                             RenderWidgetModes inWidgetMode)
{
    QDemonRef<QDemonLayerRenderData> theData = getOrCreateLayerRenderDataForNode(inNode);
    QDemonRenderCamera *theCamera = theData->camera;
    if (Q_UNLIKELY(theCamera == nullptr || theData->layerPrepResult.hasValue() == false)) {
        Q_ASSERT(false);
        return QDemonWidgetRenderInformation();
    }
    QMatrix4x4 theGlobalTransform;
    if (inNode.parent != nullptr && inNode.parent->type != QDemonRenderGraphObject::Type::Layer && !inNode.flags.testFlag(QDemonRenderNode::Flag::IgnoreParentTransform))
        theGlobalTransform = inNode.parent->globalTransform;
    QMatrix4x4 theCameraInverse = mat44::getInverse(theCamera->globalTransform);
    QMatrix4x4 theNodeParentToCamera;
    if (inWidgetMode == RenderWidgetModes::Local)
        theNodeParentToCamera = theCameraInverse * theGlobalTransform;
    else
        theNodeParentToCamera = theCameraInverse;

    const float normalMatData[9] = { theNodeParentToCamera(0, 0), theNodeParentToCamera(0, 1), theNodeParentToCamera(0, 2),
                                     theNodeParentToCamera(1, 0), theNodeParentToCamera(1, 1), theNodeParentToCamera(1, 2),
                                     theNodeParentToCamera(2, 0), theNodeParentToCamera(2, 1), theNodeParentToCamera(2, 2) };

    QMatrix3x3 theNormalMat(normalMatData);
    theNormalMat = mat33::getInverse(theNormalMat).transposed();
    QVector3D column0(theNormalMat(0, 0), theNormalMat(0, 1), theNormalMat(0, 2));
    QVector3D column1(theNormalMat(1, 0), theNormalMat(1, 1), theNormalMat(1, 2));
    QVector3D column2(theNormalMat(2, 0), theNormalMat(2, 1), theNormalMat(2, 2));
    column0.normalize();
    column1.normalize();
    column2.normalize();
    float normalizedMatData[9] = { column0.x(), column0.y(), column0.z(), column1.x(), column1.y(),
                                   column1.z(), column2.x(), column2.y(), column2.z() };

    theNormalMat = QMatrix3x3(normalizedMatData);

    QMatrix4x4 theTranslation;
    theTranslation(3, 0) = inNode.position.x();
    theTranslation(3, 1) = inNode.position.y();
    theTranslation(3, 2) = inNode.position.z();
    theTranslation(3, 2) *= -1.0f;

    theGlobalTransform = theGlobalTransform * theTranslation;

    QMatrix4x4 theNodeToParentPlusTranslation = theCameraInverse * theGlobalTransform;
    QVector3D thePos = mat44::transform(theNodeToParentPlusTranslation, inPos);
    QDemonScaleAndPosition theScaleAndPos = worldToPixelScaleFactor(*theCamera, thePos, *theData);
    QMatrix3x3 theLookAtMatrix;
    if (!theCamera->flags.testFlag(QDemonRenderCamera::Flag::Orthographic)) {
        QVector3D theNodeToCamera = theScaleAndPos.position;
        theNodeToCamera.normalize();
        QVector3D theOriginalAxis = QVector3D(0, 0, -1);
        QVector3D theRotAxis = QVector3D::crossProduct(theOriginalAxis, theNodeToCamera);
        theRotAxis.normalize();
        float theAxisLen = vec3::magnitude(theRotAxis);
        if (theAxisLen > .05f) {
            float theRotAmount = std::acos(QVector3D::dotProduct(theOriginalAxis, theNodeToCamera));
            QQuaternion theQuat(theRotAmount, theRotAxis);
            theLookAtMatrix = theQuat.toRotationMatrix();
        }
    }
    QVector3D thePosInWorldSpace = mat44::transform(theGlobalTransform, inPos);
    QVector3D theCameraPosInWorldSpace = theCamera->getGlobalPos();
    QVector3D theCameraOffset = thePosInWorldSpace - theCameraPosInWorldSpace;
    QVector3D theDir = theCameraOffset;
    theDir.normalize();
    // Things should be 600 units from the camera, as that is how all of our math is setup.
    theCameraOffset = 600.0f * theDir;
    return QDemonWidgetRenderInformation(theNormalMat,
                                         theNodeParentToCamera,
                                         theCamera->projection,
                                         theCamera->projection,
                                         theLookAtMatrix,
                                         theCameraInverse,
                                         theCameraOffset,
                                         theScaleAndPos.position,
                                         theScaleAndPos.scale,
                                         *theCamera);
}

QDemonOption<QVector2D> QDemonRendererImpl::getLayerMouseCoords(QDemonRenderLayer &inLayer,
                                                                const QVector2D &inMouseCoords,
                                                                const QVector2D &inViewportDimensions,
                                                                bool forceImageIntersect) const
{
    QDemonRef<QDemonLayerRenderData> theData = const_cast<QDemonRendererImpl &>(*this).getOrCreateLayerRenderDataForNode(inLayer);
    return getLayerMouseCoords(*theData, inMouseCoords, inViewportDimensions, forceImageIntersect);
}

bool QDemonRendererInterface::isGlEsContext(const QDemonRenderContextType &inContextType)
{
    QDemonRenderContextTypes esContextTypes(QDemonRenderContextType::GLES2 | QDemonRenderContextType::GLES3
                                           | QDemonRenderContextType::GLES3PLUS);

    return (esContextTypes & inContextType);
}

bool QDemonRendererInterface::isGlEs3Context(const QDemonRenderContextType &inContextType)
{
    return (inContextType == QDemonRenderContextType::GLES3 || inContextType == QDemonRenderContextType::GLES3PLUS);
}

bool QDemonRendererInterface::isGl2Context(const QDemonRenderContextType &inContextType)
{
    return (inContextType == QDemonRenderContextType::GL2);
}

QDemonRef<QDemonRendererInterface> QDemonRendererInterface::createRenderer(QDemonRenderContextInterface *inContext)
{
    return QDemonRef<QDemonRendererImpl>(new QDemonRendererImpl(inContext));
}

QDemonRenderPickSubResult::QDemonRenderPickSubResult() : m_subRenderer(nullptr), m_nextSibling(nullptr) {}

QDemonRenderPickSubResult::QDemonRenderPickSubResult(const QDemonRef<QDemonOffscreenRendererInterface> &inSubRenderer,
                                                     const QMatrix4x4 &inTextureMatrix,
                                                     QDemonRenderTextureCoordOp inHorizontalTilingMode,
                                                     QDemonRenderTextureCoordOp inVerticalTilingMode,
                                                     qint32 width,
                                                     qint32 height)
    : m_subRenderer(inSubRenderer)
    , m_textureMatrix(inTextureMatrix)
    , m_horizontalTilingMode(inHorizontalTilingMode)
    , m_verticalTilingMode(inVerticalTilingMode)
    , m_viewportWidth(qMax(0, width))
    , m_viewportHeight(qMax(0, height))
    , m_nextSibling(nullptr)
{
}

QDemonRenderPickSubResult::~QDemonRenderPickSubResult() = default;

QDemonRenderPickResult::QDemonRenderPickResult(const QDemonRenderGraphObject &inHitObject, float inCameraDistance, const QVector2D &inLocalUVCoords)
    : m_hitObject(&inHitObject)
    , m_cameraDistanceSq(inCameraDistance)
    , m_localUVCoords(inLocalUVCoords)
    , m_firstSubObject(nullptr)
{
}

QT_END_NAMESPACE
