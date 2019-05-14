import QtQuick 2.12
import QtDemon 1.0

DemonCustomMaterial {
    property bool uEnvironmentMappingEnabled: true
    property bool uShadowMappingEnabled: false
    property real material_ior: 1.5
    property real roughness: 0.3
    property vector2d texture_tiling: Qt.vector2d(4, 4)
    property real bump_amount: 0.15

    property real intensity: 1
    property vector3d emission_color: Qt.vector3d(0, 0, 0)

    shaderInfo: DemonCustomMaterialShaderInfo {
        version: "330"
        type: "GLSL"
        shaderKey: DemonCustomMaterialShaderInfo.Glossy | DemonCustomMaterialShaderInfo.Diffuse
        layers: 2
    }

    property DemonCustomMaterialTexture uEnvironmentTexture: DemonCustomMaterialTexture {
            id: uEnvironmentTexture
            type: DemonCustomMaterialTexture.Environment
            enabled: uEnvironmentMappingEnabled
            image: DemonImage {
                id: envImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/spherical_checker.jpg"
            }
    }
    property DemonCustomMaterialTexture uBakedShadowTexture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.LightmapShadow
            enabled: uShadowMappingEnabled
            image: DemonImage {
                id: shadowImage
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/shadow.jpg"
            }
    }

    property DemonCustomMaterialTexture reflect_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Specular
            enabled: true
            image: DemonImage {
                id: reflectionTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/bamboo_natural_spec.jpg"
            }
    }

    property DemonCustomMaterialTexture bump_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Bump
            enabled: true
            image: DemonImage {
                id: bumpTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/bamboo_natural_bump.jpg"
            }
    }

    property DemonCustomMaterialTexture diffuse_texture: DemonCustomMaterialTexture {
            type: DemonCustomMaterialTexture.Diffuse
            enabled: true
            image: DemonImage {
                id: diffuseTexture
                tilingmodehorz: DemonImage.Repeat
                tilingmodevert: DemonImage.Repeat
                source: "maps/bamboo_natural.jpg"
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
                source: "maps/emissive.jpg"
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
                source: "maps/emissive_mask.jpg"
            }
    }


    DemonCustomMaterialShader {
        id: bambooEmissiveFragShader
        stage: DemonCustomMaterialShader.Fragment
        shader: "shaders/bambooNaturalMatteEmissive.frag"
    }

    passes: [
        DemonCustomMaterialPass {
            shader: bambooEmissiveFragShader
        }
    ]
}
