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
    // These properties names need to match the ones in the shader code!
    property real roughness: 1.0
    property real blur_size: 8.0
    property real refract_depth: 5
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real glass_bfactor: 0.0
    property bool glass_binside: false
    property real uFresnelPower: 1.0
    property real reflectivity_amount: 0.1
    property real glass_ior: 1.1
    property real intLightFall: 2.0
    property real intLightRot: 0.0
    property real intLightBrt: 0.0
    property real bumpScale: 0.5
    property int bumpBands: 1
    property vector3d bumpCoords: Qt.vector3d(1.0, 1.0, 1.0)
    property vector2d intLightPos: Qt.vector2d(0.5, 0.0)
    property vector3d glass_color: Qt.vector3d(0.9, 0.9, 0.9)
    property vector3d intLightCol: Qt.vector3d(0.9, 0.9, 0.9)
    hasTransparency: true

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Refraction | DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture glass_bump: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Environment
        enabled: true
        image: DemonImage {
            id: glassBumpMap
            source: "maps/spherical_checker.png"
        }
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
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
        id: mainShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlass.frag"
    }
    DemonCustomMaterialShader {
        id: noopShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassNoop.frag"
    }
    DemonCustomMaterialShader {
        id: preBlurShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassPreBlur.frag"
    }
    DemonCustomMaterialShader {
        id: blurXShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassBlurX.frag"
    }
    DemonCustomMaterialShader {
        id: blurYShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassBlurY.frag"
    }

    DemonCustomMaterialBuffer {
        id: frameBuffer
        name: "frameBuffer"
        format: DemonCustomMaterialBuffer.Unknown
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 1.0
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    DemonCustomMaterialBuffer {
        id: dummyBuffer
        name: "dummyBuffer"
        format: DemonCustomMaterialBuffer.RGBA8
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 1.0
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    DemonCustomMaterialBuffer {
        id: tempBuffer
        name: "tempBuffer"
        format: DemonCustomMaterialBuffer.RGBA16F
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 0.5
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    DemonCustomMaterialBuffer {
        id: blurYBuffer
        name: "tempBlurY"
        format: DemonCustomMaterialBuffer.RGBA16F
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 0.5
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    DemonCustomMaterialBuffer {
        id: blurXBuffer
        name: "tempBlurX"
        format: DemonCustomMaterialBuffer.RGBA16F
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 0.5
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    passes: [ DemonCustomMaterialPass {
            shaders: noopShader
            output: dummyBuffer
            commands: [ DemonCustomMaterialBufferBlit {
                    destination: frameBuffer
                }
            ]
        }, DemonCustomMaterialPass {
            shaders: preBlurShader
            output: tempBuffer
            commands: [ DemonCustomMaterialBufferInput {
                    buffer: frameBuffer
                    param: "OriginBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            shaders: blurXShader
            output: blurXBuffer
            commands: [ DemonCustomMaterialBufferInput {
                    buffer: tempBuffer
                    param: "BlurBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            shaders: blurYShader
            output: blurYBuffer
            commands: [ DemonCustomMaterialBufferInput {
                    buffer: blurXBuffer
                    param: "BlurBuffer"
                }, DemonCustomMaterialBufferInput {
                    buffer: tempBuffer
                    param: "OriginBuffer"
                }
            ]
        }, DemonCustomMaterialPass {
            shaders: mainShader
            commands: [DemonCustomMaterialBufferInput {
                    buffer: blurYBuffer
                    param: "refractiveTexture"
                }, DemonCustomMaterialBlending {
                    srcBlending: DemonCustomMaterialBlending.SrcAlpha
                    destBlending: DemonCustomMaterialBlending.OneMinusSrcAlpha
                }
            ]
        }
    ]
}
