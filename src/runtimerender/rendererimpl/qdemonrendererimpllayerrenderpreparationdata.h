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
#ifndef QDEMON_RENDERER_IMPL_LAYER_RENDER_PREPARATION_DATA_H
#define QDEMON_RENDERER_IMPL_LAYER_RENDER_PREPARATION_DATA_H

#include <QtDemon/qdemonflags.h>
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderhelper.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderableobjects.h>
#include <QtDemonRuntimeRender/qdemonrenderclippingfrustum.h>
#include <QtDemonRuntimeRender/qdemonrenderresourcetexture2d.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrenderprofiler.h>
#include <QtDemonRuntimeRender/qdemonrendershadowmap.h>
#include <QtDemonRuntimeRender/qdemonrenderableobjects.h>

QT_BEGIN_NAMESPACE
struct QDemonLayerRenderData;
class QDemonRendererImpl;
struct QDemonRenderableObject;

struct LayerRenderPreparationResultFlagValues
{
    enum Enum {
        // Was the data in this layer dirty (meaning re-render to texture, possibly)
        WasLayerDataDirty = 1,
        // Was the data in this layer dirty *or* this layer *or* any effect dirty.
        WasDirty = 1 << 1,
        // An effect or flag or rotation on the layer dictates this object should
        // render to the texture.
        ShouldRenderToTexture = 1 << 2,
        // Some effects require depth texturing, this should be set on the effect
        // instance.
        RequiresDepthTexture = 1 << 3,

        // Should create independent viewport
        // If we aren't rendering to texture we still may have width/height manipulations
        // that require our own viewport.
        ShouldCreateIndependentViewport = 1 << 4,

        // SSAO should be done in a separate pass
        // Note that having an AO pass necessitates a DepthTexture so this flag should
        // never be set without the RequiresDepthTexture flag as well.
        RequiresSsaoPass = 1 << 5,

        // if some light cause shadow
        // we need a separate per light shadow map pass
        RequiresShadowMapPass = 1 << 6,

        // Currently we use a stencil-cover algorithm to render bezier curves.
        RequiresStencilBuffer = 1 << 7
    };
};

struct QDemonLayerRenderPreparationResultFlags : public QDemonFlags<LayerRenderPreparationResultFlagValues::Enum, quint32>
{
    bool wasLayerDataDirty() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::WasLayerDataDirty);
    }
    void setLayerDataDirty(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::WasLayerDataDirty);
    }

    bool wasDirty() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::WasDirty);
    }
    void setWasDirty(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::WasDirty);
    }

    bool shouldRenderToTexture() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::ShouldRenderToTexture);
    }
    void setShouldRenderToTexture(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::ShouldRenderToTexture);
    }

    bool requiresDepthTexture() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::RequiresDepthTexture);
    }
    void setRequiresDepthTexture(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::RequiresDepthTexture);
    }

    bool shouldCreateIndependentViewport() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::ShouldCreateIndependentViewport);
    }
    void setShouldCreateIndependentViewport(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::ShouldCreateIndependentViewport);
    }

    bool requiresSsaoPass() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::RequiresSsaoPass);
    }
    void setRequiresSsaoPass(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::RequiresSsaoPass);
    }

    bool requiresShadowMapPass() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::RequiresShadowMapPass);
    }
    void setRequiresShadowMapPass(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::RequiresShadowMapPass);
    }

    bool requiresStencilBuffer() const
    {
        return this->operator&(LayerRenderPreparationResultFlagValues::RequiresStencilBuffer);
    }
    void setRequiresStencilBuffer(bool inValue)
    {
        clearOrSet(inValue, LayerRenderPreparationResultFlagValues::RequiresStencilBuffer);
    }
};

struct QDemonLayerRenderPreparationResult : public QDemonLayerRenderHelper
{
    QDemonRenderEffect *lastEffect = nullptr;
    QDemonLayerRenderPreparationResultFlags flags;
    quint32 maxAAPassIndex = 0;
    QDemonLayerRenderPreparationResult() = default;
    QDemonLayerRenderPreparationResult(const QDemonLayerRenderHelper &inHelper)
        : QDemonLayerRenderHelper(inHelper)
        , lastEffect(nullptr)
        , maxAAPassIndex(0)
    {
    }
};

struct QDemonRenderableNodeEntry
{
    QDemonGraphNode *node = nullptr;
    QDemonNodeLightEntryList lights;
    QDemonRenderableNodeEntry() = default;
    QDemonRenderableNodeEntry(QDemonGraphNode &inNode)
        : node(&inNode)
    {
    }
};

struct QDemonScopedLightsListScope
{
    QVector<QDemonRenderLight *> &lightsList;
    QVector<QVector3D> &lightDirList;
    quint32 listOriginalSize;
    QDemonScopedLightsListScope(QVector<QDemonRenderLight *> &inLights,
                                QVector<QVector3D> &inDestLightDirList,
                                QVector<QVector3D> &inSrcLightDirList,
                                QDemonNodeLightEntryList &inScopedLights)
        : lightsList(inLights)
        , lightDirList(inDestLightDirList)
        , listOriginalSize(lightsList.size())
    {
        auto iter = inScopedLights.begin();
        const auto end = inScopedLights.end();
        while (iter != end) {
            lightsList.push_back(iter->light);
            lightDirList.push_back(inSrcLightDirList[iter->lightIndex]);
            ++iter;
        }
    }
    ~QDemonScopedLightsListScope()
    {
        lightsList.resize(listOriginalSize);
        lightDirList.resize(listOriginalSize);
    }
};

struct QDemonDefaultMaterialPreparationResult
{
    QDemonRenderableImage *firstImage;
    float opacity;
    QDemonRenderableObjectFlags renderableFlags;
    QDemonShaderDefaultMaterialKey materialKey;
    bool dirty;

    QDemonDefaultMaterialPreparationResult(QDemonShaderDefaultMaterialKey inMaterialKey);
};

// Data used strictly in the render preparation step.
struct QDemonLayerRenderPreparationData
{
    typedef void (*TRenderRenderableFunction)(QDemonLayerRenderData &inData,
                                              QDemonRenderableObject &inObject,
                                              const QVector2D &inCameraProps,
                                              TShaderFeatureSet inShaderFeatures,
                                              quint32 lightIndex,
                                              const QDemonRenderCamera &inCamera);
    typedef QHash<QDemonRenderLight *, QDemonGraphNode *> TLightToNodeMap;
    //typedef Pool<SNodeLightEntry, ForwardingAllocator> TNodeLightEntryPoolType;

    enum Enum {
        MAX_AA_LEVELS = 8,
        MAX_TEMPORAL_AA_LEVELS = 2,
    };

    QDemonRenderLayer &layer;
    QDemonRef<QDemonRendererImpl> renderer;
    // List of nodes we can render, not all may be active.  Found by doing a depth-first
    // search through m_FirstChild if length is zero.

    //TNodeLightEntryPoolType m_RenderableNodeLightEntryPool;
    QVector<QDemonRenderableNodeEntry> renderableNodes;
    TLightToNodeMap lightToNodeMap; // map of lights to nodes to cache if we have looked up a
    // given scoped light yet.
    // Built at the same time as the renderable nodes map.
    // these are processed so they are available when the shaders for the models
    // are being generated.
    QVector<QDemonGraphNode *> camerasAndLights;

    // Results of prepare for render.
    QDemonRenderCamera *camera;
    QVector<QDemonRenderLight *> lights; // Only contains lights that are global.
    TRenderableObjectList opaqueObjects;
    TRenderableObjectList transparentObjects;
    // Sorted lists of the rendered objects.  There may be other transforms applied so
    // it is simplest to duplicate the lists.
    TRenderableObjectList renderedOpaqueObjects;
    TRenderableObjectList renderedTransparentObjects;
    QMatrix4x4 viewProjection;
    QDemonClippingFrustum clippingFrustum;
    QDemonOption<QDemonLayerRenderPreparationResult> layerPrepResult;
    // Widgets drawn at particular times during the rendering process
    QVector<QDemonRenderWidgetInterface *> iRenderWidgets;
    QDemonOption<QVector3D> cameraDirection;
    // Scoped lights need a level of indirection into a light direction list.  The source light
    // directions list is as long as there are lights on the layer.  It holds invalid
    // information for
    // any lights that are not both active and scoped; but the relative position for a given
    // light
    // in this list is completely constant and immutable; this relative position is saved on a
    // structure
    // and used when looking up the light direction for a given light.
    QVector<QVector3D> sourceLightDirections;
    QVector<QVector3D> lightDirections;
    TModelContextPtrList modelContexts;
    QDemonRef<QDemonOffscreenRendererInterface> lastFrameOffscreenRenderer;

    QVector<QDemonShaderPreprocessorFeature> features;
    QString cgLightingFeatureName;
    bool featuresDirty;
    size_t featureSetHash;
    bool tooManyLightsError;

    // shadow mapps
    QDemonRef<QDemonRenderShadowMap> shadowMapManager;

    QDemonLayerRenderPreparationData(QDemonRenderLayer &inLayer, QDemonRef<QDemonRendererImpl> inRenderer);
    virtual ~QDemonLayerRenderPreparationData();
    bool getOffscreenRenderer();
    bool getShadowMapManager();
    bool needsWidgetTexture() const;

    QDemonShaderDefaultMaterialKey generateLightingKey(DefaultMaterialLighting::Enum inLightingType);

    void prepareImageForRender(QDemonRenderImage &inImage,
                               QDemonImageMapTypes::Enum inMapType,
                               QDemonRenderableImage *&ioFirstImage,
                               QDemonRenderableImage *&ioNextImage,
                               QDemonRenderableObjectFlags &ioFlags,
                               QDemonShaderDefaultMaterialKey &ioGeneratedShaderKey,
                               quint32 inImageIndex);

    QDemonDefaultMaterialPreparationResult prepareDefaultMaterialForRender(QDemonRenderDefaultMaterial &inMaterial,
                                                                           QDemonRenderableObjectFlags &inExistingFlags,
                                                                           float inOpacity,
                                                                           bool inClearMaterialFlags);

    QDemonDefaultMaterialPreparationResult prepareCustomMaterialForRender(QDemonRenderCustomMaterial &inMaterial,
                                                                          QDemonRenderableObjectFlags &inExistingFlags,
                                                                          float inOpacity);

    bool prepareModelForRender(QDemonRenderModel &inModel,
                               const QMatrix4x4 &inViewProjection,
                               const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                               QDemonNodeLightEntryList &inScopedLights);

    bool prepareTextForRender(QDemonText &inText,
                              const QMatrix4x4 &inViewProjection,
                              float inTextScaleFactor,
                              QDemonLayerRenderPreparationResultFlags &ioFlags);
    bool preparePathForRender(QDemonPath &inPath,
                              const QMatrix4x4 &inViewProjection,
                              const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                              QDemonLayerRenderPreparationResultFlags &ioFlags);
    // Helper function used during PRepareForRender and PrepareAndRender
    bool prepareRenderablesForRender(const QMatrix4x4 &inViewProjection,
                                     const QDemonOption<QDemonClippingFrustum> &inClipFrustum,
                                     float inTextScaleFactor,
                                     QDemonLayerRenderPreparationResultFlags &ioFlags);

    // returns true if this object will render something different than it rendered the last
    // time.
    virtual void prepareForRender(const QSize &inViewportDimensions);
    bool checkLightProbeDirty(QDemonRenderImage &inLightProbe);
    void addRenderWidget(QDemonRenderWidgetInterface &inWidget);
    void setShaderFeature(const char *inName, bool inValue);
    void setShaderFeature(QString inName, bool inValue);
    QVector<QDemonShaderPreprocessorFeature> getShaderFeatureSet();
    size_t getShaderFeatureSetHash();
    // The graph object is not const because this traversal updates dirty state on the objects.
    QPair<bool, QDemonGraphObject *> resolveReferenceMaterial(QDemonGraphObject *inMaterial);

    QVector3D getCameraDirection();
    // Per-frame cache of renderable objects post-sort.
    QVector<QDemonRenderableObject *> getOpaqueRenderableObjects();
    // If layer depth test is false, this may also contain opaque objects.
    QVector<QDemonRenderableObject *> getTransparentRenderableObjects();

    virtual void resetForFrame();

    // The render list and gl context are setup for what the embedded item will
    // need.
    virtual QDemonOffscreenRendererEnvironment createOffscreenRenderEnvironment() = 0;

    virtual QDemonRef<QDemonRenderTask> createRenderToTextureRunnable() = 0;
};
QT_END_NAMESPACE
#endif
