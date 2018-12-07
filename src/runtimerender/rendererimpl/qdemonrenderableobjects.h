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
#ifndef QDEMON_RENDER_IMPL_RENDERABLE_OBJECTS_H
#define QDEMON_RENDER_IMPL_RENDERABLE_OBJECTS_H

#include <QtDemonRuntimeRender/qdemonrendermodel.h>
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>
#include <QtDemonRuntimeRender/qdemonrendercustommaterial.h>
#include <QtDemonRuntimeRender/qdemonrendertext.h>
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderableimage.h>

#include <QtDemon/qdemoninvasivelinkedlist.h>

QT_BEGIN_NAMESPACE

struct RenderPreparationResultFlagValues
{
    enum Enum {
        HasTransparency = 1 << 0,
        CompletelyTransparent = 1 << 1,
        Dirty = 1 << 2,
        Pickable = 1 << 3,
        DefaultMaterialMeshSubset = 1 << 4,
        Text = 1 << 5,
        Custom = 1 << 6,
        CustomMaterialMeshSubset = 1 << 7,
        HasRefraction = 1 << 8,
        Path = 1 << 9,
    };
};

struct SRenderableObjectFlags : public QDemonFlags<RenderPreparationResultFlagValues::Enum, quint32>
{
    void ClearOrSet(bool value, RenderPreparationResultFlagValues::Enum enumVal)
    {
        if (value)
            this->operator|=(enumVal);
        else
            clear(enumVal);
    }

    void SetHasTransparency(bool inHasTransparency)
    {
        ClearOrSet(inHasTransparency, RenderPreparationResultFlagValues::HasTransparency);
    }
    bool HasTransparency() const
    {
        return this->operator&(RenderPreparationResultFlagValues::HasTransparency);
    }
    bool HasRefraction() const
    {
        return this->operator&(RenderPreparationResultFlagValues::HasRefraction);
    }
    void SetCompletelyTransparent(bool inTransparent)
    {
        ClearOrSet(inTransparent, RenderPreparationResultFlagValues::CompletelyTransparent);
    }
    bool IsCompletelyTransparent() const
    {
        return this->operator&(RenderPreparationResultFlagValues::CompletelyTransparent);
    }
    void SetDirty(bool inDirty)
    {
        ClearOrSet(inDirty, RenderPreparationResultFlagValues::Dirty);
    }
    bool IsDirty() const { return this->operator&(RenderPreparationResultFlagValues::Dirty); }
    void SetPickable(bool inPickable)
    {
        ClearOrSet(inPickable, RenderPreparationResultFlagValues::Pickable);
    }
    bool GetPickable() const
    {
        return this->operator&(RenderPreparationResultFlagValues::Pickable);
    }

    // Mutually exclusive values
    void SetDefaultMaterialMeshSubset(bool inMeshSubset)
    {
        ClearOrSet(inMeshSubset, RenderPreparationResultFlagValues::DefaultMaterialMeshSubset);
    }
    bool IsDefaultMaterialMeshSubset() const
    {
        return this->operator&(RenderPreparationResultFlagValues::DefaultMaterialMeshSubset);
    }

    void SetCustomMaterialMeshSubset(bool inMeshSubset)
    {
        ClearOrSet(inMeshSubset, RenderPreparationResultFlagValues::CustomMaterialMeshSubset);
    }
    bool IsCustomMaterialMeshSubset() const
    {
        return this->operator&(RenderPreparationResultFlagValues::CustomMaterialMeshSubset);
    }

    void SetText(bool inText) { ClearOrSet(inText, RenderPreparationResultFlagValues::Text); }
    bool IsText() const { return this->operator&(RenderPreparationResultFlagValues::Text); }

    void SetCustom(bool inCustom)
    {
        ClearOrSet(inCustom, RenderPreparationResultFlagValues::Custom);
    }
    bool IsCustom() const { return this->operator&(RenderPreparationResultFlagValues::Custom); }

    void SetPath(bool inPath) { ClearOrSet(inPath, RenderPreparationResultFlagValues::Path); }
    bool IsPath() const { return this->operator&(RenderPreparationResultFlagValues::Path); }
};

struct SNodeLightEntry
{
    SLight *m_Light;
    quint32 m_LightIndex;
    SNodeLightEntry *m_NextNode;
    SNodeLightEntry()
        : m_Light(nullptr)
        , m_NextNode(nullptr)
    {
    }
    SNodeLightEntry(SLight *inLight, quint32 inLightIndex)
        : m_Light(inLight)
        , m_LightIndex(inLightIndex)
        , m_NextNode(nullptr)
    {
    }
};

DEFINE_INVASIVE_SINGLE_LIST(NodeLightEntry);

IMPLEMENT_INVASIVE_SINGLE_LIST(NodeLightEntry, m_NextNode);

struct SRenderableObject;

typedef void (*TRenderFunction)(SRenderableObject &inObject, const QVector2D &inCameraProperties);

struct SRenderableObject
{
    // Variables used for picking
    const QMatrix4x4 &m_GlobalTransform;
    const QDemonBounds3 &m_Bounds;
    SRenderableObjectFlags m_RenderableFlags;
    // For rough sorting for transparency and for depth
    QVector3D m_WorldCenterPoint;
    float m_CameraDistanceSq;
    TessModeValues::Enum m_TessellationMode;
    // For custom renderable objects the render function must be defined
    TRenderFunction m_RenderFunction;
    TNodeLightEntryList m_ScopedLights;
    SRenderableObject(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                      const QMatrix4x4 &inGlobalTransform, const QDemonBounds3 &inBounds,
                      TessModeValues::Enum inTessMode = TessModeValues::NoTess,
                      TRenderFunction inFunction = nullptr)

        : m_GlobalTransform(inGlobalTransform)
        , m_Bounds(inBounds)
        , m_RenderableFlags(inFlags)
        , m_WorldCenterPoint(inWorldCenterPt)
        , m_CameraDistanceSq(0)
        , m_TessellationMode(inTessMode)
        , m_RenderFunction(inFunction)
    {
    }
    bool operator<(SRenderableObject *inOther) const
    {
        return m_CameraDistanceSq < inOther->m_CameraDistanceSq;
    }
};

typedef QVector<SRenderableObject *> TRenderableObjectList;

// Different subsets from the same model will get the same
// model context so we can generate the MVP and normal matrix once
// and only once per subset.
struct SModelContext
{
    const SModel &m_Model;
    QMatrix4x4 m_ModelViewProjection;
    QMatrix3x3 m_NormalMatrix;

    SModelContext(const SModel &inModel, const QMatrix4x4 &inViewProjection)
        : m_Model(inModel)
    {
        m_Model.CalculateMVPAndNormalMatrix(inViewProjection, m_ModelViewProjection,
                                            m_NormalMatrix);
    }
    SModelContext(const SModelContext &inOther)
        : m_Model(inOther.m_Model)
    {
        // The default copy constructor for these objects is pretty darn slow.
        ::memcpy(&m_ModelViewProjection, &inOther.m_ModelViewProjection,
                sizeof(m_ModelViewProjection));
        ::memcpy(&m_NormalMatrix, &inOther.m_NormalMatrix, sizeof(m_NormalMatrix));
    }
};

typedef QVector<SModelContext *> TModelContextPtrList;

class Qt3DSRendererImpl;
struct SLayerRenderData;
struct SShadowMapEntry;

struct SSubsetRenderableBase : public SRenderableObject
{
    Qt3DSRendererImpl &m_Generator;
    const SModelContext &m_ModelContext;
    SRenderSubset m_Subset;
    float m_Opacity;

    SSubsetRenderableBase(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                          Qt3DSRendererImpl &gen, const SRenderSubset &subset,
                          const SModelContext &modelContext, float inOpacity)

        : SRenderableObject(inFlags, inWorldCenterPt, modelContext.m_Model.m_GlobalTransform,
                            m_Subset.m_Bounds)
        , m_Generator(gen)
        , m_ModelContext(modelContext)
        , m_Subset(subset)
        , m_Opacity(inOpacity)
    {
    }
    void RenderShadowMapPass(const QVector2D &inCameraVec, const SLight *inLight,
                             const SCamera &inCamera, SShadowMapEntry *inShadowMapEntry);

    void RenderDepthPass(const QVector2D &inCameraVec, SRenderableImage *inDisplacementImage,
                         float inDisplacementAmount);
};

/**
     *	A renderable that corresponds to a subset (a part of a model).
     *	These are created per subset per layer and are responsible for actually
     *	rendering this type of object.
     */
struct SSubsetRenderable : public SSubsetRenderableBase
{
    const SDefaultMaterial &m_Material;
    SRenderableImage *m_FirstImage;
    SShaderDefaultMaterialKey m_ShaderDescription;
    QDemonConstDataRef<QMatrix4x4> m_Bones;

    SSubsetRenderable(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                      Qt3DSRendererImpl &gen, const SRenderSubset &subset,
                      const SDefaultMaterial &mat, const SModelContext &modelContext,
                      float inOpacity, SRenderableImage *inFirstImage,
                      SShaderDefaultMaterialKey inShaderKey,
                      QDemonConstDataRef<QMatrix4x4> inBoneGlobals)

        : SSubsetRenderableBase(inFlags, inWorldCenterPt, gen, subset, modelContext, inOpacity)
        , m_Material(mat)
        , m_FirstImage(inFirstImage)
        , m_ShaderDescription(inShaderKey)
        , m_Bones(inBoneGlobals)
    {
        m_RenderableFlags.SetDefaultMaterialMeshSubset(true);
        m_RenderableFlags.SetCustom(false);
        m_RenderableFlags.SetText(false);
    }

    void Render(const QVector2D &inCameraVec, TShaderFeatureSet inFeatureSet);

    void RenderDepthPass(const QVector2D &inCameraVec);

    DefaultMaterialBlendMode::Enum getBlendingMode()
    {
        return m_Material.m_BlendMode;
    }
};

struct SCustomMaterialRenderable : public SSubsetRenderableBase
{
    const SCustomMaterial &m_Material;
    SRenderableImage *m_FirstImage;
    SShaderDefaultMaterialKey m_ShaderDescription;

    SCustomMaterialRenderable(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                              Qt3DSRendererImpl &gen, const SRenderSubset &subset,
                              const SCustomMaterial &mat, const SModelContext &modelContext,
                              float inOpacity, SRenderableImage *inFirstImage,
                              SShaderDefaultMaterialKey inShaderKey)
        : SSubsetRenderableBase(inFlags, inWorldCenterPt, gen, subset, modelContext, inOpacity)
        , m_Material(mat)
        , m_FirstImage(inFirstImage)
        , m_ShaderDescription(inShaderKey)
    {
        m_RenderableFlags.SetCustomMaterialMeshSubset(true);
    }

    void Render(const QVector2D &inCameraVec, const SLayerRenderData &inLayerData,
                const SLayer &inLayer, QDemonDataRef<SLight *> inLights, const SCamera &inCamera,
                const QDemonRenderTexture2D *inDepthTexture, const QDemonRenderTexture2D *inSsaoTexture,
                TShaderFeatureSet inFeatureSet);

    void RenderDepthPass(const QVector2D &inCameraVec, const SLayer &inLayer,
                         QDemonConstDataRef<SLight *> inLights, const SCamera &inCamera,
                         const QDemonRenderTexture2D *inDepthTexture);
};

struct STextScaleAndOffset
{
    QVector2D m_TextOffset;
    QVector2D m_TextScale;
    STextScaleAndOffset(const QVector2D &inTextOffset, const QVector2D &inTextScale)
        : m_TextOffset(inTextOffset)
        , m_TextScale(inTextScale)
    {
    }
    STextScaleAndOffset(QDemonRenderTexture2D &inTexture, const STextTextureDetails &inTextDetails,
                        const STextRenderInfo &inInfo);
};

struct STextRenderable : public SRenderableObject, public STextScaleAndOffset
{
    Qt3DSRendererImpl &m_Generator;
    const SText &m_Text;
    QDemonRenderTexture2D &m_Texture;
    QMatrix4x4 m_ModelViewProjection;
    QMatrix4x4 m_ViewProjection;

    STextRenderable(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                    Qt3DSRendererImpl &gen, const SText &inText, const QDemonBounds3 &inBounds,
                    const QMatrix4x4 &inModelViewProjection, const QMatrix4x4 &inViewProjection,
                    QDemonRenderTexture2D &inTextTexture, const QVector2D &inTextOffset,
                    const QVector2D &inTextScale)
        : SRenderableObject(inFlags, inWorldCenterPt, inText.m_GlobalTransform, inBounds)
        , STextScaleAndOffset(inTextOffset, inTextScale)
        , m_Generator(gen)
        , m_Text(inText)
        , m_Texture(inTextTexture)
        , m_ModelViewProjection(inModelViewProjection)
        , m_ViewProjection(inViewProjection)
    {
        m_RenderableFlags.SetDefaultMaterialMeshSubset(false);
        m_RenderableFlags.SetCustom(false);
        m_RenderableFlags.SetText(true);
    }

    void Render(const QVector2D &inCameraVec);
    void RenderDepthPass(const QVector2D &inCameraVec);
};

struct SPathRenderable : public SRenderableObject
{
    Qt3DSRendererImpl &m_Generator;
    SPath &m_Path;
    QDemonBounds3 m_Bounds;
    QMatrix4x4 m_ModelViewProjection;
    QMatrix3x3 m_NormalMatrix;
    const SGraphObject &m_Material;
    float m_Opacity;
    SRenderableImage *m_FirstImage;
    SShaderDefaultMaterialKey m_ShaderDescription;
    bool m_IsStroke;

    SPathRenderable(SRenderableObjectFlags inFlags, QVector3D inWorldCenterPt,
                    Qt3DSRendererImpl &gen, const QMatrix4x4 &inGlobalTransform,
                    QDemonBounds3 &inBounds, SPath &inPath, const QMatrix4x4 &inModelViewProjection,
                    const QMatrix3x3 inNormalMat, const SGraphObject &inMaterial, float inOpacity,
                    SShaderDefaultMaterialKey inShaderKey, bool inIsStroke)

        : SRenderableObject(inFlags, inWorldCenterPt, inGlobalTransform, m_Bounds)
        , m_Generator(gen)
        , m_Path(inPath)
        , m_Bounds(inBounds)
        , m_ModelViewProjection(inModelViewProjection)
        , m_NormalMatrix(inNormalMat)
        , m_Material(inMaterial)
        , m_Opacity(inOpacity)
        , m_FirstImage(nullptr)
        , m_ShaderDescription(inShaderKey)
        , m_IsStroke(inIsStroke)
    {
        m_RenderableFlags.SetPath(true);
    }
    void Render(const QVector2D &inCameraVec, const SLayer &inLayer,
                QDemonConstDataRef<SLight *> inLights, const SCamera &inCamera,
                const QDemonRenderTexture2D *inDepthTexture, const QDemonRenderTexture2D *inSsaoTexture,
                TShaderFeatureSet inFeatureSet);

    void RenderDepthPass(const QVector2D &inCameraVec, const SLayer &inLayer,
                         QDemonConstDataRef<SLight *> inLights, const SCamera &inCamera,
                         const QDemonRenderTexture2D *inDepthTexture);

    void RenderShadowMapPass(const QVector2D &inCameraVec, const SLight *inLight,
                             const SCamera &inCamera, SShadowMapEntry *inShadowMapEntry);
};
QT_END_NAMESPACE

#endif
