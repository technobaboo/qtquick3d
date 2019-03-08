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
#ifndef QDEMON_RENDER_SHADER_GENERATOR_IMPL_H
#define QDEMON_RENDER_SHADER_GENERATOR_IMPL_H

#include <QtDemonRuntimeRender/qdemonrenderer.h>
#include <QtDemonRuntimeRender/qdemonrenderableobjects.h>
#include <QtDemonRuntimeRender/qdemonrendererimplshaders.h>
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderdata.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrenderlayer.h>
#include <QtDemonRuntimeRender/qdemonrenderray.h>
#include <QtDemonRuntimeRender/qdemonrendertext.h>
#include <QtDemonRuntimeRender/qdemonrendercamera.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrendercontextcore.h>
#include <QtDemonRuntimeRender/qdemonoffscreenrendermanager.h>
#include <QtDemonRuntimeRender/qdemonrendererimpllayerrenderhelper.h>
#include <QtDemonRuntimeRender/qdemonrenderwidgets.h>
#include <QtDemonRuntimeRender/qdemonrendershadercodegenerator.h>
#include <QtDemonRuntimeRender/qdemonrenderclippingfrustum.h>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderprofiler.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterialshadergenerator.h>
#include <QtDemonRuntimeRender/qdemonrenderbuffermanager.h>
#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemon/QDemonBounds3>
#include <QtDemon/QDemonOption>
#include <QtDemon/QDemonDataRef>

QT_BEGIN_NAMESPACE
inline bool floatLessThan(float lhs, float rhs)
{
    float diff = lhs - rhs;
    if (std::abs(diff) < .001f)
        return false;
    return diff < 0.0f ? true : false;
}
inline bool iSRenderObjectPtrLessThan(const QDemonRenderableObject *lhs, const QDemonRenderableObject *rhs)
{
    return floatLessThan(lhs->cameraDistanceSq, rhs->cameraDistanceSq);
}
inline bool iSRenderObjectPtrGreatThan(const QDemonRenderableObject *lhs, const QDemonRenderableObject *rhs)
{
    return floatLessThan(rhs->cameraDistanceSq, lhs->cameraDistanceSq);
}
inline bool nonZero(float inValue)
{
    return std::abs(inValue) > .001f;
}
inline bool nonZero(quint32 inValue)
{
    return inValue != 0;
}
inline bool isZero(float inValue)
{
    return std::abs(inValue) < .001f;
}
inline bool isNotOne(float inValue)
{
    return std::abs(1.0f - inValue) > .001f;
}

inline bool isRectEdgeInBounds(qint32 inNewRectOffset, qint32 inNewRectWidth, qint32 inCurrentRectOffset, qint32 inCurrentRectWidth)
{
    qint32 newEnd = inNewRectOffset + inNewRectWidth;
    qint32 currentEnd = inCurrentRectOffset + inCurrentRectWidth;
    return inNewRectOffset >= inCurrentRectOffset && newEnd <= currentEnd;
}

struct QDemonTextRenderHelper
{
    QDemonTextShader *shader;
    QDemonRef<QDemonRenderInputAssembler> quadInputAssembler;
    QDemonTextRenderHelper(QDemonTextShader *inShader, QDemonRef<QDemonRenderInputAssembler> inQuadInputAssembler)
        : shader(inShader), quadInputAssembler(inQuadInputAssembler)
    {
    }
};

struct QDemonPickResultProcessResult : public QDemonRenderPickResult
{
    QDemonPickResultProcessResult(const QDemonRenderPickResult &inSrc) : QDemonRenderPickResult(inSrc) {}
    QDemonPickResultProcessResult() = default;
    bool m_wasPickConsumed = false;
};

class Q_DEMONRUNTIMERENDER_EXPORT QDemonRendererImpl : public QDemonRendererInterface
{
    typedef QHash<QDemonShaderDefaultMaterialKey, QDemonRef<QDemonShaderGeneratorGeneratedShader>> TShaderMap;
    typedef QHash<QByteArray, QDemonRef<QDemonRenderConstantBuffer>> TStrConstanBufMap;
    // typedef QHash<SRenderInstanceId, QDemonRef<SLayerRenderData>, eastl::hash<SRenderInstanceId>> TInstanceRenderMap;
    typedef QHash<QDemonRenderInstanceId, QDemonRef<QDemonLayerRenderData>> TInstanceRenderMap;
    typedef QVector<QDemonLayerRenderData *> TLayerRenderList;
    typedef QVector<QDemonRenderPickResult> TPickResultArray;

    // Items to implement the widget context.
    typedef QHash<QByteArray, QDemonRef<QDemonRenderVertexBuffer>> TStrVertBufMap;
    typedef QHash<QByteArray, QDemonRef<QDemonRenderIndexBuffer>> TStrIndexBufMap;
    typedef QHash<QByteArray, QDemonRef<QDemonRenderShaderProgram>> TStrShaderMap;
    typedef QHash<QByteArray, QDemonRef<QDemonRenderInputAssembler>> TStrIAMap;

    typedef QHash<long, QDemonGraphNode *> TBoneIdNodeMap;

    QDemonRenderContextInterface *m_demonContext;
    QDemonRef<QDemonRenderContext> m_context;
    QDemonBufferManager m_bufferManager;
    QDemonRef<QDemonOffscreenRenderManagerInterface> m_offscreenRenderManager;
    // For rendering bounding boxes.
    QDemonRef<QDemonRenderVertexBuffer> m_boxVertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_boxIndexBuffer;
    QDemonRef<QDemonRenderShaderProgram> m_boxShader;
    QDemonRef<QDemonRenderShaderProgram> m_screenRectShader;

    QDemonRef<QDemonRenderVertexBuffer> m_axisVertexBuffer;
    QDemonRef<QDemonRenderShaderProgram> m_axisShader;

    // X,Y quad, broken down into 2 triangles and normalized over
    //-1,1.
    QDemonRef<QDemonRenderVertexBuffer> m_quadVertexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_quadIndexBuffer;
    QDemonRef<QDemonRenderIndexBuffer> m_rectIndexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_quadInputAssembler;
    QDemonRef<QDemonRenderInputAssembler> m_rectInputAssembler;
    QDemonRef<QDemonRenderAttribLayout> m_quadAttribLayout;
    QDemonRef<QDemonRenderAttribLayout> m_rectAttribLayout;

    // X,Y triangle strip quads in screen coord dynamiclly setup
    QDemonRef<QDemonRenderVertexBuffer> m_quadStripVertexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_quadStripInputAssembler;
    QDemonRef<QDemonRenderAttribLayout> m_quadStripAttribLayout;

    // X,Y,Z point which is used for instanced based rendering of points
    QDemonRef<QDemonRenderVertexBuffer> m_pointVertexBuffer;
    QDemonRef<QDemonRenderInputAssembler> m_pointInputAssembler;
    QDemonRef<QDemonRenderAttribLayout> m_pointAttribLayout;

    QDemonRef<QDemonLayerSceneShader> m_sceneLayerShader;
    QDemonRef<QDemonLayerProgAABlendShader> m_layerProgAAShader;

    TShaderMap m_shaders;
    TStrConstanBufMap m_constantBuffers; ///< store the the shader constant buffers
    // Option is true if we have attempted to generate the shader.
    // This does not mean we were successul, however.
    QDemonRef<QDemonDefaultMaterialRenderableDepthShader> m_defaultMaterialDepthPrepassShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthPrepassShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthPrepassShaderDisplaced;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthTessLinearPrepassShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthTessLinearPrepassShaderDisplaced;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthTessPhongPrepassShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_depthTessNPatchPrepassShader;
    QDemonRef<QDemonTextDepthShader> m_textDepthPrepassShader;
    QDemonRef<QDemonDefaultAoPassShader> m_defaultAoPassShader;
    QDemonRef<QDemonDefaultAoPassShader> m_fakeDepthShader;
    QDemonRef<QDemonDefaultAoPassShader> m_fakeCubemapDepthShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_paraboloidDepthShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_paraboloidDepthTessLinearShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_paraboloidDepthTessPhongShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_paraboloidDepthTessNPatchShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_cubemapDepthShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_cubemapDepthTessLinearShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_cubemapDepthTessPhongShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_cubemapDepthTessNPatchShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_orthographicDepthShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_orthographicDepthTessLinearShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_orthographicDepthTessPhongShader;
    QDemonRef<QDemonRenderableDepthPrepassShader> m_orthographicDepthTessNPatchShader;
    QDemonRef<QDemonShadowmapPreblurShader> m_cubeShadowBlurXShader;
    QDemonRef<QDemonShadowmapPreblurShader> m_cubeShadowBlurYShader;
    QDemonRef<QDemonShadowmapPreblurShader> m_orthoShadowBlurXShader;
    QDemonRef<QDemonShadowmapPreblurShader> m_orthoShadowBlurYShader;

#ifdef ADVANCED_BLEND_SW_FALLBACK
    QDemonRef<QDemonAdvancedModeBlendShader> m_advancedModeOverlayBlendShader;
    QDemonRef<QDemonAdvancedModeBlendShader> m_advancedModeColorBurnBlendShader;
    QDemonRef<QDemonAdvancedModeBlendShader> m_advancedModeColorDodgeBlendShader;
#endif
    // Text shaders may be generated on demand.
    QScopedPointer<QDemonTextShader> m_textShader;
    QScopedPointer<QDemonTextShader> m_textPathShader;
    QScopedPointer<QDemonTextShader> m_textWidgetShader;
    QScopedPointer<QDemonTextShader> m_textOnscreenShader;

    // Overlay used to render all widgets.
    QRect m_beginFrameViewport;
    QDemonRef<QDemonRenderTexture2D> m_widgetTexture;
    QDemonRef<QDemonRenderFrameBuffer> m_widgetFbo;

#ifdef ADVANCED_BLEND_SW_FALLBACK
    // Advanced blend mode SW fallback
    QDemonResourceTexture2D m_layerBlendTexture;
    QDemonRef<QDemonRenderFrameBuffer> m_blendFb;
#endif
    // Allocator for temporary data that is cleared after every layer.
    TInstanceRenderMap m_instanceRenderMap;
    TLayerRenderList m_lastFrameLayers;

    // Set from the first layer.
    TPickResultArray m_lastPickResults;

    // Temporary information stored only when rendering a particular layer.
    QDemonLayerRenderData *m_currentLayer;
    QMatrix4x4 m_viewProjection;
    QByteArray m_generatedShaderString;

    TStrVertBufMap m_widgetVertexBuffers;
    TStrIndexBufMap m_widgetIndexBuffers;
    TStrShaderMap m_widgetShaders;
    TStrIAMap m_widgetInputAssembler;

    TBoneIdNodeMap m_boneIdNodeMap;

    bool m_pickRenderPlugins;
    bool m_layerCachingEnabled;
    bool m_layerGPuProfilingEnabled;
    QDemonShaderDefaultMaterialKeyProperties m_defaultMaterialShaderKeyProperties;

public:
    QDemonRendererImpl(QDemonRenderContextInterface *ctx);
    virtual ~QDemonRendererImpl() override;
    QDemonShaderDefaultMaterialKeyProperties &defaultMaterialShaderKeyProperties()
    {
        return m_defaultMaterialShaderKeyProperties;
    }

    void enableLayerCaching(bool inEnabled) override { m_layerCachingEnabled = inEnabled; }
    bool isLayerCachingEnabled() const override { return m_layerCachingEnabled; }

    void enableLayerGpuProfiling(bool inEnabled) override { m_layerGPuProfilingEnabled = inEnabled; }
    bool isLayerGpuProfilingEnabled() const override { return m_layerGPuProfilingEnabled; }

    // Calls prepare layer for render
    // and then do render layer.
    bool prepareLayerForRender(QDemonRenderLayer &inLayer,
                               const QVector2D &inViewportDimensions,
                               bool inRenderSiblings,
                               const QDemonRenderInstanceId id) override;
    void renderLayer(QDemonRenderLayer &inLayer,
                     const QVector2D &inViewportDimensions,
                     bool clear,
                     QVector3D clearColor,
                     bool inRenderSiblings,
                     const QDemonRenderInstanceId id) override;
    void childrenUpdated(QDemonGraphNode &inParent) override;
    float getTextScale(const QDemonText &inText) override;

    QDemonRenderCamera *getCameraForNode(const QDemonGraphNode &inNode) const override;
    QDemonOption<QDemonCuboidRect> getCameraBounds(const QDemonGraphObject &inObject) override;
    virtual QDemonRenderLayer *getLayerForNode(const QDemonGraphNode &inNode) const;
    QDemonRef<QDemonLayerRenderData> getOrCreateLayerRenderDataForNode(const QDemonGraphNode &inNode,
                                                                       const QDemonRenderInstanceId id = nullptr);

    void beginFrame() override;
    void endFrame() override;

    void pickRenderPlugins(bool inPick) override { m_pickRenderPlugins = inPick; }
    QDemonRenderPickResult pick(QDemonRenderLayer &inLayer,
                                const QVector2D &inViewportDimensions,
                                const QVector2D &inMouseCoords,
                                bool inPickSiblings,
                                bool inPickEverything,
                                const QDemonRenderInstanceId id) override;

    virtual QDemonOption<QVector2D> facePosition(QDemonGraphNode &inNode,
                                                 QDemonBounds3 inBounds,
                                                 const QMatrix4x4 &inGlobalTransform,
                                                 const QVector2D &inViewportDimensions,
                                                 const QVector2D &inMouseCoords,
                                                 QDemonDataRef<QDemonGraphObject *> inMapperObjects,
                                                 QDemonRenderBasisPlanes inPlane) override;

    virtual QDemonRenderPickResult pickOffscreenLayer(QDemonRenderLayer &inLayer,
                                                      const QVector2D &inViewportDimensions,
                                                      const QVector2D &inMouseCoords,
                                                      bool inPickEverything);

    QVector3D unprojectToPosition(QDemonGraphNode &inNode, QVector3D &inPosition, const QVector2D &inMouseVec) const override;
    QVector3D unprojectWithDepth(QDemonGraphNode &inNode, QVector3D &inPosition, const QVector3D &inMouseVec) const override;
    QVector3D projectPosition(QDemonGraphNode &inNode, const QVector3D &inPosition) const override;

    QDemonOption<QDemonLayerPickSetup> getLayerPickSetup(QDemonRenderLayer &inLayer,
                                                         const QVector2D &inMouseCoords,
                                                         const QSize &inPickDims) override;

    QDemonOption<QRectF> getLayerRect(QDemonRenderLayer &inLayer) override;

    void runLayerRender(QDemonRenderLayer &inLayer, const QMatrix4x4 &inViewProjection) override;

    void renderLayerRect(QDemonRenderLayer &inLayer, const QVector3D &inColor) override;
    void addRenderWidget(QDemonRenderWidgetInterface &inWidget) override;

    QDemonScaleAndPosition getWorldToPixelScaleFactor(QDemonRenderLayer &inLayer, const QVector3D &inWorldPoint) override;
    QDemonScaleAndPosition getWorldToPixelScaleFactor(const QDemonRenderCamera &inCamera,
                                                      const QVector3D &inWorldPoint,
                                                      QDemonLayerRenderData &inRenderData);

    void releaseLayerRenderResources(QDemonRenderLayer &inLayer, const QDemonRenderInstanceId id) override;

    void renderQuad(const QVector2D inDimensions, const QMatrix4x4 &inMVP, QDemonRenderTexture2D &inQuadTexture) override;
    void renderQuad() override;

    void renderPointsIndirect() override;

    // render a screen aligned 2D text
    void renderText2D(float x, float y, QDemonOption<QVector3D> inColor, const QString &text) override;
    bool prepareTextureAtlasForRender();

    // render Gpu profiler values
    void renderGpuProfilerStats(float x, float y, QDemonOption<QVector3D> inColor) override;

    // Callback during the layer render process.
    void layerNeedsFrameClear(QDemonLayerRenderData &inLayer);
    void beginLayerDepthPassRender(QDemonLayerRenderData &inLayer);
    void endLayerDepthPassRender();
    void beginLayerRender(QDemonLayerRenderData &inLayer);
    void endLayerRender();
    void prepareImageForIbl(QDemonRenderImage &inImage);

    QDemonRef<QDemonRenderShaderProgram> compileShader(const QByteArray &inName, const char *inVert, const char *inFrame);

    QDemonRef<QDemonRenderShaderProgram> generateShader(QDemonSubsetRenderable &inRenderable, const TShaderFeatureSet &inFeatureSet);
    QDemonRef<QDemonShaderGeneratorGeneratedShader> getShader(QDemonSubsetRenderable &inRenderable,
                                                              const TShaderFeatureSet &inFeatureSet);

    QDemonRef<QDemonDefaultAoPassShader> getDefaultAoPassShader(TShaderFeatureSet inFeatureSet);
    QDemonRef<QDemonDefaultAoPassShader> getFakeDepthShader(TShaderFeatureSet inFeatureSet);
    QDemonRef<QDemonDefaultAoPassShader> getFakeCubeDepthShader(TShaderFeatureSet inFeatureSet);
    QDemonRef<QDemonDefaultMaterialRenderableDepthShader> getRenderableDepthShader();

    QDemonRef<QDemonRenderableDepthPrepassShader> getParaboloidDepthShader(TessModeValues inTessMode);
    QDemonRef<QDemonRenderableDepthPrepassShader> getParaboloidDepthNoTessShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getParaboloidDepthTessLinearShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getParaboloidDepthTessPhongShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getParaboloidDepthTessNPatchShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getCubeShadowDepthShader(TessModeValues inTessMode);
    QDemonRef<QDemonRenderableDepthPrepassShader> getCubeDepthNoTessShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getCubeDepthTessLinearShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getCubeDepthTessPhongShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getCubeDepthTessNPatchShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getOrthographicDepthShader(TessModeValues inTessMode);
    QDemonRef<QDemonRenderableDepthPrepassShader> getOrthographicDepthNoTessShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getOrthographicDepthTessLinearShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getOrthographicDepthTessPhongShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getOrthographicDepthTessNPatchShader();

    QDemonRef<QDemonRenderableDepthPrepassShader> getDepthPrepassShader(bool inDisplaced);
    QDemonRef<QDemonRenderableDepthPrepassShader> getDepthTessPrepassShader(TessModeValues inTessMode, bool inDisplaced);
    QDemonRef<QDemonRenderableDepthPrepassShader> getDepthTessLinearPrepassShader(bool inDisplaced);
    QDemonRef<QDemonRenderableDepthPrepassShader> getDepthTessPhongPrepassShader();
    QDemonRef<QDemonRenderableDepthPrepassShader> getDepthTessNPatchPrepassShader();
    QDemonRef<QDemonTextDepthShader> getTextDepthShader();
    QDemonTextRenderHelper getShader(QDemonTextRenderable &inRenderable, bool inUsePathRendering);
    QDemonTextRenderHelper getTextShader(bool inUsePathRendering);
    QDemonTextRenderHelper getTextWidgetShader();
    QDemonTextRenderHelper getOnscreenTextShader();
    QDemonRef<QDemonLayerSceneShader> getSceneLayerShader();
    QDemonRef<QDemonRenderShaderProgram> getTextAtlasEntryShader();
    void generateXYQuad();
    void generateXYQuadStrip();
    void generateXYZPoint();
    QPair<QDemonRef<QDemonRenderVertexBuffer>, QDemonRef<QDemonRenderIndexBuffer>> getXYQuad();
    QDemonRef<QDemonLayerProgAABlendShader> getLayerProgAABlendShader();
    QDemonRef<QDemonShadowmapPreblurShader> getCubeShadowBlurXShader();
    QDemonRef<QDemonShadowmapPreblurShader> getCubeShadowBlurYShader();
    QDemonRef<QDemonShadowmapPreblurShader> getOrthoShadowBlurXShader();
    QDemonRef<QDemonShadowmapPreblurShader> getOrthoShadowBlurYShader();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    QDemonRef<QDemonAdvancedModeBlendShader> getAdvancedBlendModeShader(AdvancedBlendModes blendMode);
    QDemonRef<QDemonAdvancedModeBlendShader> getOverlayBlendModeShader();
    QDemonRef<QDemonAdvancedModeBlendShader> getColorBurnBlendModeShader();
    QDemonRef<QDemonAdvancedModeBlendShader> getColorDodgeBlendModeShader();
#endif
    QDemonLayerRenderData *getLayerRenderData() { return m_currentLayer; }
    QDemonLayerGlobalRenderProperties getLayerGlobalRenderProperties();
    void updateCbAoShadow(const QDemonRenderLayer *pLayer, const QDemonRenderCamera *pCamera, QDemonResourceTexture2D &inDepthTexture);

    QDemonRef<QDemonRenderContext> getContext() { return m_context; }

    QDemonRenderContextInterface *getDemonContext() { return m_demonContext; }

    void drawScreenRect(QRectF inRect, const QVector3D &inColor);
    // Binds an offscreen texture.  Widgets are rendered last.
    void setupWidgetLayer();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    QDemonRef<QDemonRenderTexture2D> getLayerBlendTexture() { return m_layerBlendTexture.getTexture(); }

    QDemonRef<QDemonRenderFrameBuffer> getBlendFB() { return m_blendFb; }
#endif
    // widget context implementation
    QDemonRef<QDemonRenderVertexBuffer> getOrCreateVertexBuffer(
            const QByteArray &inStr,
            quint32 stride,
            QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>());
    QDemonRef<QDemonRenderIndexBuffer> getOrCreateIndexBuffer(
            const QByteArray &inStr,
            QDemonRenderComponentType componentType,
            size_t size,
            QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>());
    QDemonRef<QDemonRenderAttribLayout> createAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs);
    QDemonRef<QDemonRenderInputAssembler> getOrCreateInputAssembler(const QByteArray &inStr,
                                                                    QDemonRef<QDemonRenderAttribLayout> attribLayout,
                                                                    QDemonConstDataRef<QDemonRef<QDemonRenderVertexBuffer>> buffers,
                                                                    const QDemonRef<QDemonRenderIndexBuffer> indexBuffer,
                                                                    QDemonConstDataRef<quint32> strides,
                                                                    QDemonConstDataRef<quint32> offsets);

    QDemonRef<QDemonRenderVertexBuffer> getVertexBuffer(const QByteArray &inStr);
    QDemonRef<QDemonRenderIndexBuffer> getIndexBuffer(const QByteArray &inStr);
    QDemonRef<QDemonRenderInputAssembler> getInputAssembler(const QByteArray &inStr);

    QDemonRef<QDemonRenderShaderProgram> getShader(const QByteArray &inStr);
    QDemonRef<QDemonRenderShaderProgram> compileAndStoreShader(const QByteArray &inStr);
    QDemonRef<QDemonShaderProgramGeneratorInterface> getProgramGenerator();

    QDemonTextDimensions measureText(const QDemonTextRenderInfo &inText);
    void renderText(const QDemonTextRenderInfo &inText,
                    const QVector3D &inTextColor,
                    const QVector3D &inBackgroundColor,
                    const QMatrix4x4 &inMVP);

    // Given a node and a point in the node's local space (most likely its pivot point), we
    // return
    // a normal matrix so you can get the axis out, a transformation from node to camera
    // a new position and a floating point scale factor so you can render in 1/2 perspective
    // mode
    // or orthographic mode if you would like to.
    QDemonWidgetRenderInformation getWidgetRenderInformation(QDemonGraphNode &inNode,
                                                                     const QVector3D &inPos,
                                                                     RenderWidgetModes inWidgetMode);

    QDemonOption<QVector2D> getLayerMouseCoords(QDemonRenderLayer &inLayer,
                                                const QVector2D &inMouseCoords,
                                                const QVector2D &inViewportDimensions,
                                                bool forceImageIntersect = false) const override;

protected:
    QDemonOption<QVector2D> getLayerMouseCoords(QDemonLayerRenderData &inLayer,
                                                const QVector2D &inMouseCoords,
                                                const QVector2D &inViewportDimensions,
                                                bool forceImageIntersect = false) const;
    QDemonPickResultProcessResult processPickResultList(bool inPickEverything);
    // If the mouse y coordinates need to be flipped we expect that to happen before entry into
    // this function
    void getLayerHitObjectList(QDemonLayerRenderData &inLayer,
                               const QVector2D &inViewportDimensions,
                               const QVector2D &inMouseCoords,
                               bool inPickEverything,
                               TPickResultArray &outIntersectionResult);
    void intersectRayWithSubsetRenderable(const QDemonRenderRay &inRay,
                                          QDemonRenderableObject &inRenderableObject,
                                          TPickResultArray &outIntersectionResultList);
};
QT_END_NAMESPACE

#endif
