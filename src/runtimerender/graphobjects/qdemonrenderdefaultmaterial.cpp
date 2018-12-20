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

SDefaultMaterial::SDefaultMaterial()
    : SGraphObject(GraphObjectTypes::DefaultMaterial)
    , m_IblProbe(nullptr)
    , m_Lighting(DefaultMaterialLighting::VertexLighting)
    , m_BlendMode(DefaultMaterialBlendMode::Normal)
    , m_DiffuseColor(1, 1, 1)
    , m_EmissivePower(0)
    , m_EmissiveMap(nullptr)
    , m_EmissiveMap2(nullptr)
    , m_EmissiveColor(1, 1, 1)
    , m_SpecularReflection(nullptr)
    , m_SpecularMap(nullptr)
    , m_SpecularModel(DefaultMaterialSpecularModel::Default)
    , m_SpecularTint(1, 1, 1)
    , m_IOR(.2f)
    , m_FresnelPower(0.0f)
    , m_SpecularAmount(0)
    , m_SpecularRoughness(50)
    , m_RoughnessMap(nullptr)
    , m_Opacity(1)
    , m_OpacityMap(nullptr)
    , m_BumpMap(nullptr)
    , m_BumpAmount(0.f)
    , m_NormalMap(nullptr)
    , m_DisplacementMap(nullptr)
    , m_DisplaceAmount(0.f)
    , m_TranslucencyMap(nullptr)
    , m_TranslucentFalloff(0.f)
    , m_DiffuseLightWrap(0.f)
    , m_VertexColors(false)
    , m_NextSibling(nullptr)
    , m_Parent(nullptr)
{
    m_Lightmaps.m_LightmapIndirect = nullptr;
    m_Lightmaps.m_LightmapRadiosity = nullptr;
    m_Lightmaps.m_LightmapShadow = nullptr;

    m_DiffuseMaps[0] = nullptr;
    m_DiffuseMaps[2] = nullptr;
    m_DiffuseMaps[1] = nullptr;
}

QT_END_NAMESPACE
