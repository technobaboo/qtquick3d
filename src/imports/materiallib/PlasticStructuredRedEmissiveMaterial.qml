import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    // These properties names need to match the ones in the shader code!
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real roughness: 0.25
    property real material_ior: 1.8
    property real intensity: 1.0
    property real texture_scaling: 0.1
    property real bump_factor: 0.4
    property vector3d diffuse_color: Qt.vector3d(0.451, 0.04, 0.035)
    property vector3d emission_color: Qt.vector3d(0.0, 0.0, 0.0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 1
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            enabled: uEnvironmentMappingEnabled
            type: DemonCustomMaterialTexture.Environment
            image: DemonImage {
                id: envImage
                source: "maps/spherical_checker.png"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            enabled: uShadowMappingEnabled
            type: DemonCustomMaterialTexture.LightmapShadow
            image: DemonImage {
                id: shadowImage
                source: "maps/shadow.png"
            }
    }
    property DemonCustomMaterialTexture emissive_texture: DemonCustomMaterialTexture {
            id: emissiveTexture
            type: DemonCustomMaterialTexture.Emissive
            enabled: true
            image: DemonImage {
                id: emissiveImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/emissive.png"
            }
    }
    property DemonCustomMaterialTexture emissive_mask_texture: DemonCustomMaterialTexture {
            id: emissiveMaskTexture
            type: DemonCustomMaterialTexture.Unknown
            enabled: true
            image: DemonImage {
                id: emissiveMaskImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/emissive_mask.png"
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
        id: plasticStructuredRedEmissiveFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/plasticStructuredRedEmissive.frag"
    }

    passes: [ DemonCustomMaterialPass {
            shader: plasticStructuredRedEmissiveFragShader
        }
    ]
}
