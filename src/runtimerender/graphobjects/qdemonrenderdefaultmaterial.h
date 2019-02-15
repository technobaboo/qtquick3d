/****************************************************************************
**
** Copyright (C) 2008-2015 NVIDIA Corporation.
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
#ifndef QDEMON_RENDER_DEFAULT_MATERIAL_H
#define QDEMON_RENDER_DEFAULT_MATERIAL_H

#include <QtDemonRuntimeRender/qdemonrendergraphobject.h>
#include <QtDemonRuntimeRender/qdemonrendermaterialdirty.h>
#include <QtDemonRuntimeRender/qdemonrenderlightmaps.h>

#include <QtDemon/qdemonflags.h>

#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

struct DefaultMaterialLighting
{
    enum Enum {
        NoLighting = 0,
        VertexLighting,
        FragmentLighting
    };
};
struct DefaultMaterialBlendMode
{
    enum Enum {
        Normal = 0,
        Screen,
        Multiply,
        Overlay,
        ColorBurn,
        ColorDodge
    };
};

struct DefaultMaterialSpecularModel
{
    enum Enum {
        Default = 0,
        KGGX,
        KWard
    };
};

struct QDemonRenderImage;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderDefaultMaterial : QDemonGraphObject
{
    QDemonMaterialDirty dirty;
    // lightmap section
    QDemonRenderLightmaps lightmaps;
    // material section
    QDemonRenderImage *iblProbe;
    DefaultMaterialLighting::Enum lighting; // defaults to vertex
    DefaultMaterialBlendMode::Enum blendMode; // defaults to normal
    QVector3D diffuseColor; // colors are 0-1 normalized
    QDemonRenderImage *diffuseMaps[3];
    float emissivePower; // 0-100, defaults to 0
    QDemonRenderImage *emissiveMap;
    QDemonRenderImage *emissiveMap2;
    QVector3D emissiveColor;
    QDemonRenderImage *specularReflection;
    QDemonRenderImage *specularMap;
    DefaultMaterialSpecularModel::Enum specularModel;
    QVector3D specularTint;
    float ior;
    float fresnelPower;
    float specularAmount; // 0-??, defaults to 0
    float specularRoughness; // 0-??, defaults to 50
    QDemonRenderImage *roughnessMap;
    float opacity; // 0-1
    QDemonRenderImage *opacityMap;
    QDemonRenderImage *bumpMap;
    float bumpAmount; // 0-??
    QDemonRenderImage *normalMap;
    QDemonRenderImage *displacementMap;
    float displaceAmount; // 0-??
    QDemonRenderImage *translucencyMap;
    float translucentFalloff; // 0 - ??
    float diffuseLightWrap; // 0 - 1
    bool vertexColors;
    // Materials are stored as a linked list on models.
    QDemonGraphObject *nextSibling;
    QDemonRenderModel *parent;

    QDemonRenderDefaultMaterial();

    bool isSpecularEnabled() const { return specularAmount > .01f; }
    bool isFresnelEnabled() const { return fresnelPower > 0.0f; }
    bool isVertexColorsEnabled() const { return vertexColors; }
    bool hasLighting() const { return lighting != DefaultMaterialLighting::NoLighting; }

    // Generic method used during serialization
    // to remap string and object pointers
    template <typename TRemapperType>
    void remap(TRemapperType &inRemapper)
    {
        QDemonGraphObject::remap(inRemapper);
        lightmaps.remap(inRemapper);
        inRemapper.remap(iblProbe);
        inRemapper.remap(diffuseMaps[0]);
        inRemapper.remap(diffuseMaps[1]);
        inRemapper.remap(diffuseMaps[2]);
        inRemapper.remap(emissiveMap);
        inRemapper.remap(emissiveMap2);
        inRemapper.remap(specularReflection);
        inRemapper.remap(specularMap);
        inRemapper.remap(roughnessMap);
        inRemapper.remap(opacityMap);
        inRemapper.remap(bumpMap);
        inRemapper.remap(normalMap);
        inRemapper.remap(displacementMap);
        inRemapper.remap(translucencyMap);
        inRemapper.remapMaterial(nextSibling);
        inRemapper.remap(parent);
    }
};

QT_END_NAMESPACE

#endif
