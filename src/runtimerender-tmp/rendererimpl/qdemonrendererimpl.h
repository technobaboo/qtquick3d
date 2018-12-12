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
#pragma once
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

#include <QtDemonRender/qdemonrendercontext.h>
#include <QtDemonRender/qdemonrendershaderprogram.h>

#include <QtDemon/QDemonFlags>
#include <QtDemon/QDemonBounds3>
#include <QtDemon/QDemonOption>
#include <QtDemon/QDemonDataRef>
#include <QtDemon/qdemoninvasiveset.h>
#include <QtDemon/qdemonunioncast.h>

QT_BEGIN_NAMESPACE
inline bool FloatLessThan(float lhs, float rhs)
{
    float diff = lhs - rhs;
    if (fabs(diff) < .001)
        return false;
    return diff < 0.0f ? true : false;
}
inline bool ISRenderObjectPtrLessThan(const SRenderableObject *lhs,
                                      const SRenderableObject *rhs)
{
    return FloatLessThan(lhs->m_CameraDistanceSq, rhs->m_CameraDistanceSq);
}
inline bool ISRenderObjectPtrGreatThan(const SRenderableObject *lhs,
                                       const SRenderableObject *rhs)
{
    return FloatLessThan(rhs->m_CameraDistanceSq, lhs->m_CameraDistanceSq);
}
inline bool NonZero(float inValue) { return fabs(inValue) > .001f; }
inline bool NonZero(quint32 inValue) { return inValue != 0; }
inline bool IsZero(float inValue) { return fabs(inValue) < .001f; }
inline bool IsNotOne(float inValue) { return fabs(1.0f - inValue) > .001f; }

inline bool IsRectEdgeInBounds(qint32 inNewRectOffset, qint32 inNewRectWidth,
                               qint32 inCurrentRectOffset, qint32 inCurrentRectWidth)
{
    qint32 newEnd = inNewRectOffset + inNewRectWidth;
    qint32 currentEnd = inCurrentRectOffset + inCurrentRectWidth;
    return inNewRectOffset >= inCurrentRectOffset && newEnd <= currentEnd;
}

struct STextRenderHelper
{
    STextShader *m_Shader;
    QDemonRenderInputAssembler &m_QuadInputAssembler;
    STextRenderHelper(STextShader *inShader, QDemonRenderInputAssembler &inQuadInputAssembler)
        : m_Shader(inShader)
        , m_QuadInputAssembler(inQuadInputAssembler)
    {
    }
};

struct SPickResultProcessResult : public QDemonRenderPickResult
{
    SPickResultProcessResult(const QDemonRenderPickResult &inSrc)
        : QDemonRenderPickResult(inSrc)
        , m_WasPickConsumed(false)
    {
    }
    SPickResultProcessResult()
        : m_WasPickConsumed(false)
    {
    }
    bool m_WasPickConsumed;
};

struct STextShaderPtr
{
    bool m_HasGeneratedShader;
    STextShader *m_Shader;
    STextShaderPtr()
        : m_HasGeneratedShader(false)
        , m_Shader(nullptr)
    {
    }
    bool HasGeneratedShader() { return m_HasGeneratedShader; }
    void Set(STextShader *inShader)
    {
        m_Shader = inShader;
        m_HasGeneratedShader = true;
    }
    ~STextShaderPtr()
    {
        if (m_Shader)
            delete m_Shader;
    }
    operator STextShader *() { return m_Shader; }
};

class Q_DEMONRUNTIMERENDER_EXPORT QDemonRendererImpl : public IQDemonRenderer, public IRenderWidgetContext
{
    typedef QHash<SShaderDefaultMaterialKey, SShaderGeneratorGeneratedShader *> TShaderMap;
    typedef QHash<QString, QSharedPointer<QDemonRenderConstantBuffer>> TStrConstanBufMap;
    typedef QHash<SRenderInstanceId, QSharedPointer<SLayerRenderData>, eastl::hash<SRenderInstanceId>> TInstanceRenderMap;
    typedef QVector<SLayerRenderData *> TLayerRenderList;
    typedef QVector<QDemonRenderPickResult> TPickResultArray;

    // Items to implement the widget context.
    typedef QHash<QString, QSharedPointer<QDemonRenderVertexBuffer>> TStrVertBufMap;
    typedef QHash<QString, QSharedPointer<QDemonRenderIndexBuffer>> TStrIndexBufMap;
    typedef QHash<QString, QSharedPointer<QDemonRenderShaderProgram>> TStrShaderMap;
    typedef QHash<QString, QSharedPointer<QDemonRenderInputAssembler>> TStrIAMap;

    typedef QHash<long, SNode *> TBoneIdNodeMap;

    IQDemonRenderContext &m_demonContext;
    QSharedPointer<QDemonRenderContext> m_Context;
    QSharedPointer<IBufferManager> m_BufferManager;
    QSharedPointer<IOffscreenRenderManager> m_OffscreenRenderManager;
    InvasiveSet<SShaderGeneratorGeneratedShader, SGGSGet, SGGSSet> m_LayerShaders;
    // For rendering bounding boxes.
    QSharedPointer<QDemonRenderVertexBuffer> m_BoxVertexBuffer;
    QSharedPointer<QDemonRenderIndexBuffer> m_BoxIndexBuffer;
    QSharedPointer<QDemonRenderShaderProgram> m_BoxShader;
    QSharedPointer<QDemonRenderShaderProgram> m_ScreenRectShader;

    QSharedPointer<QDemonRenderVertexBuffer> m_AxisVertexBuffer;
    QSharedPointer<QDemonRenderShaderProgram> m_AxisShader;

    // X,Y quad, broken down into 2 triangles and normalized over
    //-1,1.
    QSharedPointer<QDemonRenderVertexBuffer> m_QuadVertexBuffer;
    QSharedPointer<QDemonRenderIndexBuffer> m_QuadIndexBuffer;
    QSharedPointer<QDemonRenderIndexBuffer> m_RectIndexBuffer;
    QSharedPointer<QDemonRenderInputAssembler> m_QuadInputAssembler;
    QSharedPointer<QDemonRenderInputAssembler> m_RectInputAssembler;
    QSharedPointer<QDemonRenderAttribLayout> m_QuadAttribLayout;
    QSharedPointer<QDemonRenderAttribLayout> m_RectAttribLayout;

    // X,Y triangle strip quads in screen coord dynamiclly setup
    QSharedPointer<QDemonRenderVertexBuffer> m_QuadStripVertexBuffer;
    QSharedPointer<QDemonRenderInputAssembler> m_QuadStripInputAssembler;
    QSharedPointer<QDemonRenderAttribLayout> m_QuadStripAttribLayout;

    // X,Y,Z point which is used for instanced based rendering of points
    QSharedPointer<QDemonRenderVertexBuffer> m_PointVertexBuffer;
    QSharedPointer<QDemonRenderInputAssembler> m_PointInputAssembler;
    QSharedPointer<QDemonRenderAttribLayout> m_PointAttribLayout;

    QDemonOption<QSharedPointer<SLayerSceneShader>> m_SceneLayerShader;
    QDemonOption<QSharedPointer<SLayerProgAABlendShader>> m_LayerProgAAShader;

    TShaderMap m_Shaders;
    TStrConstanBufMap m_ConstantBuffers; ///< store the the shader constant buffers
    // Option is true if we have attempted to generate the shader.
    // This does not mean we were successul, however.
    QDemonOption<QSharedPointer<SDefaultMaterialRenderableDepthShader>> m_DefaultMaterialDepthPrepassShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthPrepassShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthPrepassShaderDisplaced;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthTessLinearPrepassShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthTessLinearPrepassShaderDisplaced;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthTessPhongPrepassShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_DepthTessNPatchPrepassShader;
    QDemonOption<QSharedPointer<STextDepthShader>> m_TextDepthPrepassShader;
    QDemonOption<QSharedPointer<SDefaultAoPassShader>> m_DefaultAoPassShader;
    QDemonOption<QSharedPointer<SDefaultAoPassShader>> m_FakeDepthShader;
    QDemonOption<QSharedPointer<SDefaultAoPassShader>> m_FakeCubemapDepthShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_ParaboloidDepthShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessLinearShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessPhongShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessNPatchShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_CubemapDepthShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_CubemapDepthTessLinearShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_CubemapDepthTessPhongShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_CubemapDepthTessNPatchShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_OrthographicDepthShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_OrthographicDepthTessLinearShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_OrthographicDepthTessPhongShader;
    QDemonOption<QSharedPointer<SRenderableDepthPrepassShader>> m_OrthographicDepthTessNPatchShader;
    QDemonOption<QSharedPointer<SShadowmapPreblurShader>> m_CubeShadowBlurXShader;
    QDemonOption<QSharedPointer<SShadowmapPreblurShader>> m_CubeShadowBlurYShader;
    QDemonOption<QSharedPointer<SShadowmapPreblurShader>> m_OrthoShadowBlurXShader;
    QDemonOption<QSharedPointer<SShadowmapPreblurShader>> m_OrthoShadowBlurYShader;

#ifdef ADVANCED_BLEND_SW_FALLBACK
    QDemonOption<QSharedPointer<SAdvancedModeBlendShader>> m_AdvancedModeOverlayBlendShader;
    QDemonOption<QSharedPointer<SAdvancedModeBlendShader>> m_AdvancedModeColorBurnBlendShader;
    QDemonOption<QSharedPointer<SAdvancedModeBlendShader>> m_AdvancedModeColorDodgeBlendShader;
#endif
    // Text shaders may be generated on demand.
    STextShaderPtr m_TextShader;
    STextShaderPtr m_TextPathShader;
    STextShaderPtr m_TextWidgetShader;
    STextShaderPtr m_TextOnscreenShader;

    // Overlay used to render all widgets.
    QDemonRenderRect m_BeginFrameViewport;
    QSharedPointer<QDemonRenderTexture2D> m_WidgetTexture;
    QSharedPointer<QDemonRenderFrameBuffer> m_WidgetFBO;

#ifdef ADVANCED_BLEND_SW_FALLBACK
    // Advanced blend mode SW fallback
    CResourceTexture2D m_LayerBlendTexture;
    QSharedPointer<QDemonRenderFrameBuffer> m_BlendFB;
#endif
    // Allocator for temporary data that is cleared after every layer.
    TInstanceRenderMap m_InstanceRenderMap;
    TLayerRenderList m_LastFrameLayers;

    // Set from the first layer.
    TPickResultArray m_LastPickResults;

    // Temporary information stored only when rendering a particular layer.
    SLayerRenderData *m_CurrentLayer;
    QMatrix4x4 m_ViewProjection;
    QString m_GeneratedShaderString;

    TStrVertBufMap m_WidgetVertexBuffers;
    TStrIndexBufMap m_WidgetIndexBuffers;
    TStrShaderMap m_WidgetShaders;
    TStrIAMap m_WidgetInputAssembler;

    TBoneIdNodeMap m_BoneIdNodeMap;

    bool m_PickRenderPlugins;
    bool m_LayerCachingEnabled;
    bool m_LayerGPuProfilingEnabled;
    SShaderDefaultMaterialKeyProperties m_DefaultMaterialShaderKeyProperties;

public:
    QDemonRendererImpl(IQDemonRenderContext &ctx);
    virtual ~QDemonRendererImpl();
    SShaderDefaultMaterialKeyProperties &DefaultMaterialShaderKeyProperties()
    {
        return m_DefaultMaterialShaderKeyProperties;
    }

    void EnableLayerCaching(bool inEnabled) override { m_LayerCachingEnabled = inEnabled; }
    bool IsLayerCachingEnabled() const override { return m_LayerCachingEnabled; }

    void EnableLayerGpuProfiling(bool inEnabled) override
    {
        m_LayerGPuProfilingEnabled = inEnabled;
    }
    bool IsLayerGpuProfilingEnabled() const override { return m_LayerGPuProfilingEnabled; }

    // Calls prepare layer for render
    // and then do render layer.
    bool PrepareLayerForRender(SLayer &inLayer, const QVector2D &inViewportDimensions,
                               bool inRenderSiblings, const SRenderInstanceId id) override;
    void RenderLayer(SLayer &inLayer, const QVector2D &inViewportDimensions,
                     bool clear, QVector3D clearColor, bool inRenderSiblings,
                     const SRenderInstanceId id) override;
    void ChildrenUpdated(SNode &inParent) override;
    float GetTextScale(const SText &inText) override;

    SCamera *GetCameraForNode(const SNode &inNode) const override;
    QDemonOption<SCuboidRect> GetCameraBounds(const SGraphObject &inObject) override;
    virtual SLayer *GetLayerForNode(const SNode &inNode) const;
    SLayerRenderData *GetOrCreateLayerRenderDataForNode(const SNode &inNode,
                                                        const SRenderInstanceId id = nullptr);

    IRenderWidgetContext &GetRenderWidgetContext()
    {
        return *this;
    }

    void BeginFrame() override;
    void EndFrame() override;

    void PickRenderPlugins(bool inPick) override { m_PickRenderPlugins = inPick; }
    QDemonRenderPickResult Pick(SLayer &inLayer, const QVector2D &inViewportDimensions,
                               const QVector2D &inMouseCoords, bool inPickSiblings,
                               bool inPickEverything,
                               const SRenderInstanceId id) override;

    virtual QDemonOption<QVector2D>
    FacePosition(SNode &inNode, QDemonBounds3 inBounds, const QMatrix4x4 &inGlobalTransform,
                 const QVector2D &inViewportDimensions, const QVector2D &inMouseCoords,
                 QDemonDataRef<SGraphObject *> inMapperObjects, SBasisPlanes::Enum inPlane) override;

    virtual QDemonRenderPickResult PickOffscreenLayer(SLayer &inLayer,
                                                     const QVector2D &inViewportDimensions,
                                                     const QVector2D &inMouseCoords,
                                                     bool inPickEverything);

    QVector3D UnprojectToPosition(SNode &inNode, QVector3D &inPosition,
                                  const QVector2D &inMouseVec) const override;
    QVector3D UnprojectWithDepth(SNode &inNode, QVector3D &inPosition,
                                 const QVector3D &inMouseVec) const override;
    QVector3D ProjectPosition(SNode &inNode, const QVector3D &inPosition) const override;

    QDemonOption<SLayerPickSetup> GetLayerPickSetup(SLayer &inLayer,
                                              const QVector2D &inMouseCoords,
                                              const QSize &inPickDims) override;

    QDemonOption<QDemonRenderRectF> GetLayerRect(SLayer &inLayer) override;

    void RunLayerRender(SLayer &inLayer, const QMatrix4x4 &inViewProjection) override;

    void RenderLayerRect(SLayer &inLayer, const QVector3D &inColor) override;
    void AddRenderWidget(IRenderWidget &inWidget) override;

    SScaleAndPosition GetWorldToPixelScaleFactor(SLayer &inLayer,
                                                 const QVector3D &inWorldPoint) override;
    SScaleAndPosition GetWorldToPixelScaleFactor(const SCamera &inCamera,
                                                 const QVector3D &inWorldPoint,
                                                 SLayerRenderData &inRenderData);

    void ReleaseLayerRenderResources(SLayer &inLayer, const SRenderInstanceId id) override;

    void RenderQuad(const QVector2D inDimensions, const QMatrix4x4 &inMVP,
                    QDemonRenderTexture2D &inQuadTexture) override;
    void RenderQuad() override;

    void RenderPointsIndirect() override;

    // render a screen aligned 2D text
    void RenderText2D(float x, float y, QDemonOption<QVector3D> inColor,
                      const char *text) override;
    bool PrepareTextureAtlasForRender();

    // render Gpu profiler values
    void RenderGpuProfilerStats(float x, float y,
                                QDemonOption<QVector3D> inColor) override;

    // Callback during the layer render process.
    void LayerNeedsFrameClear(SLayerRenderData &inLayer);
    void BeginLayerDepthPassRender(SLayerRenderData &inLayer);
    void EndLayerDepthPassRender();
    void BeginLayerRender(SLayerRenderData &inLayer);
    void EndLayerRender();
    void PrepareImageForIbl(SImage &inImage);

    QDemonRenderShaderProgram *CompileShader(const QString &inName, const char *inVert,
                                             const char *inFrame);

    QDemonRenderShaderProgram *GenerateShader(SSubsetRenderable &inRenderable,
                                              TShaderFeatureSet inFeatureSet);
    SShaderGeneratorGeneratedShader *GetShader(SSubsetRenderable &inRenderable,
                                               TShaderFeatureSet inFeatureSet);

    SDefaultAoPassShader *GetDefaultAoPassShader(TShaderFeatureSet inFeatureSet);
    SDefaultAoPassShader *GetFakeDepthShader(TShaderFeatureSet inFeatureSet);
    SDefaultAoPassShader *GetFakeCubeDepthShader(TShaderFeatureSet inFeatureSet);
    SDefaultMaterialRenderableDepthShader *GetRenderableDepthShader();

    SRenderableDepthPrepassShader *GetParaboloidDepthShader(TessModeValues::Enum inTessMode);
    SRenderableDepthPrepassShader *GetParaboloidDepthNoTessShader();
    SRenderableDepthPrepassShader *GetParaboloidDepthTessLinearShader();
    SRenderableDepthPrepassShader *GetParaboloidDepthTessPhongShader();
    SRenderableDepthPrepassShader *GetParaboloidDepthTessNPatchShader();
    SRenderableDepthPrepassShader *GetCubeShadowDepthShader(TessModeValues::Enum inTessMode);
    SRenderableDepthPrepassShader *GetCubeDepthNoTessShader();
    SRenderableDepthPrepassShader *GetCubeDepthTessLinearShader();
    SRenderableDepthPrepassShader *GetCubeDepthTessPhongShader();
    SRenderableDepthPrepassShader *GetCubeDepthTessNPatchShader();
    SRenderableDepthPrepassShader *GetOrthographicDepthShader(TessModeValues::Enum inTessMode);
    SRenderableDepthPrepassShader *GetOrthographicDepthNoTessShader();
    SRenderableDepthPrepassShader *GetOrthographicDepthTessLinearShader();
    SRenderableDepthPrepassShader *GetOrthographicDepthTessPhongShader();
    SRenderableDepthPrepassShader *GetOrthographicDepthTessNPatchShader();

    SRenderableDepthPrepassShader *GetDepthPrepassShader(bool inDisplaced);
    SRenderableDepthPrepassShader *GetDepthTessPrepassShader(TessModeValues::Enum inTessMode,
                                                             bool inDisplaced);
    SRenderableDepthPrepassShader *GetDepthTessLinearPrepassShader(bool inDisplaced);
    SRenderableDepthPrepassShader *GetDepthTessPhongPrepassShader();
    SRenderableDepthPrepassShader *GetDepthTessNPatchPrepassShader();
    STextDepthShader *GetTextDepthShader();
    STextRenderHelper GetShader(STextRenderable &inRenderable, bool inUsePathRendering);
    STextRenderHelper GetTextShader(bool inUsePathRendering);
    STextRenderHelper GetTextWidgetShader();
    STextRenderHelper GetOnscreenTextShader();
    SLayerSceneShader *GetSceneLayerShader();
    QDemonRenderShaderProgram *GetTextAtlasEntryShader();
    void GenerateXYQuad();
    void GenerateXYQuadStrip();
    void GenerateXYZPoint();
    QPair<QDemonRenderVertexBuffer *, QDemonRenderIndexBuffer *> GetXYQuad();
    SLayerProgAABlendShader *GetLayerProgAABlendShader();
    SShadowmapPreblurShader *GetCubeShadowBlurXShader();
    SShadowmapPreblurShader *GetCubeShadowBlurYShader();
    SShadowmapPreblurShader *GetOrthoShadowBlurXShader();
    SShadowmapPreblurShader *GetOrthoShadowBlurYShader();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    SAdvancedModeBlendShader *GetAdvancedBlendModeShader(AdvancedBlendModes::Enum blendMode);
    SAdvancedModeBlendShader *GetOverlayBlendModeShader();
    SAdvancedModeBlendShader *GetColorBurnBlendModeShader();
    SAdvancedModeBlendShader *GetColorDodgeBlendModeShader();
#endif
    SLayerRenderData *GetLayerRenderData() { return m_CurrentLayer; }
    SLayerGlobalRenderProperties GetLayerGlobalRenderProperties();
    void UpdateCbAoShadow(const SLayer *pLayer, const SCamera *pCamera,
                          CResourceTexture2D &inDepthTexture);

    QDemonRenderContext &GetContext() { return *m_Context; }

    IQDemonRenderContext &GetDemonContext() { return m_demonContext; }

    void DrawScreenRect(QDemonRenderRectF inRect, const QVector3D &inColor);
    // Binds an offscreen texture.  Widgets are rendered last.
    void SetupWidgetLayer();

#ifdef ADVANCED_BLEND_SW_FALLBACK
    QSharedPointer<QDemonRenderTexture2D> GetLayerBlendTexture()
    {
        return m_LayerBlendTexture.GetTexture();
    }

    QSharedPointer<QDemonRenderFrameBuffer> GetBlendFB()
    {
        return m_BlendFB;
    }
#endif
    // widget context implementation
    virtual QDemonRenderVertexBuffer &
    GetOrCreateVertexBuffer(QString &inStr, quint32 stride,
                            QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) override;
    virtual QDemonRenderIndexBuffer &
    GetOrCreateIndexBuffer(QString &inStr,
                           QDemonRenderComponentTypes::Enum componentType, size_t size,
                           QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) override;
    virtual QDemonRenderAttribLayout &
    CreateAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
    virtual QDemonRenderInputAssembler &
    GetOrCreateInputAssembler(QString &inStr, QDemonRenderAttribLayout *attribLayout,
                              QDemonConstDataRef<QDemonRenderVertexBuffer *> buffers,
                              const QDemonRenderIndexBuffer *indexBuffer,
                              QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets) override;

    QDemonRenderVertexBuffer *GetVertexBuffer(QString &inStr) override;
    QDemonRenderIndexBuffer *GetIndexBuffer(QString &inStr) override;
    QDemonRenderInputAssembler *GetInputAssembler(QString &inStr) override;

    QDemonRenderShaderProgram *GetShader(const QString &inStr) override;
    QDemonRenderShaderProgram *CompileAndStoreShader(const QString &inStr) override;
    IShaderProgramGenerator &GetProgramGenerator() override;

    STextDimensions MeasureText(const STextRenderInfo &inText) override;
    void RenderText(const STextRenderInfo &inText, const QVector3D &inTextColor,
                    const QVector3D &inBackgroundColor, const QMatrix4x4 &inMVP) override;

    // Given a node and a point in the node's local space (most likely its pivot point), we
    // return
    // a normal matrix so you can get the axis out, a transformation from node to camera
    // a new position and a floating point scale factor so you can render in 1/2 perspective
    // mode
    // or orthographic mode if you would like to.
    virtual SWidgetRenderInformation
    GetWidgetRenderInformation(SNode &inNode, const QVector3D &inPos,
                               RenderWidgetModes::Enum inWidgetMode) override;

    QDemonOption<QVector2D> GetLayerMouseCoords(SLayer &inLayer, const QVector2D &inMouseCoords,
                                          const QVector2D &inViewportDimensions,
                                          bool forceImageIntersect = false) const override;

protected:
    QDemonOption<QVector2D> GetLayerMouseCoords(SLayerRenderData &inLayer, const QVector2D &inMouseCoords,
                                          const QVector2D &inViewportDimensions,
                                          bool forceImageIntersect = false) const;
    SPickResultProcessResult ProcessPickResultList(bool inPickEverything);
    // If the mouse y coordinates need to be flipped we expect that to happen before entry into
    // this function
    void GetLayerHitObjectList(SLayerRenderData &inLayer, const QVector2D &inViewportDimensions,
                               const QVector2D &inMouseCoords, bool inPickEverything,
                               TPickResultArray &outIntersectionResult);
    void IntersectRayWithSubsetRenderable(const SRay &inRay,
                                          SRenderableObject &inRenderableObject,
                                          TPickResultArray &outIntersectionResultList);
};
QT_END_NAMESPACE

#endif
