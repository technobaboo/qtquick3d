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
#include <QtDemonRuntimeRender/qdemonrenderpath.h>

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

struct QDemonRenderableObjectFlags : public QDemonFlags<RenderPreparationResultFlagValues::Enum, quint32>
{
    void clearOrSet(bool value, RenderPreparationResultFlagValues::Enum enumVal)
    {
        if (value)
            this->operator|=(enumVal);
        else
            clear(enumVal);
    }

    void setHasTransparency(bool inHasTransparency)
    {
        clearOrSet(inHasTransparency, RenderPreparationResultFlagValues::HasTransparency);
    }
    bool hasTransparency() const
    {
        return this->operator&(RenderPreparationResultFlagValues::HasTransparency);
    }
    bool hasRefraction() const
    {
        return this->operator&(RenderPreparationResultFlagValues::HasRefraction);
    }
    void setCompletelyTransparent(bool inTransparent)
    {
        clearOrSet(inTransparent, RenderPreparationResultFlagValues::CompletelyTransparent);
    }
    bool isCompletelyTransparent() const
    {
        return this->operator&(RenderPreparationResultFlagValues::CompletelyTransparent);
    }
    void setDirty(bool inDirty)
    {
        clearOrSet(inDirty, RenderPreparationResultFlagValues::Dirty);
    }
    bool isDirty() const { return this->operator&(RenderPreparationResultFlagValues::Dirty); }
    void setPickable(bool inPickable)
    {
        clearOrSet(inPickable, RenderPreparationResultFlagValues::Pickable);
    }
    bool getPickable() const
    {
        return this->operator&(RenderPreparationResultFlagValues::Pickable);
    }

    // Mutually exclusive values
    void setDefaultMaterialMeshSubset(bool inMeshSubset)
    {
        clearOrSet(inMeshSubset, RenderPreparationResultFlagValues::DefaultMaterialMeshSubset);
    }
    bool isDefaultMaterialMeshSubset() const
    {
        return this->operator&(RenderPreparationResultFlagValues::DefaultMaterialMeshSubset);
    }

    void setCustomMaterialMeshSubset(bool inMeshSubset)
    {
        clearOrSet(inMeshSubset, RenderPreparationResultFlagValues::CustomMaterialMeshSubset);
    }
    bool isCustomMaterialMeshSubset() const
    {
        return this->operator&(RenderPreparationResultFlagValues::CustomMaterialMeshSubset);
    }

    void setText(bool inText) { clearOrSet(inText, RenderPreparationResultFlagValues::Text); }
    bool IsText() const { return this->operator&(RenderPreparationResultFlagValues::Text); }

    void setCustom(bool inCustom)
    {
        clearOrSet(inCustom, RenderPreparationResultFlagValues::Custom);
    }
    bool isCustom() const { return this->operator&(RenderPreparationResultFlagValues::Custom); }

    void setPath(bool inPath) { clearOrSet(inPath, RenderPreparationResultFlagValues::Path); }
    bool isPath() const { return this->operator&(RenderPreparationResultFlagValues::Path); }
};

struct QDemonNodeLightEntry
{
    QDemonRenderLight *light = nullptr;
    quint32 lightIndex;
    QDemonNodeLightEntry *nextNode = nullptr;
    QDemonNodeLightEntry() = default;
    QDemonNodeLightEntry(QDemonRenderLight *inLight, quint32 inLightIndex)
        : light(inLight)
        , lightIndex(inLightIndex)
        , nextNode(nullptr)
    {
    }
};

DEFINE_INVASIVE_SINGLE_LIST(QDemonNodeLightEntry)
IMPLEMENT_INVASIVE_SINGLE_LIST(QDemonNodeLightEntry, nextNode)

struct QDemonRenderableObject;

typedef void (*TRenderFunction)(QDemonRenderableObject &inObject, const QVector2D &inCameraProperties);

struct QDemonRenderableObject
{
    // Variables used for picking
    const QMatrix4x4 &globalTransform;
    const QDemonBounds3 &bounds;
    QDemonRenderableObjectFlags renderableFlags;
    // For rough sorting for transparency and for depth
    QVector3D worldCenterPoint;
    float cameraDistanceSq;
    TessModeValues::Enum tessellationMode;
    // For custom renderable objects the render function must be defined
    TRenderFunction renderFunction;
    QDemonNodeLightEntryList scopedLights;
    QDemonRenderableObject(QDemonRenderableObjectFlags inFlags,
                           QVector3D inWorldCenterPt,
                           const QMatrix4x4 &inGlobalTransform,
                           const QDemonBounds3 &inBounds,
                           TessModeValues::Enum inTessMode = TessModeValues::NoTess,
                           TRenderFunction inFunction = nullptr)

        : globalTransform(inGlobalTransform)
        , bounds(inBounds)
        , renderableFlags(inFlags)
        , worldCenterPoint(inWorldCenterPt)
        , cameraDistanceSq(0)
        , tessellationMode(inTessMode)
        , renderFunction(inFunction)
    {
    }
    bool operator<(QDemonRenderableObject *inOther) const
    {
        return cameraDistanceSq < inOther->cameraDistanceSq;
    }
};

typedef QVector<QDemonRenderableObject *> TRenderableObjectList;

// Different subsets from the same model will get the same
// model context so we can generate the MVP and normal matrix once
// and only once per subset.
struct QDemonModelContext
{
    const QDemonRenderModel &model;
    QMatrix4x4 modelViewProjection;
    QMatrix3x3 normalMatrix;

    QDemonModelContext(const QDemonRenderModel &inModel, const QMatrix4x4 &inViewProjection)
        : model(inModel)
    {
        model.calculateMVPAndNormalMatrix(inViewProjection,
                                          modelViewProjection,
                                          normalMatrix);
    }
    QDemonModelContext(const QDemonModelContext &inOther)
        : model(inOther.model)
    {
        // The default copy constructor for these objects is pretty darn slow.
        ::memcpy(&modelViewProjection, &inOther.modelViewProjection,
                sizeof(modelViewProjection));
        ::memcpy(&normalMatrix, &inOther.normalMatrix, sizeof(normalMatrix));
    }
};

typedef QVector<QDemonModelContext *> TModelContextPtrList;

class QDemonRendererImpl;
struct QDemonLayerRenderData;
struct QDemonShadowMapEntry;

struct QDemonSubsetRenderableBase : public QDemonRenderableObject
{
    QDemonRef<QDemonRendererImpl> generator;
    const QDemonModelContext &modelContext;
    QDemonRenderSubset subset;
    float opacity;

    QDemonSubsetRenderableBase(QDemonRenderableObjectFlags inFlags,
                               QVector3D inWorldCenterPt,
                               QDemonRef<QDemonRendererImpl> gen,
                               const QDemonRenderSubset &inSubset,
                               const QDemonModelContext &inModelContext,
                               float inOpacity)

        : QDemonRenderableObject(inFlags, inWorldCenterPt, inModelContext.model.globalTransform, inSubset.bounds)
        , generator(gen)
        , modelContext(inModelContext)
        , subset(inSubset)
        , opacity(inOpacity)
    {
    }
    void renderShadowMapPass(const QVector2D &inCameraVec,
                             const QDemonRenderLight *inLight,
                             const QDemonRenderCamera &inCamera,
                             QDemonShadowMapEntry *inShadowMapEntry);

    void renderDepthPass(const QVector2D &inCameraVec,
                         QDemonRenderableImage *inDisplacementImage,
                         float inDisplacementAmount);
};

/**
     *	A renderable that corresponds to a subset (a part of a model).
     *	These are created per subset per layer and are responsible for actually
     *	rendering this type of object.
     */
struct QDemonSubsetRenderable : public QDemonSubsetRenderableBase
{
    const QDemonRenderDefaultMaterial &material;
    QDemonRenderableImage *firstImage;
    QDemonShaderDefaultMaterialKey shaderDescription;
    QDemonConstDataRef<QMatrix4x4> bones;

    QDemonSubsetRenderable(QDemonRenderableObjectFlags inFlags,
                           QVector3D inWorldCenterPt,
                           QDemonRef<QDemonRendererImpl> gen,
                           const QDemonRenderSubset &inSubset,
                           const QDemonRenderDefaultMaterial &mat,
                           const QDemonModelContext &inModelContext,
                           float inOpacity,
                           QDemonRenderableImage *inFirstImage,
                           QDemonShaderDefaultMaterialKey inShaderKey,
                           QDemonConstDataRef<QMatrix4x4> inBoneGlobals)

        : QDemonSubsetRenderableBase(inFlags, inWorldCenterPt, gen, inSubset, inModelContext, inOpacity)
        , material(mat)
        , firstImage(inFirstImage)
        , shaderDescription(inShaderKey)
        , bones(inBoneGlobals)
    {
        renderableFlags.setDefaultMaterialMeshSubset(true);
        renderableFlags.setCustom(false);
        renderableFlags.setText(false);
    }

    void render(const QVector2D &inCameraVec, TShaderFeatureSet inFeatureSet);

    void renderDepthPass(const QVector2D &inCameraVec);

    DefaultMaterialBlendMode::Enum getBlendingMode()
    {
        return material.blendMode;
    }
};

struct QDemonCustomMaterialRenderable : public QDemonSubsetRenderableBase
{
    const QDemonRenderCustomMaterial &material;
    QDemonRenderableImage *firstImage;
    QDemonShaderDefaultMaterialKey shaderDescription;

    QDemonCustomMaterialRenderable(QDemonRenderableObjectFlags inFlags,
                                   QVector3D inWorldCenterPt,
                                   QDemonRef<QDemonRendererImpl> gen,
                                   const QDemonRenderSubset &inSubset,
                                   const QDemonRenderCustomMaterial &mat,
                                   const QDemonModelContext &inModelContext,
                                   float inOpacity,
                                   QDemonRenderableImage *inFirstImage,
                                   QDemonShaderDefaultMaterialKey inShaderKey)
        : QDemonSubsetRenderableBase(inFlags, inWorldCenterPt, gen, inSubset, inModelContext, inOpacity)
        , material(mat)
        , firstImage(inFirstImage)
        , shaderDescription(inShaderKey)
    {
        renderableFlags.setCustomMaterialMeshSubset(true);
    }

    void render(const QVector2D &inCameraVec,
                const QDemonLayerRenderData &inLayerData,
                const QDemonRenderLayer &inLayer,
                const QVector<QDemonRenderLight *> &inLights,
                const QDemonRenderCamera &inCamera,
                const QDemonRef<QDemonRenderTexture2D> inDepthTexture,
                const QDemonRef<QDemonRenderTexture2D> inSsaoTexture,
                TShaderFeatureSet inFeatureSet);

    void renderDepthPass(const QVector2D &inCameraVec,
                         const QDemonRenderLayer &inLayer,
                         const QVector<QDemonRenderLight *> inLights,
                         const QDemonRenderCamera &inCamera,
                         const QDemonRenderTexture2D *inDepthTexture);
};

struct QDemonTextScaleAndOffset
{
    QVector2D textOffset;
    QVector2D textScale;
    QDemonTextScaleAndOffset(const QVector2D &inTextOffset, const QVector2D &inTextScale)
        : textOffset(inTextOffset)
        , textScale(inTextScale)
    {
    }
    QDemonTextScaleAndOffset(QDemonRenderTexture2D &inTexture,
                             const QDemonTextTextureDetails &inTextDetails,
                             const QDemonTextRenderInfo &inInfo);
};

struct QDemonTextRenderable : public QDemonRenderableObject, public QDemonTextScaleAndOffset
{
    QDemonRendererImpl &generator;
    const QDemonText &text;
    QDemonRenderTexture2D &texture;
    QMatrix4x4 modelViewProjection;
    QMatrix4x4 viewProjection;

    QDemonTextRenderable(QDemonRenderableObjectFlags inFlags,
                         QVector3D inWorldCenterPt,
                         QDemonRendererImpl &gen,
                         const QDemonText &inText,
                         const QDemonBounds3 &inBounds,
                         const QMatrix4x4 &inModelViewProjection,
                         const QMatrix4x4 &inViewProjection,
                         QDemonRenderTexture2D &inTextTexture,
                         const QVector2D &inTextOffset,
                         const QVector2D &inTextScale)
        : QDemonRenderableObject(inFlags, inWorldCenterPt, inText.globalTransform, inBounds)
        , QDemonTextScaleAndOffset(inTextOffset, inTextScale)
        , generator(gen)
        , text(inText)
        , texture(inTextTexture)
        , modelViewProjection(inModelViewProjection)
        , viewProjection(inViewProjection)
    {
        renderableFlags.setDefaultMaterialMeshSubset(false);
        renderableFlags.setCustom(false);
        renderableFlags.setText(true);
    }

    void render(const QVector2D &inCameraVec);
    void renderDepthPass(const QVector2D &inCameraVec);
};

struct QDemonPathRenderable : public QDemonRenderableObject
{
    QDemonRef<QDemonRendererImpl> m_generator;
    QDemonPath &m_path;
    QDemonBounds3 bounds;
    QMatrix4x4 m_mvp;
    QMatrix3x3 m_normalMatrix;
    const QDemonGraphObject &m_material;
    float m_opacity;
    QDemonRenderableImage *m_firstImage;
    QDemonShaderDefaultMaterialKey m_shaderDescription;
    bool m_isStroke;

    QDemonPathRenderable(QDemonRenderableObjectFlags inFlags,
                         QVector3D inWorldCenterPt,
                         QDemonRef<QDemonRendererImpl> gen,
                         const QMatrix4x4 &inGlobalTransform,
                         QDemonBounds3 &inBounds,
                         QDemonPath &inPath,
                         const QMatrix4x4 &inModelViewProjection,
                         const QMatrix3x3 inNormalMat,
                         const QDemonGraphObject &inMaterial,
                         float inOpacity,
                         QDemonShaderDefaultMaterialKey inShaderKey,
                         bool inIsStroke)

        : QDemonRenderableObject(inFlags, inWorldCenterPt, inGlobalTransform, bounds)
        , m_generator(gen)
        , m_path(inPath)
        , bounds(inBounds)
        , m_mvp(inModelViewProjection)
        , m_normalMatrix(inNormalMat)
        , m_material(inMaterial)
        , m_opacity(inOpacity)
        , m_firstImage(nullptr)
        , m_shaderDescription(inShaderKey)
        , m_isStroke(inIsStroke)
    {
        renderableFlags.setPath(true);
    }
    void render(const QVector2D &inCameraVec,
                const QDemonRenderLayer &inLayer,
                const QVector<QDemonRenderLight *> &inLights,
                const QDemonRenderCamera &inCamera,
                const QDemonRef<QDemonRenderTexture2D> inDepthTexture,
                const QDemonRef<QDemonRenderTexture2D> inSsaoTexture,
                TShaderFeatureSet inFeatureSet);

    void renderDepthPass(const QVector2D &inCameraVec,
                         const QDemonRenderLayer &inLayer,
                         const QVector<QDemonRenderLight *> &inLights,
                         const QDemonRenderCamera &inCamera,
                         const QDemonRenderTexture2D *inDepthTexture);

    void renderShadowMapPass(const QVector2D &inCameraVec,
                             const QDemonRenderLight *inLight,
                             const QDemonRenderCamera &inCamera,
                             QDemonShadowMapEntry *inShadowMapEntry);
};
QT_END_NAMESPACE

#endif
