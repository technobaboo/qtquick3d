/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick 3D.
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

import QtQuick 2.12
import QtQuick3D 1.0

CustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 8.0
    property real reflection_map_offset: 1.0
    property real reflection_map_scale: 1.0
    property vector2d tiling: Qt.vector2d(1, 1)
    property real roughness_map_offset: 0.0
    property real roughness_map_scale: 1.0
    property real anisotropy: 0.8
    property real aniso_tex_color_offset: 0.0
    property real aniso_tex_color_scale: 1.0
    property vector2d aniso_tiling: Qt.vector2d(1, 1)
    property real base_weight: 1.0
    property real bump_amount: 0.0

    shaderInfo: CustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: CustomMaterialShaderInfo.Glossy | CustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property CustomMaterialTexture uEnvironmentTexture: CustomMaterialTexture {
            id: uEnvironmentTexture
            type: CustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: Texture {
                id: envImage
                tilingModeHorizontal: Texture.Repeat
                tilingModeVertical: Texture.Repeat
                source: "maps/spherical_checker.png"
            }
    }
    property CustomMaterialTexture uBakedShadowTexture: CustomMaterialTexture {
            type: CustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: Texture {
                id: shadowImage
                tilingModeHorizontal: Texture.Repeat
                tilingModeVertical: Texture.Repeat
                source: "maps/shadow.png"
            }
    }

    property CustomMaterialTexture reflection_texture: CustomMaterialTexture {
            type: CustomMaterialTexture.Specular
            enabled: true
            image: Texture {
                id: reflectionTexture
                tilingModeHorizontal: Texture.Repeat
                tilingModeVertical: Texture.Repeat
                source: "maps/concentric_milled_steel.png"
            }
    }

    property CustomMaterialTexture aniso_rot_texture: CustomMaterialTexture {
            type: CustomMaterialTexture.Anisotropy
            enabled: true
            image: Texture {
                id: anisoTexture
                tilingModeHorizontal: Texture.Repeat
                tilingModeVertical: Texture.Repeat
                source: "maps/concentric_milled_steel_aniso.png"
            }
    }

    property CustomMaterialTexture bump_texture: CustomMaterialTexture {
            type: CustomMaterialTexture.Bump
            enabled: true
            image: Texture {
                id: bumpTexture
                tilingModeHorizontal: Texture.Repeat
                tilingModeVertical: Texture.Repeat
                source: "maps/concentric_milled_steel.png"
            }
    }
    CustomMaterialShader {
        id: aluminumTexturedAnisoFragShader
        stage: CustomMaterialShader.Fragment
        shader: "shaders/aluminumTexturedAnisotropic.frag"
    }

    passes: [
        CustomMaterialPass {
            shaders: aluminumTexturedAnisoFragShader
        }
    ]
}
