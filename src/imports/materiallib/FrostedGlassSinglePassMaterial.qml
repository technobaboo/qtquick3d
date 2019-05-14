import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property real roughness: 0.0
    property real blur_size: 8.0
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real uFresnelPower: 1.0
    property real reflectivity_amount: 1.0
    property real glass_ior: 1.1
    property real bumpScale: 0.5
    property real noiseScale: 2.0
    property int bumpBands: 1
    property vector3d noiseCoords: Qt.vector3d(1.0, 1.0, 1.0)
    property vector3d bumpCoords: Qt.vector3d(1.0, 1.0, 1.0)
    property vector3d glass_color: Qt.vector3d(0.9, 0.9, 0.9)
    hasTransparency: true

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Refraction | DemonCustomMaterialShaderInfo.Glossy
        layers: 1
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.jpg"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.jpg"
            }
    }
    property DemonCustomMaterialTexture randomGradient1D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient1D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient2D: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Unknown; //Gradient
            image: DemonImage {
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/randomGradient2D.png"
            }
    }
    property DemonCustomMaterialTexture randomGradient3D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/randomGradient3D.png"
        }
    }
    property DemonCustomMaterialTexture randomGradient4D: DemonCustomMaterialTexture {
        type: DemonCustomMaterialTexture.Unknown; //Gradient
        image: DemonImage {
            tilingmodehorz: DemonImage.Repeat
            tilingmodevert: DemonImage.Repeat
            source: "maps/randomGradient4D.png"
        }
    }

    DemonCustomMaterialShader {
        id: noopShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassNoop.frag"
    }

    DemonCustomMaterialShader {
        id: frostedGlassSpFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/frostedThinGlassSp.frag"
    }

    DemonCustomMaterialBuffer {
        id: tempBuffer
        name: "temp_buffer"
        format: DemonCustomMaterialBuffer.Unknown
        magOp: DemonCustomMaterialBuffer.Linear
        coordOp: DemonCustomMaterialBuffer.ClampToEdge
        sizeMultiplier: 1.0
        bufferFlags: DemonCustomMaterialBuffer.None // aka frame
    }

    passes: [ DemonCustomMaterialPass {
            shader: noopShader
            output: tempBuffer
        }, DemonCustomMaterialPass {
            shader: frostedGlassSpFragShader
            commands: [ DemonCustomMaterialBufferBlit {
                    destination: tempBuffer
                }, DemonCustomMaterialBufferInput {
                    buffer: tempBuffer
                    param: "refractiveTexture"
                }, DemonCustomMaterialBlending {
                    srcBlending: DemonCustomMaterialBlending.SrcAlpha
                    destBlending: DemonCustomMaterialBlending.OneMinusSrcAlpha
                }
            ]
        }
    ]
}
