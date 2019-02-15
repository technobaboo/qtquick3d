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
#include <QtDemonRuntimeRender/qdemonrenderdefaultmaterial.h>

QT_BEGIN_NAMESPACE

QDemonRenderDefaultMaterial::QDemonRenderDefaultMaterial()
    : QDemonGraphObject(QDemonGraphObjectTypes::DefaultMaterial)
    , iblProbe(nullptr)
    , lighting(DefaultMaterialLighting::VertexLighting)
    , blendMode(DefaultMaterialBlendMode::Normal)
    , diffuseColor(1, 1, 1)
    , emissivePower(0)
    , emissiveMap(nullptr)
    , emissiveMap2(nullptr)
    , emissiveColor(1, 1, 1)
    , specularReflection(nullptr)
    , specularMap(nullptr)
    , specularModel(DefaultMaterialSpecularModel::Default)
    , specularTint(1, 1, 1)
    , ior(.2f)
    , fresnelPower(0.0f)
    , specularAmount(0)
    , specularRoughness(50)
    , roughnessMap(nullptr)
    , opacity(1)
    , opacityMap(nullptr)
    , bumpMap(nullptr)
    , bumpAmount(0.f)
    , normalMap(nullptr)
    , displacementMap(nullptr)
    , displaceAmount(0.f)
    , translucencyMap(nullptr)
    , translucentFalloff(0.f)
    , diffuseLightWrap(0.f)
    , vertexColors(false)
    , nextSibling(nullptr)
    , parent(nullptr)
{
    lightmaps.m_lightmapIndirect = nullptr;
    lightmaps.m_lightmapRadiosity = nullptr;
    lightmaps.m_lightmapShadow = nullptr;

    diffuseMaps[0] = nullptr;
    diffuseMaps[2] = nullptr;
    diffuseMaps[1] = nullptr;
}

QT_END_NAMESPACE
