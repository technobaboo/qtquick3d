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
#include <QtDemonRuntimeRender/qdemonrendermesh.h>
#include <QtDemonRuntimeRender/qdemonrendershaderkeys.h>
#include <QtDemonRuntimeRender/qdemonrendershadercache.h>
#include <QtDemonRuntimeRender/qdemonrenderableimage.h>
#include <QtDemonRuntimeRender/qdemonrenderpath.h>

#include <QtDemon/qdemoninvasivelinkedlist.h>

QT_BEGIN_NAMESPACE

enum class QDemonRenderableObjectFlag
{
    HasTransparency = 1 << 0,
    CompletelyTransparent = 1 << 1,
    Dirty = 1 << 2,
    Pickable = 1 << 3,
    DefaultMaterialMeshSubset = 1 << 4,
    Custom = 1 << 6,
    CustomMaterialMeshSubset = 1 << 7,
    HasRefraction = 1 << 8,
    Path = 1 << 9,
};

struct QDemonRenderableObjectFlags : public QFlags<QDemonRenderableObjectFlag>
{
    void setHasTransparency(bool inHasTransparency)
    {
        setFlag(QDemonRenderableObjectFlag::HasTransparency, inHasTransparency);
    }
    bool hasTransparency() const { return this->operator&(QDemonRenderableObjectFlag::HasTransparency); }
    bool hasRefraction() const { return this->operator&(QDemonRenderableObjectFlag::HasRefraction); }
    void setCompletelyTransparent(bool inTransparent)
    {
        setFlag(QDemonRenderableObjectFlag::CompletelyTransparent, inTransparent);
    }
    bool isCompletelyTransparent() const
    {
        return this->operator&(QDemonRenderableObjectFlag::CompletelyTransparent);
    }
    void setDirty(bool inDirty) { setFlag(QDemonRenderableObjectFlag::Dirty, inDirty); }
    bool isDirty() const { return this->operator&(QDemonRenderableObjectFlag::Dirty); }
    void setPickable(bool inPickable) { setFlag(QDemonRenderableObjectFlag::Pickable, inPickable); }
    bool isPickable() const { return this->operator&(QDemonRenderableObjectFlag::Pickable); }

    // Mutually exclusive values
    void setDefaultMaterialMeshSubset(bool inMeshSubset)
    {
        setFlag(QDemonRenderableObjectFlag::DefaultMaterialMeshSubset, inMeshSubset);
    }
    bool isDefaultMaterialMeshSubset() const
    {
        return this->operator&(QDemonRenderableObjectFlag::DefaultMaterialMeshSubset);
    }

    void setCustomMaterialMeshSubset(bool inMeshSubset)
    {
        setFlag(QDemonRenderableObjectFlag::CustomMaterialMeshSubset, inMeshSubset);
    }
    bool isCustomMaterialMeshSubset() const
    {
        return this->operator&(QDemonRenderableObjectFlag::CustomMaterialMeshSubset);
    }

    void setCustom(bool inCustom) { setFlag(QDemonRenderableObjectFlag::Custom, inCustom); }
    bool isCustom() const { return this->operator&(QDemonRenderableObjectFlag::Custom); }

    void setPath(bool inPath) { setFlag(QDemonRenderableObjectFlag::Path, inPath); }
    bool isPath() const { return this->operator&(QDemonRenderableObjectFlag::Path); }
};

struct QDemonNodeLightEntry
{
    QDemonRenderLight *light = nullptr;
    quint32 lightIndex;
    QDemonNodeLightEntry *nextNode = nullptr;
    QDemonNodeLightEntry() = default;
    QDemonNodeLightEntry(QDemonRenderLight *inLight, quint32 inLightIndex)
        : light(inLight), lightIndex(inLightIndex), nextNode(nullptr)
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
    TessModeValues tessellationMode;
    // For custom renderable objects the render function must be defined
    TRenderFunction renderFunction;
    QDemonNodeLightEntryList scopedLights;
    QDemonRenderableObject(QDemonRenderableObjectFlags inFlags,
                           const QVector3D &inWorldCenterPt,
                           const QMatrix4x4 &inGlobalTransform,
                           const QDemonBounds3 &inBounds,
                           TessModeValues inTessMode = TessModeValues::NoTess,
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
    bool operator<(QDemonRenderableObject *inOther) const { return cameraDistanceSq < inOther->cameraDistanceSq; }
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonRenderableObject>::value);

// Different subsets from the same model will get the same
// model context so we can generate the MVP and normal matrix once
// and only once per subset.
struct QDemonModelContext
{
    const QDemonRenderModel &model;
    QMatrix4x4 modelViewProjection;
    QMatrix3x3 normalMatrix;

    QDemonModelContext(const QDemonRenderModel &inModel, const QMatrix4x4 &inViewProjection) : model(inModel)
    {
        model.calculateMVPAndNormalMatrix(inViewProjection, modelViewProjection, normalMatrix);
    }
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonModelContext>::value);

class QDemonRendererImpl;
struct QDemonLayerRenderData;
struct QDemonShadowMapEntry;

struct QDemonSubsetRenderableBase : public QDemonRenderableObject
{
    const QDemonRef<QDemonRendererImpl> &generator;
    const QDemonModelContext &modelContext;
    const QDemonRenderSubset &subset;
    float opacity;

    QDemonSubsetRenderableBase(QDemonRenderableObjectFlags inFlags,
                               const QVector3D &inWorldCenterPt,
                               const QDemonRef<QDemonRendererImpl> &gen,
                               const QDemonRenderSubset &inSubset,
                               const QDemonModelContext &inModelContext,
                               float inOpacity);
    void renderShadowMapPass(const QVector2D &inCameraVec,
                             const QDemonRenderLight *inLight,
                             const QDemonRenderCamera &inCamera,
                             QDemonShadowMapEntry *inShadowMapEntry) const;

    void renderDepthPass(const QVector2D &inCameraVec, QDemonRenderableImage *inDisplacementImage, float inDisplacementAmount);
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonSubsetRenderableBase>::value);

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
                           const QVector3D &inWorldCenterPt,
                           const QDemonRef<QDemonRendererImpl> &gen,
                           const QDemonRenderSubset &inSubset,
                           const QDemonRenderDefaultMaterial &mat,
                           const QDemonModelContext &inModelContext,
                           float inOpacity,
                           QDemonRenderableImage *inFirstImage,
                           QDemonShaderDefaultMaterialKey inShaderKey,
                           const QDemonConstDataRef<QMatrix4x4> &inBoneGlobals);

    void render(const QVector2D &inCameraVec, const TShaderFeatureSet &inFeatureSet);

    void renderDepthPass(const QVector2D &inCameraVec);

    QDemonRenderDefaultMaterial::MaterialBlendMode getBlendingMode() { return material.blendMode; }
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonSubsetRenderable>::value);

struct QDemonCustomMaterialRenderable : public QDemonSubsetRenderableBase
{
    const QDemonRenderCustomMaterial &material;
    QDemonRenderableImage *firstImage;
    QDemonShaderDefaultMaterialKey shaderDescription;

    QDemonCustomMaterialRenderable(QDemonRenderableObjectFlags inFlags,
                                   const QVector3D &inWorldCenterPt,
                                   const QDemonRef<QDemonRendererImpl> &gen,
                                   const QDemonRenderSubset &inSubset,
                                   const QDemonRenderCustomMaterial &mat,
                                   const QDemonModelContext &inModelContext,
                                   float inOpacity,
                                   QDemonRenderableImage *inFirstImage,
                                   QDemonShaderDefaultMaterialKey inShaderKey);

    void render(const QVector2D &inCameraVec,
                const QDemonLayerRenderData &inLayerData,
                const QDemonRenderLayer &inLayer,
                const QVector<QDemonRenderLight *> &inLights,
                const QDemonRenderCamera &inCamera,
                const QDemonRef<QDemonRenderTexture2D> inDepthTexture,
                const QDemonRef<QDemonRenderTexture2D> inSsaoTexture,
                const TShaderFeatureSet &inFeatureSet);

    void renderDepthPass(const QVector2D &inCameraVec,
                         const QDemonRenderLayer &inLayer,
                         const QVector<QDemonRenderLight *> inLights,
                         const QDemonRenderCamera &inCamera,
                         const QDemonRenderTexture2D *inDepthTexture);
};

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonCustomMaterialRenderable>::value);

struct QDemonPathRenderable : public QDemonRenderableObject
{
    const QDemonRef<QDemonRendererImpl> &m_generator;
    QDemonRenderPath &m_path;
    QMatrix4x4 m_mvp;
    QMatrix3x3 m_normalMatrix;
    const QDemonRenderGraphObject &m_material;
    float m_opacity;
    QDemonRenderableImage *m_firstImage;
    QDemonShaderDefaultMaterialKey m_shaderDescription;
    bool m_isStroke;

    QDemonPathRenderable(QDemonRenderableObjectFlags inFlags,
                         const QVector3D &inWorldCenterPt,
                         const QDemonRef<QDemonRendererImpl> &gen,
                         const QMatrix4x4 &inGlobalTransform,
                         QDemonBounds3 &inBounds,
                         QDemonRenderPath &inPath,
                         const QMatrix4x4 &inModelViewProjection,
                         const QMatrix3x3 inNormalMat,
                         const QDemonRenderGraphObject &inMaterial,
                         float inOpacity,
                         QDemonShaderDefaultMaterialKey inShaderKey,
                         bool inIsStroke);
    void render(const QVector2D &inCameraVec,
                const QDemonRenderLayer &inLayer,
                const QVector<QDemonRenderLight *> &inLights,
                const QDemonRenderCamera &inCamera,
                const QDemonRef<QDemonRenderTexture2D> &inDepthTexture,
                const QDemonRef<QDemonRenderTexture2D> &inSsaoTexture,
                const TShaderFeatureSet &inFeatureSet);

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

Q_STATIC_ASSERT(std::is_trivially_destructible<QDemonPathRenderable>::value);

QT_END_NAMESPACE

#endif
