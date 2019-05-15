import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real uFresnelPower: 1.0
    property real reflectivity_amount: 1.0
    property real glass_ior: 1.1
    property real roughness: 0.0
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

    DemonCustomMaterialShader {
        id: simpleGlassRefractiveFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/simpleGlassRefractive.frag"
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
            shader: simpleGlassRefractiveFragShader
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
