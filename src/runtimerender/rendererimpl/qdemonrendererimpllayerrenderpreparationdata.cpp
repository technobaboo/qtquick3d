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
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrendereffect.h>
#include <QtDemonRuntimeRender/qdemonrenderlight.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrenderscene.h>
#include <QtDemonRuntimeRender/qdemonrenderpresentation.h>
#include <Qt3DSFoundation.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <qdemontextrenderer.h>
#include <qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <qdemonoffscreenrenderkey.h>
#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
#include <qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <Qt3DSPerfTimer.h>
#include <AutoDeallocatorAllocator.h>
#include <qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <qdemonrendercustommaterialsystem.h>
#include <qdemonrendertexttexturecache.h>
#include <qdemonrendertexttextureatlas.h>
#include <qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <qdemonrenderpathmanager.h>

#ifdef _WIN32
#pragma warning(disable : 4355)
#endif

QT_BEGIN_NAMESPACE

using eastl::reverse;
using eastl::stable_sort;
using QDemonRenderContextScopedProperty;
using QVector2D;

namespace {
void MaybeQueueNodeForRender(SNode &inNode, QVector<SRenderableNodeEntry> &outRenderables,
                             QVector<SNode *> &outCamerasAndLights, quint32 &ioDFSIndex)
{
    ++ioDFSIndex;
    inNode.m_DFSIndex = ioDFSIndex;
    if (GraphObjectTypes::IsRenderableType(inNode.m_Type))
        outRenderables.push_back(inNode);
    else if (GraphObjectTypes::IsLightCameraType(inNode.m_Type))
        outCamerasAndLights.push_back(&inNode);

    for (SNode *theChild = inNode.m_FirstChild; theChild != nullptr;
         theChild = theChild->m_NextSibling)
        MaybeQueueNodeForRender(*theChild, outRenderables, outCamerasAndLights, ioDFSIndex);
}

bool HasValidLightProbe(SImage *inLightProbeImage)
{
    return inLightProbeImage && inLightProbeImage->m_TextureData.m_Texture;
}
}

SDefaultMaterialPreparationResult::SDefaultMaterialPreparationResult(
        SShaderDefaultMaterialKey inKey)
    : m_FirstImage(nullptr)
    , m_Opacity(1.0f)
    , m_MaterialKey(inKey)
    , m_Dirty(false)
{
}

#define MAX_AA_LEVELS 8

SLayerRenderPreparationData::SLayerRenderPreparationData(SLayer &inLayer,
                                                         Qt3DSRendererImpl &inRenderer)
    : m_Layer(inLayer)
    , m_Renderer(inRenderer)
    , m_Allocator(inRenderer.GetContext().GetAllocator())
    , m_RenderableNodeLightEntryPool(
          ForwardingAllocator(inRenderer.GetContext().GetAllocator(),
                              "SLayerRenderPreparationData::m_RenderableNodes"))
    , m_RenderableNodes(inRenderer.GetContext().GetAllocator(),
                        "SLayerRenderPreparationData::m_RenderableNodes")
    , m_LightToNodeMap(inRenderer.GetContext().GetAllocator(),
                       "SLayerRenderPreparationData::m_LightToNodeMap")
    , m_CamerasAndLights(inRenderer.GetContext().GetAllocator(),
                         "SLayerRenderPreparationData::m_CamerasAndLights")
    , m_Camera(nullptr)
    , m_Lights(inRenderer.GetContext().GetAllocator(), "SLayerRenderPreparationData::m_Lights")
    , m_OpaqueObjects(inRenderer.GetContext().GetAllocator(),
                      "SLayerRenderPreparationData::m_OpaqueObjects")
    , m_TransparentObjects(inRenderer.GetContext().GetAllocator(),
                           "SLayerRenderPreparationData::m_TransparentObjects")
    , m_RenderedOpaqueObjects(inRenderer.GetContext().GetAllocator(),
                              "SLayerRenderPreparationData::m_RenderedOpaqueObjects")
    , m_RenderedTransparentObjects(inRenderer.GetContext().GetAllocator(),
                                   "SLayerRenderPreparationData::m_RenderedTransparentObjects")
    , m_IRenderWidgets(inRenderer.GetContext().GetAllocator(),
                       "SLayerRenderPreparationData::m_IRenderWidgets")
    , m_SourceLightDirections(inRenderer.GetContext().GetAllocator(),
                              "SLayerRenderPreparationData::m_SourceLightDirections")
    , m_LightDirections(inRenderer.GetContext().GetAllocator(),
                        "SLayerRenderPreparationData::m_LightDirections")
    , m_ModelContexts(inRenderer.GetContext().GetAllocator(),
                      "SLayerRenderPreparationData::m_ModelContexts")
    , m_CGLightingFeatureName(QStringLiterial("QDEMON_ENABLE_CG_LIGHTING"))
    , m_FeaturesDirty(true)
    , m_FeatureSetHash(0)
    , m_TooManyLightsError(false)
{
}

SLayerRenderPreparationData::~SLayerRenderPreparationData() {}

bool SLayerRenderPreparationData::NeedsWidgetTexture() const
{
    return m_IRenderWidgets.size() > 0;
}

void SLayerRenderPreparationData::SetShaderFeature(QString theStr, bool inValue)
{
    SShaderPreprocessorFeature item(theStr, inValue);
    eastl::vector<SShaderPreprocessorFeature>::iterator iter = m_Features.begin(),
            end = m_Features.end();

    // empty loop intentional.
    for (; iter != end && ((iter->m_Name == theStr) == false); ++iter)
        ;

    if (iter != end) {
        if (iter->m_Enabled != inValue) {
            iter->m_Enabled = inValue;
            m_FeaturesDirty = true;
            m_FeatureSetHash = 0;
        }
    } else {
        m_Features.push_back(item);
        m_FeaturesDirty = true;
        m_FeatureSetHash = 0;
    }
}

void SLayerRenderPreparationData::SetShaderFeature(const char *inName, bool inValue)
{
    QString theStr(QString::fromLocal8Bit(inName));
    SetShaderFeature(theStr, inValue);
}

QDemonConstDataRef<SShaderPreprocessorFeature> SLayerRenderPreparationData::GetShaderFeatureSet()
{
    if (m_FeaturesDirty) {
        eastl::sort(m_Features.begin(), m_Features.end());
        m_FeaturesDirty = false;
    }
    return toConstDataRef(m_Features.data(), (quint32)m_Features.size());
}

size_t SLayerRenderPreparationData::GetShaderFeatureSetHash()
{
    if (!m_FeatureSetHash)
        m_FeatureSetHash = HashShaderFeatureSet(GetShaderFeatureSet());
    return m_FeatureSetHash;
}

bool SLayerRenderPreparationData::GetShadowMapManager()
{
    if (m_ShadowMapManager.mPtr)
        return true;

    m_ShadowMapManager.mPtr = Qt3DSShadowMap::Create(m_Renderer.GetQt3DSContext());

    return m_ShadowMapManager.mPtr != nullptr;
}

bool SLayerRenderPreparationData::GetOffscreenRenderer()
{
    if (m_LastFrameOffscreenRenderer.mPtr)
        return true;

    if (m_Layer.m_RenderPlugin && m_Layer.m_RenderPlugin->m_Flags.IsActive()) {
        IRenderPluginInstance *theInstance =
                m_Renderer.GetQt3DSContext().GetRenderPluginManager().GetOrCreateRenderPluginInstance(
                    m_Layer.m_RenderPlugin->m_PluginPath, m_Layer.m_RenderPlugin);
        if (theInstance) {
            m_Renderer.GetQt3DSContext()
                    .GetOffscreenRenderManager()
                    .MaybeRegisterOffscreenRenderer(&theInstance, *theInstance);
            m_LastFrameOffscreenRenderer = theInstance;
        }
    }
    if (m_LastFrameOffscreenRenderer.mPtr == nullptr)
        m_LastFrameOffscreenRenderer =
                m_Renderer.GetQt3DSContext().GetOffscreenRenderManager().GetOffscreenRenderer(
                    m_Layer.m_TexturePath);
    return m_LastFrameOffscreenRenderer.mPtr != nullptr;
}

QVector3D SLayerRenderPreparationData::GetCameraDirection()
{
    if (m_CameraDirection.hasValue() == false) {
        if (m_Camera)
            m_CameraDirection = m_Camera->GetScalingCorrectDirection();
        else
            m_CameraDirection = QVector3D(0, 0, -1);
    }
    return *m_CameraDirection;
}

// Per-frame cache of renderable objects post-sort.
QDemonDataRef<SRenderableObject *> SLayerRenderPreparationData::GetOpaqueRenderableObjects()
{
    if (m_RenderedOpaqueObjects.empty() == false || m_Camera == nullptr)
        return m_RenderedOpaqueObjects;
    if (m_Layer.m_Flags.IsLayerEnableDepthTest() && m_OpaqueObjects.empty() == false) {
        QVector3D theCameraDirection(GetCameraDirection());
        QVector3D theCameraPosition = m_Camera->GetGlobalPos();
        m_RenderedOpaqueObjects.assign(m_OpaqueObjects.begin(), m_OpaqueObjects.end());
        // Setup the object's sorting information
        for (quint32 idx = 0, end = m_RenderedOpaqueObjects.size(); idx < end; ++idx) {
            SRenderableObject &theInfo = *m_RenderedOpaqueObjects[idx];
            QVector3D difference = theInfo.m_WorldCenterPoint - theCameraPosition;
            theInfo.m_CameraDistanceSq = difference.dot(theCameraDirection);
        }

        ForwardingAllocator alloc(m_Renderer.GetPerFrameAllocator(), "SortAllocations");
        // Render nearest to furthest objects
        eastl::merge_sort(m_RenderedOpaqueObjects.begin(), m_RenderedOpaqueObjects.end(), alloc,
                          ISRenderObjectPtrLessThan);
    }
    return m_RenderedOpaqueObjects;
}

// If layer depth test is false, this may also contain opaque objects.
QDemonDataRef<SRenderableObject *> SLayerRenderPreparationData::GetTransparentRenderableObjects()
{
    if (m_RenderedTransparentObjects.empty() == false || m_Camera == nullptr)
        return m_RenderedTransparentObjects;

    m_RenderedTransparentObjects.assign(m_TransparentObjects.begin(),
                                        m_TransparentObjects.end());

    if (m_Layer.m_Flags.IsLayerEnableDepthTest() == false)
        m_RenderedTransparentObjects.insert(m_RenderedTransparentObjects.end(),
                                            m_OpaqueObjects.begin(), m_OpaqueObjects.end());

    if (m_RenderedTransparentObjects.empty() == false) {
        QVector3D theCameraDirection(GetCameraDirection());
        QVector3D theCameraPosition = m_Camera->GetGlobalPos();

        // Setup the object's sorting information
        for (quint32 idx = 0, end = m_RenderedTransparentObjects.size(); idx < end; ++idx) {
            SRenderableObject &theInfo = *m_RenderedTransparentObjects[idx];
            QVector3D difference = theInfo.m_WorldCenterPoint - theCameraPosition;
            theInfo.m_CameraDistanceSq = difference.dot(theCameraDirection);
        }
        ForwardingAllocator alloc(m_Renderer.GetPerFrameAllocator(), "SortAllocations");
        // render furthest to nearest.
        eastl::merge_sort(m_RenderedTransparentObjects.begin(),
                          m_RenderedTransparentObjects.end(), alloc,
                          ISRenderObjectPtrGreatThan);
    }

    return m_RenderedTransparentObjects;
}

#define MAX_LAYER_WIDGETS 200

void SLayerRenderPreparationData::AddRenderWidget(IRenderWidget &inWidget)
{
    // The if the layer is not active then the widget can't be displayed.
    // Furthermore ResetForFrame won't be called below which leads to stale
    // widgets in the m_IRenderWidgets array.  These stale widgets would get rendered
    // the next time the layer was active potentially causing a crash.
    if (!m_Layer.m_Flags.IsActive())
        return;

    // Ensure we clear the widget layer always
    m_Renderer.LayerNeedsFrameClear(*static_cast<SLayerRenderData *>(this));

    if (m_IRenderWidgets.size() < MAX_LAYER_WIDGETS)
        m_IRenderWidgets.push_back(&inWidget);
}

#define QDEMON_RENDER_MINIMUM_RENDER_OPACITY .01f

SShaderDefaultMaterialKey
SLayerRenderPreparationData::GenerateLightingKey(DefaultMaterialLighting::Enum inLightingType)
{
    SShaderDefaultMaterialKey theGeneratedKey(GetShaderFeatureSetHash());
    const bool lighting = inLightingType != DefaultMaterialLighting::NoLighting;
    m_Renderer.DefaultMaterialShaderKeyProperties().m_HasLighting.SetValue(theGeneratedKey,
                                                                           lighting);
    if (lighting) {
        const bool lightProbe = m_Layer.m_LightProbe
                && m_Layer.m_LightProbe->m_TextureData.m_Texture;
        m_Renderer.DefaultMaterialShaderKeyProperties().m_HasIbl.SetValue(theGeneratedKey,
                                                                          lightProbe);

        quint32 numLights = (quint32)m_Lights.size();
        if (numLights > SShaderDefaultMaterialKeyProperties::LightCount
                && m_TooManyLightsError == false) {
            m_TooManyLightsError = true;
            numLights = SShaderDefaultMaterialKeyProperties::LightCount;
            qCCritical(INVALID_OPERATION, "Too many lights on layer, max is 7");
            Q_ASSERT(false);
        }
        m_Renderer.DefaultMaterialShaderKeyProperties().m_LightCount.SetValue(theGeneratedKey,
                                                                              numLights);

        for (quint32 lightIdx = 0, lightEnd = m_Lights.size();
             lightIdx < lightEnd; ++lightIdx) {
            SLight *theLight(m_Lights[lightIdx]);
            const bool isDirectional = theLight->m_LightType == RenderLightTypes::Directional;
            const bool isArea = theLight->m_LightType == RenderLightTypes::Area;
            const bool castShadowsArea = (theLight->m_LightType != RenderLightTypes::Area)
                    && (theLight->m_CastShadow);

            m_Renderer.DefaultMaterialShaderKeyProperties().m_LightFlags[lightIdx]
                    .SetValue(theGeneratedKey, !isDirectional);
            m_Renderer.DefaultMaterialShaderKeyProperties().m_LightAreaFlags[lightIdx]
                    .SetValue(theGeneratedKey, isArea);
            m_Renderer.DefaultMaterialShaderKeyProperties().m_LightShadowFlags[lightIdx]
                    .SetValue(theGeneratedKey, castShadowsArea);
        }
    }
    return theGeneratedKey;
}

bool SLayerRenderPreparationData::PrepareTextForRender(
        SText &inText, const QMatrix4x4 &inViewProjection, float inTextScaleFactor,
        SLayerRenderPreparationResultFlags &ioFlags)
{
    ITextTextureCache *theTextRenderer = m_Renderer.GetQt3DSContext().GetTextureCache();
    if (theTextRenderer == nullptr)
        return false;

    SRenderableObjectFlags theFlags;
    theFlags.SetHasTransparency(true);
    theFlags.SetCompletelyTransparent(inText.m_GlobalOpacity < .01f);
    theFlags.SetPickable(true);
    bool retval = false;

    if (theFlags.IsCompletelyTransparent() == false) {
        retval = inText.m_Flags.IsDirty() || inText.m_Flags.IsTextDirty();
        inText.m_Flags.SetTextDirty(false);
        TTPathObjectAndTexture theResult =
                theTextRenderer->RenderText(inText, inTextScaleFactor);
        inText.m_TextTexture = theResult.second.second.mPtr;
        inText.m_TextTextureDetails = theResult.second.first;
        inText.m_PathFontItem = theResult.first.second;
        inText.m_PathFontDetails = theResult.first.first;
        STextScaleAndOffset theScaleAndOffset(*inText.m_TextTexture,
                                              inText.m_TextTextureDetails, inText);
        QVector2D theTextScale(theScaleAndOffset.m_TextScale);
        QVector2D theTextOffset(theScaleAndOffset.m_TextOffset);
        QVector3D minimum(theTextOffset[0] - theTextScale[0], theTextOffset[1] - theTextScale[1],
                0);
        QVector3D maximum(theTextOffset[0] + theTextScale[0], theTextOffset[1] + theTextScale[1],
                0);
        inText.m_Bounds = QDemonBounds3(minimum, maximum);
        QMatrix4x4 theMVP;
        QMatrix3x3 theNormalMatrix;
        inText.CalculateMVPAndNormalMatrix(inViewProjection, theMVP, theNormalMatrix);

        if (inText.m_PathFontDetails)
            ioFlags.SetRequiresStencilBuffer(true);

        STextRenderable *theRenderable = new STextRenderable(
                    theFlags, inText.GetGlobalPos(), m_Renderer, inText, inText.m_Bounds, theMVP,
                    inViewProjection, *inText.m_TextTexture, theTextOffset, theTextScale);
        m_TransparentObjects.push_back(theRenderable);
    }
    return retval;
}

QPair<bool, SGraphObject *>
SLayerRenderPreparationData::ResolveReferenceMaterial(SGraphObject *inMaterial)
{
    bool subsetDirty = false;
    bool badIdea = false;
    SGraphObject *theSourceMaterialObject(inMaterial);
    SGraphObject *theMaterialObject(inMaterial);
    while (theMaterialObject
           && theMaterialObject->m_Type == GraphObjectTypes::ReferencedMaterial && !badIdea) {
        SReferencedMaterial *theRefMaterial =
                static_cast<SReferencedMaterial *>(theMaterialObject);
        theMaterialObject = theRefMaterial->m_ReferencedMaterial;
        if (theMaterialObject == theSourceMaterialObject) {
            badIdea = true;
        }

        if (theRefMaterial == theSourceMaterialObject) {
            theRefMaterial->m_Dirty.UpdateDirtyForFrame();
        }
        subsetDirty = subsetDirty | theRefMaterial->m_Dirty.IsDirty();
    }
    if (badIdea) {
        theMaterialObject = nullptr;
    }
    return QPair<bool, SGraphObject *>(subsetDirty, theMaterialObject);
}

bool SLayerRenderPreparationData::PreparePathForRender(
        SPath &inPath, const QMatrix4x4 &inViewProjection,
        const QDemonOption<SClippingFrustum> &inClipFrustum, SLayerRenderPreparationResultFlags &ioFlags)
{
    SRenderableObjectFlags theSharedFlags;
    theSharedFlags.SetPickable(true);
    float subsetOpacity = inPath.m_GlobalOpacity;
    bool retval = inPath.m_Flags.IsDirty();
    inPath.m_Flags.SetDirty(false);
    QMatrix4x4 theMVP;
    QMatrix3x3 theNormalMatrix;

    inPath.CalculateMVPAndNormalMatrix(inViewProjection, theMVP, theNormalMatrix);
    QDemonBounds3 theBounds(this->m_Renderer.GetQt3DSContext().GetPathManager().GetBounds(inPath));

    if (inPath.m_GlobalOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY
            && inClipFrustum.hasValue()) {
        // Check bounding box against the clipping planes
        QDemonBounds3 theGlobalBounds = theBounds;
        theGlobalBounds.transform(inPath.m_GlobalTransform);
        if (inClipFrustum->intersectsWith(theGlobalBounds) == false)
            subsetOpacity = 0.0f;
    }

    SGraphObject *theMaterials[2] = { inPath.m_Material, inPath.m_SecondMaterial };

    if (inPath.m_PathType == PathTypes::Geometry
            || inPath.m_PaintStyle != PathPaintStyles::FilledAndStroked)
        theMaterials[1] = nullptr;

    // We need to fill material to be the first one rendered so the stroke goes on top.
    // In the timeline, however, this is reversed.

    if (theMaterials[1])
        eastl::swap(theMaterials[1], theMaterials[0]);

    for (quint32 idx = 0, end = 2; idx < end; ++idx) {
        if (theMaterials[idx] == nullptr)
            continue;

        SRenderableObjectFlags theFlags = theSharedFlags;

        QPair<bool, SGraphObject *> theMaterialAndDirty(
                    ResolveReferenceMaterial(theMaterials[idx]));
        SGraphObject *theMaterial(theMaterialAndDirty.second);
        retval = retval || theMaterialAndDirty.first;

        if (theMaterial != nullptr && theMaterial->m_Type == GraphObjectTypes::DefaultMaterial) {
            SDefaultMaterial *theDefaultMaterial = static_cast<SDefaultMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            bool clearMaterialFlags = theMaterial == inPath.m_Material;
            SDefaultMaterialPreparationResult prepResult(PrepareDefaultMaterialForRender(
                                                             *theDefaultMaterial, theFlags, subsetOpacity, clearMaterialFlags));

            theFlags = prepResult.m_RenderableFlags;
            if (inPath.m_PathType == PathTypes::Geometry) {
                if ((inPath.m_BeginCapping != PathCapping::Noner
                     && inPath.m_BeginCapOpacity < 1.0f)
                        || (inPath.m_EndCapping != PathCapping::Noner
                            && inPath.m_EndCapOpacity < 1.0f))
                    theFlags.SetHasTransparency(true);
            } else {
                ioFlags.SetRequiresStencilBuffer(true);
            }
            retval = retval || prepResult.m_Dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_PathType == PathTypes::Painted) {
                if (inPath.m_PaintStyle == PathPaintStyles::Filled
                        || inPath.m_PaintStyle == PathPaintStyles::FilledAndStroked)
                    isStroke = false;
            }

            SPathRenderable *theRenderable = new SPathRenderable(
                        theFlags, inPath.GetGlobalPos(), m_Renderer, inPath.m_GlobalTransform,
                        theBounds, inPath, theMVP, theNormalMatrix, *theMaterial, prepResult.m_Opacity,
                        prepResult.m_MaterialKey, isStroke);
            theRenderable->m_FirstImage = prepResult.m_FirstImage;

            IQDemonRenderContext &qt3dsContext(m_Renderer.GetQt3DSContext());
            IPathManager &thePathManager = qt3dsContext.GetPathManager();
            retval = thePathManager.PrepareForRender(inPath) || retval;
            retval |= (inPath.m_WireframeMode != qt3dsContext.GetWireframeMode());
            inPath.m_WireframeMode = qt3dsContext.GetWireframeMode();

            if (theFlags.HasTransparency())
                m_TransparentObjects.push_back(theRenderable);
            else
                m_OpaqueObjects.push_back(theRenderable);
        } else if (theMaterial != nullptr
                   && theMaterial->m_Type == GraphObjectTypes::CustomMaterial) {
            SCustomMaterial *theCustomMaterial = static_cast<SCustomMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            // bool clearMaterialFlags = theMaterial == inPath.m_Material;
            SDefaultMaterialPreparationResult prepResult(
                        PrepareCustomMaterialForRender(*theCustomMaterial, theFlags, subsetOpacity));

            theFlags = prepResult.m_RenderableFlags;
            if (inPath.m_PathType == PathTypes::Geometry) {
                if ((inPath.m_BeginCapping != PathCapping::Noner
                     && inPath.m_BeginCapOpacity < 1.0f)
                        || (inPath.m_EndCapping != PathCapping::Noner
                            && inPath.m_EndCapOpacity < 1.0f))
                    theFlags.SetHasTransparency(true);
            } else {
                ioFlags.SetRequiresStencilBuffer(true);
            }

            retval = retval || prepResult.m_Dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_PathType == PathTypes::Painted) {
                if (inPath.m_PaintStyle == PathPaintStyles::Filled
                        || inPath.m_PaintStyle == PathPaintStyles::FilledAndStroked)
                    isStroke = false;
            }

            SPathRenderable *theRenderable = new SPathRenderable(
                        theFlags, inPath.GetGlobalPos(), m_Renderer, inPath.m_GlobalTransform,
                        theBounds, inPath, theMVP, theNormalMatrix, *theMaterial, prepResult.m_Opacity,
                        prepResult.m_MaterialKey, isStroke);
            theRenderable->m_FirstImage = prepResult.m_FirstImage;

            IQDemonRenderContext &qt3dsContext(m_Renderer.GetQt3DSContext());
            IPathManager &thePathManager = qt3dsContext.GetPathManager();
            retval = thePathManager.PrepareForRender(inPath) || retval;
            retval |= (inPath.m_WireframeMode != qt3dsContext.GetWireframeMode());
            inPath.m_WireframeMode = qt3dsContext.GetWireframeMode();

            if (theFlags.HasTransparency())
                m_TransparentObjects.push_back(theRenderable);
            else
                m_OpaqueObjects.push_back(theRenderable);
        }
    }
    return retval;
}

void SLayerRenderPreparationData::PrepareImageForRender(
        SImage &inImage, ImageMapTypes::Enum inMapType, SRenderableImage *&ioFirstImage,
        SRenderableImage *&ioNextImage, SRenderableObjectFlags &ioFlags,
        SShaderDefaultMaterialKey &inShaderKey, quint32 inImageIndex)
{
    IQDemonRenderContext &qt3dsContext(m_Renderer.GetQt3DSContext());
    IBufferManager &bufferManager = qt3dsContext.GetBufferManager();
    IOffscreenRenderManager &theOffscreenRenderManager(
                qt3dsContext.GetOffscreenRenderManager());
    IRenderPluginManager &theRenderPluginManager(qt3dsContext.GetRenderPluginManager());
    if (inImage.ClearDirty(bufferManager, theOffscreenRenderManager, theRenderPluginManager))
        ioFlags |= RenderPreparationResultFlagValues::Dirty;

    // All objects with offscreen renderers are pickable so we can pass the pick through to the
    // offscreen renderer and let it deal with the pick.
    if (inImage.m_LastFrameOffscreenRenderer != nullptr) {
        ioFlags.SetPickable(true);
        ioFlags |= RenderPreparationResultFlagValues::HasTransparency;
    }

    if (inImage.m_TextureData.m_Texture) {
        if (inImage.m_TextureData.m_TextureFlags.HasTransparency()
                && (inMapType == ImageMapTypes::Diffuse
                    || inMapType == ImageMapTypes::Opacity
                    || inMapType == ImageMapTypes::Translucency)) {
            ioFlags |= RenderPreparationResultFlagValues::HasTransparency;
        }
        // Textures used in general have linear characteristics.
        // PKC -- The filters are properly set already.  Setting them here only overrides what
        // would
        // otherwise be a correct setting.
        // inImage.m_TextureData.m_Texture->SetMinFilter( QDemonRenderTextureMinifyingOp::Linear );
        // inImage.m_TextureData.m_Texture->SetMagFilter( QDemonRenderTextureMagnifyingOp::Linear );

        SRenderableImage *theImage = new SRenderableImage(inMapType, inImage);
        SShaderKeyImageMap &theKeyProp =
                m_Renderer.DefaultMaterialShaderKeyProperties().m_ImageMaps[inImageIndex];

        theKeyProp.SetEnabled(inShaderKey, true);
        switch (inImage.m_MappingMode) {
        default:
            Q_ASSERT(false);
            // fallthrough intentional
        case ImageMappingModes::Normal:
            break;
        case ImageMappingModes::Environment:
            theKeyProp.SetEnvMap(inShaderKey, true);
            break;
        case ImageMappingModes::LightProbe:
            theKeyProp.SetLightProbe(inShaderKey, true);
            break;
        }

        if (inImage.m_TextureData.m_TextureFlags.IsInvertUVCoords())
            theKeyProp.SetInvertUVMap(inShaderKey, true);
        if (ioFirstImage == nullptr)
            ioFirstImage = theImage;
        else
            ioNextImage->m_NextImage = theImage;

        // assume offscreen renderer produces non-premultiplied image
        if (inImage.m_LastFrameOffscreenRenderer == nullptr
                && inImage.m_TextureData.m_TextureFlags.IsPreMultiplied())
            theKeyProp.SetPremultiplied(inShaderKey, true);

        SShaderKeyTextureSwizzle &theSwizzleKeyProp =
                m_Renderer.DefaultMaterialShaderKeyProperties().m_TextureSwizzle[inImageIndex];
        theSwizzleKeyProp.SetSwizzleMode(
                    inShaderKey, inImage.m_TextureData.m_Texture->GetTextureSwizzleMode(), true);

        ioNextImage = theImage;
    }
}

SDefaultMaterialPreparationResult SLayerRenderPreparationData::PrepareDefaultMaterialForRender(
        SDefaultMaterial &inMaterial, SRenderableObjectFlags &inExistingFlags, float inOpacity,
        bool inClearDirtyFlags)
{
    SDefaultMaterial *theMaterial = &inMaterial;
    SDefaultMaterialPreparationResult retval(GenerateLightingKey(theMaterial->m_Lighting));
    retval.m_RenderableFlags = inExistingFlags;
    SRenderableObjectFlags &renderableFlags(retval.m_RenderableFlags);
    SShaderDefaultMaterialKey &theGeneratedKey(retval.m_MaterialKey);
    retval.m_Opacity = inOpacity;
    float &subsetOpacity(retval.m_Opacity);

    if (theMaterial->m_Dirty.IsDirty()) {
        renderableFlags |= RenderPreparationResultFlagValues::Dirty;
    }
    subsetOpacity *= theMaterial->m_Opacity;
    if (inClearDirtyFlags)
        theMaterial->m_Dirty.UpdateDirtyForFrame();

    SRenderableImage *firstImage = nullptr;

    // set wireframe mode
    m_Renderer.DefaultMaterialShaderKeyProperties().m_WireframeMode.SetValue(
                theGeneratedKey, m_Renderer.GetQt3DSContext().GetWireframeMode());

    if (theMaterial->m_IblProbe && CheckLightProbeDirty(*theMaterial->m_IblProbe)) {
        m_Renderer.PrepareImageForIbl(*theMaterial->m_IblProbe);
    }

    if (!m_Renderer.DefaultMaterialShaderKeyProperties().m_HasIbl.GetValue(theGeneratedKey)) {
        bool lightProbeValid = HasValidLightProbe(theMaterial->m_IblProbe);
        SetShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", lightProbeValid);
        m_Renderer.DefaultMaterialShaderKeyProperties().m_HasIbl.SetValue(theGeneratedKey,
                                                                          lightProbeValid);
        // SetShaderFeature( "QDEMON_ENABLE_IBL_FOV",
        // m_Renderer.GetLayerRenderData()->m_Layer.m_ProbeFov < 180.0f );
    }

    if (subsetOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {

        if (theMaterial->m_BlendMode != DefaultMaterialBlendMode::Normal
                || theMaterial->m_OpacityMap) {
            renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
        }

        bool specularEnabled = theMaterial->IsSpecularEnabled();
        m_Renderer.DefaultMaterialShaderKeyProperties().m_SpecularEnabled.SetValue(
                    theGeneratedKey, specularEnabled);
        if (specularEnabled) {
            m_Renderer.DefaultMaterialShaderKeyProperties().m_SpecularModel.SetSpecularModel(
                        theGeneratedKey, theMaterial->m_SpecularModel);
        }

        m_Renderer.DefaultMaterialShaderKeyProperties().m_FresnelEnabled.SetValue(
                    theGeneratedKey, theMaterial->IsFresnelEnabled());

        m_Renderer.DefaultMaterialShaderKeyProperties().m_VertexColorsEnabled.SetValue(
                    theGeneratedKey, theMaterial->IsVertexColorsEnabled());

        // Run through the material's images and prepare them for render.
        // this may in fact set pickable on the renderable flags if one of the images
        // links to a sub presentation or any offscreen rendered object.
        SRenderableImage *nextImage = nullptr;
#define CHECK_IMAGE_AND_PREPARE(img, imgtype, shadercomponent)                                     \
    if ((img))                                                                                     \
    PrepareImageForRender(*(img), imgtype, firstImage, nextImage, renderableFlags,             \
    theGeneratedKey, shadercomponent);

        CHECK_IMAGE_AND_PREPARE(theMaterial->m_DiffuseMaps[0], ImageMapTypes::Diffuse,
                SShaderDefaultMaterialKeyProperties::DiffuseMap0);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_DiffuseMaps[1], ImageMapTypes::Diffuse,
                SShaderDefaultMaterialKeyProperties::DiffuseMap1);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_DiffuseMaps[2], ImageMapTypes::Diffuse,
                SShaderDefaultMaterialKeyProperties::DiffuseMap2);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_EmissiveMap, ImageMapTypes::Emissive,
                                SShaderDefaultMaterialKeyProperties::EmissiveMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_EmissiveMap2, ImageMapTypes::Emissive,
                                SShaderDefaultMaterialKeyProperties::EmissiveMap2);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_SpecularReflection, ImageMapTypes::Specular,
                                SShaderDefaultMaterialKeyProperties::SpecularMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_RoughnessMap, ImageMapTypes::Roughness,
                                SShaderDefaultMaterialKeyProperties::RoughnessMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_OpacityMap, ImageMapTypes::Opacity,
                                SShaderDefaultMaterialKeyProperties::OpacityMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_BumpMap, ImageMapTypes::Bump,
                                SShaderDefaultMaterialKeyProperties::BumpMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_SpecularMap, ImageMapTypes::SpecularAmountMap,
                                SShaderDefaultMaterialKeyProperties::SpecularAmountMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_NormalMap, ImageMapTypes::Normal,
                                SShaderDefaultMaterialKeyProperties::NormalMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_DisplacementMap, ImageMapTypes::Displacement,
                                SShaderDefaultMaterialKeyProperties::DisplacementMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_TranslucencyMap, ImageMapTypes::Translucency,
                                SShaderDefaultMaterialKeyProperties::TranslucencyMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_Lightmaps.m_LightmapIndirect,
                                ImageMapTypes::LightmapIndirect,
                                SShaderDefaultMaterialKeyProperties::LightmapIndirect);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_Lightmaps.m_LightmapRadiosity,
                                ImageMapTypes::LightmapRadiosity,
                                SShaderDefaultMaterialKeyProperties::LightmapRadiosity);
        CHECK_IMAGE_AND_PREPARE(theMaterial->m_Lightmaps.m_LightmapShadow,
                                ImageMapTypes::LightmapShadow,
                                SShaderDefaultMaterialKeyProperties::LightmapShadow);
    }
#undef CHECK_IMAGE_AND_PREPARE

    if (subsetOpacity < QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {
        subsetOpacity = 0.0f;
        // You can still pick against completely transparent objects(or rather their bounding
        // box)
        // you just don't render them.
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
        renderableFlags |= RenderPreparationResultFlagValues::CompletelyTransparent;
    }

    if (IsNotOne(subsetOpacity))
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;

    retval.m_FirstImage = firstImage;
    if (retval.m_RenderableFlags.IsDirty())
        retval.m_Dirty = true;
    return retval;
}

SDefaultMaterialPreparationResult SLayerRenderPreparationData::PrepareCustomMaterialForRender(
        SCustomMaterial &inMaterial, SRenderableObjectFlags &inExistingFlags, float inOpacity)
{
    SDefaultMaterialPreparationResult retval(GenerateLightingKey(
                                                 DefaultMaterialLighting::FragmentLighting)); // always fragment lighting
    retval.m_RenderableFlags = inExistingFlags;
    SRenderableObjectFlags &renderableFlags(retval.m_RenderableFlags);
    SShaderDefaultMaterialKey &theGeneratedKey(retval.m_MaterialKey);
    retval.m_Opacity = inOpacity;
    float &subsetOpacity(retval.m_Opacity);

    // set wireframe mode
    m_Renderer.DefaultMaterialShaderKeyProperties().m_WireframeMode.SetValue(
                theGeneratedKey, m_Renderer.GetQt3DSContext().GetWireframeMode());

    if (subsetOpacity < QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {
        subsetOpacity = 0.0f;
        // You can still pick against completely transparent objects(or rather their bounding
        // box)
        // you just don't render them.
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
        renderableFlags |= RenderPreparationResultFlagValues::CompletelyTransparent;
    }

    if (IsNotOne(subsetOpacity))
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;

    SRenderableImage *firstImage = nullptr;
    SRenderableImage *nextImage = nullptr;

#define CHECK_IMAGE_AND_PREPARE(img, imgtype, shadercomponent)                                     \
    if ((img))                                                                                     \
    PrepareImageForRender(*(img), imgtype, firstImage, nextImage, renderableFlags,             \
    theGeneratedKey, shadercomponent);

    CHECK_IMAGE_AND_PREPARE(inMaterial.m_DisplacementMap, ImageMapTypes::Displacement,
                            SShaderDefaultMaterialKeyProperties::DisplacementMap);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_Lightmaps.m_LightmapIndirect,
                            ImageMapTypes::LightmapIndirect,
                            SShaderDefaultMaterialKeyProperties::LightmapIndirect);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_Lightmaps.m_LightmapRadiosity,
                            ImageMapTypes::LightmapRadiosity,
                            SShaderDefaultMaterialKeyProperties::LightmapRadiosity);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_Lightmaps.m_LightmapShadow,
                            ImageMapTypes::LightmapShadow,
                            SShaderDefaultMaterialKeyProperties::LightmapShadow);
#undef CHECK_IMAGE_AND_PREPARE

    retval.m_FirstImage = firstImage;
    return retval;
}

bool SLayerRenderPreparationData::PrepareModelForRender(
        SModel &inModel, const QMatrix4x4 &inViewProjection,
        const QDemonOption<SClippingFrustum> &inClipFrustum, TNodeLightEntryList &inScopedLights)
{
    IQDemonRenderContext &qt3dsContext(m_Renderer.GetQt3DSContext());
    IBufferManager &bufferManager = qt3dsContext.GetBufferManager();
    SRenderMesh *theMesh = bufferManager.LoadMesh(inModel.m_MeshPath);
    if (theMesh == nullptr)
        return false;

    SGraphObject *theSourceMaterialObject = inModel.m_FirstMaterial;
    SModelContext &theModelContext =
            *new SModelContext(inModel, inViewProjection);
    m_ModelContexts.push_back(&theModelContext);

    bool subsetDirty = false;

    SScopedLightsListScope lightsScope(m_Lights, m_LightDirections, m_SourceLightDirections,
                                       inScopedLights);
    SetShaderFeature(m_CGLightingFeatureName, m_Lights.empty() == false);
    for (quint32 idx = 0, end = theMesh->m_Subsets.size(); idx < end && theSourceMaterialObject;
         ++idx, theSourceMaterialObject = GetNextMaterialSibling(theSourceMaterialObject)) {
        SRenderSubset &theOuterSubset(theMesh->m_Subsets[idx]);
        {
            SRenderSubset &theSubset(theOuterSubset);
            SRenderableObjectFlags renderableFlags;
            renderableFlags.SetPickable(false);
            float subsetOpacity = inModel.m_GlobalOpacity;
            QVector3D theModelCenter(theSubset.m_Bounds.getCenter());
            theModelCenter = inModel.m_GlobalTransform.transform(theModelCenter);

            if (subsetOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY
                    && inClipFrustum.hasValue()) {
                // Check bounding box against the clipping planes
                QDemonBounds3 theGlobalBounds = theSubset.m_Bounds;
                theGlobalBounds.transform(theModelContext.m_Model.m_GlobalTransform);
                if (inClipFrustum->intersectsWith(theGlobalBounds) == false)
                    subsetOpacity = 0.0f;
            }

            // For now everything is pickable.  Eventually we want to have localPickable and
            // globalPickable set on the node during
            // updates and have the runtime tell us what is pickable and what is not pickable.
            // Completely transparent models cannot be pickable.  But models with completely
            // transparent materials
            // still are.  This allows the artist to control pickability in a somewhat
            // fine-grained style.
            bool canModelBePickable = inModel.m_GlobalOpacity > .01f;
            renderableFlags.SetPickable(canModelBePickable
                                        && (theModelContext.m_Model.m_Flags.IsGloballyPickable()
                                            || renderableFlags.GetPickable()));
            SRenderableObject *theRenderableObject(nullptr);
            QPair<bool, SGraphObject *> theMaterialObjectAndDirty =
                    ResolveReferenceMaterial(theSourceMaterialObject);
            SGraphObject *theMaterialObject = theMaterialObjectAndDirty.second;
            subsetDirty = subsetDirty || theMaterialObjectAndDirty.first;
            if (theMaterialObject == nullptr)
                continue;

            // set tessellation
            if (inModel.m_TessellationMode != TessModeValues::NoTess) {
                theSubset.m_PrimitiveType = QDemonRenderDrawMode::Patches;
                // set tessellation factor
                theSubset.m_EdgeTessFactor = inModel.m_EdgeTess;
                theSubset.m_InnerTessFactor = inModel.m_InnerTess;
                // update the vertex ver patch count in the input assembler
                // currently we only support triangle patches so count is always 3
                theSubset.m_InputAssembler->SetPatchVertexCount(3);
                theSubset.m_InputAssemblerDepth->SetPatchVertexCount(3);
                // check wireframe mode
                theSubset.m_WireframeMode = qt3dsContext.GetWireframeMode();

                subsetDirty =
                        subsetDirty | (theSubset.m_WireframeMode != inModel.m_WireframeMode);
                inModel.m_WireframeMode = qt3dsContext.GetWireframeMode();
            } else {
                theSubset.m_PrimitiveType = theSubset.m_InputAssembler->GetPrimitiveType();
                theSubset.m_InputAssembler->SetPatchVertexCount(1);
                theSubset.m_InputAssemblerDepth->SetPatchVertexCount(1);
                // currently we allow wirframe mode only if tessellation is on
                theSubset.m_WireframeMode = false;

                subsetDirty =
                        subsetDirty | (theSubset.m_WireframeMode != inModel.m_WireframeMode);
                inModel.m_WireframeMode = false;
            }
            // Only clear flags on the materials in this direct hierarchy.  Do not clear them of
            // this
            // references materials in another hierarchy.
            bool clearMaterialDirtyFlags = theMaterialObject == theSourceMaterialObject;

            if (theMaterialObject == nullptr)
                continue;

            if (theMaterialObject->m_Type == GraphObjectTypes::DefaultMaterial) {
                SDefaultMaterial &theMaterial(
                            static_cast<SDefaultMaterial &>(*theMaterialObject));
                SDefaultMaterialPreparationResult theMaterialPrepResult(
                            PrepareDefaultMaterialForRender(theMaterial, renderableFlags, subsetOpacity,
                                                            clearMaterialDirtyFlags));
                SShaderDefaultMaterialKey theGeneratedKey = theMaterialPrepResult.m_MaterialKey;
                subsetOpacity = theMaterialPrepResult.m_Opacity;
                SRenderableImage *firstImage(theMaterialPrepResult.m_FirstImage);
                subsetDirty |= theMaterialPrepResult.m_Dirty;
                renderableFlags = theMaterialPrepResult.m_RenderableFlags;

                m_Renderer.DefaultMaterialShaderKeyProperties()
                        .m_TessellationMode.SetTessellationMode(theGeneratedKey,
                                                                inModel.m_TessellationMode, true);

                QDemonConstDataRef<QMatrix4x4> boneGlobals;
                if (theSubset.m_Joints.size()) {
                    Q_ASSERT(false);
                }

                theRenderableObject = new SSubsetRenderable(
                            renderableFlags, theModelCenter, m_Renderer, theSubset, theMaterial,
                            theModelContext, subsetOpacity, firstImage, theGeneratedKey, boneGlobals);
                subsetDirty = subsetDirty || renderableFlags.IsDirty();

            } // if type == DefaultMaterial
            else if (theMaterialObject->m_Type == GraphObjectTypes::CustomMaterial) {
                SCustomMaterial &theMaterial(
                            static_cast<SCustomMaterial &>(*theMaterialObject));

                ICustomMaterialSystem &theMaterialSystem(
                            qt3dsContext.GetCustomMaterialSystem());
                subsetDirty |= theMaterialSystem.PrepareForRender(
                            theModelContext.m_Model, theSubset, theMaterial, clearMaterialDirtyFlags);

                SDefaultMaterialPreparationResult theMaterialPrepResult(
                            PrepareCustomMaterialForRender(theMaterial, renderableFlags,
                                                           subsetOpacity));
                SShaderDefaultMaterialKey theGeneratedKey = theMaterialPrepResult.m_MaterialKey;
                subsetOpacity = theMaterialPrepResult.m_Opacity;
                SRenderableImage *firstImage(theMaterialPrepResult.m_FirstImage);
                renderableFlags = theMaterialPrepResult.m_RenderableFlags;

                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasTransparency)
                    renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasRefraction)
                    renderableFlags |= RenderPreparationResultFlagValues::HasRefraction;

                m_Renderer.DefaultMaterialShaderKeyProperties()
                        .m_TessellationMode.SetTessellationMode(theGeneratedKey,
                                                                inModel.m_TessellationMode, true);

                if (theMaterial.m_IblProbe && CheckLightProbeDirty(*theMaterial.m_IblProbe)) {
                    m_Renderer.PrepareImageForIbl(*theMaterial.m_IblProbe);
                }

                theRenderableObject = new SCustomMaterialRenderable(
                            renderableFlags, theModelCenter, m_Renderer, theSubset, theMaterial,
                            theModelContext, subsetOpacity, firstImage, theGeneratedKey);
            }
            if (theRenderableObject) {
                theRenderableObject->m_ScopedLights = inScopedLights;
                // set tessellation
                theRenderableObject->m_TessellationMode = inModel.m_TessellationMode;

                if (theRenderableObject->m_RenderableFlags.HasTransparency()
                        || theRenderableObject->m_RenderableFlags.HasRefraction()) {
                    m_TransparentObjects.push_back(theRenderableObject);
                } else {
                    m_OpaqueObjects.push_back(theRenderableObject);
                }
            }
        }
    }
    return subsetDirty;
}

bool SLayerRenderPreparationData::PrepareRenderablesForRender(
        const QMatrix4x4 &inViewProjection, const QDemonOption<SClippingFrustum> &inClipFrustum,
        float inTextScaleFactor, SLayerRenderPreparationResultFlags &ioFlags)
{
    SStackPerfTimer __timer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                            "SLayerRenderData::PrepareRenderablesForRender");
    m_ViewProjection = inViewProjection;
    float theTextScaleFactor = inTextScaleFactor;
    bool wasDataDirty = false;
    bool hasTextRenderer = m_Renderer.GetQt3DSContext().GetTextRenderer() != nullptr;
    for (quint32 idx = 0, end = m_RenderableNodes.size(); idx < end; ++idx) {
        SRenderableNodeEntry &theNodeEntry(m_RenderableNodes[idx]);
        SNode *theNode = theNodeEntry.m_Node;
        wasDataDirty = wasDataDirty || theNode->m_Flags.IsDirty();
        switch (theNode->m_Type) {
        case GraphObjectTypes::Model: {
            SModel *theModel = static_cast<SModel *>(theNode);
            theModel->CalculateGlobalVariables();
            if (theModel->m_Flags.IsGloballyActive()) {
                bool wasModelDirty = PrepareModelForRender(
                            *theModel, inViewProjection, inClipFrustum, theNodeEntry.m_Lights);
                wasDataDirty = wasDataDirty || wasModelDirty;
            }
        } break;
        case GraphObjectTypes::Text: {
            if (hasTextRenderer) {
                SText *theText = static_cast<SText *>(theNode);
                theText->CalculateGlobalVariables();
                if (theText->m_Flags.IsGloballyActive()) {
                    bool wasTextDirty = PrepareTextForRender(*theText, inViewProjection,
                                                             theTextScaleFactor, ioFlags);
                    wasDataDirty = wasDataDirty || wasTextDirty;
                }
            }
        } break;
        case GraphObjectTypes::Path: {
            SPath *thePath = static_cast<SPath *>(theNode);
            thePath->CalculateGlobalVariables();
            if (thePath->m_Flags.IsGloballyActive()) {
                bool wasPathDirty =
                        PreparePathForRender(*thePath, inViewProjection, inClipFrustum, ioFlags);
                wasDataDirty = wasDataDirty || wasPathDirty;
            }
        } break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
    return wasDataDirty;
}

bool SLayerRenderPreparationData::CheckLightProbeDirty(SImage &inLightProbe)
{
    IQDemonRenderContext &theContext(m_Renderer.GetQt3DSContext());
    return inLightProbe.ClearDirty(theContext.GetBufferManager(),
                                   theContext.GetOffscreenRenderManager(),
                                   theContext.GetRenderPluginManager(), true);
}

struct SLightNodeMarker
{
    SLight *m_Light;
    quint32 m_LightIndex;
    quint32 m_FirstValidIndex;
    quint32 m_JustPastLastValidIndex;
    bool m_AddOrRemove;
    SLightNodeMarker()
        : m_Light(nullptr)
        , m_FirstValidIndex(0)
        , m_JustPastLastValidIndex(0)
        , m_AddOrRemove(false)
    {
    }
    SLightNodeMarker(SLight &inLight, quint32 inLightIndex, SNode &inNode, bool aorm)
        : m_Light(&inLight)
        , m_LightIndex(inLightIndex)
        , m_AddOrRemove(aorm)
    {
        if (inNode.m_Type == GraphObjectTypes::Layer) {
            m_FirstValidIndex = 0;
            m_JustPastLastValidIndex = QDEMON_MAX_U32;
        } else {
            m_FirstValidIndex = inNode.m_DFSIndex;
            SNode *lastChild = nullptr;
            SNode *firstChild = inNode.m_FirstChild;
            // find deepest last child
            while (firstChild) {
                for (SNode *childNode = firstChild; childNode;
                     childNode = childNode->m_NextSibling)
                    lastChild = childNode;

                if (lastChild)
                    firstChild = lastChild->m_FirstChild;
                else
                    firstChild = nullptr;
            }
            if (lastChild)
                // last valid index would be the last child index + 1
                m_JustPastLastValidIndex = lastChild->m_DFSIndex + 1;
            else // no children.
                m_JustPastLastValidIndex = m_FirstValidIndex + 1;
        }
    }
};

// m_Layer.m_Camera->CalculateViewProjectionMatrix(m_ViewProjection);
void
SLayerRenderPreparationData::PrepareForRender(const QSize &inViewportDimensions)
{
    SStackPerfTimer __timer(m_Renderer.GetQt3DSContext().GetPerfTimer(),
                            "SLayerRenderData::PrepareForRender");
    if (m_LayerPrepResult.hasValue())
        return;

    m_Features.clear();
    m_FeatureSetHash = 0;
    QVector2D thePresentationDimensions((float)inViewportDimensions.width(),
                                        (float)inViewportDimensions.height());
    IRenderList &theGraph(m_Renderer.GetQt3DSContext().GetRenderList());
    QDemonRenderRect theViewport(theGraph.GetViewport());
    QDemonRenderRect theScissor(theGraph.GetViewport());
    if (theGraph.IsScissorTestEnabled())
        theScissor = m_Renderer.GetContext().GetScissorRect();
    bool wasDirty = false;
    bool wasDataDirty = false;
    wasDirty = m_Layer.m_Flags.IsDirty();
    // The first pass is just to render the data.
    quint32 maxNumAAPasses = m_Layer.m_ProgressiveAAMode == AAModeValues::NoAA
            ? (quint32)0
            : (quint32)(m_Layer.m_ProgressiveAAMode) + 1;
    maxNumAAPasses = NVMin((quint32)(MAX_AA_LEVELS + 1), maxNumAAPasses);
    SEffect *theLastEffect = nullptr;
    // Uncomment the line below to disable all progressive AA.
    // maxNumAAPasses = 0;

    SLayerRenderPreparationResult thePrepResult;
    bool hasOffscreenRenderer = GetOffscreenRenderer();

    bool SSAOEnabled = (m_Layer.m_AoStrength > 0.0f && m_Layer.m_AoDistance > 0.0f);
    bool SSDOEnabled = (m_Layer.m_ShadowStrength > 0.0f && m_Layer.m_ShadowDist > 0.0f);
    SetShaderFeature("QDEMON_ENABLE_SSAO", SSAOEnabled);
    SetShaderFeature("QDEMON_ENABLE_SSDO", SSDOEnabled);
    bool requiresDepthPrepass = (hasOffscreenRenderer == false) && (SSAOEnabled || SSDOEnabled);
    SetShaderFeature("QDEMON_ENABLE_SSM", false); // by default no shadow map generation

    if (m_Layer.m_Flags.IsActive()) {
        // Get the layer's width and height.
        IEffectSystem &theEffectSystem(m_Renderer.GetQt3DSContext().GetEffectSystem());
        for (SEffect *theEffect = m_Layer.m_FirstEffect; theEffect;
             theEffect = theEffect->m_NextEffect) {
            if (theEffect->m_Flags.IsDirty()) {
                wasDirty = true;
                theEffect->m_Flags.SetDirty(false);
            }
            if (theEffect->m_Flags.IsActive()) {
                theLastEffect = theEffect;
                if (hasOffscreenRenderer == false
                        && theEffectSystem.DoesEffectRequireDepthTexture(theEffect->m_ClassName))
                    requiresDepthPrepass = true;
            }
        }
        if (m_Layer.m_Flags.IsDirty()) {
            wasDirty = true;
            m_Layer.CalculateGlobalVariables();
        }

        bool shouldRenderToTexture = true;

        if (hasOffscreenRenderer) {
            // We don't render to texture with offscreen renderers, we just render them to the
            // viewport.
            shouldRenderToTexture = false;
            // Progaa disabled when using offscreen rendering.
            maxNumAAPasses = 0;
        }

        thePrepResult = SLayerRenderPreparationResult(SLayerRenderHelper(
                                                          theViewport, theScissor, m_Layer.m_Scene->m_Presentation->m_PresentationDimensions,
                                                          m_Layer, shouldRenderToTexture, m_Renderer.GetQt3DSContext().GetScaleMode(),
                                                          m_Renderer.GetQt3DSContext().GetPresentationScaleFactor()));
        thePrepResult.m_LastEffect = theLastEffect;
        thePrepResult.m_MaxAAPassIndex = maxNumAAPasses;
        thePrepResult.m_Flags.SetRequiresDepthTexture(requiresDepthPrepass
                                                      || NeedsWidgetTexture());
        thePrepResult.m_Flags.SetShouldRenderToTexture(shouldRenderToTexture);
        if (m_Renderer.GetContext().GetRenderContextType() != QDemonRenderContextValues::GLES2)
            thePrepResult.m_Flags.SetRequiresSsaoPass(SSAOEnabled);

        if (thePrepResult.IsLayerVisible()) {
            if (shouldRenderToTexture) {
                m_Renderer.GetQt3DSContext().GetRenderList().AddRenderTask(
                            CreateRenderToTextureRunnable());
            }
            if (m_Layer.m_LightProbe && CheckLightProbeDirty(*m_Layer.m_LightProbe)) {
                m_Renderer.PrepareImageForIbl(*m_Layer.m_LightProbe);
                wasDataDirty = true;
            }

            bool lightProbeValid = HasValidLightProbe(m_Layer.m_LightProbe);

            SetShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", lightProbeValid);
            SetShaderFeature("QDEMON_ENABLE_IBL_FOV", m_Layer.m_ProbeFov < 180.0f);

            if (lightProbeValid && m_Layer.m_LightProbe2
                    && CheckLightProbeDirty(*m_Layer.m_LightProbe2)) {
                m_Renderer.PrepareImageForIbl(*m_Layer.m_LightProbe2);
                wasDataDirty = true;
            }

            SetShaderFeature("QDEMON_ENABLE_LIGHT_PROBE_2",
                             lightProbeValid && HasValidLightProbe(m_Layer.m_LightProbe2));

            // Push nodes in reverse depth first order
            if (m_RenderableNodes.empty()) {
                m_CamerasAndLights.clear();
                quint32 dfsIndex = 0;
                for (SNode *theChild = m_Layer.m_FirstChild; theChild;
                     theChild = theChild->m_NextSibling)
                    MaybeQueueNodeForRender(*theChild, m_RenderableNodes, m_CamerasAndLights,
                                            dfsIndex);
                reverse(m_CamerasAndLights.begin(), m_CamerasAndLights.end());
                reverse(m_RenderableNodes.begin(), m_RenderableNodes.end());
                m_LightToNodeMap.clear();
            }
            m_Camera = nullptr;
            m_Lights.clear();
            m_OpaqueObjects.clear();
            m_TransparentObjects.clear();
            QVector<SLightNodeMarker> theLightNodeMarkers(m_Renderer.GetPerFrameAllocator(),
                                                           "LightNodeMarkers");
            m_SourceLightDirections.clear();

            for (quint32 idx = 0, end = m_CamerasAndLights.size(); idx < end; ++idx) {
                SNode *theNode(m_CamerasAndLights[idx]);
                wasDataDirty = wasDataDirty || theNode->m_Flags.IsDirty();
                switch (theNode->m_Type) {
                case GraphObjectTypes::Camera: {
                    SCamera *theCamera = static_cast<SCamera *>(theNode);
                    SCameraGlobalCalculationResult theResult =
                            thePrepResult.SetupCameraForRender(*theCamera);
                    wasDataDirty = wasDataDirty || theResult.m_WasDirty;
                    if (theCamera->m_Flags.IsGloballyActive())
                        m_Camera = theCamera;
                    if (theResult.m_ComputeFrustumSucceeded == false) {
                        qCCritical(INTERNAL_ERROR, "Failed to calculate camera frustum");
                    }
                } break;
                case GraphObjectTypes::Light: {
                    SLight *theLight = static_cast<SLight *>(theNode);
                    bool lightResult = theLight->CalculateGlobalVariables();
                    wasDataDirty = lightResult || wasDataDirty;
                    // Note we setup the light index such that it is completely invariant of if
                    // the
                    // light is active or scoped.
                    quint32 lightIndex = (quint32)m_SourceLightDirections.size();
                    m_SourceLightDirections.push_back(QVector3D(0.0f));
                    // Note we still need a light check when building the renderable light list.
                    // We also cannot cache shader-light bindings based on layers any more
                    // because
                    // the number of lights for a given renderable does not depend on the layer
                    // as it used to but
                    // additional perhaps on the light's scoping rules.
                    if (theLight->m_Flags.IsGloballyActive()) {
                        if (theLight->m_Scope == nullptr) {
                            m_Lights.push_back(theLight);
                            if (m_Renderer.GetContext().GetRenderContextType()
                                    != QDemonRenderContextValues::GLES2
                                    && theLight->m_CastShadow && GetShadowMapManager()) {
                                // PKC -- use of "res" as an exponent of two is an annoying
                                // artifact of the XML interface
                                // I'll change this with an enum interface later on, but that's
                                // less important right now.
                                quint32 mapSize = 1 << theLight->m_ShadowMapRes;
                                ShadowMapModes::Enum mapMode =
                                        (theLight->m_LightType != RenderLightTypes::Directional)
                                        ? ShadowMapModes::CUBE
                                        : ShadowMapModes::VSM;
                                m_ShadowMapManager->AddShadowMapEntry(
                                            m_Lights.size() - 1, mapSize, mapSize,
                                            QDemonRenderTextureFormats::R16F, 1, mapMode,
                                            ShadowFilterValues::NONE);
                                thePrepResult.m_Flags.SetRequiresShadowMapPass(true);
                                SetShaderFeature("QDEMON_ENABLE_SSM", true);
                            }
                        }
                        TLightToNodeMap::iterator iter =
                                m_LightToNodeMap.insert(theLight, (SNode *)nullptr)
                                .first;
                        SNode *oldLightScope = iter->second;
                        SNode *newLightScope = theLight->m_Scope;

                        if (oldLightScope != newLightScope) {
                            iter->second = newLightScope;
                            if (oldLightScope)
                                theLightNodeMarkers.push_back(SLightNodeMarker(
                                                                  *theLight, lightIndex, *oldLightScope, false));
                            if (newLightScope)
                                theLightNodeMarkers.push_back(SLightNodeMarker(
                                                                  *theLight, lightIndex, *newLightScope, true));
                        }
                        if (newLightScope) {
                            m_SourceLightDirections.back() =
                                    theLight->GetScalingCorrectDirection();
                        }
                    }
                } break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            if (theLightNodeMarkers.empty() == false) {
                for (quint32 idx = 0, end = m_RenderableNodes.size(); idx < end; ++idx) {
                    SRenderableNodeEntry &theNodeEntry(m_RenderableNodes[idx]);
                    quint32 nodeDFSIndex = theNodeEntry.m_Node->m_DFSIndex;
                    for (quint32 markerIdx = 0, markerEnd = theLightNodeMarkers.size();
                         markerIdx < markerEnd; ++markerIdx) {
                        SLightNodeMarker &theMarker = theLightNodeMarkers[markerIdx];
                        if (nodeDFSIndex >= theMarker.m_FirstValidIndex
                                && nodeDFSIndex < theMarker.m_JustPastLastValidIndex) {
                            if (theMarker.m_AddOrRemove) {
                                SNodeLightEntry *theNewEntry =
                                        m_RenderableNodeLightEntryPool.construct(
                                            theMarker.m_Light, theMarker.m_LightIndex, __FILE__,
                                            __LINE__);
                                theNodeEntry.m_Lights.push_back(*theNewEntry);
                            } else {
                                for (TNodeLightEntryList::iterator
                                     lightIter = theNodeEntry.m_Lights.begin(),
                                     lightEnd = theNodeEntry.m_Lights.end();
                                     lightIter != lightEnd; ++lightIter) {
                                    if (lightIter->m_Light == theMarker.m_Light) {
                                        SNodeLightEntry &theEntry = *lightIter;
                                        theNodeEntry.m_Lights.remove(theEntry);
                                        m_RenderableNodeLightEntryPool.deallocate(&theEntry);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            float theTextScaleFactor = 1.0f;
            if (m_Camera) {
                m_Camera->CalculateViewProjectionMatrix(m_ViewProjection);
                theTextScaleFactor = m_Camera->GetTextScaleFactor(
                            thePrepResult.GetLayerToPresentationViewport(),
                            thePrepResult.GetPresentationDesignDimensions());
                SClipPlane nearPlane;
                QMatrix3x3 theUpper33(m_Camera->m_GlobalTransform.getUpper3x3InverseTranspose());

                QVector3D dir(theUpper33.transform(QVector3D(0, 0, -1)));
                dir.normalize();
                nearPlane.normal = dir;
                QVector3D theGlobalPos = m_Camera->GetGlobalPos() + m_Camera->m_ClipNear * dir;
                nearPlane.d = -(dir.dot(theGlobalPos));
                // the near plane's bbox edges are calculated in the clipping frustum's
                // constructor.
                m_ClippingFrustum = SClippingFrustum(m_ViewProjection, nearPlane);
            } else
                m_ViewProjection = QMatrix4x4::createIdentity();

            // Setup the light directions here.

            for (quint32 lightIdx = 0, lightEnd = m_Lights.size(); lightIdx < lightEnd;
                 ++lightIdx) {
                m_LightDirections.push_back(m_Lights[lightIdx]->GetScalingCorrectDirection());
            }

            m_ModelContexts.clear();
            if (GetOffscreenRenderer() == false) {
                bool renderablesDirty =
                        PrepareRenderablesForRender(m_ViewProjection, m_ClippingFrustum,
                                                    theTextScaleFactor, thePrepResult.m_Flags);
                wasDataDirty = wasDataDirty || renderablesDirty;
                if (thePrepResult.m_Flags.RequiresStencilBuffer())
                    thePrepResult.m_Flags.SetShouldRenderToTexture(true);
            } else {
                QDemonRenderRect theViewport =
                        thePrepResult.GetLayerToPresentationViewport().ToIntegerRect();
                bool theScissor = true;
                QDemonRenderRect theScissorRect =
                        thePrepResult.GetLayerToPresentationScissorRect().ToIntegerRect();
                // This happens here because if there are any fancy render steps
                IRenderList &theRenderList(m_Renderer.GetQt3DSContext().GetRenderList());
                QDemonRenderContext &theContext(m_Renderer.GetContext());
                SRenderListScopedProperty<bool> _listScissorEnabled(
                            theRenderList, &IRenderList::IsScissorTestEnabled,
                            &IRenderList::SetScissorTestEnabled, theScissor);
                SRenderListScopedProperty<QDemonRenderRect> _listViewport(
                            theRenderList, &IRenderList::GetViewport, &IRenderList::SetViewport,
                            theViewport);
                SRenderListScopedProperty<QDemonRenderRect> _listScissor(
                            theRenderList, &IRenderList::GetScissor, &IRenderList::SetScissorRect,
                            theScissorRect);
                // Some plugins don't use the render list so they need the actual gl context
                // setup.
                QDemonRenderContextScopedProperty<bool> __scissorEnabled(
                            theContext, &QDemonRenderContext::IsScissorTestEnabled,
                            &QDemonRenderContext::SetScissorTestEnabled, true);
                QDemonRenderContextScopedProperty<QDemonRenderRect> __scissorRect(
                            theContext, &QDemonRenderContext::GetScissorRect,
                            &QDemonRenderContext::SetScissorRect, theScissorRect);
                QDemonRenderContextScopedProperty<QDemonRenderRect> __viewportRect(
                            theContext, &QDemonRenderContext::GetViewport, &QDemonRenderContext::SetViewport,
                            theViewport);
                SOffscreenRenderFlags theResult = m_LastFrameOffscreenRenderer->NeedsRender(
                            CreateOffscreenRenderEnvironment(),
                            m_Renderer.GetQt3DSContext().GetPresentationScaleFactor(), &m_Layer);
                wasDataDirty = wasDataDirty || theResult.m_HasChangedSinceLastFrame;
            }
        }
    }
    wasDirty = wasDirty || wasDataDirty;
    thePrepResult.m_Flags.SetWasDirty(wasDirty);
    thePrepResult.m_Flags.SetLayerDataDirty(wasDataDirty);

    m_LayerPrepResult = thePrepResult;

    // Per-frame cache of renderable objects post-sort.
    GetOpaqueRenderableObjects();
    // If layer depth test is false, this may also contain opaque objects.
    GetTransparentRenderableObjects();

    GetCameraDirection();
}

void SLayerRenderPreparationData::ResetForFrame()
{
    m_TransparentObjects.clear_unsafe();
    m_OpaqueObjects.clear_unsafe();
    m_LayerPrepResult.setEmpty();
    // The check for if the camera is or is not null is used
    // to figure out if this layer was rendered at all.
    m_Camera = nullptr;
    m_LastFrameOffscreenRenderer = nullptr;
    m_IRenderWidgets.clear_unsafe();
    m_CameraDirection.setEmpty();
    m_LightDirections.clear_unsafe();
    m_RenderedOpaqueObjects.clear_unsafe();
    m_RenderedTransparentObjects.clear_unsafe();
}

QT_END_NAMESPACE
