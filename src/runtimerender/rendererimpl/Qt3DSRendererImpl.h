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
#include <Qt3DSRender.h>
#include <Qt3DSRenderer.h>
#include <Qt3DSRenderableObjects.h>
#include <Qt3DSRendererImplShaders.h>
#include <Qt3DSRendererImplLayerRenderData.h>
#include <Qt3DSFlags.h>
#include <Qt3DSRenderMesh.h>
#include <Qt3DSRenderModel.h>
#include <Qt3DSBounds3.h>
#include <qdemonrendercontext.h>
#include <qdemonrendershaderprogram.h>
#include <Qt3DSRenderDefaultMaterial.h>
#include <StringTable.h>
#include <Qt3DSInvasiveSet.h>
#include <Qt3DSDataRef.h>
#include <Qt3DSRenderLayer.h>
#include <Qt3DSRenderRay.h>
#include <Qt3DSRenderText.h>
#include <Qt3DSOffscreenRenderManager.h>
#include <Qt3DSAtomic.h>
#include <Qt3DSRenderCamera.h>
#include <Qt3DSRenderShaderCache.h>
#include <Qt3DSRenderContextCore.h>
#include <Qt3DSOffscreenRenderManager.h>
#include <Qt3DSRendererImplLayerRenderHelper.h>
#include <Qt3DSRenderWidgets.h>
#include <Qt3DSRenderShaderCodeGenerator.h>
#include <Qt3DSRenderClippingFrustum.h>
#include <Qt3DSUnionCast.h>
#include <FastAllocator.h>
#include <AutoDeallocatorAllocator.h>
#include <Qt3DSRenderShaderKeys.h>
#include <Qt3DSRenderShaderCache.h>
#include <Qt3DSRenderProfiler.h>
#include <Qt3DSRenderDefaultMaterialShaderGenerator.h>

namespace qt3ds {
namespace render {

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

    struct SPickResultProcessResult : public Qt3DSRenderPickResult
    {
        SPickResultProcessResult(const Qt3DSRenderPickResult &inSrc)
            : Qt3DSRenderPickResult(inSrc)
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
        NVAllocatorCallback &m_Allocator;
        bool m_HasGeneratedShader;
        STextShader *m_Shader;
        STextShaderPtr(NVAllocatorCallback &alloc)
            : m_Allocator(alloc)
            , m_HasGeneratedShader(false)
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
                NVDelete(m_Allocator, m_Shader);
        }
        operator STextShader *() { return m_Shader; }
    };

    class QDEMON_AUTOTEST_EXPORT Qt3DSRendererImpl : public IQt3DSRenderer, public IRenderWidgetContext
    {
        typedef nvhash_map<SShaderDefaultMaterialKey, SShaderGeneratorGeneratedShader *> TShaderMap;
        typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<QDemonRenderConstantBuffer>>
            TStrConstanBufMap;
        typedef nvhash_map<SRenderInstanceId, QDemonScopedRefCounted<SLayerRenderData>,
                           eastl::hash<SRenderInstanceId>> TInstanceRenderMap;
        typedef nvvector<SLayerRenderData *> TLayerRenderList;
        typedef nvvector<Qt3DSRenderPickResult> TPickResultArray;

        // Items to implement the widget context.
        typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<QDemonRenderVertexBuffer>>
            TStrVertBufMap;
        typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<QDemonRenderIndexBuffer>>
            TStrIndexBufMap;
        typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<QDemonRenderShaderProgram>>
            TStrShaderMap;
        typedef nvhash_map<CRegisteredString, QDemonScopedRefCounted<QDemonRenderInputAssembler>> TStrIAMap;

        typedef nvhash_map<long, SNode *> TBoneIdNodeMap;

        IQt3DSRenderContext &m_qt3dsContext;
        QDemonScopedRefCounted<QDemonRenderContext> m_Context;
        QDemonScopedRefCounted<IBufferManager> m_BufferManager;
        QDemonScopedRefCounted<IOffscreenRenderManager> m_OffscreenRenderManager;
        QDemonScopedRefCounted<IStringTable> m_StringTable;
        InvasiveSet<SShaderGeneratorGeneratedShader, SGGSGet, SGGSSet> m_LayerShaders;
        // For rendering bounding boxes.
        QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_BoxVertexBuffer;
        QDemonScopedRefCounted<QDemonRenderIndexBuffer> m_BoxIndexBuffer;
        QDemonScopedRefCounted<QDemonRenderShaderProgram> m_BoxShader;
        QDemonScopedRefCounted<QDemonRenderShaderProgram> m_ScreenRectShader;

        QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_AxisVertexBuffer;
        QDemonScopedRefCounted<QDemonRenderShaderProgram> m_AxisShader;

        // X,Y quad, broken down into 2 triangles and normalized over
        //-1,1.
        QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_QuadVertexBuffer;
        QDemonScopedRefCounted<QDemonRenderIndexBuffer> m_QuadIndexBuffer;
        QDemonScopedRefCounted<QDemonRenderIndexBuffer> m_RectIndexBuffer;
        QDemonScopedRefCounted<QDemonRenderInputAssembler> m_QuadInputAssembler;
        QDemonScopedRefCounted<QDemonRenderInputAssembler> m_RectInputAssembler;
        QDemonScopedRefCounted<QDemonRenderAttribLayout> m_QuadAttribLayout;
        QDemonScopedRefCounted<QDemonRenderAttribLayout> m_RectAttribLayout;

        // X,Y triangle strip quads in screen coord dynamiclly setup
        QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_QuadStripVertexBuffer;
        QDemonScopedRefCounted<QDemonRenderInputAssembler> m_QuadStripInputAssembler;
        QDemonScopedRefCounted<QDemonRenderAttribLayout> m_QuadStripAttribLayout;

        // X,Y,Z point which is used for instanced based rendering of points
        QDemonScopedRefCounted<QDemonRenderVertexBuffer> m_PointVertexBuffer;
        QDemonScopedRefCounted<QDemonRenderInputAssembler> m_PointInputAssembler;
        QDemonScopedRefCounted<QDemonRenderAttribLayout> m_PointAttribLayout;

        Option<QDemonScopedRefCounted<SLayerSceneShader>> m_SceneLayerShader;
        Option<QDemonScopedRefCounted<SLayerProgAABlendShader>> m_LayerProgAAShader;

        TShaderMap m_Shaders;
        TStrConstanBufMap m_ConstantBuffers; ///< store the the shader constant buffers
        // Option is true if we have attempted to generate the shader.
        // This does not mean we were successul, however.
        Option<QDemonScopedRefCounted<SDefaultMaterialRenderableDepthShader>>
            m_DefaultMaterialDepthPrepassShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_DepthPrepassShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_DepthPrepassShaderDisplaced;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_DepthTessLinearPrepassShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>>
            m_DepthTessLinearPrepassShaderDisplaced;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_DepthTessPhongPrepassShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_DepthTessNPatchPrepassShader;
        Option<QDemonScopedRefCounted<STextDepthShader>> m_TextDepthPrepassShader;
        Option<QDemonScopedRefCounted<SDefaultAoPassShader>> m_DefaultAoPassShader;
        Option<QDemonScopedRefCounted<SDefaultAoPassShader>> m_FakeDepthShader;
        Option<QDemonScopedRefCounted<SDefaultAoPassShader>> m_FakeCubemapDepthShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_ParaboloidDepthShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessLinearShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessPhongShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_ParaboloidDepthTessNPatchShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_CubemapDepthShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_CubemapDepthTessLinearShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_CubemapDepthTessPhongShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_CubemapDepthTessNPatchShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>> m_OrthographicDepthShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>>
            m_OrthographicDepthTessLinearShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>>
            m_OrthographicDepthTessPhongShader;
        Option<QDemonScopedRefCounted<SRenderableDepthPrepassShader>>
            m_OrthographicDepthTessNPatchShader;
        Option<QDemonScopedRefCounted<SShadowmapPreblurShader>> m_CubeShadowBlurXShader;
        Option<QDemonScopedRefCounted<SShadowmapPreblurShader>> m_CubeShadowBlurYShader;
        Option<QDemonScopedRefCounted<SShadowmapPreblurShader>> m_OrthoShadowBlurXShader;
        Option<QDemonScopedRefCounted<SShadowmapPreblurShader>> m_OrthoShadowBlurYShader;

#ifdef ADVANCED_BLEND_SW_FALLBACK
        Option<QDemonScopedRefCounted<SAdvancedModeBlendShader>> m_AdvancedModeOverlayBlendShader;
        Option<QDemonScopedRefCounted<SAdvancedModeBlendShader>> m_AdvancedModeColorBurnBlendShader;
        Option<QDemonScopedRefCounted<SAdvancedModeBlendShader>> m_AdvancedModeColorDodgeBlendShader;
#endif
        // Text shaders may be generated on demand.
        STextShaderPtr m_TextShader;
        STextShaderPtr m_TextPathShader;
        STextShaderPtr m_TextWidgetShader;
        STextShaderPtr m_TextOnscreenShader;

        // Overlay used to render all widgets.
        QDemonRenderRect m_BeginFrameViewport;
        QDemonScopedRefCounted<QDemonRenderTexture2D> m_WidgetTexture;
        QDemonScopedRefCounted<QDemonRenderFrameBuffer> m_WidgetFBO;

#ifdef ADVANCED_BLEND_SW_FALLBACK
        // Advanced blend mode SW fallback
        CResourceTexture2D m_LayerBlendTexture;
        QDemonScopedRefCounted<QDemonRenderFrameBuffer> m_BlendFB;
#endif
        // Allocator for temporary data that is cleared after every layer.
        TInstanceRenderMap m_InstanceRenderMap;
        TLayerRenderList m_LastFrameLayers;
        volatile qint32 mRefCount;

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
        Qt3DSRendererImpl(IQt3DSRenderContext &ctx);
        virtual ~Qt3DSRendererImpl();
        SShaderDefaultMaterialKeyProperties &DefaultMaterialShaderKeyProperties()
        {
            return m_DefaultMaterialShaderKeyProperties;
        }

        // QDemonRefCounted
        void addRef() override;
        void release() override;

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
        Option<SCuboidRect> GetCameraBounds(const SGraphObject &inObject) override;
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
        Qt3DSRenderPickResult Pick(SLayer &inLayer, const QVector2D &inViewportDimensions,
                                   const QVector2D &inMouseCoords, bool inPickSiblings,
                                   bool inPickEverything,
                                   const SRenderInstanceId id) override;

        virtual Option<QVector2D>
        FacePosition(SNode &inNode, NVBounds3 inBounds, const QMatrix4x4 &inGlobalTransform,
                     const QVector2D &inViewportDimensions, const QVector2D &inMouseCoords,
                     QDemonDataRef<SGraphObject *> inMapperObjects, SBasisPlanes::Enum inPlane) override;

        virtual Qt3DSRenderPickResult PickOffscreenLayer(SLayer &inLayer,
                                                        const QVector2D &inViewportDimensions,
                                                        const QVector2D &inMouseCoords,
                                                        bool inPickEverything);

        QVector3D UnprojectToPosition(SNode &inNode, QVector3D &inPosition,
                                           const QVector2D &inMouseVec) const override;
        QVector3D UnprojectWithDepth(SNode &inNode, QVector3D &inPosition,
                                          const QVector3D &inMouseVec) const override;
        QVector3D ProjectPosition(SNode &inNode, const QVector3D &inPosition) const override;

        Option<SLayerPickSetup> GetLayerPickSetup(SLayer &inLayer,
                                                          const QVector2D &inMouseCoords,
                                                          const QSize &inPickDims) override;

        Option<QDemonRenderRectF> GetLayerRect(SLayer &inLayer) override;

        void RunLayerRender(SLayer &inLayer, const QMatrix4x4 &inViewProjection) override;

        // Note that this allocator is completely reset on BeginFrame.
        NVAllocatorCallback &GetPerFrameAllocator() override
        {
            return m_qt3dsContext.GetPerFrameAllocator();
        }
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
        void RenderText2D(float x, float y, Option<QVector3D> inColor,
                                  const char *text) override;
        bool PrepareTextureAtlasForRender();

        // render Gpu profiler values
        void RenderGpuProfilerStats(float x, float y,
                                            Option<QVector3D> inColor) override;

        // Callback during the layer render process.
        void LayerNeedsFrameClear(SLayerRenderData &inLayer);
        void BeginLayerDepthPassRender(SLayerRenderData &inLayer);
        void EndLayerDepthPassRender();
        void BeginLayerRender(SLayerRenderData &inLayer);
        void EndLayerRender();
        void PrepareImageForIbl(SImage &inImage);

        QDemonRenderShaderProgram *CompileShader(CRegisteredString inName, const char8_t *inVert,
                                             const char8_t *inFrame);

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
        eastl::pair<QDemonRenderVertexBuffer *, QDemonRenderIndexBuffer *> GetXYQuad();
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

        IQt3DSRenderContext &GetQt3DSContext() { return m_qt3dsContext; }

        void DrawScreenRect(QDemonRenderRectF inRect, const QVector3D &inColor);
        // Binds an offscreen texture.  Widgets are rendered last.
        void SetupWidgetLayer();

#ifdef ADVANCED_BLEND_SW_FALLBACK
        QDemonScopedRefCounted<QDemonRenderTexture2D> GetLayerBlendTexture()
        {
            return m_LayerBlendTexture.GetTexture();
        }

        QDemonScopedRefCounted<QDemonRenderFrameBuffer> GetBlendFB()
        {
            return m_BlendFB;
        }
#endif
        // widget context implementation
        virtual QDemonRenderVertexBuffer &
        GetOrCreateVertexBuffer(CRegisteredString &inStr, quint32 stride,
                                QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) override;
        virtual QDemonRenderIndexBuffer &
        GetOrCreateIndexBuffer(CRegisteredString &inStr,
                               QDemonRenderComponentTypes::Enum componentType, size_t size,
                               QDemonConstDataRef<quint8> bufferData = QDemonConstDataRef<quint8>()) override;
        virtual QDemonRenderAttribLayout &
        CreateAttributeLayout(QDemonConstDataRef<QDemonRenderVertexBufferEntry> attribs) override;
        virtual QDemonRenderInputAssembler &
        GetOrCreateInputAssembler(CRegisteredString &inStr, QDemonRenderAttribLayout *attribLayout,
                                  QDemonConstDataRef<QDemonRenderVertexBuffer *> buffers,
                                  const QDemonRenderIndexBuffer *indexBuffer,
                                  QDemonConstDataRef<quint32> strides, QDemonConstDataRef<quint32> offsets) override;

        QDemonRenderVertexBuffer *GetVertexBuffer(CRegisteredString &inStr) override;
        QDemonRenderIndexBuffer *GetIndexBuffer(CRegisteredString &inStr) override;
        QDemonRenderInputAssembler *GetInputAssembler(CRegisteredString &inStr) override;

        QDemonRenderShaderProgram *GetShader(CRegisteredString inStr) override;
        QDemonRenderShaderProgram *CompileAndStoreShader(CRegisteredString inStr) override;
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

        Option<QVector2D> GetLayerMouseCoords(SLayer &inLayer, const QVector2D &inMouseCoords,
                                                   const QVector2D &inViewportDimensions,
                                                   bool forceImageIntersect = false) const override;

    protected:
        Option<QVector2D> GetLayerMouseCoords(SLayerRenderData &inLayer, const QVector2D &inMouseCoords,
                                           const QVector2D &inViewportDimensions,
                                           bool forceImageIntersect = false) const;
        SPickResultProcessResult ProcessPickResultList(bool inPickEverything);
        // If the mouse y coordinates need to be flipped we expect that to happen before entry into
        // this function
        void GetLayerHitObjectList(SLayerRenderData &inLayer, const QVector2D &inViewportDimensions,
                                   const QVector2D &inMouseCoords, bool inPickEverything,
                                   TPickResultArray &outIntersectionResult,
                                   NVAllocatorCallback &inTempAllocator);
        void IntersectRayWithSubsetRenderable(const SRay &inRay,
                                              SRenderableObject &inRenderableObject,
                                              TPickResultArray &outIntersectionResultList,
                                              NVAllocatorCallback &inTempAllocator);
    };
}
}

#endif
