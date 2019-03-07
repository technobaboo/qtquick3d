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

#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

struct DefaultMaterialLighting
{
    enum Enum { NoLighting = 0, VertexLighting, FragmentLighting };
};
struct DefaultMaterialBlendMode
{
    enum Enum { Normal = 0, Screen, Multiply, Overlay, ColorBurn, ColorDodge };
};

struct DefaultMaterialSpecularModel
{
    enum Enum { Default = 0, KGGX, KWard };
};

struct QDemonRenderImage;

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderDefaultMaterial : QDemonGraphObject
{
    QDemonMaterialDirty dirty;
    // lightmap section
    QDemonRenderLightmaps lightmaps;
    // material section
    QDemonRenderImage *iblProbe = nullptr;
    // defaults to vertex
    DefaultMaterialLighting::Enum lighting = DefaultMaterialLighting::VertexLighting;
    // defaults to normal
    DefaultMaterialBlendMode::Enum blendMode = DefaultMaterialBlendMode::Normal;
    QVector3D diffuseColor{ 1.0f, 1.0f, 1.0f }; // colors are 0-1 normalized
    QDemonRenderImage *diffuseMaps[3]{ nullptr, nullptr, nullptr };
    float emissivePower = 0.0f; // 0-100, defaults to 0
    QDemonRenderImage *emissiveMap = nullptr;
    QDemonRenderImage *emissiveMap2 = nullptr;
    QVector3D emissiveColor = { 1.0f, 1.0f, 1.0f };
    QDemonRenderImage *specularReflection = nullptr;
    QDemonRenderImage *specularMap = nullptr;
    DefaultMaterialSpecularModel::Enum specularModel = DefaultMaterialSpecularModel::Default;
    QVector3D specularTint{ 1.0f, 1.0f, 1.0f };
    float ior = 0.2f;
    float fresnelPower = 0.0f;
    float specularAmount = 0.0f; // 0-??, defaults to 0
    float specularRoughness = 50.0f; // 0-??, defaults to 50
    QDemonRenderImage *roughnessMap = nullptr;
    float opacity = 1.0f; // 0-1
    QDemonRenderImage *opacityMap = nullptr;
    QDemonRenderImage *bumpMap = nullptr;
    float bumpAmount = 0.0f; // 0-??
    QDemonRenderImage *normalMap = nullptr;
    QDemonRenderImage *displacementMap = nullptr;
    float displaceAmount = 0.0f; // 0-??
    QDemonRenderImage *translucencyMap = nullptr;
    float translucentFalloff = 0.0f; // 0 - ??
    float diffuseLightWrap = 0.0f; // 0 - 1
    bool vertexColors = false;
    // Materials are stored as a linked list on models.
    QDemonGraphObject *nextSibling = nullptr;
    QDemonRenderModel *parent = nullptr;

    QDemonRenderDefaultMaterial();

    bool isSpecularEnabled() const { return specularAmount > .01f; }
    bool isFresnelEnabled() const { return fresnelPower > 0.0f; }
    bool isVertexColorsEnabled() const { return vertexColors; }
    bool hasLighting() const { return lighting != DefaultMaterialLighting::NoLighting; }
};

QT_END_NAMESPACE

#endif
