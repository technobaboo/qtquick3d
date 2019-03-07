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
#ifndef QDEMON_RENDER_CUSTOM_MATERIAL_H
#define QDEMON_RENDER_CUSTOM_MATERIAL_H

#include <QtDemonRuntimeRender/qdemonrenderdynamicobject.h>
#include <QtDemonRuntimeRender/qdemonrenderimage.h>
#include <QtDemonRuntimeRender/qdemonrenderlightmaps.h>

QT_BEGIN_NAMESPACE

// IMPORTANT: These flags matches the key produced by a MDL export file
enum class QDemonCustomMaterialShaderKeyValues
{
    diffuse = 1 << 0,
    specular = 1 << 1,
    glossy = 1 << 2,
    cutout = 1 << 3,
    refraction = 1 << 4,
    transparent = 1 << 5,
    displace = 1 << 6,
    volumetric = 1 << 7,
    transmissive = 1 << 8,
};

Q_DECLARE_FLAGS(QDemonCustomMaterialShaderKeyFlags, QDemonCustomMaterialShaderKeyValues)
Q_DECLARE_OPERATORS_FOR_FLAGS(QDemonCustomMaterialShaderKeyFlags)

struct Q_DEMONRUNTIMERENDER_EXPORT QDemonRenderCustomMaterial : public QDemonDynamicObject
{
private:
    // These objects are only created via the dynamic object system.
    QDemonRenderCustomMaterial(const QDemonRenderCustomMaterial &);
    QDemonRenderCustomMaterial &operator=(const QDemonRenderCustomMaterial &);
    QDemonRenderCustomMaterial();

public:
    // lightmap section
    QDemonRenderLightmaps m_lightmaps;
    // material section
    bool m_hasTransparency;
    bool m_hasRefraction;
    bool m_hasVolumetricDF;
    QDemonRenderImage *m_iblProbe;
    QDemonRenderImage *m_emissiveMap2;
    QDemonRenderImage *m_displacementMap;
    float m_displaceAmount; ///< depends on the object size

    QDemonGraphObject *m_nextSibling;

    QDemonCustomMaterialShaderKeyFlags m_shaderKeyValues; ///< input from MDL files
    quint32 m_layerCount; ///< input from MDL files

    void initialize(quint32 inKey, quint32 inLayerCount)
    {
        m_lightmaps.m_lightmapIndirect = nullptr;
        m_lightmaps.m_lightmapRadiosity = nullptr;
        m_lightmaps.m_lightmapShadow = nullptr;
        m_hasTransparency = false;
        m_hasRefraction = false;
        m_hasVolumetricDF = false;
        m_nextSibling = nullptr;
        m_dirtyFlagWithInFrame = flags.isDirty();
        m_iblProbe = nullptr;
        m_emissiveMap2 = nullptr;
        m_displacementMap = nullptr;
        m_displaceAmount = 0.0;
        m_shaderKeyValues = static_cast<QDemonCustomMaterialShaderKeyFlags>(inKey);
        m_layerCount = inLayerCount;
    }

    bool isDielectric() const { return m_shaderKeyValues & QDemonCustomMaterialShaderKeyValues::diffuse; }
    bool isSpecularEnabled() const { return m_shaderKeyValues & QDemonCustomMaterialShaderKeyValues::specular; }
    bool isCutOutEnabled() const { return m_shaderKeyValues & QDemonCustomMaterialShaderKeyValues::cutout; }
    bool isVolumetric() const { return m_shaderKeyValues & QDemonCustomMaterialShaderKeyValues::volumetric; }
    bool isTransmissive() const { return m_shaderKeyValues & QDemonCustomMaterialShaderKeyValues::transmissive; }
    bool hasLighting() const { return true; }

    // Dirty
    bool m_dirtyFlagWithInFrame;
    bool isDirty() const { return flags.isDirty() || m_dirtyFlagWithInFrame; }
    void updateDirtyForFrame()
    {
        m_dirtyFlagWithInFrame = flags.isDirty();
        flags.setDirty(false);
    }
};

QT_END_NAMESPACE

#endif
