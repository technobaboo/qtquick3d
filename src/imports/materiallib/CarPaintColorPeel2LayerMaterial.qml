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
import QtDemon 1.0

DemonCustomMaterial {

    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real coating_ior: 1.5
    property real coat_weight: 1.0
    property real coat_roughness: 0
    property vector3d coat_color: Qt.vector3d(1, 1, 1)

    property real flake_intensity: 0.5
    property real flake_size: 0.002
    property real flake_amount: 0.22
    property real flake_roughness: 0.2
    property vector3d flake_color: Qt.vector3d(1, 0.7, 0.02)

    property vector3d base_color: Qt.vector3d(0.1, 0.001, 0.001)
    property real flake_bumpiness: 0.6
    property real peel_size: 1.0
    property real peel_amount: 0.1

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 3
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/shadow.png"
            }
    }

    property DemonCustomMaterialTexture randomGradient1D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/randomGradient1D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient2D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingModeHorizontal: DemonImage.Repeat
                tilingModeVertical: DemonImage.Repeat
                source: "maps/randomGradient2D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient3D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/randomGradient3D.png"
        }
    }
    property DemonCustomMaterialTexture randomGradient4D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingModeHorizontal: DemonImage.Repeat
            tilingModeVertical: DemonImage.Repeat
            source: "maps/randomGradient4D.png"
        }
    }

    DemonCustomMaterialShader {
        id: carpaint2LayerFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/carPaintColorPeel2Layer.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shaders: carpaint2LayerFragShader
        }
    ]
}
