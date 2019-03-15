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
#include <QtDemonRuntimeRender/qdemonrenderreferencedmaterial.h>
#include <QtDemonRuntimeRender/qdemonrendereffectsystem.h>
#include <QtDemonRender/qdemonrenderframebuffer.h>
#include <QtDemonRender/qdemonrenderrenderbuffer.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrenderkey.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugin.h>
//#include <QtDemonRuntimeRender/qdemonrenderplugingraphobject.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcebufferobjects.h>
#include <QtDemon/qdemonperftimer.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterialsystem.h>
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
void MaybeQueueNodeForRender(QDemonRenderNode &inNode,
                             QVector<QDemonRenderableNodeEntry> &outRenderables,
                             QVector<QDemonRenderNode *> &outCamerasAndLights,
                             quint32 &ioDFSIndex)
{
    ++ioDFSIndex;
    inNode.dfsIndex = ioDFSIndex;
    if (inNode.isRenderableType())
        outRenderables.push_back(inNode);
    else if (inNode.isLightCameraType())
        outCamerasAndLights.push_back(&inNode);

    for (QDemonRenderNode *theChild = inNode.firstChild; theChild != nullptr; theChild = theChild->nextSibling)
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

void QDemonLayerRenderPreparationData::createShadowMapManager()
{
    if (shadowMapManager)
        return;

    shadowMapManager = QDemonRenderShadowMap::create(renderer->demonContext());
}

bool QDemonLayerRenderPreparationData::usesOffscreenRenderer()
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
        lastFrameOffscreenRenderer = renderer->demonContext()->getOffscreenRenderManager()->getOffscreenRenderer(layer.texturePath);
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
    if (layer.flags.testFlag(QDemonRenderLayer::Flag::LayerEnableDepthTest) && !opaqueObjects.empty()) {
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

    if (!layer.flags.testFlag(QDemonRenderLayer::Flag::LayerEnableDepthTest))
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
    if (!layer.flags.testFlag(QDemonRenderLayer::Flag::Active))
        return;

    // Ensure we clear the widget layer always
    renderer->layerNeedsFrameClear(*static_cast<QDemonLayerRenderData *>(this));

    if (iRenderWidgets.size() < MAX_LAYER_WIDGETS)
        iRenderWidgets.push_back(&inWidget);
}

#define RENDER_FRAME_NEW(type) new (renderer->demonContext()->getPerFrameAllocator().allocate(sizeof(type))) type

#define QDEMON_RENDER_MINIMUM_RENDER_OPACITY .01f

QDemonShaderDefaultMaterialKey QDemonLayerRenderPreparationData::generateLightingKey(QDemonRenderDefaultMaterial::MaterialLighting inLightingType)
{
    QDemonShaderDefaultMaterialKey theGeneratedKey(getShaderFeatureSetHash());
    const bool lighting = inLightingType != QDemonRenderDefaultMaterial::MaterialLighting::NoLighting;
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
            const bool isDirectional = theLight->m_lightType == QDemonRenderLight::Type::Directional;
            const bool isArea = theLight->m_lightType == QDemonRenderLight::Type::Area;
            const bool castShadowsArea = (theLight->m_lightType != QDemonRenderLight::Type::Area) && (theLight->m_castShadow);

            renderer->defaultMaterialShaderKeyProperties().m_lightFlags[lightIdx].setValue(theGeneratedKey, !isDirectional);
            renderer->defaultMaterialShaderKeyProperties().m_lightAreaFlags[lightIdx].setValue(theGeneratedKey, isArea);
            renderer->defaultMaterialShaderKeyProperties().m_lightShadowFlags[lightIdx].setValue(theGeneratedKey, castShadowsArea);
        }
    }
    return theGeneratedKey;
}

QPair<bool, QDemonRenderGraphObject *> QDemonLayerRenderPreparationData::resolveReferenceMaterial(QDemonRenderGraphObject *inMaterial)
{
    bool subsetDirty = false;
    bool badIdea = false;
    QDemonRenderGraphObject *theSourceMaterialObject(inMaterial);
    QDemonRenderGraphObject *theMaterialObject(inMaterial);
    while (theMaterialObject && theMaterialObject->type == QDemonRenderGraphObject::Type::ReferencedMaterial && !badIdea) {
        QDemonRenderReferencedMaterial *theRefMaterial = static_cast<QDemonRenderReferencedMaterial *>(theMaterialObject);
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
    return QPair<bool, QDemonRenderGraphObject *>(subsetDirty, theMaterialObject);
}

bool QDemonLayerRenderPreparationData::preparePathForRender(QDemonRenderPath &inPath,
                                                            const QMatrix4x4 &inViewProjection,
                                                            const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                                                            QDemonLayerRenderPreparationResultFlags &ioFlags)
{
    QDemonRenderableObjectFlags theSharedFlags;
    theSharedFlags.setPickable(true);
    float subsetOpacity = inPath.globalOpacity;
    bool retval = inPath.flags.testFlag(QDemonRenderPath::Flag::Dirty);
    inPath.flags.setFlag(QDemonRenderPath::Flag::Dirty, false);
    QMatrix4x4 theMVP;
    QMatrix3x3 theNormalMatrix;

    inPath.calculateMVPAndNormalMatrix(inViewProjection, theMVP, theNormalMatrix);
    QDemonBounds3 theBounds(this->renderer->demonContext()->getPathManager()->getBounds(inPath));

    if (inPath.globalOpacity >= QDEMON_RENDER_MINIMUM_RENDER_OPACITY && inClipFrustum.hasValue()) {
        // Check bounding box against the clipping planes
        QDemonBounds3 theGlobalBounds = theBounds;
        theGlobalBounds.transform(inPath.globalTransform);
        if (inClipFrustum->intersectsWith(theGlobalBounds) == false)
            subsetOpacity = 0.0f;
    }

    QDemonRenderGraphObject *theMaterials[2] = { inPath.m_material, inPath.m_secondMaterial };

    if (inPath.m_pathType == QDemonRenderPath::PathType::Geometry || inPath.m_paintStyle != QDemonRenderPath::PaintStyle::FilledAndStroked)
        theMaterials[1] = nullptr;

    // We need to fill material to be the first one rendered so the stroke goes on top.
    // In the timeline, however, this is reversed.

    if (theMaterials[1])
        std::swap(theMaterials[1], theMaterials[0]);

    for (quint32 idx = 0, end = 2; idx < end; ++idx) {
        if (theMaterials[idx] == nullptr)
            continue;

        QDemonRenderableObjectFlags theFlags = theSharedFlags;

        QPair<bool, QDemonRenderGraphObject *> theMaterialAndDirty(resolveReferenceMaterial(theMaterials[idx]));
        QDemonRenderGraphObject *theMaterial(theMaterialAndDirty.second);
        retval = retval || theMaterialAndDirty.first;

        if (theMaterial != nullptr && theMaterial->type == QDemonRenderGraphObject::Type::DefaultMaterial) {
            QDemonRenderDefaultMaterial *theDefaultMaterial = static_cast<QDemonRenderDefaultMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            bool clearMaterialFlags = theMaterial == inPath.m_material;
            QDemonDefaultMaterialPreparationResult prepResult(
                    prepareDefaultMaterialForRender(*theDefaultMaterial, theFlags, subsetOpacity, clearMaterialFlags));

            theFlags = prepResult.renderableFlags;
            if (inPath.m_pathType == QDemonRenderPath::PathType::Geometry) {
                if ((inPath.m_beginCapping != QDemonRenderPath::Capping::None && inPath.m_beginCapOpacity < 1.0f)
                    || (inPath.m_endCapping != QDemonRenderPath::Capping::None && inPath.m_endCapOpacity < 1.0f))
                    theFlags.setHasTransparency(true);
            } else {
                ioFlags.setRequiresStencilBuffer(true);
            }
            retval = retval || prepResult.dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_pathType == QDemonRenderPath::PathType::Painted) {
                if (inPath.m_paintStyle == QDemonRenderPath::PaintStyle::Filled || inPath.m_paintStyle == QDemonRenderPath::PaintStyle::FilledAndStroked)
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

            QDemonRef<QDemonRenderContextInterface> demonContext(renderer->demonContext());
            QDemonRef<QDemonPathManagerInterface> thePathManager = demonContext->getPathManager();
            retval = thePathManager->prepareForRender(inPath) || retval;
            retval |= (inPath.m_wireframeMode != demonContext->getWireframeMode());
            inPath.m_wireframeMode = demonContext->getWireframeMode();

            if (theFlags.hasTransparency())
                transparentObjects.push_back(theRenderable);
            else
                opaqueObjects.push_back(theRenderable);
        } else if (theMaterial != nullptr && theMaterial->type == QDemonRenderGraphObject::Type::CustomMaterial) {
            QDemonRenderCustomMaterial *theCustomMaterial = static_cast<QDemonRenderCustomMaterial *>(theMaterial);
            // Don't clear dirty flags if the material was referenced.
            // bool clearMaterialFlags = theMaterial == inPath.m_Material;
            QDemonDefaultMaterialPreparationResult prepResult(prepareCustomMaterialForRender(*theCustomMaterial, theFlags, subsetOpacity));

            theFlags = prepResult.renderableFlags;
            if (inPath.m_pathType == QDemonRenderPath::PathType::Geometry) {
                if ((inPath.m_beginCapping != QDemonRenderPath::Capping::None && inPath.m_beginCapOpacity < 1.0f)
                    || (inPath.m_endCapping != QDemonRenderPath::Capping::None && inPath.m_endCapOpacity < 1.0f))
                    theFlags.setHasTransparency(true);
            } else {
                ioFlags.setRequiresStencilBuffer(true);
            }

            retval = retval || prepResult.dirty;
            bool isStroke = true;
            if (idx == 0 && inPath.m_pathType == QDemonRenderPath::PathType::Painted) {
                if (inPath.m_paintStyle == QDemonRenderPath::PaintStyle::Filled || inPath.m_paintStyle == QDemonRenderPath::PaintStyle::FilledAndStroked)
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

            QDemonRef<QDemonRenderContextInterface> demonContext(renderer->demonContext());
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
                                                             QDemonImageMapTypes inMapType,
                                                             QDemonRenderableImage *&ioFirstImage,
                                                             QDemonRenderableImage *&ioNextImage,
                                                             QDemonRenderableObjectFlags &ioFlags,
                                                             QDemonShaderDefaultMaterialKey &inShaderKey,
                                                             quint32 inImageIndex)
{
    QDemonRef<QDemonRenderContextInterface> demonContext(renderer->demonContext());
    QDemonBufferManager bufferManager = demonContext->getBufferManager();
    QDemonRef<QDemonOffscreenRenderManagerInterface> theOffscreenRenderManager(demonContext->getOffscreenRenderManager());
    //    IRenderPluginManager &theRenderPluginManager(demonContext.GetRenderPluginManager());
    if (inImage.clearDirty(bufferManager, *theOffscreenRenderManager /*, theRenderPluginManager*/))
        ioFlags |= QDemonRenderableObjectFlag::Dirty;

    // All objects with offscreen renderers are pickable so we can pass the pick through to the
    // offscreen renderer and let it deal with the pick.
    if (inImage.m_lastFrameOffscreenRenderer != nullptr) {
        ioFlags.setPickable(true);
        ioFlags |= QDemonRenderableObjectFlag::HasTransparency;
    }

    if (inImage.m_textureData.m_texture) {
        if (inImage.m_textureData.m_textureFlags.hasTransparency()
            && (inMapType == QDemonImageMapTypes::Diffuse || inMapType == QDemonImageMapTypes::Opacity
                || inMapType == QDemonImageMapTypes::Translucency)) {
            ioFlags |= QDemonRenderableObjectFlag::HasTransparency;
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
        case QDemonRenderImage::MappingModes::Normal:
            break;
        case QDemonRenderImage::MappingModes::Environment:
            theKeyProp.setEnvMap(inShaderKey, true);
            break;
        case QDemonRenderImage::MappingModes::LightProbe:
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
        theSwizzleKeyProp.setSwizzleMode(inShaderKey, inImage.m_textureData.m_texture->textureSwizzleMode(), true);

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
        renderableFlags |= QDemonRenderableObjectFlag::Dirty;
    }
    subsetOpacity *= theMaterial->opacity;
    if (inClearDirtyFlags)
        theMaterial->dirty.updateDirtyForFrame();

    QDemonRenderableImage *firstImage = nullptr;

    // set wireframe mode
    renderer->defaultMaterialShaderKeyProperties().m_wireframeMode.setValue(theGeneratedKey,
                                                                            renderer->demonContext()->getWireframeMode());

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

        if (theMaterial->blendMode != QDemonRenderDefaultMaterial::MaterialBlendMode::Normal || theMaterial->opacityMap) {
            renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;
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
        renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;
        renderableFlags |= QDemonRenderableObjectFlag::CompletelyTransparent;
    }

    if (isNotOne(subsetOpacity))
        renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;

    retval.firstImage = firstImage;
    if (retval.renderableFlags.isDirty())
        retval.dirty = true;
    return retval;
}

QDemonDefaultMaterialPreparationResult QDemonLayerRenderPreparationData::prepareCustomMaterialForRender(QDemonRenderCustomMaterial &inMaterial,
                                                                                                        QDemonRenderableObjectFlags &inExistingFlags,
                                                                                                        float inOpacity)
{
    QDemonDefaultMaterialPreparationResult retval(generateLightingKey(QDemonRenderDefaultMaterial::MaterialLighting::FragmentLighting)); // always fragment lighting
    retval.renderableFlags = inExistingFlags;
    QDemonRenderableObjectFlags &renderableFlags(retval.renderableFlags);
    QDemonShaderDefaultMaterialKey &theGeneratedKey(retval.materialKey);
    retval.opacity = inOpacity;
    float &subsetOpacity(retval.opacity);

    // set wireframe mode
    renderer->defaultMaterialShaderKeyProperties().m_wireframeMode.setValue(theGeneratedKey,
                                                                            renderer->demonContext()->getWireframeMode());

    if (subsetOpacity < QDEMON_RENDER_MINIMUM_RENDER_OPACITY) {
        subsetOpacity = 0.0f;
        // You can still pick against completely transparent objects(or rather their bounding
        // box)
        // you just don't render them.
        renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;
        renderableFlags |= QDemonRenderableObjectFlag::CompletelyTransparent;
    }

    if (isNotOne(subsetOpacity))
        renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;

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
    QDemonRef<QDemonRenderContextInterface> demonContext(renderer->demonContext());
    QDemonBufferManager bufferManager = demonContext->getBufferManager();
    QDemonRenderMesh *theMesh = bufferManager.loadMesh(inModel.meshPath);
    if (theMesh == nullptr)
        return false;

    QDemonRenderGraphObject *theSourceMaterialObject = inModel.firstMaterial;
    QDemonModelContext &theModelContext = *RENDER_FRAME_NEW(QDemonModelContext)(inModel, inViewProjection);
    modelContexts.push_back(&theModelContext);

    bool subsetDirty = false;

    QDemonScopedLightsListScope lightsScope(lights, lightDirections, sourceLightDirections, inScopedLights);
    setShaderFeature(cgLightingFeatureName, lights.empty() == false);
    for (quint32 idx = 0, end = theMesh->subsets.size(); idx < end && theSourceMaterialObject;
         ++idx, theSourceMaterialObject = theSourceMaterialObject->nextMaterialSibling()) {
        QDemonRenderSubset &theOuterSubset(theMesh->subsets[idx]);
        {
            QDemonRenderSubset &theSubset(theOuterSubset);
            QDemonRenderableObjectFlags renderableFlags;
            renderableFlags.setPickable(false);
            float subsetOpacity = inModel.globalOpacity;
            QVector3D theModelCenter(theSubset.bounds.center());
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
                                        && (theModelContext.model.flags.testFlag(QDemonRenderModel::Flag::GloballyPickable)
                                            || renderableFlags.isPickable()));
            QDemonRenderableObject *theRenderableObject = nullptr;
            QPair<bool, QDemonRenderGraphObject *> theMaterialObjectAndDirty = resolveReferenceMaterial(theSourceMaterialObject);
            QDemonRenderGraphObject *theMaterialObject = theMaterialObjectAndDirty.second;
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
                theSubset.primitiveType = theSubset.inputAssembler->drawMode();
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

            if (theMaterialObject->type == QDemonRenderGraphObject::Type::DefaultMaterial) {
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
            } else if (theMaterialObject->type == QDemonRenderGraphObject::Type::CustomMaterial) {
                QDemonRenderCustomMaterial &theMaterial(static_cast<QDemonRenderCustomMaterial &>(*theMaterialObject));

                QDemonRef<QDemonMaterialSystem> theMaterialSystem(demonContext->getCustomMaterialSystem());
                subsetDirty |= theMaterialSystem->prepareForRender(theModelContext.model, theSubset, theMaterial, clearMaterialDirtyFlags);

                QDemonDefaultMaterialPreparationResult theMaterialPrepResult(
                        prepareCustomMaterialForRender(theMaterial, renderableFlags, subsetOpacity));
                QDemonShaderDefaultMaterialKey theGeneratedKey = theMaterialPrepResult.materialKey;
                subsetOpacity = theMaterialPrepResult.opacity;
                QDemonRenderableImage *firstImage(theMaterialPrepResult.firstImage);
                renderableFlags = theMaterialPrepResult.renderableFlags;

                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasTransparency)
                    renderableFlags |= QDemonRenderableObjectFlag::HasTransparency;
                // prepare for render tells us if the object is transparent
                if (theMaterial.m_hasRefraction)
                    renderableFlags |= QDemonRenderableObjectFlag::HasRefraction;

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
                                                                   QDemonLayerRenderPreparationResultFlags &ioFlags)
{
    QDemonStackPerfTimer perfTimer(renderer->demonContext()->getPerfTimer(), Q_FUNC_INFO);
    viewProjection = inViewProjection;
    bool wasDataDirty = false;
    for (quint32 idx = 0, end = renderableNodes.size(); idx < end; ++idx) {
        QDemonRenderableNodeEntry &theNodeEntry(renderableNodes[idx]);
        QDemonRenderNode *theNode = theNodeEntry.node;
        wasDataDirty = wasDataDirty || theNode->flags.testFlag(QDemonRenderNode::Flag::Dirty);
        switch (theNode->type) {
        case QDemonRenderGraphObject::Type::Model: {
            QDemonRenderModel *theModel = static_cast<QDemonRenderModel *>(theNode);
            theModel->calculateGlobalVariables();
            if (theModel->flags.testFlag(QDemonRenderModel::Flag::GloballyActive)) {
                bool wasModelDirty = prepareModelForRender(*theModel, inViewProjection, inClipFrustum, theNodeEntry.lights);
                wasDataDirty = wasDataDirty || wasModelDirty;
            }
        } break;
        case QDemonRenderGraphObject::Type::Path: {
            QDemonRenderPath *thePath = static_cast<QDemonRenderPath *>(theNode);
            thePath->calculateGlobalVariables();
            if (thePath->flags.testFlag(QDemonRenderPath::Flag::GloballyActive)) {
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
    QDemonRef<QDemonRenderContextInterface> theContext(renderer->demonContext());
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
    QDemonLightNodeMarker(QDemonRenderLight &inLight, quint32 inLightIndex, QDemonRenderNode &inNode, bool aorm)
        : light(&inLight), lightIndex(inLightIndex), addOrRemove(aorm)
    {
        if (inNode.type == QDemonRenderGraphObject::Type::Layer) {
            firstValidIndex = 0;
            justPastLastValidIndex = std::numeric_limits<quint32>::max();
        } else {
            firstValidIndex = inNode.dfsIndex;
            QDemonRenderNode *lastChild = nullptr;
            QDemonRenderNode *firstChild = inNode.firstChild;
            // find deepest last child
            while (firstChild) {
                for (QDemonRenderNode *childNode = firstChild; childNode; childNode = childNode->nextSibling)
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
    QDemonStackPerfTimer perfTimer(renderer->demonContext()->getPerfTimer(), Q_FUNC_INFO);
    if (layerPrepResult.hasValue())
        return;

    features.clear();
    featureSetHash = 0;
    QVector2D thePresentationDimensions((float)inViewportDimensions.width(), (float)inViewportDimensions.height());
    QDemonRef<QDemonRenderListInterface> theGraph(renderer->demonContext()->getRenderList());
    QRect theViewport(theGraph->getViewport());
    QRect theScissor(theGraph->getViewport());
    if (theGraph->isScissorTestEnabled())
        theScissor = renderer->context()->scissorRect();
    bool wasDirty = false;
    bool wasDataDirty = false;
    wasDirty = layer.flags.testFlag(QDemonRenderLayer::Flag::Dirty);
    // The first pass is just to render the data.
    quint32 maxNumAAPasses = layer.progressiveAAMode == QDemonRenderLayer::AAMode::NoAA ? (quint32)0 : (quint32)(layer.progressiveAAMode) + 1;
    maxNumAAPasses = qMin((quint32)(MAX_AA_LEVELS + 1), maxNumAAPasses);
    QDemonRenderEffect *theLastEffect = nullptr;
    // Uncomment the line below to disable all progressive AA.
    // maxNumAAPasses = 0;

    QDemonLayerRenderPreparationResult thePrepResult;
    bool hasOffscreenRenderer = usesOffscreenRenderer();

    bool SSAOEnabled = (layer.aoStrength > 0.0f && layer.aoDistance > 0.0f);
    bool SSDOEnabled = (layer.shadowStrength > 0.0f && layer.shadowDist > 0.0f);
    setShaderFeature("QDEMON_ENABLE_SSAO", SSAOEnabled);
    setShaderFeature("QDEMON_ENABLE_SSDO", SSDOEnabled);
    bool requiresDepthPrepass = (hasOffscreenRenderer == false) && (SSAOEnabled || SSDOEnabled);
    setShaderFeature("QDEMON_ENABLE_SSM", false); // by default no shadow map generation

    if (layer.flags.testFlag(QDemonRenderLayer::Flag::Active)) {
        // Get the layer's width and height.
        QDemonRef<QDemonEffectSystemInterface> theEffectSystem(renderer->demonContext()->getEffectSystem());
        for (QDemonRenderEffect *theEffect = layer.firstEffect; theEffect; theEffect = theEffect->m_nextEffect) {
            if (theEffect->flags.testFlag(QDemonRenderEffect::Flag::Dirty)) {
                wasDirty = true;
                theEffect->flags.setFlag(QDemonRenderEffect::Flag::Dirty, false);
            }
            if (theEffect->flags.testFlag(QDemonRenderEffect::Flag::Active)) {
                theLastEffect = theEffect;
                if (hasOffscreenRenderer == false && theEffectSystem->doesEffectRequireDepthTexture(theEffect->className))
                    requiresDepthPrepass = true;
            }
        }
        if (layer.flags.testFlag(QDemonRenderLayer::Flag::Dirty)) {
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
                                        renderer->demonContext()->getScaleMode(),
                                        renderer->demonContext()->getPresentationScaleFactor()));
        thePrepResult.lastEffect = theLastEffect;
        thePrepResult.maxAAPassIndex = maxNumAAPasses;
        thePrepResult.flags.setRequiresDepthTexture(requiresDepthPrepass || needsWidgetTexture());
        thePrepResult.flags.setShouldRenderToTexture(shouldRenderToTexture);
        if (renderer->context()->renderContextType() != QDemonRenderContextType::GLES2)
            thePrepResult.flags.setRequiresSsaoPass(SSAOEnabled);

        if (thePrepResult.isLayerVisible()) {
            if (shouldRenderToTexture) {
                renderer->demonContext()->getRenderList()->addRenderTask(createRenderToTextureRunnable());
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
//            if (renderableNodes.empty()) {
//                camerasAndLights.clear();
//                quint32 dfsIndex = 0;
//                for (QDemonRenderNode *theChild = layer.firstChild; theChild; theChild = theChild->nextSibling)
//                    MaybeQueueNodeForRender(*theChild, renderableNodes, camerasAndLights, dfsIndex);
//                std::reverse(camerasAndLights.begin(), camerasAndLights.end());
//                std::reverse(renderableNodes.begin(), renderableNodes.end());
//                lightToNodeMap.clear();
//            }
            // ### TODO: Really this should only be done if renderableNodes is empty or dirty
            // but we don't have a way to say it's dirty yet (new renderables added to the tree)
            camerasAndLights.clear();
            renderableNodes.clear();
            quint32 dfsIndex = 0;
            for (QDemonRenderNode *theChild = layer.firstChild; theChild; theChild = theChild->nextSibling)
                MaybeQueueNodeForRender(*theChild, renderableNodes, camerasAndLights, dfsIndex);
            std::reverse(camerasAndLights.begin(), camerasAndLights.end());
            std::reverse(renderableNodes.begin(), renderableNodes.end());
            lightToNodeMap.clear();

            camera = nullptr;
            lights.clear();
            opaqueObjects.clear();
            qDeleteAll(opaqueObjects);
            transparentObjects.clear();
            qDeleteAll(transparentObjects);
            QVector<QDemonLightNodeMarker> theLightNodeMarkers;
            sourceLightDirections.clear();

            for (quint32 idx = 0, end = camerasAndLights.size(); idx < end; ++idx) {
                QDemonRenderNode *theNode(camerasAndLights[idx]);
                wasDataDirty = wasDataDirty || theNode->flags.testFlag(QDemonRenderNode::Flag::Dirty);
                switch (theNode->type) {
                case QDemonRenderGraphObject::Type::Camera: {
                    QDemonRenderCamera *theCamera = static_cast<QDemonRenderCamera *>(theNode);
                    QDemonCameraGlobalCalculationResult theResult = thePrepResult.setupCameraForRender(*theCamera);
                    wasDataDirty = wasDataDirty || theResult.m_wasDirty;
                    if (theCamera->flags.testFlag(QDemonRenderCamera::Flag::GloballyActive))
                        camera = theCamera;
                    if (theResult.m_computeFrustumSucceeded == false) {
                        qCCritical(INTERNAL_ERROR, "Failed to calculate camera frustum");
                    }
                } break;
                case QDemonRenderGraphObject::Type::Light: {
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
                    if (theLight->flags.testFlag(QDemonRenderLight::Flag::GloballyActive)) {
                        if (theLight->m_scope == nullptr) {
                            lights.push_back(theLight);
                            if (renderer->context()->renderContextType() != QDemonRenderContextType::GLES2
                                && theLight->m_castShadow) {
                                createShadowMapManager();
                                // PKC -- use of "res" as an exponent of two is an annoying
                                // artifact of the XML interface
                                // I'll change this with an enum interface later on, but that's
                                // less important right now.
                                quint32 mapSize = 1 << theLight->m_shadowMapRes;
                                ShadowMapModes mapMode = (theLight->m_lightType != QDemonRenderLight::Type::Directional)
                                        ? ShadowMapModes::CUBE
                                        : ShadowMapModes::VSM;
                                shadowMapManager->addShadowMapEntry(lights.size() - 1,
                                                                    mapSize,
                                                                    mapSize,
                                                                    QDemonRenderTextureFormat::R16F,
                                                                    1,
                                                                    mapMode,
                                                                    ShadowFilterValues::NONE);
                                thePrepResult.flags.setRequiresShadowMapPass(true);
                                setShaderFeature("QDEMON_ENABLE_SSM", true);
                            }
                        }
                        TLightToNodeMap::iterator iter = lightToNodeMap.insert(theLight, (QDemonRenderNode *)nullptr);
                        QDemonRenderNode *oldLightScope = iter.value();
                        QDemonRenderNode *newLightScope = theLight->m_scope;

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

            if (camera) {
                camera->calculateViewProjectionMatrix(viewProjection);
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
            if (usesOffscreenRenderer() == false) {
                bool renderablesDirty = prepareRenderablesForRender(viewProjection,
                                                                    clippingFrustum,
                                                                    thePrepResult.flags);
                wasDataDirty = wasDataDirty || renderablesDirty;
                if (thePrepResult.flags.requiresStencilBuffer())
                    thePrepResult.flags.setShouldRenderToTexture(true);
            } else {
                QRect theViewport = thePrepResult.viewport().toRect();
                bool theScissor = true;
                QRect theScissorRect = thePrepResult.scissor().toRect();
                // This happens here because if there are any fancy render steps
                QDemonRef<QDemonRenderListInterface> theRenderList(renderer->demonContext()->getRenderList());
                auto theContext = renderer->context();
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
                                                                       &QDemonRenderContext::scissorRect,
                                                                       &QDemonRenderContext::setScissorRect,
                                                                       theScissorRect);
                QDemonRenderContextScopedProperty<QRect> __viewportRect(*theContext,
                                                                        &QDemonRenderContext::viewport,
                                                                        &QDemonRenderContext::setViewport,
                                                                        theViewport);
                QDemonOffscreenRenderFlags theResult = lastFrameOffscreenRenderer
                                                               ->needsRender(createOffscreenRenderEnvironment(),
                                                                             renderer->demonContext()->getPresentationScaleFactor(),
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
