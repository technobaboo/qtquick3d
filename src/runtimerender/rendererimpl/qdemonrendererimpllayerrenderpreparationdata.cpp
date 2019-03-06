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
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcemanager.h>
#include <QtDemonRuntimeRender/qdemontextrenderer.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrenderkey.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <QtDemon/qdemonperftimer.h>
#include <QtDemonRuntimeRender/qdemonrendermaterialhelpers.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
#include <QtDemonRuntimeRender/qdemonrendertexttexturecache.h>
#include <QtDemonRuntimeRender/qdemonrendertexttextureatlas.h>
#include <QtDemonRuntimeRender/qdemonrenderrenderlist.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>
#include <QtDemonRuntimeRender/qdemonrenderpathmanager.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonperframeallocator.h>

#ifdef _WIN32
#pragma warning(disable : 4355)
#endif

QT_BEGIN_NAMESPACE

namespace {
void MaybeQueueNodeForRender(QDemonGraphNode &inNode,
                             QVector<QDemonRenderableNodeEntry> &outRenderables,
                             QVector<QDemonGraphNode *> &outCamerasAndLights,
                             quint32 &ioDFSIndex)
{
    ++ioDFSIndex;
    inNode.dfsIndex = ioDFSIndex;
    if (QDemonGraphObjectTypes::IsRenderableType(inNode.type))
        outRenderables.push_back(inNode);
    else if (QDemonGraphObjectTypes::IsLightCameraType(inNode.type))
        outCamerasAndLights.push_back(&inNode);

    for (QDemonGraphNode *theChild = inNode.firstChild; theChild != nullptr; theChild = theChild->nextSibling)
        MaybeQueueNodeForRender(*theChild, outRenderables, outCamerasAndLights, ioDFSIndex);
}

bool HasValidLightProbe(QDemonRenderImage *inLightProbeImage)
{
    return inLightProbeImage && inLightProbeImage->m_textureData.m_texture;
}
}

QDemonDefaultMaterialPreparationResult::QDemonDefaultMaterialPreparationResult(QDemonShaderDefaultMaterialKey inKey)
    : firstImage(nullptr), opacity(1.0f), materialKey(inKey), dirty(false)
{
}

#define MAX_AA_LEVELS 8

QDemonLayerRenderPreparationData::QDemonLayerRenderPreparationData(QDemonRenderLayer &inLayer,
                                                                   const QDemonRef<QDemonRendererImpl> &inRenderer)
    : layer(inLayer)
    , renderer(inRenderer)
    , camera(nullptr)
    , cgLightingFeatureName(QStringLiteral("QDEMON_ENABLE_CG_LIGHTING"))
    , featuresDirty(true)
    , featureSetHash(0)
    , tooManyLightsError(false)
{
}

QDemonLayerRenderPreparationData::~QDemonLayerRenderPreparationData() = default;

bool QDemonLayerRenderPreparationData::needsWidgetTexture() const
{
    return iRenderWidgets.size() > 0;
}

void QDemonLayerRenderPreparationData::setShaderFeature(const QString &theStr, bool inValue)
{
    QDemonShaderPreprocessorFeature item(theStr, inValue);
    QVector<QDemonShaderPreprocessorFeature>::iterator iter = features.begin(), end = features.end();

    // empty loop intentional.
    for (; iter != end && ((iter->name == theStr) == false); ++iter)
        ;

    if (iter != end) {
        if (iter->enabled != inValue) {
            iter->enabled = inValue;
            featuresDirty = true;
            featureSetHash = 0;
        }
    } else {
        features.push_back(item);
        featuresDirty = true;
        featureSetHash = 0;
    }
}

void QDemonLayerRenderPreparationData::setShaderFeature(const char *inName, bool inValue)
{
    QString theStr(QString::fromLocal8Bit(inName));
    setShaderFeature(theStr, inValue);
}

QVector<QDemonShaderPreprocessorFeature> QDemonLayerRenderPreparationData::getShaderFeatureSet()
{
    if (featuresDirty) {
        std::sort(features.begin(), features.end());
        featuresDirty = false;
    }
    return features;
}

size_t QDemonLayerRenderPreparationData::getShaderFeatureSetHash()
{
    if (!featureSetHash)
        featureSetHash = hashShaderFeatureSet(getShaderFeatureSet());
    return featureSetHash;
}

bool QDemonLayerRenderPreparationData::getShadowMapManager()
{
    if (shadowMapManager)
        return true;

    shadowMapManager = QDemonRenderShadowMap::create(renderer->getDemonContext());

    return shadowMapManager != nullptr;
}

bool QDemonLayerRenderPreparationData::getOffscreenRenderer()
{
    if (lastFrameOffscreenRenderer)
        return true;

    //    if (m_Layer.m_RenderPlugin && m_Layer.m_RenderPlugin->m_Flags.IsActive()) {
    //        IRenderPluginInstance *theInstance =
    //                m_Renderer.GetDemonContext().GetRenderPluginManager().GetOrCreateRenderPluginInstance(
    //                    m_Layer.m_RenderPlugin->m_PluginPath, m_Layer.m_RenderPlugin);
    //        if (theInstance) {
    //            m_Renderer.GetDemonContext()
    //                    .GetOffscreenRenderManager()
    //                    .MaybeRegisterOffscreenRenderer(&theInstance, *theInstance);
    //            m_LastFrameOffscreenRenderer = theInstance;
    //        }
    //    }
    if (lastFrameOffscreenRenderer == nullptr)
        lastFrameOffscreenRenderer = renderer->getDemonContext()->getOffscreenRenderManager()->getOffscreenRenderer(layer.texturePath);
    return lastFrameOffscreenRenderer != nullptr;
}

QVector3D QDemonLayerRenderPreparationData::getCameraDirection()
{
    if (cameraDirection.hasValue() == false) {
        if (camera)
            cameraDirection = camera->getScalingCorrectDirection();
        else
            cameraDirection = QVector3D(0, 0, -1);
    }
    return *cameraDirection;
}

// Per-frame cache of renderable objects post-sort.
QVector<QDemonRenderableObject *> QDemonLayerRenderPreparationData::getOpaqueRenderableObjects()
{
    if (renderedOpaqueObjects.empty() == false || camera == nullptr)
        return renderedOpaqueObjects;
    if (layer.flags.isLayerEnableDepthTest() && opaqueObjects.empty() == false) {
        QVector3D theCameraDirection(getCameraDirection());
        QVector3D theCameraPosition = camera->getGlobalPos();
        renderedOpaqueObjects = opaqueObjects;
        // Setup the object's sorting information
        for (quint32 idx = 0, end = renderedOpaqueObjects.size(); idx < end; ++idx) {
            QDemonRenderableObject &theInfo = *renderedOpaqueObjects[idx];
            QVector3D difference = theInfo.worldCenterPoint - theCameraPosition;
            theInfo.cameraDistanceSq = QVector3D::dotProduct(difference, theCameraDirection);
        }
        // Render nearest to furthest objects
        std::sort(renderedOpaqueObjects.begin(), renderedOpaqueObjects.end(), iSRenderObjectPtrLessThan);
    }
    return renderedOpaqueObjects;
}

// If layer depth test is false, this may also contain opaque objects.
QVector<QDemonRenderableObject *> QDemonLayerRenderPreparationData::getTransparentRenderableObjects()
{
    if (renderedTransparentObjects.empty() == false || camera == nullptr)
        return renderedTransparentObjects;

    renderedTransparentObjects = transparentObjects;

    if (layer.flags.isLayerEnableDepthTest() == false)
        renderedTransparentObjects.append(opaqueObjects);

    if (renderedTransparentObjects.empty() == false) {
        QVector3D theCameraDirection(getCameraDirection());
        QVector3D theCameraPosition = camera->getGlobalPos();

        // Setup the object's sorting information
        for (quint32 idx = 0, end = renderedTransparentObjects.size(); idx < end; ++idx) {
            QDemonRenderableObject &theInfo = *renderedTransparentObjects[idx];
            QVector3D difference = theInfo.worldCenterPoint - theCameraPosition;
            theInfo.cameraDistanceSq = QVector3D::dotProduct(difference, theCameraDirection);
        }
        // render furthest to nearest.
        std::sort(renderedTransparentObjects.begin(), renderedTransparentObjects.end(), iSRenderObjectPtrGreatThan);
    }

    return renderedTransparentObjects;
}

#define MAX_LAYER_WIDGETS 200

void QDemonLayerRenderPreparationData::addRenderWidget(QDemonRenderWidgetInterface &inWidget)
{
    // The if the layer is not active then the widget can't be displayed.
    // Furthermore ResetForFrame won't be called below which leads to stale
    // widgets in the m_IRenderWidgets array.  These stale widgets would get rendered
    // the next time the layer was active potentially causing a crash.
    if (!layer.flags.isActive())
        return;

    // Ensure we clear the widget layer always
    renderer->layerNeedsFrameClear(*static_cast<QDemonLayerRenderData *>(this));

    if (iRenderWidgets.size() < MAX_LAYER_WIDGETS)
        iRenderWidgets.push_back(&inWidget);
}

#define RENDER_FRAME_NEW(type) new (renderer->getDemonContext()->getPerFrameAllocator().allocate(sizeof(type))) type

#define QDEMON_RENDER_MINIMUM_RENDER_OPACITY .01f

QDemonShaderDefaultMaterialKey QDemonLayerRenderPreparationData::generateLightingKey(DefaultMaterialLighting::Enum inLightingType)
{
    QDemonShaderDefaultMaterialKey theGeneratedKey(getShaderFeatureSetHash());
    const bool lighting = inLightingType != DefaultMaterialLighting::NoLighting;
    renderer->defaultMaterialShaderKeyProperties().m_hasLighting.setValue(theGeneratedKey, lighting);
    if (lighting) {
        const bool lightProbe = layer.lightProbe && layer.lightProbe->m_textureData.m_texture;
        renderer->defaultMaterialShaderKeyProperties().m_hasIbl.setValue(theGeneratedKey, lightProbe);

        quint32 numLights = (quint32)lights.size();
        if (numLights > QDemonShaderDefaultMaterialKeyProperties::LightCount && tooManyLightsError == false) {
            tooManyLightsError = true;
            numLights = QDemonShaderDefaultMaterialKeyProperties::LightCount;
            qCCritical(INVALID_OPERATION, "Too many lights on layer, max is 7");
            Q_ASSERT(false);
        }
        renderer->defaultMaterialShaderKeyProperties().m_lightCount.setValue(theGeneratedKey, numLights);

        for (quint32 lightIdx = 0, lightEnd = lights.size(); lightIdx < lightEnd; ++lightIdx) {
            QDemonRenderLight *theLight(lights[lightIdx]);
            const bool isDirectional = theLight->m_lightType == RenderLightTypes::Directional;
            const bool isArea = theLight->m_lightType == RenderLightTypes::Area;
            const bool castShadowsArea = (theLight->m_lightType != RenderLightTypes::Area) && (theLight->m_castShadow);

            renderer->defaultMaterialShaderKeyProperties().m_lightFlags[lightIdx].setValue(theGeneratedKey, !isDirectional);
            renderer->defaultMaterialShaderKeyProperties().m_lightAreaFlags[lightIdx].setValue(theGeneratedKey, isArea);
            renderer->defaultMaterialShaderKeyProperties().m_lightShadowFlags[lightIdx].setValue(theGeneratedKey, castShadowsArea);
        }
    }
    return theGeneratedKey;
}

bool QDemonLayerRenderPreparationData::prepareTextForRender(QDemonText &inText,
                                                            const QMatrix4x4 &inViewProjection,
                                                            float inTextScaleFactor,
                                                            QDemonLayerRenderPreparationResultFlags &ioFlags)
{
    QDemonRef<QDemonTextTextureCacheInterface> theTextRenderer = renderer->getDemonContext()->getTextureCache();
    if (theTextRenderer == nullptr)
        return false;

    QDemonRenderableObjectFlags theFlags;
    theFlags.setHasTransparency(true);
    theFlags.setCompletelyTransparent(inText.globalOpacity < .01f);
    theFlags.setPickable(true);
    bool retval = false;

    if (theFlags.isCompletelyTransparent() == false) {
        retval = inText.flags.isDirty() || inText.flags.isTextDirty();
        inText.flags.setTextDirty(false);
        TTPathObjectAndTexture theResult = theTextRenderer->renderText(inText, inTextScaleFactor);
        inText.m_textTexture = theResult.second.second;
        inText.m_textTextureDetails = theResult.second.first;
        inText.m_pathFontItem = theResult.first.second;
        inText.m_pathFontDetails = theResult.first.first;
        QDemonTextScaleAndOffset theScaleAndOffset(*inText.m_textTexture, inText.m_textTextureDetails, inText);
        QVector2D theTextScale(theScaleAndOffset.textScale);
        QVector2D theTextOffset(theScaleAndOffset.textOffset);
        QVector3D minimum(theTextOffset[0] - theTextScale[0], theTextOffset[1] - theTextScale[1], 0);
        QVector3D maximum(theTextOffset[0] + theTextScale[0], theTextOffset[1] + theTextScale[1], 0);
        inText.m_bounds = QDemonBounds3(minimum, maximum);
        QMatrix4x4 theMVP;
        QMatrix3x3 theNormalMatrix;
        inText.calculateMVPAndNormalMatrix(inViewProjection, theMVP, theNormalMatrix);

        if (inText.m_pathFontDetails)
            ioFlags.setRequiresStencilBuffer(true);

        QDemonTextRenderable *theRenderable = RENDER_FRAME_NEW(QDemonTextRenderable)(theFlags,
                                                                                     inText.getGlobalPos(),
                                                                                     *renderer,
                                                                                     inText,
                                                                                     inText.m_bounds,
                                                                                     theMVP,
                                                                                     inViewProjection,
                                                                                     *inText.m_textTexture,
                                                                                     theTextOffset,
                                                                                     theTextScale);
        transparentObjects.push_back(theRenderable);
    }
    return retval;
}

QPair<bool, QDemonGraphObject *> QDemonLayerRenderPreparationData::resolveReferenceMaterial(QDemonGraphObject *inMaterial)
{
    bool subsetDirty = false;
    bool badIdea = false;
    QDemonGraphObject *theSourceMaterialObject(inMaterial);
    QDemonGraphObject *theMaterialObject(inMaterial);
    while (theMaterialObject && theMaterialObject->type == QDemonGraphObjectTypes::ReferencedMaterial && !badIdea) {
        QDemonReferencedMaterial *theRefMaterial = static_cast<QDemonReferencedMaterial *>(theMaterialObject);
        theMaterialObject = theRefMaterial->m_referencedMaterial;
        if (theMaterialObject == theSourceMaterialObject) {
            badIdea = true;
        }

        if (theRefMaterial == theSourceMaterialObject) {
            theRefMaterial->m_dirty.updateDirtyForFrame();
        }
        subsetDirty = subsetDirty | theRefMaterial->m_dirty.isDirty();
    }
    if (badIdea) {
        theMaterialObject = nullptr;
    }
    return QPair<bool, QDemonGraphObject *>(subsetDirty, theMaterialObject);
}

bool QDemonLayerRenderPreparationData::preparePathForRender(QDemonPath &inPath,
                                                            const QMatrix4x4 &inViewProjection,
                                                            const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                                                            QDemonLayerRenderPreparationResultFlags &ioFlags)
{
    QDemonRenderableObjectFlags theSharedFlags;
    theSharedFlags.setPickable(true);
    float subsetOpacity = inPath.globalOpacity;
    bool retval = inPath.flags.isDirty();
    inPath.flags.setDirty(false);
    QMatrix4x4 theMVP;
    QMatrix3x3 theNormalMatrix;

    inPath.calculateMVPAndNormalMatrix(inViewProjection, theMVP, theNormalMatrix);
    QDemonBounds3 theBounds(this->renderer->getDemonContext()->getPathManager()->getBounds(inPath));

    if (inPath.globalOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY && inClipFrustum.hasValue()) {
        // Check bounding box against the clipping planes
        QDemonBounds3 theGlobalBounds = theBounds;
        theGlobalBounds.transform(inPath.globalTransform);
        if (inClipFrustum->intersectsWith(theGlobalBounds) == false)
            subsetOpacity = 0.0f;
    }

    QDemonGraphObject *theMaterials[2] = { inPath.m_material, inPath.m_secondMaterial };

    if (inPath.m_pathType == PathTypes::Geometry || inPath.m_paintStyle != PathPaintStyles::FilledAndStroked)
        theMaterials[1] = nullptr;

    // We need to fill material to be the first one rendered so the stroke goes on top.
    // In the timeline, however, this is reversed.

    if (theMaterials[1])
        std::swap(theMaterials[1], theMaterials[0]);

    for (quint32 idx = 0, end = 2; idx < end; ++idx) {
        if (theMaterials[idx] == nullptr)
            continue;

        QDemonRenderableObjectFlags theFlags = theSharedFlags;

        QPair<bool, QDemonGraphObject *> theMaterialAndDirty(resolveReferenceMaterial(theMaterials[idx]));
        QDemonGraphObject *theMaterial(theMaterialAndDirty.second);
        retval = retval || theMaterialAndDirty.first;

        if (theMaterial != nullptr && theMaterial->type == QDemonGraphObjectTypes::DefaultMaterial) {
            QDemonRenderDefaultMaterial *theDefaultMaterial = static_cast<QDemonRenderDefaultMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            bool clearMaterialFlags = theMaterial == inPath.m_material;
            QDemonDefaultMaterialPreparationResult prepResult(
                    prepareDefaultMaterialForRender(*theDefaultMaterial, theFlags, subsetOpacity, clearMaterialFlags));

            theFlags = prepResult.renderableFlags;
            if (inPath.m_pathType == PathTypes::Geometry) {
                if ((inPath.m_beginCapping != PathCapping::Noner && inPath.m_beginCapOpacity < 1.0f)
                    || (inPath.m_endCapping != PathCapping::Noner && inPath.m_endCapOpacity < 1.0f))
                    theFlags.setHasTransparency(true);
            } else {
                ioFlags.setRequiresStencilBuffer(true);
            }
            retval = retval || prepResult.dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_pathType == PathTypes::Painted) {
                if (inPath.m_paintStyle == PathPaintStyles::Filled || inPath.m_paintStyle == PathPaintStyles::FilledAndStroked)
                    isStroke = false;
            }

            QDemonPathRenderable *theRenderable = RENDER_FRAME_NEW(QDemonPathRenderable)(theFlags,
                                                                                         inPath.getGlobalPos(),
                                                                                         renderer,
                                                                                         inPath.globalTransform,
                                                                                         theBounds,
                                                                                         inPath,
                                                                                         theMVP,
                                                                                         theNormalMatrix,
                                                                                         *theMaterial,
                                                                                         prepResult.opacity,
                                                                                         prepResult.materialKey,
                                                                                         isStroke);
            theRenderable->m_firstImage = prepResult.firstImage;

            QDemonRenderContextInterface *demonContext(renderer->getDemonContext());
            QDemonRef<QDemonPathManagerInterface> thePathManager = demonContext->getPathManager();
            retval = thePathManager->prepareForRender(inPath) || retval;
            retval |= (inPath.m_wireframeMode != demonContext->getWireframeMode());
            inPath.m_wireframeMode = demonContext->getWireframeMode();

            if (theFlags.hasTransparency())
                transparentObjects.push_back(theRenderable);
            else
                opaqueObjects.push_back(theRenderable);
        } else if (theMaterial != nullptr && theMaterial->type == QDemonGraphObjectTypes::CustomMaterial) {
            QDemonRenderCustomMaterial *theCustomMaterial = static_cast<QDemonRenderCustomMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            // bool clearMaterialFlags = theMaterial == inPath.m_Material;
            QDemonDefaultMaterialPreparationResult prepResult(prepareCustomMaterialForRender(*theCustomMaterial, theFlags, subsetOpacity));

            theFlags = prepResult.renderableFlags;
            if (inPath.m_pathType == PathTypes::Geometry) {
                if ((inPath.m_beginCapping != PathCapping::Noner && inPath.m_beginCapOpacity < 1.0f)
                    || (inPath.m_endCapping != PathCapping::Noner && inPath.m_endCapOpacity < 1.0f))
                    theFlags.setHasTransparency(true);
            } else {
                ioFlags.setRequiresStencilBuffer(true);
            }

            retval = retval || prepResult.dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_pathType == PathTypes::Painted) {
                if (inPath.m_paintStyle == PathPaintStyles::Filled || inPath.m_paintStyle == PathPaintStyles::FilledAndStroked)
                    isStroke = false;
            }

            QDemonPathRenderable *theRenderable = RENDER_FRAME_NEW(QDemonPathRenderable)(theFlags,
                                                                                         inPath.getGlobalPos(),
                                                                                         renderer,
                                                                                         inPath.globalTransform,
                                                                                         theBounds,
                                                                                         inPath,
                                                                                         theMVP,
                                                                                         theNormalMatrix,
                                                                                         *theMaterial,
                                                                                         prepResult.opacity,
                                                                                         prepResult.materialKey,
                                                                                         isStroke);
            theRenderable->m_firstImage = prepResult.firstImage;

            QDemonRenderContextInterface *demonContext(renderer->getDemonContext());
            QDemonRef<QDemonPathManagerInterface> thePathManager = demonContext->getPathManager();
            retval = thePathManager->prepareForRender(inPath) || retval;
            retval |= (inPath.m_wireframeMode != demonContext->getWireframeMode());
            inPath.m_wireframeMode = demonContext->getWireframeMode();

            if (theFlags.hasTransparency())
                transparentObjects.push_back(theRenderable);
            else
                opaqueObjects.push_back(theRenderable);
        }
    }
    return retval;
}

void QDemonLayerRenderPreparationData::prepareImageForRender(QDemonRenderImage &inImage,
                                                             QDemonImageMapTypes::Enum inMapType,
                                                             QDemonRenderableImage *&ioFirstImage,
                                                             QDemonRenderableImage *&ioNextImage,
                                                             QDemonRenderableObjectFlags &ioFlags,
                                                             QDemonShaderDefaultMaterialKey &inShaderKey,
                                                             quint32 inImageIndex)
{
    QDemonRenderContextInterface *demonContext(renderer->getDemonContext());
    QDemonBufferManager bufferManager = demonContext->getBufferManager();
    QDemonRef<QDemonOffscreenRenderManagerInterface> theOffscreenRenderManager(demonContext->getOffscreenRenderManager());
    //    IRenderPluginManager &theRenderPluginManager(demonContext.GetRenderPluginManager());
    if (inImage.clearDirty(bufferManager, *theOffscreenRenderManager /*, theRenderPluginManager*/))
        ioFlags |= RenderPreparationResultFlagValues::Dirty;

    // All objects with offscreen renderers are pickable so we can pass the pick through to the
    // offscreen renderer and let it deal with the pick.
    if (inImage.m_lastFrameOffscreenRenderer != nullptr) {
        ioFlags.setPickable(true);
        ioFlags |= RenderPreparationResultFlagValues::HasTransparency;
    }

    if (inImage.m_textureData.m_texture) {
        if (inImage.m_textureData.m_textureFlags.hasTransparency()
            && (inMapType == QDemonImageMapTypes::Diffuse || inMapType == QDemonImageMapTypes::Opacity
                || inMapType == QDemonImageMapTypes::Translucency)) {
            ioFlags |= RenderPreparationResultFlagValues::HasTransparency;
        }
        // Textures used in general have linear characteristics.
        // PKC -- The filters are properly set already.  Setting them here only overrides what
        // would
        // otherwise be a correct setting.
        // inImage.m_TextureData.m_Texture->SetMinFilter( QDemonRenderTextureMinifyingOp::Linear );
        // inImage.m_TextureData.m_Texture->SetMagFilter( QDemonRenderTextureMagnifyingOp::Linear );

        QDemonRenderableImage *theImage = RENDER_FRAME_NEW(QDemonRenderableImage)(inMapType, inImage);
        QDemonShaderKeyImageMap &theKeyProp = renderer->defaultMaterialShaderKeyProperties().m_imageMaps[inImageIndex];

        theKeyProp.setEnabled(inShaderKey, true);
        switch (inImage.m_mappingMode) {
        default:
            Q_ASSERT(false);
            // fallthrough intentional
        case ImageMappingModes::Normal:
            break;
        case ImageMappingModes::Environment:
            theKeyProp.setEnvMap(inShaderKey, true);
            break;
        case ImageMappingModes::LightProbe:
            theKeyProp.setLightProbe(inShaderKey, true);
            break;
        }

        if (inImage.m_textureData.m_textureFlags.isInvertUVCoords())
            theKeyProp.setInvertUVMap(inShaderKey, true);
        if (ioFirstImage == nullptr)
            ioFirstImage = theImage;
        else
            ioNextImage->m_nextImage = theImage;

        // assume offscreen renderer produces non-premultiplied image
        if (inImage.m_lastFrameOffscreenRenderer == nullptr && inImage.m_textureData.m_textureFlags.isPreMultiplied())
            theKeyProp.setPremultiplied(inShaderKey, true);

        QDemonShaderKeyTextureSwizzle &theSwizzleKeyProp = renderer->defaultMaterialShaderKeyProperties().m_textureSwizzle[inImageIndex];
        theSwizzleKeyProp.setSwizzleMode(inShaderKey, inImage.m_textureData.m_texture->getTextureSwizzleMode(), true);

        ioNextImage = theImage;
    }
}

QDemonDefaultMaterialPreparationResult QDemonLayerRenderPreparationData::prepareDefaultMaterialForRender(
        QDemonRenderDefaultMaterial &inMaterial,
        QDemonRenderableObjectFlags &inExistingFlags,
        float inOpacity,
        bool inClearDirtyFlags)
{
    QDemonRenderDefaultMaterial *theMaterial = &inMaterial;
    QDemonDefaultMaterialPreparationResult retval(generateLightingKey(theMaterial->lighting));
    retval.renderableFlags = inExistingFlags;
    QDemonRenderableObjectFlags &renderableFlags(retval.renderableFlags);
    QDemonShaderDefaultMaterialKey &theGeneratedKey(retval.materialKey);
    retval.opacity = inOpacity;
    float &subsetOpacity(retval.opacity);

    if (theMaterial->dirty.isDirty()) {
        renderableFlags |= RenderPreparationResultFlagValues::Dirty;
    }
    subsetOpacity *= theMaterial->opacity;
    if (inClearDirtyFlags)
        theMaterial->dirty.updateDirtyForFrame();

    QDemonRenderableImage *firstImage = nullptr;

    // set wireframe mode
    renderer->defaultMaterialShaderKeyProperties().m_wireframeMode.setValue(theGeneratedKey,
                                                                            renderer->getDemonContext()->getWireframeMode());

    if (theMaterial->iblProbe && checkLightProbeDirty(*theMaterial->iblProbe)) {
        renderer->prepareImageForIbl(*theMaterial->iblProbe);
    }

    if (!renderer->defaultMaterialShaderKeyProperties().m_hasIbl.getValue(theGeneratedKey)) {
        bool lightProbeValid = HasValidLightProbe(theMaterial->iblProbe);
        setShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", lightProbeValid);
        renderer->defaultMaterialShaderKeyProperties().m_hasIbl.setValue(theGeneratedKey, lightProbeValid);
        // SetShaderFeature( "QDEMON_ENABLE_IBL_FOV",
        // m_Renderer.GetLayerRenderData()->m_Layer.m_ProbeFov < 180.0f );
    }

    if (subsetOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {

        if (theMaterial->blendMode != DefaultMaterialBlendMode::Normal || theMaterial->opacityMap) {
            renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
        }

        bool specularEnabled = theMaterial->isSpecularEnabled();
        renderer->defaultMaterialShaderKeyProperties().m_specularEnabled.setValue(theGeneratedKey, specularEnabled);
        if (specularEnabled) {
            renderer->defaultMaterialShaderKeyProperties().m_specularModel.setSpecularModel(theGeneratedKey, theMaterial->specularModel);
        }

        renderer->defaultMaterialShaderKeyProperties().m_fresnelEnabled.setValue(theGeneratedKey, theMaterial->isFresnelEnabled());

        renderer->defaultMaterialShaderKeyProperties().m_vertexColorsEnabled.setValue(theGeneratedKey,
                                                                                      theMaterial->isVertexColorsEnabled());

        // Run through the material's images and prepare them for render.
        // this may in fact set pickable on the renderable flags if one of the images
        // links to a sub presentation or any offscreen rendered object.
        QDemonRenderableImage *nextImage = nullptr;
#define CHECK_IMAGE_AND_PREPARE(img, imgtype, shadercomponent)                                                         \
    if ((img))                                                                                                         \
        prepareImageForRender(*(img), imgtype, firstImage, nextImage, renderableFlags, theGeneratedKey, shadercomponent);

        CHECK_IMAGE_AND_PREPARE(theMaterial->diffuseMaps[0],
                                QDemonImageMapTypes::Diffuse,
                                QDemonShaderDefaultMaterialKeyProperties::DiffuseMap0);
        CHECK_IMAGE_AND_PREPARE(theMaterial->diffuseMaps[1],
                                QDemonImageMapTypes::Diffuse,
                                QDemonShaderDefaultMaterialKeyProperties::DiffuseMap1);
        CHECK_IMAGE_AND_PREPARE(theMaterial->diffuseMaps[2],
                                QDemonImageMapTypes::Diffuse,
                                QDemonShaderDefaultMaterialKeyProperties::DiffuseMap2);
        CHECK_IMAGE_AND_PREPARE(theMaterial->emissiveMap, QDemonImageMapTypes::Emissive, QDemonShaderDefaultMaterialKeyProperties::EmissiveMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->emissiveMap2, QDemonImageMapTypes::Emissive, QDemonShaderDefaultMaterialKeyProperties::EmissiveMap2);
        CHECK_IMAGE_AND_PREPARE(theMaterial->specularReflection,
                                QDemonImageMapTypes::Specular,
                                QDemonShaderDefaultMaterialKeyProperties::SpecularMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->roughnessMap,
                                QDemonImageMapTypes::Roughness,
                                QDemonShaderDefaultMaterialKeyProperties::RoughnessMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->opacityMap, QDemonImageMapTypes::Opacity, QDemonShaderDefaultMaterialKeyProperties::OpacityMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->bumpMap, QDemonImageMapTypes::Bump, QDemonShaderDefaultMaterialKeyProperties::BumpMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->specularMap,
                                QDemonImageMapTypes::SpecularAmountMap,
                                QDemonShaderDefaultMaterialKeyProperties::SpecularAmountMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->normalMap, QDemonImageMapTypes::Normal, QDemonShaderDefaultMaterialKeyProperties::NormalMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->displacementMap,
                                QDemonImageMapTypes::Displacement,
                                QDemonShaderDefaultMaterialKeyProperties::DisplacementMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->translucencyMap,
                                QDemonImageMapTypes::Translucency,
                                QDemonShaderDefaultMaterialKeyProperties::TranslucencyMap);
        CHECK_IMAGE_AND_PREPARE(theMaterial->lightmaps.m_lightmapIndirect,
                                QDemonImageMapTypes::LightmapIndirect,
                                QDemonShaderDefaultMaterialKeyProperties::LightmapIndirect);
        CHECK_IMAGE_AND_PREPARE(theMaterial->lightmaps.m_lightmapRadiosity,
                                QDemonImageMapTypes::LightmapRadiosity,
                                QDemonShaderDefaultMaterialKeyProperties::LightmapRadiosity);
        CHECK_IMAGE_AND_PREPARE(theMaterial->lightmaps.m_lightmapShadow,
                                QDemonImageMapTypes::LightmapShadow,
                                QDemonShaderDefaultMaterialKeyProperties::LightmapShadow);
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

    if (isNotOne(subsetOpacity))
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;

    retval.firstImage = firstImage;
    if (retval.renderableFlags.isDirty())
        retval.dirty = true;
    return retval;
}

QDemonDefaultMaterialPreparationResult QDemonLayerRenderPreparationData::prepareCustomMaterialForRender(QDemonRenderCustomMaterial &inMaterial,
                                                                                                        QDemonRenderableObjectFlags &inExistingFlags,
                                                                                                        float inOpacity)
{
    QDemonDefaultMaterialPreparationResult retval(generateLightingKey(DefaultMaterialLighting::FragmentLighting)); // always fragment lighting
    retval.renderableFlags = inExistingFlags;
    QDemonRenderableObjectFlags &renderableFlags(retval.renderableFlags);
    QDemonShaderDefaultMaterialKey &theGeneratedKey(retval.materialKey);
    retval.opacity = inOpacity;
    float &subsetOpacity(retval.opacity);

    // set wireframe mode
    renderer->defaultMaterialShaderKeyProperties().m_wireframeMode.setValue(theGeneratedKey,
                                                                            renderer->getDemonContext()->getWireframeMode());

    if (subsetOpacity < QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {
        subsetOpacity = 0.0f;
        // You can still pick against completely transparent objects(or rather their bounding
        // box)
        // you just don't render them.
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
        renderableFlags |= RenderPreparationResultFlagValues::CompletelyTransparent;
    }

    if (isNotOne(subsetOpacity))
        renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;

    QDemonRenderableImage *firstImage = nullptr;
    QDemonRenderableImage *nextImage = nullptr;

#define CHECK_IMAGE_AND_PREPARE(img, imgtype, shadercomponent)                                                         \
    if ((img))                                                                                                         \
        prepareImageForRender(*(img), imgtype, firstImage, nextImage, renderableFlags, theGeneratedKey, shadercomponent);

    CHECK_IMAGE_AND_PREPARE(inMaterial.m_displacementMap,
                            QDemonImageMapTypes::Displacement,
                            QDemonShaderDefaultMaterialKeyProperties::DisplacementMap);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_lightmaps.m_lightmapIndirect,
                            QDemonImageMapTypes::LightmapIndirect,
                            QDemonShaderDefaultMaterialKeyProperties::LightmapIndirect);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_lightmaps.m_lightmapRadiosity,
                            QDemonImageMapTypes::LightmapRadiosity,
                            QDemonShaderDefaultMaterialKeyProperties::LightmapRadiosity);
    CHECK_IMAGE_AND_PREPARE(inMaterial.m_lightmaps.m_lightmapShadow,
                            QDemonImageMapTypes::LightmapShadow,
                            QDemonShaderDefaultMaterialKeyProperties::LightmapShadow);
#undef CHECK_IMAGE_AND_PREPARE

    retval.firstImage = firstImage;
    return retval;
}

bool QDemonLayerRenderPreparationData::prepareModelForRender(QDemonRenderModel &inModel,
                                                             const QMatrix4x4 &inViewProjection,
                                                             const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                                                             QDemonNodeLightEntryList &inScopedLights)
{
    QDemonRenderContextInterface *demonContext(renderer->getDemonContext());
    QDemonBufferManager bufferManager = demonContext->getBufferManager();
    QDemonRenderMesh *theMesh = bufferManager.loadMesh(inModel.meshPath);
    if (theMesh == nullptr)
        return false;

    QDemonGraphObject *theSourceMaterialObject = inModel.firstMaterial;
    QDemonModelContext &theModelContext = *RENDER_FRAME_NEW(QDemonModelContext)(inModel, inViewProjection);
    modelContexts.push_back(&theModelContext);

    bool subsetDirty = false;

    QDemonScopedLightsListScope lightsScope(lights, lightDirections, sourceLightDirections, inScopedLights);
    setShaderFeature(cgLightingFeatureName, lights.empty() == false);
    for (quint32 idx = 0, end = theMesh->subsets.size(); idx < end && theSourceMaterialObject;
         ++idx, theSourceMaterialObject = getNextMaterialSibling(theSourceMaterialObject)) {
        QDemonRenderSubset &theOuterSubset(theMesh->subsets[idx]);
        {
            QDemonRenderSubset &theSubset(theOuterSubset);
            QDemonRenderableObjectFlags renderableFlags;
            renderableFlags.setPickable(false);
            float subsetOpacity = inModel.globalOpacity;
            QVector3D theModelCenter(theSubset.bounds.getCenter());
            theModelCenter = mat44::transform(inModel.globalTransform, theModelCenter);

            if (subsetOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY && inClipFrustum.hasValue()) {
                // Check bounding box against the clipping planes
                QDemonBounds3 theGlobalBounds = theSubset.bounds;
                theGlobalBounds.transform(theModelContext.model.globalTransform);
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
            bool canModelBePickable = inModel.globalOpacity > .01f;
            renderableFlags.setPickable(canModelBePickable
                                        && (theModelContext.model.flags.isGloballyPickable() || renderableFlags.getPickable()));
            QDemonRenderableObject *theRenderableObject = nullptr;
            QPair<bool, QDemonGraphObject *> theMaterialObjectAndDirty = resolveReferenceMaterial(theSourceMaterialObject);
            QDemonGraphObject *theMaterialObject = theMaterialObjectAndDirty.second;
            subsetDirty = subsetDirty || theMaterialObjectAndDirty.first;
            if (theMaterialObject == nullptr)
                continue;

            // set tessellation
            if (inModel.tessellationMode != TessModeValues::NoTess) {
                theSubset.primitiveType = QDemonRenderDrawMode::Patches;
                // set tessellation factor
                theSubset.edgeTessFactor = inModel.edgeTess;
                theSubset.innerTessFactor = inModel.innerTess;
                // update the vertex ver patch count in the input assembler
                // currently we only support triangle patches so count is always 3
                theSubset.inputAssembler->setPatchVertexCount(3);
                theSubset.inputAssemblerDepth->setPatchVertexCount(3);
                // check wireframe mode
                theSubset.wireframeMode = demonContext->getWireframeMode();

                subsetDirty = subsetDirty | (theSubset.wireframeMode != inModel.wireframeMode);
                inModel.wireframeMode = demonContext->getWireframeMode();
            } else {
                theSubset.primitiveType = theSubset.inputAssembler->getPrimitiveType();
                theSubset.inputAssembler->setPatchVertexCount(1);
                theSubset.inputAssemblerDepth->setPatchVertexCount(1);
                // currently we allow wirframe mode only if tessellation is on
                theSubset.wireframeMode = false;

                subsetDirty = subsetDirty | (theSubset.wireframeMode != inModel.wireframeMode);
                inModel.wireframeMode = false;
            }
            // Only clear flags on the materials in this direct hierarchy.  Do not clear them of
            // this
            // references materials in another hierarchy.
            bool clearMaterialDirtyFlags = theMaterialObject == theSourceMaterialObject;

            if (theMaterialObject == nullptr)
                continue;

            if (theMaterialObject->type == QDemonGraphObjectTypes::DefaultMaterial) {
                QDemonRenderDefaultMaterial &theMaterial(static_cast<QDemonRenderDefaultMaterial &>(*theMaterialObject));
                QDemonDefaultMaterialPreparationResult theMaterialPrepResult(
                        prepareDefaultMaterialForRender(theMaterial, renderableFlags, subsetOpacity, clearMaterialDirtyFlags));
                QDemonShaderDefaultMaterialKey theGeneratedKey = theMaterialPrepResult.materialKey;
                subsetOpacity = theMaterialPrepResult.opacity;
                QDemonRenderableImage *firstImage(theMaterialPrepResult.firstImage);
                subsetDirty |= theMaterialPrepResult.dirty;
                renderableFlags = theMaterialPrepResult.renderableFlags;

                renderer->defaultMaterialShaderKeyProperties().m_tessellationMode.setTessellationMode(theGeneratedKey,
                                                                                                      inModel.tessellationMode,
                                                                                                      true);

                QDemonConstDataRef<QMatrix4x4> boneGlobals;
                if (theSubset.joints.size()) {
                    Q_ASSERT(false);
                }

                theRenderableObject = RENDER_FRAME_NEW(QDemonSubsetRenderable)(renderableFlags,
                                                                               theModelCenter,
                                                                               renderer,
                                                                               theSubset,
                                                                               theMaterial,
                                                                               theModelContext,
                                                                               subsetOpacity,
                                                                               firstImage,
                                                                               theGeneratedKey,
                                                                               boneGlobals);
                subsetDirty = subsetDirty || renderableFlags.isDirty();
            } else if (theMaterialObject->type == QDemonGraphObjectTypes::CustomMaterial) {
                QDemonRenderCustomMaterial &theMaterial(static_cast<QDemonRenderCustomMaterial &>(*theMaterialObject));

                QDemonMaterialSystem theMaterialSystem(demonContext->getCustomMaterialSystem());
                subsetDirty |= theMaterialSystem.prepareForRender(theModelContext.model, theSubset, theMaterial, clearMaterialDirtyFlags);

                QDemonDefaultMaterialPreparationResult theMaterialPrepResult(
                        prepareCustomMaterialForRender(theMaterial, renderableFlags, subsetOpacity));
                QDemonShaderDefaultMaterialKey theGeneratedKey = theMaterialPrepResult.materialKey;
                subsetOpacity = theMaterialPrepResult.opacity;
                QDemonRenderableImage *firstImage(theMaterialPrepResult.firstImage);
                renderableFlags = theMaterialPrepResult.renderableFlags;

                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasTransparency)
                    renderableFlags |= RenderPreparationResultFlagValues::HasTransparency;
                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasRefraction)
                    renderableFlags |= RenderPreparationResultFlagValues::HasRefraction;

                renderer->defaultMaterialShaderKeyProperties().m_tessellationMode.setTessellationMode(theGeneratedKey,
                                                                                                      inModel.tessellationMode,
                                                                                                      true);

                if (theMaterial.m_iblProbe && checkLightProbeDirty(*theMaterial.m_iblProbe)) {
                    renderer->prepareImageForIbl(*theMaterial.m_iblProbe);
                }

                theRenderableObject = RENDER_FRAME_NEW(QDemonCustomMaterialRenderable)(renderableFlags,
                                                                                       theModelCenter,
                                                                                       renderer,
                                                                                       theSubset,
                                                                                       theMaterial,
                                                                                       theModelContext,
                                                                                       subsetOpacity,
                                                                                       firstImage,
                                                                                       theGeneratedKey);
            }
            if (theRenderableObject) {
                theRenderableObject->scopedLights = inScopedLights;
                // set tessellation
                theRenderableObject->tessellationMode = inModel.tessellationMode;

                if (theRenderableObject->renderableFlags.hasTransparency() || theRenderableObject->renderableFlags.hasRefraction()) {
                    transparentObjects.push_back(theRenderableObject);
                } else {
                    opaqueObjects.push_back(theRenderableObject);
                }
            }
        }
    }
    return subsetDirty;
}

bool QDemonLayerRenderPreparationData::prepareRenderablesForRender(const QMatrix4x4 &inViewProjection,
                                                                   const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                                                                   float inTextScaleFactor,
                                                                   QDemonLayerRenderPreparationResultFlags &ioFlags)
{
    QDemonStackPerfTimer __timer(renderer->getDemonContext()->getPerfTimer(), "SLayerRenderData::PrepareRenderablesForRender");
    viewProjection = inViewProjection;
    float theTextScaleFactor = inTextScaleFactor;
    bool wasDataDirty = false;
    bool hasTextRenderer = renderer->getDemonContext()->getTextRenderer() != nullptr;
    for (quint32 idx = 0, end = renderableNodes.size(); idx < end; ++idx) {
        QDemonRenderableNodeEntry &theNodeEntry(renderableNodes[idx]);
        QDemonGraphNode *theNode = theNodeEntry.node;
        wasDataDirty = wasDataDirty || theNode->flags.isDirty();
        switch (theNode->type) {
        case QDemonGraphObjectTypes::Model: {
            QDemonRenderModel *theModel = static_cast<QDemonRenderModel *>(theNode);
            theModel->calculateGlobalVariables();
            if (theModel->flags.isGloballyActive()) {
                bool wasModelDirty = prepareModelForRender(*theModel, inViewProjection, inClipFrustum, theNodeEntry.lights);
                wasDataDirty = wasDataDirty || wasModelDirty;
            }
        } break;
        case QDemonGraphObjectTypes::Text: {
            if (hasTextRenderer) {
                QDemonText *theText = static_cast<QDemonText *>(theNode);
                theText->calculateGlobalVariables();
                if (theText->flags.isGloballyActive()) {
                    bool wasTextDirty = prepareTextForRender(*theText, inViewProjection, theTextScaleFactor, ioFlags);
                    wasDataDirty = wasDataDirty || wasTextDirty;
                }
            }
        } break;
        case QDemonGraphObjectTypes::Path: {
            QDemonPath *thePath = static_cast<QDemonPath *>(theNode);
            thePath->calculateGlobalVariables();
            if (thePath->flags.isGloballyActive()) {
                bool wasPathDirty = preparePathForRender(*thePath, inViewProjection, inClipFrustum, ioFlags);
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

bool QDemonLayerRenderPreparationData::checkLightProbeDirty(QDemonRenderImage &inLightProbe)
{
    QDemonRenderContextInterface *theContext(renderer->getDemonContext());
    QDemonBufferManager bufferManager = theContext->getBufferManager();
    return inLightProbe.clearDirty(bufferManager,
                                   *theContext->getOffscreenRenderManager() /*,
                                    theContext.GetRenderPluginManager()*/
                                   ,
                                   true);
}

struct QDemonLightNodeMarker
{
    QDemonRenderLight *light = nullptr;
    quint32 lightIndex = 0;
    quint32 firstValidIndex = 0;
    quint32 justPastLastValidIndex = 0;
    bool addOrRemove = false;
    QDemonLightNodeMarker() = default;
    QDemonLightNodeMarker(QDemonRenderLight &inLight, quint32 inLightIndex, QDemonGraphNode &inNode, bool aorm)
        : light(&inLight), lightIndex(inLightIndex), addOrRemove(aorm)
    {
        if (inNode.type == QDemonGraphObjectTypes::Layer) {
            firstValidIndex = 0;
            justPastLastValidIndex = std::numeric_limits<quint32>::max();
        } else {
            firstValidIndex = inNode.dfsIndex;
            QDemonGraphNode *lastChild = nullptr;
            QDemonGraphNode *firstChild = inNode.firstChild;
            // find deepest last child
            while (firstChild) {
                for (QDemonGraphNode *childNode = firstChild; childNode; childNode = childNode->nextSibling)
                    lastChild = childNode;

                if (lastChild)
                    firstChild = lastChild->firstChild;
                else
                    firstChild = nullptr;
            }
            if (lastChild)
                // last valid index would be the last child index + 1
                justPastLastValidIndex = lastChild->dfsIndex + 1;
            else // no children.
                justPastLastValidIndex = firstValidIndex + 1;
        }
    }
};

// m_Layer.m_Camera->CalculateViewProjectionMatrix(m_ViewProjection);
void QDemonLayerRenderPreparationData::prepareForRender(const QSize &inViewportDimensions)
{
    QDemonStackPerfTimer __timer(renderer->getDemonContext()->getPerfTimer(), "SLayerRenderData::PrepareForRender");
    if (layerPrepResult.hasValue())
        return;

    features.clear();
    featureSetHash = 0;
    QVector2D thePresentationDimensions((float)inViewportDimensions.width(), (float)inViewportDimensions.height());
    QDemonRef<QDemonRenderListInterface> theGraph(renderer->getDemonContext()->getRenderList());
    QRect theViewport(theGraph->getViewport());
    QRect theScissor(theGraph->getViewport());
    if (theGraph->isScissorTestEnabled())
        theScissor = renderer->getContext()->getScissorRect();
    bool wasDirty = false;
    bool wasDataDirty = false;
    wasDirty = layer.flags.isDirty();
    // The first pass is just to render the data.
    quint32 maxNumAAPasses = layer.progressiveAAMode == AAModeValues::NoAA ? (quint32)0 : (quint32)(layer.progressiveAAMode) + 1;
    maxNumAAPasses = qMin((quint32)(MAX_AA_LEVELS + 1), maxNumAAPasses);
    QDemonRenderEffect *theLastEffect = nullptr;
    // Uncomment the line below to disable all progressive AA.
    // maxNumAAPasses = 0;

    QDemonLayerRenderPreparationResult thePrepResult;
    bool hasOffscreenRenderer = getOffscreenRenderer();

    bool SSAOEnabled = (layer.aoStrength > 0.0f && layer.aoDistance > 0.0f);
    bool SSDOEnabled = (layer.shadowStrength > 0.0f && layer.shadowDist > 0.0f);
    setShaderFeature("QDEMON_ENABLE_SSAO", SSAOEnabled);
    setShaderFeature("QDEMON_ENABLE_SSDO", SSDOEnabled);
    bool requiresDepthPrepass = (hasOffscreenRenderer == false) && (SSAOEnabled || SSDOEnabled);
    setShaderFeature("QDEMON_ENABLE_SSM", false); // by default no shadow map generation

    if (layer.flags.isActive()) {
        // Get the layer's width and height.
        QDemonRef<QDemonEffectSystemInterface> theEffectSystem(renderer->getDemonContext()->getEffectSystem());
        for (QDemonRenderEffect *theEffect = layer.firstEffect; theEffect; theEffect = theEffect->m_nextEffect) {
            if (theEffect->flags.isDirty()) {
                wasDirty = true;
                theEffect->flags.setDirty(false);
            }
            if (theEffect->flags.isActive()) {
                theLastEffect = theEffect;
                if (hasOffscreenRenderer == false && theEffectSystem->doesEffectRequireDepthTexture(theEffect->className))
                    requiresDepthPrepass = true;
            }
        }
        if (layer.flags.isDirty()) {
            wasDirty = true;
            layer.calculateGlobalVariables();
        }

        bool shouldRenderToTexture = true;

        if (hasOffscreenRenderer) {
            // We don't render to texture with offscreen renderers, we just render them to the
            // viewport.
            shouldRenderToTexture = false;
            // Progaa disabled when using offscreen rendering.
            maxNumAAPasses = 0;
        }

        thePrepResult = QDemonLayerRenderPreparationResult(
                QDemonLayerRenderHelper(theViewport,
                                        theScissor,
                                        layer.scene->presentation->presentationDimensions,
                                        layer,
                                        shouldRenderToTexture,
                                        renderer->getDemonContext()->getScaleMode(),
                                        renderer->getDemonContext()->getPresentationScaleFactor()));
        thePrepResult.lastEffect = theLastEffect;
        thePrepResult.maxAAPassIndex = maxNumAAPasses;
        thePrepResult.flags.setRequiresDepthTexture(requiresDepthPrepass || needsWidgetTexture());
        thePrepResult.flags.setShouldRenderToTexture(shouldRenderToTexture);
        if (renderer->getContext()->getRenderContextType() != QDemonRenderContextValues::GLES2)
            thePrepResult.flags.setRequiresSsaoPass(SSAOEnabled);

        if (thePrepResult.isLayerVisible()) {
            if (shouldRenderToTexture) {
                renderer->getDemonContext()->getRenderList()->addRenderTask(createRenderToTextureRunnable());
            }
            if (layer.lightProbe && checkLightProbeDirty(*layer.lightProbe)) {
                renderer->prepareImageForIbl(*layer.lightProbe);
                wasDataDirty = true;
            }

            bool lightProbeValid = HasValidLightProbe(layer.lightProbe);

            setShaderFeature("QDEMON_ENABLE_LIGHT_PROBE", lightProbeValid);
            setShaderFeature("QDEMON_ENABLE_IBL_FOV", layer.probeFov < 180.0f);

            if (lightProbeValid && layer.lightProbe2 && checkLightProbeDirty(*layer.lightProbe2)) {
                renderer->prepareImageForIbl(*layer.lightProbe2);
                wasDataDirty = true;
            }

            setShaderFeature("QDEMON_ENABLE_LIGHT_PROBE_2", lightProbeValid && HasValidLightProbe(layer.lightProbe2));

            // Push nodes in reverse depth first order
            if (renderableNodes.empty()) {
                camerasAndLights.clear();
                quint32 dfsIndex = 0;
                for (QDemonGraphNode *theChild = layer.firstChild; theChild; theChild = theChild->nextSibling)
                    MaybeQueueNodeForRender(*theChild, renderableNodes, camerasAndLights, dfsIndex);
                std::reverse(camerasAndLights.begin(), camerasAndLights.end());
                std::reverse(renderableNodes.begin(), renderableNodes.end());
                lightToNodeMap.clear();
            }
            camera = nullptr;
            lights.clear();
            opaqueObjects.clear();
            qDeleteAll(opaqueObjects);
            transparentObjects.clear();
            qDeleteAll(transparentObjects);
            QVector<QDemonLightNodeMarker> theLightNodeMarkers;
            sourceLightDirections.clear();

            for (quint32 idx = 0, end = camerasAndLights.size(); idx < end; ++idx) {
                QDemonGraphNode *theNode(camerasAndLights[idx]);
                wasDataDirty = wasDataDirty || theNode->flags.isDirty();
                switch (theNode->type) {
                case QDemonGraphObjectTypes::Camera: {
                    QDemonRenderCamera *theCamera = static_cast<QDemonRenderCamera *>(theNode);
                    QDemonCameraGlobalCalculationResult theResult = thePrepResult.setupCameraForRender(*theCamera);
                    wasDataDirty = wasDataDirty || theResult.m_wasDirty;
                    if (theCamera->flags.isGloballyActive())
                        camera = theCamera;
                    if (theResult.m_computeFrustumSucceeded == false) {
                        qCCritical(INTERNAL_ERROR, "Failed to calculate camera frustum");
                    }
                } break;
                case QDemonGraphObjectTypes::Light: {
                    QDemonRenderLight *theLight = static_cast<QDemonRenderLight *>(theNode);
                    bool lightResult = theLight->calculateGlobalVariables();
                    wasDataDirty = lightResult || wasDataDirty;
                    // Note we setup the light index such that it is completely invariant of if
                    // the
                    // light is active or scoped.
                    quint32 lightIndex = (quint32)sourceLightDirections.size();
                    sourceLightDirections.push_back(QVector3D(0.0, 0.0, 0.0));
                    // Note we still need a light check when building the renderable light list.
                    // We also cannot cache shader-light bindings based on layers any more
                    // because
                    // the number of lights for a given renderable does not depend on the layer
                    // as it used to but
                    // additional perhaps on the light's scoping rules.
                    if (theLight->flags.isGloballyActive()) {
                        if (theLight->m_scope == nullptr) {
                            lights.push_back(theLight);
                            if (renderer->getContext()->getRenderContextType() != QDemonRenderContextValues::GLES2
                                && theLight->m_castShadow && getShadowMapManager()) {
                                // PKC -- use of "res" as an exponent of two is an annoying
                                // artifact of the XML interface
                                // I'll change this with an enum interface later on, but that's
                                // less important right now.
                                quint32 mapSize = 1 << theLight->m_shadowMapRes;
                                ShadowMapModes::Enum mapMode = (theLight->m_lightType != RenderLightTypes::Directional)
                                        ? ShadowMapModes::CUBE
                                        : ShadowMapModes::VSM;
                                shadowMapManager->addShadowMapEntry(lights.size() - 1,
                                                                    mapSize,
                                                                    mapSize,
                                                                    QDemonRenderTextureFormats::R16F,
                                                                    1,
                                                                    mapMode,
                                                                    ShadowFilterValues::NONE);
                                thePrepResult.flags.setRequiresShadowMapPass(true);
                                setShaderFeature("QDEMON_ENABLE_SSM", true);
                            }
                        }
                        TLightToNodeMap::iterator iter = lightToNodeMap.insert(theLight, (QDemonGraphNode *)nullptr);
                        QDemonGraphNode *oldLightScope = iter.value();
                        QDemonGraphNode *newLightScope = theLight->m_scope;

                        if (oldLightScope != newLightScope) {
                            iter.value() = newLightScope;
                            if (oldLightScope)
                                theLightNodeMarkers.push_back(QDemonLightNodeMarker(*theLight, lightIndex, *oldLightScope, false));
                            if (newLightScope)
                                theLightNodeMarkers.push_back(QDemonLightNodeMarker(*theLight, lightIndex, *newLightScope, true));
                        }
                        if (newLightScope) {
                            sourceLightDirections.back() = theLight->getScalingCorrectDirection();
                        }
                    }
                } break;
                default:
                    Q_ASSERT(false);
                    break;
                }
            }

            if (theLightNodeMarkers.empty() == false) {
                for (quint32 idx = 0, end = renderableNodes.size(); idx < end; ++idx) {
                    QDemonRenderableNodeEntry &theNodeEntry(renderableNodes[idx]);
                    quint32 nodeDFSIndex = theNodeEntry.node->dfsIndex;
                    for (quint32 markerIdx = 0, markerEnd = theLightNodeMarkers.size(); markerIdx < markerEnd; ++markerIdx) {
                        QDemonLightNodeMarker &theMarker = theLightNodeMarkers[markerIdx];
                        if (nodeDFSIndex >= theMarker.firstValidIndex && nodeDFSIndex < theMarker.justPastLastValidIndex) {
                            if (theMarker.addOrRemove) {
                                QDemonNodeLightEntry *theNewEntry = new QDemonNodeLightEntry(theMarker.light, theMarker.lightIndex);
                                theNodeEntry.lights.push_back(*theNewEntry);
                            } else {
                                for (QDemonNodeLightEntryList::iterator lightIter = theNodeEntry.lights.begin(),
                                                                        lightEnd = theNodeEntry.lights.end();
                                     lightIter != lightEnd;
                                     ++lightIter) {
                                    if (lightIter->light == theMarker.light) {
                                        QDemonNodeLightEntry &theEntry = *lightIter;
                                        theNodeEntry.lights.remove(theEntry);
                                        delete &theEntry;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            float theTextScaleFactor = 1.0f;
            if (camera) {
                camera->calculateViewProjectionMatrix(viewProjection);
                theTextScaleFactor = camera->getTextScaleFactor(thePrepResult.getLayerToPresentationViewport(),
                                                                thePrepResult.getPresentationDesignDimensions());
                QDemonClipPlane nearPlane;
                QMatrix3x3 theUpper33(camera->globalTransform.normalMatrix());

                QVector3D dir(mat33::transform(theUpper33, QVector3D(0, 0, -1)));
                dir.normalize();
                nearPlane.normal = dir;
                QVector3D theGlobalPos = camera->getGlobalPos() + camera->clipNear * dir;
                nearPlane.d = -(QVector3D::dotProduct(dir, theGlobalPos));
                // the near plane's bbox edges are calculated in the clipping frustum's
                // constructor.
                clippingFrustum = QDemonClippingFrustum(viewProjection, nearPlane);
            } else
                viewProjection = QMatrix4x4();

            // Setup the light directions here.

            for (quint32 lightIdx = 0, lightEnd = lights.size(); lightIdx < lightEnd; ++lightIdx) {
                lightDirections.push_back(lights[lightIdx]->getScalingCorrectDirection());
            }

            modelContexts.clear();
            if (getOffscreenRenderer() == false) {
                bool renderablesDirty = prepareRenderablesForRender(viewProjection,
                                                                    clippingFrustum,
                                                                    theTextScaleFactor,
                                                                    thePrepResult.flags);
                wasDataDirty = wasDataDirty || renderablesDirty;
                if (thePrepResult.flags.requiresStencilBuffer())
                    thePrepResult.flags.setShouldRenderToTexture(true);
            } else {
                QRect theViewport = thePrepResult.getLayerToPresentationViewport().toRect();
                bool theScissor = true;
                QRect theScissorRect = thePrepResult.getLayerToPresentationScissorRect().toRect();
                // This happens here because if there are any fancy render steps
                QDemonRef<QDemonRenderListInterface> theRenderList(renderer->getDemonContext()->getRenderList());
                auto theContext = renderer->getContext();
                QDemonRenderListScopedProperty<bool> _listScissorEnabled(*theRenderList,
                                                                         &QDemonRenderListInterface::isScissorTestEnabled,
                                                                         &QDemonRenderListInterface::setScissorTestEnabled,
                                                                         theScissor);
                QDemonRenderListScopedProperty<QRect> _listViewport(*theRenderList,
                                                                    &QDemonRenderListInterface::getViewport,
                                                                    &QDemonRenderListInterface::setViewport,
                                                                    theViewport);
                QDemonRenderListScopedProperty<QRect> _listScissor(*theRenderList,
                                                                   &QDemonRenderListInterface::getScissor,
                                                                   &QDemonRenderListInterface::setScissorRect,
                                                                   theScissorRect);
                // Some plugins don't use the render list so they need the actual gl context
                // setup.
                QDemonRenderContextScopedProperty<bool> __scissorEnabled(*theContext,
                                                                         &QDemonRenderContext::isScissorTestEnabled,
                                                                         &QDemonRenderContext::setScissorTestEnabled,
                                                                         true);
                QDemonRenderContextScopedProperty<QRect> __scissorRect(*theContext,
                                                                       &QDemonRenderContext::getScissorRect,
                                                                       &QDemonRenderContext::setScissorRect,
                                                                       theScissorRect);
                QDemonRenderContextScopedProperty<QRect> __viewportRect(*theContext,
                                                                        &QDemonRenderContext::getViewport,
                                                                        &QDemonRenderContext::setViewport,
                                                                        theViewport);
                QDemonOffscreenRenderFlags theResult = lastFrameOffscreenRenderer
                                                               ->needsRender(createOffscreenRenderEnvironment(),
                                                                             renderer->getDemonContext()->getPresentationScaleFactor(),
                                                                             &layer);
                wasDataDirty = wasDataDirty || theResult.hasChangedSinceLastFrame;
            }
        }
    }
    wasDirty = wasDirty || wasDataDirty;
    thePrepResult.flags.setWasDirty(wasDirty);
    thePrepResult.flags.setLayerDataDirty(wasDataDirty);

    layerPrepResult = thePrepResult;

    // Per-frame cache of renderable objects post-sort.
    getOpaqueRenderableObjects();
    // If layer depth test is false, this may also contain opaque objects.
    getTransparentRenderableObjects();

    getCameraDirection();
}

void QDemonLayerRenderPreparationData::resetForFrame()
{
    transparentObjects.clear();
    opaqueObjects.clear();
    layerPrepResult.setEmpty();
    // The check for if the camera is or is not null is used
    // to figure out if this layer was rendered at all.
    camera = nullptr;
    lastFrameOffscreenRenderer = nullptr;
    iRenderWidgets.clear();
    cameraDirection.setEmpty();
    lightDirections.clear();
    renderedOpaqueObjects.clear();
    renderedTransparentObjects.clear();
}

QT_END_NAMESPACE
